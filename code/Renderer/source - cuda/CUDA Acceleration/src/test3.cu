#include "../head/gpu_type.h"
#include "../head/gpu_math.cuh"
#include "../head/gpu_shading.cuh"
#include <cstdio>

__global__ void test_kernel(MaterialData mat, LightData light, float* out) {
    // ≤‚ ‘ Lambert BRDF
    float3 brdf = material_brdf(mat, make_float3(0, 0, 1), make_float3(0, 0, 1));
    out[0] = brdf.x;

    // ≤‚ ‘µ„π‚‘¥
    float3 sourcePos;
    float3 L = light_radiance(light, make_float3(0, 0, 0), sourcePos);
    out[1] = L.x;
    out[2] = sourcePos.x;
}

int main() {
    MaterialData mat;
    mat.type = MAT_LAMBERT;
    mat.albedo = make_float3(0.8f, 0.2f, 0.2f);

    LightData light;
    light.type = LIGHT_POINT;
    light.position = make_float3(0, 5, 0);
    light.intensity = make_float3(1, 1, 1);
    light.attenuation = make_float3(1, 0, 0);   // A=1, B=0, C=0

    float* d_out;
    cudaMalloc(&d_out, 3 * sizeof(float));

    test_kernel<<<1, 1>>>(mat, light, d_out);
    cudaDeviceSynchronize();
    cudaError_t err = cudaGetLastError();
    printf("err: %s\n", cudaGetErrorString(err));

    float h_out[3];
    cudaMemcpy(h_out, d_out, 3 * sizeof(float), cudaMemcpyDeviceToHost);
    printf("brdf.x = %f\n", h_out[0]);
    printf("L.x = %f\n", h_out[1]);
    printf("sourcePos.x = %f\n", h_out[2]);

    cudaFree(d_out);
    return 0;
}