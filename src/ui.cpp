#include "ui.h"
#include "camera.h"
#include "imgui.h"
#include "model_manager.h"
#include "tinyfiledialogs.h"
#include <iostream>
#include <filesystem>
#include <glm/glm.hpp>
#include <algorithm>

// ========== 显式引用 camera.cpp 中的变量 ==========
extern float yaw;
extern float pitch;
extern float zoom;
extern float lightPosX;
extern float lightPosY;
extern float lightPosZ;

// ========== UI 控制变量定义 ==========
float lightIntensity = 1.0f;
float ambientStrength = 0.15f;
float specularStrength = 0.3f;
float metallicValue = 0.0f;
float roughnessValue = 0.5f;
float diffuseStrength = 1.0f;
float textureStrength = 1.0f;
float rotationSpeed = 0.001f;
bool autoRotate = false;
glm::vec3 objectColor = glm::vec3(1.0f, 1.0f, 1.0f);
std::string currentModelPath = "../assets/models/Dragon 2.5_fbx.fbx";
bool needReloadModel = false;

// ========== 纹理选择变量 ==========
std::string selectedColorTexture = "";
std::string selectedNormalTexture = "";
std::string selectedRoughnessTexture = "";
std::string selectedMetallicTexture = "";
std::string selectedAOTexture = "";
std::string selectedEmissiveTexture = "";
std::string selectedOpacityTexture = "";
std::string selectedHeightTexture = "";
bool needReloadTexture = false;
// ========== END ==========

// ========== 材质高亮控制 ==========
int selectedMaterialIndex = -1;
std::vector<MaterialTextureSelection> materialSelections;
// ========== END ==========

