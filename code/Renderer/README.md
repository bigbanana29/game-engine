# CUDA Path Tracer

GPU accelerated real-time path tracer, migrated from a CPU renderer.

## Features

- Real-time rendering with MiniFB
- Path tracing with Russian roulette
- Glass material (Fresnel + refraction)
- Reflection and refraction via explicit stack (CUDA doesn't support recursion)
- Scene loading from XML
- CPU and GPU versions for comparison

## Build

### Requirements

- Windows 10/11
- NVIDIA GPU with CUDA support
- CUDA Toolkit 13.1
- Visual Studio 2019/2022 (or Build Tools)
- MiniFB (included)

### GPU Version (CUDA)

\`\`\`bash
cd "source - cuda/CUDA Acceleration/src"
nvcc main_gpu.cpp gpu_renderer.cu gpu_kernel.cu ^
  ../../src/Scene.cpp ../../src/SceneLoader.cpp ^
  ../../src/SceneObject.cpp ../../src/Sphere.cpp ^
  ../../src/Triangle.cpp ../../src/Disk.cpp ^
  ../../src/Material.cpp ../../src/Light.cpp ^
  ../../src/Camera.cpp ../../src/Common.cpp ^
  ../../src/Primitive.cpp ../../src/tinyxml2.cpp ^
  -o renderer_gpu.exe ^
  -I"../head" -I"../../head" -I"../../.." ^
  -I"../../../minifb/include" ^
  -L"../../../../build/minifb/Release" -lminifb ^
  user32.lib gdi32.lib opengl32.lib winmm.lib ^
  -Xcompiler /MD -Xcompiler /wd4819 ^
  -allow-unsupported-compiler --expt-relaxed-constexpr
\`\`\`

### CPU Version

Build with CMake or Visual Studio. See `source/` directory.

## Performance

| Version | Resolution | SPP | Time |
|---------|-----------|-----|------|
| CPU | 800x600 | 100 | ? |
| GPU | 800x600 | 100 | ? |
| Speedup | | | ? |

## Structure

\`\`\`text
Renderer/
©À©¤©¤ source/                    # Original CPU renderer
©¦   ©À©¤©¤ head/
©¦   ©¸©¤©¤ src/
©À©¤©¤ source - cuda/             # CUDA accelerated version
©¦   ©¸©¤©¤ CUDA Acceleration/
©¦       ©À©¤©¤ head/              # GPU data structures
©¦       ©¸©¤©¤ src/               # CUDA kernels
©À©¤©¤ minifb/                    # MiniFB window library
©¸©¤©¤ Scenexml/                  # Scene files
\`\`\`

## How It Works

1. **Scene loading**: Reuse the original CPU `SceneLoader` to parse XML
2. **Flattening**: Convert C++ objects (with pointers, virtual functions, STL) into POD arrays
3. **Upload**: `cudaMemcpy` scene data to GPU memory
4. **Render**: CUDA kernel processes each pixel in parallel
5. **Display**: Copy result back to host, show with MiniFB

## Technical Highlights

- **Explicit stack** replaces recursion (CUDA doesn't support `__device__` recursion)
- **Virtual functions ¡ú switch** (GPU doesn't support virtual dispatch)
- **Pointers ¡ú indices** (CPU and GPU have different address spaces)
- **glm ¡ú float3/float4x4** (avoid CUDA/glm compatibility issues)

## License

MIT