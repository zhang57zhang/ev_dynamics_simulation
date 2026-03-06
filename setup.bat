@echo off
REM =============================================================================
REM 快速启动脚本 - 验证开发环境 (Windows)
REM =============================================================================

setlocal EnableDelayedExpansion

REM 颜色
set "GREEN=[92m"
set "YELLOW=[93m"
set "RED=[91m"
set "RESET=[0m"

echo ========================================
echo EV Dynamics Simulation - 环境检查
echo ========================================
echo.

REM 检查操作系统
print_info "操作系统: Windows"

REM 检查CMake
where cmake >nul 2>&1
if errorlevel 1 (
    echo %RED%X%RESET% CMake未安装
    exit /b 1
) else (
    for /f "tokens=3" %%i in ('cmake --version 2^>^&1 ^| findstr /C:"cmake version"') do set CMAKE_VER=%%i
    echo %GREEN%✓%RESET% CMake: !CMAKE_VER!
)

REM 检查编译器
where cl >nul 2>&1
if errorlevel 1 (
    echo %YELLOW%!%RESET% MSVC编译器未找到（需要运行vcvarsall.bat）
) else (
    echo %GREEN%✓%RESET% MSVC编译器已配置
)

REM 检查Python
where python >nul 2>&1
if errorlevel 1 (
    echo %RED%X%RESET% Python未安装
    exit /b 1
) else (
    for /f "tokens=2" %%i in ('python --version 2^>^&1') do set PYTHON_VER=%%i
    echo %GREEN%✓%RESET% Python: !PYTHON_VER!
)

REM 检查pybind11
python -c "import pybind11" 2>nul
if errorlevel 1 (
    echo %YELLOW%!%RESET% pybind11未安装 (pip install pybind11)
) else (
    for /f %%i in ('python -c "import pybind11; print(pybind11.__version__)"') do set PYBIND_VER=%%i
    echo %GREEN%✓%RESET% pybind11: !PYBIND_VER!
)

REM 检查项目文件
echo.
echo ========================================
echo 项目文件检查
echo ========================================

set FILES=CMakeLists.txt dependencies.cmake build.bat README.md
for %%f in (%FILES%) do (
    if exist %%f (
        echo %GREEN%✓%RESET% %%f
    ) else (
        echo %RED%X%RESET% %%f (缺失)
    )
)

REM 检查构建目录
echo.
echo ========================================
echo 构建状态
echo ========================================

if exist build (
    echo ℹ 构建目录已存在
    
    if exist build\python_modules\powertrain*.pyd (
        echo %GREEN%✓%RESET% Python模块已构建
        
        echo.
        echo ℹ 测试模块导入...
        set PYTHONPATH=%CD%\build\python_modules
        
        python -c "import powertrain" 2>nul
        if errorlevel 1 (
            echo %YELLOW%!%RESET% powertrain模块导入失败
        ) else (
            echo %GREEN%✓%RESET% powertrain模块导入成功
        )
    ) else (
        echo ℹ Python模块未构建
        echo ℹ 运行 'build.bat' 进行构建
    )
) else (
    echo ℹ 构建目录不存在
    echo ℹ 运行 'build.bat' 进行首次构建
)

REM 总结
echo.
echo ========================================
echo 环境检查完成
echo ========================================
echo ℹ 下一步操作:
echo   1. 安装缺失的依赖（如有）
echo   2. 运行 'build.bat' 构建项目
echo   3. 运行 'python tests\python\test_all_modules.py' 测试模块
echo   4. 查看 'docs\DEV_ENVIRONMENT_SETUP.md' 了解更多

endlocal
