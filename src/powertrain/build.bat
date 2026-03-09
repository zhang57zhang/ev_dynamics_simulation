@echo off
REM Powertrain模块构建脚本 (Windows)

echo ========================================
echo Powertrain模块 - 构建验证
echo ========================================
echo.

REM 检查CMake
echo 步骤1: 检查依赖库...
where cmake >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] CMake已安装
) else (
    echo [ERROR] CMake未安装，请先安装
    exit /b 1
)

REM 检查编译器
where cl >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] MSVC已安装
) else (
    echo [WARN] MSVC未找到，尝试使用MinGW...
    where g++ >nul 2>&1
    if %errorlevel% equ 0 (
        echo [OK] MinGW已安装
    ) else (
        echo [ERROR] 未找到C++编译器
        exit /b 1
    )
)

echo.
echo 步骤2: 创建构建目录...
if exist build (
    echo 清理旧构建...
    rmdir /s /q build
)
mkdir build
cd build

echo.
echo 步骤3: 配置CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON

if %errorlevel% neq 0 (
    echo [ERROR] CMake配置失败！
    exit /b 1
)

echo.
echo 步骤4: 编译项目...
cmake --build . --config Release

if %errorlevel% equ 0 (
    echo [OK] 编译成功！
) else (
    echo [ERROR] 编译失败！
    exit /b 1
)

echo.
echo 步骤5: 运行测试...
ctest -C Release --output-on-failure

if %errorlevel% equ 0 (
    echo [OK] 所有测试通过！
) else (
    echo [ERROR] 测试失败！
    exit /b 1
)

echo.
echo ========================================
echo 构建验证完成！
echo ========================================
echo.
echo 生成文件:
echo   - powertrain.lib
echo   - test_powertrain.exe
echo.
echo 测试结果:
ctest -N | find "Total Tests"
echo.

echo [OK] Powertrain模块验证成功！
pause
