#define _CRT_SECURE_NO_WARNINGS

#include "Scene.h"
#include "Disk.h"
#include "Sphere.h"
#include "Triangle.h"
#include "tinyxml2.h"
#include "Material.h"
#include <cstdio>
#include <iostream>
#include <string>

//解析“x，y，z”格式的字符串为Vector3f
static Vector3f ParseVector3f(const char* str)
{
    Vector3f v(0.0f);
    if (!str) return v;

    if (std::sscanf(str,"%f , %f , %f",&v.x,&v.y,&v.z) != 3)
    {
        //解析失敗，可選擇拋出異常或返回默認值
        //throw std::runtime_error("Failed to parse Vector3f from string");
        v = Vector3f(0.0f);
    }
    return v;
}

//解析"x,y"格式的字符串为Vector2f
static Vector2f ParseVector2f(const char* str)
{
    Vector2f v(0.0f);
    if (!str) return v;

    if (std::sscanf(str,"%f , %f",&v.x,&v.y) != 2)
    {
        //解析失敗，可選擇拋出異常或返回默認值
        //throw std::runtime_error("Failed to parse Vector2f from string");
        v = Vector2f(0.0f);
    }
    return v;
}

static float GetChildFloat(tinyxml2::XMLElement* parent, const char* childName, float defaultVal)
{
    tinyxml2::XMLElement* child = parent->FirstChildElement(childName);
    if (child && child->GetText())
    {
        return std::stof(child->GetText());
    }
    return defaultVal;
}

static const char* GetChildText(tinyxml2::XMLElement* parent, const char* childName)
{
    tinyxml2::XMLElement* child = parent->FirstChildElement(childName);
    if (child)
    {
        return child->GetText();
    }
    return nullptr;
}

