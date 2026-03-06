# 常见问题（FAQ）

**版本**: 1.0  
**更新日期**: 2026-03-06

---

## 目录

1. [安装与部署](#1-安装与部署)
2. [编译问题](#2-编译问题)
3. [运行问题](#3-运行问题)
4. [性能问题](#4-性能问题)
5. [Python绑定](#5-python绑定)
6. [HIL集成](#6-hil集成)
7. [配置与使用](#7-配置与使用)
8. [仿真精度](#8-仿真精度)

---

## 1. 安装与部署

### Q1.1: 系统支持哪些操作系统？

**A**: 支持以下操作系统：
- **Linux**: Ubuntu 20.04+（推荐，生产环境）
- **Windows**: Windows 10/11（开发环境）
- **macOS**: 10.15+（开发环境）

**推荐**: 生产环境使用Linux + PREEMPT_RT内核以获得最佳实时性能。

---

### Q1.2: 最低硬件要求是什么？

**A**: 
- **CPU**: 4核心，2.5 GHz+
- **内存**: 8 GB RAM
- **存储**: 20 GB可用空间
- **网络**: 以太网接口（HIL集成时需要）

**推荐配置**: 8核心，16 GB RAM，SSD存储

---

### Q1.3: 必须安装哪些依赖库？

**A**: 必需依赖：
- Eigen3 3.3+
- Google Test 1.10+
- pybind11 2.6+
- Python 3.10+

可选依赖：
- yaml-cpp 0.6+（YAML配置解析）
- Boost 1.71+（高级功能）

---

### Q1.4: 如何验证安装是否成功？

**A**: 运行以下命令：

```bash
# 1. 检查编译
ls build/bin/ev_dynamics_simulation

# 2. 检查Python绑定
python3 -c "import sys; sys.path.append('build/python_modules'); import powertrain; print('OK')"

# 3. 运行测试
cd build
ctest --output-on-failure

# 4. 健康检查
./scripts/health_check.sh
```

预期结果：所有测试通过，无错误信息。

---

## 2. 编译问题

### Q2.1: CMake找不到Eigen3怎么办？

**A**: 

**方法1**: 安装系统包（Linux）
```bash
sudo apt install libeigen3-dev
```

**方法2**: 手动指定路径
```bash
cmake .. -DEigen3_DIR=/path/to/eigen3
```

**方法3**: 使用vcpkg
```bash
vcpkg install eigen3:x64-linux
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

### Q2.2: Python绑定编译失败怎么办？

**A**: 

**检查1**: Python版本
```bash
python3 --version  # 必须 >= 3.10
```

**检查2**: pybind11安装
```bash
pip install pybind11
python3 -c "import pybind11; print(pybind11.get_cmake_dir())"
```

**检查3**: 指定Python路径
```bash
cmake .. -DPYTHON_EXECUTABLE=/usr/bin/python3.10
```

---

### Q2.3: Windows编译报错找不到依赖？

**A**: 

**推荐使用vcpkg**:
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

# 重新编译
cmake .. -G "Visual Studio 16 2019" -A x64
```

---

### Q2.4: 编译时内存不足怎么办？

**A**: 

**方法1**: 限制并行编译数
```bash
make -j2  # 只用2个核心
```

**方法2**: 增加交换空间
```bash
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

---

## 3. 运行问题

### Q3.1: 运行时找不到共享库？

**A**: 

**Linux**:
```bash
# 方法1: 临时设置
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# 方法2: 永久设置
sudo ldconfig
```

**Windows**: 将DLL文件路径添加到PATH环境变量

---

### Q3.2: 配置文件找不到？

**A**: 

**检查1**: 文件是否存在
```bash
ls config/simulation_config.yaml
```

**检查2**: 使用绝对路径
```python
vehicle = Vehicle(config='/absolute/path/to/simulation_config.yaml')
```

**检查3**: 设置环境变量
```bash
export EV_DYNAMICS_CONFIG=/path/to/config
```

---

### Q3.3: 程序崩溃或出现Segmentation Fault？

**A**: 

**诊断步骤**:

1. **使用调试版本**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
gdb ./build/bin/ev_dynamics_simulation
```

2. **检查日志**
```bash
tail -f logs/ev_dynamics.log
```

3. **验证参数**
```python
# 确保所有参数在合理范围内
assert vehicle.mass > 0
assert 0 < vehicle.powertrain.motor.max_torque <= 1000
```

4. **减小时间步长**
```python
dt = 0.001  # 从2ms降到1ms
```

---

### Q3.4: 仿真结果不收敛？

**A**: 

**可能原因**:
1. 时间步长过大
2. 参数设置不合理
3. 初始条件冲突

**解决方案**:
```python
# 1. 减小时间步长
dt = 0.001  # 1ms

# 2. 检查参数
vehicle.validate_params()

# 3. 使用默认初始条件
vehicle.reset()

# 4. 添加数值检查
import numpy as np
state = vehicle.get_state()
if np.isnan(state.speed) or np.isinf(state.speed):
    raise ValueError("数值不稳定")
```

---

## 4. 性能问题

### Q4.1: 仿真运行很慢怎么办？

**A**: 

**优化1**: 使用Release模式编译
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**优化2**: 减少日志输出
```yaml
logging:
  level: WARNING  # 从INFO改为WARNING
  console: false
```

**优化3**: 降低数据记录频率
```python
# 每10步记录一次，而不是每步
if i % 10 == 0:
    record_data()
```

**优化4**: 使用C++核心模块
```cpp
// 性能敏感部分使用C++实现
for (int i = 0; i < 10000; i++) {
    vehicle.update(dt);
}
```

---

### Q4.2: 无法满足实时性要求？

**A**: 

**检查1**: 系统配置
```bash
# 检查RT内核
uname -v | grep PREEMPT

# 检查CPU负载
htop
```

**优化2**: 系统调优
```bash
# 禁用不必要的服务
sudo systemctl disable bluetooth

# 设置性能模式
sudo cpupower frequency-set -g performance
```

**优化3**: 调整调度优先级
```bash
# 设置实时优先级
sudo chrt -rr 90 ./build/bin/ev_dynamics_simulation
```

**目标**: 单步更新时间 <2ms（500Hz）

---

### Q4.3: CPU占用率过高？

**A**: 

**诊断**:
```bash
# 查看CPU使用
top -H -p $(pgrep ev_dynamics)

# 查看线程信息
ps -eLf | grep ev_dynamics
```

**优化**:
1. 降低仿真频率（如果可接受）
2. 简化模型复杂度
3. 减少日志和监控频率
4. 优化算法（如使用查表代替实时计算）

---

## 5. Python绑定

### Q5.1: 导入模块失败？

**A**: 

**检查1**: 模块路径
```python
import sys
sys.path.append('build/python_modules')
```

**检查2**: 模块是否生成
```bash
ls build/python_modules/
# 应该看到: powertrain.*.so, chassis.*.so, ...
```

**检查3**: 依赖库
```bash
ldd build/python_modules/powertrain.*.so
# 检查是否有缺失的库
```

---

### Q5.2: Python和C++数据不一致？

**A**: 

**原因**: Python和C++使用不同的内存管理

**解决方案**: 使用副本
```python
# ❌ 错误：直接引用
state = vehicle.get_state()
state.speed = 100  # 不会影响C++对象

# ✅ 正确：通过API修改
vehicle.set_speed(100)
```

---

### Q5.3: Python性能较慢？

**A**: 

**原因**: Python解释器开销

**优化1**: 使用C++核心循环
```cpp
// C++中运行仿真循环
for (int i = 0; i < 10000; i++) {
    vehicle.update(dt);
}
```

**优化2**: 批量操作
```python
# ❌ 慢：逐个更新
for i in range(10000):
    vehicle.update(dt)

# ✅ 快：批量更新
vehicle.run_batch(10000, dt)
```

---

## 6. HIL集成

### Q6.1: CAN通信失败？

**A**: 

**检查1**: CAN接口
```bash
ip link show can0
```

**检查2**: CAN配置
```bash
sudo ip link set can0 up type can bitrate 500000
```

**检查3**: 权限
```bash
sudo usermod -a -G can evdynamics
```

**测试**:
```bash
# 发送测试帧
cansend can0 123#DEADBEEF

# 接收测试帧
candump can0
```

---

### Q6.2: HIL实时性不满足？

**A**: 

**检查1**: 系统延迟
```bash
sudo cyclictest -l100000 -m -Sp90 -i200 -h400 -q
```

**优化2**: CPU隔离
```bash
# 编辑GRUB
GRUB_CMDLINE_LINUX="isolcpus=0,1"

# 绑定HIL进程到隔离CPU
taskset -c 0,1 ./ev_dynamics_simulation
```

**优化3**: 内存锁定
```c
// 在代码中锁定内存
mlockall(MCL_CURRENT | MCL_FUTURE);
```

---

### Q6.3: HIL数据延迟过大？

**A**: 

**诊断**:
```python
import time

start = time.time()
hil.send_can({'test': 123})
data = hil.read_can()
elapsed = (time.time() - start) * 1000

print(f"往返延迟: {elapsed:.2f}ms")
```

**优化**:
1. 使用实时内核
2. 提高进程优先级
3. 减少数据量（如降低采样率）
4. 使用零拷贝技术

**目标**: 往返延迟 <1ms

---

## 7. 配置与使用

### Q7.1: 如何自定义车辆参数？

**A**: 

**方法1**: 修改配置文件
```yaml
vehicle:
  mass: 1600.0  # 修改质量
```

**方法2**: 代码中覆盖
```python
vehicle = Vehicle(
    config='config/simulation_config.yaml',
    mass=1600.0  # 覆盖配置文件
)
```

**方法3**: 运行时修改
```python
vehicle.mass = 1600.0
vehicle.initialize()  # 重新初始化
```

---

### Q7.2: 如何添加自定义驾驶循环？

**A**: 

```python
from ev_dynamics import DrivingCycle

# 创建自定义循环
cycle = DrivingCycle()

# 定义时间-车速序列（秒, km/h）
cycle.set_speed_profile([
    (0.0, 0.0),
    (10.0, 50.0),
    (20.0, 100.0),
    (30.0, 100.0),
    (40.0, 0.0)
])

# 运行循环
results = vehicle.run_cycle(cycle)
```

---

### Q7.3: 如何保存和加载仿真状态？

**A**: 

**保存状态**:
```python
import pickle

state = vehicle.get_state()
with open('vehicle_state.pkl', 'wb') as f:
    pickle.dump(state, f)
```

**加载状态**:
```python
with open('vehicle_state.pkl', 'rb') as f:
    state = pickle.load(f)
    
vehicle.set_state(state)
```

---

### Q7.4: 如何批量运行仿真？

**A**: 

```python
import pandas as pd
from concurrent.futures import ProcessPoolExecutor

def run_simulation(params):
    vehicle = Vehicle(**params)
    sim = Simulation(vehicle, duration=10.0)
    return sim.run()

# 参数组合
param_list = [
    {'mass': 1500, 'motor_torque': 300},
    {'mass': 1600, 'motor_torque': 300},
    {'mass': 1500, 'motor_torque': 350},
]

# 并行运行
with ProcessPoolExecutor(max_workers=4) as executor:
    results = list(executor.map(run_simulation, param_list))

# 保存结果
df = pd.DataFrame([r.__dict__ for r in results])
df.to_csv('batch_results.csv')
```

---

## 8. 仿真精度

### Q8.1: 仿真结果与实车数据差异较大？

**A**: 

**检查1**: 参数校准
- 使用实车测试数据校准参数
- 调整轮胎模型参数（Pacejka系数）
- 验证动力系统效率

**检查2**: 模型精度
```python
# 使用更小的时间步长
dt = 0.001  # 1ms而不是2ms

# 使用更高阶的积分器
integrator = RK4Integrator()  # 而不是欧拉法
```

**检查3**: 环境因素
```yaml
environment:
  friction_coefficient: 0.9  # 根据实际路面调整
  wind_speed: 5.0  # 考虑风速影响
  grade: 0.02  # 考虑坡度
```

---

### Q8.2: 如何验证仿真精度？

**A**: 

**方法1**: 单元测试
```bash
pytest tests/ --cov=src --cov-report=html
```

**方法2**: 对比标准工况
```python
# 运行WLTC循环
results = vehicle.run_cycle(DrivingCycle('WLTC'))

# 对比标准值
expected_energy = 15.5  # kWh/100km
error = abs(results.energy_consumption - expected_energy) / expected_energy

assert error < 0.05, f"误差过大: {error:.2%}"
```

**方法3**: 敏感性分析
```python
# 改变参数，观察影响
base_results = vehicle.run_cycle(cycle)

vehicle.mass *= 1.1
modified_results = vehicle.run_cycle(cycle)

sensitivity = (modified_results.energy - base_results.energy) / base_results.energy
print(f"质量+10% -> 能耗{sensitivity:+.2%}")
```

---

### Q8.3: 数值积分误差如何控制？

**A**: 

**方法1**: 使用高阶积分器
```python
# RK4积分器（4阶精度）
integrator = RK4Integrator()

# 而不是欧拉法（1阶精度）
# integrator = EulerIntegrator()
```

**方法2**: 自适应步长
```python
# 根据误差自动调整步长
dt = adaptive_step(current_error, dt)
```

**方法3**: 误差监控
```python
# 监控能量守恒（物理约束）
initial_energy = vehicle.get_total_energy()

for i in range(10000):
    vehicle.update(dt)
    
    current_energy = vehicle.get_total_energy()
    error = abs(current_energy - initial_energy) / initial_energy
    
    if error > 0.01:  # 1%误差
        print(f"⚠️ 能量误差过大: {error:.2%}")
```

---

## 联系支持

如果以上FAQ未能解决您的问题，请通过以下方式获取支持：

- **文档**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- **问题追踪**: GitHub Issues
- **邮件支持**: support@example.com
- **技术社区**: 论坛/Slack频道

---

**文档版本**: 1.0  
**最后更新**: 2026-03-06  
**维护者**: EV Dynamics Team
