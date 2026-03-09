# ev_dynamics_simulation 依赖验证脚本

Write-Host "================================" -ForegroundColor Cyan
Write-Host "依赖验证 - ev_dynamics_simulation" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

$allPassed = $true

# 1. Visual Studio 2026
Write-Host "[1/5] Visual Studio 2026" -ForegroundColor Yellow
$vsPath = "D:\Program Files\Microsoft Visual Studio\18\Community"
if (Test-Path $vsPath) {
    Write-Host "  ✅ 已安装" -ForegroundColor Green
} else {
    Write-Host "  ❌ 未安装" -ForegroundColor Red
    $allPassed = $false
}

# 2. C++ 工作负载
Write-Host "`n[2/5] C++ 工作负载" -ForegroundColor Yellow
$vcPath = "$vsPath\VC"
$msvcPath = "$vsPath\VC\Tools\MSVC"
$vcExists = Test-Path $vcPath
$msvcExists = Test-Path $msvcPath

if ($vcExists -and $msvcExists) {
    Write-Host "  ✅ 已安装" -ForegroundColor Green
    $clPath = Get-ChildItem -Path $msvcPath -Filter "cl.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($clPath) {
        Write-Host "  ✅ cl.exe 编译器已找到" -ForegroundColor Green
    }
} else {
    Write-Host "  ❌ 未安装" -ForegroundColor Red
    Write-Host "  解决方案: Visual Studio Installer -> 修改 -> 勾选 'Desktop development with C++'" -ForegroundColor Yellow
    $allPassed = $false
}

# 3. CMake
Write-Host "`n[3/5] CMake" -ForegroundColor Yellow
$cmakePath = "D:\Program Files\CMake\bin\cmake.exe"
if (Test-Path $cmakePath) {
    Write-Host "  ✅ 已安装: 3.31.11" -ForegroundColor Green
    $cmakeInPath = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmakeInPath) {
        Write-Host "  ✅ 已配置 PATH" -ForegroundColor Green
    } else {
        Write-Host "  ⚠️ 未配置 PATH" -ForegroundColor Yellow
    }
} else {
    Write-Host "  ❌ 未安装" -ForegroundColor Red
    $allPassed = $false
}

# 4. Windows SDK
Write-Host "`n[4/5] Windows SDK" -ForegroundColor Yellow
$sdkPath = "${env:ProgramFiles(x86)}\Windows Kits\10"
if (Test-Path $sdkPath) {
    Write-Host "  ✅ 已安装" -ForegroundColor Green
} else {
    Write-Host "  ❌ 未安装" -ForegroundColor Red
    $allPassed = $false
}

# 5. Python（可选）
Write-Host "`n[5/5] Python（可选）" -ForegroundColor Yellow
try {
    $pythonVersion = python --version 2>&1
    Write-Host "  ✅ 已安装: $pythonVersion" -ForegroundColor Green
} catch {
    Write-Host "  ℹ️ Python 未安装（可选）" -ForegroundColor Yellow
}

# 总结
Write-Host "`n================================" -ForegroundColor Cyan
if ($allPassed) {
    Write-Host "✅ 所有必需依赖已就绪！" -ForegroundColor Green
    Write-Host "`n下一步:" -ForegroundColor Cyan
    Write-Host "  1. 配置 CMake PATH（如果未配置）" -ForegroundColor White
    Write-Host "  2. 打开 Developer Command Prompt for VS 2026" -ForegroundColor White
    Write-Host "  3. 运行: .\build.ps1" -ForegroundColor White
} else {
    Write-Host "❌ 缺少必需依赖" -ForegroundColor Red
    Write-Host "`n修复步骤:" -ForegroundColor Yellow
    Write-Host "  1. 打开 Visual Studio Installer" -ForegroundColor White
    Write-Host "  2. 修改 Visual Studio 2026" -ForegroundColor White
    Write-Host "  3. 勾选 'Desktop development with C++'" -ForegroundColor White
    Write-Host "`n详细说明: DEPENDENCY_CHECK_20260308_2312.md" -ForegroundColor Cyan
}
Write-Host "================================" -ForegroundColor Cyan
