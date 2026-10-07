#pragma once
#include "Primitive.h"

class Sphere : public Primitive
{
public:
    Sphere(SceneObject* pSceneObject, float R);
    virtual bool Intersect(Ray ray, Intersection& isect) const override;

    // ===== ĞÂÔö =====
    int GetType() const override { return 0; }
    float GetRadius() const override { return mRadius; }

private:
    float mRadius;
};