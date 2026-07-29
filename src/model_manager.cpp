#include "model_manager.h"
#include "model.h"
#include "texture.h"
#include <iostream>
#include <vector>
#include <windows.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <algorithm>

// ========== 辅助函数：获取文件扩展名 ==========
std::string getFileExtension(const std::string& path) {
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        return path.substr(dotPos);
    }
    return "";
}

// ========== 辅助函数：获取文件名（不含路径） ==========
std::string getFileName(const std::string& path) {
    size_t slashPos = path.find_last_of("/\\");
    if (slashPos != std::string::npos) {
        return path.substr(slashPos + 1);
    }
    return path;
}

// ========== 辅助函数：获取目录路径 ==========
std::string getDirectoryPath(const std::string& path) {
    size_t slashPos = path.find_last_of("/\\");
    if (slashPos != std::string::npos) {
        return path.substr(0, slashPos);
    }
    return ".";
}

// ========== 遍历目录，查找纹理文件 ==========
std::vector<std::string> findFilesInDirectory(const std::string& dirPath, const std::vector<std::string>& extensions) {
    std::vector<std::string> result;
    
    std::string searchPath = dirPath + "\\*.*";
    
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return result;
    }
    
    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        
        std::string filename = findData.cFileName;
        std::string ext = getFileExtension(filename);
        
        for (const auto& supportedExt : extensions) {
            std::string extLower = ext;
            std::string supportedLower = supportedExt;
            for (char& c : extLower) c = tolower(c);
            for (char& c : supportedLower) c = tolower(c);
            
            if (extLower == supportedLower) {
                result.push_back(dirPath + "\\" + filename);
                break;
            }
        }
        
    } while (FindNextFileA(hFind, &findData));
    
    FindClose(hFind);
    return result;
}

// ---------- 修改：增加 .dds 支持 ----------
std::vector<std::string> getAllImagesInModelFolder(const std::string& modelPath) {
    std::string modelDir = getDirectoryPath(modelPath);
    std::vector<std::string> imageExts = {".jpg", ".jpeg", ".png", ".tga", ".bmp", ".tif", ".tiff", ".dds"};  // 添加 .dds
    
    std::vector<std::string> files = findFilesInDirectory(modelDir, imageExts);
    
    std::string textureDir = modelDir + "\\textures";
    std::vector<std::string> texFiles = findFilesInDirectory(textureDir, imageExts);
    files.insert(files.end(), texFiles.begin(), texFiles.end());
    
    return files;
}

// ========== 查找颜色贴图 ==========
std::string findTextureFile(const std::string& modelPath) {
    std::string modelDir = getDirectoryPath(modelPath);
    
    std::vector<std::string> colorKeywords = {
        "diffuse", "basecolor", "color", "albedo",
        "col", "bump", "specular", "roughness",
        "ao", "metallic", "height"
    };
    
    std::vector<std::string> imageExts = {".jpg", ".jpeg", ".png", ".tga", ".bmp", ".tif", ".tiff", ".dds"};
    
    std::string textureDir = modelDir + "\\textures";
    std::vector<std::string> files = findFilesInDirectory(textureDir, imageExts);
    
    for (const auto& keyword : colorKeywords) {
        for (const auto& file : files) {
            std::string filename = getFileName(file);
            std::string filenameLower = filename;
            for (char& c : filenameLower) c = tolower(c);
            if (filenameLower.find(keyword) != std::string::npos) {
                return file;
            }
        }
    }
    
    files = findFilesInDirectory(modelDir, imageExts);
    for (const auto& keyword : colorKeywords) {
        for (const auto& file : files) {
            std::string filename = getFileName(file);
            std::string filenameLower = filename;
            for (char& c : filenameLower) c = tolower(c);
            if (filenameLower.find(keyword) != std::string::npos) {
                return file;
            }
        }
    }
    
    return "";
}

