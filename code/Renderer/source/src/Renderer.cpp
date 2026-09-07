#include "Renderer.h"
#include <MiniFB.h>
#include "Common.h"
#include "Material.h"
#include <thread>
#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <vector>

Renderer::Renderer(int w,int h,int samplePerPixel,const char* filepath)
    :mViewportWidth(w),
     mViewportHeight(h),
     SamplePerPixel(samplePerPixel)
{
    mCurrentPixelIndex = 0;
    mScene = Scene::LoadSceneFromXML(filepath, w, h);
    if (!mScene) {
        std::cerr << "Scene loading failed, exiting." << std::endl;
        exit(EXIT_FAILURE);
    }

    //auto pMaterial = mScene->CreateMaterial<LambertMaterial>("RedLambert", Color(1.0f,0.0f,0.0f));
    //auto pSceneObject1 = mScene->CreateSceneObject(Vector3f(0,0,5),Vector3f(0,0,0),2.0f);
    //auto pSceneObject2 = mScene->CreateSceneObject(Vector3f(0,0,2),Vector3f(0,0,0),1.0f);
    
    //pSceneObject1->SetMaterial(pMaterial);
    //pSceneObject2->SetMaterial(pMaterial);
    //mScene = new Scene();

    //设置摄像机
    //Camera camera;
    //camera.Initialize(
    //    Vector3f(0.0f,0.0f,0.0f),//相机位置
    //    Vector3f(0.0f,0.0f,1.0f),//目标位置
    //    Vector3f(0.0f,1.0f,0.0f),//上位置
    //    glm::radians(60.0f),//pov
    //    0.1f,//近裁剪面
    //    1000.0f,//远裁剪面
    //    w,h//视口宽高
    //);

    //mScene -> SetCamera(camera);

    //给场景添加物体
    //SceneObject* pSceneObject = mScene ->CreateSceneObject(Vector3f(0,0,5),Vector3f(0,0,0),2.0f);
    //pSceneObject->CreatePrimitive<Triangle>(Vector3f(-1, -1, 0), Vector3f(1, -1, 0), Vector3f(1, 1, 0));
    //pSceneObject->CreatePrimitive<Triangle>(Vector3f(-1, -1, 0), Vector3f(1, 1, 0), Vector3f(-1, 1, 0));

    //SceneObject* pSceneObject2 = mScene ->CreateSceneObject(Vector3f(0,0,2),Vector3f(0,0,0),1.0f);
    //pSceneObject2 -> CreatePrimitive<Sphere>(0.5f);

    //auto pSphere = new Sphere(Vector3f(-1.0f,-1.0f,10),1.0f);
    //auto pDisk = new Disk(Vector3f(0,-2.0f,5),Vector3f(glm::radians(0.0f),0,0),1.0f);
    //auto pTriangle = new Triangle(Vector3f(-1,0,0),Vector3f(0,1,0),Vector3f(1,0,0),
    //    MakeWorldTransform(Vector3f(0,0,5),Vector3f(0,glm::radians(45.0f),glm::radians(60.0f)),2.0f));

    //mPrimitives.push_back(pSphere);
    //mPrimitives.push_back(pDisk);
    //mPrimitives.push_back(pTriangle);

    // 添加一个矩形：

    //mTestSceneObject = new SceneObject(Vector3f(0,0,5),Vector3f(0,0,0),2.0f);

    //auto pTriangle1 = new Triangle(mTestSceneObject,Vector3f(-1, -1, 0), Vector3f(1, -1, 0), Vector3f(1, 1, 0));

    //auto pTriangle2 = new Triangle(mTestSceneObject,Vector3f(-1, -1, 0), Vector3f(1, 1, 0), Vector3f(-1, 1, 0));

    //mTestSceneObject -> AddPrimitive(pTriangle1);
    //mTestSceneObject -> AddPrimitive(pTriangle2);
}

Renderer::~Renderer()
{
    if(mScene)
    {
        delete mScene;
    }
}

void Renderer::Run() {
    struct mfb_window *window = mfb_open_ex("my display", mViewportWidth, mViewportHeight, MFB_WF_RESIZABLE);
    if (window == NULL)
        return;

    // 视口上每个像素点的颜色，以32位整数表示，格式为0xAARRGGBB(Alpha，Red，Green，Blue)
    mBuffer = reinterpret_cast<uint32_t*>(calloc(mViewportWidth * mViewportHeight, 4));

    std::thread renderThread(&Renderer::RunRenderThread,this);
    renderThread.detach();

    int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> renderThreads(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        renderThreads[i] = std::thread(&Renderer::RunRenderThread,this);
        renderThreads[i].detach();
    }
    
    // present方法:
    mfb_update_state state;
    do {
        state = mfb_update_ex(window, mBuffer, mViewportWidth, mViewportHeight);
        if (state != MFB_STATE_OK)
            break;
    } while (mfb_wait_sync(window));

    free(mBuffer);
    mBuffer = NULL;
    window = NULL;
}

