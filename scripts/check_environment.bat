@echo off
REM =============================================================================
REM 环境诊断脚本 - Windows 10
REM EV Dynamics Simulation Project
REM =============================================================================

setlocal EnableDelayedExpansion

REM 颜色支持（Windows 10+）
set "RESET=[0m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "RED=[91m"
set "BLUE=[94m"
set "CYAN=[96m"

REM 计数器
set PASS=0
set FAIL=0
set WARN=0

echo.
echo %CYAN%========================================%RESET%
echo %CYAN%环境诊断工具 - Windows 10%RESET%
echo %CYAN%========================================%RESET%
echo.

REM =============================================================================
REM 1. 操作系统检查
REM =============================================================================
echo %BLUE%[1/8] 检查操作系统...%RESET%

ver | findstr /i "10.0" >nul
if errorlevel 1 (
    echo   %RED%✗ Windows 10 未检测到%RESET%
    set /a FAIL+=1
) else (
    for /f "tokens=4-5 delims=. " %%i in ('ver') do (
        set VERSION=%%i.%%j
    )
    echo   %GREEN%✓ Windows 10 检测到: !VERSION!%RESET%
    set /a PASS+=1
)

echo.

REM =============================================================================
REM 2. Visual Studio 检查
REM =============================================================================
echo %BLUE%[2/8] 检查 Visual Studio...%RESET%

set VS_FOUND=0

REM Visual Studio 2022
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    echo   %GREEN%✓ Visual Studio 2022 Community 已安装%RESET%
    set VS_FOUND=1
    set /a PASS+=1
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    echo   %GREEN%✓ Visual Studio 2022 Professional 已安装%RESET%
    set VS_FOUND=1
    set /a PASS+=1
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    echo   %GREEN%✓ Visual Studio 2022 Enterprise 已安装%RESET%
    set VS_FOUND=1
    set /a PASS+=1
)

REM Visual Studio 2019
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    echo   %GREEN%✓ Visual Studio 2019 Community 已安装%RESET%
    set VS_FOUND=1
    set /a PASS+=1
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    echo   %GREEN%✓ Visual Studio 2019 Professional 已安装%RESET%
    set VS_FOUND=1
    set /a PASS+=1
)

if %VS_FOUND%==0 (
    echo   %RED%✗ Visual Studio 未安装%RESET%
    echo   %YELLOW%  建议: 安装 Visual Studio 2019 或 2022%RESET%
    set /a FAIL+=1
)

echo.

REM =============================================================================
REM 3. CMake 检查
REM =============================================================================
echo %BLUE%[3/8] 检查 CMake...%RESET%

where cmake >nul 2>&1
if errorlevel 1 (
    echo   %RED%✗ CMake 未安装%RESET%
    echo   %YELLOW%  建议: 安装 CMake 3.16 或更高版本%RESET%
    set /a FAIL+=1
) else (
    for /f "tokens=3" %%i in ('cmake --version 2^>^&1 ^| findstr /C:"cmake version"') do set CMAKE_VERSION=%%i
    
    REM 提取主版本号
    for /f "tokens=1,2 delims=." %%a in ("!CMAKE_VERSION!") do (
        set CMAKE_MAJOR=%%a
        set CMAKE_MINOR=%%b
    )
    
    REM 检查版本是否 >= 3.16
    if !CMAKE_MAJOR! LSS 3 (
        echo   %RED%✗ CMake 版本过低: !CMAKE_VERSION! (需要 3.16+)%RESET%
        set /a FAIL+=1
    ) else if !CMAKE_MAJOR! EQU 3 (
        if !CMAKE_MINOR! LSS 16 (
            echo   %RED%✗ CMake 版本过低: !CMAKE_VERSION! (需要 3.16+)%RESET%
            set /a FAIL+=1
        ) else (
            echo   %GREEN%✓ CMake !CMAKE_VERSION!%RESET%
            set /a PASS+=1
        )
    ) else (
        echo   %GREEN%✓ CMake !CMAKE_VERSION!%RESET%
        set /a PASS+=1
    )
)

echo.

REM =============================================================================
REM 4. Python 检查
REM =============================================================================
echo %BLUE%[4/8] 检查 Python...%RESET%

where python >nul 2>&1
if errorlevel 1 (
    echo   %RED%✗ Python 未安装%RESET%
    echo   %YELLOW%  建议: 安装 Python 3.8 或更高版本%RESET%
    set /a FAIL+=1
) else (
    for /f "tokens=2" %%i in ('python --version 2^>^&1') do set PYTHON_VERSION=%%i
    
    REM 提取主版本号
    for /f "tokens=1,2 delims=." %%a in ("!PYTHON_VERSION!") do (
        set PYTHON_MAJOR=%%a
        set PYTHON_MINOR=%%b
    )
    
    REM 检查版本是否 >= 3.8
    if !PYTHON_MAJOR! LSS 3 (
        echo   %RED%✗ Python 版本过低: !PYTHON_VERSION! (需要 3.8+)%RESET%
        set /a FAIL+=1
    ) else if !PYTHON_MAJOR! EQU 3 (
        if !PYTHON_MINOR! LSS 8 (
            echo   %RED%✗ Python 版本过低: !PYTHON_VERSION! (需要 3.8+)%RESET%
            set /a FAIL+=1
        ) else (
            echo   %GREEN%✓ Python !PYTHON_VERSION!%RESET%
            set /a PASS+=1
        )
    ) else (
        echo   %GREEN%✓ Python !PYTHON_VERSION!%RESET%
        set /a PASS+=1
    )
    
    REM 检查 pip
    where pip >nul 2>&1
    if errorlevel 1 (
        echo   %YELLOW%! pip 未找到%RESET%
        set /a WARN+=1
    ) else (
        echo   %GREEN%  ✓ pip 已安装%RESET%
    )
)