// 后面的 findNormalMap、findEmissiveMap、findOpacityMap、findHeightMap 等函数，建议也在各自的 imageExts 中加入 ".dds"，这里为节省篇幅不再展开，但你可以参照上面的方式添加。
// 下面给出一个例子（法线贴图），其他类似修改即可。
std::string findNormalMap(const std::string& modelPath) {
    std::string modelDir = getDirectoryPath(modelPath);
    
    std::vector<std::string> normalKeywords = {"normal", "nor", "bump", "normalmap"};
    std::vector<std::string> imageExts = {".jpg", ".jpeg", ".png", ".tga", ".bmp", ".dds"}; // 添加 .dds
    
    std::string textureDir = modelDir + "\\textures";
    std::vector<std::string> files = findFilesInDirectory(textureDir, imageExts);
    
    for (const auto& keyword : normalKeywords) {
        for (const auto& file : files) {
            std::string filename = getFileName(file);
            std::string filenameLower = filename;
            for (char& c : filenameLower) c = tolower(c);
            if (filenameLower.find(keyword) != std::string::npos) {
                return file;
            }
        }
    }
    
    files = findFilesInDirectory(modelDir, imageExts);
    for (const auto& keyword : normalKeywords) {
        for (const auto& file : files) {
            std::string filename = getFileName(file);
            std::string filenameLower = filename;
            for (char& c : filenameLower) c = tolower(c);
            if (filenameLower.find(keyword) != std::string::npos) {
                return file;
            }
        }
    }
    
    return "";
}

// ========== 查找自发光贴图 ==========
std::string findEmissiveMap(const std::string& modelPath) {
    std::string modelDir = getDirectoryPath(modelPath);
    
    std::vector<std::string> emissiveKeywords = {"emissive", "emission", "e", "glow"};
    std::vector<std::string> imageExts = {".jpg", ".jpeg", ".png", ".tga", ".bmp", ".dds"};
    
    std::string textureDir = modelDir + "\\textures";
    std::vector<std::string> files = findFilesInDirectory(textureDir, imageExts);
    
    for (const auto& keyword : emissiveKeywords) {
        for (const auto& file : files) {
            std::string filename = getFileName(file);
            std::string filenameLower = filename;
            for (char& c : filenameLower) c = tolower(c);
            if (filenameLower.find(keyword) != std::string::npos) {
                return file;
            }
        }
    }
    
    files = findFilesInDirectory(modelDir, imageExts);
    for (const auto& keyword : emissiveKeywords) {
        for (const auto& file : files) {
            std::string filename = getFileName(file);
            std::string filenameLower = filename;
            for (char& c : filenameLower) c = tolower(c);
            if (filenameLower.find(keyword) != std::string::npos) {
                return file;
            }
        }
    }
    
    return "";
}

// ========== 查找透明度贴图 ==========
std::string findOpacityMap(const std::string& modelPath) {
    std::string modelDir = getDirectoryPath(modelPath);
    
    std::vector<std::string> opacityKeywords = {"opacity", "alpha", "mask", "transparency"};
    std::vector<std::string> imageExts = {".jpg", ".jpeg", ".png", ".tga", ".bmp", ".dds"};
    
    std::string textureDir = modelDir + "\\textures";
    std::vector<std::string> files = findFilesInDirectory(textureDir, imageExts);
    
    for (const auto& keyword : opacityKeywords) {
        for (const auto& file : files) {
            std::string filename = getFileName(file);
            std::string filenameLower = filename;
            for (char& c : filenameLower) c = tolower(c);
            if (filenameLower.find(keyword) != std::string::npos) {
                return file;
            }
        }
    }
    
    files = findFilesInDirectory(modelDir, imageExts);
    for (const auto& keyword : opacityKeywords) {
        for (const auto& file : files) {
            std::string filename = getFileName(file);
            std::string filenameLower = filename;
            for (char& c : filenameLower) c = tolower(c);
            if (filenameLower.find(keyword) != std::string::npos) {
                return file;
            }
        }
    }
    
    return "";
}

// ========== 查找高度贴图 ==========
std::string findHeightMap(const std::string& modelPath) {
    std::string modelDir = getDirectoryPath(modelPath);
    
    std::vector<std::string> heightKeywords = {"height", "displacement", "h", "bump"};
    std::vector<std::string> imageExts = {".jpg", ".jpeg", ".png", ".tga", ".bmp", ".dds"};
    
    std::string textureDir = modelDir + "\\textures";
    std::vector<std::string> files = findFilesInDirectory(textureDir, imageExts);
    
    for (const auto& keyword : heightKeywords) {
        for (const auto& file : files) {
            std::string filename = getFileName(file);
            std::string filenameLower = filename;
            for (char& c : filenameLower) c = tolower(c);
            if (filenameLower.find(keyword) != std::string::npos) {
                return file;
            }
        }
    }
    
    files = findFilesInDirectory(modelDir, imageExts);
    for (const auto& keyword : heightKeywords) {
        for (const auto& file : files) {
            std::string filename = getFileName(file);
            std::string filenameLower = filename;
            for (char& c : filenameLower) c = tolower(c);
            if (filenameLower.find(keyword) != std::string::npos) {
                return file;
            }
        }
    }
    
    return "";
}

