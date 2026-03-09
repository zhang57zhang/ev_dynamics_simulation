# ev_dynamics_simulation 依赖验证

Write-Host "================================" -ForegroundColor Cyan
Write-Host "依赖验证 - VS Insiders" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

$allPassed = $true

# 1. Visual Studio 2026 Insiders
Write-Host "[1/5] Visual Studio 2026 Insiders" -ForegroundColor Yellow
$vsPath = "D:\Program Files\Microsoft Visual Studio\18\Insiders"
if (Test-Path $vsPath) {
    Write-Host "  OK - 已安装" -ForegroundColor Green
} else {
    Write-Host "  FAIL - 未安装" -ForegroundColor Red
    $allPassed = $false
}

# 2. MSVC 编译器
Write-Host "`n[2/5] MSVC 编译器" -ForegroundColor Yellow
$msvcPath = "$vsPath\VC\Tools\MSVC\14.50.35717"
$clPath = "$msvcPath\bin\Hostx64\x64\cl.exe"

if ((Test-Path $msvcPath) -and (Test-Path $clPath)) {
    Write-Host "  OK - MSVC 14.50.35717" -ForegroundColor Green
    Write-Host "  OK - cl.exe 编译器" -ForegroundColor Green
} else {
    Write-Host "  FAIL - MSVC 或 cl.exe 未找到" -ForegroundColor Red
    $allPassed = $false
}

# 3. CMake
Write-Host "`n[3/5] CMake" -ForegroundColor Yellow
$cmakePath = "D:\Program Files\CMake\bin\cmake.exe"
if (Test-Path $cmakePath) {
    Write-Host "  OK - 已安装: 3.31.11" -ForegroundColor Green
    $cmakeInPath = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmakeInPath) {
        Write-Host "  OK - 已配置 PATH" -ForegroundColor Green
    } else {
        Write-Host "  WARN - 未配置 PATH" -ForegroundColor Yellow
    }
} else {
    Write-Host "  FAIL - 未安装" -ForegroundColor Red
    $allPassed = $false
}

# 4. Windows SDK
Write-Host "`n[4/5] Windows SDK" -ForegroundColor Yellow
$sdkPath = "${env:ProgramFiles(x86)}\Windows Kits\10"
if (Test-Path $sdkPath) {
    Write-Host "  OK - 已安装" -ForegroundColor Green
} else {
    Write-Host "  FAIL - 未安装" -ForegroundColor Red
    $allPassed = $false
}

# 5. Developer Command Prompt
Write-Host "`n[5/5] Developer Command Prompt" -ForegroundColor Yellow
$devCmd = "$vsPath\Common7\Tools\VsDevCmd.bat"
if (Test-Path $devCmd) {
    Write-Host "  OK - 已安装" -ForegroundColor Green
} else {
    Write-Host "  FAIL - 未安装" -ForegroundColor Red
    $allPassed = $false
}

# 总结
Write-Host "`n================================" -ForegroundColor Cyan
if ($allPassed) {
    Write-Host "SUCCESS - 所有依赖已就绪！" -ForegroundColor Green
    Write-Host "`n下一步:" -ForegroundColor Cyan
    Write-Host "  1. 配置 CMake PATH（管理员）" -ForegroundColor White
    Write-Host "  2. 打开 Developer Command Prompt" -ForegroundColor White
    Write-Host "  3. 运行: .\build.ps1" -ForegroundColor White
} else {
    Write-Host "FAIL - 缺少必需依赖" -ForegroundColor Red
}
Write-Host "================================" -ForegroundColor Cyan
