#ifndef UI_H
#define UI_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

// 前向声明
struct ModelData;
struct TextureData;

// ========== 每个材质的贴图选择 ==========
struct MaterialTextureSelection {
    std::string name;
    std::string colorTexture;
    std::string normalTexture;
    std::string roughnessTexture;
    std::string metallicTexture;
    std::string aoTexture;
    std::string emissiveTexture;
    std::string opacityTexture;
    std::string heightTexture;
    bool needReload = false;
};
// ========== END ==========

// ========== UI 控制变量（extern 声明，定义在其他 .cpp 中） ==========
extern float lightIntensity;
extern float ambientStrength;
extern float specularStrength;
extern float metallicValue;
extern float roughnessValue;
extern float diffuseStrength;
extern float textureStrength;
extern float rotationSpeed;
extern bool autoRotate;
// zoom, lightPosX/Y/Z 在 camera.cpp 中定义
extern glm::vec3 objectColor;
extern std::string currentModelPath;
extern bool needReloadModel;

// ========== 纹理选择变量 ==========
extern std::string selectedColorTexture;
extern std::string selectedNormalTexture;
extern std::string selectedRoughnessTexture;
extern std::string selectedMetallicTexture;
extern std::string selectedAOTexture;
extern std::string selectedEmissiveTexture;
extern std::string selectedOpacityTexture;
extern std::string selectedHeightTexture;
extern bool needReloadTexture;
// ========== END ==========

// ========== 材质高亮控制 ==========
extern int selectedMaterialIndex;  // -1 表示没有选中
extern std::vector<MaterialTextureSelection> materialSelections;
// ========== END ==========

// ========== 函数声明 ==========
void setupUI();
void renderUI(ModelData& modelData, TextureData& textureData);

#endif