#include "Material.h"
#include <algorithm>
#include <cmath>

// ========== LambertMaterial ==========
LambertMaterial::LambertMaterial(const Color& albedo)
    : mAlbedo(albedo)
{
}

Color LambertMaterial::BRDF(const Vector3f& wo, const Vector3f& wi) const
{
    return mAlbedo * INV_PI; // Lambert BRDF
}

// ========== ConductorSpecularMaterial ==========
ConductorSpecularMaterial::ConductorSpecularMaterial(const Color& eta,
                                                     const Color& absorptionCoef,
                                                     const Color& reflectionColor)
    : mEta(eta)
    , mAbsorptionCoef(absorptionCoef)
    , mReflectionColor(reflectionColor)
{
}

bool ConductorSpecularMaterial::IsSpecular() const
{
    return true;
}

Color ConductorSpecularMaterial::BRDF(const Vector3f& wo, const Vector3f& wi) const
{
    if (!(fabs(wo.x + wi.x) < 0.0001f &&
          fabs(wo.y + wi.y) < 0.0001f &&
          fabs(wo.z - wi.z) < 0.0001f))
    {
        return Color(0.0f, 0.0f, 0.0f);
    }

    float cosTheta = std::max(1e-5f, wi.z);
    Color k = mAbsorptionCoef;

    Color r1 = ((mEta * mEta + k * k) * cosTheta * cosTheta - 2.0f * mEta * cosTheta + 1.0f)
               / ((mEta * mEta + k * k) * cosTheta * cosTheta + 2.0f * mEta * cosTheta + 1.0f);

    Color r2 = (mEta * mEta + k * k - 2.0f * mEta * cosTheta + cosTheta * cosTheta)
               / (mEta * mEta + k * k + 2.0f * mEta * cosTheta + cosTheta * cosTheta);

    Color Fr = (r1 + r2) * 0.5f;

    return Fr * mReflectionColor / std::max(fabs(cosTheta), 0.0001f); // Conductor BRDF
}

// 金属永远反射
float ConductorSpecularMaterial::Reflectance(const Vector3f& wi) const
{
    return 1.0f;
}

// ========== DielectricSpecularMaterial ==========
DielectricSpecularMaterial::DielectricSpecularMaterial(float eta, const Color& transmissionColor)
    : mEta(eta)
    , mTransmissionColor(transmissionColor)
{
}

bool DielectricSpecularMaterial::IsSpecular() const
{
    return true;
}

bool DielectricSpecularMaterial::IsRefractable() const
{
    return true;
}

Color DielectricSpecularMaterial::BRDF(const Vector3f& wo, const Vector3f& wi) const
{
    float cos_i = fabs(wi.z);
    float eta_i = 1.0f, eta_t = mEta;

    // 修正：光线从介质射向空气时，wi.z > 0
    if (wi.z > 0.0f)
    {
        eta_i = mEta;
        eta_t = 1.0f;
    }

    Vector3f transmittedDir;
    float Fr = 1.0f;

    if (ComputeRefracVector(wi, eta_i, eta_t, transmittedDir))
    {
        float cos_t = fabs(transmittedDir.z);
        Fr = Fresnel(eta_i, eta_t, cos_i, cos_t);
    }

    return Color(Fr) / std::max(fabs(wi.z), 0.0001f); // Dielectric BRDF
}

Color DielectricSpecularMaterial::BTDF(const Vector3f& transmittedDir, const Vector3f& wi) const
{
    float cos_i = fabs(wi.z);
    float cos_t = fabs(transmittedDir.z);

    float eta_i = 1.0f, eta_t = mEta;
    // 修正：光线从介质射向空气时，wi.z > 0
    if (wi.z > 0.0f)
    {
        eta_i = mEta;
        eta_t = 1.0f;
    }

    float Fr = Fresnel(eta_i, eta_t, cos_i, cos_t);
    return mTransmissionColor * (1.0f - Fr) * eta_t * eta_t / (eta_i * eta_i * std::max(cos_t, 1e-4f));
}

float DielectricSpecularMaterial::Fresnel(float eta_i, float eta_t, float cos_i, float cos_t) const
{
    float r1 = (eta_t * cos_i - eta_i * cos_t) / (eta_t * cos_i + eta_i * cos_t);
    float r2 = (eta_i * cos_i - eta_t * cos_t) / (eta_i * cos_i + eta_t * cos_t);
    return 0.5f * (r1 * r1 + r2 * r2);
}

bool DielectricSpecularMaterial::SampleWt(const Vector3f& wi, Vector3f& transmittedDir) const
{
    float eta_i = 1.0f, eta_t = mEta;
    // 修正：光线从介质射向空气时，wi.z > 0
    if (wi.z > 0.0f)
    {
        eta_i = mEta;
        eta_t = 1.0f;
    }

    return ComputeRefracVector(wi, eta_i, eta_t, transmittedDir);
}

// 计算反射概率
float DielectricSpecularMaterial::Reflectance(const Vector3f& wi) const
{
    float cos_i = fabs(wi.z);
    float eta_i = 1.0f, eta_t = mEta;
    if (wi.z > 0.0f)
    {
        eta_i = mEta;
        eta_t = 1.0f;
    }

    Vector3f transmittedDir;
    if (!ComputeRefracVector(wi, eta_i, eta_t, transmittedDir))
        return 1.0f; // 全反射

    float cos_t = fabs(transmittedDir.z);
    return Fresnel(eta_i, eta_t, cos_i, cos_t);
}