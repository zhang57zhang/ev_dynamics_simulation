# Windows 10 用户使用指南

**六自由度新能源车辆动力学仿真系统 - Windows 10 专属指南**

---

## 📋 目录

1. [系统要求](#系统要求)
2. [环境准备](#环境准备)
3. [安装步骤](#安装步骤)
4. [构建项目](#构建项目)
5. [运行测试](#运行测试)
6. [使用示例](#使用示例)
7. [IDE配置](#ide配置)
8. [故障排查](#故障排查)
9. [性能优化](#性能优化)
10. [常见问题](#常见问题)

---

## 🖥️ 系统要求

### 操作系统
- **版本:** Windows 10 版本 1809 或更高（推荐 21H2+）
- **架构:** x64（64位）
- **内存:** 最低 8GB，推荐 16GB
- **存储:** 至少 5GB 可用空间

### 必需软件
| 软件 | 最低版本 | 推荐版本 | 用途 |
|------|---------|---------|------|
| Visual Studio | 2019 | 2022 Community | C++编译器 |
| CMake | 3.16 | 3.25+ | 构建系统 |
| Python | 3.8 | 3.10+ | Python绑定 |
| Git | 2.20 | 最新版 | 版本控制 |

### 硬件建议
- **CPU:** 4核心以上（推荐 Intel i7/i9 或 AMD Ryzen 7/9）
- **内存:** 16GB DDR4 或更高
- **硬盘:** SSD（提升编译速度）
- **显卡:** 不限（仿真不依赖GPU）

---

## 🛠️ 环境准备

### 步骤 1: 安装 Visual Studio 2022

#### 1.1 下载和安装

1. **下载 Visual Studio 2022 Community**（免费版）
   - 访问: https://visualstudio.microsoft.com/downloads/
   - 选择 "Visual Studio 2022 Community"
   - 下载并运行安装程序

2. **选择工作负载**
   ```
   必需工作负载:
   ✓ 使用 C++ 的桌面开发 (Desktop development with C++)
   
   推荐额外组件:
   ✓ CMake 工具
   ✓ Windows 10 SDK
   ✓ C++ Clang 工具（可选）
   ```

3. **安装位置**
   - 建议安装在 SSD 上
   - 需要约 20GB 空间

#### 1.2 验证安装

```powershell
# 打开 Developer Command Prompt for VS 2022
# 或在 PowerShell 中运行:
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

# 检查编译器
cl
# 应显示: Microsoft (R) C/C++ Optimizing Compiler Version 19.xx
```

### 步骤 2: 安装 CMake

#### 方式 A: MSI 安装（推荐）

1. **下载 CMake 3.16+**
   ```
   官方下载: https://cmake.org/download/
   选择: cmake-3.25.x-windows-x86_64.msi
   ```

2. **安装步骤**
   - 双击 MSI 文件
   - 选择 "Add CMake to the system PATH for all users"
   - 完成安装

3. **验证安装**
   ```powershell
   cmake --version
   # 应显示: cmake version 3.25.x
   ```

#### 方式 B: 便携版

```powershell
# 下载 ZIP 版本
# 解压到: C:\Program Files\CMake

# 手动添加到 PATH
[Environment]::SetEnvironmentVariable(
    "Path",
    $env:Path + ";C:\Program Files\CMake\bin",
    "Machine"
)

# 重启 PowerShell 后验证
cmake --version
```

### 步骤 3: 安装 Python 3.10+

#### 3.1 下载和安装

1. **下载 Python**
   ```
   官方网站: https://www.python.org/downloads/
   选择: Python 3.10.x 或 3.11.x
   ```

2. **安装选项**
   ```
   ✓ Install launcher for all users (recommended)
   ✓ Add Python to PATH
   ✓ Install pip
   ✓ Install for all users
   ```

3. **验证安装**
   ```powershell
   python --version
   # Python 3.10.x
   
   pip --version
   # pip 23.x from ...
   ```

#### 3.2 安装 Python 依赖

```powershell
# 安装 pybind11（用于 C++ 绑定）
pip install pybind11

# 安装测试工具
pip install pytest numpy

# 验证 pybind11
python -c "import pybind11; print(pybind11.__version__)"
```

### 步骤 4: 安装 vcpkg（可选，用于依赖管理）

```powershell
# 克隆 vcpkg
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# 启动 vcpkg
.\bootstrap-vcpkg.bat

# 集成到系统
.\vcpkg integrate install

# 设置环境变量
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "Machine")

# 安装依赖
.\vcpkg install eigen3:x64-windows
.\vcpkg install gtest:x64-windows
```

---

## 📦 安装步骤

### 步骤 1: 克隆项目

```powershell
# 创建工作目录
cd E:\
mkdir workspace
cd workspace

# 克隆项目
git clone https://github.com/your-username/ev_dynamics_simulation.git
cd ev_dynamics_simulation

# 查看项目结构
tree /F /A
```

### 步骤 2: 运行环境检查

```powershell
# 运行环境检查脚本
.\setup.bat

# 预期输出:
# ✓ CMake: 3.25.x
# ✓ MSVC 编译器已配置
# ✓ Python: 3.10.x
# ✓ pybind11: 2.10.x
```

### 步骤 3: 安装项目依赖

#### 方式 A: 使用 vcpkg（推荐）

```powershell
# 如果使用 vcpkg
vcpkg install eigen3:x64-windows
vcpkg install gtest:x64-windows

# 配置 CMake 使用 vcpkg
$env:CMAKE_TOOLCHAIN_FILE = "C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

#### 方式 B: 手动安装 Eigen3

```powershell
# 下载 Eigen3
cd C:\Libraries
git clone https://gitlab.com/libeigen/eigen.git
cd eigen

# 创建构建目录
mkdir build
cd build

# 配置和安装
cmake .. -DCMAKE_INSTALL_PREFIX=C:\Libraries\eigen3
cmake --build . --config Release --target INSTALL

# 设置环境变量
[Environment]::SetEnvironmentVariable(
    "Eigen3_DIR",
    "C:\Libraries\eigen3\share\eigen3\cmake",
    "User"
)
```

---

## 🔨 构建项目

### 方式 A: 使用构建脚本（推荐）

```powershell
# 标准构建（Release 模式）
.\build.bat

# 调试构建
.\build.bat --debug

# 清理构建
.\build.bat --clean

# 构建并运行测试
.\build.bat --test

# 详细输出
.\build.bat --verbose
```

### 方式 B: 手动构建

```powershell
# 1. 创建构建目录
mkdir build
cd build

# 2. 配置项目（Release 模式）
cmake .. `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_INSTALL_PREFIX=E:\workspace\ev_dynamics_simulation\install `
    -DBUILD_PYTHON_BINDINGS=ON `
    -DBUILD_TESTING=ON

# 3. 编译（使用 4 个并行任务）
cmake --build . --config Release -- /m:4

# 4. 安装（可选）
cmake --install . --config Release
```

### 方式 C: 使用 Visual Studio

1. **打开项目**
   ```
   Visual Studio 2022 → File → Open → CMake → 选择 CMakeLists.txt
   ```

2. **配置项目**
   ```
   Project → CMake Settings
   - Configuration Type: Release
   - Build Root: ${projectDir}\build
   - CMake Command Arguments: -DBUILD_PYTHON_BINDINGS=ON
   ```

3. **构建**
   ```
   Build → Build All (Ctrl+Shift+B)
   ```

---

## ✅ 运行测试

### 运行所有测试

```powershell
# 进入构建目录
cd build

# 运行 CTest
ctest -C Release --output-on-failure

# 预期输出:
# 100% tests passed, 0 tests failed out of 195
# Total Test time (real) =  12.34 sec
```

### 运行特定测试

```powershell
# 列出所有测试
ctest -N

# 运行特定测试
ctest -R test_powertrain -C Release --verbose

# 运行匹配模式的测试
ctest -R "test_dynamics.*" -C Release
```

### Python 测试

```powershell
# 设置 Python 路径
$env:PYTHONPATH = "$PWD\build\python_modules"

# 运行 Python 测试
python tests\python\test_all_modules.py

# 使用 pytest
pytest tests\python\ -v
```

---

## 💡 使用示例

### 示例 1: 基础仿真

```python
# example_basic_simulation.py
import sys
sys.path.append('E:/workspace/ev_dynamics_simulation/build/python_modules')

from powertrain import ElectricMotor, Battery
from dynamics import VehicleState, VehicleDynamics
from chassis import SuspensionSystem

# 创建车辆组件
motor = ElectricMotor(
    max_power=150000,      # 150 kW
    max_torque=300,        # 300 Nm
    max_speed=12000        # 12000 rpm
)

battery = Battery(
    capacity=75.0,         # 75 kWh
    max_discharge_rate=200 # 200 kW
)

# 创建车辆动力学
dynamics = VehicleDynamics(
    mass=1500.0,           # 1500 kg
    wheelbase=2.7,         # 2.7 m
    track_width=1.6        # 1.6 m
)

# 初始化状态
state = VehicleState()
state.position_x = 0.0
state.position_y = 0.0
state.velocity_x = 0.0

# 仿真循环
dt = 0.001  # 1ms 时间步长
for i in range(10000):  # 10秒仿真
    # 设置踏板输入
    throttle = 0.5 if i < 5000 else 0.0  # 前5秒半油门
    
    # 更新动力系统
    motor_torque = motor.calculate_torque(throttle, state.velocity)
    
    # 更新车辆动力学
    state = dynamics.update(state, motor_torque, dt)
    
    # 每 100ms 输出一次
    if i % 100 == 0:
        print(f"Time: {i*dt:.2f}s, "
              f"Speed: {state.velocity_x*3.6:.2f} km/h, "
              f"Accel: {state.acceleration_x:.2f} m/s²")

print("仿真完成！")
```

**运行示例:**
```powershell
# 设置 Python 路径
$env:PYTHONPATH = "E:\workspace\ev_dynamics_simulation\build\python_modules"

# 运行脚本
python example_basic_simulation.py
```

### 示例 2: 参数化测试

```python
# example_parametric_study.py
import sys
sys.path.append('E:/workspace/ev_dynamics_simulation/build/python_modules')

from dynamics import VehicleDynamics, VehicleState
import numpy as np
import json

# 测试不同质量对性能的影响
masses = [1200, 1400, 1600, 1800, 2000]  # kg
results = []

for mass in masses:
    # 创建车辆
    dynamics = VehicleDynamics(mass=mass)
    state = VehicleState()
    
    # 运行 0-100 km/h 加速测试
    throttle = 1.0  # 全油门
    target_speed = 100 / 3.6  # 100 km/h → m/s
    dt = 0.001
    
    time_to_100 = 0.0
    for i in range(20000):  # 最多20秒
        state = dynamics.update(state, throttle * 300, dt)
        
        if state.velocity_x >= target_speed:
            time_to_100 = i * dt
            break
    
    results.append({
        'mass': mass,
        'time_0_to_100': time_to_100,
        'final_speed': state.velocity_x * 3.6  # m/s → km/h
    })
    
    print(f"质量 {mass}kg: 0-100 km/h 用时 {time_to_100:.2f}s")

# 保存结果
with open('parametric_study_results.json', 'w') as f:
    json.dump(results, f, indent=2)

print("\n结果已保存到 parametric_study_results.json")
```

### 示例 3: 实时监控

```python
# example_realtime_monitoring.py
import sys
sys.path.append('E:/workspace/ev_dynamics_simulation/build/python_modules')

from dynamics import VehicleDynamics, VehicleState
import time

# 创建车辆
dynamics = VehicleDynamics(mass=1500.0)
state = VehicleState()

# 实时仿真（1ms 步长）
dt = 0.001
real_time_factor = 1.0  # 1x 实时速度

print("开始实时仿真 (Ctrl+C 停止)...")
print("Time(s)  | Speed(km/h) | Accel(m/s²) | LatAcc(m/s²)")
print("-" * 60)

try:
    start_time = time.time()
    sim_time = 0.0
    
    while True:
        # 仿真步进
        state = dynamics.update(state, 150.0, dt)  # 150 Nm 恒定扭矩
        sim_time += dt
        
        # 实时输出（每 0.1 秒）
        if int(sim_time * 10) % 1 == 0 and sim_time < 0.001 + dt:
            print(f"{sim_time:7.2f}  | "
                  f"{state.velocity_x * 3.6:11.2f} | "
                  f"{state.acceleration_x:11.2f} | "
                  f"{state.acceleration_y:11.2f}")
        
        # 实时控制
        elapsed = time.time() - start_time
        expected_time = sim_time / real_time_factor
        
        if elapsed < expected_time:
            time.sleep(expected_time - elapsed)
        
        # 运行 10 秒后停止
        if sim_time >= 10.0:
            break
            
except KeyboardInterrupt:
    print("\n仿真被用户中断")

print("仿真完成！")
```

---

## 💻 IDE 配置

### Visual Studio Code

#### 1. 安装扩展

```json
// 扩展列表
[
    "ms-vscode.cpptools",           // C/C++
    "ms-vscode.cmake-tools",        // CMake Tools
    "ms-python.python",             // Python
    "twxs.cmake",                   // CMake 语法高亮
    "ms-vscode.makefile-tools"      // Makefile 支持
]
```

#### 2. 配置文件

**`.vscode/settings.json`:**
```json
{
    "cmake.configureOnOpen": true,
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.generator": "Visual Studio 17 2022",
    "cmake.configurationType": "Release",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "python.analysis.extraPaths": [
        "${workspaceFolder}/build/python_modules"
    ],
    "python.envFile": "${workspaceFolder}/.env",
    "files.exclude": {
        "**/.git": true,
        "**/build": false
    }
}
```

**`.vscode/launch.json`:**
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Python: 仿真脚本",
            "type": "python",
            "request": "launch",
            "program": "${file}",
            "console": "integratedTerminal",
            "env": {
                "PYTHONPATH": "${workspaceFolder}/build/python_modules"
            }
        },
        {
            "name": "C++: 调试测试",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/tests/Debug/test_dynamics.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "console": "integratedTerminal"
        }
    ]
}
```

**`.vscode/tasks.json`:**
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake: 配置",
            "type": "shell",
            "command": "cmake",
            "args": [
                "..",
                "-DCMAKE_BUILD_TYPE=Release",
                "-DBUILD_PYTHON_BINDINGS=ON"
            ],
            "options": {
                "cwd": "${workspaceFolder}/build"
            }
        },
        {
            "label": "CMake: 构建",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build",
                ".",
                "--config",
                "Release",
                "--",
                "/m:4"
            ],
            "options": {
                "cwd": "${workspaceFolder}/build"
            },
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
        {
            "label": "运行测试",
            "type": "shell",
            "command": "ctest",
            "args": [
                "-C",
                "Release",
                "--output-on-failure"
            ],
            "options": {
                "cwd": "${workspaceFolder}/build"
            }
        }
    ]
}
```

### Visual Studio 2022

#### 1. 打开项目

```
File → Open → CMake → 选择 E:\workspace\ev_dynamics_simulation\CMakeLists.txt
```

#### 2. 配置项目

**Project → CMake Settings:**
```json
{
  "configurations": [
    {
      "name": "x64-Release",
      "generator": "Visual Studio 17 2022 Win64",
      "configurationType": "Release",
      "buildRoot": "${projectDir}\\build",
      "cmakeCommandArgs": "-DBUILD_PYTHON_BINDINGS=ON -DBUILD_TESTING=ON",
      "buildCommandArgs": "/m:4",
      "ctestCommandArgs": "-C Release --output-on-failure"
    }
  ]
}
```

#### 3. 设置 Python 环境

**Tools → Options → Python:**
- Environment: Python 3.10+
- Search Paths: 添加 `build\python_modules`

#### 4. 调试配置

**调试 → 属性:**
- Command: `python.exe`
- Command Arguments: `tests\python\test_all_modules.py`
- Working Directory: `$(ProjectDir)`
- Environment: `PYTHONPATH=$(ProjectDir)\build\python_modules`

---

## 🔧 故障排查

### 问题 1: CMake 找不到编译器

**症状:**
```
CMake Error: Could not find CMAKE_C_COMPILER
```

**解决方案:**
```powershell
# 方式 1: 使用 Developer Command Prompt
# 开始菜单 → Visual Studio 2022 → Developer Command Prompt

# 方式 2: 手动设置环境
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

# 方式 3: 在 CMake 中指定
cmake .. -G "Visual Studio 17 2022" -A x64
```

### 问题 2: 找不到 Eigen3

**症状:**
```
CMake Error: Could not find Eigen3
```

**解决方案:**
```powershell
# 方式 1: 使用 vcpkg
vcpkg install eigen3:x64-windows
vcpkg integrate install

# 方式 2: 设置 CMAKE_PREFIX_PATH
$env:CMAKE_PREFIX_PATH = "C:\Libraries\eigen3"
cmake ..

# 方式 3: 指定 Eigen3_DIR
cmake .. -DEigen3_DIR="C:\Libraries\eigen3\share\eigen3\cmake"
```

### 问题 3: Python 模块导入失败

**症状:**
```
ModuleNotFoundError: No module named 'powertrain'
```

**解决方案:**
```powershell
# 1. 确认模块已构建
ls build\python_modules\

# 应该看到:
# powertrain.pyd
# dynamics.pyd
# chassis.pyd
# tire.pyd
# scheduler.pyd

# 2. 设置 PYTHONPATH
$env:PYTHONPATH = "$PWD\build\python_modules"

# 3. 或在 Python 代码中添加
import sys
sys.path.append('E:/workspace/ev_dynamics_simulation/build/python_modules')

# 4. 检查 Python 版本
python --version  # 必须与构建时使用的版本一致
```

### 问题 4: 测试失败

**症状:**
```
ctest 报告测试失败
```

**解决方案:**
```powershell
# 1. 查看详细输出
ctest -C Release --output-on-failure --verbose

# 2. 单独运行失败的测试
cd build\tests\Debug\
.\test_dynamics.exe

# 3. 检查依赖
dumpbin /dependents test_dynamics.exe

# 4. 清理并重新构建
.\build.bat --clean
.\build.bat --test
```

### 问题 5: 实时性能不达标

**症状:**
```
仿真步长超过 1ms
```

**解决方案:**
```powershell
# 1. 使用 Release 模式
.\build.bat --clean
.\build.bat  # 默认 Release

# 2. 检查 CPU 占用
# 任务管理器 → 性能 → CPU

# 3. 关闭不必要的后台程序

# 4. 检查电源设置
# 控制面板 → 电源选项 → 高性能

# 5. 设置进程优先级（需要管理员权限）
# 任务管理器 → 详细信息 → python.exe → 设置优先级 → 高
```

---

## ⚡ 性能优化

### 编译优化

```powershell
# 1. 使用 Release 模式
cmake .. -DCMAKE_BUILD_TYPE=Release

# 2. 启用链接时优化 (LTO)
cmake .. -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON

# 3. 并行编译
cmake --build . -- /m:8  # 使用 8 个核心

# 4. 使用预编译头（可选）
cmake .. -DUSE_PRECOMPILED_HEADERS=ON
```

### 运行时优化

```powershell
# 1. 关闭调试输出
# 编辑 config/simulation_config.yaml
logging:
  level: WARNING  # 从 DEBUG 改为 WARNING

# 2. 减少输出频率
output:
  frequency: 100  # 每 100 步输出一次

# 3. 使用批处理模式
simulation:
  mode: batch  # 而非 interactive
```

### 系统优化

```powershell
# 1. 设置高性能电源计划
powercfg /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c

# 2. 禁用 CPU 频率缩放
# BIOS 设置 → 关闭 SpeedStep/Cool'n'Quiet

# 3. 关闭超线程（如果 CPU 核心数足够）
# BIOS 设置 → Hyper-Threading → Disabled

# 4. 设置进程亲和性
# 任务管理器 → 详细信息 → 右键 → 设置亲和性
```

---

## ❓ 常见问题

### Q1: 支持哪些 Windows 版本？

**A:** 支持 Windows 10 1809+ 和 Windows 11。推荐使用 Windows 10 21H2 或 Windows 11 22H2。

### Q2: 必须使用 Visual Studio 吗？

**A:** 不是必须的，但推荐。也可以使用：
- MinGW-w64（GCC for Windows）
- Clang for Windows
- MSYS2

但 Visual Studio 提供最好的调试体验和工具集成。

### Q3: 可以在虚拟机中运行吗？

**A:** 可以，但性能会降低 20-40%。推荐配置：
- 虚拟机: VMware Workstation 或 Hyper-V
- 内存: 8GB+
- CPU: 4 核心
- 启用嵌套虚拟化

### Q4: 如何卸载项目？

**A:**
```powershell
# 1. 删除构建目录
Remove-Item -Recurse -Force build

# 2. 删除安装目录（如果安装了）
Remove-Item -Recurse -Force install

# 3. 删除项目目录
cd ..
Remove-Item -Recurse -Force ev_dynamics_simulation

# 4. 清理环境变量（可选）
[Environment]::SetEnvironmentVariable("PYTHONPATH", $null, "User")
```

### Q5: 如何更新项目？

**A:**
```powershell
# 1. 拉取最新代码
git pull origin main

# 2. 更新子模块（如果有）
git submodule update --init --recursive

# 3. 清理并重新构建
.\build.bat --clean
.\build.bat
```

### Q6: 支持多语言吗？

**A:** 项目主要使用英文，但文档支持中英文双语。代码注释使用英文。

### Q7: 如何贡献代码？

**A:**
1. Fork 项目到你的 GitHub
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

### Q8: 如何报告 Bug？

**A:**
1. 查看 GitHub Issues 是否已有相同问题
2. 如果没有，创建新 Issue
3. 包含以下信息：
   - Windows 版本
   - Visual Studio 版本
   - CMake 版本
   - Python 版本
   - 错误消息和日志
   - 重现步骤

### Q9: 可以用于商业项目吗？

**A:** 可以。项目使用 MIT 许可证，允许商业使用。详见 LICENSE 文件。

### Q10: 如何获取技术支持？

**A:**
- **文档:** 查看项目 `docs/` 目录
- **FAQ:** `docs/FAQ.md`
- **Issues:** GitHub Issues
- **邮件:** support@example.com

---

## 📚 参考资源

### 官方文档
- **README.md** - 项目概览
- **QUICKSTART_GUIDE.md** - 快速开始
- **docs/USER_GUIDE.md** - 完整用户指南
- **docs/API_REFERENCE.md** - API 参考
- **docs/ARCHITECTURE.md** - 架构设计

### 外部资源
- **CMake 文档:** https://cmake.org/documentation/
- **Visual Studio 文档:** https://docs.microsoft.com/visualstudio/
- **pybind11 文档:** https://pybind11.readthedocs.io/
- **Eigen3 文档:** https://eigen.tuxfamily.org/

### 视频教程（计划中）
- 环境搭建教程
- 基础使用教程
- 高级功能教程
- 性能优化教程

---

## 🎯 下一步

1. ✅ 完成环境搭建
2. ✅ 成功构建项目
3. ✅ 运行测试通过
4. 📖 阅读 `docs/USER_GUIDE.md` 了解更多功能
5. 🚀 运行 `examples/` 中的示例
6. 💡 开始你的仿真项目！

---

## 📝 版本历史

- **v1.0** (2026-03-07) - 初始版本
  - 完整的 Windows 10 安装指南
  - Visual Studio 2022 配置
  - 详细的故障排查
  - 性能优化建议

---

## 📧 联系方式

- **项目主页:** https://github.com/your-username/ev_dynamics_simulation
- **问题反馈:** https://github.com/your-username/ev_dynamics_simulation/issues
- **邮件支持:** support@example.com

---

**祝你使用愉快！如有问题，请随时查阅文档或联系支持。** 🚗💨

---

*最后更新: 2026-03-07*  
*文档版本: 1.0*  
*适用项目版本: v1.0+*
