#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <windows.h>
#include <filesystem>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "camera.h"
#include "shader.h"
#include "model.h"
#include "light.h"
#include "texture.h"
#include "model_manager.h"
#include "ui.h"
#include "bloom.h"

// ========== 全局变量 ==========
extern float yaw;
extern float pitch;
extern float zoom;
extern float lightPosX;
extern float lightPosY;
extern float lightPosZ;
extern float lightIntensity;
extern float ambientStrength;
extern float specularStrength;
extern float rotationSpeed;
extern bool autoRotate;
extern glm::vec3 objectColor;
extern std::string currentModelPath;
extern bool needReloadModel;
extern std::string selectedColorTexture;
extern std::string selectedNormalTexture;
extern std::string selectedRoughnessTexture;
extern std::string selectedMetallicTexture;
extern std::string selectedAOTexture;
// ========== 新增 ==========
extern std::string selectedEmissiveTexture;
extern std::string selectedOpacityTexture;
extern std::string selectedHeightTexture;
// ========== END ==========
extern bool needReloadTexture;

int windowWidth = 1280;
int windowHeight = 720;

// ========== 窗口大小回调 ==========
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    resizeBloom(width, height);
}

// ========== 拖拽文件回调 ==========
void drop_callback(GLFWwindow* window, int count, const char** paths) {
    if (count > 0) {
        std::string filePath = paths[0];
        
        std::string ext;
        size_t dotPos = filePath.find_last_of('.');
        if (dotPos != std::string::npos) {
            ext = filePath.substr(dotPos);
        }
        
        std::vector<std::string> supportedExts = {".fbx", ".obj", ".dae", ".3ds", ".gltf", ".glb", ".stl", ".ply"};
        
        bool isSupported = false;
        for (const auto& supported : supportedExts) {
            if (ext == supported) {
                isSupported = true;
                break;
            }
        }
        
        if (isSupported) {
            currentModelPath = filePath;
            needReloadModel = true;
            std::cout << "拖拽加载模型: " << filePath << std::endl;
        } else {
            std::cout << "不支持的文件格式: " << ext << std::endl;
        }
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    if (!glfwInit()) {
        std::cout << "GLFW 初始化失败！" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "3D 模型查看器", nullptr, nullptr);
    if (!window) {
        std::cout << "窗口创建失败！" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    setupCameraCallbacks(window);
    glfwSetDropCallback(window, drop_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "GLAD 初始化失败！" << std::endl;
        return -1;
    }

    std::cout << "OpenGL 版本: " << glGetString(GL_VERSION) << std::endl;

    // 初始化 Bloom（只调用一次）
    initBloom(windowWidth, windowHeight);

    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ========== 编译着色器 ==========
    unsigned int shaderProgram = createShaderProgram();
    if (shaderProgram == 0) return -1;
    
    unsigned int lightShaderProgram = createLightShaderProgram();
    if (lightShaderProgram == 0) return -1;

    // ========== 加载默认模型 ==========
    ModelData modelData;
    TextureData textureData;
    textureData.colorTexture = 0;
    textureData.normalTexture = 0;
    textureData.hasColor = false;
    textureData.hasNormal = false;
    
    reloadModel(currentModelPath, modelData, textureData);

    // ========== 创建 VAO/VBO/EBO ==========
    std::vector<unsigned int> subVAOs;
    std::vector<unsigned int> subVBOs;
    std::vector<unsigned int> subEBOs;
    
    for (size_t i = 0; i < modelData.subMeshes.size(); i++) {
        unsigned int VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, modelData.subMeshes[i].vertices.size() * sizeof(float), 
                     modelData.subMeshes[i].vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelData.subMeshes[i].indices.size() * sizeof(unsigned int), 
                     modelData.subMeshes[i].indices.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);
        
        subVAOs.push_back(VAO);
        subVBOs.push_back(VBO);
        subEBOs.push_back(EBO);
        
        modelData.subMeshes[i].VAO = VAO;
        modelData.subMeshes[i].VBO = VBO;
        modelData.subMeshes[i].EBO = EBO;
    }
    
    // 兼容旧 VAO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, modelData.vertices.size() * sizeof(float), modelData.vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelData.indices.size() * sizeof(unsigned int), modelData.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // ========== 光源小球 ==========
    std::vector<float> lightVertices;
    std::vector<unsigned int> lightIndices;
    generateSphere(lightVertices, lightIndices, 0.3f, 16);

    unsigned int lightVAO, lightVBO, lightEBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glGenBuffers(1, &lightEBO);

    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, lightVertices.size() * sizeof(float), lightVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lightIndices.size() * sizeof(unsigned int), lightIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ========== Uniform 位置 ==========
    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    int textureSamplerLoc = glGetUniformLocation(shaderProgram, "textureSampler");
    int normalSamplerLoc = glGetUniformLocation(shaderProgram, "normalSampler");
    int roughnessSamplerLoc = glGetUniformLocation(shaderProgram, "roughnessSampler");
    int metallicSamplerLoc = glGetUniformLocation(shaderProgram, "metallicSampler");
    int aoSamplerLoc = glGetUniformLocation(shaderProgram, "aoSampler");
    int aoValueLoc = glGetUniformLocation(shaderProgram, "aoValue");
    int emissiveSamplerLoc = glGetUniformLocation(shaderProgram, "emissiveSampler");
    int opacitySamplerLoc = glGetUniformLocation(shaderProgram, "opacitySampler");
    int heightSamplerLoc = glGetUniformLocation(shaderProgram, "heightSampler");
    int isHighlightedLoc = glGetUniformLocation(shaderProgram, "isHighlighted");
    int highlightColorLoc = glGetUniformLocation(shaderProgram, "highlightColor");
    int highlightStrengthLoc = glGetUniformLocation(shaderProgram, "highlightStrength");
    int lightIntensityLoc = glGetUniformLocation(shaderProgram, "lightIntensity");
    int ambientLoc = glGetUniformLocation(shaderProgram, "ambientStrength");
    int specularLoc = glGetUniformLocation(shaderProgram, "specularStrength");
    int metallicLoc = glGetUniformLocation(shaderProgram, "metallicValue");
    int roughnessLoc = glGetUniformLocation(shaderProgram, "roughnessValue");
    int diffuseLoc = glGetUniformLocation(shaderProgram, "diffuseStrength");
    int textureStrengthLoc = glGetUniformLocation(shaderProgram, "textureStrength");
    int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    unsigned int lightTransformLoc = glGetUniformLocation(lightShaderProgram, "transform");

    float angle = 0.0f;

    // ---------- 线框模式状态变量 ----------
    bool wireframeMode = false;

    // ========== 主循环 ==========
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // ========== 点击视口取消选中材质 ==========
        static bool mousePressedInViewport = false;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mousePressedInViewport) {
            mousePressedInViewport = true;
            if (!ImGui::GetIO().WantCaptureMouse && selectedMaterialIndex != -1) {
                selectedMaterialIndex = -1;
            }
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            mousePressedInViewport = false;
        }

        // ========== 按键处理 ==========
        static bool fKeyPressed = false;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyPressed) {
            fKeyPressed = true;
            wireframeMode = !wireframeMode;          // 仅切换状态，不在此处设置 OpenGL 模式
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) fKeyPressed = false;

        static bool bKeyPressed = false;
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bKeyPressed) {
            bKeyPressed = true;
            static bool cullingEnabled = true;
            cullingEnabled = !cullingEnabled;
            if (cullingEnabled) glEnable(GL_CULL_FACE);
            else glDisable(GL_CULL_FACE);
        }
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) bKeyPressed = false;

        static bool f11KeyPressed = false;
        if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS && !f11KeyPressed) {
            f11KeyPressed = true;
            static bool fullscreen = false;
            static int wx, wy, ww, wh;
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            if (fullscreen = !fullscreen) {
                glfwGetWindowPos(window, &wx, &wy);
                glfwGetWindowSize(window, &ww, &wh);
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            } else {
                glfwSetWindowMonitor(window, nullptr, wx, wy, ww, wh, 0);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_RELEASE) f11KeyPressed = false;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) selectedMaterialIndex = -1;

        static bool rKeyPressed = false;
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rKeyPressed) {
            rKeyPressed = true;
            yaw = 0.0f; pitch = 0.0f; zoom = 3.0f; angle = 0.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) rKeyPressed = false;

        // ========== ImGui 新帧 ==========
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        renderUI(modelData, textureData);

        // ========== 重新加载模型 ==========
        if (needReloadModel) {
            modelData.vertices.clear();
            modelData.indices.clear();
            modelData.subMeshes.clear();
            modelData.materials.clear();
            reloadModel(currentModelPath, modelData, textureData);

            subVAOs.clear(); subVBOs.clear(); subEBOs.clear();
            for (size_t i = 0; i < modelData.subMeshes.size(); i++) {
                unsigned int VAO, VBO, EBO;
                glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glGenBuffers(1, &EBO);
                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, modelData.subMeshes[i].vertices.size() * sizeof(float),
                             modelData.subMeshes[i].vertices.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelData.subMeshes[i].indices.size() * sizeof(unsigned int),
                             modelData.subMeshes[i].indices.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
                glEnableVertexAttribArray(3);
                subVAOs.push_back(VAO); subVBOs.push_back(VBO); subEBOs.push_back(EBO);
                modelData.subMeshes[i].VAO = VAO;
                modelData.subMeshes[i].VBO = VBO;
                modelData.subMeshes[i].EBO = EBO;
            }

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, modelData.vertices.size() * sizeof(float), modelData.vertices.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelData.indices.size() * sizeof(unsigned int), modelData.indices.data(), GL_STATIC_DRAW);

            needReloadModel = false;
        }

        // ========== 重新加载纹理 ==========
        if (needReloadTexture) {
            if (modelData.materials.size() > 0) {
                for (auto& sel : materialSelections) {
                    for (auto& mat : modelData.materials) {
                        if (sel.name == mat.name) {
                            if (!sel.colorTexture.empty()) { if (mat.colorTexture) glDeleteTextures(1, &mat.colorTexture); mat.colorTexture = loadTexture(sel.colorTexture); mat.hasColor = true; } else mat.hasColor = false;
                            if (!sel.normalTexture.empty()) { if (mat.normalTexture) glDeleteTextures(1, &mat.normalTexture); mat.normalTexture = loadTexture(sel.normalTexture); mat.hasNormal = true; } else mat.hasNormal = false;
                            if (!sel.roughnessTexture.empty()) { if (mat.roughnessTexture) glDeleteTextures(1, &mat.roughnessTexture); mat.roughnessTexture = loadTexture(sel.roughnessTexture); mat.hasRoughness = true; } else mat.hasRoughness = false;
                            if (!sel.metallicTexture.empty()) { if (mat.metallicTexture) glDeleteTextures(1, &mat.metallicTexture); mat.metallicTexture = loadTexture(sel.metallicTexture); mat.hasMetallic = true; } else mat.hasMetallic = false;
                            if (!sel.aoTexture.empty()) { if (mat.aoTexture) glDeleteTextures(1, &mat.aoTexture); mat.aoTexture = loadTexture(sel.aoTexture); mat.hasAO = true; } else mat.hasAO = false;
                            if (!sel.emissiveTexture.empty()) { if (mat.emissiveTexture) glDeleteTextures(1, &mat.emissiveTexture); mat.emissiveTexture = loadTexture(sel.emissiveTexture); mat.hasEmissive = true; } else mat.hasEmissive = false;
                            if (!sel.opacityTexture.empty()) { if (mat.opacityTexture) glDeleteTextures(1, &mat.opacityTexture); mat.opacityTexture = loadTexture(sel.opacityTexture); mat.hasOpacity = true; } else mat.hasOpacity = false;
                            if (!sel.heightTexture.empty()) { if (mat.heightTexture) glDeleteTextures(1, &mat.heightTexture); mat.heightTexture = loadTexture(sel.heightTexture); mat.hasHeight = true; } else mat.hasHeight = false;
                        }
                    }
                }
            } else {
                auto loadTex = [](std::string& path, unsigned int& tex, bool& has) {
                    if (!path.empty()) { if (tex) glDeleteTextures(1, &tex); tex = loadTexture(path); has = true; } else has = false;
                };
                loadTex(selectedColorTexture, textureData.colorTexture, textureData.hasColor);
                loadTex(selectedNormalTexture, textureData.normalTexture, textureData.hasNormal);
                loadTex(selectedRoughnessTexture, textureData.roughnessTexture, textureData.hasRoughness);
                loadTex(selectedMetallicTexture, textureData.metallicTexture, textureData.hasMetallic);
                loadTex(selectedAOTexture, textureData.aoTexture, textureData.hasAO);
                loadTex(selectedEmissiveTexture, textureData.emissiveTexture, textureData.hasEmissive);
                loadTex(selectedOpacityTexture, textureData.opacityTexture, textureData.hasOpacity);
                loadTex(selectedHeightTexture, textureData.heightTexture, textureData.hasHeight);
            }
            needReloadTexture = false;
        }

        // ========== 渲染到 FBO ==========
        glBindFramebuffer(GL_FRAMEBUFFER, getBloomSceneFBO());
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        if (autoRotate) angle += rotationSpeed;

        float aspect = (float)windowWidth / (float)windowHeight;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        if (autoRotate) model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(zoom));
        glm::mat4 transform = projection * view * model;

        // ---------- 应用线框模式 ----------
        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

        // ========== 绘制多材质 ==========
        glUseProgram(shaderProgram);
        for (size_t i = 0; i < modelData.subMeshes.size(); i++) {
            std::string matName = modelData.subMeshes[i].materialName;

            MultiMaterial* mat = nullptr;
            for (auto& m : modelData.materials) if (m.name == matName) { mat = &m; break; }

            bool useBlending = false;
            if (mat) useBlending = mat->hasEmissive || mat->hasOpacity;
            if (matName == "rune_arc" || matName.find("beam") != std::string::npos ||
                matName.find("particle") != std::string::npos || matName.find("glow") != std::string::npos ||
                matName == "light_trail") {
                useBlending = true;
            }

            if (useBlending) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glDepthMask(GL_FALSE);
            } else {
                glDisable(GL_BLEND);
                glDepthMask(GL_TRUE);
            }

            bool isHighlighted = false;
            if (selectedMaterialIndex >= 0 && selectedMaterialIndex < (int)modelData.materials.size())
                isHighlighted = (matName == modelData.materials[selectedMaterialIndex].name);

            auto bindTex = [&](int unit, unsigned int tex, bool has, int loc) {
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, has ? tex : 0);
                glUniform1i(loc, unit);
            };
            if (mat == nullptr) {
                bindTex(1, textureData.colorTexture, textureData.hasColor, textureSamplerLoc);
                bindTex(2, textureData.normalTexture, textureData.hasNormal, normalSamplerLoc);
                bindTex(3, textureData.roughnessTexture, textureData.hasRoughness, roughnessSamplerLoc);
                bindTex(4, textureData.metallicTexture, textureData.hasMetallic, metallicSamplerLoc);
                bindTex(5, textureData.aoTexture, textureData.hasAO, aoSamplerLoc);
                bindTex(6, textureData.emissiveTexture, textureData.hasEmissive, emissiveSamplerLoc);
                bindTex(7, textureData.opacityTexture, textureData.hasOpacity, opacitySamplerLoc);
                bindTex(8, textureData.heightTexture, textureData.hasHeight, heightSamplerLoc);
            } else {
                bindTex(1, mat->colorTexture, mat->hasColor, textureSamplerLoc);
                bindTex(2, mat->normalTexture, mat->hasNormal, normalSamplerLoc);
                bindTex(3, mat->roughnessTexture, mat->hasRoughness, roughnessSamplerLoc);
                bindTex(4, mat->metallicTexture, mat->hasMetallic, metallicSamplerLoc);
                bindTex(5, mat->aoTexture, mat->hasAO, aoSamplerLoc);
                bindTex(6, mat->emissiveTexture, mat->hasEmissive, emissiveSamplerLoc);
                bindTex(7, mat->opacityTexture, mat->hasOpacity, opacitySamplerLoc);
                bindTex(8, mat->heightTexture, mat->hasHeight, heightSamplerLoc);
            }

            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
            glUniform3f(lightPosLoc, lightPosX, lightPosY, lightPosZ);
            glUniform3f(viewPosLoc, 0.0f, 0.0f, 5.0f);
            glUniform1f(lightIntensityLoc, lightIntensity);
            glUniform1f(ambientLoc, ambientStrength);
            glUniform1f(specularLoc, specularStrength);
            glUniform1f(metallicLoc, metallicValue);
            glUniform1f(roughnessLoc, roughnessValue);
            glUniform1f(diffuseLoc, diffuseStrength);
            glUniform1f(textureStrengthLoc, textureStrength);
            glUniform3f(colorLoc, objectColor.r, objectColor.g, objectColor.b);
            glUniform1i(isHighlightedLoc, isHighlighted ? 1 : 0);
            glUniform3f(highlightColorLoc, 1.0f, 0.0f, 0.0f);
            glUniform1f(highlightStrengthLoc, 0.5f);

            bool usePointSprites = (matName.find("particle") != std::string::npos ||
                                    matName.find("spark") != std::string::npos);
            if (usePointSprites) {
                glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
                glPointSize(12.0f);
            }

            glBindVertexArray(subVAOs[i]);
            if (usePointSprites)
                glDrawElements(GL_POINTS, modelData.subMeshes[i].indices.size(), GL_UNSIGNED_INT, 0);
            else
                glDrawElements(GL_TRIANGLES, modelData.subMeshes[i].indices.size(), GL_UNSIGNED_INT, 0);
            if (usePointSprites) glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
        }

        // ========== 兼容模式 ==========
        if (modelData.subMeshes.empty()) {
            glUseProgram(shaderProgram);
            auto bindTex = [&](int unit, unsigned int tex, bool has, int loc) {
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, has ? tex : 0);
                glUniform1i(loc, unit);
            };
            bindTex(1, textureData.colorTexture, textureData.hasColor, textureSamplerLoc);
            bindTex(2, textureData.normalTexture, textureData.hasNormal, normalSamplerLoc);
            bindTex(3, textureData.roughnessTexture, textureData.hasRoughness, roughnessSamplerLoc);
            bindTex(4, textureData.metallicTexture, textureData.hasMetallic, metallicSamplerLoc);
            bindTex(5, textureData.aoTexture, textureData.hasAO, aoSamplerLoc);
            bindTex(6, textureData.emissiveTexture, textureData.hasEmissive, emissiveSamplerLoc);
            bindTex(7, textureData.opacityTexture, textureData.hasOpacity, opacitySamplerLoc);
            bindTex(8, textureData.heightTexture, textureData.hasHeight, heightSamplerLoc);

            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
            glUniform3f(lightPosLoc, lightPosX, lightPosY, lightPosZ);
            glUniform3f(viewPosLoc, 0.0f, 0.0f, 5.0f);
            glUniform1f(lightIntensityLoc, lightIntensity);
            glUniform1f(ambientLoc, ambientStrength);
            glUniform1f(specularLoc, specularStrength);
            glUniform1f(metallicLoc, metallicValue);
            glUniform1f(roughnessLoc, roughnessValue);
            glUniform1f(diffuseLoc, diffuseStrength);
            glUniform1f(textureStrengthLoc, textureStrength);
            glUniform3f(colorLoc, objectColor.r, objectColor.g, objectColor.b);

            bool useBlending = textureData.hasEmissive || textureData.hasOpacity;
            if (useBlending) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glDepthMask(GL_FALSE);
            } else {
                glDisable(GL_BLEND);
                glDepthMask(GL_TRUE);
            }

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, modelData.indices.size(), GL_UNSIGNED_INT, 0);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        // ========== 光源小球 ==========
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        // 强制光源小球以实体方式绘制（可选，如果你希望它始终是球体）
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), glm::vec3(lightPosX, lightPosY, lightPosZ));
        lightModel = glm::scale(lightModel, glm::vec3(0.3f));
        glUseProgram(lightShaderProgram);
        glUniformMatrix4fv(lightTransformLoc, 1, GL_FALSE, glm::value_ptr(lightModel));
        glBindVertexArray(lightVAO);
        glDrawElements(GL_TRIANGLES, lightIndices.size(), GL_UNSIGNED_INT, 0);

        // ========== 结束 FBO，应用 Bloom ==========
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        applyBloom(0);

        // ========== ImGui 渲染 ==========
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // ========== 清理 ==========
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    for (auto& sub : modelData.subMeshes) {
        if (sub.VAO) glDeleteVertexArrays(1, &sub.VAO);
        if (sub.VBO) glDeleteBuffers(1, &sub.VBO);
        if (sub.EBO) glDeleteBuffers(1, &sub.EBO);
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &lightVBO);
    glDeleteBuffers(1, &lightEBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(lightShaderProgram);
    glfwTerminate();
    return 0;
}