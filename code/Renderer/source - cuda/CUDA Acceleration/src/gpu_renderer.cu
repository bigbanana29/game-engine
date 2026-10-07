#include "gpu_renderer.h"
#include "gpu_math.cuh"
#include <cstdio>
#include <cstdlib>
#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <vector>
#include <map>
#include <MiniFB.h>

// 原 CPU 版头文件
#include "Scene.h"
#include "SceneObject.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Disk.h"
#include "Material.h"
#include "Light.h"
#include "Camera.h"

// kernel 声明
__global__ void render_kernel(SceneData scene, unsigned int* output,
                               int W, int H, int samplesPerPixel,
                               curandState* states);
__global__ void init_curand(curandState* states, int W, int H, unsigned long seed);

// ============================================================
// glm → GPU 类型
// ============================================================
static float3 glmToFloat3(const glm::vec3& v) {
    return make_float3(v.x, v.y, v.z);
}

static float4x4 glmToFloat4x4(const glm::mat4& m) {
    float4x4 r;
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            r.m[row * 4 + col] = m[col][row];
    return r;
}

// ============================================================
// 构造 / 析构
// ============================================================
GPURenderer::GPURenderer(int w, int h, int minDepth, int maxDepth, int spp,
                          const char* filepath)
    : mW(w), mH(h), mMinDepth(minDepth), mMaxDepth(maxDepth), mSPP(spp) {

    cudaMalloc(&mDBuffer, w * h * sizeof(unsigned int));
    cudaMalloc(&mDStates, w * h * sizeof(curandState));
    mHBuffer = (uint32_t*)calloc(w * h, 4);

    dim3 block(16, 16);
    dim3 grid((w + 15) / 16, (h + 15) / 16);
    init_curand<<<grid, block>>>(mDStates, w, h, 42);
    cudaDeviceSynchronize();

    mScene = Scene::LoadSceneFromXML(filepath, w, h);
    if (!mScene) {
        printf("Scene loading failed\n");
        exit(1);
    }
}

GPURenderer::~GPURenderer() {
    cudaFree(mDBuffer);
    cudaFree(mDStates);
    if (mDSpheres) cudaFree(mDSpheres);
    if (mDTriangles) cudaFree(mDTriangles);
    if (mDDisks) cudaFree(mDDisks);
    if (mDMaterials) cudaFree(mDMaterials);
    if (mDLights) cudaFree(mDLights);
    free(mHBuffer);
    if (mScene) delete mScene;
}

