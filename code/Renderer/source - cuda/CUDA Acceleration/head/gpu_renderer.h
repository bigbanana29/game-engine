#pragma once
#include "gpu_type.h"
#include <cstdint>
#include <curand_kernel.h>

class Scene;

class GPURenderer {
public:
    GPURenderer(int w, int h, int minDepth, int maxDepth, int spp,
                const char* filepath);
    ~GPURenderer();

    void Run();
    void RunRealtime();   // 新：MiniFB 实时显示
    void FlattenScene();

private:
    int mW, mH;
    int mMinDepth, mMaxDepth, mSPP;
    SceneData mSceneData;

    Scene* mScene = nullptr;

    SphereData* mDSpheres = nullptr;
    TriangleData* mDTriangles = nullptr;
    DiskData* mDDisks = nullptr;
    MaterialData* mDMaterials = nullptr;
    LightData* mDLights = nullptr;
    unsigned int* mDBuffer = nullptr;
    curandState* mDStates = nullptr;

    uint32_t* mHBuffer = nullptr;

    int mNumSpheres = 0;
    int mNumTriangles = 0;
    int mNumDisks = 0;
    int mNumMaterials = 0;
    int mNumLights = 0;
};