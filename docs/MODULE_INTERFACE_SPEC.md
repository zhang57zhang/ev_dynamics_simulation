# 六自由度新能源车辆动力学仿真系统 - 模块接口规范

**文档版本**: v1.0  
**创建日期**: 2026-03-05  
**作者**: ArchitectAgent  
**状态**: 设计阶段

---

## 目录

1. [接口设计原则](#1-接口设计原则)
2. [动力系统接口](#2-动力系统接口)
3. [底盘系统接口](#3-底盘系统接口)
4. [车辆动力学接口](#4-车辆动力学接口)
5. [轮胎模型接口](#5-轮胎模型接口)
6. [实时调度接口](#6-实时调度接口)
7. [HIL接口](#7-hil接口)
8. [通用数据类型](#8-通用数据类型)
9. [错误处理机制](#9-错误处理机制)
10. [接口版本管理](#10-接口版本管理)

---

## 1. 接口设计原则

### 1.1 核心原则

1. **最小知识原则**：模块只应知道它需要知道的
2. **依赖倒置**：高层模块不应依赖低层模块，两者都应依赖抽象
3. **接口隔离**：不应强迫客户依赖它们不使用的方法
4. **单一职责**：每个接口应只有一个变更理由

### 1.2 接口命名规范

**Python接口命名**：
```python
# 使用Protocol（Python 3.8+）
from typing import Protocol, runtime_checkable

@runtime_checkable
class IModuleInterface(Protocol):
    """模块接口协议"""
    
    def initialize(self, config: ModuleConfig) -> None:
        """初始化模块"""
        ...
    
    def update(self, dt: float) -> None:
        """更新模块状态（每个时间步）"""
        ...
    
    def shutdown(self) -> None:
        """关闭模块"""
        ...
```

**C++接口命名**：
```cpp
// 使用抽象基类
class IModuleInterface {
public:
    virtual ~IModuleInterface() = default;
    
    // 生命周期
    virtual void initialize(const ModuleConfig& config) = 0;
    virtual void update(double dt) = 0;
    virtual void shutdown() = 0;
    
    // 状态查询
    virtual bool isInitialized() const = 0;
    virtual std::string getName() const = 0;
};
```

### 1.3 版本控制策略

所有接口都包含版本信息：

```python
class InterfaceVersion:
    major: int      # 不兼容的API变更
    minor: int      # 向后兼容的功能新增
    patch: int      # 向后兼容的问题修复
    
    def is_compatible(self, other: 'InterfaceVersion') -> bool:
        return self.major == other.major and self.minor <= other.minor
```

---

## 2. 动力系统接口

### 2.1 电机接口（IMotorInterface）

#### 2.1.1 接口定义（Python）

```python
from typing import Protocol, Optional
from dataclasses import dataclass
from enum import Enum

class MotorType(Enum):
    """电机类型"""
    PMSM = "Permanent Magnet Synchronous Motor"
    INDUCTION = "Induction Motor"
    BLDC = "Brushless DC Motor"

@dataclass
class MotorConfig:
    """电机配置参数"""
    # 基本参数
    motor_type: MotorType
    max_torque: float              # 最大扭矩 (N·m)
    max_power: float               # 最大功率 (W)
    max_speed: float               # 最大转速 (rpm)
    base_speed: float              # 基速 (rpm)
    
    # 电气参数
    nominal_voltage: float         # 额定电压 (V)
    max_current: float             # 最大电流 (A)
    stator_resistance: float       # 定子电阻 (Ω)
    inductance_d: float            # d轴电感 (H)
    inductance_q: float            # q轴电感 (H)
    flux_linkage: float            # 永磁体磁链 (Wb)
    pole_pairs: int                # 极对数
    
    # 机械参数
    rotor_inertia: float           # 转子惯量 (kg·m²)
    damping_coefficient: float     # 阻尼系数 (N·m·s/rad)
    
    # 热参数
    max_temperature: float         # 最高温度 (°C)
    thermal_resistance: float      # 热阻 (°C/W)
    thermal_capacity: float        # 热容 (J/°C)

@dataclass
class MotorState:
    """电机状态"""
    # 转矩和转速
    torque_request: float          # 扭矩请求 (N·m)
    torque_actual: float           # 实际扭矩 (N·m)
    rotor_speed: float             # 转子转速 (rpm)
    rotor_angle: float             # 转子角度 (rad)
    
    # 电气状态
    current_d: float               # d轴电流 (A)
    current_q: float               # q轴电流 (A)
    voltage_d: float               # d轴电压 (V)
    voltage_q: float               # q轴电压 (V)
    dc_bus_voltage: float          # 直流母线电压 (V)
    
    # 热状态
    temperature: float             # 电机温度 (°C)
    
    # 诊断
    is_overloaded: bool            # 过载标志
    is_overheated: bool            # 过热标志
    fault_code: int                # 故障码

@dataclass
class MotorOutput:
    """电机输出"""
    torque: float                  # 输出扭矩 (N·m)
    power: float                   # 输出功率 (W)
    efficiency: float              # 效率 (0-1)

class IMotorInterface(Protocol):
    """电机接口协议"""
    
    @property
    def version(self) -> InterfaceVersion:
        """接口版本"""
        ...
    
    @property
    def config(self) -> MotorConfig:
        """电机配置"""
        ...
    
    @property
    def state(self) -> MotorState:
        """电机状态（只读）"""
        ...
    
    # === 生命周期管理 ===
    
    def initialize(self, config: MotorConfig) -> None:
        """
        初始化电机模型
        
        Args:
            config: 电机配置参数
        
        Raises:
            ConfigurationError: 配置参数无效
        """
        ...
    
    def reset(self) -> None:
        """重置电机状态到初始值"""
        ...
    
    def shutdown(self) -> None:
        """关闭电机模型，释放资源"""
        ...
    
    # === 控制接口 ===
    
    def set_torque_request(self, torque: float) -> None:
        """
        设置扭矩请求
        
        Args:
            torque: 目标扭矩 (N·m)，正值驱动，负值制动
        
        Raises:
            ValueError: 扭矩超出有效范围
        """
        ...
    
    def set_dc_bus_voltage(self, voltage: float) -> None:
        """
        设置直流母线电压（来自电池）
        
        Args:
            voltage: 直流母线电压 (V)
        """
        ...
    
    def update(self, dt: float) -> None:
        """
        更新电机状态（每个时间步调用）
        
        Args:
            dt: 时间步长 (s)
        
        Note:
            该方法应在每个仿真周期调用一次
        """
        ...
    
    # === 状态查询 ===
    
    def get_actual_torque(self) -> float:
        """
        获取实际输出扭矩
        
        Returns:
            实际扭矩 (N·m)
        """
        ...
    
    def get_rotor_speed(self) -> float:
        """
        获取转子转速
        
        Returns:
            转子转速 (rpm)
        """
        ...
    
    def get_motor_temperature(self) -> float:
        """
        获取电机温度
        
        Returns:
            电机温度 (°C)
        """
        ...
    
    def get_power_consumption(self) -> float:
        """
        获取功率消耗
        
        Returns:
            功率 (W)，正值消耗，负值回收
        """
        ...
    
    def get_efficiency(self) -> float:
        """
        获取当前工作点效率
        
        Returns:
            效率 (0-1)
        """
        ...
    
    # === 诊断接口 ===
    
    def is_healthy(self) -> bool:
        """
        检查电机健康状态
        
        Returns:
            True: 健康，False: 存在故障
        """
        ...
    
    def get_fault_code(self) -> int:
        """
        获取故障码
        
        Returns:
            故障码（0: 无故障）
        """
        ...
    
    def get_diagnostics(self) -> dict:
        """
        获取诊断信息
        
        Returns:
            诊断数据字典
        """
        ...
```

#### 2.1.2 接口定义（C++）

```cpp
// motor_interface.h
#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include "common_types.h"

namespace ev_simulation {
namespace powertrain {

/**
 * @brief 电机类型枚举
 */
enum class MotorType {
    PMSM,       ///< 永磁同步电机
    INDUCTION,  ///< 感应电机
    BLDC        ///< 无刷直流电机
};

/**
 * @brief 电机配置参数
 */
struct MotorConfig {
    // 基本参数
    MotorType motor_type;
    double max_torque;              ///< 最大扭矩 (N·m)
    double max_power;               ///< 最大功率 (W)
    double max_speed;               ///< 最大转速 (rpm)
    double base_speed;              ///< 基速 (rpm)
    
    // 电气参数
    double nominal_voltage;         ///< 额定电压 (V)
    double max_current;             ///< 最大电流 (A)
    double stator_resistance;       ///< 定子电阻 (Ω)
    double inductance_d;            ///< d轴电感 (H)
    double inductance_q;            ///< q轴电感 (H)
    double flux_linkage;            ///< 永磁体磁链 (Wb)
    int pole_pairs;                 ///< 极对数
    
    // 机械参数
    double rotor_inertia;           ///< 转子惯量 (kg·m²)
    double damping_coefficient;     ///< 阻尼系数 (N·m·s/rad)
    
    // 热参数
    double max_temperature;         ///< 最高温度 (°C)
    double thermal_resistance;      ///< 热阻 (°C/W)
    double thermal_capacity;        ///< 热容 (J/°C)
    
    /**
     * @brief 验证配置参数有效性
     * @throws std::invalid_argument 参数无效
     */
    void validate() const {
        if (max_torque <= 0) {
            throw std::invalid_argument("max_torque must be positive");
        }
        if (max_power <= 0) {
            throw std::invalid_argument("max_power must be positive");
        }
        if (max_speed <= 0) {
            throw std::invalid_argument("max_speed must be positive");
        }
        // ... 更多验证
    }
};

/**
 * @brief 电机状态
 */
struct MotorState {
    // 转矩和转速
    double torque_request{0.0};     ///< 扭矩请求 (N·m)
    double torque_actual{0.0};      ///< 实际扭矩 (N·m)
    double rotor_speed{0.0};        ///< 转子转速 (rpm)
    double rotor_angle{0.0};        ///< 转子角度 (rad)
    
    // 电气状态
    double current_d{0.0};          ///< d轴电流 (A)
    double current_q{0.0};          ///< q轴电流 (A)
    double voltage_d{0.0};          ///< d轴电压 (V)
    double voltage_q{0.0};          ///< q轴电压 (V)
    double dc_bus_voltage{400.0};   ///< 直流母线电压 (V)
    
    // 热状态
    double temperature{25.0};       ///< 电机温度 (°C)
    
    // 诊断
    bool is_overloaded{false};      ///< 过载标志
    bool is_overheated{false};      ///< 过热标志
    int fault_code{0};              ///< 故障码
};

/**
 * @brief 电机输出
 */
struct MotorOutput {
    double torque{0.0};             ///< 输出扭矩 (N·m)
    double power{0.0};              ///< 输出功率 (W)
    double efficiency{0.0};         ///< 效率 (0-1)
};

/**
 * @brief 电机接口（抽象基类）
 * 
 * 定义了所有电机模型必须实现的接口。
 * 支持PMSM、感应电机、BLDC等不同类型的电机模型。
 */
class IMotorInterface {
public:
    virtual ~IMotorInterface() = default;
    
    // === 生命周期管理 ===
    
    /**
     * @brief 初始化电机模型
     * @param config 电机配置参数
     * @throws ConfigurationError 配置参数无效
     */
    virtual void initialize(const MotorConfig& config) = 0;
    
    /**
     * @brief 重置电机状态到初始值
     */
    virtual void reset() = 0;
    
    /**
     * @brief 关闭电机模型，释放资源
     */
    virtual void shutdown() = 0;
    
    // === 控制接口 ===
    
    /**
     * @brief 设置扭矩请求
     * @param torque 目标扭矩 (N·m)，正值驱动，负值制动
     * @throws std::invalid_argument 扭矩超出有效范围
     */
    virtual void setTorqueRequest(double torque) = 0;
    
    /**
     * @brief 设置直流母线电压（来自电池）
     * @param voltage 直流母线电压 (V)
     */
    virtual void setDcBusVoltage(double voltage) = 0;
    
    /**
     * @brief 更新电机状态（每个时间步调用）
     * @param dt 时间步长 (s)
     * @note 该方法应在每个仿真周期调用一次
     */
    virtual void update(double dt) = 0;
    
    // === 状态查询 ===
    
    /**
     * @brief 获取实际输出扭矩
     * @return 实际扭矩 (N·m)
     */
    virtual double getActualTorque() const = 0;
    
    /**
     * @brief 获取转子转速
     * @return 转子转速 (rpm)
     */
    virtual double getRotorSpeed() const = 0;
    
    /**
     * @brief 获取电机温度
     * @return 电机温度 (°C)
     */
    virtual double getMotorTemperature() const = 0;
    
    /**
     * @brief 获取功率消耗
     * @return 功率 (W)，正值消耗，负值回收
     */
    virtual double getPowerConsumption() const = 0;
    
    /**
     * @brief 获取当前工作点效率
     * @return 效率 (0-1)
     */
    virtual double getEfficiency() const = 0;
    
    /**
     * @brief 获取完整状态
     * @return 电机状态结构体
     */
    virtual const MotorState& getState() const = 0;
    
    /**
     * @brief 获取输出数据
     * @return 电机输出结构体
     */
    virtual MotorOutput getOutput() const = 0;
    
    // === 诊断接口 ===
    
    /**
     * @brief 检查电机健康状态
     * @return true: 健康，false: 存在故障
     */
    virtual bool isHealthy() const = 0;
    
    /**
     * @brief 获取故障码
     * @return 故障码（0: 无故障）
     */
    virtual int getFaultCode() const = 0;
    
    /**
     * @brief 获取诊断信息
     * @return 诊断数据映射
     */
    virtual std::map<std::string, double> getDiagnostics() const = 0;
    
    // === 元信息 ===
    
    /**
     * @brief 获取接口版本
     * @return 版本信息
     */
    virtual InterfaceVersion getVersion() const = 0;
    
    /**
     * @brief 获取电机名称
     * @return 电机名称
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief 检查是否已初始化
     * @return true: 已初始化，false: 未初始化
     */
    virtual bool isInitialized() const = 0;
};

} // namespace powertrain
} // namespace ev_simulation
```

#### 2.1.3 使用示例

**Python示例**：

```python
# 创建电机实例
from ev_simulation.powertrain import PMSMMotor

# 配置电机
config = MotorConfig(
    motor_type=MotorType.PMSM,
    max_torque=300.0,
    max_power=150000.0,
    max_speed=12000.0,
    base_speed=4000.0,
    nominal_voltage=400.0,
    max_current=400.0,
    stator_resistance=0.02,
    inductance_d=0.0002,
    inductance_q=0.0003,
    flux_linkage=0.1,
    pole_pairs=4,
    rotor_inertia=0.05,
    damping_coefficient=0.001,
    max_temperature=150.0,
    thermal_resistance=0.1,
    thermal_capacity=500.0
)

# 初始化
motor = PMSMMotor()
motor.initialize(config)

# 仿真循环
dt = 0.002  # 500Hz
motor.set_dc_bus_voltage(380.0)  # 来自电池

for step in range(1000):
    # 设置扭矩请求
    torque_request = 150.0  # N·m
    motor.set_torque_request(torque_request)
    
    # 更新状态
    motor.update(dt)
    
    # 读取输出
    actual_torque = motor.get_actual_torque()
    rotor_speed = motor.get_rotor_speed()
    power = motor.get_power_consumption()
    
    print(f"Step {step}: Torque={actual_torque:.1f} N·m, "
          f"Speed={rotor_speed:.0f} rpm, Power={power:.0f} W")
    
    # 检查健康状态
    if not motor.is_healthy():
        fault_code = motor.get_fault_code()
        print(f"Fault detected! Code: {fault_code}")
        break

# 关闭
motor.shutdown()
```

**C++示例**：

```cpp
#include "pmsm_motor.h"
#include <iostream>

using namespace ev_simulation::powertrain;

int main() {
    // 配置电机
    MotorConfig config;
    config.motor_type = MotorType::PMSM;
    config.max_torque = 300.0;
    config.max_power = 150000.0;
    config.max_speed = 12000.0;
    config.base_speed = 4000.0;
    config.nominal_voltage = 400.0;
    config.max_current = 400.0;
    config.stator_resistance = 0.02;
    config.inductance_d = 0.0002;
    config.inductance_q = 0.0003;
    config.flux_linkage = 0.1;
    config.pole_pairs = 4;
    config.rotor_inertia = 0.05;
    config.damping_coefficient = 0.001;
    config.max_temperature = 150.0;
    config.thermal_resistance = 0.1;
    config.thermal_capacity = 500.0;
    
    // 验证配置
    config.validate();
    
    // 创建电机实例
    auto motor = std::make_unique<PMSMMotor>();
    
    // 初始化
    motor->initialize(config);
    
    // 仿真循环
    double dt = 0.002;  // 500Hz
    motor->setDcBusVoltage(380.0);  // 来自电池
    
    for (int step = 0; step < 1000; step++) {
        // 设置扭矩请求
        double torque_request = 150.0;  // N·m
        motor->setTorqueRequest(torque_request);
        
        // 更新状态
        motor->update(dt);
        
        // 读取输出
        double actual_torque = motor->getActualTorque();
        double rotor_speed = motor->getRotorSpeed();
        double power = motor->getPowerConsumption();
        
        std::cout << "Step " << step 
                  << ": Torque=" << actual_torque << " N·m, "
                  << "Speed=" << rotor_speed << " rpm, "
                  << "Power=" << power << " W" << std::endl;
        
        // 检查健康状态
        if (!motor->isHealthy()) {
            int fault_code = motor->getFaultCode();
            std::cerr << "Fault detected! Code: " << fault_code << std::endl;
            break;
        }
    }
    
    // 关闭
    motor->shutdown();
    
    return 0;
}
```

---

### 2.2 电池接口（IBatteryInterface）

#### 2.2.1 接口定义（Python）

```python
from typing import Protocol, List
from dataclasses import dataclass
from enum import Enum

class BatteryChemistry(Enum):
    """电池化学类型"""
    LITHIUM_ION = "Lithium-Ion"
    LITHIUM_IRON_PHOSPHATE = "LiFePO4"
    LITHIUM_POLYMER = "Li-Po"
    NICKEL_METAL_HYDRIDE = "NiMH"

@dataclass
class BatteryConfig:
    """电池配置参数"""
    # 基本参数
    chemistry: BatteryChemistry
    capacity: float                # 容量 (kWh)
    nominal_voltage: float         # 额定电压 (V)
    num_cells_series: int          # 串联单体数
    num_cells_parallel: int        # 并联单体数
    
    # 性能参数
    max_discharge_rate: float      # 最大放电倍率 (C)
    max_charge_rate: float         # 最大充电倍率 (C)
    internal_resistance: float     # 内阻 (Ω)
    
    # 工作范围
    min_voltage: float             # 最低电压 (V)
    max_voltage: float             # 最高电压 (V)
    min_temperature: float         # 最低温度 (°C)
    max_temperature: float         # 最高温度 (°C)
    
    # 老化参数
    initial_soh: float             # 初始SOH (0-1)
    calendar_aging_rate: float     # 日历老化速率 (1/天)
    cycle_aging_rate: float        # 循环老化速率 (1/kWh)

@dataclass
class BatteryState:
    """电池状态"""
    # 荷电状态
    soc: float                     # 荷电状态 (0-1)
    soh: float                     # 健康状态 (0-1)
    
    # 电压电流
    voltage: float                 # 端电压 (V)
    current: float                 # 电流 (A)，正值放电
    open_circuit_voltage: float    # 开路电压 (V)
    
    # 功率
    power: float                   # 功率 (W)，正值放电
    available_power: float         # 可用功率 (W)
    
    # 温度
    temperature: float             # 电池温度 (°C)
    ambient_temperature: float     # 环境温度 (°C)
    
    # 老化
    cycle_count: int               # 循环次数
    total_energy_throughput: float # 累计能量吞吐量 (kWh)
    
    # 诊断
    is_overcharged: bool           # 过充标志
    is_overdischarged: bool        # 过放标志
    is_overheated: bool            # 过热标志
    fault_code: int                # 故障码

@dataclass
class PowerLimits:
    """功率限制"""
    max_discharge_power: float     # 最大放电功率 (W)
    max_charge_power: float        # 最大充电功率 (W)
    max_regen_power: float         # 最大回收功率 (W)
    
    def is_valid(self, power: float) -> bool:
        """检查功率是否在限制范围内"""
        if power >= 0:
            return power <= self.max_discharge_power
        else:
            return abs(power) <= self.max_charge_power

class IBatteryInterface(Protocol):
    """电池接口协议"""
    
    @property
    def version(self) -> InterfaceVersion:
        """接口版本"""
        ...
    
    @property
    def config(self) -> BatteryConfig:
        """电池配置"""
        ...
    
    @property
    def state(self) -> BatteryState:
        """电池状态（只读）"""
        ...
    
    # === 生命周期管理 ===
    
    def initialize(self, config: BatteryConfig) -> None:
        """
        初始化电池模型
        
        Args:
            config: 电池配置参数
        
        Raises:
            ConfigurationError: 配置参数无效
        """
        ...
    
    def reset(self, initial_soc: float = 0.8) -> None:
        """
        重置电池状态
        
        Args:
            initial_soc: 初始SOC (0-1)
        """
        ...
    
    def shutdown(self) -> None:
        """关闭电池模型，释放资源"""
        ...
    
    # === 功率接口 ===
    
    def set_power_request(self, power: float) -> None:
        """
        设置功率请求
        
        Args:
            power: 功率请求 (W)，正值放电，负值充电
        
        Note:
            实际功率会受到功率限制的约束
        """
        ...
    
    def update(self, dt: float) -> None:
        """
        更新电池状态（每个时间步调用）
        
        Args:
            dt: 时间步长 (s)
        """
        ...
    
    # === 状态查询 ===
    
    def get_soc(self) -> float:
        """
        获取电池SOC
        
        Returns:
            荷电状态 (0-1)
        """
        ...
    
    def get_soh(self) -> float:
        """
        获取电池SOH
        
        Returns:
            健康状态 (0-1)
        """
        ...
    
    def get_voltage(self) -> float:
        """
        获取端电压
        
        Returns:
            端电压 (V)
        """
        ...
    
    def get_current(self) -> float:
        """
        获取电流
        
        Returns:
            电流 (A)，正值放电
        """
        ...
    
    def get_power(self) -> float:
        """
        获取实际功率
        
        Returns:
            功率 (W)，正值放电
        """
        ...
    
    def get_power_limits(self) -> PowerLimits:
        """
        获取功率限制
        
        Returns:
            功率限制结构体
        """
        ...
    
    def get_temperature(self) -> float:
        """
        获取电池温度
        
        Returns:
            电池温度 (°C)
        """
        ...
    
    # === 诊断接口 ===
    
    def is_healthy(self) -> bool:
        """检查电池健康状态"""
        ...
    
    def get_fault_code(self) -> int:
        """获取故障码"""
        ...
    
    def get_cell_voltages(self) -> List[float]:
        """
        获取各单体电压（如果支持）
        
        Returns:
            单体电压列表 (V)
        """
        ...
    
    # === 老化管理 ===
    
    def age_battery(self, days: float) -> None:
        """
        加速电池老化（用于测试）
        
        Args:
            days: 老化天数
        """
        ...
    
    def get_remaining_life(self) -> float:
        """
        获取剩余寿命
        
        Returns:
            剩余寿命比例 (0-1)
        """
        ...
```

#### 2.2.2 C++接口（略，类似电机接口）

---

### 2.3 动力系统集成接口（IPowertrainInterface）

```python
class IPowertrainInterface(Protocol):
    """动力系统集成接口"""
    
    @property
    def motor(self) -> IMotorInterface:
        """电机接口"""
        ...
    
    @property
    def battery(self) -> IBatteryInterface:
        """电池接口"""
        ...
    
    @property
    def transmission(self) -> ITransmissionInterface:
        """变速器接口"""
        ...
    
    def initialize(self, config: PowertrainConfig) -> None:
        """初始化动力系统"""
        ...
    
    def set_driver_inputs(
        self,
        throttle: float,  # 0-1
        brake: float      # 0-1
    ) -> None:
        """
        设置驾驶员输入
        
        Args:
            throttle: 油门踏板位置 (0-1)
            brake: 制动踏板位置 (0-1)
        """
        ...
    
    def update(self, dt: float, vehicle_speed: float) -> None:
        """
        更新动力系统状态
        
        Args:
            dt: 时间步长 (s)
            vehicle_speed: 车速 (m/s)，用于计算再生制动
        """
        ...
    
    def get_drive_torque(self) -> float:
        """
        获取驱动扭矩（轮端）
        
        Returns:
            驱动扭矩 (N·m)
        """
        ...
    
    def get_powertrain_state(self) -> PowertrainState:
        """获取完整动力系统状态"""
        ...
```

---

## 3. 底盘系统接口

### 3.1 悬架接口（ISuspensionInterface）

```python
from typing import Protocol, Tuple
from dataclasses import dataclass
from enum import Enum

class SuspensionType(Enum):
    """悬架类型"""
    PASSIVE = "Passive"
    SEMI_ACTIVE = "Semi-Active"
    ACTIVE = "Active"

@dataclass
class SuspensionConfig:
    """悬架配置参数（单轮）"""
    suspension_type: SuspensionType
    
    # 刚度和阻尼
    spring_stiffness: float        # 弹簧刚度 (N/m)
    damping_coefficient: float     # 阻尼系数 (N·s/m)
    
    # 运动学参数
    motion_ratio: float            # 运动比
    unsprung_mass: float           # 簧下质量 (kg)
    
    # 主动悬架参数
    max_actuator_force: float      # 最大作动器力 (N)
    control_strategy: str          # 控制策略

@dataclass
class SuspensionState:
    """悬架状态（单轮）"""
    # 位移
    displacement: float            # 悬架位移 (m)，正值压缩
    velocity: float                # 悬架速度 (m/s)
    
    # 力
    spring_force: float            # 弹簧力 (N)
    damper_force: float            # 阻尼力 (N)
    actuator_force: float          # 作动器力 (N)
    total_force: float             # 总力 (N)
    
    # 载荷
    vertical_load: float           # 垂直载荷 (N)

class ISuspensionInterface(Protocol):
    """悬架接口协议"""
    
    def initialize(self, config: SuspensionConfig) -> None:
        """初始化悬架模型"""
        ...
    
    def set_road_input(self, road_displacement: float, road_velocity: float) -> None:
        """
        设置路面输入
        
        Args:
            road_displacement: 路面位移 (m)
            road_velocity: 路面速度 (m/s)
        """
        ...
    
    def set_vertical_load(self, load: float) -> None:
        """
        设置垂直载荷（来自载荷转移）
        
        Args:
            load: 垂直载荷 (N)
        """
        ...
    
    def update(self, dt: float) -> None:
        """更新悬架状态"""
        ...
    
    def get_suspension_force(self) -> float:
        """
        获取悬架力（作用在车身）
        
        Returns:
            悬架力 (N)
        """
        ...
    
    def get_state(self) -> SuspensionState:
        """获取悬架状态"""
        ...
```

### 3.2 转向接口（ISteeringInterface）

```python
class SteeringType(Enum):
    """转向类型"""
    MECHANICAL = "Mechanical"
    EPS = "Electric Power Steering"
    SBW = "Steer-by-Wire"

@dataclass
class SteeringConfig:
    """转向配置参数"""
    steering_type: SteeringType
    
    # 传动参数
    steering_ratio: float          # 转向传动比
    max_steering_angle: float      # 最大转向角 (deg)
    max_steering_wheel_angle: float  # 最大转向盘转角 (deg)
    
    # 助力参数（EPS）
    max_assist_torque: float       # 最大助力扭矩 (N·m)
    assist_gain_low_speed: float   # 低速助力增益
    assist_gain_high_speed: float  # 高速助力增益
    
    # 动力学参数
    steering_inertia: float        # 转向系统惯量 (kg·m²)
    steering_damping: float        # 转向阻尼 (N·m·s/rad)

@dataclass
class SteeringState:
    """转向状态"""
    steering_wheel_angle: float    # 转向盘转角 (deg)
    steering_wheel_torque: float   # 转向盘扭矩 (N·m)
    front_wheel_angle: float       # 前轮转角 (deg)
    assist_torque: float           # 助力扭矩 (N·m)

class ISteeringInterface(Protocol):
    """转向接口协议"""
    
    def initialize(self, config: SteeringConfig) -> None:
        """初始化转向模型"""
        ...
    
    def set_steering_input(self, angle: float) -> None:
        """
        设置转向盘转角
        
        Args:
            angle: 转向盘转角 (deg)
        """
        ...
    
    def set_self_aligning_torque(self, torque: float) -> None:
        """
        设置回正力矩（来自轮胎）
        
        Args:
            torque: 回正力矩 (N·m)
        """
        ...
    
    def update(self, dt: float, vehicle_speed: float) -> None:
        """
        更新转向状态
        
        Args:
            dt: 时间步长 (s)
            vehicle_speed: 车速 (m/s)，用于速敏感助力
        """
        ...
    
    def get_front_wheel_angle(self) -> float:
        """
        获取前轮转角
        
        Returns:
            前轮转角 (deg)
        """
        ...
    
    def get_steering_wheel_torque(self) -> float:
        """
        获取转向盘扭矩
        
        Returns:
            转向盘扭矩 (N·m)
        """
        ...
    
    def get_state(self) -> SteeringState:
        """获取转向状态"""
        ...
```

### 3.3 制动接口（IBrakingInterface）

```python
class BrakingType(Enum):
    """制动类型"""
    HYDRAULIC = "Hydraulic"
    EMB = "Electro-Mechanical Brake"
    REGENERATIVE = "Regenerative"

@dataclass
class BrakingConfig:
    """制动配置参数"""
    braking_type: BrakingType
    
    # 制动力参数
    max_brake_torque_per_wheel: float  # 单轮最大制动力矩 (N·m)
    brake_response_time: float         # 制动响应时间 (s)
    
    # 分配策略
    front_rear_ratio: float        # 前后制动力分配比
    ebd_enabled: bool              # 是否启用EBD
    
    # 再生制动参数
    max_regen_ratio: float         # 最大再生制动比例 (0-1)
    regen_blend_strategy: str      # 再生混合策略

@dataclass
class BrakeForces:
    """制动力（四轮）"""
    front_left: float              # 前左制动力 (N)
    front_right: float             # 前右制动力 (N)
    rear_left: float               # 后左制动力 (N)
    rear_right: float              # 后右制动力 (N)

class IBrakingInterface(Protocol):
    """制动接口协议"""
    
    def initialize(self, config: BrakingConfig) -> None:
        """初始化制动模型"""
        ...
    
    def set_brake_request(self, request: float) -> None:
        """
        设置制动请求
        
        Args:
            request: 制动请求 (0-1)
        """
        ...
    
    def set_regeneration_torque(self, torque: float) -> None:
        """
        设置再生制动扭矩（来自动力系统）
        
        Args:
            torque: 再生制动扭矩 (N·m)
        """
        ...
    
    def update(self, dt: float, vehicle_state: 'VehicleState') -> None:
        """
        更新制动状态
        
        Args:
            dt: 时间步长 (s)
            vehicle_state: 车辆状态（用于EBD）
        """
        ...
    
    def get_brake_forces(self) -> BrakeForces:
        """
        获取制动力
        
        Returns:
            四轮制动力
        """
        ...
    
    def get_total_brake_torque(self) -> float:
        """
        获取总制动力矩
        
        Returns:
            总制动力矩 (N·m)
        """
        ...
```

---

## 4. 车辆动力学接口

### 4.1 6-DOF动力学接口（IVehicleDynamicsInterface）

```python
@dataclass
class VehicleConfig:
    """车辆配置参数"""
    # 质量参数
    mass: float                    # 整车质量 (kg)
    inertia_xx: float              # 绕X轴转动惯量 (kg·m²)
    inertia_yy: float              # 绕Y轴转动惯量 (kg·m²)
    inertia_zz: float              # 绕Z轴转动惯量 (kg·m²)
    
    # 尺寸参数
    wheelbase: float               # 轴距 (m)
    track_width_front: float       # 前轮距 (m)
    track_width_rear: float        # 后轮距 (m)
    cg_height: float               # 质心高度 (m)
    cg_to_front_axle: float        # 质心到前轴距离 (m)
    cg_to_rear_axle: float         # 质心到后轴距离 (m)
    
    # 空气动力学
    drag_coefficient: float        # 阻力系数
    frontal_area: float            # 迎风面积 (m²)
    lift_coefficient: float        # 升力系数

@dataclass
class VehicleState:
    """车辆状态（6-DOF）"""
    # 位置（惯性系）
    position_x: float              # X坐标 (m)
    position_y: float              # Y坐标 (m)
    position_z: float              # Z坐标 (m)
    
    # 欧拉角
    roll: float                    # 侧倾角 (rad)
    pitch: float                   # 俯仰角 (rad)
    yaw: float                     # 横摆角 (rad)
    
    # 速度（车身系）
    velocity_x: float              # 纵向速度 (m/s)
    velocity_y: float              # 横向速度 (m/s)
    velocity_z: float              # 垂向速度 (m/s)
    
    # 角速度（车身系）
    omega_x: float                 # 侧倾角速度 (rad/s)
    omega_y: float                 # 俯仰角速度 (rad/s)
    omega_z: float                 # 横摆角速度 (rad/s)
    
    # 加速度（车身系）
    acceleration_x: float          # 纵向加速度 (m/s²)
    acceleration_y: float          # 横向加速度 (m/s²)
    acceleration_z: float          # 垂向加速度 (m/s²)
    
    # 辅助量
    speed: float                   # 车速 (m/s)
    lateral_acceleration: float    # 侧向加速度 (m/s²)
    longitudinal_acceleration: float  # 纵向加速度 (m/s²)

@dataclass
class DynamicsInput:
    """动力学输入"""
    # 外力
    drive_torque: float            # 驱动扭矩 (N·m)
    brake_torques: BrakeForces     # 制动力
    
    # 悬架力
    suspension_forces: List[float]  # 四轮悬架力 (N)
    
    # 轮胎力
    tire_forces: List[TireForces]  # 四轮轮胎力
    
    # 环境力
    aero_force_x: float            # 空气阻力 (N)
    aero_force_z: float            # 空气升力 (N)
    grade_force: float             # 坡道力 (N)

class IVehicleDynamicsInterface(Protocol):
    """车辆动力学接口协议"""
    
    @property
    def state(self) -> VehicleState:
        """车辆状态（只读）"""
        ...
    
    def initialize(self, config: VehicleConfig) -> None:
        """初始化车辆动力学模型"""
        ...
    
    def reset(self, initial_state: Optional[VehicleState] = None) -> None:
        """
        重置车辆状态
        
        Args:
            initial_state: 初始状态（可选）
        """
        ...
    
    def set_input(self, input: DynamicsInput) -> None:
        """
        设置动力学输入
        
        Args:
            input: 动力学输入
        """
        ...
    
    def update(self, dt: float) -> None:
        """
        更新车辆状态（RK4积分）
        
        Args:
            dt: 时间步长 (s)
        """
        ...
    
    def get_state(self) -> VehicleState:
        """
        获取车辆状态
        
        Returns:
            车辆状态
        """
        ...
    
    def get_wheel_positions(self) -> List[Tuple[float, float, float]]:
        """
        获取四轮位置（车身系）
        
        Returns:
            四轮位置 [(x, y, z), ...]
        """
        ...
    
    def get_wheel_loads(self) -> List[float]:
        """
        获取四轮垂直载荷
        
        Returns:
            四轮载荷 [N, ...]
        """
        ...
    
    def get_body_acceleration(self) -> Tuple[float, float, float]:
        """
        获取车身加速度
        
        Returns:
            (ax, ay, az) (m/s²)
        """
        ...
```

---

## 5. 轮胎模型接口

### 5.1 Pacejka轮胎接口（ITireInterface）

```python
@dataclass
class TireConfig:
    """轮胎配置参数"""
    # 基本参数
    tire_radius: float             # 轮胎半径 (m)
    tire_width: float              # 轮胎宽度 (m)
    aspect_ratio: float            # 扁平比
    
    # Pacejka参数（纵向）
    B_x: float                     # 刚度因子
    C_x: float                     # 形状因子
    D_x: float                     # 峰值因子
    E_x: float                     # 曲率因子
    
    # Pacejka参数（侧向）
    B_y: float
    C_y: float
    D_y: float
    E_y: float
    
    # 摩擦系数
    mu_x: float                    # 纵向摩擦系数
    mu_y: float                    # 侧向摩擦系数

@dataclass
class TireInput:
    """轮胎输入"""
    # 运动学
    wheel_speed: float             # 轮速 (rad/s)
    wheel_velocity_x: float        # 轮心纵向速度 (m/s)
    wheel_velocity_y: float        # 轮心横向速度 (m/s)
    steering_angle: float          # 转向角 (rad)
    
    # 力学
    vertical_load: float           # 垂直载荷 (N)
    
    # 环境
    friction_coefficient: float    # 路面摩擦系数

@dataclass
class TireForces:
    """轮胎力"""
    F_x: float                     # 纵向力 (N)
    F_y: float                     # 侧向力 (N)
    F_z: float                     # 垂直载荷 (N)
    M_z: float                     # 回正力矩 (N·m)

class ITireInterface(Protocol):
    """轮胎接口协议"""
    
    def initialize(self, config: TireConfig) -> None:
        """初始化轮胎模型"""
        ...
    
    def calculate(self, input: TireInput) -> TireForces:
        """
        计算轮胎力
        
        Args:
            input: 轮胎输入
        
        Returns:
            轮胎力
        """
        ...
    
    def get_slip_ratio(self) -> float:
        """
        获取滑移率
        
        Returns:
            滑移率 (-1 到 1)
        """
        ...
    
    def get_slip_angle(self) -> float:
        """
        获取侧偏角
        
        Returns:
            侧偏角 (rad)
        """
        ...
```

---

## 6. 实时调度接口

### 6.1 调度器接口（ISchedulerInterface）

```python
from typing import Callable, List
from dataclasses import dataclass

@dataclass
class TaskConfig:
    """任务配置"""
    name: str                      # 任务名称
    priority: int                  # 优先级 (1-99)
    period: float                  # 周期 (s)
    deadline: float                # 截止时间 (s)
    cpu_affinity: List[int]        # CPU亲和性

@dataclass
class TaskStatistics:
    """任务统计"""
    name: str
    count: int                     # 执行次数
    avg_time: float                # 平均执行时间 (s)
    max_time: float                # 最大执行时间 (s)
    min_time: float                # 最小执行时间 (s)
    deadline_misses: int           # 截止时间错失次数

TaskFunction = Callable[[float], None]  # 任务函数类型

class ISchedulerInterface(Protocol):
    """实时调度器接口"""
    
    def initialize(self) -> None:
        """
        初始化调度器
        
        配置实时调度策略、CPU亲和性、内存锁定等
        """
        ...
    
    def register_task(
        self,
        name: str,
        task_func: TaskFunction,
        config: TaskConfig
    ) -> None:
        """
        注册任务
        
        Args:
            name: 任务名称
            task_func: 任务函数，参数为dt
            config: 任务配置
        """
        ...
    
    def start(self) -> None:
        """启动调度器"""
        ...
    
    def stop(self) -> None:
        """停止调度器"""
        ...
    
    def get_task_statistics(self, name: str) -> TaskStatistics:
        """
        获取任务统计信息
        
        Args:
            name: 任务名称
        
        Returns:
            任务统计
        """
        ...
    
    def get_all_statistics(self) -> List[TaskStatistics]:
        """获取所有任务统计信息"""
        ...
```

---

## 7. HIL接口

### 7.1 CAN接口（ICANInterface）

```python
from typing import List, Optional

@dataclass
class CANMessage:
    """CAN消息"""
    id: int                        # CAN ID
    dlc: int                       # 数据长度
    data: bytes                    # 数据
    timestamp: float               # 时间戳 (s)

@dataclass
class CANSignal:
    """CAN信号定义"""
    name: str                      # 信号名称
    start_bit: int                 # 起始位
    bit_length: int                # 位长度
    scale: float                   # 缩放因子
    offset: float                  # 偏移量
    is_signed: bool                # 是否有符号

class ICANInterface(Protocol):
    """CAN接口协议"""
    
    def initialize(self, interface: str, bitrate: int) -> None:
        """
        初始化CAN接口
        
        Args:
            interface: 接口名称（如"can0"）
            bitrate: 波特率（如500000）
        """
        ...
    
    def send(self, message: CANMessage) -> bool:
        """
        发送CAN消息
        
        Args:
            message: CAN消息
        
        Returns:
            True: 成功，False: 失败
        """
        ...
    
    def receive(self, timeout_ms: int) -> Optional[CANMessage]:
        """
        接收CAN消息
        
        Args:
            timeout_ms: 超时时间 (ms)
        
        Returns:
            CAN消息（如果超时则返回None）
        """
        ...
    
    def encode_signal(self, value: float, signal: CANSignal) -> int:
        """
        编码CAN信号
        
        Args:
            value: 物理值
            signal: 信号定义
        
        Returns:
            原始值
        """
        ...
    
    def decode_signal(self, raw_value: int, signal: CANSignal) -> float:
        """
        解码CAN信号
        
        Args:
            raw_value: 原始值
            signal: 信号定义
        
        Returns:
            物理值
        """
        ...
```

---

## 8. 通用数据类型

### 8.1 基本数据类型

```python
# common_types.py

from typing import NewType, Tuple
from dataclasses import dataclass

# 物理量类型（增强类型安全性）
Velocity = NewType('Velocity', float)      # m/s
Acceleration = NewType('Acceleration', float)  # m/s²
Force = NewType('Force', float)            # N
Torque = NewType('Torque', float)          # N·m
Power = NewType('Power', float)            # W
Angle = NewType('Angle', float)            # rad
AngularVelocity = NewType('AngularVelocity', float)  # rad/s

@dataclass(frozen=True)
class Vector3D:
    """三维向量"""
    x: float
    y: float
    z: float
    
    def __add__(self, other: 'Vector3D') -> 'Vector3D':
        return Vector3D(self.x + other.x, self.y + other.y, self.z + other.z)
    
    def __sub__(self, other: 'Vector3D') -> 'Vector3D':
        return Vector3D(self.x - other.x, self.y - other.y, self.z - other.z)
    
    def __mul__(self, scalar: float) -> 'Vector3D':
        return Vector3D(self.x * scalar, self.y * scalar, self.z * scalar)
    
    def magnitude(self) -> float:
        return (self.x**2 + self.y**2 + self.z**2)**0.5
    
    def normalize(self) -> 'Vector3D':
        mag = self.magnitude()
        if mag == 0:
            return Vector3D(0, 0, 0)
        return self * (1.0 / mag)

@dataclass(frozen=True)
class Quaternion:
    """四元数"""
    w: float
    x: float
    y: float
    z: float
    
    def to_euler_angles(self) -> Tuple[Angle, Angle, Angle]:
        """转换为欧拉角（roll, pitch, yaw）"""
        # 实现略
        pass
```

### 8.2 枚举类型

```python
# enums.py

from enum import Enum, auto

class SimulationState(Enum):
    """仿真状态"""
    UNINITIALIZED = auto()
    INITIALIZED = auto()
    RUNNING = auto()
    PAUSED = auto()
    STOPPED = auto()
    ERROR = auto()

class FaultType(Enum):
    """故障类型"""
    NONE = 0
    SENSOR_FAULT = 1
    ACTUATOR_FAULT = 2
    COMMUNICATION_FAULT = 3
    NUMERICAL_INSTABILITY = 4
    TIMEOUT = 5
    OUT_OF_RANGE = 6

class WheelPosition(Enum):
    """车轮位置"""
    FRONT_LEFT = 0
    FRONT_RIGHT = 1
    REAR_LEFT = 2
    REAR_RIGHT = 3

class CoordinateSystem(Enum):
    """坐标系"""
    EARTH_FIXED = "Earth-Fixed (Inertial)"
    BODY_FIXED = "Body-Fixed"
    WHEEL_FIXED = "Wheel-Fixed"
```

---

## 9. 错误处理机制

### 9.1 异常层次结构

```python
# exceptions.py

class EVSimulationError(Exception):
    """基础异常类"""
    pass

class ConfigurationError(EVSimulationError):
    """配置错误"""
    pass

class ParameterOutOfRangeError(ConfigurationError):
    """参数超出范围"""
    def __init__(self, param_name: str, value: float, valid_range: tuple):
        self.param_name = param_name
        self.value = value
        self.valid_range = valid_range
        super().__init__(
            f"Parameter '{param_name}' value {value} is outside valid range {valid_range}"
        )

class NumericalInstabilityError(EVSimulationError):
    """数值不稳定"""
    def __init__(self, component: str, value: float):
        self.component = component
        self.value = value
        super().__init__(
            f"Numerical instability detected in {component}: value={value}"
        )

class RealtimeViolationError(EVSimulationError):
    """实时性违反"""
    def __init__(self, task_name: str, deadline_ms: float, actual_ms: float):
        self.task_name = task_name
        self.deadline_ms = deadline_ms
        self.actual_ms = actual_ms
        super().__init__(
            f"Deadline miss in task '{task_name}': "
            f"deadline={deadline_ms}ms, actual={actual_ms}ms"
        )

class CommunicationError(EVSimulationError):
    """通信错误"""
    pass

class CANError(CommunicationError):
    """CAN通信错误"""
    pass

class HardwareError(EVSimulationError):
    """硬件错误"""
    pass
```

### 9.2 错误处理策略

```python
# error_handler.py

from typing import Callable, Optional
from enum import Enum

class ErrorSeverity(Enum):
    """错误严重程度"""
    WARNING = 0      # 警告：记录日志，继续运行
    ERROR = 1        # 错误：记录日志，尝试恢复
    CRITICAL = 2     # 严重：记录日志，降级运行
    FATAL = 3        # 致命：停止仿真

class ErrorHandler:
    """错误处理器"""
    
    def __init__(self):
        self.error_count = {}
        self.max_errors_per_type = 10
    
    def handle(
        self,
        error: EVSimulationError,
        severity: ErrorSeverity,
        context: Optional[dict] = None
    ) -> bool:
        """
        处理错误
        
        Args:
            error: 异常对象
            severity: 严重程度
            context: 上下文信息
        
        Returns:
            True: 继续运行，False: 停止仿真
        """
        # 记录错误
        error_type = type(error).__name__
        self.error_count[error_type] = self.error_count.get(error_type, 0) + 1
        
        # 记录日志
        self._log_error(error, severity, context)
        
        # 根据严重程度采取行动
        if severity == ErrorSeverity.WARNING:
            return True
        
        elif severity == ErrorSeverity.ERROR:
            # 尝试恢复
            if self._attempt_recovery(error):
                return True
            else:
                return False
        
        elif severity == ErrorSeverity.CRITICAL:
            # 降级运行
            self._degrade_system()
            return True
        
        elif severity == ErrorSeverity.FATAL:
            # 停止仿真
            return False
        
        return True
    
    def _log_error(self, error: EVSimulationError, severity: ErrorSeverity, context: Optional[dict]):
        """记录错误日志"""
        # 实现略
        pass
    
    def _attempt_recovery(self, error: EVSimulationError) -> bool:
        """尝试恢复"""
        # 实现略
        return False
    
    def _degrade_system(self):
        """降级系统"""
        # 实现略
        pass
```

---

## 10. 接口版本管理

### 10.1 版本兼容性

```python
# version.py

from dataclasses import dataclass
from typing import Tuple

@dataclass(frozen=True)
class InterfaceVersion:
    """接口版本"""
    major: int
    minor: int
    patch: int
    
    def __str__(self) -> str:
        return f"{self.major}.{self.minor}.{self.patch}"
    
    def is_compatible_with(self, other: 'InterfaceVersion') -> bool:
        """
        检查版本兼容性
        
        规则：
        - major必须相同
        - minor可以不同（向后兼容）
        - patch不影响兼容性
        """
        return self.major == other.major
    
    def __lt__(self, other: 'InterfaceVersion') -> bool:
        return (self.major, self.minor, self.patch) < (other.major, other.minor, other.patch)

# 当前版本
CURRENT_VERSION = InterfaceVersion(1, 0, 0)

# 支持的最低版本
MIN_SUPPORTED_VERSION = InterfaceVersion(1, 0, 0)
```

### 10.2 版本检查

```python
# version_checker.py

class VersionChecker:
    """版本检查器"""
    
    @staticmethod
    def check_compatibility(
        interface_version: InterfaceVersion,
        required_version: InterfaceVersion
    ) -> Tuple[bool, str]:
        """
        检查接口兼容性
        
        Args:
            interface_version: 接口实际版本
            required_version: 需要的版本
        
        Returns:
            (是否兼容, 消息)
        """
        if interface_version.major != required_version.major:
            return False, f"Incompatible major version: {interface_version} vs {required_version}"
        
        if interface_version.minor < required_version.minor:
            return False, f"Interface version too old: {interface_version} < {required_version}"
        
        return True, "Versions compatible"
```

---

## 附录

### A. 接口清单

| 模块 | 接口名称 | 版本 | 描述 |
|-----|---------|------|------|
| 动力系统 | IMotorInterface | 1.0.0 | 电机接口 |
| 动力系统 | IBatteryInterface | 1.0.0 | 电池接口 |
| 动力系统 | ITransmissionInterface | 1.0.0 | 变速器接口 |
| 动力系统 | IPowertrainInterface | 1.0.0 | 动力系统集成接口 |
| 底盘系统 | ISuspensionInterface | 1.0.0 | 悬架接口 |
| 底盘系统 | ISteeringInterface | 1.0.0 | 转向接口 |
| 底盘系统 | IBrakingInterface | 1.0.0 | 制动接口 |
| 车辆动力学 | IVehicleDynamicsInterface | 1.0.0 | 6-DOF动力学接口 |
| 轮胎模型 | ITireInterface | 1.0.0 | 轮胎接口 |
| 实时调度 | ISchedulerInterface | 1.0.0 | 调度器接口 |
| HIL接口 | ICANInterface | 1.0.0 | CAN接口 |
| HIL接口 | IEthernetInterface | 1.0.0 | 以太网接口 |

### B. 变更历史

| 版本 | 日期 | 作者 | 变更内容 |
|-----|------|------|---------|
| 1.0.0 | 2026-03-05 | ArchitectAgent | 初始版本 |

---

**文档结束**

本模块接口规范详细定义了六自由度新能源车辆动力学仿真系统中所有模块的接口，包括Python和C++两种语言的定义。所有接口都遵循清晰的设计原则，包含完整的参数说明、返回值说明、异常说明和使用示例。
