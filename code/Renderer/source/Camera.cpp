#include "Camera.h"

void Camera::Initialize(const Vector3f& p,const Vector3f& target,const Vector3f& up,
        float fov,float n,float f,int W,int H)
{
    mPosition = p;

    //求观察矩阵:
    // left hand 左手坐标系：
    Matrix4x4 viewMatrix = glm::lookAtLH(p,target,up);

    //求投影矩阵:
    Matrix4x4 projectionMatrix = glm::perspectiveFovLH_ZO(fov,(float) W,(float) H,n,f);

    //求视口矩阵:
    Matrix4x4 viewportMatrix = Matrix4x4{
        W / 2.0f,0.0f,0.0f,0.0f,
        0.0f,-H / 2.0f,0.0f,0.0f,
        0.0f,0.0f,1.0f,0.0f,
        W / 2.0f,H / 2.0f,0.0f,1.0f
    };

    Matrix4x4 combinedMatrix = viewportMatrix * projectionMatrix * viewMatrix;
    Matrix4x4 invCombinedMatrix = glm::inverse(combinedMatrix);

    mCombinedMatrix = combinedMatrix;
    mInvCombinedMatrix = invCombinedMatrix;
}

Ray Camera::GetRay(float x,float y) const
{
    Ray ray;
    ray.o = mPosition;

    Vector4f p(x,y,0.0f,1.0f);
    Vector4f worldPos = mInvCombinedMatrix * p;
    worldPos /= worldPos.w;//齐次坐标除以w分量，得到世界坐标
    ray.d = glm::normalize(Vector3f(worldPos) - mPosition);//从相机位置指向世界位置的方向向量

    return ray;
}