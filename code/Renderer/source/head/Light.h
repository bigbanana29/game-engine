#pragma once
#include "Ray.h"

class Light
{
public:
    //對於一點p，求他的L(p)
    virtual Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const = 0;
    virtual ~Light() = default;
};

//平行光
class DirectionalLight : public Light
{
public:
    DirectionalLight(const Vector3f& direction, const Color& radiance);
    Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override;

private:
    Vector3f mDirection;//光線方向,單位向量
    Color mRadiance;//光線強度
};

//點光源
class PointLight : public Light
{
public:
    PointLight(const Vector3f& position, const Color& intensity, const Vector3f& attenuation);
    Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override;

private:
    Vector3f mPosition;//光源位置
    Color mIntensity;//光線強度
    Vector3f mAttenuations;//衰減係數,分別為A, B, C, 對應公式I/(A + B*d + C*d^2)
};

//聚光燈
class SpotLight : public Light
{
public:
    SpotLight(const Vector3f& position, const Vector3f& direction,
              const Color& intensity, float innerAngle, float outerAngle,
              const Vector3f& attenuation);
    Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override;

private:
    Vector3f mPosition; //光源位置
    Vector3f mDirection; //光線方向,單位向量
    Color mIntensity; //光線強度

    float mCosInnerAngle; //內角度,單位為弧度，alpha
    float mCosOuterAngle; //外角度，單位爲弧度，beta

    Vector3f mAttenuations; //衰減係數,分別為A, B, C
};