// ============================================================
// 扁平化
// ============================================================
void GPURenderer::FlattenScene() {
    // ---- 1. 材质 ----
    std::vector<MaterialData> h_materials;
    std::map<Material*, int> matIdMap;

    for (auto& kv : mScene->GetMaterials()) {
        Material* mat = kv.second;
        if (matIdMap.find(mat) != matIdMap.end()) continue;

        MaterialData md;
        md.type = -1;
        md.albedo = make_float3(0, 0, 0);
        md.eta = make_float3(0, 0, 0);
        md.absorptionCoef = make_float3(0, 0, 0);
        md.reflectionColor = make_float3(0, 0, 0);
        md.ior = 1.0f;
        md.transmissionColor = make_float3(0, 0, 0);

        if (auto* lam = dynamic_cast<LambertMaterial*>(mat)) {
            md.type = MAT_LAMBERT;
            md.albedo = glmToFloat3(lam->GetAlbedo());
        }
        else if (auto* cond = dynamic_cast<ConductorSpecularMaterial*>(mat)) {
            md.type = MAT_CONDUCTOR;
            md.eta = glmToFloat3(cond->GetEta());
            md.absorptionCoef = glmToFloat3(cond->GetAbsorptionCoef());
            md.reflectionColor = glmToFloat3(cond->GetReflectionColor());
        }
        else if (auto* diel = dynamic_cast<DielectricSpecularMaterial*>(mat)) {
            md.type = MAT_DIELECTRIC;
            md.ior = diel->GetEta();
            md.transmissionColor = glmToFloat3(diel->GetTransmissionColor());
        }

        matIdMap[mat] = (int)h_materials.size();
        h_materials.push_back(md);
    }

    // ---- 2. 图元 ----
    std::vector<SphereData> h_spheres;
    std::vector<TriangleData> h_triangles;
    std::vector<DiskData> h_disks;

    printf("SceneObjects: %d\n", (int)mScene->GetSceneObjects().size());

    for (SceneObject* obj : mScene->GetSceneObjects()) {
        // ===== 诊断 =====
        printf("Object: primitives=%d, material=%p\n",
            (int)obj->GetPrimitives().size(),
            (void*)obj->GetMaterial());
        // ================

        int matId = -1;
        if (obj->GetMaterial() && matIdMap.find(obj->GetMaterial()) != matIdMap.end())
            matId = matIdMap[obj->GetMaterial()];

        float4x4 objToWorld = glmToFloat4x4(obj->GetObjectToWorld());
        float4x4 worldToObj = glmToFloat4x4(obj->GetWorldToObject());

        for (Primitive* prim : obj->GetPrimitives()) {
            if (prim->GetType() == 0) {
                SphereData sd;
                sd.center = make_float3(0, 0, 0);
                sd.radius = prim->GetRadius();
                sd.objectToWorld = objToWorld;
                sd.worldToObject = worldToObj;
                sd.materialId = matId;
                h_spheres.push_back(sd);
            }
            else if (prim->GetType() == 1) {
                Vector3f v0, v1, v2;
                prim->GetTriangleVertices(v0, v1, v2);
                TriangleData td;
                td.v0 = glmToFloat3(v0);
                td.v1 = glmToFloat3(v1);
                td.v2 = glmToFloat3(v2);
                td.normal = glmToFloat3(prim->GetTriangleNormal());
                td.materialId = matId;
                h_triangles.push_back(td);
            }
            else if (prim->GetType() == 2) {
                DiskData dd;
                dd.center = make_float3(0, 0, 0);
                dd.normal = make_float3(0, 0, 1);
                dd.radius = prim->GetRadius();
                dd.objectToWorld = objToWorld;
                dd.worldToObject = worldToObj;
                dd.materialId = matId;
                h_disks.push_back(dd);
            }
        }
    }

    // ---- 3. 光源 ----
    std::vector<LightData> h_lights;
    for (Light* light : mScene->GetLights()) {
        LightData ld;
        ld.type = -1;
        ld.position = make_float3(0, 0, 0);
        ld.direction = make_float3(0, 0, 0);
        ld.intensity = make_float3(0, 0, 0);
        ld.attenuation = make_float3(0, 0, 0);
        ld.cosInnerAngle = 0;
        ld.cosOuterAngle = 0;

        if (auto* dir = dynamic_cast<DirectionalLight*>(light)) {
            ld.type = LIGHT_DIRECTIONAL;
            ld.direction = glmToFloat3(dir->GetDirection());
            ld.intensity = glmToFloat3(dir->GetRadianceColor());
        }
        else if (auto* pt = dynamic_cast<PointLight*>(light)) {
            ld.type = LIGHT_POINT;
            ld.position = glmToFloat3(pt->GetPosition());
            ld.intensity = glmToFloat3(pt->GetIntensity());
            ld.attenuation = glmToFloat3(pt->GetAttenuations());
        }
        else if (auto* spot = dynamic_cast<SpotLight*>(light)) {
            ld.type = LIGHT_SPOT;
            ld.position = glmToFloat3(spot->GetPosition());
            ld.direction = glmToFloat3(spot->GetDirection());
            ld.intensity = glmToFloat3(spot->GetIntensity());
            ld.cosInnerAngle = spot->GetCosInnerAngle();
            ld.cosOuterAngle = spot->GetCosOuterAngle();
            ld.attenuation = glmToFloat3(spot->GetAttenuations());
        }

        if (ld.type >= 0) h_lights.push_back(ld);
    }

    // ---- 4. 上传到显存 ----
    mNumSpheres = (int)h_spheres.size();
    mNumTriangles = (int)h_triangles.size();
    mNumDisks = (int)h_disks.size();
    mNumMaterials = (int)h_materials.size();
    mNumLights = (int)h_lights.size();

    if (mNumSpheres > 0) {
        cudaMalloc(&mDSpheres, mNumSpheres * sizeof(SphereData));
        cudaMemcpy(mDSpheres, h_spheres.data(), mNumSpheres * sizeof(SphereData),
                   cudaMemcpyHostToDevice);
    }
    if (mNumTriangles > 0) {
        cudaMalloc(&mDTriangles, mNumTriangles * sizeof(TriangleData));
        cudaMemcpy(mDTriangles, h_triangles.data(), mNumTriangles * sizeof(TriangleData),
                   cudaMemcpyHostToDevice);
    }
    if (mNumDisks > 0) {
        cudaMalloc(&mDDisks, mNumDisks * sizeof(DiskData));
        cudaMemcpy(mDDisks, h_disks.data(), mNumDisks * sizeof(DiskData),
                   cudaMemcpyHostToDevice);
    }
    if (mNumMaterials > 0) {
        cudaMalloc(&mDMaterials, mNumMaterials * sizeof(MaterialData));
        cudaMemcpy(mDMaterials, h_materials.data(), mNumMaterials * sizeof(MaterialData),
                   cudaMemcpyHostToDevice);
    }
    if (mNumLights > 0) {
        cudaMalloc(&mDLights, mNumLights * sizeof(LightData));
        cudaMemcpy(mDLights, h_lights.data(), mNumLights * sizeof(LightData),
                   cudaMemcpyHostToDevice);
    }

    // ---- 5. 组装 SceneData ----
    mSceneData.spheres = mDSpheres;
    mSceneData.numSpheres = mNumSpheres;
    mSceneData.triangles = mDTriangles;
    mSceneData.numTriangles = mNumTriangles;
    mSceneData.disks = mDDisks;
    mSceneData.numDisks = mNumDisks;
    mSceneData.materials = mDMaterials;
    mSceneData.numMaterials = mNumMaterials;
    mSceneData.lights = mDLights;
    mSceneData.numLights = mNumLights;

    // ---- 6. 相机 ----
    const Camera& cam = mScene->GetCamera();
    mSceneData.cameraPosition = glmToFloat3(cam.GetPosition());
    mSceneData.invCameraMatrix = glmToFloat4x4(cam.GetInvCombinedMatrix());
    mSceneData.viewportWidth = mW;
    mSceneData.viewportHeight = mH;
    mSceneData.minDepth = mMinDepth;
    mSceneData.maxDepth = mMaxDepth;
}

