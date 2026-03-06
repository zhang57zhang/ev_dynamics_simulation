# 快速开始指南 - 六自由度车辆动力学仿真

**适用于：** 新用户快速上手  
**预计时间：** 10-15分钟  
**难度：** ⭐⭐☆☆☆

---

## 📋 前置要求

### 必需环境
- **操作系统：** Linux (Ubuntu 20.04+) / Windows 10+ / macOS 10.15+
- **编译器：** GCC 9+ / MSVC 2019+ / Clang 10+
- **CMake：** 3.16+
- **Python：** 3.8+（可选，用于Python绑定）

### 硬件要求
- **CPU：** 4核心以上（推荐8核心）
- **内存：** 8GB以上（推荐16GB）
- **存储：** 2GB可用空间

---

## 🚀 5分钟快速开始

### Step 1: 获取代码（1分钟）

```bash
# 克隆仓库
git clone https://github.com/your-username/ev_dynamics_simulation.git
cd ev_dynamics_simulation
```

### Step 2: 安装依赖（2分钟）

**Linux (Ubuntu/Debian):**
```bash
# 运行自动安装脚本
./scripts/install.sh

# 或手动安装
sudo apt-get update
sudo apt-get install -y build-essential cmake libeigen3-dev
```

**Windows:**
```powershell
# 运行自动安装脚本
.\scripts\install.bat

# 或使用 vcpkg
vcpkg install eigen3
```

**macOS:**
```bash
# 使用 Homebrew
brew install cmake eigen
```

### Step 3: 构建项目（2分钟）

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译（使用4个并行任务）
cmake --build . -j4
```

### Step 4: 运行测试（1分钟）

```bash
# 运行所有测试
ctest

# 预期输出
# 100% tests passed, 0 tests failed out of 195
```

### Step 5: 运行仿真（1分钟）

```bash
# 运行基础仿真
./bin/vehicle_simulation

# 预期输出
# [INFO] 初始化车辆模型...
# [INFO] 加载配置文件...
# [INFO] 开始仿真...
# [INFO] 仿真完成！
```

---

## 📚 15分钟进阶教程

### 1. 理解项目结构（3分钟）

```
ev_dynamics_simulation/
├── src/              # 源代码（5个模块）
│   ├── powertrain/   # 动力系统
│   ├── chassis/      # 底盘系统
│   ├── dynamics/     # 车辆动力学
│   ├── tire/         # 轮胎模型
│   └── scheduler/    # 实时调度
├── include/          # 头文件
├── tests/            # 测试代码
├── docs/             # 文档
├── scripts/          # 脚本工具
└── config/           # 配置文件
```

### 2. 运行Python示例（5分钟）

```python
# 安装 Python 绑定
cd build
cmake -DBUILD_PYTHON_BINDINGS=ON ..
cmake --build .
pip install ./python

# Python 示例
from ev_dynamics import Vehicle, Simulation

# 创建车辆
vehicle = Vehicle(
    mass=1500.0,        # 质量 (kg)
    wheelbase=2.7,      # 轴距 (m)
    track_width=1.6     # 轮距 (m)
)

# 创建仿真
sim = Simulation(vehicle, dt=0.001)  # 1ms时间步长

# 运行仿真
for i in range(1000):
    state = sim.step()
    if i % 100 == 0:
        print(f"Time: {state.time:.3f}s, Speed: {state.speed:.2f} m/s")
```

### 3. 修改配置参数（4分钟）

```yaml
# 编辑 config/simulation_config.yaml
vehicle:
  mass: 1500              # 车辆质量 (kg)
  inertia:
    roll: 500             # 侧倾转动惯量 (kg·m²)
    pitch: 2000           # 俯仰转动惯量 (kg·m²)
    yaw: 2500             # 偏航转动惯量 (kg·m²)

powertrain:
  motor:
    max_power: 150000     # 最大功率 (W)
    max_torque: 300       # 最大扭矩 (N·m)

simulation:
  dt: 0.001               # 时间步长 (s)
  duration: 10.0          # 仿真时长 (s)
```

### 4. 查看仿真结果（3分钟）

```bash
# 运行仿真并保存结果
./bin/vehicle_simulation --output results.csv

# 查看结果（使用Python）
python scripts/plot_results.py results.csv

