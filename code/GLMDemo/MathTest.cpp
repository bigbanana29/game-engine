#include<iostream>
#include<glm/glm.hpp>
#include<glm/gtc/constants.hpp>

using Vector2f = glm::vec2;
using Vector3f = glm::vec3;
using Vector4f = glm::vec4;

using Vector2i = glm::ivec2;
using Vector3i = glm::ivec3;
using Vector4i = glm::ivec4;

using Matrix3x3 = glm::mat3;
using Matrix4x4 = glm::mat4;

const float PI = glm::pi<float>();

inline void DumpVector (const Vector3f& v)
{
    std::cout << "Vector3f(" << v.x << "," << v.y << "," << v.z << ")" << std::endl;
}

//构造一个平移矩阵
inline Matrix4x4 MakeTranslation(const Vector3f& t)
{
    return Matrix4x4{
        1.0f,0.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,0.0f,
        0.0f,0.0f,1.0f,0.0f,
        t.x,t.y,t.z,1.0f
    };
}

//构造一个旋转矩阵
inline Matrix4x4 MakeRotation(const Vector3f& eular){
    //先绕x轴旋转，再绕y轴旋转，最后绕z轴旋转
    float cx = cosf(eular.x);
    float sx = sinf(eular.x);
    float cy = cosf(eular.y);
    float sy = sinf(eular.y);
    float cz = cosf(eular.z);
    float sz = sinf(eular.z);

    Matrix4x4 rx={
        1.0f,0.0f,0.0f,0.0f,
        0.0f,cx,sx,0.0f,
        0.0f,-sx,cx,0.0f,
        0.0f,0.0f,0.0f,1.0f
    };
    Matrix4x4 ry={
        cy,0.0f,-sy,0.0f,
        0.0f,1.0f,0.0f,0.0f,
        sy,0.0f,cy,0.0f,
        0.0f,0.0f,0.0f,1.0f
    };
    Matrix4x4 rz={
        cz,sz,0.0f,0.0f,
        -sz,cz,0.0f,0.0f,
        0.0f,0.0f,1.0f,0.0f,
        0.0f,0.0f,0.0f,1.0f
    };
    return rz*ry*rx;
}

inline Matrix4x4 MakeScale(float s){
    return Matrix4x4{
        s,0.0f,0.0f,0.0f,
        0.0f,s,0.0f,0.0f,
        0.0f,0.0f,s,0.0f,
        0.0f,0.0f,0.0f,1.0f
    };
}

inline Matrix4x4 MakeWorldTransform(const Vector3f& position,const Vector3f& rotation,float s)
{
    Matrix4x4 T = MakeTranslation(position);
    Matrix4x4 R = MakeRotation(rotation);
    Matrix4x4 S = MakeScale(s);
    return T*R*S;
}

//这里返回的三维矩阵为A坐标系的三个垂直基向量
//同时也可直接作为从A坐标系到世界坐标系的变换矩阵
//从世界坐标系到A坐标系的变换矩阵就是这个矩阵的逆矩阵
inline Matrix3x3 MakeCoordinateSystem(const Vector3f w)
{
    //获得 u v
    Vector3f u = (1.0F,0.0F,0.0F);

    if(glm::dot(u,w) > 0.99f)
    {
        u = Vector3f(0.0f,1.0f,0.0f);
    }

    Vector3f v = glm::cross(u,w);
    u = glm::cross(v,w);

    u = glm::normalize(u);
    v = glm::normalize(v);

    return Matrix3x3(u,v,w)
}


int main(){
    //向量的基础运算 点乘 叉乘 归一化 取模等
    {
        Vector3f v1(1.0f,2.0f,3.0f);
        Vector3f v2(4.0f,5.0f,6.0f);
        
        Vector3f sum = v1 + v2;//向量加法
        Vector3f diff = v1 - v2;//向量减法
        
        float x = glm::dot(v1,v2);//点乘
        Vector3f cross = glm::cross(v1,v2);//叉乘

        float length = glm::length(v1);//向量长度
        Vector3f normalized = glm::normalize(v1);//向量归一化

    }

    //矩阵
    {
        //构造一个矩阵：
        //4x4矩阵：
        //1 2 3 4
        //5 6 7 8
        //9 10 11 12
        //13 14 15 16

        glm::mat4 m{
            1.0f,5.0f,9.0f,13.0f,//第一列
            2.0f,6.0f,10.0f,14.0f,//第二列
            3.0f,7.0f,11.0f,15.0f,//第三列
            4.0f,8.0f,12.0f,16.0f//第四列
        };

        //矩阵转置 求逆运算：
        glm::mat4 m1 = glm::transpose(m);
        glm::mat4 m2 = glm::inverse(m);
    }

    //平移变换
    {
        Vector3f p(1.0f,2.0f,3.0f);
        Vector3f t(4.0f,5.0f,6.0f);
        Vector3f p1 = p+t;//平移变换

        //用矩阵实现平移变换：
        Matrix4x4 T = MakeTranslation(t);
        Vector3f p2 = T * Vector4f(p,1.0f);//齐次坐标乘以平移矩阵

        DumpVector(p1);
        DumpVector(p2);
    }

    //旋转变换：
    {
        Matrix4x4 R= MakeRotation(Vector3f(glm::radians(30.0f),glm::radians(45.0f),glm::radians(60.0f)));

        Vector3f v = R * Vector4f(1,2,3,0);
        DumpVector(v);
    }

    //缩放变换：
    {
        Matrix4x4 S = MakeScale(2.0f);
        Vector3f v = S * Vector4f(1,2,3,1);
        DumpVector(v);
    }
}