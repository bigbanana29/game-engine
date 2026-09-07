#include "Common.h"
#include <cmath>

const float PI = glm::pi<float>();
const float INV_PI = 1.0f / PI;

void DumpVector(const Vector3f& v) {
    std::cout << "Vector3f(" << v.x << "," << v.y << "," << v.z << ")" << std::endl;
}

Matrix4x4 MakeTranslation(const Vector3f& t) {
    return Matrix4x4{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        t.x, t.y, t.z, 1.0f
    };
}

Matrix4x4 MakeRotation(const Vector3f& eular) {
    float cx = cosf(eular.x);
    float sx = sinf(eular.x);
    float cy = cosf(eular.y);
    float sy = sinf(eular.y);
    float cz = cosf(eular.z);
    float sz = sinf(eular.z);

    Matrix4x4 rx = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cx, sx, 0.0f,
        0.0f, -sx, cx, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    Matrix4x4 ry = {
        cy, 0.0f, -sy, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        sy, 0.0f, cy, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    Matrix4x4 rz = {
        cz, sz, 0.0f, 0.0f,
        -sz, cz, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return rz * ry * rx;
}

Matrix4x4 MakeScale(float s) {
    return Matrix4x4{
        s, 0.0f, 0.0f, 0.0f,
        0.0f, s, 0.0f, 0.0f,
        0.0f, 0.0f, s, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

Matrix4x4 MakeWorldTransform(const Vector3f& position, const Vector3f& rotation, float s) {
    Matrix4x4 T = MakeTranslation(position);
    Matrix4x4 R = MakeRotation(rotation);
    Matrix4x4 S = MakeScale(s);
    return T * R * S;
}

Matrix3x3 MakeCoordinateSystem(const Vector3f w) {
    Vector3f u = Vector3f(1.0f, 0.0f, 0.0f);
    if (glm::dot(u, w) > 0.99f) {
        u = Vector3f(0.0f, 1.0f, 0.0f);
    }
    Vector3f v = glm::cross(u, w);
    u = glm::cross(v, w);
    u = glm::normalize(u);
    v = glm::normalize(v);
    return Matrix3x3(u, v, w);
}