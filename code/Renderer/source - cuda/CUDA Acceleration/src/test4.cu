#include "../head/gpu_type.h"
#include "../head/gpu_math.cuh"
#include "../head/gpu_scene.h"
#include <cstdio>

__global__ void test_kernel(SceneData scene, int* out) {
    RayGPU ray;
    ray.o = make_float3(0, 0, -5);
    ray.d = make_float3(0, 0, 1);
    ray.mint = 1e-3f;
    ray.maxt = 1e30f;

    IntersectionGPU isect;
    out[0] = scene_intersect(scene, ray, isect);
    out[1] = (int)(isect.t * 100);
}

int main() {
    // 球心在原点，半径 1
    SphereData sphere;
    sphere.center = make_float3(0, 0, 0);
    sphere.radius = 1.0f;
    sphere.worldToObject = make_identity();
    sphere.objectToWorld = make_identity();
    sphere.materialId = 0;

    // 上传到 GPU
    SphereData* d_spheres;
    cudaMalloc(&d_spheres, sizeof(SphereData));
    cudaMemcpy(d_spheres, &sphere, sizeof(SphereData), cudaMemcpyHostToDevice);

    SceneData scene;
    scene.spheres = d_spheres;
    scene.numSpheres = 1;
    scene.triangles = nullptr;
    scene.numTriangles = 0;
    scene.disks = nullptr;
    scene.numDisks = 0;
    scene.materials = nullptr;
    scene.numMaterials = 0;
    scene.lights = nullptr;
    scene.numLights = 0;

    int* d_out;
    cudaMalloc(&d_out, 2 * sizeof(int));

    test_kernel<<<1, 1>>>(scene, d_out);
    cudaDeviceSynchronize();
    cudaError_t err = cudaGetLastError();
    printf("err: %s\n", cudaGetErrorString(err));

    int h_out[2];
    cudaMemcpy(h_out, d_out, 2 * sizeof(int), cudaMemcpyDeviceToHost);
    printf("hitId: %d, t*100: %d\n", h_out[0], h_out[1]);

    cudaFree(d_spheres);
    cudaFree(d_out);
    return 0;
}