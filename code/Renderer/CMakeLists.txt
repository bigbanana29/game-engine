cmake_minimum_required(VERSION 3.15)
project(Renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 收集所有源文件
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/src/*.cpp"
)

add_executable(Renderer ${APP_SOURCES})

# ============================================================
# 所有头文件搜索路径
# ============================================================
target_include_directories(Renderer PRIVATE
    "${CMAKE_SOURCE_DIR}/source/head"
    "${CMAKE_SOURCE_DIR}/minifb/include"
    "${CMAKE_SOURCE_DIR}"          # ← 关键！让 #include <glm/glm.hpp> 能找到
)

# 添加 minifb 子项目
add_subdirectory(minifb)

# 链接库
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