// ========== 加载模型 ==========
bool loadModelFromFile(const std::string& path, ModelData& outData) {
    outData.vertices.clear();
    outData.indices.clear();
    
    bool success = loadModel(path, outData.vertices, outData.indices);
    outData.loaded = success;
    outData.path = path;
    
    return success;
}

// ========== 重新加载模型（含纹理查找） ==========
void reloadModel(const std::string& path, ModelData& modelData, TextureData& textureData) {
    modelData.vertices.clear();
    modelData.indices.clear();
    modelData.subMeshes.clear();
    modelData.materials.clear();

    if (loadModelMulti(path, modelData)) {
        std::cout << "模型加载成功: " << path << std::endl;
        std::cout << "  顶点数: " << modelData.vertices.size() / 11 << std::endl;
        std::cout << "  索引数: " << modelData.indices.size() << std::endl;
        
        // ========== 自动查找纹理 ==========
        std::string texPath = findTextureFile(path);
        if (!texPath.empty()) {
            if (textureData.colorTexture != 0) glDeleteTextures(1, &textureData.colorTexture);
            textureData.colorTexture = loadTexture(texPath);
            textureData.hasColor = true;
            std::cout << "  颜色贴图: " << texPath << std::endl;
        } else {
            textureData.hasColor = false;
            std::cout << "  颜色贴图: 未找到" << std::endl;
        }
        
        std::string normalPath = findNormalMap(path);
        if (!normalPath.empty()) {
            if (textureData.normalTexture != 0) glDeleteTextures(1, &textureData.normalTexture);
            textureData.normalTexture = loadTexture(normalPath);
            textureData.hasNormal = true;
            std::cout << "  法线贴图: " << normalPath << std::endl;
        } else {
            textureData.hasNormal = false;
            std::cout << "  法线贴图: 未找到" << std::endl;
        }

        std::string emissivePath = findEmissiveMap(path);
        if (!emissivePath.empty()) {
            if (textureData.emissiveTexture != 0) glDeleteTextures(1, &textureData.emissiveTexture);
            textureData.emissiveTexture = loadTexture(emissivePath);
            textureData.hasEmissive = true;
            std::cout << "  自发光贴图: " << emissivePath << std::endl;
        } else {
            textureData.hasEmissive = false;
            std::cout << "  自发光贴图: 未找到" << std::endl;
        }

        std::string opacityPath = findOpacityMap(path);
        if (!opacityPath.empty()) {
            if (textureData.opacityTexture != 0) glDeleteTextures(1, &textureData.opacityTexture);
            textureData.opacityTexture = loadTexture(opacityPath);
            textureData.hasOpacity = true;
            std::cout << "  透明度贴图: " << opacityPath << std::endl;
        } else {
            textureData.hasOpacity = false;
            std::cout << "  透明度贴图: 未找到" << std::endl;
        }

        std::string heightPath = findHeightMap(path);
        if (!heightPath.empty()) {
            if (textureData.heightTexture != 0) glDeleteTextures(1, &textureData.heightTexture);
            textureData.heightTexture = loadTexture(heightPath);
            textureData.hasHeight = true;
            std::cout << "  高度贴图: " << heightPath << std::endl;
        } else {
            textureData.hasHeight = false;
            std::cout << "  高度贴图: 未找到" << std::endl;
        }
    } else {
        std::cout << "模型加载失败: " << path << std::endl;
    }

    // 填充模型文件夹中的所有图片文件（供 UI 下拉列表使用）
    modelData.availableTextures = getAllImagesInModelFolder(path);
}

