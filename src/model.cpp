#include "model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <algorithm>

bool loadModel(const std::string& path, std::vector<float>& outVertices, std::vector<unsigned int>& outIndices) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals|aiProcess_CalcTangentSpace);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "Assimp 错误: " << importer.GetErrorString() << std::endl;
        return false;
    }
    
    if (scene->mNumMeshes == 0) {
        std::cout << "模型没有网格数据" << std::endl;
        return false;
    }
    
    // 打印模型结构信息
    std::cout << "模型结构信息:" << std::endl;
    std::cout << "  总网格数: " << scene->mNumMeshes << std::endl;
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* meshPtr = scene->mMeshes[m];
        std::cout << "  网格 " << m << ": 顶点数=" << meshPtr->mNumVertices << ", 面数=" << meshPtr->mNumFaces << std::endl;
        std::cout << "    是否有纹理坐标: " << (meshPtr->HasTextureCoords(0) ? "是" : "否") << std::endl;
        
        // ========== 新增：打印材质名称和贴图引用 ==========
        unsigned int matIndex = meshPtr->mMaterialIndex;
        aiMaterial* material = scene->mMaterials[matIndex];
        aiString matName;
        material->Get(AI_MATKEY_NAME, matName);
        std::cout << "    材质索引: " << matIndex << ", 材质名称: " << matName.C_Str() << std::endl;
        
        // 打印贴图引用
        aiString texPath;
        // 颜色贴图 (漫反射)
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            std::cout << "      颜色贴图: " << texPath.C_Str() << std::endl;
        }
        // 法线贴图
        if (material->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS) {
            std::cout << "      法线贴图: " << texPath.C_Str() << std::endl;
        }
        // 粗糙度贴图 (有些模型用 aiTextureType_SHININESS 或 aiTextureType_DIFFUSE_ROUGHNESS)
        if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texPath) == AI_SUCCESS) {
            std::cout << "      粗糙度贴图: " << texPath.C_Str() << std::endl;
        }
        // 金属度贴图
        if (material->GetTexture(aiTextureType_METALNESS, 0, &texPath) == AI_SUCCESS) {
            std::cout << "      金属度贴图: " << texPath.C_Str() << std::endl;
        }
        // AO贴图 (环境光遮蔽)
        if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texPath) == AI_SUCCESS) {
            std::cout << "      AO贴图: " << texPath.C_Str() << std::endl;
        }
        // 自发光贴图
        if (material->GetTexture(aiTextureType_EMISSIVE, 0, &texPath) == AI_SUCCESS) {
            std::cout << "      自发光贴图: " << texPath.C_Str() << std::endl;
        }
        // 透明度贴图
        if (material->GetTexture(aiTextureType_OPACITY, 0, &texPath) == AI_SUCCESS) {
            std::cout << "      透明度贴图: " << texPath.C_Str() << std::endl;
        }
        // 高度贴图
        if (material->GetTexture(aiTextureType_HEIGHT, 0, &texPath) == AI_SUCCESS) {
            std::cout << "      高度贴图: " << texPath.C_Str() << std::endl;
        }
        // ========== END ==========
    }
    
    // 计算总包围盒
    float minX=0, maxX=0, minY=0, maxY=0, minZ=0, maxZ=0;
    bool first = true;
    
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* meshPtr = scene->mMeshes[m];
        for (unsigned int i = 0; i < meshPtr->mNumVertices; i++) {
            float x = meshPtr->mVertices[i].x;
            float y = meshPtr->mVertices[i].y;
            float z = meshPtr->mVertices[i].z;
            if (first) {
                minX = maxX = x;
                minY = maxY = y;
                minZ = maxZ = z;
                first = false;
            } else {
                if (x < minX) minX = x; if (x > maxX) maxX = x;
                if (y < minY) minY = y; if (y > maxY) maxY = y;
                if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
            }
        }
    }
    
    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;
    float centerZ = (minZ + maxZ) / 2.0f;
    float maxRange = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    float scale = 1.0f / maxRange;
    
    std::cout << "包围盒: X[" << minX << ", " << maxX << "], Y[" << minY << ", " << maxY << "], Z[" << minZ << ", " << maxZ << "]" << std::endl;
    std::cout << "缩放因子: " << scale << std::endl;
    
    // 提取顶点和索引
    outVertices.clear();
    outIndices.clear();
    unsigned int vertexOffset = 0;
    
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* meshPtr = scene->mMeshes[m];
        
        // ========== 只有一个循环！ ==========
        for (unsigned int i = 0; i < meshPtr->mNumVertices; i++) {
            // 1. 位置 (3)
            outVertices.push_back((meshPtr->mVertices[i].x - centerX) * scale);
            outVertices.push_back((meshPtr->mVertices[i].y - centerY) * scale);
            outVertices.push_back((meshPtr->mVertices[i].z - centerZ) * scale);
            
            // 2. 法线 (3)
            if (meshPtr->HasNormals()) {
                outVertices.push_back(meshPtr->mNormals[i].x);
                outVertices.push_back(meshPtr->mNormals[i].y);
                outVertices.push_back(meshPtr->mNormals[i].z);
            } else {
                outVertices.push_back(0.0f);
                outVertices.push_back(0.0f);
                outVertices.push_back(1.0f);
            }
            
            // 3. UV (2)
            if (meshPtr->HasTextureCoords(0)) {
                outVertices.push_back(meshPtr->mTextureCoords[0][i].x);
                outVertices.push_back(meshPtr->mTextureCoords[0][i].y);
            } else {
                outVertices.push_back(0.0f);
                outVertices.push_back(0.0f);
            }

            // 4. 切线 (3) ← 新增
            if (meshPtr->HasTangentsAndBitangents()) {
                outVertices.push_back(meshPtr->mTangents[i].x);
                outVertices.push_back(meshPtr->mTangents[i].y);
                outVertices.push_back(meshPtr->mTangents[i].z);
            } else {
                outVertices.push_back(1.0f);
                outVertices.push_back(0.0f);
                outVertices.push_back(0.0f);
    }
        }
        // ========== END ==========
        
        for (unsigned int i = 0; i < meshPtr->mNumFaces; i++) {
            aiFace face = meshPtr->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                outIndices.push_back(face.mIndices[j] + vertexOffset);
            }
        }
        
        vertexOffset += meshPtr->mNumVertices;
    }
    
    std::cout << "合并后顶点数: " << outVertices.size() / 11 << ", 索引数: " << outIndices.size() << std::endl;
    std::cout << "模型加载成功！" << std::endl;
    return true;
}