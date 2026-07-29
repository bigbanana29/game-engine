#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>

// ========== 相机状态 ==========
extern float yaw;
extern float pitch;
extern float zoom;
extern bool isDragging;
extern double lastX;
extern double lastY;

// ========== 光源控制 ==========
extern bool isDraggingLight;
extern float lightPosX;
extern float lightPosY;
extern float lightPosZ;

// ========== 鼠标拾取（用于点击光源） ==========
extern bool isHoveringLight;  // 鼠标是否悬停在光源上

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void setupCameraCallbacks(GLFWwindow* window);

#endif