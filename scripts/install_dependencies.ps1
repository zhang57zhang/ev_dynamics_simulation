# 依赖安装脚本
# ev_dynamics_simulation 项目

Write-Host "================================" -ForegroundColor Cyan
Write-Host "EV Dynamics Simulation 依赖安装" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

# 1. 检查 Visual Studio
Write-Host "[1/4] 检查 Visual Studio..." -ForegroundColor Yellow
$vsPath = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community"
if (Test-Path $vsPath) {
    Write-Host "✅ Visual Studio 2022 已安装" -ForegroundColor Green
} else {
    Write-Host "❌ Visual Studio 2022 未安装" -ForegroundColor Red
    Write-Host "请下载安装: https://visualstudio.microsoft.com/downloads/" -ForegroundColor Yellow
    Write-Host "选择: Desktop development with C++" -ForegroundColor Yellow
    exit 1
}

# 2. 检查 CMake
Write-Host "`n[2/4] 检查 CMake..." -ForegroundColor Yellow
$cmakeVersion = cmake --version 2>&1 | Select-String "cmake version"
if ($cmakeVersion) {
    Write-Host "✅ $cmakeVersion" -ForegroundColor Green
} else {
    Write-Host "❌ CMake 未安装" -ForegroundColor Red
    exit 1
}

# 3. 检查 Python
Write-Host "`n[3/4] 检查 Python..." -ForegroundColor Yellow
$pythonVersion = python --version 2>&1
Write-Host "当前版本: $pythonVersion" -ForegroundColor Cyan

if ($pythonVersion -match "3\.1[0-2]") {
    Write-Host "✅ Python 版本兼容" -ForegroundColor Green

    # 安装 pybind11
    Write-Host "安装 pybind11..." -ForegroundColor Yellow
    pip install pybind11 numpy scipy
} else {
    Write-Host "⚠️ Python 版本可能不兼容（推荐 3.10-3.12）" -ForegroundColor Yellow
    Write-Host "pybind11 可能不支持 Python 3.14+" -ForegroundColor Yellow

    $install = Read-Host "是否尝试安装 pybind11? (y/n)"
    if ($install -eq "y") {
        pip install pybind11 numpy scipy
    }
}

# 4. 配置 CMake
Write-Host "`n[4/4] 配置 CMake..." -ForegroundColor Yellow
$projectRoot = Split-Path $PSScriptRoot -Parent
$buildDir = Join-Path $projectRoot "build"

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Write-Host "构建目录: $buildDir" -ForegroundColor Cyan
Write-Host "项目目录: $projectRoot" -ForegroundColor Cyan

# 进入 Developer Command Prompt
$vsDevCmd = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
if (Test-Path $vsDevCmd) {
    Write-Host "配置编译环境..." -ForegroundColor Yellow
    cmd /c "`"$vsDevCmd`" && cd /d `"$buildDir`" && cmake .. -DBUILD_TESTING=ON -DBUILD_PYTHON_BINDINGS=ON"
} else {
    Write-Host "⚠️ 无法找到 VsDevCmd.bat" -ForegroundColor Yellow
    Write-Host "请手动在 Developer Command Prompt 中运行:" -ForegroundColor Yellow
    Write-Host "  cd $buildDir" -ForegroundColor Cyan
    Write-Host "  cmake .. -DBUILD_TESTING=ON -DBUILD_PYTHON_BINDINGS=ON" -ForegroundColor Cyan
}

Write-Host "`n================================" -ForegroundColor Cyan
Write-Host "安装完成！" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Cyan
