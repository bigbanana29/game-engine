#include "gpu_renderer.h"

int main() {
    GPURenderer renderer(1920, 1080, 1, 5, 200,
        "C:\\Users\\bigbanana666\\Desktop\\game engine\\code\\Renderer\\source - cuda\\Scenexml\\Scene03.xml");
    renderer.RunRealtime();   // ← 改成实时渲染
    return 0;
}