#include "Scene.h"

// ------------------------------------------------------------
// 原有功能：创建场景对象
// ------------------------------------------------------------
SceneObject* Scene::CreateSceneObject(const Vector3f& position,
                                      const Vector3f& euler,
                                      float scale)
{
    SceneObject* pSceneObject = new SceneObject(position, euler, scale);
    mSceneObjects.push_back(pSceneObject);
    return pSceneObject;
}

// ------------------------------------------------------------
// 原有功能：析构函数
// ------------------------------------------------------------
Scene::~Scene()
{
    for (SceneObject* pSceneObject : mSceneObjects)
        delete pSceneObject;

    for (Light* pLight : mLights)
        delete pLight;

    for (auto& pair : mMaterials)
        delete pair.second;
}

// ------------------------------------------------------------
// 原有功能：场景求交
// ------------------------------------------------------------
SceneObject* Scene::Intersect(Ray ray, Intersection& isect) const
{
    SceneObject* pHitObject = nullptr;
    for (const auto pSceneObject : mSceneObjects)
    {
        if (pSceneObject->Intersect(ray, isect))
        {
            ray.maxt = isect.t;
            pHitObject = pSceneObject;
        }
    }
    return pHitObject;
}