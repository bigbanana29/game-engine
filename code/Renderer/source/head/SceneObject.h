#pragma once
#include "Ray.h"
#include "Primitive.h"
#include <vector>

class Material; //材质类

class SceneObject
{
public:
    SceneObject(const Vector3f& position, const Vector3f& euler, float scale)
    {
        mObjectToWorld = MakeWorldTransform(position, euler, scale);
        mWorldToObject = glm::inverse(mObjectToWorld);
    };

    bool Intersect(Ray ray, Intersection& isect) const;
    void AddPrimitive(Primitive* primitive) { mPrimitives.push_back(primitive); }

    template<typename T, typename... Args>
    T* CreatePrimitive(Args&&... args)
    {
        T* primitive = new T(this, std::forward<Args>(args)...);
        mPrimitives.push_back(primitive);
        return primitive;
    }

    virtual ~SceneObject();

    Matrix4x4 GetObjectToWorld() const { return mObjectToWorld; } 
    Matrix4x4 GetWorldToObject() const { return mWorldToObject; } 

    void SetMaterial(Material* material) { m_Material = material; } 
    Material* GetMaterial() const { return m_Material; }

private:
    Matrix4x4 mObjectToWorld;
    Matrix4x4 mWorldToObject;

    Material* m_Material = nullptr;//場景中所有物体的材质

    std::vector<Primitive*> mPrimitives;
};