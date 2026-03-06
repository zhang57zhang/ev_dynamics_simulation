# API参考文档

**版本**: 1.0  
**更新日期**: 2026-03-06  
**API版本**: v1.0.0

---

## 目录

1. [Python API](#1-python-api)
2. [C++ API](#2-c-api)
3. [数据结构](#3-数据结构)
4. [错误处理](#4-错误处理)

---

## 1. Python API

### 1.1 Vehicle类

#### 构造函数

```python
Vehicle(config: str = None, **kwargs)
```

**功能**: 创建车辆动力学仿真对象

**参数**:
- `config` (str, optional): 配置文件路径（YAML格式）
- `**kwargs`: 车辆参数（覆盖配置文件）

**示例**:

```python
# 方式1：使用配置文件
vehicle = Vehicle(config='config/simulation_config.yaml')

# 方式2：使用参数字典
vehicle = Vehicle(
    mass=1500.0,
    inertia={'ix': 500, 'iy': 2000, 'iz': 2500},
    powertrain={'motor_max_torque': 300}
)

# 方式3：混合模式
vehicle = Vehicle(
    config='config/simulation_config.yaml',
    mass=1600.0  # 覆盖配置文件中的mass
)
```

---

#### 方法

##### update()

```python
update(dt: float) -> None
```

**功能**: 更新车辆状态一个时间步

**参数**:
- `dt` (float): 时间步长（秒），推荐0.002s（2ms）

**返回**: None

**示例**:

```python
dt = 0.002  # 2ms
for i in range(5000):
    vehicle.update(dt)
```

**注意**:
- `dt` 必须为正数
- 建议保持 `dt` 恒定以确保仿真精度
- 推荐使用500Hz频率（dt=0.002s）

---

##### set_pedal()

```python
set_pedal(pedal: float) -> None
```

**功能**: 设置加速踏板位置

**参数**:
- `pedal` (float): 踏板位置，范围 [0.0, 1.0]
  - 0.0 = 完全松开
  - 1.0 = 完全踩下

**示例**:

```python
vehicle.set_pedal(0.5)   # 50%油门
vehicle.set_pedal(1.0)   # 全油门
vehicle.set_pedal(0.0)   # 松开油门
```

---

##### set_brake()

```python
set_brake(brake: float) -> None
```

**功能**: 设置制动踏板位置

**参数**:
- `brake` (float): 制动踏板位置，范围 [0.0, 1.0]
  - 0.0 = 无制动
  - 1.0 = 最大制动

**示例**:

```python
vehicle.set_brake(0.3)   # 30%制动
vehicle.set_brake(1.0)   # 紧急制动
```

---

##### set_steering()

```python
set_steering(steering: float) -> None
```

**功能**: 设置转向输入

**参数**:
- `steering` (float): 转向盘转角归一化值，范围 [-1.0, 1.0]
  - -1.0 = 最大左转
  - 0.0 = 直行
  - 1.0 = 最大右转

**示例**:

```python
vehicle.set_steering(-0.5)  # 左转
vehicle.set_steering(0.0)   # 直行
vehicle.set_steering(0.3)   # 右转
```

---

##### get_state()

```python
get_state() -> VehicleState
```

**功能**: 获取当前车辆状态

**返回**: `VehicleState` 对象

**VehicleState 属性**:

| 属性 | 类型 | 单位 | 说明 |
|------|------|------|------|
| `x` | float | m | 纵向位置 |
| `y` | float | m | 侧向位置 |
| `z` | float | m | 垂向位置 |
| `vx` | float | m/s | 纵向速度 |
| `vy` | float | m/s | 侧向速度 |
| `vz` | float | m/s | 垂向速度 |
| `speed` | float | m/s | 合成速度 |
| `ax` | float | m/s² | 纵向加速度 |
| `ay` | float | m/s² | 侧向加速度 |
| `roll` | float | rad | 侧倾角 |
| `pitch` | float | rad | 俯仰角 |
| `yaw` | float | rad | 横摆角 |
| `roll_rate` | float | rad/s | 侧倾角速度 |
| `pitch_rate` | float | rad/s | 俯仰角速度 |
| `yaw_rate` | float | rad/s | 横摆角速度 |
| `battery_soc` | float | - | 电池SOC (0-1) |
| `motor_torque` | float | N·m | 电机扭矩 |

**示例**:

```python
state = vehicle.get_state()
print(f"车速: {state.speed * 3.6:.2f} km/h")
print(f"加速度: {state.ax:.2f} m/s²")
print(f"SOC: {state.battery_soc * 100:.1f}%")
```

---

##### set_speed()

```python
set_speed(speed: float) -> None
```

**功能**: 直接设置车速（用于测试）

**参数**:
- `speed` (float): 目标速度（m/s）

**示例**:

```python
vehicle.set_speed(100 / 3.6)  # 设置为100 km/h
```

---

##### initialize()

```python
initialize() -> None
```

**功能**: 重新初始化车辆状态

**示例**:

```python
vehicle.mass = 1600.0
vehicle.initialize()  # 应用新参数
```

---

### 1.2 Simulation类

#### 构造函数

```python
Simulation(
    vehicle: Vehicle,
    duration: float,
    frequency: float = 500,
    config: dict = None
)
```

**功能**: 创建仿真管理对象

**参数**:
- `vehicle` (Vehicle): 车辆对象
- `duration` (float): 仿真时长（秒）
- `frequency` (float): 仿真频率（Hz），默认500
- `config` (dict, optional): 仿真配置

**示例**:

```python
vehicle = Vehicle(config='config/simulation_config.yaml')
sim = Simulation(
    vehicle=vehicle,
    duration=10.0,
    frequency=500
)
```

---

#### 方法

##### run()

```python
run() -> SimulationResults
```

**功能**: 运行仿真

**返回**: `SimulationResults` 对象

**SimulationResults 属性**:

| 属性 | 类型 | 说明 |
|------|------|------|
| `duration` | float | 仿真时长（秒） |
| `time_steps` | int | 总时间步数 |
| `avg_speed` | float | 平均速度（m/s） |
| `max_speed` | float | 最高速度（m/s） |
| `avg_acceleration` | float | 平均加速度（m/s²） |
| `energy_consumption` | float | 能耗（kWh/100km） |
| `distance` | float | 行驶距离（m） |
| `data` | DataFrame | 详细数据记录 |

**示例**:

```python
results = sim.run()
print(f"平均车速: {results.avg_speed * 3.6:.2f} km/h")
print(f"能耗: {results.energy_consumption:.2f} kWh/100km")
```

---

##### set_driver_input()

```python
set_driver_input(
    time: float,
    throttle: float,
    brake: float,
    steering: float
) -> None
```

**功能**: 设置驾驶员输入事件

**参数**:
- `time` (float): 时间点（秒）
- `throttle` (float): 油门 [0, 1]
- `brake` (float): 制动 [0, 1]
- `steering` (float): 转向 [-1, 1]

**示例**:

```python
sim.set_driver_input(time=0.0, throttle=0.5, brake=0.0, steering=0.0)
sim.set_driver_input(time=5.0, throttle=0.0, brake=0.3, steering=0.0)
```

---

##### plot_results()

```python
plot_results(
    variables: list = None,
    save_path: str = None
) -> None
```

**功能**: 绘制仿真结果图表

**参数**:
- `variables` (list, optional): 要绘制的变量列表
- `save_path` (str, optional): 图片保存路径

**示例**:

```python
results = sim.run()

# 绘制默认图表
sim.plot_results()

# 绘制指定变量
sim.plot_results(variables=['speed', 'acceleration', 'battery_soc'])

# 保存图表
sim.plot_results(save_path='results/simulation_plots.png')
```

---

### 1.3 DrivingCycle类

#### 构造函数

```python
DrivingCycle(name: str = None)
```

**功能**: 加载或创建驾驶循环

**参数**:
- `name` (str, optional): 标准循环名称
  - `'WLTC'`: Worldwide harmonized Light vehicles Test Cycle
  - `'NEDC'`: New European Driving Cycle
  - `'CLTC'`: China Light-duty vehicle Test Cycle
  - `'UDDS'`: Urban Dynamometer Driving Schedule

**示例**:

```python
# 加载标准循环
cycle = DrivingCycle('WLTC')

# 创建自定义循环
custom_cycle = DrivingCycle()
```

---

#### 方法

##### set_speed_profile()

```python
set_speed_profile(profile: list) -> None
```

**功能**: 设置自定义速度曲线

**参数**:
- `profile` (list): 时间-速度列表 `[(time, speed), ...]`
  - `time`: 秒
  - `speed`: km/h

**示例**:

```python
cycle = DrivingCycle()
cycle.set_speed_profile([
    (0.0, 0.0),
    (10.0, 50.0),
    (20.0, 100.0),
    (30.0, 100.0),
    (40.0, 0.0)
])
```

---

##### get_speed_at()

```python
get_speed_at(time: float) -> float
```

**功能**: 获取指定时间的车速

**参数**:
- `time` (float): 时间点（秒）

**返回**: 车速（m/s）

**示例**:

```python
speed = cycle.get_speed_at(15.0)  # 15秒时的车速
```

---

### 1.4 HILInterface类

#### 构造函数

```python
HILInterface(
    can_interface: str,
    can_bitrate: int = 500000,
    enable_logging: bool = False
)
```

**功能**: 创建HIL接口

**参数**:
- `can_interface` (str): CAN接口名称（如 'can0'）
- `can_bitrate` (int): CAN波特率，默认500000
- `enable_logging` (bool): 是否启用日志，默认False

**示例**:

```python
hil = HILInterface(
    can_interface='can0',
    can_bitrate=500000,
    enable_logging=True
)
```

---

#### 方法

##### start()

```python
start() -> None
```

**功能**: 启动CAN接口

**示例**:

```python
hil.start()
```

---

##### stop()

```python
stop() -> None
```

**功能**: 停止CAN接口

**示例**:

```python
hil.stop()
```

---

##### read_can()

```python
read_can(timeout: float = 1.0) -> dict
```

**功能**: 读取CAN数据

**参数**:
- `timeout` (float): 超时时间（秒），默认1.0

**返回**: CAN数据字典

**示例**:

```python
can_data = hil.read_can(timeout=0.1)
throttle = can_data['throttle']
brake = can_data['brake']
```

---

##### send_can()

```python
send_can(data: dict) -> None
```

**功能**: 发送CAN数据

**参数**:
- `data` (dict): 要发送的数据

**示例**:

```python
hil.send_can({
    'speed': vehicle.speed,
    'soc': vehicle.battery_soc
})
```

---

## 2. C++ API

### 2.1 VehicleDynamics类

#### 构造函数

```cpp
VehicleDynamics(const VehicleParams& params);
```

**功能**: 创建车辆动力学对象

**参数**:
- `params`: 车辆参数结构体

**示例**:

```cpp
#include <ev_dynamics/vehicle_dynamics.h>

ev_dynamics::VehicleParams params;
params.mass = 1500.0;
params.inertia_ix = 500.0;
params.inertia_iy = 2000.0;
params.inertia_iz = 2500.0;

ev_dynamics::VehicleDynamics vehicle(params);
```

---

#### 方法

##### update()

```cpp
void update(double dt);
```

**功能**: 更新车辆状态

**参数**:
- `dt`: 时间步长（秒）

**示例**:

```cpp
double dt = 0.002;
for (int i = 0; i < 5000; i++) {
    vehicle.update(dt);
}
```

---

##### get_state()

```cpp
VehicleState get_state() const;
```

**功能**: 获取车辆状态

**返回**: `VehicleState` 结构体

**VehicleState 成员**:

```cpp
struct VehicleState {
    double x, y, z;           // 位置 (m)
    double vx, vy, vz;        // 速度 (m/s)
    double ax, ay, az;        // 加速度 (m/s²)
    double roll, pitch, yaw;  // 姿态角 (rad)
    double roll_rate, pitch_rate, yaw_rate;  // 角速度 (rad/s)
};
```

**示例**:

```cpp
auto state = vehicle.get_state();
std::cout << "Speed: " << std::sqrt(state.vx*state.vx + state.vy*state.vy) 
          << " m/s" << std::endl;
```

---

##### set_control_input()

```cpp
void set_control_input(double throttle, double brake, double steering);
```

**功能**: 设置控制输入

**参数**:
- `throttle`: 油门 [0, 1]
- `brake`: 制动 [0, 1]
- `steering`: 转向 [-1, 1]

**示例**:

```cpp
vehicle.set_control_input(0.5, 0.0, 0.0);  // 50%油门
```

---

### 2.2 RK4Integrator类

#### 构造函数

```cpp
RK4Integrator();
```

**功能**: 创建RK4积分器

**示例**:

```cpp
#include <ev_dynamics/integrators/rk4_integrator.h>

ev_dynamics::RK4Integrator integrator;
```

---

#### 方法

##### integrate()

```cpp
VectorXd integrate(
    const VectorXd& state,
    std::function<VectorXd(const VectorXd&)> derivative,
    double dt
);
```

**功能**: 执行RK4积分

**参数**:
- `state`: 当前状态向量
- `derivative`: 状态导数函数
- `dt`: 时间步长

**返回**: 积分后的状态向量

**示例**:

```cpp
Eigen::VectorXd state(6);
state << 0, 0, 0, 10, 0, 0;  // 初始状态

auto derivative = [](const Eigen::VectorXd& s) {
    Eigen::VectorXd ds(6);
    // 计算状态导数
    ds << s(3), s(4), s(5), 0, 0, 0;  // 简化示例
    return ds;
};

auto new_state = integrator.integrate(state, derivative, 0.002);
```

---

### 2.3 Motor类

#### 构造函数

```cpp
Motor(double max_torque, double max_speed);
```

**功能**: 创建电机模型

**参数**:
- `max_torque`: 最大扭矩（N·m）
- `max_speed`: 最大转速（rpm）

**示例**:

```cpp
#include <ev_dynamics/powertrain/motor.h>

ev_dynamics::powertrain::Motor motor(300.0, 15000.0);
```

---

#### 方法

##### set_torque_request()

```cpp
void set_torque_request(double torque);
```

**功能**: 设置扭矩请求

**参数**:
- `torque`: 请求扭矩（N·m）

**示例**:

```cpp
motor.set_torque_request(150.0);
```

---

##### update()

```cpp
void update(double dt);
```

**功能**: 更新电机状态

**参数**:
- `dt`: 时间步长（秒）

**示例**:

```cpp
motor.update(0.002);
```

---

##### get_torque()

```cpp
double get_torque() const;
```

**功能**: 获取当前输出扭矩

**返回**: 扭矩（N·m）

**示例**:

```cpp
double torque = motor.get_torque();
```

---

## 3. 数据结构

### 3.1 VehicleParams

```cpp
struct VehicleParams {
    // 基本参数
    double mass;              // 整车质量 (kg)
    double length, width, height;  // 尺寸 (m)
    
    // 质心位置
    double cg_to_front;       // 质心到前轴距离 (m)
    double cg_to_rear;        // 质心到后轴距离 (m)
    double cg_height;         // 质心高度 (m)
    
    // 转动惯量
    double inertia_ix;        // 侧倾惯量 (kg·m²)
    double inertia_iy;        // 俯仰惯量 (kg·m²)
    double inertia_iz;        // 横摆惯量 (kg·m²)
    
    // 动力系统
    double motor_max_torque;  // 电机最大扭矩 (N·m)
    double motor_max_speed;   // 电机最大转速 (rpm)
    double battery_capacity;  // 电池容量 (kWh)
    double battery_max_power; // 电池最大功率 (kW)
    
    // 底盘
    double suspension_stiffness_front;   // 前悬架刚度 (N/m)
    double suspension_damping_front;     // 前悬架阻尼 (N·s/m)
    double suspension_stiffness_rear;    // 后悬架刚度 (N/m)
    double suspension_damping_rear;      // 后悬架阻尼 (N·s/m)
};
```

### 3.2 SimulationConfig

```python
@dataclass
class SimulationConfig:
    frequency: int = 500      # Hz
    duration: float = 10.0    # 秒
    mode: str = 'realtime'    # realtime, batch, replay
    
    # 输出配置
    output_enabled: bool = True
    output_format: str = 'csv'
    output_path: str = 'results/'
    
    # 日志配置
    log_level: str = 'INFO'
    log_to_file: bool = True
    log_path: str = 'logs/'
```

---

## 4. 错误处理

### 4.1 Python异常

```python
try:
    vehicle = Vehicle(config='invalid_path.yaml')
except FileNotFoundError as e:
    print(f"配置文件未找到: {e}")
except ValueError as e:
    print(f"参数错误: {e}")
except RuntimeError as e:
    print(f"运行时错误: {e}")
```

### 4.2 C++异常

```cpp
#include <ev_dynamics/exceptions.h>

try {
    auto vehicle = VehicleDynamics(params);
    vehicle.update(dt);
} catch (const ev_dynamics::ParameterError& e) {
    std::cerr << "参数错误: " << e.what() << std::endl;
} catch (const ev_dynamics::SimulationError& e) {
    std::cerr << "仿真错误: " << e.what() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "未知错误: " << e.what() << std::endl;
}
```

---

## 附录

### A. 完整示例

#### Python完整示例

```python
import sys
sys.path.append('build/python_modules')

from ev_dynamics import Vehicle, Simulation, DrivingCycle

# 1. 创建车辆
vehicle = Vehicle(config='config/simulation_config.yaml')

# 2. 创建仿真
sim = Simulation(
    vehicle=vehicle,
    duration=20.0,
    frequency=500
)

# 3. 设置驾驶员输入
sim.set_driver_input(time=0.0, throttle=0.8, brake=0.0, steering=0.0)
sim.set_driver_input(time=10.0, throttle=0.0, brake=0.5, steering=0.0)

# 4. 运行仿真
results = sim.run()

# 5. 分析结果
print(f"平均车速: {results.avg_speed * 3.6:.2f} km/h")
print(f"最高车速: {results.max_speed * 3.6:.2f} km/h")
print(f"能耗: {results.energy_consumption:.2f} kWh/100km")
print(f"行驶距离: {results.distance / 1000:.2f} km")

# 6. 绘制图表
sim.plot_results(save_path='results/simulation.png')
```

#### C++完整示例

```cpp
#include <ev_dynamics/vehicle_dynamics.h>
#include <ev_dynamics/powertrain/motor.h>
#include <ev_dynamics/powertrain/battery.h>
#include <iostream>

int main() {
    // 1. 配置参数
    ev_dynamics::VehicleParams params;
    params.mass = 1500.0;
    params.inertia_ix = 500.0;
    params.inertia_iy = 2000.0;
    params.inertia_iz = 2500.0;
    
    // 2. 创建车辆
    ev_dynamics::VehicleDynamics vehicle(params);
    
    // 3. 仿真循环
    double dt = 0.002;
    for (int i = 0; i < 10000; i++) {
        // 设置控制输入
        if (i < 5000) {
            vehicle.set_control_input(0.8, 0.0, 0.0);  // 加速
        } else {
            vehicle.set_control_input(0.0, 0.5, 0.0);  // 制动
        }
        
        // 更新状态
        vehicle.update(dt);
        
        // 获取状态
        auto state = vehicle.get_state();
        
        if (i % 500 == 0) {
            double speed = std::sqrt(state.vx*state.vx + state.vy*state.vy);
            std::cout << "时间: " << i * dt << "s, "
                      << "车速: " << speed * 3.6 << " km/h"
                      << std::endl;
        }
    }
    
    std::cout << "仿真完成！" << std::endl;
    return 0;
}
```

### B. 性能基准

| 操作 | 平均时间 | 说明 |
|------|---------|------|
| `Vehicle.update()` | <50 μs | 单步更新 |
| `Vehicle.get_state()` | <5 μs | 状态查询 |
| `Simulation.run()` | ~实时 | 10秒仿真 ≈ 10秒实际时间 |

---

**文档版本**: 1.0  
**最后更新**: 2026-03-06  
**维护者**: EV Dynamics Team