echo.

REM =============================================================================
REM 5. pybind11 检查
REM =============================================================================
echo %BLUE%[5/8] 检查 pybind11...%RESET%

python -c "import pybind11" 2>nul
if errorlevel 1 (
    echo   %RED%✗ pybind11 未安装%RESET%
    echo   %YELLOW%  建议: pip install pybind11%RESET%
    set /a FAIL+=1
) else (
    for /f %%i in ('python -c "import pybind11; print(pybind11.__version__)"') do set PYBIND_VERSION=%%i
    echo   %GREEN%✓ pybind11 !PYBIND_VERSION!%RESET%
    set /a PASS+=1
)

echo.

REM =============================================================================
REM 6. 依赖库检查
REM =============================================================================
echo %BLUE%[6/8] 检查依赖库...%RESET%

REM 检查 Eigen3
set EIGEN_FOUND=0

REM vcpkg
if exist "C:\vcpkg\installed\x64-windows\include\eigen3\" (
    echo   %GREEN%✓ Eigen3 (vcpkg)%RESET%
    set EIGEN_FOUND=1
    set /a PASS+=1
)

REM 手动安装
if exist "C:\Libraries\eigen3\include\eigen3\" (
    echo   %GREEN%✓ Eigen3 (手动安装)%RESET%
    set EIGEN_FOUND=1
    set /a PASS+=1
)

if %EIGEN_FOUND%==0 (
    echo   %YELLOW%! Eigen3 未检测到%RESET%
    echo   %YELLOW%  建议: vcpkg install eigen3:x64-windows%RESET%
    set /a WARN+=1
)

REM 检查 Google Test
set GTEST_FOUND=0

if exist "C:\vcpkg\installed\x64-windows\include\gtest\" (
    echo   %GREEN%✓ Google Test (vcpkg)%RESET%
    set GTEST_FOUND=1
    set /a PASS+=1
)

if %GTEST_FOUND%==0 (
    echo   %YELLOW%! Google Test 未检测到%RESET%
    echo   %YELLOW%  建议: vcpkg install gtest:x64-windows%RESET%
    set /a WARN+=1
)

echo.

REM =============================================================================
REM 7. 项目文件检查
REM =============================================================================
echo %BLUE%[7/8] 检查项目文件...%RESET%

set FILES=CMakeLists.txt dependencies.cmake build.bat setup.bat README.md

for %%f in (%FILES%) do (
    if exist %%f (
        echo   %GREEN%✓ %%f%RESET%
        set /a PASS+=1
    ) else (
        echo   %RED%✗ %%f (缺失)%RESET%
        set /a FAIL+=1
    )
)

echo.

REM =============================================================================
REM 8. 构建状态检查
REM =============================================================================
echo %BLUE%[8/8] 检查构建状态...%RESET%

if exist build (
    echo   %GREEN%✓ 构建目录存在%RESET%
    set /a PASS+=1
    
    if exist build\python_modules\powertrain*.pyd (
        echo   %GREEN%✓ Python 模块已构建%RESET%
        set /a PASS+=1
        
        REM 测试模块导入
        set PYTHONPATH=%CD%\build\python_modules
        python -c "import powertrain" 2>nul
        if errorlevel 1 (
            echo   %YELLOW%! powertrain 模块导入失败%RESET%
            set /a WARN+=1
        ) else (
            echo   %GREEN%  ✓ powertrain 模块导入成功%RESET%
            set /a PASS+=1
        )
    ) else (
        echo   %YELLOW%! Python 模块未构建%RESET%
        set /a WARN+=1
    )
) else (
    echo   %YELLOW%! 构建目录不存在%RESET%
    set /a WARN+=1
)

echo.

REM =============================================================================
REM 总结
REM =============================================================================
echo %CYAN%========================================%RESET%
echo %CYAN%诊断总结%RESET%
echo %CYAN%========================================%RESET%
echo.

echo   通过: %GREEN%!PASS!%RESET%
echo   失败: %RED%!FAIL!%RESET%
echo   警告: %YELLOW%!WARN!%RESET%
echo.

if !FAIL! EQU 0 (
    echo %GREEN%✓ 环境检查通过！%RESET%
    echo.
    echo 下一步操作:
    echo   1. 运行 'build.bat' 构建项目
    echo   2. 运行 'ctest -C Release' 测试
    echo   3. 查看 'docs\WINDOWS10_USER_GUIDE.md' 了解更多
) else (
    echo %RED%✗ 环境检查未通过%RESET%
    echo.
    echo 请先解决上述问题:
    echo   - 查看详细的安装指南: docs\WINDOWS10_USER_GUIDE.md
    echo   - 常见问题解答: docs\FAQ.md
    echo   - 故障排查: docs\TROUBLESHOOTING.md
)

echo.
echo %CYAN%========================================%RESET%

endlocal

REM 返回退出码
if %FAIL% GTR 0 exit /b 1
exit /b 0
