#pragma once
#include "Ray.h"

class SceneObject;

class Primitive
{
public:
    Primitive(SceneObject* pSceneObject) : m_pSceneObject(pSceneObject) {}
    virtual ~Primitive() {}
    virtual bool Intersect(Ray ray, Intersection& isect) const = 0;

    // ===== ÐÂÔö =====
    virtual int GetType() const = 0;   // 0=Sphere, 1=Triangle, 2=Disk
    virtual float GetRadius() const { return 0.0f; }
    virtual void GetTriangleVertices(Vector3f& v0, Vector3f& v1, Vector3f& v2) const {
        v0 = v1 = v2 = Vector3f(0.0f);
    }
    virtual Vector3f GetTriangleNormal() const { return Vector3f(0.0f); }

protected:
    SceneObject* m_pSceneObject = nullptr;
};