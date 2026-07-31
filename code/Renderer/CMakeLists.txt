cmake_minimum_required(VERSION 3.15)
project(Renderer LANGUAGES CXX)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
# 项目本身的源文件：
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/source/*.h"
    "${CMAKE_SOURCE_DIR}/source/*.hpp"
)
 
add_executable(Renderer ${APP_SOURCES})
 
# 头文件附加目录：minifb/include 和 glm
target_include_directories(Renderer PRIVATE
    "${CMAKE_SOURCE_DIR}/minifb/include"
    "C:/Users/bigbanana666/Desktop/game engine/code/Renderer"   # ← 指向 Renderer 目录，而不是 glm 目录
)
 
# 添加 minifb 到构建项目：
add_subdirectory(minifb)
 
# 链接库：
target_link_libraries(Renderer PRIVATE minifb)
if (WIN32)
target_link_libraries(Renderer PRIVATE
    minifb
    opengl32
    winmm
    kernel32
    user32
    gdi32
    winspool
    shell32
    ole32
    oleaut32
    uuid
    comdlg32
    advapi32)
endif()