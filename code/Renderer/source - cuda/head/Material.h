#pragma once

#include "Ray.h"
#include "Common.h"

class Material
{
public:
    virtual Color BRDF(const Vector3f& wo, const Vector3f& wi) const = 0;
    virtual Color BTDF(const Vector3f& transmittedDir, const Vector3f& wi) const
    {
        return Color(0.0f, 0.0f, 0.0f);
    }
    virtual bool IsSpecular() const { return false; }
    virtual bool IsRefractable() const { return false; }
    virtual bool SampleWt(const Vector3f& wi, Vector3f& transmittedDir) const
    {
        return false;
    }
    virtual float Reflectance(const Vector3f& wi) const { return 0.0f; }
};

class LambertMaterial : public Material
{
public:
    LambertMaterial(const Color& albedo);
    virtual Color BRDF(const Vector3f& wo, const Vector3f& wi) const override;

    // ===== 新增 Getter =====
    const Color& GetAlbedo() const { return mAlbedo; }

private:
    Color mAlbedo;
};

class ConductorSpecularMaterial : public Material
{
public:
    ConductorSpecularMaterial(const Color& eta,
                              const Color& absorptionCoef,
                              const Color& reflectionColor);
    virtual bool IsSpecular() const override;
    virtual Color BRDF(const Vector3f& wo, const Vector3f& wi) const override;
    virtual float Reflectance(const Vector3f& wi) const override;

    // ===== 新增 Getter =====
    const Color& GetEta() const { return mEta; }
    const Color& GetAbsorptionCoef() const { return mAbsorptionCoef; }
    const Color& GetReflectionColor() const { return mReflectionColor; }

private:
    Color mEta;
    Color mAbsorptionCoef;
    Color mReflectionColor;
};

class DielectricSpecularMaterial : public Material
{
public:
    DielectricSpecularMaterial(float eta, const Color& transmissionColor);
    virtual bool IsSpecular() const override;
    virtual bool IsRefractable() const override;
    virtual Color BRDF(const Vector3f& wo, const Vector3f& wi) const override;
    virtual Color BTDF(const Vector3f& transmittedDir, const Vector3f& wi) const override;
    virtual bool SampleWt(const Vector3f& wi, Vector3f& transmittedDir) const override;
    virtual float Reflectance(const Vector3f& wi) const override;

    // ===== 新增 Getter =====
    float GetEta() const { return mEta; }
    const Color& GetTransmissionColor() const { return mTransmissionColor; }

private:
    float Fresnel(float eta_i, float eta_t, float cos_i, float cos_t) const;
    float mEta;
    Color mTransmissionColor;
};