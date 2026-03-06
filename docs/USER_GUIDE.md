# EV Dynamics Simulation - 用户指南

**版本**: 1.0  
**更新日期**: 2026-03-06  
**适用对象**: 仿真工程师、测试工程师、研究人员

---

## 目录

1. [快速开始](#1-快速开始)
2. [基本使用](#2-基本使用)
3. [高级功能](#3-高级功能)
4. [示例代码](#4-示例代码)
5. [故障排查](#5-故障排查)
6. [最佳实践](#6-最佳实践)

---

## 1. 快速开始

### 1.1 环境准备

确保已按照 [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md) 完成系统部署。

#### 验证安装

```bash
# 检查Python绑定
python3 -c "import sys; sys.path.append('build/python_modules'); import powertrain; print('✅ 安装成功')"

# 检查配置文件
ls config/simulation_config.yaml
```

### 1.2 第一个仿真

#### Python版本

```python
import sys
sys.path.append('build/python_modules')

from powertrain import Motor, Battery
from dynamics import VehicleDynamics
import numpy as np

# 创建电机模型
motor = Motor(max_torque=300.0, max_speed=15000.0)

# 创建电池模型
battery = Battery(capacity=60.0, max_power=150.0)

# 设置初始条件
motor.set_torque_request(100.0)  # 请求100Nm扭矩

# 仿真循环
dt = 0.002  # 2ms时间步长
for i in range(5000):  # 10秒仿真
    motor.update(dt)
    battery.update(dt, power_demand=50.0)
    
    if i % 500 == 0:  # 每1秒打印一次
        print(f"时间: {i*dt:.1f}s, SOC: {battery.get_soc():.2f}%")

print("✅ 仿真完成！")
```

#### C++版本

```cpp
#include <ev_dynamics/vehicle_dynamics.h>
#include <ev_dynamics/powertrain/motor.h>
#include <ev_dynamics/powertrain/battery.h>
#include <iostream>

int main() {
    // 创建模型
    ev_dynamics::powertrain::Motor motor(300.0, 15000.0);
    ev_dynamics::powertrain::Battery battery(60.0, 150.0);
    
    // 设置扭矩请求
    motor.set_torque_request(100.0);
    
    // 仿真循环
    double dt = 0.002;
    for (int i = 0; i < 5000; i++) {
        motor.update(dt);
        battery.update(dt, 50.0);
        
        if (i % 500 == 0) {
            std::cout << "时间: " << i * dt << "s, "
                      << "SOC: " << battery.get_soc() << "%"
                      << std::endl;
        }
    }
    
    return 0;
}
```

---

## 2. 基本使用

### 2.1 加载配置文件

#### 使用YAML配置

```python
import yaml
from ev_dynamics import Vehicle

# 加载配置
with open('config/simulation_config.yaml', 'r') as f:
    config = yaml.safe_load(f)

# 创建车辆
vehicle = Vehicle(config=config)

# 或者直接指定配置文件路径
vehicle = Vehicle(config='config/simulation_config.yaml')
```

#### 动态修改配置

```python
# 修改车辆质量
vehicle.mass = 1600.0

# 修改电机参数
vehicle.powertrain.motor.max_torque = 350.0

# 重新初始化
vehicle.initialize()
```

### 2.2 仿真循环

#### 基本仿真

```python
# 创建仿真对象
from ev_dynamics import Simulation

sim = Simulation(
    vehicle=vehicle,
    duration=10.0,    # 10秒
    frequency=500     # 500Hz
)

# 运行仿真
results = sim.run()

# 查看结果
print(f"平均能耗: {results.avg_energy_consumption} kWh/100km")
print(f"最高车速: {results.max_speed} km/h")
```

#### 自定义控制输入

```python
# 设置驾驶员输入
sim.set_driver_input(
    time=0.0,
    throttle=0.5,    # 50%油门
    brake=0.0,
    steering=0.0
)

sim.set_driver_input(
    time=5.0,
    throttle=0.0,
    brake=0.3,       # 30%制动
    steering=0.0
)

# 运行仿真
results = sim.run()
```

### 2.3 数据记录与分析

#### 启用数据记录

```python
# 配置输出
sim.configure_output(
    enabled=True,
    format='csv',
    path='results/',
    prefix='sim_20260306_'
)

# 运行仿真
results = sim.run()

# 结果会保存到 results/sim_20260306_*.csv
```

#### 分析结果

```python
import pandas as pd
import matplotlib.pyplot as plt

# 加载结果数据
df = pd.read_csv('results/sim_20260306_data.csv')

# 绘制车速曲线
plt.figure(figsize=(10, 6))
plt.plot(df['time'], df['speed'], label='车速 (m/s)')
plt.xlabel('时间 (s)')
plt.ylabel('车速 (m/s)')
plt.title('车速-时间曲线')
plt.legend()
plt.grid(True)
plt.savefig('speed_curve.png')
plt.show()

# 计算统计数据
print(f"平均车速: {df['speed'].mean():.2f} m/s")
print(f"最高车速: {df['speed'].max():.2f} m/s")
print(f"平均加速度: {df['acceleration'].mean():.2f} m/s^2")
```

---

## 3. 高级功能

### 3.1 自定义车辆参数

#### 完整车辆定义

```python
from ev_dynamics import Vehicle

vehicle = Vehicle(
    # 基本参数
    mass=1500.0,
    length=4.5,
    width=1.8,
    height=1.4,
    
    # 质心位置
    cg_to_front=1.2,
    cg_to_rear=1.5,
    cg_height=0.5,
    
    # 转动惯量
    inertia={
        'ix': 500.0,   # 侧倾惯量
        'iy': 2000.0,  # 俯仰惯量
        'iz': 2500.0   # 横摆惯量
    },
    
    # 动力系统
    powertrain={
        'motor': {
            'max_torque': 300.0,
            'max_speed': 15000.0,
            'efficiency': 0.95
        },
        'battery': {
            'capacity': 60.0,
            'max_power': 150.0,
            'efficiency': 0.92
        }
    },
    
    # 底盘系统
    chassis={
        'suspension_front': {
            'stiffness': 30000.0,
            'damping': 3000.0
        },
        'suspension_rear': {
            'stiffness': 35000.0,
            'damping': 3500.0
        }
    }
)
```

### 3.2 驾驶循环测试

#### 加载标准驾驶循环

```python
from ev_dynamics import DrivingCycle

# 加载WLTC循环
cycle = DrivingCycle('WLTC')

# 查看循环信息
print(f"循环时长: {cycle.duration} 秒")
print(f"平均车速: {cycle.avg_speed} km/h")
print(f"最高车速: {cycle.max_speed} km/h")

# 运行循环
results = vehicle.run_cycle(cycle)

# 分析结果
print(f"能耗: {results.energy_consumption:.2f} kWh/100km")
print(f"续航里程: {results.range:.2f} km")
print(f"能量回收: {results.regenerated_energy:.2f} kWh")
```

#### 自定义驾驶循环

```python
# 创建自定义循环
custom_cycle = DrivingCycle()

# 定义时间-车速序列
custom_cycle.set_speed_profile([
    (0.0, 0.0),      # 0s, 0 km/h
    (5.0, 50.0),     # 5s, 50 km/h
    (10.0, 100.0),   # 10s, 100 km/h
    (15.0, 100.0),   # 15s, 100 km/h
    (20.0, 0.0)      # 20s, 0 km/h
])

# 运行自定义循环
results = vehicle.run_cycle(custom_cycle)
```

### 3.3 HIL集成

#### 配置CAN接口

```python
from ev_dynamics import HILInterface

# 配置CAN接口
hil = HILInterface(
    can_interface='can0',
    can_bitrate=500000,
    enable_logging=True
)

# 启动接口
hil.start()
```

#### 实时仿真循环

```python
import time

# 实时仿真
dt = 0.002  # 2ms
frequency = 500  # 500Hz

try:
    while True:
        start_time = time.time()
        
        # 读取CAN数据
        can_data = hil.read_can()
        
        # 解析驾驶员输入
        throttle = can_data['throttle']
        brake = can_data['brake']
        steering = can_data['steering']
        
        # 更新车辆状态
        vehicle.set_pedal(throttle)
        vehicle.set_brake(brake)
        vehicle.set_steering(steering)
        vehicle.update(dt)
        
        # 获取车辆状态
        state = vehicle.get_state()
        
        # 发送CAN数据
        hil.send_can({
            'speed': state.speed,
            'acceleration': state.acceleration,
            'soc': state.battery_soc
        })
        
        # 实时性保障
        elapsed = time.time() - start_time
        if elapsed < dt:
            time.sleep(dt - elapsed)
        else:
            print(f"⚠️ 超时: {elapsed*1000:.2f}ms > {dt*1000:.2f}ms")
            
except KeyboardInterrupt:
    print("\n仿真结束")
    hil.stop()
```

### 3.4 参数敏感性分析

```python
import numpy as np
import matplotlib.pyplot as plt

# 参数范围
mass_range = np.linspace(1200, 2000, 10)  # 1200-2000kg
results_list = []

for mass in mass_range:
    # 修改参数
    vehicle.mass = mass
    vehicle.initialize()
    
    # 运行仿真
    results = vehicle.run_cycle(cycle)
    
    results_list.append({
        'mass': mass,
        'energy': results.energy_consumption,
        'range': results.range
    })

# 绘制敏感性曲线
df = pd.DataFrame(results_list)

plt.figure(figsize=(12, 5))

plt.subplot(1, 2, 1)
plt.plot(df['mass'], df['energy'], 'b-o')
plt.xlabel('整车质量 (kg)')
plt.ylabel('能耗 (kWh/100km)')
plt.title('质量-能耗敏感性')
plt.grid(True)

plt.subplot(1, 2, 2)
plt.plot(df['mass'], df['range'], 'r-o')
plt.xlabel('整车质量 (kg)')
plt.ylabel('续航里程 (km)')
plt.title('质量-续航敏感性')
plt.grid(True)

plt.tight_layout()
plt.savefig('sensitivity_analysis.png')
plt.show()
```

---

## 4. 示例代码

### 4.1 加速测试（0-100 km/h）

```python
from ev_dynamics import Vehicle
import time

# 创建车辆
vehicle = Vehicle(config='config/simulation_config.yaml')

# 初始化
vehicle.initialize()
vehicle.set_pedal(1.0)  # 全油门

# 仿真循环
dt = 0.002
start_time = time.time()
distance = 0.0

while vehicle.speed < 100 / 3.6:  # 100 km/h = 27.78 m/s
    vehicle.update(dt)
    distance += vehicle.speed * dt

elapsed_time = time.time() - start_time

print(f"✅ 0-100 km/h 加速测试完成")
print(f"加速时间: {vehicle.time:.2f} 秒")
print(f"加速距离: {distance:.2f} 米")
print(f"仿真耗时: {elapsed_time:.3f} 秒")
```

### 4.2 制动测试（100-0 km/h）

```python
from ev_dynamics import Vehicle

vehicle = Vehicle(config='config/simulation_config.yaml')

# 设置初始车速
vehicle.set_speed(100 / 3.6)  # 100 km/h
vehicle.set_brake(1.0)  # 全制动

# 仿真循环
dt = 0.002
distance = 0.0

while vehicle.speed > 0.1:  # 接近停止
    vehicle.update(dt)
    distance += vehicle.speed * dt

print(f"✅ 100-0 km/h 制动测试完成")
print(f"制动距离: {distance:.2f} 米")
print(f"制动时间: {vehicle.time:.2f} 秒")
print(f"平均减速度: {(100/3.6)/vehicle.time:.2f} m/s^2")
```

### 4.3 续航里程测试

```python
from ev_dynamics import Vehicle, DrivingCycle

vehicle = Vehicle(config='config/simulation_config.yaml')

# 加载CLTC循环
cycle = DrivingCycle('CLTC')

# 初始化SOC
vehicle.powertrain.battery.soc = 0.9  # 90%

# 运行循环直到SOC < 10%
total_distance = 0.0
cycle_count = 0

while vehicle.powertrain.battery.soc > 0.1:
    results = vehicle.run_cycle(cycle)
    total_distance += results.distance / 1000  # km
    cycle_count += 1
    
    print(f"循环 {cycle_count}: SOC={vehicle.powertrain.battery.soc:.1%}, "
          f"累计里程={total_distance:.1f}km")

print(f"\n✅ 续航测试完成")
print(f"总续航里程: {total_distance:.1f} km")
print(f"平均能耗: {vehicle.powertrain.battery.capacity * 0.8 / total_distance * 100:.2f} kWh/100km")
```

### 4.4 操稳性测试（双移线）

```python
from ev_dynamics import Vehicle
import numpy as np

vehicle = Vehicle(config='config/simulation_config.yaml')
vehicle.initialize()

# 设置初始车速 80 km/h
vehicle.set_speed(80 / 3.6)

# 双移线轨迹
def double_lane_change_steering(t):
    """ISO 3888 双移线工况"""
    if t < 1.0:
        return 0.0
    elif t < 2.0:
        return 0.1  # 左转
    elif t < 3.0:
        return -0.1  # 右转
    elif t < 4.0:
        return 0.05  # 回正
    else:
        return 0.0

# 仿真循环
dt = 0.002
results = []

for i in range(5000):  # 10秒
    t = i * dt
    
    # 设置转向角
    steering = double_lane_change_steering(t)
    vehicle.set_steering(steering)
    vehicle.update(dt)
    
    # 记录数据
    state = vehicle.get_state()
    results.append({
        'time': t,
        'steering': steering,
        'lateral_accel': state.lateral_acceleration,
        'yaw_rate': state.yaw_rate
    })

# 分析结果
df = pd.DataFrame(results)
max_lateral_accel = df['lateral_accel'].max()
max_yaw_rate = df['yaw_rate'].max()

print(f"✅ 双移线测试完成")
print(f"最大侧向加速度: {max_lateral_accel:.2f} m/s^2")
print(f"最大横摆角速度: {max_yaw_rate:.2f} deg/s")
```

---

## 5. 故障排查

### 5.1 仿真不稳定

**症状**: 仿真过程中数值发散，出现NaN或Inf

**可能原因**:
1. 时间步长过大
2. 参数设置不合理
3. 初始条件冲突

**解决方案**:

```python
# 1. 减小时间步长
dt = 0.001  # 从2ms降到1ms

# 2. 检查参数合理性
assert vehicle.mass > 0, "质量必须大于0"
assert vehicle.powertrain.motor.max_torque > 0, "最大扭矩必须大于0"

# 3. 添加数值稳定性检查
def safe_update(vehicle, dt):
    vehicle.update(dt)
    state = vehicle.get_state()
    
    # 检查NaN
    if np.isnan(state.speed) or np.isinf(state.speed):
        raise ValueError(f"数值不稳定: speed={state.speed}")
    
    return state

# 4. 使用try-except捕获异常
try:
    for i in range(5000):
        state = safe_update(vehicle, dt)
except ValueError as e:
    print(f"❌ 仿真失败: {e}")
    # 保存现场数据
    vehicle.save_state('debug_state.pkl')
```

### 5.2 性能问题

**症状**: 仿真运行缓慢，无法满足实时性要求

**诊断步骤**:

```python
import time
import cProfile

# 1. 性能分析
def profile_simulation():
    vehicle = Vehicle(config='config/simulation_config.yaml')
    for i in range(1000):
        vehicle.update(0.002)

cProfile.run('profile_simulation()', 'simulation.prof')

# 2. 查看性能热点
import pstats
p = pstats.Stats('simulation.prof')
p.sort_stats('cumulative')
p.print_stats(10)

# 3. 时间测量
start = time.time()
for i in range(1000):
    vehicle.update(0.002)
elapsed = time.time() - start

avg_time = elapsed / 1000 * 1000  # ms
print(f"平均更新时间: {avg_time:.3f} ms")
print(f"实时倍率: {2.0 / avg_time:.1f}x")
```

**优化建议**:
- 使用Release模式编译（`-O3`）
- 减少不必要的日志输出
- 批量处理数据记录
- 使用C++核心模块

### 5.3 HIL通信问题

**症状**: CAN通信失败或数据延迟

**诊断代码**:

```python
from ev_dynamics import HILInterface
import time

hil = HILInterface('can0', 500000)

# 测试通信
test_data = {'test': 123}
start = time.time()

hil.send_can(test_data)
received = hil.read_can(timeout=1.0)

elapsed = (time.time() - start) * 1000

if received:
    print(f"✅ CAN通信正常，延迟: {elapsed:.2f}ms")
else:
    print(f"❌ CAN通信失败")

# 检查接口状态
stats = hil.get_statistics()
print(f"发送帧数: {stats['tx_count']}")
print(f"接收帧数: {stats['rx_count']}")
print(f"错误帧数: {stats['error_count']}")
```

---

## 6. 最佳实践

### 6.1 配置管理

```python
# ✅ 好的做法：使用配置文件
vehicle = Vehicle(config='config/simulation_config.yaml')

# ❌ 不好的做法：硬编码参数
vehicle = Vehicle(mass=1500, motor_torque=300, ...)

# ✅ 参数验证
def validate_config(config):
    assert config['vehicle']['mass'] > 0
    assert 0 < config['powertrain']['battery']['capacity'] <= 200
    assert config['simulation']['frequency'] >= 100

validate_config(config)
```

### 6.2 数据记录

```python
# ✅ 好的做法：结构化记录
import pandas as pd

results = []
for i in range(5000):
    state = vehicle.update(dt)
    results.append({
        'time': i * dt,
        'speed': state.speed,
        'acceleration': state.acceleration,
        'soc': state.battery_soc
    })

df = pd.DataFrame(results)
df.to_csv('results/simulation_data.csv', index=False)

# ❌ 不好的做法：无格式输出
for i in range(5000):
    print(f"{i}: {vehicle.speed}")
```

### 6.3 错误处理

```python
# ✅ 好的做法：完善的错误处理
try:
    results = vehicle.run_cycle(cycle)
    vehicle.validate_results(results)
    vehicle.save_results(results, 'output/')
except ValueError as e:
    print(f"参数错误: {e}")
    vehicle.load_default_config()
except RuntimeError as e:
    print(f"运行错误: {e}")
    vehicle.save_debug_info('debug/')
    raise
finally:
    vehicle.cleanup()

# ❌ 不好的做法：无错误处理
results = vehicle.run_cycle(cycle)
```

### 6.4 性能优化

```python
# ✅ 好的做法：批量处理
states = []
for i in range(5000):
    vehicle.update(dt)
    if i % 100 == 0:  # 每100步记录一次
        states.append(vehicle.get_state())

# ❌ 不好的做法：频繁IO
for i in range(5000):
    vehicle.update(dt)
    with open('log.txt', 'a') as f:  # 每步都写文件
        f.write(f"{vehicle.speed}\n")
```

---

## 附录

### A. 常用单位换算

| 物理量 | 国际单位 | 常用单位 | 换算关系 |
|--------|---------|---------|---------|
| 速度 | m/s | km/h | 1 m/s = 3.6 km/h |
| 扭矩 | N·m | - | - |
| 功率 | W | kW | 1 kW = 1000 W |
| 能耗 | Wh/km | kWh/100km | - |
| 角度 | rad | deg | 1 rad = 57.3° |

### B. 常见错误代码

| 错误码 | 说明 | 解决方案 |
|--------|------|---------|
| E001 | 配置文件未找到 | 检查文件路径 |
| E002 | 参数超出范围 | 检查参数取值 |
| E003 | 数值不稳定 | 减小时间步长 |
| E004 | CAN通信失败 | 检查硬件连接 |
| E005 | 内存不足 | 减少数据记录频率 |

### C. 参考资源

- **API文档**: [API_REFERENCE.md](API_REFERENCE.md)
- **故障排查**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- **常见问题**: [FAQ.md](FAQ.md)
- **架构文档**: [ARCHITECTURE.md](ARCHITECTURE.md)

---

**文档版本**: 1.0  
**最后更新**: 2026-03-06  
**维护者**: EV Dynamics Team
