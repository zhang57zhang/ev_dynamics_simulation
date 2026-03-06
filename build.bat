@echo off
REM =============================================================================
REM 构建脚本 - Windows
REM EV Dynamics Simulation Project
REM =============================================================================

setlocal EnableDelayedExpansion

REM 颜色支持（Windows 10+）
set "RESET=[0m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "RED=[91m"

REM 打印函数
goto :main

:print_info
echo %GREEN%[INFO]%RESET% %~1
goto :eof

:print_warning
echo %YELLOW%[WARN]%RESET% %~1
goto :eof

:print_error
echo %RED%[ERROR]%RESET% %~1
goto :eof

:main

REM =============================================================================
REM 检查依赖
REM =============================================================================
call :print_info "Checking dependencies..."

REM 检查CMake
where cmake >nul 2>&1
if errorlevel 1 (
    call :print_error "CMake not found! Please install CMake 3.16 or later"
    exit /b 1
)
for /f "tokens=3" %%i in ('cmake --version 2^>^&1 ^| findstr /C:"cmake version"') do set CMAKE_VERSION=%%i
call :print_info "CMake version: %CMAKE_VERSION%"

REM 检查MSVC编译器
where cl >nul 2>&1
if errorlevel 1 (
    call :print_warning "MSVC compiler not found in PATH"
    call :print_info "Attempting to find Visual Studio..."
    
    REM 尝试查找Visual Studio
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    ) else (
        call :print_error "Visual Studio not found! Please install Visual Studio 2019 or later"
        exit /b 1
    )
)

REM 检查Python
where python >nul 2>&1
if errorlevel 1 (
    call :print_error "Python not found!"
    exit /b 1
)
for /f "tokens=2" %%i in ('python --version 2^>^&1') do set PYTHON_VERSION=%%i
call :print_info "Python version: %PYTHON_VERSION%"

REM =============================================================================
REM 配置选项
REM =============================================================================
set BUILD_TYPE=Release
set BUILD_DIR=build
set CLEAN_BUILD=0
set RUN_TESTS=0
set VERBOSE=0

REM 解析命令行参数
:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto :parse_args
)
if /i "%~1"=="--release" (
    set BUILD_TYPE=Release
    shift
    goto :parse_args
)
if /i "%~1"=="--clean" (
    set CLEAN_BUILD=1
    shift
    goto :parse_args
)
if /i "%~1"=="--test" (
    set RUN_TESTS=1
    shift
    goto :parse_args
)
if /i "%~1"=="--verbose" (
    set VERBOSE=1
    shift
    goto :parse_args
)
if /i "%~1"=="-v" (
    set VERBOSE=1
    shift
    goto :parse_args
)
if /i "%~1"=="--help" (
    goto :show_help
)
if /i "%~1"=="-h" (
    goto :show_help
)

call :print_error "Unknown option: %~1"
exit /b 1

:show_help
echo Usage: %~nx0 [OPTIONS]
echo.
echo Options:
echo   --debug       Build in Debug mode
echo   --release     Build in Release mode (default)
echo   --clean       Clean build directory before building
echo   --test        Run tests after build
echo   --verbose, -v Enable verbose output
echo   --help, -h    Show this help message
exit /b 0

:args_done

REM =============================================================================
REM 清理构建目录
REM =============================================================================
if %CLEAN_BUILD%==1 (
    call :print_info "Cleaning build directory..."
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

REM =============================================================================
REM 创建构建目录
REM =============================================================================
call :print_info "Creating build directory..."
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM =============================================================================
REM 配置项目
REM =============================================================================
call :print_info "Configuring project (Build type: %BUILD_TYPE%)..."

cd "%BUILD_DIR%"

set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE%

if %VERBOSE%==1 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_VERBOSE_MAKEFILE=ON
)

cmake .. %CMAKE_ARGS%
if errorlevel 1 (
    call :print_error "CMake configuration failed!"
    cd ..
    exit /b 1
)

REM =============================================================================
REM 构建项目
REM =============================================================================
call :print_info "Building project..."

if %VERBOSE%==1 (
    cmake --build . --config %BUILD_TYPE% --verbose
) else (
    cmake --build . --config %BUILD_TYPE%
)

if errorlevel 1 (
    call :print_error "Build failed!"
    cd ..
    exit /b 1
)

REM =============================================================================
REM 运行测试
REM =============================================================================
if %RUN_TESTS%==1 (
    call :print_info "Running tests..."
    ctest -C %BUILD_TYPE% --output-on-failure
)

REM =============================================================================
REM 完成
REM =============================================================================
cd ..

call :print_info "Build completed successfully!"
call :print_info "Build artifacts are in: %BUILD_DIR%\"
call :print_info "Python modules are in: %BUILD_DIR%\python_modules\"

echo.
call :print_info "To use the Python modules:"
echo   set PYTHONPATH=%%PYTHONPATH%%;%CD%\%BUILD_DIR%\python_modules
echo   python -c "import powertrain; print('Powertrain module loaded!')"

endlocal
