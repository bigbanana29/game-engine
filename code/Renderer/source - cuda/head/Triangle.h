#pragma once
#include "Primitive.h"

class Triangle : public Primitive
{
public:
    Triangle(SceneObject* pSceneObject, const Vector3f& v0, const Vector3f& v1, const Vector3f& v2);
    virtual bool Intersect(Ray ray, Intersection& isect) const override;

    // ===== ÐÂÔö =====
    int GetType() const override { return 1; }
    void GetTriangleVertices(Vector3f& v0, Vector3f& v1, Vector3f& v2) const override {
        v0 = mVertices[0];
        v1 = mVertices[1];
        v2 = mVertices[2];
    }
    Vector3f GetTriangleNormal() const override { return mNormal; }

private:
    Vector3f mVertices[3];
    Vector3f mNormal;
};