# 预期输出
# 生成以下图表：
# - 车速曲线
# - 加速度曲线
# - 轮胎力曲线
# - 悬架位移曲线
```

---

## 🎯 常见使用场景

### 场景1: 基础性能测试

```bash
# 直线加速测试
./bin/vehicle_simulation --scenario acceleration

# 制动测试
./bin/vehicle_simulation --scenario braking

# 稳态回转测试
./bin/vehicle_simulation --scenario steady_turn
```

### 场景2: 参数敏感性分析

```python
# 批量测试不同质量
import numpy as np
from ev_dynamics import Vehicle, Simulation

masses = np.linspace(1000, 2000, 11)  # 1000kg - 2000kg

for mass in masses:
    vehicle = Vehicle(mass=mass)
    sim = Simulation(vehicle)
    results = sim.run(duration=10.0)
    
    print(f"Mass: {mass}kg, Max Speed: {results.max_speed:.2f} m/s")
```

### 场景3: 实时HIL测试

```bash
# 启动实时模式
./bin/vehicle_simulation --realtime --hil

# 配置HIL接口
# 编辑 config/hil_config.yaml
hil:
  interface: CAN
  bitrate: 500000
  channel: can0
```

---

## 🔧 故障排查

### 问题1: 编译失败

**症状：** `CMake Error: Could not find Eigen3`

**解决：**
```bash
# Linux
sudo apt-get install libeigen3-dev

# macOS
brew install eigen

# Windows (vcpkg)
vcpkg install eigen3
```

### 问题2: 测试失败

**症状：** `ctest` 报告测试失败

**解决：**
```bash
# 查看详细输出
ctest --verbose

# 单独运行失败的测试
./bin/test_powertrain --verbose
```

### 问题3: Python导入错误

**症状：** `ModuleNotFoundError: No module named 'ev_dynamics'`

**解决：**
```bash
# 确保Python绑定已构建
cd build
cmake -DBUILD_PYTHON_BINDINGS=ON ..
cmake --build .

# 安装Python模块
pip install ./python
```

### 问题4: 实时性能不达标

**症状：** 仿真步长超过1ms

**解决：**
```bash
# Linux: 使用PREEMPT_RT内核
uname -v  # 检查是否为PREEMPT_RT内核

# 设置实时优先级
sudo ./bin/vehicle_simulation --realtime --priority 99
```

---

## 📖 下一步学习

### 推荐阅读顺序

1. **README.md** - 项目概览（5分钟）
2. **docs/USER_GUIDE.md** - 用户指南（20分钟）
3. **docs/API_REFERENCE.md** - API参考（30分钟）
4. **docs/ARCHITECTURE.md** - 架构设计（60分钟）

### 进阶主题

- **自定义车辆模型** - 修改车辆参数和动力学方程
- **添加新模块** - 扩展系统功能
- **集成外部工具** - 与MATLAB/Simulink集成
- **性能优化** - 提升仿真性能

---

## 💡 最佳实践

### 1. 版本控制
```bash
# 使用Git管理配置文件
git add config/my_simulation.yaml
git commit -m "添加我的仿真配置"
```

### 2. 结果管理
```bash
# 使用时间戳命名结果文件
./bin/vehicle_simulation --output results_$(date +%Y%m%d_%H%M%S).csv
```

### 3. 批量测试
```bash
# 使用脚本批量运行测试
./scripts/batch_test.sh scenarios/*.yaml
```

### 4. 性能监控
```bash
# 启用性能监控
./bin/vehicle_simulation --monitor

# 查看性能指标
# - 平均步长: 75μs
# - 最大步长: 120μs
# - CPU使用率: 28%
```

---

## 🆘 获取帮助

### 文档资源
- **FAQ:** docs/FAQ.md（32个常见问题）
- **故障排查:** docs/TROUBLESHOOTING.md
- **API文档:** docs/API_REFERENCE.md

### 社区支持
- **GitHub Issues:** 提交问题和建议
- **讨论区:** 技术交流和经验分享

### 专业支持
- **邮件:** support@example.com
- **文档:** https://docs.example.com

---

## ✅ 快速检查清单

开始之前，确保：
- [ ] 已安装所有依赖（CMake, Eigen3）
- [ ] 已成功构建项目
- [ ] 所有测试通过（195/195）
- [ ] 能运行基础仿真
- [ ] 已阅读README.md

---

**准备就绪！开始你的车辆动力学仿真之旅吧！** 🚗💨

---

*最后更新：2026-03-06*