// ==========多材质加载函数 ==========
bool loadModelMulti(const std::string& path, ModelData& outData) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_FlipUVs | 
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "Assimp 错误: " << importer.GetErrorString() << std::endl;
        return false;
    }
    
    if (scene->mNumMeshes == 0) {
        std::cout << "模型没有网格数据" << std::endl;
        return false;
    }
    
    outData.subMeshes.clear();
    outData.materials.clear();
    outData.vertices.clear();
    outData.indices.clear();
    
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
    
    std::cout << "材质信息:" << std::endl;
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* meshPtr = scene->mMeshes[m];
        unsigned int matIndex = meshPtr->mMaterialIndex;
        aiMaterial* material = scene->mMaterials[matIndex];
        aiString matName;
        material->Get(AI_MATKEY_NAME, matName);
        std::cout << "  网格 " << m << " 材质: " << matName.C_Str() << std::endl;
        
        aiString texPath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            std::cout << "    颜色贴图: " << texPath.C_Str() << std::endl;
        }
        if (material->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS) {
            std::cout << "    法线贴图: " << texPath.C_Str() << std::endl;
        }
        if (material->GetTexture(aiTextureType_EMISSIVE, 0, &texPath) == AI_SUCCESS) {
            std::cout << "    自发光贴图: " << texPath.C_Str() << std::endl;
        }
        if (material->GetTexture(aiTextureType_OPACITY, 0, &texPath) == AI_SUCCESS) {
            std::cout << "    透明度贴图: " << texPath.C_Str() << std::endl;
        }
    }
    
    std::vector<std::string> materialNames;
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* meshPtr = scene->mMeshes[m];
        unsigned int matIndex = meshPtr->mMaterialIndex;
        aiMaterial* material = scene->mMaterials[matIndex];
        aiString matName;
        material->Get(AI_MATKEY_NAME, matName);
        std::string name = matName.C_Str();
        
        if (name.empty()) {
            name = "Material_" + std::to_string(matIndex);
        }
        
        bool exists = false;
        for (const auto& existing : materialNames) {
            if (existing == name) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            materialNames.push_back(name);
        }
    }
    
    for (const auto& name : materialNames) {
        MultiMaterial mat;
        mat.name = name;
        outData.materials.push_back(mat);
    }
    
    unsigned int globalVertexOffset = 0;
    unsigned int globalIndexOffset = 0;
    
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* meshPtr = scene->mMeshes[m];
        
        SubMesh sub;
        sub.vertexOffset = globalVertexOffset;
        sub.indexOffset = globalIndexOffset;
        
        unsigned int matIndex = meshPtr->mMaterialIndex;
        aiMaterial* material = scene->mMaterials[matIndex];
        aiString matName;
        material->Get(AI_MATKEY_NAME, matName);
        sub.materialName = matName.C_Str();
        if (sub.materialName.empty()) {
            sub.materialName = "Material_" + std::to_string(matIndex);
        }
        
        for (unsigned int i = 0; i < meshPtr->mNumVertices; i++) {
            sub.vertices.push_back((meshPtr->mVertices[i].x - centerX) * scale);
            sub.vertices.push_back((meshPtr->mVertices[i].y - centerY) * scale);
            sub.vertices.push_back((meshPtr->mVertices[i].z - centerZ) * scale);
            
            if (meshPtr->HasNormals()) {
                sub.vertices.push_back(meshPtr->mNormals[i].x);
                sub.vertices.push_back(meshPtr->mNormals[i].y);
                sub.vertices.push_back(meshPtr->mNormals[i].z);
            } else {
                sub.vertices.push_back(0.0f);
                sub.vertices.push_back(0.0f);
                sub.vertices.push_back(1.0f);
            }
            
            if (meshPtr->HasTextureCoords(0)) {
                sub.vertices.push_back(meshPtr->mTextureCoords[0][i].x);
                sub.vertices.push_back(meshPtr->mTextureCoords[0][i].y);
            } else {
                sub.vertices.push_back(0.0f);
                sub.vertices.push_back(0.0f);
            }
            
            if (meshPtr->HasTangentsAndBitangents()) {
                sub.vertices.push_back(meshPtr->mTangents[i].x);
                sub.vertices.push_back(meshPtr->mTangents[i].y);
                sub.vertices.push_back(meshPtr->mTangents[i].z);
            } else {
                sub.vertices.push_back(1.0f);
                sub.vertices.push_back(0.0f);
                sub.vertices.push_back(0.0f);
            }
        }
        
        for (unsigned int i = 0; i < meshPtr->mNumFaces; i++) {
            aiFace face = meshPtr->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                sub.indices.push_back(face.mIndices[j]);
            }
        }       
        globalVertexOffset += meshPtr->mNumVertices;
        globalIndexOffset += sub.indices.size();
        
        outData.vertices.insert(outData.vertices.end(), sub.vertices.begin(), sub.vertices.end());
        outData.indices.insert(outData.indices.end(), sub.indices.begin(), sub.indices.end());
        
        outData.subMeshes.push_back(sub);
        outData.path = path;
        outData.loaded = true;
    }
    
    std::cout << "模型加载成功！" << std::endl;
    std::cout << "  子网格数: " << outData.subMeshes.size() << std::endl;
    std::cout << "  材质数: " << outData.materials.size() << std::endl;
    std::cout << "  总顶点数: " << outData.vertices.size() / 11 << std::endl;
    std::cout << "  总索引数: " << outData.indices.size() << std::endl;
    
    return true;
}