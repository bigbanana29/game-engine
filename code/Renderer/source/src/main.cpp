#include "Renderer.h"

int main() {
    Renderer renderer(1920, 1080, 100, "Renderer/source/Scenexml/Scene03.xml");
    renderer.Run();

    return 0;
}