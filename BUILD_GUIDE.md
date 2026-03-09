# ev_dynamics_simulation 编译指南

**创建时间**: 2026-03-08 22:55 GMT+8
**状态**: 准备编译

---

## ✅ 已确认的依赖

| 依赖项 | 状态 | 版本/路径 |
|--------|------|-----------|
| **CMake** | ✅ 已安装 | 3.31.11 @ D:\Program Files\CMake\bin |
| **Windows SDK** | ✅ 已安装 | Windows Kits\10 |

---

## 🔧 立即执行的步骤

### 步骤 1: 配置 CMake PATH（必需）

**方式 A: 以管理员身份运行（推荐）**

1. 右键点击 PowerShell
2. 选择"以管理员身份运行"
3. 执行以下命令：

```powershell
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";D:\Program Files\CMake\bin", "Machine")
```

4. 关闭并重新打开 PowerShell
5. 验证：

```powershell
cmake --version
# 应显示: cmake version 3.31.11
```

**方式 B: 使用完整路径（临时方案）**

```powershell
# 每次使用 CMake 时指定完整路径
& "D:\Program Files\CMake\bin\cmake.exe" --version
```

---

### 步骤 2: 验证 Visual Studio 安装

**打开 Developer Command Prompt for VS 2022**

1. 按 `Win` 键
2. 搜索 "Developer Command Prompt"
3. 打开 "Developer Command Prompt for VS 2022"

**验证编译器**:

```powershell
cl
# 应显示: Microsoft (R) C/C++ Optimizing Compiler Version 19.xx
```

**如果找不到**:
- 可能是 Visual Studio 安装不完整
- 重新运行 VS 安装程序
- 确保勾选了 "Desktop development with C++"

---

### 步骤 3: 修复 Eigen3（自动下载）

```powershell
cd E:\workspace\ev_dynamics_simulation

# 删除不完整的 Eigen3
Remove-Item -Recurse -Force third_party\eigen3 -ErrorAction SilentlyContinue

# 创建构建目录
mkdir build -Force
cd build
```

---

### 步骤 4: 配置 CMake

**在 Developer Command Prompt for VS 2022 中执行**:

```powershell
cd E:\workspace\ev_dynamics_simulation\build

# 方式 A: 完整编译（包含测试和 Python 绑定）
cmake .. -DBUILD_TESTING=ON -DBUILD_PYTHON_BINDINGS=ON

# 方式 B: 最小编译（仅核心库，推荐快速验证）
cmake .. -DBUILD_TESTING=OFF -DBUILD_PYTHON_BINDINGS=OFF
```

**如果 CMake 未在 PATH 中**:

```powershell
& "D:\Program Files\CMake\bin\cmake.exe" .. -DBUILD_TESTING=OFF -DBUILD_PYTHON_BINDINGS=OFF
```

---

### 步骤 5: 编译项目

```powershell
# 编译（Release 模式）
cmake --build . --config Release

# 或使用完整路径
& "D:\Program Files\CMake\bin\cmake.exe" --build . --config Release
```

**预计编译时间**: 5-10 分钟

---

### 步骤 6: 验证编译结果

```powershell
# 检查编译产物
Get-ChildItem lib\*.lib
Get-ChildItem bin\*.exe

# 运行测试（如果启用了测试）
ctest -C Release
```

---

## 🚀 一键编译脚本

创建以下脚本文件以简化流程：

### `build.ps1`（保存到项目根目录）

```powershell
# ev_dynamics_simulation 一键编译脚本

Write-Host "================================" -ForegroundColor Cyan
Write-Host "EV Dynamics Simulation 编译" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan

# 配置
$cmakePath = "D:\Program Files\CMake\bin\cmake.exe"
$buildDir = "build"
$srcDir = $PSScriptRoot

# 清理旧的 Eigen3
Write-Host "`n[1/5] 清理旧的 Eigen3..." -ForegroundColor Yellow
Remove-Item -Recurse -Force "$srcDir\third_party\eigen3" -ErrorAction SilentlyContinue

# 创建构建目录
Write-Host "`n[2/5] 创建构建目录..." -ForegroundColor Yellow
if (-not (Test-Path "$srcDir\$buildDir")) {
    New-Item -ItemType Directory -Path "$srcDir\$buildDir" | Out-Null
}
Set-Location "$srcDir\$buildDir"

# CMake 配置
Write-Host "`n[3/5] CMake 配置..." -ForegroundColor Yellow
& $cmakePath .. -DBUILD_TESTING=OFF -DBUILD_PYTHON_BINDINGS=OFF

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ CMake 配置失败" -ForegroundColor Red
    exit 1
}

# 编译
Write-Host "`n[4/5] 编译项目..." -ForegroundColor Yellow
& $cmakePath --build . --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ 编译失败" -ForegroundColor Red
    exit 1
}

# 验证
Write-Host "`n[5/5] 验证编译结果..." -ForegroundColor Yellow
$libs = Get-ChildItem lib\*.lib -ErrorAction SilentlyContinue
if ($libs) {
    Write-Host "✅ 编译成功！" -ForegroundColor Green
    Write-Host "`n生成的库文件:" -ForegroundColor Cyan
    $libs | ForEach-Object { Write-Host "  $($_.Name) - $([math]::Round($_.Length/1KB, 2)) KB" }
} else {
    Write-Host "⚠️ 未找到编译产物" -ForegroundColor Yellow
}

Write-Host "`n================================" -ForegroundColor Cyan
Write-Host "编译完成！" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Cyan
```

### 使用方法:

```powershell
# 在 Developer Command Prompt for VS 2022 中
cd E:\workspace\ev_dynamics_simulation
.\build.ps1
```

---

## 📋 故障排查

### 问题 1: CMake 找不到

**解决方案**: 使用完整路径
```powershell
& "D:\Program Files\CMake\bin\cmake.exe" --version
```

### 问题 2: cl.exe 找不到

**解决方案**: 确保在 Developer Command Prompt 中运行
- 搜索并打开 "Developer Command Prompt for VS 2022"
- 不要使用普通的 PowerShell 或 CMD

### 问题 3: Eigen3 下载失败

**解决方案**: 手动下载
```powershell
cd E:\workspace\ev_dynamics_simulation\third_party
git clone https://gitlab.com/libeigen/eigen.git eigen3
cd eigen3
git checkout 3.4.0
```

### 问题 4: Python 绑定编译失败

**解决方案**: 禁用 Python 绑定
```powershell
cmake .. -DBUILD_PYTHON_BINDINGS=OFF
```

---

## 📊 预计时间

| 任务 | 时间 |
|------|------|
| 配置 CMake PATH | 2 分钟 |
| 验证 VS 安装 | 1 分钟 |
| CMake 配置 | 2-5 分钟 |
| 编译项目 | 5-10 分钟 |
| **总计** | **10-18 分钟** |

---

## ✅ 编译成功的标志

```
✅ lib\*.lib 文件存在（静态库）
✅ bin\*.exe 文件存在（测试程序，如果启用测试）
✅ 无编译错误
✅ 无链接错误
```

---

## 📞 下一步

**立即执行**:

1. 以管理员身份运行 PowerShell
2. 配置 CMake PATH
3. 打开 Developer Command Prompt for VS 2022
4. 运行编译脚本

**需要帮助？** 请告诉我遇到的具体错误信息。
