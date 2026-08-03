#include "Scene.h"

SceneObject* Scene::CreateSceneObject()
{
    SceneObject* pSceneObject = new SceneObject();
    mSceneObjects.push_back(pSceneObject);
    return pSceneObject;
}

Scene::~Scene()
{
    for (SceneObject* pSceneObject : mSceneObjects)
        delete pSceneObject;
}