void GPURenderer::Run() {
    FlattenScene();

    // ===== 诊断输出 =====
    printf("========== Scene Stats ==========\n");
    printf("Spheres:   %d\n", mNumSpheres);
    printf("Triangles: %d\n", mNumTriangles);
    printf("Disks:     %d\n", mNumDisks);
    printf("Materials: %d\n", mNumMaterials);
    printf("Lights:    %d\n", mNumLights);
    printf("Camera pos: (%f, %f, %f)\n",
           mSceneData.cameraPosition.x,
           mSceneData.cameraPosition.y,
           mSceneData.cameraPosition.z);
    printf("invCameraMatrix:\n");
    for (int i = 0; i < 16; i++) {
        printf("%10.4f ", mSceneData.invCameraMatrix.m[i]);
        if (i % 4 == 3) printf("\n");
    }
    printf("==================================\n");
    // ====================

    dim3 block(16, 16);
    dim3 grid((mW + 15) / 16, (mH + 15) / 16);

    render_kernel<<<grid, block>>>(mSceneData, mDBuffer, mW, mH, mSPP, mDStates);
    cudaDeviceSynchronize();

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("kernel err: %s\n", cudaGetErrorString(err));
        return;
    }

    cudaMemcpy(mHBuffer, mDBuffer, mW * mH * sizeof(unsigned int),
               cudaMemcpyDeviceToHost);

    FILE* f = fopen("output.ppm", "wb");
    fprintf(f, "P6\n%d %d\n255\n", mW, mH);
    for (int i = 0; i < mW * mH; i++) {
        unsigned char rgb[3] = {
            (unsigned char)(mHBuffer[i] >> 16),
            (unsigned char)(mHBuffer[i] >> 8),
            (unsigned char)(mHBuffer[i])
        };
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
    printf("Rendered to output.ppm\n");
}

void GPURenderer::RunRealtime() {
    FlattenScene();

    // 创建 MiniFB 窗口
    struct mfb_window* window = mfb_open_ex("CUDA Renderer", mW, mH, MFB_WF_RESIZABLE);
    if (!window) {
        printf("Failed to open window\n");
        return;
    }

    dim3 block(16, 16);
    dim3 grid((mW + 15) / 16, (mH + 15) / 16);

    // 计时
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    mfb_update_state state;
    do {
        // CUDA 渲染
        cudaEventRecord(start);
        render_kernel<<<grid, block>>>(mSceneData, mDBuffer, mW, mH, mSPP, mDStates);
        cudaEventRecord(stop);
        cudaEventSynchronize(stop);

        //float ms = 0;
        //cudaEventElapsedTime(&ms, start, stop);
        //printf("GPU render time: %.2f ms\n", ms);

        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess) {
            printf("kernel err: %s\n", cudaGetErrorString(err));
            break;
        }

        // 拷回主机
        cudaMemcpy(mHBuffer, mDBuffer, mW * mH * sizeof(unsigned int),
                   cudaMemcpyDeviceToHost);

        // MiniFB 显示
        state = mfb_update_ex(window, mHBuffer, mW, mH);
        if (state != MFB_STATE_OK) break;

    } while (mfb_wait_sync(window));

    cudaEventDestroy(start);
    cudaEventDestroy(stop);
}