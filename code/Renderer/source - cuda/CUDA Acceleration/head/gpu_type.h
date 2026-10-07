#pragma once
#include <cuda_runtime.h>

// ============================================================
// 基础数学类型（替代 glm）
// ============================================================
// float3 用 CUDA 自带的，不要重定义
struct float4x4 {
    float m[16];   // 行优先：m[row * 4 + col]
};

// ============================================================
// 图元类型
// ============================================================
enum PrimitiveType {
    PRIM_SPHERE = 0,
    PRIM_TRIANGLE = 1,
    PRIM_DISK = 2
};

// ============================================================
// 材质类型
// ============================================================
enum MaterialType {
    MAT_LAMBERT = 0,
    MAT_CONDUCTOR = 1,
    MAT_DIELECTRIC = 2
};

// ============================================================
// 光源类型
// ============================================================
enum LightType {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT = 1,
    LIGHT_SPOT = 2
};

// ============================================================
// 球体数据
// ============================================================
struct SphereData {
    float3 center;
    float radius;
    float4x4 worldToObject;
    float4x4 objectToWorld;
    int materialId;
};

// ============================================================
// 三角形数据
// ============================================================
struct TriangleData {
    float3 v0, v1, v2;
    float3 normal;
    int materialId;
};

// ============================================================
// 圆盘数据
// ============================================================
struct DiskData {
    float3 center;
    float3 normal;
    float radius;
    float4x4 worldToObject;
    float4x4 objectToWorld;
    int materialId;
};

// ============================================================
// 材质数据
// ============================================================
struct MaterialData {
    int type;
    float3 albedo;              // Lambert
    float3 eta;                 // Conductor
    float3 absorptionCoef;      // Conductor
    float3 reflectionColor;     // Conductor
    float ior;                  // Dielectric
    float3 transmissionColor;   // Dielectric
};

// ============================================================
// 光源数据
// ============================================================
struct LightData {
    int type;
    float3 position;
    float3 direction;
    float3 intensity;
    float3 attenuation;
    float cosInnerAngle;
    float cosOuterAngle;
};

// ============================================================
// 场景数据（全部扁平化，POD）
// ============================================================
struct SceneData {
    SphereData* spheres;
    int numSpheres;
    TriangleData* triangles;
    int numTriangles;
    DiskData* disks;
    int numDisks;
    MaterialData* materials;
    int numMaterials;
    LightData* lights;
    int numLights;

    // 相机
    float3 cameraPosition;
    float4x4 cameraMatrix;
    float4x4 invCameraMatrix;
    int viewportWidth;
    int viewportHeight;
    int minDepth;
    int maxDepth;
};

// ============================================================
// 光线、交点
// ============================================================
struct RayGPU {
    float3 o;
    float3 d;
    float mint;
    float maxt;
};

struct IntersectionGPU {
    float3 position;
    float3 normal;
    float t;
};