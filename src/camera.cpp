#include "camera.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"


// ========== 相机状态 ==========
float yaw = 0.0f;
float pitch = 0.0f;
float zoom = 1.0f;
bool isDragging = false;
double lastX = 0.0f;
double lastY = 0.0f;

// ========== 光源控制 ==========
bool isDraggingLight = false;
float lightPosX = 2.0f;
float lightPosY = 3.0f;
float lightPosZ = 4.0f;
bool isHoveringLight = false;

// ========== 投影和视图矩阵（用于拾取） ==========
glm::mat4 lightProjectionMatrix;
glm::mat4 lightViewMatrix;

// ========== 判断鼠标是否点击到光源 ==========
bool isPointOnLight(float mouseX, float mouseY, float lightWorldX, float lightWorldY, float lightWorldZ) {
    // 将光源世界坐标转换到屏幕坐标
    glm::vec4 lightScreen = lightProjectionMatrix * lightViewMatrix * glm::vec4(lightWorldX, lightWorldY, lightWorldZ, 1.0f);
    
    if (lightScreen.w == 0.0f) return false;
    
    // 归一化到 [-1, 1]
    float screenX = lightScreen.x / lightScreen.w;
    float screenY = lightScreen.y / lightScreen.w;
    
    // 转换到像素坐标
    float pixelX = (screenX + 1.0f) * 400.0f;   // 窗口宽度的一半
    float pixelY = (1.0f - screenY) * 300.0f;   // 窗口高度的一半
    
    // 判断鼠标是否在光源附近（阈值 30 像素）
    float threshold = 30.0f;
    float dx = mouseX - pixelX;
    float dy = mouseY - pixelY;
    return (dx * dx + dy * dy) < (threshold * threshold);
}

// ========== 新增：检查鼠标是否在 ImGui 界面上 ==========
bool isMouseOverImGui() {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}
// ========== END ==========

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // ========== 新增：如果鼠标在 ImGui 上，不处理 3D 操作 ==========
    if (isMouseOverImGui()) {
        // 如果正在拖拽光源，取消
        if (action == GLFW_RELEASE) {
            isDraggingLight = false;
            isDragging = false;
        }
        return;
    }
    // ========== END ==========
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            
            if (mods & GLFW_MOD_SHIFT) {
                isDraggingLight = true;
                lastX = xpos;
                lastY = ypos;
                std::cout << "开始拖拽光源" << std::endl;
                return;
            }
            
            isDragging = true;
            lastX = xpos;
            lastY = ypos;
        } else if (action == GLFW_RELEASE) {
            isDragging = false;
            if (isDraggingLight) {
                std::cout << "光源位置: (" << lightPosX << ", " << lightPosY << ", " << lightPosZ << ")" << std::endl;
            }
            isDraggingLight = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    // ========== 新增：如果鼠标在 ImGui 上，不处理 3D 操作 ==========
    if (isMouseOverImGui()) {
        return;
    }
    // ========== END ==========
    
    float dx = (float)(xpos - lastX);
    float dy = (float)(ypos - lastY);
    
    // ========== 光源拖拽 ==========
    if (isDraggingLight) {
        // 鼠标左右移动 → 光源左右移动
        lightPosX += dx * 0.02f;
        // 鼠标上下移动 → 光源上下移动
        lightPosY -= dy * 0.02f;
        
        // 限制光源范围
        if (lightPosX < -10.0f) lightPosX = -10.0f;
        if (lightPosX > 10.0f) lightPosX = 10.0f;
        if (lightPosY < -5.0f) lightPosY = -5.0f;
        if (lightPosY > 10.0f) lightPosY = 10.0f;
        
        lastX = xpos;
        lastY = ypos;
        return;
    }
    // ========== END ==========
    
    // ========== 模型旋转 ==========
    if (isDragging) {
        yaw += dx * 0.005f;
        pitch += dy * 0.005f;
        
        if (pitch > 1.55f) pitch = 1.55f;
        if (pitch < -1.55f) pitch = -1.55f;
        
        lastX = xpos;
        lastY = ypos;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // ========== 新增：如果鼠标在 ImGui 上，不处理滚动 ==========
    if (isMouseOverImGui()) {
        return;
    }
    // ========== END ==========
    
    zoom += (float)yoffset * 0.5f;
    if (zoom < 0.5f) zoom = 0.5f;
    if (zoom > 10.0f) zoom = 10.0f;
}

void setupCameraCallbacks(GLFWwindow* window) {
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

// ========== 更新投影和视图矩阵（供拾取使用） ==========
void updatePickMatrices(const glm::mat4& projection, const glm::mat4& view) {
    lightProjectionMatrix = projection;
    lightViewMatrix = view;
}