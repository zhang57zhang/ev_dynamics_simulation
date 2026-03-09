# ev_dynamics_simulation 依赖验证脚本

Write-Host "================================" -ForegroundColor Cyan
Write-Host "依赖验证 - ev_dynamics_simulation" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host "检查时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host ""

$allPassed = $true

# 1. Visual Studio 2026
Write-Host "[1/5] Visual Studio 2026" -ForegroundColor Yellow
$vsPath = "D:\Program Files\Microsoft Visual Studio\18\Community"
if (Test-Path $vsPath) {
    Write-Host "  ✅ 已安装" -ForegroundColor Green
    Write-Host "     路径: $vsPath" -ForegroundColor White
} else {
    Write-Host "  ❌ 未安装" -ForegroundColor Red
    $allPassed = $false
}

# 2. C++ 工作负载
Write-Host "`n[2/5] C++ 工作负载" -ForegroundColor Yellow
$vcPath = "$vsPath\VC"
$msvcPath = "$vsPath\VC\Tools\MSVC"
if ((Test-Path $vcPath) -and (Test-Path $msvcPath)) {
    Write-Host "  ✅ 已安装" -ForegroundColor Green
    $versions = Get-ChildItem $msvcPath -Directory -ErrorAction SilentlyContinue
    if ($versions) {
        Write-Host "     MSVC 版本: $($versions.Name -join ', ')" -ForegroundColor White
    }

    # 检查 cl.exe
    $clPath = Get-ChildItem -Path $msvcPath -Filter "cl.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($clPath) {
        Write-Host "  ✅ cl.exe 编译器" -ForegroundColor Green
    } else {
        Write-Host "  ⚠️ cl.exe 未找到（可能需要重启）" -ForegroundColor Yellow
    }
} else {
    Write-Host "  ❌ 未安装" -ForegroundColor Red
    Write-Host "  解决方案: 打开 Visual Studio Installer" -ForegroundColor Yellow
    Write-Host "           添加 'Desktop development with C++'" -ForegroundColor Yellow
    $allPassed = $false
}

# 3. CMake
Write-Host "`n[3/5] CMake" -ForegroundColor Yellow
$cmakePath = "D:\Program Files\CMake\bin\cmake.exe"
if (Test-Path $cmakePath) {
    $cmakeVersion = & $cmakePath --version 2>&1 | Select-String "cmake version"
    Write-Host "  ✅ 已安装" -ForegroundColor Green
    Write-Host "     $cmakeVersion" -ForegroundColor White

    # 检查 PATH
    $cmakeInPath = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmakeInPath) {
        Write-Host "  ✅ 已配置 PATH" -ForegroundColor Green
    } else {
        Write-Host "  ⚠️ 未配置 PATH（需要手动添加）" -ForegroundColor Yellow
        Write-Host "     运行: [Environment]::SetEnvironmentVariable('Path', `$env:Path + ';D:\Program Files\CMake\bin', 'Machine')" -ForegroundColor Cyan
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
    $versions = Get-ChildItem $sdkPath -Directory -ErrorAction SilentlyContinue | Where-Object { $_.Name -match "^\d+\." }
    if ($versions) {
        Write-Host "     版本: $($versions.Name -join ', ')" -ForegroundColor White
    }
} else {
    Write-Host "  ❌ 未安装" -ForegroundColor Red
    $allPassed = $false
}

# 5. Python（可选）
Write-Host "`n[5/5] Python（可选）" -ForegroundColor Yellow
try {
    $pythonVersion = python --version 2>&1
    Write-Host "  ✅ 已安装: $pythonVersion" -ForegroundColor Green

    $pybind = pip show pybind11 2>&1 | Select-String "Version"
    if ($pybind) {
        Write-Host "  ✅ pybind11: $pybind" -ForegroundColor Green
    } else {
        Write-Host "  ℹ️ pybind11 未安装（可选）" -ForegroundColor Yellow
    }
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
    Write-Host "`n请按照以下步骤修复:" -ForegroundColor Yellow
    Write-Host "  1. 打开 Visual Studio Installer" -ForegroundColor White
    Write-Host "  2. 找到 Visual Studio Community 2026" -ForegroundColor White
    Write-Host "  3. 点击 '修改'" -ForegroundColor White
    Write-Host "  4. 勾选 'Desktop development with C++'" -ForegroundColor White
    Write-Host "  5. 点击 '修改' 开始安装" -ForegroundColor White
    Write-Host "`n详细说明: DEPENDENCY_CHECK_20260308_2312.md" -ForegroundColor Cyan
}
Write-Host "================================" -ForegroundColor Cyan
