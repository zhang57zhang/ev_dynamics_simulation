# ev_dynamics_simulation 一键编译脚本

Write-Host "================================" -ForegroundColor Cyan
Write-Host "EV Dynamics Simulation 编译" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan

# 配置
$cmakePath = "D:\Program Files\CMake\bin\cmake.exe"
$buildDir = "build"
$srcDir = $PSScriptRoot

# Visual Studio 2026 Insiders 路径（包含完整 C++ 工具）
$vsPath = "D:\Program Files\Microsoft Visual Studio\18\Insiders"
$vsDisplayName = "Visual Studio 2026 Insiders"

# 检查 CMake
Write-Host "`n[0/5] 检查依赖..." -ForegroundColor Yellow
if (-not (Test-Path $cmakePath)) {
    Write-Host "❌ CMake 未找到: $cmakePath" -ForegroundColor Red
    exit 1
}
Write-Host "✅ CMake: $cmakePath" -ForegroundColor Green

# 检查 Visual Studio
$vcPath = "$vsPath\VC"
$msvcPath = "$vsPath\VC\Tools\MSVC"

# 检查 VS 基础安装
if (-not (Test-Path $vsPath)) {
    Write-Host "❌ $vsDisplayName 未安装" -ForegroundColor Red
    exit 1
}
Write-Host "✅ $vsDisplayName 基础: $vsPath" -ForegroundColor Green

# 检查 C++ 工作负载
if (-not (Test-Path $vcPath) -or -not (Test-Path $msvcPath)) {
    Write-Host "❌ C++ 工作负载未安装" -ForegroundColor Red
    Write-Host "请运行 Visual Studio Installer 并添加 'Desktop development with C++'" -ForegroundColor Yellow
    Write-Host "详见: DEPENDENCY_CHECK_20260308_2312.md" -ForegroundColor Yellow
    exit 1
}
Write-Host "✅ C++ 工作负载: 已安装" -ForegroundColor Green

# 清理旧的 Eigen3
Write-Host "`n[1/5] 清理旧的 Eigen3..." -ForegroundColor Yellow
Remove-Item -Recurse -Force "$srcDir\third_party\eigen3" -ErrorAction SilentlyContinue
Write-Host "✅ 已清理" -ForegroundColor Green

# 创建构建目录
Write-Host "`n[2/5] 创建构建目录..." -ForegroundColor Yellow
if (-not (Test-Path "$srcDir\$buildDir")) {
    New-Item -ItemType Directory -Path "$srcDir\$buildDir" | Out-Null
    Write-Host "✅ 已创建: $buildDir" -ForegroundColor Green
} else {
    Write-Host "✅ 已存在: $buildDir" -ForegroundColor Green
}
Set-Location "$srcDir\$buildDir"

# CMake 配置
Write-Host "`n[3/5] CMake 配置..." -ForegroundColor Yellow
Write-Host "命令: cmake .. -DBUILD_TESTING=OFF -DBUILD_PYTHON_BINDINGS=OFF" -ForegroundColor Cyan
& $cmakePath .. -DBUILD_TESTING=OFF -DBUILD_PYTHON_BINDINGS=OFF

if ($LASTEXITCODE -ne 0) {
    Write-Host "`n❌ CMake 配置失败" -ForegroundColor Red
    Write-Host "可能的原因:" -ForegroundColor Yellow
    Write-Host "  1. Visual Studio 未安装或不完整" -ForegroundColor Yellow
    Write-Host "  2. 未在 Developer Command Prompt 中运行" -ForegroundColor Yellow
    Write-Host "  3. 缺少依赖项" -ForegroundColor Yellow
    exit 1
}

Write-Host "`n✅ CMake 配置成功" -ForegroundColor Green

# 编译
Write-Host "`n[4/5] 编译项目 (Release 模式)..." -ForegroundColor Yellow
Write-Host "命令: cmake --build . --config Release" -ForegroundColor Cyan
& $cmakePath --build . --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "`n❌ 编译失败" -ForegroundColor Red
    Write-Host "请检查编译错误信息" -ForegroundColor Yellow
    exit 1
}

Write-Host "`n✅ 编译成功" -ForegroundColor Green

# 验证
Write-Host "`n[5/5] 验证编译结果..." -ForegroundColor Yellow
$libs = Get-ChildItem lib\*.lib -ErrorAction SilentlyContinue
$exes = Get-ChildItem bin\*.exe -ErrorAction SilentlyContinue

Write-Host "`n================================" -ForegroundColor Cyan
if ($libs -or $exes) {
    Write-Host "✅ 编译完成！" -ForegroundColor Green

    if ($libs) {
        Write-Host "`n生成的静态库 ($($libs.Count) 个):" -ForegroundColor Cyan
        $libs | ForEach-Object { Write-Host "  📦 $($_.Name) - $([math]::Round($_.Length/1KB, 2)) KB" }
    }

    if ($exes) {
        Write-Host "`n生成的可执行文件 ($($exes.Count) 个):" -ForegroundColor Cyan
        $exes | ForEach-Object { Write-Host "  ⚙️ $($_.Name) - $([math]::Round($_.Length/1KB, 2)) KB" }
    }

    Write-Host "`n编译产物位置:" -ForegroundColor Cyan
    Write-Host "  库文件: $srcDir\$buildDir\lib\" -ForegroundColor White
    Write-Host "  可执行文件: $srcDir\$buildDir\bin\" -ForegroundColor White
} else {
    Write-Host "⚠️ 编译完成但未找到产物" -ForegroundColor Yellow
    Write-Host "请检查 CMake 输出信息" -ForegroundColor Yellow
}

Write-Host "`n================================" -ForegroundColor Cyan
Write-Host "编译流程完成！" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Cyan

# 返回项目根目录
Set-Location $srcDir
