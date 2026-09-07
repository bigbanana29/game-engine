#include "Disk.h"
#include "SceneObject.h"

Disk::Disk(SceneObject* pSceneObject,float radius)
    :Primitive(pSceneObject),mRadius(radius)
{

}

bool Disk::Intersect (Ray ray,Intersection& isect) const
{
    //ray转到圆盘的局部坐标系
    Ray r = m_pSceneObject ->GetWorldToObject() * ray;

    //t
    if(r.d.z < 1e-6f) //射线与圆盘所在平面平行无交点
        return false;

    float t = -r.o.z / r.d.z; // 圆盘所在平面z=0

    if(t < r.mint || t > r.maxt) //交点不在有效范围内
        return false;

    Vector3f p = r.o + r.d * t;//交点位置(局部空间)

    if(glm::dot(p,p) > mRadius * mRadius)
        return false;

    isect.position = Vector3f(m_pSceneObject ->GetObjectToWorld() * Vector4f(p,1.0f));//交点位置(世界空间)
    isect.normal = glm::normalize(Vector3f(m_pSceneObject ->GetObjectToWorld() * Vector4f(0,0,1,0)));//圆盘的法线(世界位置)
    isect.t = t;

    return true;
}