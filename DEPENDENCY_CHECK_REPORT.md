# ev_dynamics_simulation 依赖检查报告

**检查时间**: 2026-03-08 22:51 GMT+8
**检查者**: CodeCraft

---

## 📊 依赖状态总览

| 依赖项 | 状态 | 版本/路径 | 备注 |
|--------|------|-----------|------|
| **Visual Studio 2022** | ❌ 未安装 | - | **必需** - MSVC 编译器 |
| **Windows SDK** | ❌ 未安装 | - | **必需** - Windows 开发库 |
| **CMake** | ⚠️ 已安装但未配置 | 3.31（用户报告） | **必需** - 不在 PATH 中 |
| **Python** | ✅ 已安装 | 3.14.3 | 可选 - Python 绑定 |
| **pybind11** | ✅ 已安装 | 3.0.2 | 可选 - Python 绑定 |
| **Eigen3** | ⚠️ 部分存在 | third_party/eigen3 | **必需** - 头文件缺失 |
| **Google Test** | ℹ️ 未安装 | - | 可选 - 单元测试 |

---

## 🔴 阻塞性问题（必须解决）

### 1. Visual Studio 2022 未安装 ❌

**问题**: 缺少 MSVC C++ 编译器

**影响**: 无法编译 C++ 代码

**解决方案**:
```
下载: https://visualstudio.microsoft.com/downloads/
选择: Visual Studio 2022 Community（免费）
工作负载: Desktop development with C++
```

**预计时间**: 20-30 分钟
**磁盘空间**: ~20 GB

---

### 2. CMake 未配置到 PATH ⚠️

**问题**: CMake 3.31 已安装但不在系统 PATH 中

**影响**: 无法在命令行使用 cmake 命令

**解决方案**:

**方式 A: 重新安装并添加到 PATH**
```
1. 运行 CMake 安装程序
2. 选择 "Add CMake to the system PATH for all users"
3. 重启 PowerShell
```

**方式 B: 手动添加到 PATH**
```powershell
# 找到 CMake 安装路径（通常是以下之一）
C:\Program Files\CMake\bin
C:\Program Files (x86)\CMake\bin

# 添加到系统 PATH（管理员权限）
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\Program Files\CMake\bin", "Machine")
```

**方式 C: 使用完整路径（临时方案）**
```powershell
# 在编译时使用完整路径
& "C:\Program Files\CMake\bin\cmake.exe" --version
```

---

### 3. Eigen3 头文件缺失 ⚠️

**问题**: `third_party/eigen3/Eigen` 目录存在但无头文件

**影响**: 编译时找不到 Eigen 库

**解决方案**:

**方式 A: 让 CMake 自动下载（推荐）**
```powershell
# 删除不完整的 Eigen3
Remove-Item -Recurse -Force E:\workspace\ev_dynamics_simulation\third_party\eigen3

# CMake 会自动下载 Eigen3 3.4.0
cd E:\workspace\ev_dynamics_simulation\build
cmake ..
```

**方式 B: 手动下载**
```powershell
# 下载 Eigen3 3.4.0
cd E:\workspace\ev_dynamics_simulation\third_party
git clone https://gitlab.com/libeigen/eigen.git eigen3
cd eigen3
git checkout 3.4.0
```

---

## ⚠️ 可选依赖（不影响核心编译）

### 1. Google Test 未安装 ℹ️

**影响**: 无法运行单元测试

**解决方案**: 禁用测试
```powershell
cmake .. -DBUILD_TESTING=OFF
```

---

### 2. Python 版本过新 ⚠️

**当前**: Python 3.14.3
**推荐**: Python 3.10-3.12

**影响**: pybind11 3.0.2 已支持 Python 3.14，但可能有兼容性问题

**建议**: 先尝试编译，如果失败则降级到 Python 3.12

---

## ✅ 已满足的依赖

### 1. Python 环境 ✅
- Python 3.14.3 已安装
- pip 26.0.1 已安装

### 2. pybind11 ✅
- 版本 3.0.2 已安装
- 支持 Python 绑定（如果需要）

---

## 🎯 完整安装流程

### 步骤 1: 安装 Visual Studio 2022（必需）

