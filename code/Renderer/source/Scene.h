#pragma once

#include "SceneObject.h"
#include "Camera.h"

class Scene
{
public:
    void SetCamera(const Camera& camera) { mCamera = camera; }
    const Camera& GetCamera() const { return mCamera; }

    SceneObject* CreateSceneObject();
    ~Scene();

private:
    Camera mCamera;
    std::vector<SceneObject*> mSceneObjects;
};