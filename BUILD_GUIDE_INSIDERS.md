# ev_dynamics_simulation 编译指南 - 使用 VS Insiders

**创建时间**: 2026-03-08 23:20 GMT+8
**Visual Studio**: 2026 Insiders (18.3)
**MSVC 版本**: 14.50.35717

---

## ✅ 依赖状态（全部就绪！）

| 依赖项 | 状态 | 详情 |
|--------|------|------|
| **Visual Studio 2026 Insiders** | ✅ 已安装 | D:\Program Files\Microsoft Visual Studio\18\Insiders |
| **MSVC 编译器** | ✅ 已安装 | 14.50.35717 |
| **cl.exe (x64)** | ✅ 已找到 | VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64\cl.exe |
| **cl.exe (x86)** | ✅ 已找到 | VC\Tools\MSVC\14.50.35717\bin\Hostx64\x86\cl.exe |
| **Developer Command Prompt** | ✅ 已安装 | VsDevCmd.bat |
| **CMake** | ✅ 已安装 | 3.31.11 @ D:\Program Files\CMake\bin |
| **Windows SDK** | ✅ 已安装 | Windows Kits\10 |
| **Python** | ✅ 已安装 | 3.14.3 + pybind11 3.0.2 |

---

## 🚀 开始编译（2 步）

### **步骤 1: 配置 CMake PATH**（1 分钟）

**以管理员身份运行 PowerShell**，然后执行：

```powershell
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";D:\Program Files\CMake\bin", "Machine")
```

**关闭并重新打开 PowerShell**，验证：

```powershell
cmake --version
# 应显示: cmake version 3.31.11
```

---

### **步骤 2: 运行编译脚本**（5-10 分钟）

**打开 Developer Command Prompt for VS 2026**：

1. 按 `Win` 键
2. 搜索 **"Developer Command Prompt"**
3. 应该能看到 **"Developer Command Prompt for VS 2026"** 或 **"Developer PowerShell for VS 2026"**
4. 打开它

**运行编译脚本**：

```powershell
cd E:\workspace\ev_dynamics_simulation
.\build.ps1
```

**或者手动编译**：

```powershell
# 1. 进入项目目录
cd E:\workspace\ev_dynamics_simulation

# 2. 清理旧的 Eigen3（如果需要）
Remove-Item -Recurse -Force third_party\eigen3 -ErrorAction SilentlyContinue

# 3. 创建构建目录
mkdir build -Force
cd build

# 4. 配置 CMake（禁用测试和 Python 绑定，快速验证）
& "D:\Program Files\CMake\bin\cmake.exe" .. -DBUILD_TESTING=OFF -DBUILD_PYTHON_BINDINGS=OFF

# 5. 编译
& "D:\Program Files\CMake\bin\cmake.exe" --build . --config Release

# 6. 检查编译产物
Get-ChildItem lib\*.lib
Get-ChildItem bin\*.exe
```

---

## 📋 一键编译脚本

`build.ps1` 已更新为使用 VS Insiders，运行：

```powershell
cd E:\workspace\ev_dynamics_simulation
.\build.ps1
```

**脚本会自动**：
- ✅ 检查 Visual Studio 2026 Insiders
- ✅ 检查 C++ 工作负载（已确认存在）
- ✅ 检查 CMake
- ✅ 清理并配置 Eigen3（自动下载）
- ✅ 编译项目（Release 模式）
- ✅ 验证编译结果

---

## 🔧 如果找不到 Developer Command Prompt

**手动配置编译环境**：

```powershell
# 在普通 PowerShell 中运行
$vsPath = "D:\Program Files\Microsoft Visual Studio\18\Insiders"
$devCmd = "$vsPath\Common7\Tools\VsDevCmd.bat"

# 配置编译环境
cmd /c "`"$devCmd`" && cd /d E:\workspace\ev_dynamics_simulation\build && D:\Progra~1\CMake\bin\cmake.exe .. -DBUILD_TESTING=OFF && D:\Progra~1\CMake\bin\cmake.exe --build . --config Release"
```

---

## 📊 预计编译时间

| 任务 | 时间 |
|------|------|
| 配置 CMake PATH | 1 分钟 |
| 运行编译脚本 | 5-10 分钟 |
| **总计** | **6-11 分钟** |

---

## ✅ 编译成功的标志

```
✅ lib\*.lib 文件存在（静态库）
✅ bin\*.exe 文件存在（测试程序，如果启用测试）
✅ 无编译错误
✅ 无链接错误
```

---

## 🚨 故障排查

### 问题 1: CMake 找不到

**解决方案**: 使用完整路径
```powershell
& "D:\Program Files\CMake\bin\cmake.exe" --version
```

### 问题 2: cl.exe 找不到

**解决方案**: 确保在 Developer Command Prompt 中运行
- 搜索并打开 **"Developer Command Prompt for VS 2026"**
- 不要使用普通的 PowerShell 或 CMD

### 问题 3: Eigen3 下载失败

**解决方案**: 手动下载
```powershell
cd E:\workspace\ev_dynamics_simulation\third_party
git clone https://gitlab.com/libeigen/eigen.git eigen3
cd eigen3
git checkout 3.4.0
```

### 问题 4: 找不到 Developer Command Prompt

**解决方案**: 手动配置环境
```powershell
$env:PATH = "D:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64;" + $env:PATH
$env:INCLUDE = "D:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Tools\MSVC\14.50.35717\include;" + $env:INCLUDE
$env:LIB = "D:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Tools\MSVC\14.50.35717\lib\x64;" + $env:LIB
```

---

## 🎯 下一步

**立即执行**:

1. ✅ 配置 CMake PATH
2. ✅ 打开 Developer Command Prompt for VS 2026
3. ✅ 运行 `.\build.ps1`

**预计完成时间**: 6-11 分钟

---

## 📞 需要帮助？

**遇到任何问题，请复制错误信息给我。**
