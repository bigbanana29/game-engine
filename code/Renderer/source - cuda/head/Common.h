#pragma once

// CUDA 自带 PI 宏，会覆盖我们的 PI，先取消
#ifdef PI
#undef PI
#endif

#ifdef INV_PI
#undef INV_PI
#endif

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>
#include <random>
#include <cmath>

using Vector2f = glm::vec2;
using Vector3f = glm::vec3;
using Vector4f = glm::vec4;

using Vector2i = glm::ivec2;
using Vector3i = glm::ivec3;
using Vector4i = glm::ivec4;

using Matrix3x3 = glm::mat3;
using Matrix4x4 = glm::mat4;

using Color = glm::vec3;

// 常量声明
extern const float PI;
extern const float INV_PI;

void DumpVector(const Vector3f& v);
Matrix4x4 MakeTranslation(const Vector3f& t);
Matrix4x4 MakeRotation(const Vector3f& eular);
Matrix4x4 MakeScale(float s);
Matrix4x4 MakeWorldTransform(const Vector3f& position, const Vector3f& rotation, float s);
Matrix3x3 MakeCoordinateSystem(const Vector3f w);

inline float Random01()
{
    thread_local static std::mt19937 gen(std::random_device{}());
    thread_local static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(gen);
}

inline float Random(float a, float b)
{
    return a + (b - a) * Random01();
}

inline Vector3f GetSphericalCoordinate(float theta, float phi)
{
    return Vector3f(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
}

// 计算折射方向（原 wt 改为输出参数 transmittedDir）
inline bool ComputeRefracVector(const Vector3f& wi, float eta_i, float eta_t, Vector3f& transmittedDir)
{
    float cos_i = wi.z;
    float sin_i = sqrt(1.0f - cos_i * cos_i);
    float sin_t = sin_i * eta_i / eta_t;

    // 全反射
    if (sin_t > 1.0f)
    {
        return false;
    }
    
    float cos_t = sqrt(1.0f - sin_t * sin_t);
    transmittedDir.z = wi.z > 0.0f ? cos_t : -cos_t;
    transmittedDir.x = wi.x * sin_t / glm::max(1e-5f, sin_i);
    transmittedDir.y = wi.y * sin_t / glm::max(1e-5f, sin_i);

    transmittedDir = glm::normalize(transmittedDir);
    return true;
}