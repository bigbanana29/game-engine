@echo off
chcp 65001 >nul
echo ========================================
echo  正在编译 3D 模型查看器...
echo ========================================
echo.

cd src

g++ -std=c++17 main.cpp bloom.cpp ui.cpp model_manager.cpp shader.cpp model.cpp camera.cpp light.cpp texture.cpp glad.c tinyfiledialogs.c ../imgui/*.cpp ../imgui/backends/imgui_impl_glfw.cpp ../imgui/backends/imgui_impl_opengl3.cpp -o ../build/test.exe -I "../include" -I "../glfw-3.4.bin.WIN64/include" -I "../imgui" -I "../imgui/backends" -L "../glfw-3.4.bin.WIN64/lib-mingw-w64" -L "../build" -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32 -lassimp -lcomdlg32 -lole32 -luser32 -static-libstdc++ -static-libgcc

if %errorlevel% == 0 (
    echo.
    echo ========================================
    echo  编译成功！
    echo ========================================
    echo.
    echo  运行 build\test.exe 启动程序
) else (
    echo.
    echo ========================================
    echo  编译失败！请检查错误信息
    echo ========================================
)

echo.
pause