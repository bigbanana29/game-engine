#pragma once
#include "gpu_type.h"
#include "gpu_intersect.cuh"

__device__ inline int scene_intersect(const SceneData& scene,
                                       const RayGPU& ray,
                                       IntersectionGPU& isect) {
    int hitId = -1;
    float closest = ray.maxt;

    for (int i = 0; i < scene.numSpheres; i++) {
        RayGPU r = ray;
        r.maxt = closest;
        IntersectionGPU is;
        if (intersect_sphere(scene.spheres[i], r, is)) {
            closest = is.t;
            isect = is;
            hitId = i;
        }
    }

    for (int i = 0; i < scene.numTriangles; i++) {
        RayGPU r = ray;
        r.maxt = closest;
        IntersectionGPU is;
        if (intersect_triangle(scene.triangles[i], r, is)) {
            closest = is.t;
            isect = is;
            hitId = scene.numSpheres + i;
        }
    }

    for (int i = 0; i < scene.numDisks; i++) {
        RayGPU r = ray;
        r.maxt = closest;
        IntersectionGPU is;
        if (intersect_disk(scene.disks[i], r, is)) {
            closest = is.t;
            isect = is;
            hitId = scene.numSpheres + scene.numTriangles + i;
        }
    }

    return hitId;
}