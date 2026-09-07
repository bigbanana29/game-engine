#pragma once

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

using Vector2f = glm::vec2;
using Vector3f = glm::vec3;
using Vector4f = glm::vec4;

using Vector2i = glm::ivec2;
using Vector3i = glm::ivec3;
using Vector4i = glm::ivec4;

using Matrix3x3 = glm::mat3;
using Matrix4x4 = glm::mat4;

using Color = glm::vec3;

// 声明（不定义）
extern const float PI;
extern const float INV_PI;

void DumpVector(const Vector3f& v);
Matrix4x4 MakeTranslation(const Vector3f& t);
Matrix4x4 MakeRotation(const Vector3f& eular);
Matrix4x4 MakeScale(float s);
Matrix4x4 MakeWorldTransform(const Vector3f& position, const Vector3f& rotation, float s);
Matrix3x3 MakeCoordinateSystem(const Vector3f w);