Scene* Scene::LoadSceneFromXML(const char* filepath, int W, int H)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filepath) != tinyxml2::XML_SUCCESS)
    {
        std::cerr << "Failed to load XML file: " << filepath << std::endl;
        return nullptr;
    }

    tinyxml2::XMLElement* pRoot = doc.FirstChildElement("Scene");
    if (!pRoot)
    {
        std::cerr << "No <Scene> root element found in XML file: " << filepath << std::endl;
        return nullptr;
    }

    Scene* pScene = new Scene();

    //解析摄像机
    tinyxml2::XMLElement* pCameraElement = pRoot->FirstChildElement("Camera");
    if (pCameraElement)
    {
        Vector3f position = ParseVector3f(GetChildText(pCameraElement, "Position"));
        Vector3f target = ParseVector3f(GetChildText(pCameraElement, "Target"));
        Vector3f up = ParseVector3f(GetChildText(pCameraElement, "Up"));
        float nearZ = GetChildFloat(pCameraElement, "NearZ", 0.1f);
        float farZ = GetChildFloat(pCameraElement, "FarZ", 1000.0f);
        float fovDeg = GetChildFloat(pCameraElement, "FOV", 45.0f);
        float fovRad = glm::radians(fovDeg);

        Camera camera;
        camera.Initialize(position, target, up, fovRad, nearZ, farZ, W, H);
        pScene->SetCamera(camera);
    }

    // 解析 Materials
    tinyxml2::XMLElement* pMaterialsElem = pRoot->FirstChildElement("Materials");
    if (pMaterialsElem)
    {
        for (tinyxml2::XMLElement* pElem = pMaterialsElem->FirstChildElement("Material");
            pElem != nullptr; pElem = pElem->NextSiblingElement("Material"))
        {
            const char* nameText = GetChildText(pElem, "Name");
            const char* typeText = GetChildText(pElem, "Type");
            if (!nameText || !typeText) {
                std::cerr << "Invalid Material element in XML file: " << filepath << std::endl;
                continue;
            }

            std::string name = nameText;
            std::string type = typeText;
            if (type == "Lambert")
            {
                Color albedo = ParseVector3f(GetChildText(pElem, "Albedo"));
                pScene->CreateMaterial<LambertMaterial>(name, albedo);
            }
        }
    }

    //解析SceneObjects
    tinyxml2::XMLElement* pSceneObjectsElement = pRoot->FirstChildElement("SceneObjects");
    if (pSceneObjectsElement)
    {
        for (tinyxml2::XMLElement* pObjElem = pSceneObjectsElement->FirstChildElement("SceneObject");
             pObjElem != nullptr; pObjElem = pObjElem->NextSiblingElement("SceneObject"))
        {
            SceneObject* pSceneObject = nullptr;

            //解析Transform
            tinyxml2::XMLElement* pTransformElem = pObjElem->FirstChildElement("Transform");
            if (pTransformElem)
            {
                Vector3f position = ParseVector3f(GetChildText(pTransformElem, "Position"));
                Vector3f rotation = ParseVector3f(GetChildText(pTransformElem, "Rotation"));
                float scale = GetChildFloat(pTransformElem, "Scale", 1.0f);

                //rotation從度轉換為弧度
                rotation = glm::radians(rotation);
                pSceneObject = pScene->CreateSceneObject(position, rotation, scale);
            }

            if (!pSceneObject)
            {
                std::cerr << "Failed to create SceneObject from XML." << std::endl;
                continue;
            }

            //解析Material引用
            const char* materialName = GetChildText(pObjElem, "Material");
            if (materialName)
            {
                Material* pMaterial = pScene->GetMaterial(materialName);
                if (pMaterial)
                {
                    pSceneObject->SetMaterial(pMaterial);
                }
                else
                {
                    std::cerr << "Material not found: " << materialName << std::endl;
                }
            }

            //解析Primitives
            tinyxml2::XMLElement* pPrimitivesElem = pObjElem->FirstChildElement("Primitives");
            if (pPrimitivesElem)
            {
                //Sphere
                for (tinyxml2::XMLElement* pElem = pPrimitivesElem->FirstChildElement("Sphere");
                     pElem != nullptr; pElem = pElem->NextSiblingElement("Sphere"))
                {
                    float radius = GetChildFloat(pElem, "Radius", 1.0f);
                    pSceneObject->CreatePrimitive<Sphere>(radius);
                }

                //Disk
                for (tinyxml2::XMLElement* pElem = pPrimitivesElem->FirstChildElement("Disk");
                     pElem != nullptr; pElem = pElem->NextSiblingElement("Disk"))
                {
                    float radius = GetChildFloat(pElem, "Radius", 1.0f);
                    pSceneObject->CreatePrimitive<Disk>(radius);
                }

                //Triangle
                for (tinyxml2::XMLElement* pElem = pPrimitivesElem->FirstChildElement("Triangle");
                     pElem != nullptr; pElem = pElem->NextSiblingElement("Triangle"))
                {
                    Vector3f v0 = ParseVector3f(GetChildText(pElem, "V0"));
                    Vector3f v1 = ParseVector3f(GetChildText(pElem, "V1"));
                    Vector3f v2 = ParseVector3f(GetChildText(pElem, "V2"));
                    pSceneObject->CreatePrimitive<Triangle>(v0, v1, v2);
                }
            }
        }
    }

    //解析Lights
    tinyxml2::XMLElement* pLightsElement = pRoot->FirstChildElement("Lights");
    if (pLightsElement)
    {
        //DirectionalLight
        for (tinyxml2::XMLElement* pElem = pLightsElement->FirstChildElement("DirectionalLight");
             pElem != nullptr; pElem = pElem->NextSiblingElement("DirectionalLight"))
        {
            Vector3f direction = ParseVector3f(GetChildText(pElem, "Direction"));
            Vector3f radiance = ParseVector3f(GetChildText(pElem, "Radiance"));
            pScene->CreateLight<DirectionalLight>(direction, radiance);
        }

        //PointLight
        for (tinyxml2::XMLElement* pElem = pLightsElement->FirstChildElement("PointLight");
             pElem != nullptr; pElem = pElem->NextSiblingElement("PointLight"))
        {
            Vector3f position = ParseVector3f(GetChildText(pElem, "Position"));
            Color intensity = ParseVector3f(GetChildText(pElem, "Intensity"));
            Vector3f attenuations = ParseVector3f(GetChildText(pElem, "Attenuations"));
            pScene->CreateLight<PointLight>(position, intensity, attenuations);
        }

        //SpotLight
        for (tinyxml2::XMLElement* pElem = pLightsElement->FirstChildElement("SpotLight");
             pElem != nullptr; pElem = pElem->NextSiblingElement("SpotLight"))
        {
            Vector3f position = ParseVector3f(GetChildText(pElem, "Position"));
            Vector3f direction = ParseVector3f(GetChildText(pElem, "Direction"));
            Color intensity = ParseVector3f(GetChildText(pElem, "Intensity"));
            float innerAngle = GetChildFloat(pElem, "InnerAngle", 15.0f);
            float outerAngle = GetChildFloat(pElem, "OuterAngle", 30.0f);
            Vector3f attenuations = ParseVector3f(GetChildText(pElem, "Attenuations"));

            //将角度从度转换为弧度
            innerAngle = glm::radians(innerAngle);
            outerAngle = glm::radians(outerAngle);

            pScene->CreateLight<SpotLight>(position, direction, intensity, innerAngle, outerAngle, attenuations);
        }
    }

    return pScene;
}