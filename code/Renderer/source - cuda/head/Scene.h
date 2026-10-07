#pragma once
#include "SceneObject.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include <map>
#include <string>
#include <vector>

class Scene
{
public:
    static Scene* LoadSceneFromXML(const char* filepath, int W, int H);
    ~Scene();

    void SetCamera(const Camera& camera) { mCamera = camera; }
    const Camera& GetCamera() const { return mCamera; }

    SceneObject* Intersect(Ray ray, Intersection& isect) const;
    SceneObject* CreateSceneObject(const Vector3f& position, const Vector3f& euler, float scale);

    template<typename T, typename... Args>
    T* CreateLight(Args&&... args)
    {
        T* light = new T(std::forward<Args>(args)...);
        mLights.push_back(light);
        return light;
    }

    template<typename T, typename... Args>
    T* CreateMaterial(const std::string& name, Args&&... args)
    {
        T* material = new T(std::forward<Args>(args)...);
        mMaterials.insert(std::make_pair(name, material));
        return material;
    }

    Material* GetMaterial(const std::string& name) const {
        auto it = mMaterials.find(name);
        if (it != mMaterials.end())
            return it->second;
        return nullptr;
    }

    std::vector<Light*> GetLights() const { return mLights; }

    // ===== ÐÂÔö Getter =====
    const std::vector<SceneObject*>& GetSceneObjects() const { return mSceneObjects; }
    const std::map<std::string, Material*>& GetMaterials() const { return mMaterials; }

private:
    Camera mCamera;
    std::vector<SceneObject*> mSceneObjects;
    std::vector<Light*> mLights;
    std::map<std::string, Material*> mMaterials;
};