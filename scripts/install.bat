@echo off
REM EV Dynamics Simulation - Installation Script (Windows)
REM Version: 1.0
REM Date: 2026-03-06

setlocal enabledelayedexpansion

echo ========================================
echo EV Dynamics Simulation - Installer
echo ========================================
echo.

REM Check prerequisites
echo [1/7] Checking prerequisites...

where cmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Error: CMake not found. Please install CMake 3.16+
    exit /b 1
)

where python >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Error: Python not found. Please install Python 3.10+
    exit /b 1
)

echo √ Prerequisites check passed

REM Install dependencies (using vcpkg)
echo [2/7] Installing dependencies...

where vcpkg >nul 2>&1
if %ERRORLEVEL% equ 0 (
    vcpkg install eigen3:x64-windows gtest:x64-windows yaml-cpp:x64-windows
    echo √ Dependencies installed via vcpkg
) else (
    echo Warning: vcpkg not found. Please install dependencies manually.
    echo Required: Eigen3, Google Test, yaml-cpp
)

REM Create build directory
echo [3/7] Creating build directory...

if not exist build mkdir build
cd build

echo √ Build directory created

REM Configure CMake
echo [4/7] Configuring CMake...

cmake .. ^
    -G "Visual Studio 16 2019" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX="C:\Program Files\EVDynamics" ^
    -DBUILD_TESTS=ON ^
    -DBUILD_PYTHON_BINDINGS=ON

if %ERRORLEVEL% neq 0 (
    echo Error: CMake configuration failed
    exit /b 1
)

echo √ CMake configured

REM Build
echo [5/7] Building project...

cmake --build . --config Release --parallel

if %ERRORLEVEL% neq 0 (
    echo Error: Build failed
    exit /b 1
)

echo √ Build completed

REM Run tests
echo [6/7] Running tests...

ctest -C Release --output-on-failure

if %ERRORLEVEL% neq 0 (
    echo Warning: Some tests failed
) else (
    echo √ All tests passed
)

REM Install
echo [7/7] Installing...

cmake --install . --config Release

REM Install Python package
cd ..
pip install -e .

echo √ Installation completed

echo.
echo ========================================
echo √ EV Dynamics Simulation installed successfully!
echo ========================================
echo.
echo Installation directory: C:\Program Files\EVDynamics
echo Binary: C:\Program Files\EVDynamics\bin\ev_dynamics_sim.exe
echo Python package: ev_dynamics
echo.
echo To get started, run:
echo   ev_dynamics_sim.exe --help
echo   python -c "import ev_dynamics; print(ev_dynamics.__version__)"
echo.

pause
