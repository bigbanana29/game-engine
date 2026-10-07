#include "../head/gpu_type.h"
#include "../head/gpu_math.cuh"
#include "../head/gpu_intersect.cuh"
#include <cstdio>

__global__ void test_kernel(SphereData sphere, int* out) {
    RayGPU ray;
    ray.o = make_float3(0, 0, 0);
    ray.d = make_float3(0, 0, -1);
    ray.mint = 1e-3f;
    ray.maxt = 1e30f;

    IntersectionGPU isect;
    out[0] = intersect_sphere(sphere, ray, isect) ? 1 : 0;
}

int main() {
    SphereData sphere;
    sphere.center = make_float3(0, 0, -5);
    sphere.radius = 1.0f;
    sphere.worldToObject = make_identity();
    sphere.objectToWorld = make_identity();
    sphere.materialId = 0;

    int* d_out;
    cudaMalloc(&d_out, sizeof(int));

    test_kernel<<<1, 1>>>(sphere, d_out);
    cudaDeviceSynchronize();
    cudaError_t err = cudaGetLastError();
    printf("err: %s\n", cudaGetErrorString(err));

    int h_out;
    cudaMemcpy(&h_out, d_out, sizeof(int), cudaMemcpyDeviceToHost);
    printf("hit: %d\n", h_out);

    cudaFree(d_out);
    return 0;
}