#pragma once
#include "Common.h"
#include <atomic>
#include "Camera.h"
#include "Sphere.h"
#include "Disk.h"
#include "Triangle.h"
#include "SceneObject.h"
#include <vector>

class Renderer {
public:
    Renderer(int w,int h,int SamplePerPixel);
    virtual ~Renderer();
    void Run();

private:
    Color RenderPixel(int x,int y);
    Color Renderer::RenderSubPixel(float x,float y);
    void RunRenderThread();

    int mViewportWidth = 800;
    int mViewportHeight = 600;
    //屏幕上每个像素的采样次数(SPP)
    int SamplePerPixel = 100;

    uint32_t* mBuffer = nullptr;

    std::atomic<int> mCurrentPixelIndex = 0;

    Camera mCamera;

    //std::vector<Primitive*> mPrimitives;//场景中的所有集合体

    SceneObject* mTestSceneObject = nullptr;
};