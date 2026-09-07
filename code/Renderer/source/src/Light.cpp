#include "Light.h"

// ---------- DirectionalLight ----------
DirectionalLight::DirectionalLight(const Vector3f& direction, const Color& radiance)
    : mDirection(direction), mRadiance(radiance) {}

Color DirectionalLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos) const
{
    sourcePos = p - mDirection * 100000.0f; //假設光源在無限遠處
    return mRadiance;
}

// ---------- PointLight ----------
PointLight::PointLight(const Vector3f& position, const Color& intensity, const Vector3f& attenuation)
    : mPosition(position), mIntensity(intensity), mAttenuations(attenuation) {}

Color PointLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos) const
{
    sourcePos = mPosition;
    float R = glm::length(p - mPosition);

    if (R < 1e-6f) R = 1e-6f; //避免除以零
    float attenuation = 1.0f / (mAttenuations.z + mAttenuations.y * R + mAttenuations.x * R * R);
    return mIntensity * attenuation;
}

// ---------- SpotLight ----------
SpotLight::SpotLight(const Vector3f& position, const Vector3f& direction,
                     const Color& intensity, float innerAngle, float outerAngle,
                     const Vector3f& attenuation)
    : mPosition(position), mDirection(glm::normalize(direction)), mIntensity(intensity),
      mCosInnerAngle(std::cos(innerAngle)), mCosOuterAngle(std::cos(outerAngle)),
      mAttenuations(attenuation) {}

Color SpotLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos) const
{
    sourcePos = mPosition;
    //距離衰減，k1
    float R = glm::length(p - mPosition);
    if (R < 1e-6f) R = 1e-6f;
    float k1 = 1.0f / (mAttenuations.z + mAttenuations.y * R + mAttenuations.x * R * R);

    //角度衰減，k2
    Vector3f L = glm::normalize(p - mPosition);
    float cosTheta = glm::dot(L, mDirection);

    float k2 = (cosTheta - mCosOuterAngle) / (mCosInnerAngle - mCosOuterAngle);

    return mIntensity * k1 * glm::clamp(k2, 0.0f, 1.0f);
}