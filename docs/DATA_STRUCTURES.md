# 六自由度新能源车辆动力学仿真系统 - 数据结构定义

**文档版本**: v1.0  
**创建日期**: 2026-03-05  
**作者**: ArchitectAgent  
**状态**: 设计阶段

---

## 目录

1. [设计原则](#1-设计原则)
2. [基础数据结构](#2-基础数据结构)
3. [物理模型数据结构](#3-物理模型数据结构)
4. [消息格式定义](#4-消息格式定义)
5. [配置文件格式](#5-配置文件格式)
6. [日志格式](#6-日志格式)
7. [数据序列化](#7-数据序列化)
8. [内存布局优化](#8-内存布局优化)

---

## 1. 设计原则

### 1.1 数据结构设计原则

1. **类型安全**：使用强类型，避免隐式转换
2. **内存对齐**：按缓存行对齐，提高访问效率
3. **不可变性**：只读数据使用不可变结构
4. **零拷贝**：支持零拷贝传输
5. **序列化友好**：支持高效序列化/反序列化

### 1.2 命名规范

**Python命名**：
- 数据类：PascalCase（如`VehicleState`）
- 字段：snake_case（如`velocity_x`）
- 常量：UPPER_SNAKE_CASE（如`MAX_VOLTAGE`）

**C++命名**：
- 结构体：PascalCase（如`VehicleState`）
- 成员变量：snake_case_（如`velocity_x_`）
- 常量：kCamelCase（如`kMaxVoltage`）

---

## 2. 基础数据结构

### 2.1 向量和矩阵

#### 2.1.1 Python实现

```python
# data_structures/vectors.py

from dataclasses import dataclass
from typing import Union, List
import math

@dataclass(frozen=True)
class Vector2D:
    """二维向量"""
    x: float
    y: float
    
    def __add__(self, other: 'Vector2D') -> 'Vector2D':
        return Vector2D(self.x + other.x, self.y + other.y)
    
    def __sub__(self, other: 'Vector2D') -> 'Vector2D':
        return Vector2D(self.x - other.x, self.y - other.y)
    
    def __mul__(self, scalar: float) -> 'Vector2D':
        return Vector2D(self.x * scalar, self.y * scalar)
    
    def __truediv__(self, scalar: float) -> 'Vector2D':
        return Vector2D(self.x / scalar, self.y / scalar)
    
    def dot(self, other: 'Vector2D') -> float:
        """点积"""
        return self.x * other.x + self.y * other.y
    
    def cross(self, other: 'Vector2D') -> float:
        """叉积（返回标量）"""
        return self.x * other.y - self.y * other.x
    
    def magnitude(self) -> float:
        """模长"""
        return math.sqrt(self.x**2 + self.y**2)
    
    def magnitude_squared(self) -> float:
        """模长平方（避免开方）"""
        return self.x**2 + self.y**2
    
    def normalize(self) -> 'Vector2D':
        """归一化"""
        mag = self.magnitude()
        if mag < 1e-10:
            return Vector2D(0.0, 0.0)
        return self / mag
    
    def angle(self) -> float:
        """角度（弧度）"""
        return math.atan2(self.y, self.x)
    
    def rotate(self, angle: float) -> 'Vector2D':
        """旋转"""
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)
        return Vector2D(
            self.x * cos_a - self.y * sin_a,
            self.x * sin_a + self.y * cos_a
        )
    
    @staticmethod
    def from_polar(magnitude: float, angle: float) -> 'Vector2D':
        """从极坐标创建"""
        return Vector2D(
            magnitude * math.cos(angle),
            magnitude * math.sin(angle)
        )
    
    def to_tuple(self) -> tuple:
        return (self.x, self.y)
    
    def to_list(self) -> List[float]:
        return [self.x, self.y]

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
    
    def __truediv__(self, scalar: float) -> 'Vector3D':
        return Vector3D(self.x / scalar, self.y / scalar, self.z / scalar)
    
    def dot(self, other: 'Vector3D') -> float:
        """点积"""
        return self.x * other.x + self.y * other.y + self.z * other.z
    
    def cross(self, other: 'Vector3D') -> 'Vector3D':
        """叉积"""
        return Vector3D(
            self.y * other.z - self.z * other.y,
            self.z * other.x - self.x * other.z,
            self.x * other.y - self.y * other.x
        )
    
    def magnitude(self) -> float:
        """模长"""
        return math.sqrt(self.x**2 + self.y**2 + self.z**2)
    
    def magnitude_squared(self) -> float:
        """模长平方"""
        return self.x**2 + self.y**2 + self.z**2
    
    def normalize(self) -> 'Vector3D':
        """归一化"""
        mag = self.magnitude()
        if mag < 1e-10:
            return Vector3D(0.0, 0.0, 0.0)
        return self / mag
    
    def to_tuple(self) -> tuple:
        return (self.x, self.y, self.z)
    
    def to_list(self) -> List[float]:
        return [self.x, self.y, self.z]
    
    def to_numpy(self):
        """转换为numpy数组"""
        import numpy as np
        return np.array([self.x, self.y, self.z])

@dataclass(frozen=True)
class Quaternion:
    """四元数（w, x, y, z）"""
    w: float
    x: float
    y: float
    z: float
    
    @staticmethod
    def from_euler_angles(roll: float, pitch: float, yaw: float) -> 'Quaternion':
        """从欧拉角创建（ZYX顺序）"""
        cr = math.cos(roll / 2)
        sr = math.sin(roll / 2)
        cp = math.cos(pitch / 2)
        sp = math.sin(pitch / 2)
        cy = math.cos(yaw / 2)
        sy = math.sin(yaw / 2)
        
        w = cr * cp * cy + sr * sp * sy
        x = sr * cp * cy - cr * sp * sy
        y = cr * sp * cy + sr * cp * sy
        z = cr * cp * sy - sr * sp * cy
        
        return Quaternion(w, x, y, z)
    
    def to_euler_angles(self) -> tuple:
        """转换为欧拉角（roll, pitch, yaw）"""
        # Roll (x-axis rotation)
        sinr_cosp = 2 * (self.w * self.x + self.y * self.z)
        cosr_cosp = 1 - 2 * (self.x * self.x + self.y * self.y)
        roll = math.atan2(sinr_cosp, cosr_cosp)
        
        # Pitch (y-axis rotation)
        sinp = 2 * (self.w * self.y - self.z * self.x)
        if abs(sinp) >= 1:
            pitch = math.copysign(math.pi / 2, sinp)
        else:
            pitch = math.asin(sinp)
        
        # Yaw (z-axis rotation)
        siny_cosp = 2 * (self.w * self.z + self.x * self.y)
        cosy_cosp = 1 - 2 * (self.y * self.y + self.z * self.z)
        yaw = math.atan2(siny_cosp, cosy_cosp)
        
        return (roll, pitch, yaw)
    
    def __mul__(self, other: 'Quaternion') -> 'Quaternion':
        """四元数乘法"""
        return Quaternion(
            w=self.w*other.w - self.x*other.x - self.y*other.y - self.z*other.z,
            x=self.w*other.x + self.x*other.w + self.y*other.z - self.z*other.y,
            y=self.w*other.y - self.x*other.z + self.y*other.w + self.z*other.x,
            z=self.w*other.z + self.x*other.y - self.y*other.x + self.z*other.w
        )
    
    def conjugate(self) -> 'Quaternion':
        """共轭"""
        return Quaternion(self.w, -self.x, -self.y, -self.z)
    
    def normalize(self) -> 'Quaternion':
        """归一化"""
        mag = math.sqrt(self.w**2 + self.x**2 + self.y**2 + self.z**2)
        if mag < 1e-10:
            return Quaternion(1.0, 0.0, 0.0, 0.0)
        return Quaternion(self.w/mag, self.x/mag, self.y/mag, self.z/mag)
    
    def rotate_vector(self, v: Vector3D) -> Vector3D:
        """旋转向量"""
        qv = Quaternion(0, v.x, v.y, v.z)
        result = self * qv * self.conjugate()
        return Vector3D(result.x, result.y, result.z)
```

#### 2.1.2 C++实现

```cpp
// data_structures/vectors.h
#pragma once

#include <cmath>
#include <array>

namespace ev_simulation {
namespace math {

/**
 * @brief 三维向量（缓存行对齐）
 */
struct alignas(32) Vector3D {
    union {
        struct {
            float x, y, z, _pad;  // 填充到16字节
        };
        float data[4];
    };
    
    // 构造函数
    Vector3D() : x(0), y(0), z(0), _pad(0) {}
    Vector3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_), _pad(0) {}
    
    // 运算符重载
    Vector3D operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }
    
    Vector3D operator-(const Vector3D& other) const {
        return Vector3D(x - other.x, y - other.y, z - other.z);
    }
    
    Vector3D operator*(float scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }
    
    Vector3D operator/(float scalar) const {
        return Vector3D(x / scalar, y / scalar, z / scalar);
    }
    
    // 点积
    float dot(const Vector3D& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
    // 叉积
    Vector3D cross(const Vector3D& other) const {
        return Vector3D(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
    
    // 模长
    float magnitude() const {
        return std::sqrt(x*x + y*y + z*z);
    }
    
    // 模长平方
    float magnitudeSquared() const {
        return x*x + y*y + z*z;
    }
    
    // 归一化
    Vector3D normalize() const {
        float mag = magnitude();
        if (mag < 1e-10f) {
            return Vector3D(0, 0, 0);
        }
        return *this / mag;
    }
    
    // 转换为数组
    std::array<float, 3> toArray() const {
        return {x, y, z};
    }
};

/**
 * @brief 四元数
 */
struct alignas(16) Quaternion {
    float w, x, y, z;
    
    Quaternion() : w(1), x(0), y(0), z(0) {}
    Quaternion(float w_, float x_, float y_, float z_) 
        : w(w_), x(x_), y(y_), z(z_) {}
    
    // 从欧拉角创建
    static Quaternion fromEulerAngles(float roll, float pitch, float yaw) {
        float cr = std::cos(roll / 2);
        float sr = std::sin(roll / 2);
        float cp = std::cos(pitch / 2);
        float sp = std::sin(pitch / 2);
        float cy = std::cos(yaw / 2);
        float sy = std::sin(yaw / 2);
        
        return Quaternion(
            cr * cp * cy + sr * sp * sy,
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy
        );
    }
    
    // 四元数乘法
    Quaternion operator*(const Quaternion& other) const {
        return Quaternion(
            w*other.w - x*other.x - y*other.y - z*other.z,
            w*other.x + x*other.w + y*other.z - z*other.y,
            w*other.y - x*other.z + y*other.w + z*other.x,
            w*other.z + x*other.y - y*other.x + z*other.w
        );
    }
    
    // 归一化
    Quaternion normalize() const {
        float mag = std::sqrt(w*w + x*x + y*y + z*z);
        if (mag < 1e-10f) {
            return Quaternion(1, 0, 0, 0);
        }
        return Quaternion(w/mag, x/mag, y/mag, z/mag);
    }
};

/**
 * @brief 3x3矩阵（行优先）
 */
struct Matrix3x3 {
    float data[9];  // 行优先存储
    
    Matrix3x3() {
        for (int i = 0; i < 9; i++) data[i] = 0;
    }
    
    // 单位矩阵
    static Matrix3x3 identity() {
        Matrix3x3 m;
        m.data[0] = m.data[4] = m.data[8] = 1.0f;
        return m;
    }
    
    // 旋转矩阵（绕Z轴）
    static Matrix3x3 rotationZ(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Matrix3x3 m;
        m.data[0] = c;  m.data[1] = -s; m.data[2] = 0;
        m.data[3] = s;  m.data[4] = c;  m.data[5] = 0;
        m.data[6] = 0;  m.data[7] = 0;  m.data[8] = 1;
        return m;
    }
    
    // 矩阵乘向量
    Vector3D operator*(const Vector3D& v) const {
        return Vector3D(
            data[0]*v.x + data[1]*v.y + data[2]*v.z,
            data[3]*v.x + data[4]*v.y + data[5]*v.z,
            data[6]*v.x + data[7]*v.y + data[8]*v.z
        );
    }
    
    // 矩阵乘法
    Matrix3x3 operator*(const Matrix3x3& other) const {
        Matrix3x3 result;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result.data[i*3 + j] = 0;
                for (int k = 0; k < 3; k++) {
                    result.data[i*3 + j] += data[i*3 + k] * other.data[k*3 + j];
                }
            }
        }
        return result;
    }
};

} // namespace math
} // namespace ev_simulation
```

---

## 3. 物理模型数据结构

### 3.1 车辆状态数据结构

#### 3.1.1 Python定义

```python
# data_structures/vehicle_state.py

from dataclasses import dataclass, field
from typing import List
from enum import Enum
import time

class VehicleMode(Enum):
    """车辆模式"""
    PARK = 0
    REVERSE = 1
    NEUTRAL = 2
    DRIVE = 3

@dataclass
class VehicleState:
    """
    车辆完整状态（6-DOF）
    
    内存布局：连续存储，便于序列化
    """
    # === 时间戳 ===
    timestamp: float = 0.0                    # 时间戳 (s)
    simulation_time: float = 0.0              # 仿真时间 (s)
    
    # === 位置（惯性系）===
    position_x: float = 0.0                   # X坐标 (m)
    position_y: float = 0.0                   # Y坐标 (m)
    position_z: float = 0.0                   # Z坐标 (m)
    
    # === 姿态（欧拉角）===
    roll: float = 0.0                         # 侧倾角 (rad)
    pitch: float = 0.0                        # 俯仰角 (rad)
    yaw: float = 0.0                          # 横摆角 (rad)
    
    # === 速度（车身系）===
    velocity_x: float = 0.0                   # 纵向速度 (m/s)
    velocity_y: float = 0.0                   # 横向速度 (m/s)
    velocity_z: float = 0.0                   # 垂向速度 (m/s)
    
    # === 角速度（车身系）===
    omega_x: float = 0.0                      # 侧倾角速度 (rad/s)
    omega_y: float = 0.0                      # 俯仰角速度 (rad/s)
    omega_z: float = 0.0                      # 横摆角速度 (rad/s)
    
    # === 加速度（车身系）===
    acceleration_x: float = 0.0               # 纵向加速度 (m/s²)
    acceleration_y: float = 0.0               # 横向加速度 (m/s²)
    acceleration_z: float = 0.0               # 垂向加速度 (m/s²)
    
    # === 车轮状态 ===
    wheel_speeds: List[float] = field(default_factory=lambda: [0.0]*4)  # 四轮转速 (rad/s)
    wheel_angles: List[float] = field(default_factory=lambda: [0.0]*4)  # 四轮转角 (rad)
    wheel_loads: List[float] = field(default_factory=lambda: [0.0]*4)   # 四轮载荷 (N)
    
    # === 档位和模式 ===
    gear: int = 0                             # 档位
    mode: VehicleMode = VehicleMode.PARK      # 车辆模式
    
    # === 辅助量（计算得出）===
    speed: float = 0.0                        # 车速 (m/s)
    speed_kmh: float = 0.0                    # 车速 (km/h)
    
    def update_derived_quantities(self):
        """更新派生量"""
        self.speed = (self.velocity_x**2 + self.velocity_y**2)**0.5
        self.speed_kmh = self.speed * 3.6
    
    def to_dict(self) -> dict:
        """转换为字典（用于序列化）"""
        return {
            'timestamp': self.timestamp,
            'simulation_time': self.simulation_time,
            'position': [self.position_x, self.position_y, self.position_z],
            'euler_angles': [self.roll, self.pitch, self.yaw],
            'velocity': [self.velocity_x, self.velocity_y, self.velocity_z],
            'omega': [self.omega_x, self.omega_y, self.omega_z],
            'acceleration': [self.acceleration_x, self.acceleration_y, self.acceleration_z],
            'wheel_speeds': self.wheel_speeds,
            'wheel_angles': self.wheel_angles,
            'wheel_loads': self.wheel_loads,
            'gear': self.gear,
            'mode': self.mode.value,
            'speed': self.speed,
            'speed_kmh': self.speed_kmh
        }
    
    @staticmethod
    def from_dict(data: dict) -> 'VehicleState':
        """从字典创建"""
        state = VehicleState()
        state.timestamp = data['timestamp']
        state.simulation_time = data['simulation_time']
        state.position_x, state.position_y, state.position_z = data['position']
        state.roll, state.pitch, state.yaw = data['euler_angles']
        state.velocity_x, state.velocity_y, state.velocity_z = data['velocity']
        state.omega_x, state.omega_y, state.omega_z = data['omega']
        state.acceleration_x, state.acceleration_y, state.acceleration_z = data['acceleration']
        state.wheel_speeds = data['wheel_speeds']
        state.wheel_angles = data['wheel_angles']
        state.wheel_loads = data['wheel_loads']
        state.gear = data['gear']
        state.mode = VehicleMode(data['mode'])
        state.speed = data['speed']
        state.speed_kmh = data['speed_kmh']
        return state
```

#### 3.1.2 C++定义

```cpp
// data_structures/vehicle_state.h
#pragma once

#include <array>
#include <cstdint>

namespace ev_simulation {
namespace core {

/**
 * @brief 车辆模式
 */
enum class VehicleMode : uint8_t {
    PARK = 0,
    REVERSE = 1,
    NEUTRAL = 2,
    DRIVE = 3
};

/**
 * @brief 车辆完整状态（6-DOF）
 * 
 * 内存布局：紧凑排列，便于DMA传输
 * 总大小：约200字节
 */
struct alignas(64) VehicleState {
    // === 时间戳 ===
    double timestamp{0.0};           ///< 时间戳 (s)
    double simulation_time{0.0};     ///< 仿真时间 (s)
    
    // === 位置（惯性系）===
    float position_x{0.0f};          ///< X坐标 (m)
    float position_y{0.0f};          ///< Y坐标 (m)
    float position_z{0.0f};          ///< Z坐标 (m)
    
    // === 姿态（欧拉角）===
    float roll{0.0f};                ///< 侧倾角 (rad)
    float pitch{0.0f};               ///< 俯仰角 (rad)
    float yaw{0.0f};                 ///< 横摆角 (rad)
    
    // === 速度（车身系）===
    float velocity_x{0.0f};          ///< 纵向速度 (m/s)
    float velocity_y{0.0f};          ///< 横向速度 (m/s)
    float velocity_z{0.0f};          ///< 垂向速度 (m/s)
    
    // === 角速度（车身系）===
    float omega_x{0.0f};             ///< 侧倾角速度 (rad/s)
    float omega_y{0.0f};             ///< 俯仰角速度 (rad/s)
    float omega_z{0.0f};             ///< 横摆角速度 (rad/s)
    
    // === 加速度（车身系）===
    float acceleration_x{0.0f};      ///< 纵向加速度 (m/s²)
    float acceleration_y{0.0f};      ///< 横向加速度 (m/s²)
    float acceleration_z{0.0f};      ///< 垂向加速度 (m/s²)
    
    // === 车轮状态 ===
    std::array<float, 4> wheel_speeds{{0.0f, 0.0f, 0.0f, 0.0f}};   ///< 四轮转速 (rad/s)
    std::array<float, 4> wheel_angles{{0.0f, 0.0f, 0.0f, 0.0f}};   ///< 四轮转角 (rad)
    std::array<float, 4> wheel_loads{{0.0f, 0.0f, 0.0f, 0.0f}};    ///< 四轮载荷 (N)
    
    // === 档位和模式 ===
    int8_t gear{0};                  ///< 档位
    VehicleMode mode{VehicleMode::PARK};  ///< 车辆模式
    
    // === 辅助量 ===
    float speed{0.0f};               ///< 车速 (m/s)
    float speed_kmh{0.0f};           ///< 车速 (km/h)
    
    /**
     * @brief 更新派生量
     */
    void updateDerivedQuantities() {
        speed = std::sqrt(velocity_x*velocity_x + velocity_y*velocity_y);
        speed_kmh = speed * 3.6f;
    }
    
    /**
     * @brief 获取结构体大小（字节）
     */
    static constexpr size_t getSize() {
        return sizeof(VehicleState);
    }
};

// 静态断言：确保内存布局符合预期
static_assert(sizeof(VehicleState) <= 256, "VehicleState too large");
static_assert(alignof(VehicleState) == 64, "VehicleState alignment incorrect");

} // namespace core
} // namespace ev_simulation
```

### 3.2 动力系统数据结构

```python
# data_structures/powertrain_state.py

@dataclass
class MotorState:
    """电机状态"""
    # 扭矩和转速
    torque_request: float = 0.0       # 扭矩请求 (N·m)
    torque_actual: float = 0.0        # 实际扭矩 (N·m)
    rotor_speed: float = 0.0          # 转子转速 (rpm)
    rotor_angle: float = 0.0          # 转子角度 (rad)
    
    # 电气状态
    current_d: float = 0.0            # d轴电流 (A)
    current_q: float = 0.0            # q轴电流 (A)
    voltage_d: float = 0.0            # d轴电压 (V)
    voltage_q: float = 0.0            # q轴电压 (V)
    dc_bus_voltage: float = 400.0     # 直流母线电压 (V)
    
    # 热状态
    temperature: float = 25.0         # 电机温度 (°C)
    temperature_limit: float = 150.0  # 温度限制 (°C)
    
    # 诊断
    is_overloaded: bool = False
    is_overheated: bool = False
    fault_code: int = 0

@dataclass
class BatteryState:
    """电池状态"""
    # 荷电状态
    soc: float = 0.8                  # 荷电状态 (0-1)
    soh: float = 1.0                  # 健康状态 (0-1)
    
    # 电压电流
    voltage: float = 400.0            # 端电压 (V)
    current: float = 0.0              # 电流 (A)，正值放电
    open_circuit_voltage: float = 400.0  # 开路电压 (V)
    
    # 功率
    power: float = 0.0                # 功率 (W)
    available_power: float = 150000.0 # 可用功率 (W)
    
    # 温度
    temperature: float = 25.0         # 电池温度 (°C)
    
    # 老化
    cycle_count: int = 0              # 循环次数
    total_energy_throughput: float = 0.0  # 累计能量吞吐量 (kWh)
    
    # 诊断
    is_overcharged: bool = False
    is_overdischarged: bool = False
    is_overheated: bool = False
    fault_code: int = 0

@dataclass
class PowertrainState:
    """动力系统状态"""
    timestamp: float = 0.0
    
    motor: MotorState = field(default_factory=MotorState)
    battery: BatteryState = field(default_factory=BatteryState)
    
    # 驾驶员输入
    throttle_position: float = 0.0    # 油门踏板位置 (0-1)
    brake_position: float = 0.0       # 制动踏板位置 (0-1)
    
    # 输出
    drive_torque: float = 0.0         # 驱动扭矩 (N·m)
    regeneration_torque: float = 0.0  # 再生制动扭矩 (N·m)
    
    def to_dict(self) -> dict:
        return {
            'timestamp': self.timestamp,
            'motor': {
                'torque_request': self.motor.torque_request,
                'torque_actual': self.motor.torque_actual,
                'rotor_speed': self.motor.rotor_speed,
                'temperature': self.motor.temperature,
                'dc_bus_voltage': self.motor.dc_bus_voltage
            },
            'battery': {
                'soc': self.battery.soc,
                'soh': self.battery.soh,
                'voltage': self.battery.voltage,
                'current': self.battery.current,
                'power': self.battery.power,
                'temperature': self.battery.temperature
            },
            'driver_input': {
                'throttle': self.throttle_position,
                'brake': self.brake_position
            },
            'output': {
                'drive_torque': self.drive_torque,
                'regen_torque': self.regeneration_torque
            }
        }
```

### 3.3 底盘系统数据结构

```python
# data_structures/chassis_state.py

@dataclass
class SuspensionState:
    """悬架状态（单轮）"""
    displacement: float = 0.0         # 悬架位移 (m)
    velocity: float = 0.0             # 悬架速度 (m/s)
    
    spring_force: float = 0.0         # 弹簧力 (N)
    damper_force: float = 0.0         # 阻尼力 (N)
    actuator_force: float = 0.0       # 作动器力 (N)
    total_force: float = 0.0          # 总力 (N)
    
    vertical_load: float = 0.0        # 垂直载荷 (N)

@dataclass
class SteeringState:
    """转向状态"""
    steering_wheel_angle: float = 0.0  # 转向盘转角 (deg)
    steering_wheel_torque: float = 0.0  # 转向盘扭矩 (N·m)
    front_wheel_angle: float = 0.0    # 前轮转角 (deg)
    assist_torque: float = 0.0        # 助力扭矩 (N·m)

@dataclass
class BrakingState:
    """制动状态"""
    brake_pressure: List[float] = field(default_factory=lambda: [0.0]*4)  # 四轮制动压力 (bar)
    brake_torque: List[float] = field(default_factory=lambda: [0.0]*4)    # 四轮制动力矩 (N·m)
    regen_torque: float = 0.0         # 再生制动力矩 (N·m)
    total_brake_torque: float = 0.0   # 总制动力矩 (N·m)

@dataclass
class ChassisState:
    """底盘系统状态"""
    timestamp: float = 0.0
    
    suspension: List[SuspensionState] = field(
        default_factory=lambda: [SuspensionState() for _ in range(4)]
    )
    steering: SteeringState = field(default_factory=SteeringState)
    braking: BrakingState = field(default_factory=BrakingState)
    
    def to_dict(self) -> dict:
        return {
            'timestamp': self.timestamp,
            'suspension': [
                {
                    'displacement': s.displacement,
                    'velocity': s.velocity,
                    'total_force': s.total_force,
                    'vertical_load': s.vertical_load
                } for s in self.suspension
            ],
            'steering': {
                'steering_wheel_angle': self.steering.steering_wheel_angle,
                'front_wheel_angle': self.steering.front_wheel_angle
            },
            'braking': {
                'brake_torque': self.braking.brake_torque,
                'regen_torque': self.braking.regen_torque,
                'total_brake_torque': self.braking.total_brake_torque
            }
        }
```

### 3.4 轮胎力数据结构

```python
# data_structures/tire_state.py

@dataclass
class TireForces:
    """轮胎力（单轮）"""
    F_x: float = 0.0                  # 纵向力 (N)
    F_y: float = 0.0                  # 侧向力 (N)
    F_z: float = 0.0                  # 垂直载荷 (N)
    M_z: float = 0.0                  # 回正力矩 (N·m)

@dataclass
class TireSlip:
    """轮胎滑移"""
    slip_ratio: float = 0.0           # 滑移率 (-1 到 1)
    slip_angle: float = 0.0           # 侧偏角 (rad)

@dataclass
class TireState:
    """轮胎状态（单轮）"""
    forces: TireForces = field(default_factory=TireForces)
    slip: TireSlip = field(default_factory=TireSlip)
    
    # 运动学
    wheel_speed: float = 0.0          # 轮速 (rad/s)
    wheel_velocity_x: float = 0.0     # 轮心纵向速度 (m/s)
    wheel_velocity_y: float = 0.0     # 轮心横向速度 (m/s)
    
    # 温度
    tire_temperature: float = 25.0    # 轮胎温度 (°C)

@dataclass
class FourWheelTireState:
    """四轮轮胎状态"""
    timestamp: float = 0.0
    tires: List[TireState] = field(
        default_factory=lambda: [TireState() for _ in range(4)]
    )
    
    def get_total_forces(self) -> dict:
        """计算总轮胎力"""
        total_fx = sum(t.forces.F_x for t in self.tires)
        total_fy = sum(t.forces.F_y for t in self.tires)
        total_mz = sum(t.forces.M_z for t in self.tires)
        
        return {
            'total_F_x': total_fx,
            'total_F_y': total_fy,
            'total_M_z': total_mz
        }
```

---

## 4. 消息格式定义

### 4.1 消息头格式

```python
# messaging/message_types.py

from dataclasses import dataclass
from enum import IntEnum
import struct
import time

class MessageType(IntEnum):
    """消息类型"""
    # 系统消息
    SYSTEM_INIT = 0x00
    SYSTEM_SHUTDOWN = 0x01
    SYSTEM_HEARTBEAT = 0x02
    
    # 模块更新消息
    POWERTRAIN_UPDATE = 0x10
    CHASSIS_UPDATE = 0x11
    VEHICLE_DYNAMICS_UPDATE = 0x12
    TIRE_UPDATE = 0x13
    
    # HIL消息
    HIL_COMMAND = 0x20
    HIL_RESPONSE = 0x21
    HIL_SENSOR_DATA = 0x22
    
    # 故障消息
    FAULT_INJECTION = 0x30
    FAULT_CLEAR = 0x31
    
    # 控制消息
    SIMULATION_START = 0x40
    SIMULATION_STOP = 0x41
    SIMULATION_PAUSE = 0x42
    SIMULATION_RESUME = 0x43

class MessagePriority(IntEnum):
    """消息优先级"""
    LOW = 0
    NORMAL = 1
    HIGH = 2
    CRITICAL = 3

@dataclass
class MessageHeader:
    """
    消息头（固定16字节）
    
    二进制格式：
    - type (1 byte): 消息类型
    - priority (1 byte): 优先级
    - sequence (2 bytes): 序列号
    - timestamp (8 bytes): 时间戳 (ns)
    - payload_size (4 bytes): 载荷大小
    """
    type: MessageType
    priority: MessagePriority
    sequence: int
    timestamp: int                    # 纳秒
    payload_size: int
    
    @staticmethod
    def size() -> int:
        return 16
    
    def pack(self) -> bytes:
        """序列化为字节"""
        return struct.pack(
            '<BBHIQI',
            self.type,
            self.priority,
            self.sequence,
            self.timestamp >> 32,     # 高32位
            self.timestamp & 0xFFFFFFFF,  # 低32位
            self.payload_size
        )
    
    @staticmethod
    def unpack(data: bytes) -> 'MessageHeader':
        """从字节反序列化"""
        type_, priority, sequence, ts_high, ts_low, payload_size = struct.unpack(
            '<BBHIQI', data[:16]
        )
        
        timestamp = (ts_high << 32) | ts_low
        
        return MessageHeader(
            type=MessageType(type_),
            priority=MessagePriority(priority),
            sequence=sequence,
            timestamp=timestamp,
            payload_size=payload_size
        )

@dataclass
class Message:
    """完整消息"""
    header: MessageHeader
    payload: bytes
    
    def pack(self) -> bytes:
        """序列化为字节"""
        return self.header.pack() + self.payload
    
    @staticmethod
    def unpack(data: bytes) -> 'Message':
        """从字节反序列化"""
        header = MessageHeader.unpack(data[:16])
        payload = data[16:16+header.payload_size]
        
        return Message(header=header, payload=payload)
    
    @staticmethod
    def create(
        msg_type: MessageType,
        priority: MessagePriority,
        payload: bytes
    ) -> 'Message':
        """创建消息"""
        sequence = Message._get_next_sequence()
        timestamp = int(time.time() * 1e9)
        
        header = MessageHeader(
            type=msg_type,
            priority=priority,
            sequence=sequence,
            timestamp=timestamp,
            payload_size=len(payload)
        )
        
        return Message(header=header, payload=payload)
    
    _sequence_counter = 0
    
    @staticmethod
    def _get_next_sequence() -> int:
        Message._sequence_counter = (Message._sequence_counter + 1) % 65536
        return Message._sequence_counter
```

### 4.2 模块更新消息

```python
# messaging/module_messages.py

import json
from dataclasses import asdict

class PowertrainUpdateMessage:
    """动力系统更新消息"""
    
    @staticmethod
    def create(state: PowertrainState) -> Message:
        """创建动力系统更新消息"""
        payload = json.dumps(state.to_dict()).encode('utf-8')
        
        return Message.create(
            msg_type=MessageType.POWERTRAIN_UPDATE,
            priority=MessagePriority.HIGH,
            payload=payload
        )
    
    @staticmethod
    def parse(message: Message) -> PowertrainState:
        """解析动力系统更新消息"""
        data = json.loads(message.payload.decode('utf-8'))
        return PowertrainState.from_dict(data)

class VehicleDynamicsUpdateMessage:
    """车辆动力学更新消息"""
    
    @staticmethod
    def create(state: VehicleState) -> Message:
        """创建车辆动力学更新消息"""
        payload = json.dumps(state.to_dict()).encode('utf-8')
        
        return Message.create(
            msg_type=MessageType.VEHICLE_DYNAMICS_UPDATE,
            priority=MessagePriority.HIGH,
            payload=payload
        )
    
    @staticmethod
    def parse(message: Message) -> VehicleState:
        """解析车辆动力学更新消息"""
        data = json.loads(message.payload.decode('utf-8'))
        return VehicleState.from_dict(data)
```

---

## 5. 配置文件格式

### 5.1 YAML配置示例

```yaml
# config/vehicle_config.yaml

# 车辆基本参数
vehicle:
  name: "EV_Sedan_2026"
  version: "1.0"
  
  mass: 2000                      # 整车质量 (kg)
  
  inertia:
    xx: 500                       # 绕X轴转动惯量 (kg·m²)
    yy: 2500                      # 绕Y轴转动惯量 (kg·m²)
    zz: 2800                      # 绕Z轴转动惯量 (kg·m²)
  
  dimensions:
    wheelbase: 2.8                # 轴距 (m)
    track_width_front: 1.6        # 前轮距 (m)
    track_width_rear: 1.6         # 后轮距 (m)
    cg_height: 0.5                # 质心高度 (m)
    cg_to_front_axle: 1.4         # 质心到前轴距离 (m)
    overall_length: 4.8           # 总长 (m)
    overall_width: 1.9            # 总宽 (m)
    overall_height: 1.5           # 总高 (m)
  
  aerodynamics:
    drag_coefficient: 0.28        # 阻力系数
    frontal_area: 2.3             # 迎风面积 (m²)
    lift_coefficient: 0.05        # 升力系数

# 动力系统参数
powertrain:
  motor:
    type: "PMSM"
    max_torque: 300               # 最大扭矩 (N·m)
    max_power: 150000             # 最大功率 (W)
    max_speed: 12000              # 最大转速 (rpm)
    base_speed: 4000              # 基速 (rpm)
    
    electrical:
      nominal_voltage: 400        # 额定电压 (V)
      max_current: 400            # 最大电流 (A)
      stator_resistance: 0.02     # 定子电阻 (Ω)
      inductance_d: 0.0002        # d轴电感 (H)
      inductance_q: 0.0003        # q轴电感 (H)
      flux_linkage: 0.1           # 永磁体磁链 (Wb)
      pole_pairs: 4               # 极对数
    
    mechanical:
      rotor_inertia: 0.05         # 转子惯量 (kg·m²)
      damping_coefficient: 0.001  # 阻尼系数 (N·m·s/rad)
    
    thermal:
      max_temperature: 150        # 最高温度 (°C)
      thermal_resistance: 0.1     # 热阻 (°C/W)
      thermal_capacity: 500       # 热容 (J/°C)
  
  battery:
    type: "Lithium-Ion"
    capacity: 100                 # 容量 (kWh)
    nominal_voltage: 400          # 额定电压 (V)
    num_cells_series: 108         # 串联单体数
    num_cells_parallel: 3         # 并联单体数
    
    performance:
      max_discharge_rate: 3.0     # 最大放电倍率 (C)
      max_charge_rate: 1.0        # 最大充电倍率 (C)
      internal_resistance: 0.05   # 内阻 (Ω)
    
    limits:
      min_voltage: 300            # 最低电压 (V)
      max_voltage: 420            # 最高电压 (V)
      min_temperature: -20        # 最低温度 (°C)
      max_temperature: 60         # 最高温度 (°C)
  
  transmission:
    type: "Single-Speed"
    gear_ratio: 9.73              # 传动比
    efficiency: 0.96              # 效率

# 底盘参数
chassis:
  suspension:
    front:
      type: "Active"
      spring_stiffness: 25000     # 弹簧刚度 (N/m)
      damping_coefficient: 2500   # 阻尼系数 (N·s/m)
      motion_ratio: 0.8           # 运动比
      unsprung_mass: 40           # 簧下质量 (kg)
      max_actuator_force: 5000    # 最大作动器力 (N)
    
    rear:
      type: "Active"
      spring_stiffness: 22000
      damping_coefficient: 2200
      motion_ratio: 0.8
      unsprung_mass: 35
      max_actuator_force: 5000
  
  steering:
    type: "EPS"
    steering_ratio: 16            # 转向传动比
    max_steering_angle: 45        # 最大转向角 (deg)
    max_steering_wheel_angle: 720 # 最大转向盘转角 (deg)
    max_assist_torque: 10         # 最大助力扭矩 (N·m)
    assist_gain_low_speed: 2.5
    assist_gain_high_speed: 1.0
  
  brake:
    type: "EMB"
    max_brake_torque_per_wheel: 10000  # 单轮最大制动力矩 (N·m)
    brake_response_time: 0.05     # 制动响应时间 (s)
    front_rear_ratio: 0.6         # 前后制动力分配比
    ebd_enabled: true
    max_regen_ratio: 0.3          # 最大再生制动比例

# 轮胎参数
tire:
  model: "Pacejka"
  
  dimensions:
    radius: 0.33                  # 轮胎半径 (m)
    width: 0.225                  # 轮胎宽度 (m)
    aspect_ratio: 45              # 扁平比 (%)
  
  pacejka_parameters:
    longitudinal:
      B_x: 10.0
      C_x: 1.65
      D_x: 1.0                    # 相对值
      E_x: 0.97
    
    lateral:
      B_y: 12.0
      C_y: 2.3
      D_y: 1.0
      E_y: -1.0
    
    friction:
      mu_x: 1.0                   # 纵向摩擦系数
      mu_y: 1.0                   # 侧向摩擦系数

# 仿真参数
simulation:
  frequency: 500                  # 仿真频率 (Hz)
  integration_method: "RK4"       # 积分方法
  max_duration: 3600              # 最大时长 (s)
  
  realtime:
    enabled: true
    priority: 90
    cpu_affinity: [2, 3]
    memory_lock: true
  
  logging:
    enabled: true
    level: "INFO"
    frequency: 100                # 日志频率 (Hz)
    format: "MDF"                 # MDF/CSV/HDF5
    output_dir: "/var/log/ev_simulation"

# HIL接口参数
hil:
  can:
    enabled: true
    interface: "can0"
    bitrate: 500000
  
  ethernet:
    enabled: true
    ip_address: "192.168.1.100"
    port: 50000
    protocol: "XCP"
  
  analog_io:
    enabled: false
    device: "Dev1"
    sample_rate: 1000
```

### 5.2 配置加载器

```python
# config/config_loader.py

import yaml
from typing import Dict, Any
from pathlib import Path
import jsonschema

class ConfigLoader:
    """配置加载器"""
    
    def __init__(self, schema_path: str = None):
        """
        初始化配置加载器
        
        Args:
            schema_path: JSON Schema文件路径（用于验证）
        """
        self.schema = None
        if schema_path:
            with open(schema_path, 'r') as f:
                self.schema = yaml.safe_load(f)
    
    def load(self, config_path: str) -> Dict[str, Any]:
        """
        加载YAML配置文件
        
        Args:
            config_path: 配置文件路径
        
        Returns:
            配置字典
        
        Raises:
            FileNotFoundError: 文件不存在
            yaml.YAMLError: YAML格式错误
            jsonschema.ValidationError: 配置验证失败
        """
        path = Path(config_path)
        if not path.exists():
            raise FileNotFoundError(f"Configuration file not found: {config_path}")
        
        with open(path, 'r') as f:
            config = yaml.safe_load(f)
        
        # 验证配置
        if self.schema:
            jsonschema.validate(config, self.schema)
        
        return config
    
    def load_multiple(self, config_files: list) -> Dict[str, Any]:
        """
        加载多个配置文件并合并
        
        Args:
            config_files: 配置文件路径列表
        
        Returns:
            合并后的配置字典
        """
        merged_config = {}
        
        for config_file in config_files:
            config = self.load(config_file)
            merged_config = self._deep_merge(merged_config, config)
        
        return merged_config
    
    @staticmethod
    def _deep_merge(dict1: dict, dict2: dict) -> dict:
        """深度合并两个字典"""
        result = dict1.copy()
        
        for key, value in dict2.items():
            if key in result and isinstance(result[key], dict) and isinstance(value, dict):
                result[key] = ConfigLoader._deep_merge(result[key], value)
            else:
                result[key] = value
        
        return result

class VehicleConfigParser:
    """车辆配置解析器"""
    
    @staticmethod
    def parse(config: dict) -> VehicleConfig:
        """解析车辆配置"""
        vehicle_config = VehicleConfig()
        
        # 基本信息
        vehicle_config.mass = config['vehicle']['mass']
        
        # 转动惯量
        inertia = config['vehicle']['inertia']
        vehicle_config.inertia_xx = inertia['xx']
        vehicle_config.inertia_yy = inertia['yy']
        vehicle_config.inertia_zz = inertia['zz']
        
        # 尺寸
        dims = config['vehicle']['dimensions']
        vehicle_config.wheelbase = dims['wheelbase']
        vehicle_config.track_width_front = dims['track_width_front']
        vehicle_config.track_width_rear = dims['track_width_rear']
        vehicle_config.cg_height = dims['cg_height']
        vehicle_config.cg_to_front_axle = dims['cg_to_front_axle']
        
        # 空气动力学
        aero = config['vehicle']['aerodynamics']
        vehicle_config.drag_coefficient = aero['drag_coefficient']
        vehicle_config.frontal_area = aero['frontal_area']
        vehicle_config.lift_coefficient = aero['lift_coefficient']
        
        return vehicle_config
```

---

## 6. 日志格式

### 6.1 二进制日志格式（MDF）

```python
# logging/mdf_logger.py

import struct
from datetime import datetime
from typing import List, BinaryIO

class MDFLogger:
    """
    MDF (Measurement Data Format) 日志记录器
    
    二进制格式：
    - Header Block (128 bytes)
    - Channel Group Block
    - Data Group Block
    - Data Records
    """
    
    MAGIC_NUMBER = b'MDF3.30'
    
    def __init__(self, filename: str):
        self.filename = filename
        self.file: BinaryIO = None
        self.channels: List[dict] = []
        self.record_count = 0
    
    def open(self):
        """打开MDF文件"""
        self.file = open(self.filename, 'wb')
        self._write_header()
    
    def define_channel(self, name: str, unit: str, data_type: str):
        """定义通道"""
        self.channels.append({
            'name': name,
            'unit': unit,
            'data_type': data_type
        })
    
    def write_record(self, timestamp: float, data: dict):
        """
        写入数据记录
        
        Args:
            timestamp: 时间戳 (s)
            data: 数据字典 {channel_name: value}
        """
        # 时间戳（8字节）
        self.file.write(struct.pack('<d', timestamp))
        
        # 数据值
        for channel in self.channels:
            name = channel['name']
            dtype = channel['data_type']
            value = data.get(name, 0)
            
            if dtype == 'float32':
                self.file.write(struct.pack('<f', value))
            elif dtype == 'float64':
                self.file.write(struct.pack('<d', value))
            elif dtype == 'int32':
                self.file.write(struct.pack('<i', value))
            elif dtype == 'uint8':
                self.file.write(struct.pack('<B', value))
        
        self.record_count += 1
    
    def close(self):
        """关闭MDF文件"""
        self._write_footer()
        self.file.close()
    
    def _write_header(self):
        """写入文件头"""
        # 魔数
        self.file.write(self.MAGIC_NUMBER)
        
        # 版本
        self.file.write(struct.pack('<H', 330))
        
        # 创建时间
        now = datetime.now()
        self.file.write(struct.pack('<H', now.year))
        self.file.write(struct.pack('<H', now.month))
        self.file.write(struct.pack('<H', now.day))
        self.file.write(struct.pack('<H', now.hour))
        self.file.write(struct.pack('<H', now.minute))
        self.file.write(struct.pack('<H', now.second))
        
        # 通道数量
        self.file.write(struct.pack('<I', len(self.channels)))
        
        # 保留空间（填充到128字节）
        self.file.write(b'\x00' * (128 - self.file.tell()))
    
    def _write_footer(self):
        """写入文件尾"""
        # 记录数量
        self.file.write(struct.pack('<Q', self.record_count))
```

### 6.2 CSV日志格式

```python
# logging/csv_logger.py

import csv
from typing import List, Dict

class CSVLogger:
    """CSV日志记录器"""
    
    def __init__(self, filename: str, columns: List[str]):
        """
        初始化CSV日志记录器
        
        Args:
            filename: 文件名
            columns: 列名列表
        """
        self.filename = filename
        self.columns = columns
        self.file = None
        self.writer = None
    
    def open(self):
        """打开CSV文件"""
        self.file = open(self.filename, 'w', newline='')
        self.writer = csv.DictWriter(self.file, fieldnames=self.columns)
        self.writer.writeheader()
    
    def write(self, data: Dict[str, float]):
        """
        写入一行数据
        
        Args:
            data: 数据字典
        """
        self.writer.writerow(data)
    
    def close(self):
        """关闭CSV文件"""
        if self.file:
            self.file.close()
```

### 6.3 结构化日志格式（JSON Lines）

```python
# logging/json_logger.py

import json
from datetime import datetime

class JSONLogger:
    """JSON Lines日志记录器"""
    
    def __init__(self, filename: str):
        self.filename = filename
        self.file = None
    
    def open(self):
        """打开日志文件"""
        self.file = open(self.filename, 'w')
    
    def log(self, level: str, component: str, message: str, **kwargs):
        """
        记录日志
        
        Args:
            level: 日志级别 (DEBUG/INFO/WARNING/ERROR/CRITICAL)
            component: 组件名称
            message: 日志消息
            **kwargs: 额外字段
        """
        entry = {
            'timestamp': datetime.now().isoformat(),
            'level': level,
            'component': component,
            'message': message,
            **kwargs
        }
        
        self.file.write(json.dumps(entry) + '\n')
    
    def close(self):
        """关闭日志文件"""
        if self.file:
            self.file.close()
```

---

## 7. 数据序列化

### 7.1 二进制序列化

```python
# serialization/binary_serializer.py

import struct
from typing import Any

class BinarySerializer:
    """二进制序列化器"""
    
    @staticmethod
    def serialize_vehicle_state(state: VehicleState) -> bytes:
        """
        序列化车辆状态为二进制
        
        格式：
        - timestamp (8 bytes)
        - position (3 * 4 = 12 bytes)
        - euler_angles (3 * 4 = 12 bytes)
        - velocity (3 * 4 = 12 bytes)
        - omega (3 * 4 = 12 bytes)
        - acceleration (3 * 4 = 12 bytes)
        - wheel_speeds (4 * 4 = 16 bytes)
        - wheel_angles (4 * 4 = 16 bytes)
        - wheel_loads (4 * 4 = 16 bytes)
        - gear (1 byte)
        - mode (1 byte)
        总计：~130 bytes
        """
        data = bytearray()
        
        # 时间戳
        data.extend(struct.pack('<d', state.timestamp))
        data.extend(struct.pack('<d', state.simulation_time))
        
        # 位置
        data.extend(struct.pack('<fff', state.position_x, state.position_y, state.position_z))
        
        # 欧拉角
        data.extend(struct.pack('<fff', state.roll, state.pitch, state.yaw))
        
        # 速度
        data.extend(struct.pack('<fff', state.velocity_x, state.velocity_y, state.velocity_z))
        
        # 角速度
        data.extend(struct.pack('<fff', state.omega_x, state.omega_y, state.omega_z))
        
        # 加速度
        data.extend(struct.pack('<fff', state.acceleration_x, state.acceleration_y, state.acceleration_z))
        
        # 车轮状态
        for ws in state.wheel_speeds:
            data.extend(struct.pack('<f', ws))
        for wa in state.wheel_angles:
            data.extend(struct.pack('<f', wa))
        for wl in state.wheel_loads:
            data.extend(struct.pack('<f', wl))
        
        # 档位和模式
        data.extend(struct.pack('<bb', state.gear, state.mode.value))
        
        return bytes(data)
    
    @staticmethod
    def deserialize_vehicle_state(data: bytes) -> VehicleState:
        """从二进制反序列化车辆状态"""
        state = VehicleState()
        offset = 0
        
        # 时间戳
        state.timestamp, state.simulation_time = struct.unpack_from('<dd', data, offset)
        offset += 16
        
        # 位置
        state.position_x, state.position_y, state.position_z = struct.unpack_from('<fff', data, offset)
        offset += 12
        
        # 欧拉角
        state.roll, state.pitch, state.yaw = struct.unpack_from('<fff', data, offset)
        offset += 12
        
        # 速度
        state.velocity_x, state.velocity_y, state.velocity_z = struct.unpack_from('<fff', data, offset)
        offset += 12
        
        # 角速度
        state.omega_x, state.omega_y, state.omega_z = struct.unpack_from('<fff', data, offset)
        offset += 12
        
        # 加速度
        state.acceleration_x, state.acceleration_y, state.acceleration_z = struct.unpack_from('<fff', data, offset)
        offset += 12
        
        # 车轮状态
        for i in range(4):
            state.wheel_speeds[i] = struct.unpack_from('<f', data, offset)[0]
            offset += 4
        for i in range(4):
            state.wheel_angles[i] = struct.unpack_from('<f', data, offset)[0]
            offset += 4
        for i in range(4):
            state.wheel_loads[i] = struct.unpack_from('<f', data, offset)[0]
            offset += 4
        
        # 档位和模式
        state.gear, mode_value = struct.unpack_from('<bb', data, offset)
        state.mode = VehicleMode(mode_value)
        
        return state
```

---

## 8. 内存布局优化

### 8.1 缓存行对齐

```cpp
// memory/aligned_allocator.h
#pragma once

#include <memory>
#include <cstdlib>

namespace ev_simulation {
namespace memory {

/**
 * @brief 缓存行大小（64字节）
 */
constexpr size_t CACHE_LINE_SIZE = 64;

/**
 * @brief 对齐分配器
 */
template<typename T, size_t Alignment = CACHE_LINE_SIZE>
class AlignedAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    
    template<typename U>
    struct rebind {
        using other = AlignedAllocator<U, Alignment>;
    };
    
    AlignedAllocator() = default;
    
    template<typename U>
    AlignedAllocator(const AlignedAllocator<U, Alignment>&) {}
    
    pointer allocate(size_type n) {
        void* ptr = nullptr;
        if (posix_memalign(&ptr, Alignment, n * sizeof(T)) != 0) {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(ptr);
    }
    
    void deallocate(pointer p, size_type) {
        free(p);
    }
    
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        new(p) U(std::forward<Args>(args)...);
    }
    
    template<typename U>
    void destroy(U* p) {
        p->~U();
    }
};

/**
 * @brief 对齐的向量
 */
template<typename T>
using AlignedVector = std::vector<T, AlignedAllocator<T>>;

} // namespace memory
} // namespace ev_simulation
```

### 8.2 内存池

```cpp
// memory/memory_pool.h
#pragma once

#include <cstddef>
#include <vector>
#include <stack>

namespace ev_simulation {
namespace memory {

/**
 * @brief 固定大小内存池
 * 
 * 用于避免实时系统中的动态内存分配
 */
template<typename T, size_t PoolSize>
class MemoryPool {
public:
    MemoryPool() {
        // 预分配内存
        pool_.reserve(PoolSize);
        free_list_.reserve(PoolSize);
        
        for (size_t i = 0; i < PoolSize; i++) {
            pool_.emplace_back();
            free_list_.push_back(&pool_[i]);
        }
    }
    
    /**
     * @brief 分配对象
     */
    T* allocate() {
        if (free_list_.empty()) {
            throw std::bad_alloc();
        }
        
        T* ptr = free_list_.back();
        free_list_.pop_back();
        return ptr;
    }
    
    /**
     * @brief 释放对象
     */
    void deallocate(T* ptr) {
        // 重置对象
        ptr->~T();
        new(ptr) T();
        
        free_list_.push_back(ptr);
    }
    
    /**
     * @brief 获取可用数量
     */
    size_t available() const {
        return free_list_.size();
    }
    
private:
    std::vector<T> pool_;
    std::vector<T*> free_list_;
};

} // namespace memory
} // namespace ev_simulation
```

---

## 附录

### A. 数据结构大小参考

| 数据结构 | 大小（字节） | 对齐 | 说明 |
|---------|------------|------|------|
| VehicleState | ~200 | 64 | 完整车辆状态 |
| MotorState | ~80 | 32 | 电机状态 |
| BatteryState | ~80 | 32 | 电池状态 |
| TireForces | 16 | 16 | 单轮轮胎力 |
| MessageHeader | 16 | 8 | 消息头 |
| Vector3D | 12 | 16 | 三维向量 |
| Quaternion | 16 | 16 | 四元数 |

### B. 变更历史

| 版本 | 日期 | 作者 | 变更内容 |
|-----|------|------|---------|
| 1.0.0 | 2026-03-05 | ArchitectAgent | 初始版本 |

---

**文档结束**

本数据结构定义文档详细描述了六自由度新能源车辆动力学仿真系统中所有核心数据结构，包括Python和C++两种语言的实现、消息格式、配置文件格式、日志格式以及内存优化策略。所有数据结构都经过精心设计，以满足实时性和性能要求。
