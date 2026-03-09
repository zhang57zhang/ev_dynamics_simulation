# Windows 10 快速参考卡片

**六自由度新能源车辆动力学仿真 - 常用命令速查**

---

## 🚀 快速开始（5 步）

```powershell
# 1. 克隆项目
git clone https://github.com/your-username/ev_dynamics_simulation.git
cd ev_dynamics_simulation

# 2. 检查环境
.\setup.bat

# 3. 构建项目
.\build.bat

# 4. 运行测试
cd build
ctest -C Release --output-on-failure

# 5. 运行示例
$env:PYTHONPATH = "$PWD\python_modules"
python ..\tests\python\test_all_modules.py
```

---

## 📦 构建命令

| 命令 | 说明 |
|------|------|
| `.\build.bat` | 标准构建（Release） |
| `.\build.bat --debug` | 调试构建 |
| `.\build.bat --clean` | 清理后构建 |
| `.\build.bat --test` | 构建并测试 |
| `.\build.bat --verbose` | 详细输出 |

---

## 🧪 测试命令

| 命令 | 说明 |
|------|------|
| `ctest -C Release` | 运行所有测试 |
| `ctest -C Release --output-on-failure` | 显示失败详情 |
| `ctest -C Release -R test_powertrain` | 运行特定测试 |
| `ctest -C Release --verbose` | 详细输出 |

---

## 🐍 Python 命令

```powershell
# 设置 Python 路径
$env:PYTHONPATH = "E:\workspace\ev_dynamics_simulation\build\python_modules"

# 测试导入
python -c "import powertrain; print('OK')"

# 运行 Python 测试
python tests\python\test_all_modules.py

# 使用 pytest
pytest tests\python\ -v
```

---

## 🔧 环境变量

```powershell
# Python 模块路径
$env:PYTHONPATH = "$PWD\build\python_modules"

# CMake 前缀路径
$env:CMAKE_PREFIX_PATH = "C:\Libraries\eigen3"

# vcpkg 工具链
$env:CMAKE_TOOLCHAIN_FILE = "C:\vcpkg\scripts\buildsystems\vcpkg.cmake"

# 永久设置（用户级别）
[Environment]::SetEnvironmentVariable("PYTHONPATH", "E:\workspace\ev_dynamics_simulation\build\python_modules", "User")
```

---

## 📁 目录结构

```
ev_dynamics_simulation/
├── build/              # 构建输出
│   ├── python_modules/ # Python 绑定
│   └── tests/          # 测试可执行文件
├── src/                # 源代码
├── include/            # 头文件
├── tests/              # 测试代码
├── docs/               # 文档
├── config/             # 配置文件
├── examples/           # 示例代码
├── scripts/            # 工具脚本
├── build.bat           # Windows 构建脚本
└── setup.bat           # 环境检查脚本
```

---

## 🛠️ 常用工具

### Visual Studio 2019/2022

```powershell
# 打开 Developer Command Prompt
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

# 检查编译器
cl /?

# 检查环境
where cl
where cmake
where python
```

### CMake

```powershell
# 配置项目
cmake .. -G "Visual Studio 17 2022" -A x64

# 构建项目
cmake --build . --config Release -- /m:4

# 安装项目
cmake --install . --config Release

# 清理项目
cmake --build . --target clean
```

### Git

```powershell
# 更新代码
git pull origin main

# 查看状态
git status

# 提交更改
git add .
git commit -m "描述信息"
git push origin main
```

---

## 🔍 故障排查

### 编译错误

```powershell
# 清理并重新构建
.\build.bat --clean

# 检查编译器
where cl
cl

# 检查 CMake
cmake --version
```

### 依赖问题

```powershell
# 检查 Eigen3
cmake .. -DEigen3_DIR="C:\Libraries\eigen3\share\eigen3\cmake"

# 使用 vcpkg
vcpkg install eigen3:x64-windows
vcpkg integrate install
```

### Python 问题

```powershell
# 检查 Python
python --version
pip list

# 重新安装 pybind11
pip uninstall pybind11
pip install pybind11

# 检查模块
python -c "import pybind11; print(pybind11.__version__)"
```

---

## 📊 性能监控

```powershell
# 检查 CPU 占用
Get-Counter '\Processor(_Total)\% Processor Time'

# 检查内存占用
Get-Counter '\Memory\Available MBytes'

# 检查进程
Get-Process python | Select-Object CPU, WorkingSet

# 实时监控
while ($true) {
    Get-Process python | Select-Object Name, CPU, WorkingSet
    Start-Sleep 1
}
```

---

## 📚 文档快速链接

| 文档 | 路径 | 说明 |
|------|------|------|
| 快速开始 | `QUICKSTART_GUIDE.md` | 10分钟上手 |
| 用户指南 | `docs/USER_GUIDE.md` | 完整使用指南 |
| Windows 指南 | `docs/WINDOWS10_USER_GUIDE.md` | Windows 专属指南 |
| API 参考 | `docs/API_REFERENCE.md` | API 文档 |
| 架构设计 | `docs/ARCHITECTURE.md` | 系统架构 |
| 故障排查 | `docs/TROUBLESHOOTING.md` | 常见问题 |
| FAQ | `docs/FAQ.md` | 常见问答 |

---

## 💡 常用配置

### config/simulation_config.yaml

```yaml
simulation:
  dt: 0.001           # 时间步长 (s)
  duration: 10.0      # 仿真时长 (s)
  mode: realtime      # realtime / batch

vehicle:
  mass: 1500          # 质量 (kg)
  wheelbase: 2.7      # 轴距 (m)

output:
  frequency: 100      # 输出频率
  format: csv         # csv / json / hdf5
```

---

## 🎯 一键命令

```powershell
# 完整流程（清理 + 构建 + 测试）
.\build.bat --clean --test

# 开发流程（快速构建）
.\build.bat

# 验证环境
.\setup.bat

# 运行示例
$env:PYTHONPATH = "$PWD\build\python_modules"; python examples\basic_simulation.py
```

---

## 📞 获取帮助

```powershell
# 查看构建帮助
.\build.bat --help

# 查看 CMake 帮助
cmake --help

# 查看 ctest 帮助
ctest --help

# 查看项目文档
start docs\WINDOWS10_USER_GUIDE.md
```

---

## 🔗 有用链接

- **项目主页:** https://github.com/your-username/ev_dynamics_simulation
- **问题反馈:** https://github.com/your-username/ev_dynamics_simulation/issues
- **CMake 文档:** https://cmake.org/documentation/
- **Visual Studio:** https://visualstudio.microsoft.com/
- **vcpkg:** https://vcpkg.io/

---

**打印此文档作为快速参考！** 🖨️

---

*最后更新: 2026-03-07*
