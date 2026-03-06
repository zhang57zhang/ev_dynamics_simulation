# EV Dynamics Simulation - 部署指南

**版本**: 1.0  
**更新日期**: 2026-03-06  
**适用阶段**: Phase 2+（核心模块已完成）

---

## 目录

1. [系统要求](#1-系统要求)
2. [依赖安装](#2-依赖安装)
3. [编译步骤](#3-编译步骤)
4. [安装步骤](#4-安装步骤)
5. [配置步骤](#5-配置步骤)
6. [启动系统](#6-启动系统)
7. [验证安装](#7-验证安装)
8. [故障排查](#8-故障排查)
9. [下一步](#9-下一步)

---

## 1. 系统要求

### 1.1 硬件要求

#### 最低配置
- **CPU**: 4核心，2.5 GHz+
- **内存**: 8 GB RAM
- **存储**: 20 GB 可用空间
- **网络**: 以太网接口（HIL集成时需要）

#### 推荐配置（实时仿真）
- **CPU**: 8核心，3.0 GHz+
- **内存**: 16 GB RAM
- **存储**: 50 GB SSD
- **网络**: 千兆以太网（HIL接口）

### 1.2 软件要求

#### Linux（推荐，生产环境）

| 组件 | 版本要求 | 用途 |
|------|---------|------|
| 操作系统 | Ubuntu 20.04 LTS | 宿主系统 |
| 内核 | Linux 5.4+ with PREEMPT_RT | 实时性保障 |
| 编译器 | GCC 9+ 或 Clang 10+ | C++编译 |
| CMake | 3.16+ | 构建系统 |
| Python | 3.10+ | Python绑定 |

#### Windows（开发环境）

| 组件 | 版本要求 | 用途 |
|------|---------|------|
| 操作系统 | Windows 10/11 | 宿主系统 |
| 编译器 | MSVC 2019+ | C++编译 |
| CMake | 3.16+ | 构建系统 |
| Python | 3.10+ | Python绑定 |

#### macOS（开发环境）

| 组件 | 版本要求 | 用途 |
|------|---------|------|
| 操作系统 | macOS 10.15+ | 宿主系统 |
| 编译器 | Clang 10+ (Xcode) | C++编译 |
| CMake | 3.16+ | 构建系统 |
| Python | 3.10+ | Python绑定 |

### 1.3 依赖库

| 库 | 版本 | 用途 | 必需 |
|-----|------|------|------|
| Eigen3 | 3.3+ | 线性代数运算 | ✅ |
| Google Test | 1.10+ | 单元测试框架 | ✅ |
| pybind11 | 2.6+ | Python绑定 | ✅ |
| yaml-cpp | 0.6+ | YAML配置解析 | ⚪ 可选 |
| Boost | 1.71+ | 高级功能（可选） | ⚪ 可选 |

---

## 2. 依赖安装

### 2.1 Linux (Ubuntu 20.04)

#### 方法一：系统包管理器（推荐）

```bash
# 更新系统
sudo apt update && sudo apt upgrade -y

# 安装编译工具
sudo apt install -y build-essential cmake git

# 安装Eigen3
sudo apt install -y libeigen3-dev

# 安装Google Test
sudo apt install -y libgtest-dev
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib/

# 安装Python开发包
sudo apt install -y python3-dev python3-pip

# 安装pybind11
pip3 install pybind11

# 安装Python依赖
pip3 install numpy scipy matplotlib pyyaml
```

#### 方法二：vcpkg（推荐用于跨平台一致性）

```bash
# 安装vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# 安装依赖
./vcpkg install eigen3:x64-linux
./vcpkg install gtest:x64-linux
./vcpkg install pybind11:x64-linux
```

### 2.2 Windows

#### 方法一：vcpkg（推荐）

```powershell
# 安装vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 安装依赖
.\vcpkg install eigen3:x64-windows
.\vcpkg install gtest:x64-windows
.\vcpkg install pybind11:x64-windows

# 集成到Visual Studio
.\vcpkg integrate install

# 安装Python依赖
pip install numpy scipy matplotlib pyyaml pybind11
```

#### 方法二：手动安装

1. **Eigen3**: 下载源码编译或使用包管理器
2. **Google Test**: 从GitHub下载预编译版本
3. **pybind11**: `pip install pybind11`

### 2.3 macOS

```bash
# 安装Homebrew（如果未安装）
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装依赖
brew install eigen
brew install googletest
brew install cmake

# 安装Python依赖
pip3 install numpy scipy matplotlib pyyaml pybind11
```

---

## 3. 编译步骤

### 3.1 克隆代码

```bash
# 克隆仓库
git clone https://github.com/your-org/ev_dynamics_simulation.git
cd ev_dynamics_simulation

# 检查分支（可选）
git branch -a
git checkout main
```

### 3.2 配置CMake

#### Linux/macOS

```bash
# 创建构建目录
mkdir build
cd build

# 配置CMake
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_PYTHON_BINDINGS=ON \
  -DENABLE_TESTS=ON \
  -DCMAKE_INSTALL_PREFIX=/usr/local
```

#### Windows

```powershell
# 创建构建目录
mkdir build
cd build

# 配置CMake（Visual Studio）
cmake .. `
  -G "Visual Studio 16 2019" `
  -A x64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DENABLE_PYTHON_BINDINGS=ON `
  -DENABLE_TESTS=ON `
  -DCMAKE_INSTALL_PREFIX="C:\Program Files\ev_dynamics"
```

### 3.3 编译

#### Linux/macOS

```bash
# 并行编译（使用所有CPU核心）
make -j$(nproc)

# 或者使用提供的脚本
cd ..
./build.sh
```

#### Windows

```powershell
# 编译Release版本
cmake --build . --config Release

# 或者使用提供的脚本
cd ..
.\build.bat
```

**预期输出**:
- 编译5个核心模块（powertrain/chassis/dynamics/tire/scheduler）
- 生成Python绑定模块
- 编译测试程序

### 3.4 运行测试

```bash
# 运行所有测试
cd build
ctest --output-on-failure

# 运行特定模块测试
ctest -R test_powertrain -V
ctest -R test_chassis -V
ctest -R test_dynamics -V
ctest -R test_tire -V
ctest -R test_scheduler -V

# 生成覆盖率报告（可选）
make coverage
```

**预期结果**:
- 所有159个测试通过
- 代码覆盖率 >90%

---

## 4. 安装步骤

### 4.1 系统安装

#### Linux

```bash
# 安装到系统目录（需要root权限）
cd build
sudo make install

# 安装后的文件位置
# - 可执行文件: /usr/local/bin/
# - 库文件: /usr/local/lib/
# - 头文件: /usr/local/include/ev_dynamics/
# - 配置文件: /usr/local/etc/ev_dynamics/
```

#### Windows

```powershell
# 安装到Program Files（需要管理员权限）
cd build
cmake --install . --config Release

# 安装后的文件位置
# - 可执行文件: C:\Program Files\ev_dynamics\bin\
# - 库文件: C:\Program Files\ev_dynamics\lib\
# - 头文件: C:\Program Files\ev_dynamics\include\
```

### 4.2 Python包安装

```bash
# 开发模式安装（推荐）
cd python
pip install -e .

# 或者正式安装
pip install .
```

**验证Python安装**:

```python
# 测试Python绑定
import sys
sys.path.append('build/python_modules')

import powertrain
import chassis
import dynamics
import tire
import scheduler

# 创建电机模型
motor = powertrain.Motor(max_torque=300.0, max_speed=15000.0)
print(f"✅ Python绑定安装成功！")
```

---

## 5. 配置步骤

### 5.1 基本配置

编辑配置文件 `config/simulation_config.yaml`:

```yaml
# 仿真参数
simulation:
  frequency: 500  # Hz - 仿真频率
  duration: 10.0  # 秒 - 仿真时长
  mode: "realtime"  # realtime, batch, replay

# 车辆参数
vehicle:
  mass: 1500.0  # kg - 整车质量
  inertia:
    ix: 500.0   # kg*m^2
    iy: 2000.0
    iz: 2500.0

# 动力系统参数
powertrain:
  motor:
    max_torque: 300.0  # Nm
  battery:
    capacity: 60.0  # kWh
```

**完整配置示例**: 参考 `config/simulation_config.yaml`

### 5.2 实时配置（Linux）

#### 安装PREEMPT_RT内核

```bash
# 安装实时内核
sudo apt install linux-image-rt-amd64

# 查看可用内核
dpkg --list | grep linux-image

# 更新GRUB
sudo update-grub

# 重启选择RT内核
sudo reboot
```

#### 验证RT内核

```bash
# 检查当前内核
uname -v
# 应该包含 "PREEMPT RT"

# 运行实时性测试
python3 tests/test_realtime_prerequisite.py
```

#### 系统优化

```bash
# 配置CPU亲和性（隔离CPU核心）
sudo systemctl set-property --runtime -- system.cpu-affinity=0-3

# 配置内存锁定（避免交换）
sudo bash -c "echo '* hard memlock unlimited' >> /etc/security/limits.conf"
sudo bash -c "echo '* soft memlock unlimited' >> /etc/security/limits.conf"

# 禁用中断平衡
sudo systemctl stop irqbalance
sudo systemctl disable irqbalance

# 配置CPU频率为性能模式
sudo cpupower frequency-set -g performance
```

---

## 6. 启动系统

### 6.1 命令行启动

#### C++ 可执行文件

```bash
# Linux/macOS
./build/bin/ev_dynamics_simulation

# Windows
.\build\bin\Release\ev_dynamics_simulation.exe
```

#### Python 脚本

```python
# 基本仿真示例
from ev_dynamics import Vehicle, Simulation

# 加载配置
vehicle = Vehicle(config='config/simulation_config.yaml')

# 创建仿真
sim = Simulation(vehicle, duration=10.0, frequency=500)

# 运行仿真
sim.run()

# 查看结果
sim.plot_results()
```

### 6.2 Systemd服务（Linux）

#### 创建服务文件

```bash
# 创建服务文件
sudo nano /etc/systemd/system/ev_dynamics.service
```

内容：

```ini
[Unit]
Description=EV Dynamics Simulation Service
After=network.target

[Service]
Type=simple
User=evdynamics
Group=evdynamics
WorkingDirectory=/opt/ev_dynamics
ExecStart=/usr/local/bin/ev_dynamics_simulation
Restart=on-failure
RestartSec=10

# 实时性配置
LimitRTPRIO=95
LimitMEMLOCK=infinity
CPUSchedulingPolicy=rr
CPUSchedulingPriority=90

[Install]
WantedBy=multi-user.target
```

#### 启动服务

```bash
# 重载systemd配置
sudo systemctl daemon-reload

# 启用开机自启
sudo systemctl enable ev_dynamics

# 启动服务
sudo systemctl start ev_dynamics

# 查看状态
sudo systemctl status ev_dynamics

# 查看日志
sudo journalctl -u ev_dynamics -f
```

---

## 7. 验证安装

### 7.1 健康检查脚本

```bash
# 运行健康检查
./scripts/health_check.sh
```

**健康检查内容**:
- ✅ 依赖库是否安装
- ✅ 配置文件是否有效
- ✅ 编译产物是否存在
- ✅ Python绑定是否正常
- ✅ 实时性是否满足要求

### 7.2 测试仿真

#### 单元测试

```bash
# 运行所有单元测试
python3 tests/run_all_tests.py

# 运行性能测试
python3 tests/test_shared_memory_perf.py
python3 tests/test_message_queue_perf.py
```

#### 集成测试

```bash
# 运行驾驶循环测试
python3 tests/system/test_driving_cycles.py

# 运行实时性测试
python3 tests/test_realtime_prerequisite.py
```

**预期结果**:
- ✅ 所有159个测试通过
- ✅ 性能指标达标（延迟 <80μs）
- ✅ 精度 >99%

---

## 8. 故障排查

### 8.1 编译错误

#### 问题：找不到Eigen3

```
CMake Error: Could not find Eigen3
```

**解决方案**:

```bash
# Linux
sudo apt install libeigen3-dev

# 或指定Eigen3路径
cmake .. -DEigen3_DIR=/path/to/eigen3
```

#### 问题：Python绑定编译失败

```
error: pybind11 not found
```

**解决方案**:

```bash
# 安装pybind11
pip install pybind11

# 或指定pybind11路径
cmake .. -Dpybind11_DIR=/path/to/pybind11
```

### 8.2 运行错误

#### 问题：找不到共享库

```
error while loading shared libraries: libev_dynamics.so
```

**解决方案**:

```bash
# Linux
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# 或更新链接器缓存
sudo ldconfig
```

#### 问题：配置文件未找到

```
Error: Cannot open config file
```

**解决方案**:

```bash
# 检查配置文件路径
ls config/simulation_config.yaml

# 或使用绝对路径
vehicle = Vehicle(config='/absolute/path/to/simulation_config.yaml')
```

### 8.3 性能问题

#### 问题：实时性不满足

```
Warning: Frame time exceeded (2.5ms > 2.0ms)
```

**解决方案**:

```bash
# 1. 检查是否使用RT内核
uname -v | grep PREEMPT

# 2. 关闭不必要的服务
sudo systemctl disable bluetooth
sudo systemctl disable cups

# 3. 使用性能模式
sudo cpupower frequency-set -g performance

# 4. 检查系统负载
top -H -p $(pgrep ev_dynamics)
```

### 8.4 测试失败

#### 问题：测试超时

```
Timeout: test_realtime_prerequisite.py exceeded 30s
```

**解决方案**:

```bash
# 检查系统负载
htop

# 增加测试超时时间
ctest --timeout 60
```

---

## 9. 下一步

### 9.1 阅读文档

- **用户指南**: [USER_GUIDE.md](USER_GUIDE.md) - 学习如何使用系统
- **API参考**: [API_REFERENCE.md](API_REFERENCE.md) - 查看详细API文档
- **生产部署**: [PRODUCTION_DEPLOYMENT.md](PRODUCTION_DEPLOYMENT.md) - 生产环境配置

### 9.2 运行示例

```bash
# 查看示例代码
cd examples

# 运行基本仿真
python3 basic_simulation.py

# 运行加速测试
python3 acceleration_test.py

# 运行制动测试
python3 braking_test.py
```

### 9.3 参与开发

- **开发指南**: [AGENT_DEVELOPMENT_GUIDE.md](AGENT_DEVELOPMENT_GUIDE.md)
- **架构文档**: [ARCHITECTURE.md](ARCHITECTURE.md)
- **贡献指南**: 参见项目根目录 README.md

---

## 附录

### A. 构建选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `CMAKE_BUILD_TYPE` | Release | Debug/Release/RelWithDebInfo |
| `ENABLE_PYTHON_BINDINGS` | ON | 启用Python绑定 |
| `ENABLE_TESTS` | ON | 启用测试 |
| `ENABLE_COVERAGE` | OFF | 启用覆盖率报告 |
| `CMAKE_INSTALL_PREFIX` | /usr/local | 安装路径 |

### B. 环境变量

| 变量 | 说明 | 示例 |
|------|------|------|
| `EV_DYNAMICS_CONFIG` | 配置文件路径 | `/etc/ev_dynamics/config.yaml` |
| `EV_DYNAMICS_LOG_LEVEL` | 日志级别 | `DEBUG`, `INFO`, `WARNING` |
| `LD_LIBRARY_PATH` | 库搜索路径 | `/usr/local/lib` |

### C. 性能基准

| 指标 | 目标值 | 实测值 | 状态 |
|------|--------|--------|------|
| 仿真频率 | 500 Hz | 500 Hz | ✅ |
| 时间步长 | 2 ms | 2 ms | ✅ |
| 延迟 | <1 ms | <80 μs | ✅ 超额完成 |
| 精度 | >95% | >99% | ✅ 超额完成 |
| 代码覆盖率 | >90% | >90% | ✅ |

---

**文档版本**: 1.0  
**最后更新**: 2026-03-06  
**维护者**: EV Dynamics Team
