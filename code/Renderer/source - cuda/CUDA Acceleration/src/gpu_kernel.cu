#include "gpu_type.h"
#include "gpu_math.cuh"
#include "gpu_scene.h"
#include "gpu_shading.cuh"
#include <cstdio>
#include <curand_kernel.h>

// ============================================================
// 显式栈节点
// ============================================================
struct TraceState {
    RayGPU ray;
    int depth;
    float3 throughput;
};

#define STACK_SIZE 128

// ============================================================
// 相机射线（对照原 Camera::GetRay）
// ============================================================
__device__ inline RayGPU camera_get_ray(const SceneData& scene, float px, float py) {
    RayGPU ray;
    ray.o = scene.cameraPosition;

    // 齐次坐标 (px, py, 0, 1) 乘 invCameraMatrix
    float x = scene.invCameraMatrix.m[0] * px
            + scene.invCameraMatrix.m[1] * py
            + scene.invCameraMatrix.m[3];

    float y = scene.invCameraMatrix.m[4] * px
            + scene.invCameraMatrix.m[5] * py
            + scene.invCameraMatrix.m[7];

    float z = scene.invCameraMatrix.m[8] * px
            + scene.invCameraMatrix.m[9] * py
            + scene.invCameraMatrix.m[11];

    float w = scene.invCameraMatrix.m[12] * px
            + scene.invCameraMatrix.m[13] * py
            + scene.invCameraMatrix.m[15];

    // 除以 w
    if (fabsf(w) > 1e-8f) {
        x /= w;
        y /= w;
        z /= w;
    }

    ray.d = normalize(make_float3(x, y, z) - ray.o);
    ray.mint = 1e-3f;
    ray.maxt = 1e30f;
    return ray;
}

