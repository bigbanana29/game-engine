#pragma once
#include "Ray.h"

class Light
{
public:
    virtual Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const = 0;
    virtual ~Light() = default;
};

class DirectionalLight : public Light
{
public:
    DirectionalLight(const Vector3f& direction, const Color& radiance);
    Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override;

    // ===== 新增 Getter =====
    const Vector3f& GetDirection() const { return mDirection; }
    const Color& GetRadianceColor() const { return mRadiance; }

private:
    Vector3f mDirection;
    Color mRadiance;
};

class PointLight : public Light
{
public:
    PointLight(const Vector3f& position, const Color& intensity, const Vector3f& attenuation);
    Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override;

    // ===== 新增 Getter =====
    const Vector3f& GetPosition() const { return mPosition; }
    const Color& GetIntensity() const { return mIntensity; }
    const Vector3f& GetAttenuations() const { return mAttenuations; }

private:
    Vector3f mPosition;
    Color mIntensity;
    Vector3f mAttenuations;
};

class SpotLight : public Light
{
public:
    SpotLight(const Vector3f& position, const Vector3f& direction,
              const Color& intensity, float innerAngle, float outerAngle,
              const Vector3f& attenuation);
    Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override;

    // ===== 新增 Getter =====
    const Vector3f& GetPosition() const { return mPosition; }
    const Vector3f& GetDirection() const { return mDirection; }
    const Color& GetIntensity() const { return mIntensity; }
    float GetCosInnerAngle() const { return mCosInnerAngle; }
    float GetCosOuterAngle() const { return mCosOuterAngle; }
    const Vector3f& GetAttenuations() const { return mAttenuations; }

private:
    Vector3f mPosition;
    Vector3f mDirection;
    Color mIntensity;
    float mCosInnerAngle;
    float mCosOuterAngle;
    Vector3f mAttenuations;
};