// ---------- 辅助函数：贴图选择器（下拉列表 + 浏览按钮） ----------
static bool TextureSelector(const char* label, const std::vector<std::string>& availableFiles,
                            std::string& currentTexture, const char* matSuffix = "") {
    bool changed = false;
    ImGui::PushID(label);
    ImGui::PushID(matSuffix);

    // 显示标签（如 "Color"）
    ImGui::Text("%s", label);
    ImGui::SameLine();

    // 准备下拉按钮显示的预览文字（仅文件名）
    std::string previewName = currentTexture.empty() ? "None" : currentTexture;
    size_t pos = previewName.find_last_of("/\\");
    if (pos != std::string::npos) previewName = previewName.substr(pos + 1);

    std::string comboLabel = std::string("##combo") + label + matSuffix;
    if (ImGui::BeginCombo(comboLabel.c_str(), previewName.c_str())) {
        for (int i = 0; i < (int)availableFiles.size(); i++) {
            bool isSelected = (currentTexture == availableFiles[i]);
            std::string itemName = availableFiles[i];
            size_t p = itemName.find_last_of("/\\");
            if (p != std::string::npos) itemName = itemName.substr(p + 1);

            if (ImGui::Selectable(itemName.c_str(), isSelected)) {
                currentTexture = availableFiles[i];
                changed = true;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    if (ImGui::Button("Browse")) {
        const char* filters[] = {"*.jpg", "*.jpeg", "*.png", "*.tga", "*.bmp", "*.tif", "*.tiff", "*.dds"};
        const char* file = tinyfd_openFileDialog("Choose texture", "", 8, filters, NULL, 0);
        if (file) {
            currentTexture = file;
            changed = true;
        }
    }

    // 原版在这里有一个显示完整文件名的 ImGui::Text，为了方便已整合到预览中，
    // 若需要保留独立显示，可取消下面注释：
    // ImGui::SameLine();
    // std::string fullName = currentTexture.empty() ? "None" : currentTexture;
    // ImGui::Text("%s", fullName.c_str());

    ImGui::PopID();
    ImGui::PopID();
    return changed;
}

void setupUI() {
    // UI 初始化（如有需要）
}

void renderUI(ModelData& modelData, TextureData& textureData) {
    ImGui::Begin("Control Panel");

    ImGui::Text("3D Model Viewer Control");
    ImGui::Separator();

    // ========== File Load ==========
    ImGui::Text("File");
    if (ImGui::Button("Open Model...")) {
        const char* filters[] = { "*.fbx", "*.obj", "*.dae", "*.3ds", "*.gltf", "*.glb", "*.stl", "*.ply" };
        const char* filename = tinyfd_openFileDialog(
            "Select Model File",
            "",
            8,
            filters,
            NULL,
            0
        );
        if (filename) {
            currentModelPath = filename;
            needReloadModel = true;
            std::cout << "Selected file: " << filename << std::endl;
        }
    }
    ImGui::SameLine();
    std::string filename = currentModelPath;
    size_t slashPos = filename.find_last_of("/\\");
    if (slashPos != std::string::npos) {
        filename = filename.substr(slashPos + 1);
    }
    ImGui::Text("%s", filename.c_str());
    ImGui::Separator();

    // ========== 多材质选择面板 ==========
    if (modelData.materials.size() > 0) {
        ImGui::Text("Materials");
        ImGui::Separator();
        
        // 同步 materialSelections
        if (materialSelections.size() != modelData.materials.size()) {
            materialSelections.clear();
            for (const auto& mat : modelData.materials) {
                MaterialTextureSelection sel;
                sel.name = mat.name;
                materialSelections.push_back(sel);
            }
            selectedMaterialIndex = -1;
        }
        
        for (size_t i = 0; i < modelData.materials.size(); i++) {
            std::string matName = modelData.materials[i].name;
            bool isSelected = (selectedMaterialIndex == (int)i);
            
            // 使用 Selectable 实现点击高亮
            std::string label = matName + "##mat_" + std::to_string(i);
            if (ImGui::Selectable(label.c_str(), &isSelected)) {
                selectedMaterialIndex = (int)i;
                std::cout << "选中材质: " << matName << std::endl;
            }
            
            // 如果选中，显示该材质的贴图选择
            if (isSelected) {
                ImGui::Indent();
                auto& sel = materialSelections[i];
                std::string suffix = "##" + std::to_string(i);
                
                // 颜色贴图
                if (TextureSelector("Color", modelData.availableTextures, sel.colorTexture, suffix.c_str())) {
                    sel.needReload = true;
                    needReloadTexture = true;
                }
                
                // 法线贴图
                if (TextureSelector("Normal", modelData.availableTextures, sel.normalTexture, suffix.c_str())) {
                    sel.needReload = true;
                    needReloadTexture = true;
                }
                
                // 粗糙度贴图
                if (TextureSelector("Roughness", modelData.availableTextures, sel.roughnessTexture, suffix.c_str())) {
                    sel.needReload = true;
                    needReloadTexture = true;
                }
                
                // 金属度贴图
                if (TextureSelector("Metallic", modelData.availableTextures, sel.metallicTexture, suffix.c_str())) {
                    sel.needReload = true;
                    needReloadTexture = true;
                }
                
                // AO贴图
                if (TextureSelector("AO", modelData.availableTextures, sel.aoTexture, suffix.c_str())) {
                    sel.needReload = true;
                    needReloadTexture = true;
                }
                
                // 自发光贴图
                if (TextureSelector("Emissive", modelData.availableTextures, sel.emissiveTexture, suffix.c_str())) {
                    sel.needReload = true;
                    needReloadTexture = true;
                }
                
                // 透明度贴图
                if (TextureSelector("Opacity", modelData.availableTextures, sel.opacityTexture, suffix.c_str())) {
                    sel.needReload = true;
                    needReloadTexture = true;
                }
                
                // 高度贴图
                if (TextureSelector("Height", modelData.availableTextures, sel.heightTexture, suffix.c_str())) {
                    sel.needReload = true;
                    needReloadTexture = true;
                }
                
                ImGui::Unindent();
                ImGui::Separator();
            }
        }
        ImGui::Separator();
    }
    // ========== END ==========

    // ========== Texture Selection（单材质模式，兼容旧代码） ==========
    if (modelData.materials.size() == 0) {
        ImGui::Text("Textures");
        
        if (TextureSelector("Color", modelData.availableTextures, selectedColorTexture))
            needReloadTexture = true;
        if (TextureSelector("Normal", modelData.availableTextures, selectedNormalTexture))
            needReloadTexture = true;
        if (TextureSelector("Roughness", modelData.availableTextures, selectedRoughnessTexture))
            needReloadTexture = true;
        if (TextureSelector("Metallic", modelData.availableTextures, selectedMetallicTexture))
            needReloadTexture = true;
        if (TextureSelector("AO", modelData.availableTextures, selectedAOTexture))
            needReloadTexture = true;
        if (TextureSelector("Emissive", modelData.availableTextures, selectedEmissiveTexture))
            needReloadTexture = true;
        if (TextureSelector("Opacity", modelData.availableTextures, selectedOpacityTexture))
            needReloadTexture = true;
        if (TextureSelector("Height", modelData.availableTextures, selectedHeightTexture))
            needReloadTexture = true;
        
        // 清除纹理按钮
        if (ImGui::Button("Clear Textures")) {
            selectedColorTexture = "";
            selectedNormalTexture = "";
            selectedRoughnessTexture = "";
            selectedMetallicTexture = "";
            selectedAOTexture = "";
            selectedEmissiveTexture = "";
            selectedOpacityTexture = "";
            selectedHeightTexture = "";
            needReloadTexture = true;
        }
        ImGui::Separator();
    }
    // ========== END ==========

    // ========== Light Control ==========
    ImGui::Text("Light Control");
    ImGui::SliderFloat("Light X", &lightPosX, -5.0f, 5.0f);
    ImGui::SliderFloat("Light Y", &lightPosY, -5.0f, 10.0f);
    ImGui::SliderFloat("Light Z", &lightPosZ, -10.0f, 10.0f);
    ImGui::SliderFloat("Light Intensity", &lightIntensity, 0.0f, 2.0f);
    ImGui::Separator();

    // ========== Material ==========
    ImGui::Text("Material");
    ImGui::SliderFloat("Metallic", &metallicValue, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness", &roughnessValue, 0.0f, 1.0f);
    ImGui::SliderFloat("Ambient", &ambientStrength, 0.0f, 0.5f);
    ImGui::SliderFloat("Diffuse", &diffuseStrength, 0.0f, 2.0f);
    ImGui::SliderFloat("Specular", &specularStrength, 0.0f, 2.0f);
    ImGui::SliderFloat("Texture Strength", &textureStrength, 0.0f, 2.0f);
    ImGui::ColorEdit3("Object Color", (float*)&objectColor);
    ImGui::Separator();

    // ========== Model Control ==========
    ImGui::Text("Model Control");
    ImGui::SliderFloat("Zoom", &zoom, 0.5f, 10.0f);
    ImGui::Checkbox("Auto Rotate", &autoRotate);
    if (autoRotate) {
        ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.0001f, 0.01f);
    }
    ImGui::Separator();

    // ========== Debug Info ==========
    ImGui::Text("Debug Info");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Vertices: %zu", modelData.vertices.size() / 11);
    ImGui::Text("Indices: %zu", modelData.indices.size());
    ImGui::Text("SubMeshes: %zu", modelData.subMeshes.size());
    ImGui::Text("Materials: %zu", modelData.materials.size());
    ImGui::Text("Selected Material: %d", selectedMaterialIndex);
    ImGui::Text("Light Pos: (%.2f, %.2f, %.2f)", lightPosX, lightPosY, lightPosZ);
    ImGui::Text("Texture: %s", textureData.hasColor ? "Loaded" : "None");
    ImGui::Text("Normal: %s", textureData.hasNormal ? "Loaded" : "None");

    ImGui::End();
}