// ============================================================
// 路径追踪（显式栈，严格对照原 Renderer::GetRadiance）
// ============================================================
__device__ float3 trace_ray(const SceneData& scene, RayGPU initialRay, curandState* state) {
    TraceState stack[STACK_SIZE];
    int stackTop = 0;

    // 初始状态入栈
    stack[stackTop].ray = initialRay;
    stack[stackTop].depth = 0;
    stack[stackTop].throughput = make_float3(1, 1, 1);
    stackTop++;

    float3 result = make_float3(0, 0, 0);

    while (stackTop > 0) {
        // 弹栈
        stackTop--;
        TraceState st = stack[stackTop];

        // 深度终止（对照原：depth > mMaxDepth 返回 0）
        if (st.depth > scene.maxDepth) continue;

        // 俄罗斯轮盘赌（对照原）
        const float SurvivalProbability = 0.8f;
        float RewardFactor = 1.0f;
        if (st.depth >= scene.minDepth) {
            float K = random01(state);
            if (K > SurvivalProbability) continue;
            RewardFactor = 1.0f / SurvivalProbability;
        }

        // 求交
        IntersectionGPU isect;
        int hitId = scene_intersect(scene, st.ray, isect);
        if (hitId < 0) continue;

        // 拿到材质
        int matId;
        if (hitId < scene.numSpheres) {
            matId = scene.spheres[hitId].materialId;
        } else if (hitId < scene.numSpheres + scene.numTriangles) {
            matId = scene.triangles[hitId - scene.numSpheres].materialId;
        } else {
            matId = scene.disks[hitId - scene.numSpheres - scene.numTriangles].materialId;
        }
        if (matId < 0 || matId >= scene.numMaterials) continue;
        MaterialData mat = scene.materials[matId];

        // 局部坐标系
        float3 w = isect.normal;
        float3 u, v;
        make_coordinate_system(w, u, v);
        // worldToLocal = transpose(localToWorld)
        // wo = worldToLocal * (-ray.d)
        float3 wo = make_float3(dot(u, -st.ray.d), dot(v, -st.ray.d), dot(w, -st.ray.d));

        // ---- 直接光照（非镜面材质） ----
        float3 directLo = make_float3(0, 0, 0);
        if (!material_is_specular(mat)) {
            for (int li = 0; li < scene.numLights; li++) {
                float3 sourcePos;
                float3 L = light_radiance(scene.lights[li], isect.position, sourcePos);

                RayGPU shadowRay;
                shadowRay.o = isect.position;
                shadowRay.d = normalize(sourcePos - isect.position);
                shadowRay.mint = 1e-3f;
                shadowRay.maxt = length(sourcePos - isect.position);

                IntersectionGPU shadowIsect;
                if (scene_intersect(scene, shadowRay, shadowIsect) >= 0) continue;

                // wi = worldToLocal * shadowRay.d
                float3 wi = make_float3(dot(u, shadowRay.d),
                                        dot(v, shadowRay.d),
                                        dot(w, shadowRay.d));
                float cosTheta = dot(isect.normal, shadowRay.d);
                float3 brdf = material_brdf(mat, wo, wi);
                directLo += brdf * L * fmaxf(cosTheta, 0.0f);
            }
        }

        // 累积直接光照（乘 throughput 和 RewardFactor）
        result += directLo * st.throughput * RewardFactor;

        // ---- 间接光照 ----
        if (material_is_specular(mat)) {
            // 反射
            {
                float3 wi = make_float3(-wo.x, -wo.y, wo.z);
                float3 brdf = material_brdf(mat, wo, wi);
                float3 dir = normalize(u * wi.x + v * wi.y + w * wi.z);

                TraceState newState;
                newState.ray.o = isect.position;
                newState.ray.d = dir;
                newState.ray.mint = 1e-3f;
                newState.ray.maxt = 1e30f;
                newState.depth = st.depth + 1;
                newState.throughput = st.throughput * brdf * fabsf(wi.z) * RewardFactor;

                if (stackTop < STACK_SIZE) {
                    stack[stackTop++] = newState;
                }
            }

            // 折射
            {
                float3 wi;
                if (dielectric_sample_wt(mat, wo, wi)) {
                    float3 btdf = material_btdf(mat, wo, wi);
                    float3 dir = normalize(u * wi.x + v * wi.y + w * wi.z);

                    TraceState newState;
                    newState.ray.o = isect.position;
                    newState.ray.d = dir;
                    newState.ray.mint = 1e-3f;
                    newState.ray.maxt = 1e30f;
                    newState.depth = st.depth + 1;
                    newState.throughput = st.throughput * btdf * fabsf(wi.z) * RewardFactor;

                    if (stackTop < STACK_SIZE) {
                        stack[stackTop++] = newState;
                    }
                }
            }
        } else {
            // 漫反射
            const float theta = random_range(state, 0.0f, PI * 0.5f);
            const float phi = random_range(state, 0.0f, 2.0f * PI);
            float3 wi = get_spherical_coordinate(theta, phi);
            float3 brdf = material_brdf(mat, wo, wi);
            float3 dir = normalize(u * wi.x + v * wi.y + w * wi.z);

            TraceState newState;
            newState.ray.o = isect.position;
            newState.ray.d = dir;
            newState.ray.mint = 1e-3f;
            newState.ray.maxt = 1e30f;
            newState.depth = st.depth + 1;
            newState.throughput = st.throughput * brdf * cosf(theta) * sinf(theta) * PI * PI * RewardFactor;

            if (stackTop < STACK_SIZE) {
                stack[stackTop++] = newState;
            }
        }
    }

    return result;
}

// ============================================================
// 渲染 kernel
// ============================================================
__global__ void render_kernel(SceneData scene, unsigned int* output,
                               int W, int H, int samplesPerPixel,
                               curandState* states) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= W || y >= H) return;

    int pixelIndex = y * W + x;
    curandState state = states[pixelIndex];

    float3 color = make_float3(0, 0, 0);
    for (int s = 0; s < samplesPerPixel; s++) {
        float px = x + random01(&state);
        float py = y + random01(&state);
        RayGPU ray = camera_get_ray(scene, px, py);
        color += trace_ray(scene, ray, &state);
    }
    color = color / (float)samplesPerPixel;

    // 写回
    int r = (int)(fminf(color.x, 1.0f) * 255.0f);
    int g = (int)(fminf(color.y, 1.0f) * 255.0f);
    int b = (int)(fminf(color.z, 1.0f) * 255.0f);
    output[pixelIndex] = (r << 16) | (g << 8) | b;
}

// ============================================================
// 初始化随机数
// ============================================================
__global__ void init_curand(curandState* states, int W, int H, unsigned long seed) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= W || y >= H) return;
    int idx = y * W + x;
    curand_init(seed, idx, 0, &states[idx]);
}