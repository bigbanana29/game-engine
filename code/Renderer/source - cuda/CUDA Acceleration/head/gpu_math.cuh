#pragma once
#include "gpu_type.h"
#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <math.h>

#define PI 3.14159265358979323846f
#define INV_PI 0.31830988618379067154f

// float3 用 CUDA 自带的，make_float3 也用 CUDA 自带的
// 这里只重载运算符

// ============================================================
// float3 与 float3
// ============================================================
__host__ __device__ inline float3 operator+(float3 a, float3 b) {
    return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

__host__ __device__ inline float3 operator-(float3 a, float3 b) {
    return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

__host__ __device__ inline float3 operator-(float3 a) {
    return make_float3(-a.x, -a.y, -a.z);
}

__host__ __device__ inline float3 operator*(float3 a, float3 b) {
    return make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
}

__host__ __device__ inline float3 operator/(float3 a, float3 b) {
    return make_float3(a.x / b.x, a.y / b.y, a.z / b.z);
}

// ============================================================
// float3 与 float
// ============================================================
__host__ __device__ inline float3 operator+(float3 a, float s) {
    return make_float3(a.x + s, a.y + s, a.z + s);
}

__host__ __device__ inline float3 operator+(float s, float3 a) {
    return make_float3(a.x + s, a.y + s, a.z + s);
}

__host__ __device__ inline float3 operator-(float3 a, float s) {
    return make_float3(a.x - s, a.y - s, a.z - s);
}

__host__ __device__ inline float3 operator-(float s, float3 a) {
    return make_float3(s - a.x, s - a.y, s - a.z);
}

__host__ __device__ inline float3 operator*(float3 a, float s) {
    return make_float3(a.x * s, a.y * s, a.z * s);
}

__host__ __device__ inline float3 operator*(float s, float3 a) {
    return make_float3(a.x * s, a.y * s, a.z * s);
}

__host__ __device__ inline float3 operator/(float3 a, float s) {
    return make_float3(a.x / s, a.y / s, a.z / s);
}

__host__ __device__ inline float3 operator/(float s, float3 a) {
    return make_float3(s / a.x, s / a.y, s / a.z);
}

// ============================================================
// 复合赋值
// ============================================================
__host__ __device__ inline float3& operator+=(float3& a, float3 b) {
    a.x += b.x; a.y += b.y; a.z += b.z;
    return a;
}

__host__ __device__ inline float3& operator-=(float3& a, float3 b) {
    a.x -= b.x; a.y -= b.y; a.z -= b.z;
    return a;
}

__host__ __device__ inline float3& operator*=(float3& a, float s) {
    a.x *= s; a.y *= s; a.z *= s;
    return a;
}

__host__ __device__ inline float3& operator*=(float3& a, float3 b) {
    a.x *= b.x; a.y *= b.y; a.z *= b.z;
    return a;
}

__host__ __device__ inline float3& operator/=(float3& a, float s) {
    a.x /= s; a.y /= s; a.z /= s;
    return a;
}

// ============================================================
// 向量运算
// ============================================================
__host__ __device__ inline float dot(float3 a, float3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

__host__ __device__ inline float3 cross(float3 a, float3 b) {
    return make_float3(a.y * b.z - a.z * b.y,
                       a.z * b.x - a.x * b.z,
                       a.x * b.y - a.y * b.x);
}

__host__ __device__ inline float length(float3 v) {
    return sqrtf(dot(v, v));
}

__host__ __device__ inline float3 normalize(float3 v) {
    float len = length(v);
    return len > 1e-8f ? v / len : v;
}

// ============================================================
// float4x4 运算
// ============================================================
__host__ __device__ inline float4x4 make_identity() {
    float4x4 r;
    for (int i = 0; i < 16; i++) r.m[i] = 0;
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1;
    return r;
}

__host__ __device__ inline float3 transform_point(float4x4 m, float3 p) {
    return make_float3(
        m.m[0]*p.x + m.m[1]*p.y + m.m[2]*p.z + m.m[3],
        m.m[4]*p.x + m.m[5]*p.y + m.m[6]*p.z + m.m[7],
        m.m[8]*p.x + m.m[9]*p.y + m.m[10]*p.z + m.m[11]);
}

__host__ __device__ inline float3 transform_vector(float4x4 m, float3 v) {
    return make_float3(
        m.m[0]*v.x + m.m[1]*v.y + m.m[2]*v.z,
        m.m[4]*v.x + m.m[5]*v.y + m.m[6]*v.z,
        m.m[8]*v.x + m.m[9]*v.y + m.m[10]*v.z);
}

// ============================================================
// 坐标系统（对照原 MakeCoordinateSystem）
// 返回 u, v, w，对应 localToWorld 的三列
// ============================================================
__host__ __device__ inline void make_coordinate_system(float3 w, float3& u, float3& v) {
    u = make_float3(1.0f, 0.0f, 0.0f);
    if (fabsf(dot(w, u)) > 0.99f) {
        u = make_float3(0.0f, 1.0f, 0.0f);
    }
    v = cross(w, u);
    u = cross(v, w);
    u = normalize(u);
    v = normalize(v);
    // localToWorld 的列：u, v, w
    // worldToLocal = transpose(localToWorld)
}

// ============================================================
// 随机数
// ============================================================
__device__ inline float random01(curandState* state) {
    return curand_uniform(state);
}

__device__ inline float random_range(curandState* state, float a, float b) {
    return a + (b - a) * curand_uniform(state);
}

// ============================================================
// 球坐标（对照原 GetSphericalCoordinate）
// 输入 theta 是极角，phi 是方位角
// ============================================================
__host__ __device__ inline float3 get_spherical_coordinate(float theta, float phi) {
    return make_float3(
        sinf(theta) * cosf(phi),
        sinf(theta) * sinf(phi),
        cosf(theta)
    );
}

// ============================================================
// 折射（对照原 ComputeRefractVector）
// ============================================================
__host__ __device__ inline bool compute_refrac_vector(
    float3 wi, float eta_i, float eta_t, float3& wt) {
    float cos_i = wi.z;
    float sin_i = sqrtf(fmaxf(0.0f, 1.0f - cos_i * cos_i));
    float sin_t = sin_i * eta_i / eta_t;

    // 全反射
    if (sin_t >= 1.0f) {
        return false;
    }

    float cos_t = sqrtf(1.0f - sin_t * sin_t);
    wt.z = wi.z > 0 ? -cos_t : cos_t;
    wt.x = -wi.x * sin_t / fmaxf(1e-3f, sin_i);
    wt.y = -wi.y * sin_t / fmaxf(1e-3f, sin_i);

    wt = normalize(wt);
    return true;
}