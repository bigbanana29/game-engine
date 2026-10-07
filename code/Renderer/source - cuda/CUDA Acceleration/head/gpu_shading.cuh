#pragma once
#include "gpu_type.h"
#include "gpu_math.cuh"

// ============================================================
// Lambert BRDF（对照原 LambertMaterial::BRDF）
// ============================================================
__device__ inline float3 lambert_brdf(const MaterialData& mat, float3 wo, float3 wi) {
    return mat.albedo * INV_PI;
}

// ============================================================
// Conductor BRDF（对照原 ConductorSpecularMaterial::BRDF）
// ============================================================
__device__ inline float3 conductor_brdf(const MaterialData& mat, float3 wo, float3 wi) {
    if (!(fabsf(wo.x + wi.x) < 1e-4f
        && fabsf(wo.y + wi.y) < 1e-4f
        && fabsf(wo.z - wi.z) < 1e-4f))
    {
        return make_float3(0.0f, 0.0f, 0.0f);
    }

    float cosTheta = fmaxf(1e-5f, wi.z);
    float3 k = mat.absorptionCoef;
    float3 eta2 = mat.eta * mat.eta;
    float3 k2 = k * k;

    float3 r1 = ((eta2 + k2) * cosTheta * cosTheta - 2.0f * mat.eta * cosTheta + 1.0f)
              / ((eta2 + k2) * cosTheta * cosTheta + 2.0f * mat.eta * cosTheta + 1.0f);

    float3 r2 = (eta2 + k2 - 2.0f * mat.eta * cosTheta + cosTheta * cosTheta)
              / (eta2 + k2 + 2.0f * mat.eta * cosTheta + cosTheta * cosTheta);
    float3 Fr = (r1 + r2) * 0.5f;

    return Fr * mat.reflectionColor / fmaxf(fabsf(cosTheta), 1e-4f);
}

// ============================================================
// Fresnel（对照原 DielectricSpecularMaterial::Fresnel）
// ============================================================
__device__ inline float dielectric_fresnel(float eta_i, float eta_t, float cos_i, float cos_t) {
    float r1 = (eta_t * cos_i - eta_i * cos_t) / fmaxf(1e-4f, eta_t * cos_i + eta_i * cos_t);
    float r2 = (eta_i * cos_i - eta_t * cos_t) / fmaxf(1e-4f, eta_i * cos_i + eta_t * cos_t);
    return 0.5f * (r1 * r1 + r2 * r2);
}

// ============================================================
// Dielectric BRDF（对照原 DielectricSpecularMaterial::BRDF）
// ============================================================
__device__ inline float3 dielectric_brdf(const MaterialData& mat, float3 wo, float3 wi) {
    float cos_i = fabsf(wi.z);
    float eta_i, eta_t;
    if (wi.z > 0) {
        eta_i = 1.0f;
        eta_t = mat.ior;
    } else {
        eta_i = mat.ior;
        eta_t = 1.0f;
    }

    float3 wt;
    float Fr = 1.0f;
    if (compute_refrac_vector(wi, eta_i, eta_t, wt)) {
        float cos_t = fabsf(wt.z);
        Fr = dielectric_fresnel(eta_i, eta_t, cos_i, cos_t);
    }
    return make_float3(Fr, Fr, Fr) / fmaxf(cos_i, 1e-3f);
}

// ============================================================
// Dielectric BTDF（对照原 DielectricSpecularMaterial::BTDF）
// ============================================================
__device__ inline float3 dielectric_btdf(const MaterialData& mat, float3 wt, float3 wi) {
    float cos_i = fabsf(wi.z);
    float eta_i, eta_t;
    if (wi.z > 0) {
        eta_i = 1.0f;
        eta_t = mat.ior;
    } else {
        eta_i = mat.ior;
        eta_t = 1.0f;
    }

    float cos_t = fabsf(wt.z);
    float Fr = dielectric_fresnel(eta_i, eta_t, cos_i, cos_t);
    float factor = fmaxf(0.0f, 1.0f - Fr) * eta_t * eta_t / (eta_i * eta_i * fmaxf(cos_i, 1e-3f));
    return mat.transmissionColor * factor;
}

// ============================================================
// SampleWt（对照原 DielectricSpecularMaterial::SampleWt）
// ============================================================
__device__ inline bool dielectric_sample_wt(const MaterialData& mat, float3 wi, float3& wt) {
    float eta_i, eta_t;
    if (wi.z > 0) {
        eta_i = 1.0f;
        eta_t = mat.ior;
    } else {
        eta_i = mat.ior;
        eta_t = 1.0f;
    }

    return compute_refrac_vector(wi, eta_i, eta_t, wt);
}

// ============================================================
// 统一 BRDF 入口
// ============================================================
__device__ inline float3 material_brdf(const MaterialData& mat, float3 wo, float3 wi) {
    switch (mat.type) {
        case MAT_LAMBERT:     return lambert_brdf(mat, wo, wi);
        case MAT_CONDUCTOR:   return conductor_brdf(mat, wo, wi);
        case MAT_DIELECTRIC:  return dielectric_brdf(mat, wo, wi);
    }
    return make_float3(0, 0, 0);
}

// ============================================================
// 统一 BTDF 入口
// ============================================================
__device__ inline float3 material_btdf(const MaterialData& mat, float3 wt, float3 wi) {
    if (mat.type != MAT_DIELECTRIC) return make_float3(0, 0, 0);
    return dielectric_btdf(mat, wt, wi);
}

// ============================================================
// 是否镜面
// ============================================================
__device__ inline bool material_is_specular(const MaterialData& mat) {
    return mat.type != MAT_LAMBERT;
}

// ============================================================
// 光源采样（对照原 Light::GetRadiance）
// ============================================================
__device__ inline float3 light_radiance(const LightData& light,
                                         float3 p, float3& sourcePos) {
    switch (light.type) {
        case LIGHT_DIRECTIONAL:
            sourcePos = p - light.direction * 100000.0f;
            return light.intensity;

        case LIGHT_POINT: {
            sourcePos = light.position;
            float R = length(p - light.position);
            if (R < 1e-6f) R = 1e-6f;
            // attenuation = (A, B, C) 对应公式 1/(C + B*R + A*R^2)
            float atten = 1.0f / (light.attenuation.z +
                                  light.attenuation.y * R +
                                  light.attenuation.x * R * R);
            return light.intensity * atten;
        }

        case LIGHT_SPOT: {
            sourcePos = light.position;
            float R = length(p - light.position);
            if (R < 1e-6f) R = 1e-6f;
            float k1 = 1.0f / (light.attenuation.z +
                               light.attenuation.y * R +
                               light.attenuation.x * R * R);

            float3 L = normalize(p - light.position);
            float cosTheta = dot(L, light.direction);
            float k2 = (cosTheta - light.cosOuterAngle) /
                       (light.cosInnerAngle - light.cosOuterAngle);
            k2 = fmaxf(0.0f, fminf(1.0f, k2));

            return light.intensity * k1 * k2;
        }
    }
    return make_float3(0, 0, 0);
}