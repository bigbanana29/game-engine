#pragma once
#include "gpu_type.h"
#include "gpu_math.cuh"

// ============================================================
// 球体求交
// ============================================================
__device__ inline bool intersect_sphere(const SphereData& sphere,
                                        const RayGPU& ray,
                                        IntersectionGPU& isect) {
    // 转到球体局部空间
    RayGPU r;
    r.o = transform_point(sphere.worldToObject, ray.o);
    r.d = transform_vector(sphere.worldToObject, ray.d);
    r.mint = ray.mint;
    r.maxt = ray.maxt;

    float A = dot(r.d, r.d);
    float B = 2.0f * dot(r.d, r.o);
    float C = dot(r.o, r.o) - sphere.radius * sphere.radius;

    float delta = B * B - 4 * A * C;
    if (delta < 0.0f) return false;

    float sqrtDelta = sqrtf(delta);
    float t1 = (-B - sqrtDelta) / (2.0f * A);
    float t2 = (-B + sqrtDelta) / (2.0f * A);

    if (t2 < r.mint || t1 > r.maxt) return false;

    float t = t1;
    if (t < r.mint) {
        t = t2;
        if (t > r.maxt) return false;
    }

    float3 p = r.o + r.d * t;
    float3 n = normalize(p);

    isect.position = transform_point(sphere.objectToWorld, p);
    isect.normal = normalize(transform_vector(sphere.objectToWorld, n));
    isect.t = t;
    return true;
}

// ============================================================
// 三角形求交（M?ller-Trumbore）
// ============================================================
__device__ inline bool intersect_triangle(const TriangleData& tri,
                                          const RayGPU& ray,
                                          IntersectionGPU& isect) {
    float3 p0 = tri.v0;
    float3 e1 = tri.v1 - p0;
    float3 e2 = tri.v2 - p0;
    float3 s = ray.o - p0;

    float3 s1 = cross(ray.d, e2);
    float3 s2 = cross(s, e1);

    float det = dot(s1, e1);
    if (fabsf(det) < 1e-6f) return false;

    float invDet = 1.0f / det;
    float b1 = dot(s1, s) * invDet;
    float b2 = dot(s2, ray.d) * invDet;
    float t = dot(s2, e2) * invDet;

    if (t < ray.mint || t > ray.maxt) return false;

    float b0 = 1.0f - b1 - b2;
    if (b1 < 0.0f || b2 < 0.0f || b0 < 0.0f) return false;

    isect.position = ray.o + ray.d * t;
    isect.normal = tri.normal;
    isect.t = t;
    return true;
}

// ============================================================
// 圆盘求交
// ============================================================
__device__ inline bool intersect_disk(const DiskData& disk,
                                      const RayGPU& ray,
                                      IntersectionGPU& isect) {
    // 转到局部空间
    RayGPU r;
    r.o = transform_point(disk.worldToObject, ray.o);
    r.d = transform_vector(disk.worldToObject, ray.d);
    r.mint = ray.mint;
    r.maxt = ray.maxt;

    // 圆盘在局部空间是 z=0 平面，法线 (0,0,1)
    if (fabsf(r.d.z) < 1e-6f) return false;

    float t = -r.o.z / r.d.z;
    if (t < r.mint || t > r.maxt) return false;

    float3 p = r.o + r.d * t;
    if (p.x * p.x + p.y * p.y > disk.radius * disk.radius) return false;

    float3 n = make_float3(0, 0, 1);

    isect.position = transform_point(disk.objectToWorld, p);
    isect.normal = normalize(transform_vector(disk.objectToWorld, n));
    isect.t = t;
    return true;
}