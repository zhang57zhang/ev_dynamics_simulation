# ev_dynamics_simulation 依赖检查报告

**检查时间**: 2026-03-08 23:12 GMT+8
**检查者**: CodeCraft
**Visual Studio 版本**: 2026 (18.3.11520.95)

---

## 📊 依赖状态总览

| 依赖项 | 状态 | 版本/路径 | 备注 |
|--------|------|-----------|------|
| **Visual Studio 2026** | ✅ 已安装 | 18.3.11520.95 @ D:\Program Files\Microsoft Visual Studio\18\Community | 基础已安装 |
| **C++ 工作负载** | ❌ 未安装 | - | **缺少** - 需要添加 |
| **MSVC 编译器** | ❌ 未安装 | - | **缺少** - VC 目录不存在 |
| **cl.exe** | ❌ 未找到 | - | **缺少** - 无法编译 C++ |
| **Developer Command Prompt** | ✅ 已安装 | VsDevCmd.bat 存在 | 但缺少编译器 |
| **CMake** | ✅ 已安装 | 3.31.11 @ D:\Program Files\CMake\bin | **需要配置 PATH** |
| **Windows SDK** | ✅ 已安装 | Windows Kits\10 | 已就绪 |
| **Python** | ✅ 已安装 | 3.14.3 | 可选 - Python 绑定 |
| **pybind11** | ✅ 已安装 | 3.0.2 | 可选 - Python 绑定 |
| **Eigen3** | ⚠️ 需配置 | CMake 可自动下载 | 自动处理 |

---

## 🔴 关键问题

### **Visual Studio 2026 已安装，但缺少 C++ 工作负载**

**诊断结果**:
- ✅ Visual Studio Community 2026 (18.3) 已安装
- ❌ VC 目录不存在
- ❌ MSVC 编译器未安装
- ❌ cl.exe 未找到

**影响**: 无法编译 C++ 代码

---

## ✅ 解决方案

### **添加 C++ 工作负载**（5-10 分钟）

#### **步骤 1: 打开 Visual Studio Installer**

```powershell
Start-Process "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe"
```

或：
1. 按 `Win` 键
2. 搜索 "Visual Studio Installer"
3. 打开它

---

#### **步骤 2: 修改安装**

1. 找到 **"Visual Studio Community 2026"**
2. 点击 **"修改"** 按钮
3. 在 **"工作负载"** 标签页中，勾选：
   ```
   ☑ 使用 C++ 的桌面开发 (Desktop development with C++)
   ```
4. 确认包含以下组件（自动勾选）：
   - MSVC v143 - VS 2026 C++ x64/x86 生成工具
   - Windows 10/11 SDK
   - C++ 核心功能
5. 点击 **"修改"** 按钮
6. 等待下载和安装（5-10 分钟，约 8-10 GB）

---

#### **步骤 3: 验证安装**

安装完成后，运行以下 PowerShell 脚本验证：

```powershell
# 验证脚本
Write-Host "=== 验证 C++ 工作负载安装 ===" -ForegroundColor Cyan

# 1. 检查 VC 目录
$vcPath = "D:\Program Files\Microsoft Visual Studio\18\Community\VC"
if (Test-Path $vcPath) {
    Write-Host "✅ VC 目录已创建" -ForegroundColor Green
    Get-ChildItem $vcPath -Directory | Select-Object Name
} else {
    Write-Host "❌ VC 目录不存在" -ForegroundColor Red
    exit 1
}

# 2. 检查 MSVC 编译器
$msvcPath = "$vcPath\Tools\MSVC"
if (Test-Path $msvcPath) {
    Write-Host "✅ MSVC 编译器已安装" -ForegroundColor Green
    $versions = Get-ChildItem $msvcPath -Directory
    Write-Host "   版本: $($versions.Name -join ', ')" -ForegroundColor White
} else {
    Write-Host "❌ MSVC 编译器未安装" -ForegroundColor Red
    exit 1
}

# 3. 检查 cl.exe
$clPath = Get-ChildItem -Path $msvcPath -Filter "cl.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
if ($clPath) {
    Write-Host "✅ cl.exe 编译器已安装" -ForegroundColor Green
    Write-Host "   路径: $($clPath.FullName)" -ForegroundColor White
} else {
    Write-Host "❌ cl.exe 编译器未找到" -ForegroundColor Red
    exit 1
}

Write-Host "`n=== 安装验证成功 ===" -ForegroundColor Green
Write-Host "现在可以进行编译了！" -ForegroundColor Cyan
```

---

## 🔧 其他需要配置的依赖

### **配置 CMake PATH**（1 分钟）

CMake 已安装但未在 PATH 中，需要手动添加：

```powershell
# 以管理员身份运行 PowerShell
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";D:\Program Files\CMake\bin", "Machine")

# 重启 PowerShell 后验证
cmake --version
# 应显示: cmake version 3.31.11
```

---

## 📋 完整安装流程（11-21 分钟）

| 任务 | 时间 | 状态 |
|------|------|------|
| 添加 C++ 工作负载 | 5-10 分钟 | ⏳ 待完成 |
| 配置 CMake PATH | 1 分钟 | ⏳ 待完成 |
| 验证安装 | 1 分钟 | ⏳ 待完成 |
| 运行编译脚本 | 5-10 分钟 | ⏳ 待完成 |
| **总计** | **12-22 分钟** | - |

---

## 🚀 安装完成后的编译流程

### **步骤 1: 配置 CMake PATH**

```powershell
# 以管理员身份运行
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";D:\Program Files\CMake\bin", "Machine")

# 重启 PowerShell
```

---

### **步骤 2: 打开 Developer Command Prompt**

1. 按 `Win` 键
2. 搜索 **"Developer Command Prompt"**
3. 应该能看到 **"Developer Command Prompt for VS 2026"**

**验证编译器**:

```powershell
cl
# 应显示: Microsoft (R) C/C++ Optimizing Compiler Version 19.xx
```

---

### **步骤 3: 运行编译脚本**

```powershell
cd E:\workspace\ev_dynamics_simulation
.\build.ps1
```

**脚本会自动**：
- ✅ 检查 Visual Studio C++ 工作负载
- ✅ 检查 CMake
- ✅ 清理并配置 Eigen3（自动下载）
- ✅ 编译项目（Release 模式）
- ✅ 验证编译结果

---

## 📊 依赖清单总结

### ✅ 已就绪

- ✅ Visual Studio 2026 基础安装
- ✅ Developer Command Prompt
- ✅ CMake 3.31.11
- ✅ Windows SDK
- ✅ Python 3.14.3
- ✅ pybind11 3.0.2

### ❌ 缺少（阻塞编译）

- ❌ **C++ 工作负载**（MSVC 编译器）

### ⚠️ 需要配置

- ⚠️ CMake PATH（需手动添加）

---

## 🎯 下一步行动

**立即执行**:

1. ✅ 打开 Visual Studio Installer
2. ✅ 添加 "Desktop development with C++" 工作负载
3. ✅ 配置 CMake PATH
4. ✅ 运行编译脚本

**预计完成时间**: 12-22 分钟

---

## 📞 需要帮助？

**完成 C++ 工作负载安装后，告诉我，我会帮你验证并开始编译！**

**遇到任何问题，请复制错误信息给我。**
