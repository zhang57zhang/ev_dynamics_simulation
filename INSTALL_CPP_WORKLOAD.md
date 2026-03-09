# 添加 C++ 工作负载指南

**创建时间**: 2026-03-08 22:58 GMT+8
**Visual Studio 路径**: D:\Program Files\Microsoft Visual Studio\18\Community

---

## ⚠️ 当前问题

**Visual Studio 已安装，但缺少 C++ 编译工具**

| 组件 | 状态 |
|------|------|
| Visual Studio 基础 | ✅ 已安装 |
| MSVC 编译器 | ❌ 未安装 |
| cl.exe | ❌ 未找到 |
| Developer Command Prompt | ❌ 不存在 |

---

## ✅ 解决步骤

### 步骤 1: 打开 Visual Studio Installer

**方式 A: 命令行**
```powershell
Start-Process "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe"
```

**方式 B: 开始菜单**
1. 按 `Win` 键
2. 搜索 "Visual Studio Installer"
3. 打开它

---

### 步骤 2: 修改安装

1. 找到 **"Visual Studio Community 2022"**
2. 点击 **"修改"** 按钮
3. 在 **"工作负载"** 标签页中，勾选：
   ```
   ☑ 使用 C++ 的桌面开发 (Desktop development with C++)
   ```
4. 确认包含以下组件（自动勾选）：
   - MSVC v143 - VS 2022 C++ x64/x86 生成工具
   - Windows 10 SDK
   - C++ 核心功能
5. 点击 **"修改"** 按钮
6. 等待下载和安装（5-10 分钟，约 8-10 GB）

---

### 步骤 3: 验证安装

安装完成后，运行以下 PowerShell 脚本验证：

```powershell
# 验证脚本
Write-Host "=== 验证 C++ 工作负载安装 ===" -ForegroundColor Cyan

# 1. 检查 MSVC 路径
$msvcPath = "D:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC"
if (Test-Path $msvcPath) {
    Write-Host "✅ MSVC 编译器已安装" -ForegroundColor Green
    $versions = Get-ChildItem $msvcPath -Directory
    Write-Host "   版本: $($versions.Name -join ', ')" -ForegroundColor White
} else {
    Write-Host "❌ MSVC 编译器未安装" -ForegroundColor Red
    exit 1
}

# 2. 检查 cl.exe
$clPath = Get-ChildItem -Path $msvcPath -Filter "cl.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
if ($clPath) {
    Write-Host "✅ cl.exe 编译器已安装" -ForegroundColor Green
    Write-Host "   路径: $($clPath.FullName)" -ForegroundColor White
} else {
    Write-Host "❌ cl.exe 编译器未找到" -ForegroundColor Red
    exit 1
}

# 3. 检查 Developer Command Prompt
$devCmd = "D:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
if (Test-Path $devCmd) {
    Write-Host "✅ Developer Command Prompt 已安装" -ForegroundColor Green
    Write-Host "   路径: $devCmd" -ForegroundColor White
} else {
    Write-Host "❌ Developer Command Prompt 未找到" -ForegroundColor Red
    exit 1
}

Write-Host "`n=== 安装验证成功 ===" -ForegroundColor Green
Write-Host "现在可以进行编译了！" -ForegroundColor Cyan
```

---

## 📋 验证成功的标志

运行验证脚本后，应该看到：

```
=== 验证 C++ 工作负载安装 ===
✅ MSVC 编译器已安装
   版本: 14.xx.xxxxx
✅ cl.exe 编译器已安装
   路径: D:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.xx.xxxxx\bin\Hostx64\x64\cl.exe
✅ Developer Command Prompt 已安装
   路径: D:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat

=== 安装验证成功 ===
现在可以进行编译了！
```

---

## 🚀 安装完成后的下一步

### 1. 配置 CMake PATH

```powershell
# 以管理员身份运行
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";D:\Program Files\CMake\bin", "Machine")
```

### 2. 打开 Developer Command Prompt

1. 按 `Win` 键
2. 搜索 "Developer Command Prompt"
3. 应该能看到 **"Developer Command Prompt for VS 2022"**

### 3. 运行编译脚本

```powershell
cd E:\workspace\ev_dynamics_simulation
.\build.ps1
```

---

## ⏱️ 预计时间

| 任务 | 时间 |
|------|------|
| 打开 Visual Studio Installer | 1 分钟 |
| 选择并安装 C++ 工作负载 | 5-10 分钟 |
| 验证安装 | 1 分钟 |
| 配置 CMake PATH | 1 分钟 |
| 运行编译脚本 | 5-10 分钟 |
| **总计** | **13-23 分钟** |

---

## 🔍 常见问题

### Q: 为什么需要 C++ 工作负载？

A: Visual Studio 基础安装不包含 C++ 编译器。必须安装 "Desktop development with C++" 工作负载才能编译 C++ 项目。

### Q: 安装失败怎么办？

A:
1. 检查网络连接
2. 以管理员身份运行 Visual Studio Installer
3. 尝试先卸载再重新安装

### Q: 磁盘空间不足？

A:
- C++ 工作负载需要约 8-10 GB
- 可以选择安装到其他盘符（在 Installer 中修改安装位置）

---

## 📞 需要帮助？

**完成 C++ 工作负载安装后，告诉我，我会继续指导编译流程。**

**遇到任何问题，请复制错误信息给我。**
