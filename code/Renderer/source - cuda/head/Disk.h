#pragma once
#include "Primitive.h"
#include "SceneObject.h"

class Disk : public Primitive
{
public:
    Disk(SceneObject* pSceneObject, float radius);
    virtual bool Intersect(Ray ray, Intersection& isect) const override;

    // ===== ÐÂÔö =====
    int GetType() const override { return 2; }
    float GetRadius() const override { return mRadius; }

private:
    float mRadius;
    Matrix4x4 mObjectToWorld;
    Matrix4x4 mWorldToObject;
};