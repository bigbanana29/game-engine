#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <string>
#include <vector>

// ========== 子网格数据 ==========
struct SubMesh {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::string materialName;  // 材质名称（如 "marikahammer"）
    unsigned int vertexOffset = 0;
    unsigned int indexOffset = 0;
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
};
// ========== END ==========

// ========== 材质信息 ==========
struct MultiMaterial {
    std::string name;
    unsigned int colorTexture = 0;
    unsigned int normalTexture = 0;
    unsigned int roughnessTexture = 0;
    unsigned int metallicTexture = 0;
    unsigned int aoTexture = 0;
    unsigned int emissiveTexture = 0;
    unsigned int opacityTexture = 0;
    unsigned int heightTexture = 0;
    bool hasColor = false;
    bool hasNormal = false;
    bool hasRoughness = false;
    bool hasMetallic = false;
    bool hasAO = false;
    bool hasEmissive = false;
    bool hasOpacity = false;
    bool hasHeight = false;
};
// ========== END ==========

// 模型数据（多材质版本）
struct ModelData {
    std::vector<SubMesh> subMeshes;  // 所有子网格
    std::vector<MultiMaterial> materials;  // 所有材质
    std::string path;
    std::vector<std::string> availableTextures;   // 模型文件夹中的所有图片文件路径
    bool loaded = false;
    
    // 兼容旧接口（所有顶点合并）
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

// 纹理数据
struct TextureData {
    unsigned int colorTexture = 0;
    unsigned int normalTexture = 0;
    unsigned int roughnessTexture = 0;
    unsigned int metallicTexture = 0;
    unsigned int aoTexture = 0;
    unsigned int emissiveTexture = 0;
    unsigned int opacityTexture = 0;
    unsigned int heightTexture = 0;

    bool hasColor = false;
    bool hasNormal = false;
    bool hasRoughness = false;
    bool hasMetallic = false;
    bool hasAO = false;
    bool hasEmissive = false;
    bool hasOpacity = false;
    bool hasHeight = false;
};

// 函数声明
bool loadModelFromFile(const std::string& path, ModelData& outData);
bool loadModelMulti(const std::string& path, ModelData& outData);
std::string findTextureFile(const std::string& modelPath);
std::string findNormalMap(const std::string& modelPath);
std::string findRoughnessMap(const std::string& modelPath);
std::string findMetallicMap(const std::string& modelPath);
std::string findAOMap(const std::string& modelPath);
std::string findEmissiveMap(const std::string& modelPath);
std::string findOpacityMap(const std::string& modelPath);
std::string findHeightMap(const std::string& modelPath);
void reloadModel(const std::string& path, ModelData& modelData, TextureData& textureData);
std::vector<std::string> getAllImagesInModelFolder(const std::string& modelPath);

#endif