Color Renderer::RenderPixel(int x,int y)
{
    //SSAA
    static const int N = 10;
    Color resultColor(0,0,0);

    for(int i = 0;i < N;i++)
    {
        //(x,y) - (x+1,y+1)范围内随机采样一个点
        float px = x + glm::linearRand(0.0f,1.0f);
        float py = y + glm::linearRand(0.0f,1.0f);

        Color color = RenderSubPixel(px,py);
        resultColor += (color / (float)N);
    }
    return resultColor; //取平均值，得到最终颜色
}

Color Renderer::RenderSubPixel(float x,float y)
{
    Ray ray = mScene->GetCamera().GetRay(x,y);
    Color color = GetRadiance(ray);
    return color;

    //bool bHit = false;
    //for (const auto& primitive : mPrimitives)
    //{
    //    if(primitive ->Intersect(ray,isect))
    //    {
    //        ray.maxt = isect.t;
    //        bHit = true;
    //    }
    //}

    //if(bHit)
    //{
    //    color = isect.normal * 0.5f + 0.5f;//将法线向量映射到[0，1]范围内，作为颜色输出
    //}

    //return color;
}

Color Renderer::GetIrradiance(const Ray& ray)
{
    Intersection isect;
    if(!mScene->Intersect(ray, isect))
    {
        return Color(0,0,0);
    }

    Color E(0,0,0); //累加光照的辐射度
    //E(p)
    for(Light* pLight : mScene->GetLights())
    {
        Vector3f sourcePos;
        Color L = pLight->GetRadiance(isect.position, sourcePos);

        
        //求shadowRay
        Ray shadowRay;
        shadowRay.o = isect.position;
        shadowRay.d = glm::normalize(sourcePos - isect.position);
        shadowRay.mint = 0.001f; //避免自相交
        shadowRay.maxt = glm::length(sourcePos - isect.position);

        Intersection shadow_isect;
        if(mScene->Intersect(shadowRay, shadow_isect))
        {
            continue; //被遮挡，跳过该光源
        }

        float cosTheta = glm::dot(isect.normal, shadowRay.d);

        E += L * glm::max(cosTheta, 0.0f);
    }
    return E;
}

Color Renderer::GetRadiance(const Ray& ray)
{
    Intersection isect;
    SceneObject* pSceneObject = mScene->Intersect(ray, isect);
    if(pSceneObject == nullptr)
    {
        return Color(0,0,0);
    }

    Material* pMaterial = pSceneObject->GetMaterial();
    Color Lo(0,0,0); //累加光照的辐射度

    Matrix3x3 localToWorld = MakeCoordinateSystem(isect.normal);
    Matrix3x3 worldToLocal = glm::inverse(localToWorld);

    Vector3f wo = worldToLocal * (-ray.d); //出射方向，轉換到局部坐标系

    for(Light* pLight : mScene->GetLights())
    {
        Vector3f sourcePos;
        Color L = pLight->GetRadiance(isect.position, sourcePos);
        
        //求shadowRay
        Ray shadowRay;
        shadowRay.o = isect.position;
        shadowRay.d = glm::normalize(sourcePos - isect.position);
        shadowRay.mint = 0.001f; //避免自相交
        shadowRay.maxt = glm::length(sourcePos - isect.position);

        Intersection shadow_isect;
        if(mScene->Intersect(shadowRay, shadow_isect))
        {
            continue; //被遮挡，跳过该光源
        }

        Vector3f wi = worldToLocal * shadowRay.d;//入射方向，轉換到局部坐标系
        float cosTheta = glm::dot(isect.normal, shadowRay.d);
        Color brdf = pMaterial->BRDF(wo,wi);
        Lo += brdf * L * glm::max(cosTheta, 0.0f);
    }
    return Lo;
}

//渲染线程的入口函数，负责执行渲染循环
void Renderer::RunRenderThread()
{
    //读取当前屏幕的下一个像素
    while(true)
    {
        int pixelIndex = mCurrentPixelIndex.fetch_add(1);
        if(pixelIndex >= mViewportWidth * mViewportHeight)
            break;

        int x = pixelIndex % mViewportWidth;
        int y = pixelIndex / mViewportWidth;

        Color color = RenderPixel(x,y);
        uint32_t r = glm::clamp((uint32_t)std::round(color.r * 255.0f),0u,255u);
        uint32_t g = glm::clamp((uint32_t)std::round(color.g * 255.0f),0u,255u);
        uint32_t b = glm::clamp((uint32_t)std::round(color.b * 255.0f),0u,255u);
        mBuffer[y * mViewportWidth + x] = (r << 16) | (g << 8) | b;
    }
}