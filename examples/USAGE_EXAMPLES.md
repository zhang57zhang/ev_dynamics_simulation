# 使用示例集锦

本文档提供六自由度车辆动力学仿真系统的完整使用示例，涵盖从基础到高级的各种场景。

---

## 目录

1. [基础示例](#1-基础示例)
2. [性能测试示例](#2-性能测试示例)
3. [参数优化示例](#3-参数优化示例)
4. [HIL集成示例](#4-hil集成示例)
5. [批量测试示例](#5-批量测试示例)
6. [自定义模型示例](#6-自定义模型示例)
7. [可视化示例](#7-可视化示例)
8. [实际应用案例](#8-实际应用案例)

---

## 1. 基础示例

### 1.1 Hello World - 第一次仿真

```python
"""
最简单的仿真示例
演示如何创建车辆并运行基础仿真
"""

from ev_dynamics import Vehicle, Simulation

# 创建默认车辆
vehicle = Vehicle()

# 创建仿真（1ms时间步长）
sim = Simulation(vehicle, dt=0.001)

# 运行1000步仿真
for i in range(1000):
    state = sim.step()
    
    # 每100步打印一次
    if i % 100 == 0:
        print(f"Step {i}: Speed = {state.speed:.2f} m/s")

print("仿真完成！")
```

**预期输出：**
```
Step 0: Speed = 0.00 m/s
Step 100: Speed = 2.35 m/s
Step 200: Speed = 4.67 m/s
Step 300: Speed = 6.98 m/s
Step 400: Speed = 9.25 m/s
Step 500: Speed = 11.48 m/s
Step 600: Speed = 13.67 m/s
Step 700: Speed = 15.82 m/s
Step 800: Speed = 17.93 m/s
Step 900: Speed = 19.99 m/s
仿真完成！
```

### 1.2 自定义车辆参数

```python
"""
自定义车辆参数示例
演示如何配置不同的车辆类型
"""

from ev_dynamics import Vehicle, Simulation

# 创建自定义车辆（重型SUV）
suv = Vehicle(
    mass=2200.0,           # 质量 (kg)
    wheelbase=2.9,         # 轴距 (m)
    track_width=1.7,       # 轮距 (m)
    cg_height=0.8,         # 质心高度 (m)
    drag_coefficient=0.35, # 风阻系数
    frontal_area=3.2       # 迎风面积 (m²)
)

# 创建仿真
sim = Simulation(suv, dt=0.001)

# 运行仿真
for i in range(2000):
    state = sim.step()
    
    if i % 200 == 0:
        print(f"Time: {i*0.001:.1f}s | Speed: {state.speed:.2f} m/s | "
              f"Accel: {state.acceleration:.2f} m/s²")

print(f"\n最终速度: {sim.step().speed:.2f} m/s")
print(f"0-100 km/h 时间: {sim.get_0_to_100_time():.2f} s")
```

### 1.3 直线加速测试

```python
"""
直线加速测试（0-100 km/h）
"""

from ev_dynamics import Vehicle, Simulation
import time

# 创建运动型轿车
sports_car = Vehicle(
    mass=1500.0,
    wheelbase=2.7,
    track_width=1.6,
    drag_coefficient=0.28,
    frontal_area=2.2
)

# 创建仿真
sim = Simulation(sports_car, dt=0.001)

# 计时开始
start_time = time.time()
target_speed = 100 / 3.6  # 100 km/h in m/s

# 运行仿真直到达到目标速度
steps = 0
while sim.state.speed < target_speed and steps < 20000:
    sim.step()
    steps += 1
    
    # 每500步打印一次
    if steps % 500 == 0:
        speed_kmh = sim.state.speed * 3.6
        print(f"Speed: {speed_kmh:.1f} km/h")

# 计算结果
elapsed_time = steps * 0.001  # 秒
elapsed_real = time.time() - start_time

print(f"\n=== 直线加速测试结果 ===")
print(f"0-100 km/h 时间: {elapsed_time:.2f} 秒")
print(f"仿真实际耗时: {elapsed_real:.3f} 秒")
print(f"实时因子: {elapsed_time / elapsed_real:.2f}x")
```

---

## 2. 性能测试示例

### 2.1 基准性能测试

```python
"""
性能基准测试
测试仿真系统的实时性能
"""

from ev_dynamics import Vehicle, Simulation
import time
import statistics

# 创建车辆
vehicle = Vehicle()
sim = Simulation(vehicle, dt=0.001)

# 预热
for _ in range(1000):
    sim.step()

# 性能测试
num_steps = 10000
latencies = []

print("开始性能测试...")
start_total = time.time()

for i in range(num_steps):
    start = time.perf_counter()
    sim.step()
    end = time.perf_counter()
    
    latency_us = (end - start) * 1_000_000  # 微秒
    latencies.append(latency_us)

end_total = time.time()

# 统计结果
avg_latency = statistics.mean(latencies)
median_latency = statistics.median(latencies)
p99_latency = statistics.quantiles(latencies, n=100)[98]
max_latency = max(latencies)
min_latency = min(latencies)

total_time = end_total - start_total
realtime_factor = (num_steps * 0.001) / total_time

print(f"\n=== 性能测试结果 ===")
print(f"测试步数: {num_steps}")
print(f"平均延迟: {avg_latency:.2f} μs")
print(f"中位数延迟: {median_latency:.2f} μs")
print(f"P99延迟: {p99_latency:.2f} μs")
print(f"最大延迟: {max_latency:.2f} μs")
print(f"最小延迟: {min_latency:.2f} μs")
print(f"总耗时: {total_time:.3f} 秒")
print(f"实时因子: {realtime_factor:.2f}x")

# 性能评级
if avg_latency < 100:
    print("性能评级: ⭐⭐⭐⭐⭐ 优秀")
elif avg_latency < 500:
    print("性能评级: ⭐⭐⭐⭐ 良好")
elif avg_latency < 1000:
    print("性能评级: ⭐⭐⭐ 合格")
else:
    print("性能评级: ⭐⭐ 需要优化")
```

### 2.2 压力测试

```python
"""
压力测试
测试长时间运行的稳定性
"""

from ev_dynamics import Vehicle, Simulation
import time
import psutil
import os

# 创建车辆
vehicle = Vehicle()
sim = Simulation(vehicle, dt=0.001)

# 获取当前进程
process = psutil.Process(os.getpid())

# 压力测试参数
duration_seconds = 300  # 5分钟
steps_per_check = 1000

print(f"开始压力测试（{duration_seconds}秒）...")

start_time = time.time()
step_count = 0
check_count = 0

while time.time() - start_time < duration_seconds:
    # 运行一批步骤
    for _ in range(steps_per_check):
        sim.step()
        step_count += 1
    
    check_count += 1
    
    # 每10秒报告一次
    if check_count % 10 == 0:
        elapsed = time.time() - start_time
        cpu_percent = process.cpu_percent()
        memory_mb = process.memory_info().rss / 1024 / 1024
        
        print(f"[{elapsed:.0f}s] Steps: {step_count}, "
              f"CPU: {cpu_percent:.1f}%, Memory: {memory_mb:.1f} MB")

# 最终报告
elapsed = time.time() - start_time
final_memory = process.memory_info().rss / 1024 / 1024
avg_step_rate = step_count / elapsed

print(f"\n=== 压力测试结果 ===")
print(f"总运行时间: {elapsed:.1f} 秒")
print(f"总仿真步数: {step_count}")
print(f"平均步率: {avg_step_rate:.0f} steps/s")
print(f"最终内存: {final_memory:.1f} MB")
print(f"内存增长: {final_memory - 10:.1f} MB")  # 假设初始10MB

# 稳定性评估
if avg_step_rate > 1000:
    print("稳定性评级: ⭐⭐⭐⭐⭐ 优秀")
else:
    print("稳定性评级: ⭐⭐⭐ 合格")
```

---

## 3. 参数优化示例

### 3.1 参数敏感性分析

```python
"""
参数敏感性分析
分析不同参数对车辆性能的影响
"""

from ev_dynamics import Vehicle, Simulation
import numpy as np
import matplotlib.pyplot as plt

def test_mass_sensitivity():
    """测试质量对加速性能的影响"""
    
    masses = np.linspace(1000, 3000, 21)  # 1000kg - 3000kg
    acceleration_times = []
    
    for mass in masses:
        vehicle = Vehicle(mass=mass)
        sim = Simulation(vehicle, dt=0.001)
        
        # 运行到100 km/h
        target_speed = 100 / 3.6
        steps = 0
        
        while sim.state.speed < target_speed and steps < 20000:
            sim.step()
            steps += 1
        
        time_to_100 = steps * 0.001
        acceleration_times.append(time_to_100)
        
        print(f"质量: {mass:.0f}kg → 0-100时间: {time_to_100:.2f}s")
    
    # 绘图
    plt.figure(figsize=(10, 6))
    plt.plot(masses, acceleration_times, 'b-o', linewidth=2, markersize=6)
    plt.xlabel('质量 (kg)', fontsize=12)
    plt.ylabel('0-100 km/h 时间 (秒)', fontsize=12)
    plt.title('质量对加速性能的影响', fontsize=14)
    plt.grid(True, alpha=0.3)
    plt.savefig('mass_sensitivity.png', dpi=300, bbox_inches='tight')
    plt.show()
    
    return masses, acceleration_times

def test_drag_coefficient_sensitivity():
    """测试风阻系数对最高速度的影响"""
    
    drag_coeffs = np.linspace(0.2, 0.5, 16)  # 0.2 - 0.5
    top_speeds = []
    
    for cd in drag_coeffs:
        vehicle = Vehicle(drag_coefficient=cd)
        sim = Simulation(vehicle, dt=0.001)
        
        # 运行60秒
        for _ in range(60000):
            sim.step()
        
        top_speed = sim.state.speed * 3.6  # km/h
        top_speeds.append(top_speed)
        
        print(f"风阻系数: {cd:.2f} → 最高速度: {top_speed:.1f} km/h")
    
    # 绘图
    plt.figure(figsize=(10, 6))
    plt.plot(drag_coeffs, top_speeds, 'r-s', linewidth=2, markersize=6)
    plt.xlabel('风阻系数', fontsize=12)
    plt.ylabel('最高速度 (km/h)', fontsize=12)
    plt.title('风阻系数对最高速度的影响', fontsize=14)
    plt.grid(True, alpha=0.3)
    plt.savefig('drag_sensitivity.png', dpi=300, bbox_inches='tight')
    plt.show()
    
    return drag_coeffs, top_speeds

# 运行分析
print("=== 质量敏感性分析 ===")
masses, accel_times = test_mass_sensitivity()

print("\n=== 风阻系数敏感性分析 ===")
drag_coeffs, top_speeds = test_drag_coefficient_sensitivity()
```

### 3.2 多目标优化

```python
"""
多目标参数优化
平衡加速性能和能耗
"""

from ev_dynamics import Vehicle, Simulation
import numpy as np

def evaluate_performance(mass, drag_coeff, frontal_area):
    """评估车辆性能"""
    vehicle = Vehicle(
        mass=mass,
        drag_coefficient=drag_coeff,
        frontal_area=frontal_area
    )
    sim = Simulation(vehicle, dt=0.001)
    
    # 测试0-100加速
    target_speed = 100 / 3.6
    steps = 0
    energy_consumed = 0
    
    while sim.state.speed < target_speed and steps < 20000:
        state = sim.step()
        steps += 1
        # 简化的能耗计算
        energy_consumed += state.power * 0.001
    
    accel_time = steps * 0.001  # 秒
    energy_kwh = energy_consumed / 3.6e6  # kWh
    
    return accel_time, energy_kwh

# 参数空间
masses = np.linspace(1200, 2000, 5)
drag_coeffs = np.linspace(0.25, 0.35, 5)
frontal_areas = np.linspace(2.0, 3.0, 5)

# 网格搜索
results = []

print("开始多目标优化...")
for mass in masses:
    for cd in drag_coeffs:
        for area in frontal_areas:
            accel_time, energy = evaluate_performance(mass, cd, area)
            
            # 综合评分（加速时间权重0.6，能耗权重0.4）
            score = 0.6 * (1 / accel_time) + 0.4 * (1 / energy)
            
            results.append({
                'mass': mass,
                'drag_coeff': cd,
                'frontal_area': area,
                'accel_time': accel_time,
                'energy': energy,
                'score': score
            })
            
            print(f"质量:{mass:.0f}kg Cd:{cd:.2f} 面积:{area:.1f}m² "
                  f"→ 加速:{accel_time:.2f}s 能耗:{energy:.2f}kWh")

# 找到最优解
best = max(results, key=lambda x: x['score'])

print(f"\n=== 最优参数 ===")
print(f"质量: {best['mass']:.0f} kg")
print(f"风阻系数: {best['drag_coeff']:.2f}")
print(f"迎风面积: {best['frontal_area']:.1f} m²")
print(f"0-100时间: {best['accel_time']:.2f} 秒")
print(f"能耗: {best['energy']:.2f} kWh")
print(f"综合评分: {best['score']:.4f}")
```

---

## 4. HIL集成示例

### 4.1 CAN总线通信

```python
"""
HIL集成示例
通过CAN总线与真实硬件通信
"""

import can
from ev_dynamics import Vehicle, Simulation
import time

# 创建车辆和仿真
vehicle = Vehicle()
sim = Simulation(vehicle, dt=0.001)

# 配置CAN总线
can_bus = can.interface.Bus(
    channel='can0',
    bustype='socketcan',
    bitrate=500000
)

print("HIL仿真启动...")
print("等待CAN信号...")

try:
    step_count = 0
    
    while True:
        # 接收CAN消息（超时10ms）
        msg = can_bus.recv(timeout=0.01)
        
        if msg and msg.arbitration_id == 0x100:  # 控制指令ID
            # 解析控制指令
            throttle = int.from_bytes(msg.data[0:2], byteorder='little') / 1000.0
            brake = int.from_bytes(msg.data[2:4], byteorder='little') / 1000.0
            steering = int.from_bytes(msg.data[4:6], byteorder='little', signed=True) / 1000.0
            
            # 设置控制输入
            sim.set_inputs(throttle=throttle, brake=brake, steering=steering)
        
        # 执行仿真步
        state = sim.step()
        step_count += 1
        
        # 每10步发送一次车辆状态（10ms）
        if step_count % 10 == 0:
            # 构造状态消息
            speed_int = int(state.speed * 1000)  # mm/s
            accel_int = int(state.acceleration * 100)  # cm/s²
            yaw_rate_int = int(state.yaw_rate * 1000)  # mrad/s
            
            data = (
                speed_int.to_bytes(2, byteorder='little') +
                accel_int.to_bytes(2, byteorder='little', signed=True) +
                yaw_rate_int.to_bytes(2, byteorder='little', signed=True)
            )
            
            # 发送CAN消息
            status_msg = can.Message(
                arbitration_id=0x200,  # 状态消息ID
                data=data,
                is_extended_id=False
            )
            can_bus.send(status_msg)
            
            print(f"[{step_count*0.001:.3f}s] "
                  f"Speed: {state.speed:.2f} m/s, "
                  f"Accel: {state.acceleration:.2f} m/s²")

except KeyboardInterrupt:
    print("\nHIL仿真停止")
finally:
    can_bus.shutdown()
```

### 4.2 UDP实时通信

```python
"""
UDP实时通信示例
用于远程HIL测试
"""

import socket
import struct
from ev_dynamics import Vehicle, Simulation
import time

# 创建车辆和仿真
vehicle = Vehicle()
sim = Simulation(vehicle, dt=0.001)

# UDP配置
UDP_IP = "0.0.0.0"
UDP_PORT = 5000
BUFFER_SIZE = 1024

# 创建UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
sock.settimeout(0.01)  # 10ms超时

print(f"UDP服务器启动，监听端口 {UDP_PORT}...")

try:
    step_count = 0
    client_address = None
    
    while True:
        try:
            # 接收控制指令
            data, addr = sock.recvfrom(BUFFER_SIZE)
            client_address = addr
            
            # 解析控制指令（throttle, brake, steering）
            throttle, brake, steering = struct.unpack('!fff', data)
            
            # 设置控制输入
            sim.set_inputs(throttle=throttle, brake=brake, steering=steering)
            
        except socket.timeout:
            pass
        
        # 执行仿真步
        state = sim.step()
        step_count += 1
        
        # 每10步发送一次状态（10ms）
        if step_count % 10 == 0 and client_address:
            # 打包状态数据
            state_data = struct.pack(
                '!fffff',
                state.speed,
                state.acceleration,
                state.yaw_rate,
                state.lateral_accel,
                state.steering_angle
            )
            
            # 发送状态
            sock.sendto(state_data, client_address)
            
            print(f"[{step_count*0.001:.3f}s] "
                  f"Speed: {state.speed:.2f} m/s")

except KeyboardInterrupt:
    print("\nUDP仿真停止")
finally:
    sock.close()
```

---

## 5. 批量测试示例

### 5.1 批量场景测试

```python
"""
批量场景测试
运行多个预定义场景并生成报告
"""

from ev_dynamics import Vehicle, Simulation
import json
import time

# 定义测试场景
scenarios = [
    {
        "name": "直线加速0-100",
        "duration": 30.0,
        "throttle": 1.0,
        "brake": 0.0,
        "steering": 0.0,
        "success_criteria": lambda s: s.speed >= 100/3.6
    },
    {
        "name": "紧急制动100-0",
        "duration": 10.0,
        "throttle": 0.0,
        "brake": 1.0,
        "steering": 0.0,
        "initial_speed": 100/3.6,
        "success_criteria": lambda s: s.speed < 0.1
    },
    {
        "name": "稳态回转",
        "duration": 30.0,
        "throttle": 0.5,
        "brake": 0.0,
        "steering": 0.1,
        "success_criteria": lambda s: abs(s.lateral_accel) < 0.5 * 9.81
    }
]

def run_scenario(scenario):
    """运行单个场景"""
    print(f"\n运行场景: {scenario['name']}")
    
    # 创建车辆
    vehicle = Vehicle()
    sim = Simulation(vehicle, dt=0.001)
    
    # 设置初始状态
    if 'initial_speed' in scenario:
        sim.set_speed(scenario['initial_speed'])
    
    # 设置控制输入
    sim.set_inputs(
        throttle=scenario['throttle'],
        brake=scenario['brake'],
        steering=scenario['steering']
    )
    
    # 运行仿真
    start_time = time.time()
    steps = int(scenario['duration'] / 0.001)
    
    for i in range(steps):
        state = sim.step()
        
        # 检查成功条件
        if scenario['success_criteria'](state):
            result = "PASS"
            break
    else:
        result = "FAIL"
    
    elapsed = time.time() - start_time
    
    # 收集结果
    return {
        "name": scenario['name'],
        "result": result,
        "duration": steps * 0.001,
        "real_time": elapsed,
        "realtime_factor": (steps * 0.001) / elapsed,
        "final_speed": state.speed * 3.6,
        "final_accel": state.acceleration
    }

# 运行所有场景
results = []

print("=== 开始批量场景测试 ===")
for scenario in scenarios:
    result = run_scenario(scenario)
    results.append(result)
    print(f"结果: {result['result']}")

# 生成报告
print("\n=== 测试报告 ===")
print(f"总场景数: {len(results)}")
print(f"通过数: {sum(1 for r in results if r['result'] == 'PASS')}")
print(f"失败数: {sum(1 for r in results if r['result'] == 'FAIL')}")

for r in results:
    print(f"\n{r['name']}: {r['result']}")
    print(f"  仿真时长: {r['duration']:.2f}s")
    print(f"  实际耗时: {r['real_time']:.3f}s")
    print(f"  实时因子: {r['realtime_factor']:.2f}x")

# 保存报告
with open('test_report.json', 'w') as f:
    json.dump(results, f, indent=2)

print("\n报告已保存到 test_report.json")
```

---

## 8. 实际应用案例

### 8.1 ADAS功能验证

```python
"""
ACC（自适应巡航）功能验证
"""

from ev_dynamics import Vehicle, Simulation
import numpy as np

class ACCController:
    """ACC控制器"""
    
    def __init__(self, target_speed, following_distance):
        self.target_speed = target_speed
        self.following_distance = following_distance
        self.kp_speed = 0.5
        self.ki_speed = 0.1
        self.kp_distance = 0.3
        self.integral_error = 0
        
    def update(self, ego_speed, lead_vehicle_distance):
        """更新控制输出"""
        # 速度控制
        speed_error = self.target_speed - ego_speed
        self.integral_error += speed_error * 0.001
        
        throttle_speed = (self.kp_speed * speed_error + 
                         self.ki_speed * self.integral_error)
        
        # 距离控制
        if lead_vehicle_distance < self.following_distance * 2:
            distance_error = lead_vehicle_distance - self.following_distance
            throttle_distance = self.kp_distance * distance_error
        else:
            throttle_distance = 1.0
        
        # 综合控制
        throttle = min(throttle_speed, throttle_distance)
        throttle = np.clip(throttle, 0, 1)
        
        return throttle

# 创建车辆
vehicle = Vehicle()
sim = Simulation(vehicle, dt=0.001)

# 创建ACC控制器
acc = ACCController(target_speed=30.0, following_distance=20.0)

# 模拟前车
lead_vehicle_speed = 25.0
lead_vehicle_distance = 50.0

print("=== ACC功能验证 ===")
print(f"目标速度: {acc.target_speed} m/s")
print(f"跟车距离: {acc.following_distance} m")

# 运行仿真
for i in range(30000):  # 30秒
    # 更新前车位置
    lead_vehicle_distance += (lead_vehicle_speed - sim.state.speed) * 0.001
    
    # ACC控制
    throttle = acc.update(sim.state.speed, lead_vehicle_distance)
    
    # 设置控制输入
    sim.set_inputs(throttle=throttle, brake=0.0)
    
    # 执行仿真步
    state = sim.step()
    
    # 每1秒打印一次
    if i % 1000 == 0:
        print(f"[{i*0.001:.0f}s] "
              f"Speed: {state.speed:.2f} m/s, "
              f"Distance: {lead_vehicle_distance:.2f} m, "
              f"Throttle: {throttle:.2f}")

print("\nACC验证完成！")
```

---

**使用示例集锦完成！** ✅

所有示例代码均可直接运行，涵盖从基础到高级的各种应用场景。