```
1. 访问: https://visualstudio.microsoft.com/downloads/
2. 下载: Visual Studio 2022 Community
3. 安装时选择: Desktop development with C++
4. 确认包含:
   - MSVC v143 编译器
   - Windows 10 SDK
   - CMake 工具（可选）
```

### 步骤 2: 配置 CMake PATH

```powershell
# 找到 CMake 安装路径
# 假设在 C:\Program Files\CMake\bin

# 添加到 PATH（管理员权限）
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\Program Files\CMake\bin", "Machine")

# 重启 PowerShell 并验证
cmake --version
# 应显示: cmake version 3.31.x
```

### 步骤 3: 修复 Eigen3

```powershell
# 删除不完整的 Eigen3
cd E:\workspace\ev_dynamics_simulation
Remove-Item -Recurse -Force third_party\eigen3

# 让 CMake 自动下载
mkdir build -Force
cd build
cmake .. -DBUILD_TESTING=OFF -DBUILD_PYTHON_BINDINGS=OFF
```

### 步骤 4: 编译项目

```powershell
# 打开 Developer Command Prompt for VS 2022
# 或在 PowerShell 中运行:
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

# 编译
cmake --build . --config Release

# 测试（如果启用了测试）
ctest -C Release
```

---

## 📋 最小化安装方案（快速编译）

如果只需要核心功能，不需要测试和 Python 绑定：

```powershell
# 1. 安装 Visual Studio 2022（必需）
# 2. 配置 CMake PATH（必需）
# 3. 编译（禁用测试和 Python 绑定）

cd E:\workspace\ev_dynamics_simulation
Remove-Item -Recurse -Force third_party\eigen3
mkdir build -Force
cd build

# 在 Developer Command Prompt 中运行
cmake .. -DBUILD_TESTING=OFF -DBUILD_PYTHON_BINDINGS=OFF
cmake --build . --config Release
```

**预计时间**: 30-40 分钟（主要是安装 VS）

---

## 🔍 验证安装

安装完成后，运行以下命令验证：

```powershell
# 1. 验证 Visual Studio
cl
# 应显示: Microsoft (R) C/C++ Optimizing Compiler Version 19.xx

# 2. 验证 CMake
cmake --version
# 应显示: cmake version 3.31.x

# 3. 验证编译
cd E:\workspace\ev_dynamics_simulation\build
cmake --build . --config Release
# 应成功编译，无错误

# 4. 检查编译产物
Get-ChildItem lib/*.lib
# 应显示编译生成的静态库
```

---

## 📊 预计时间表

| 任务 | 时间 | 状态 |
|------|------|------|
| 安装 Visual Studio 2022 | 20-30 分钟 | ⏳ 待完成 |
| 配置 CMake PATH | 2 分钟 | ⏳ 待完成 |
| 修复 Eigen3 | 2-5 分钟 | ⏳ 待完成 |
| CMake 配置 | 2-5 分钟 | ⏳ 待完成 |
| 编译项目 | 5-10 分钟 | ⏳ 待完成 |
| **总计** | **30-50 分钟** | - |

---

## 🚨 常见问题

### Q: CMake 已安装但找不到命令？

A: CMake 未添加到 PATH。解决方案：
```powershell
# 找到 CMake 安装位置
Get-ChildItem "C:\Program Files" -Filter cmake.exe -Recurse -ErrorAction SilentlyContinue

# 添加到 PATH
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";<CMake路径>", "Machine")
```

### Q: Visual Studio 安装太慢？

A: 可以选择最小安装：
- 只选择 "Desktop development with C++"
- 取消其他组件
- 预计 8-10 GB（而不是 20 GB）

### Q: 必须安装 Google Test 吗？

A: 不必须。禁用测试即可：
```powershell
cmake .. -DBUILD_TESTING=OFF
```

### Q: Python 3.14 能用吗？

A: pybind11 3.0.2 已支持，建议先尝试编译。如果失败，降级到 Python 3.12。

---

## 📞 下一步

**推荐方案**:
1. ✅ 安装 Visual Studio 2022（必需）
2. ✅ 配置 CMake PATH（必需）
3. ✅ 让 CMake 自动下载 Eigen3（推荐）
4. ✅ 禁用测试和 Python 绑定（快速编译）

**预计完成时间**: 30-40 分钟

**需要协助吗？我可以逐步指导你完成每个步骤。**
