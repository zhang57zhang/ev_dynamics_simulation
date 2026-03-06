# Phase 2: 核心模块开发 - Agent任务详细描述

**阶段**：Phase 2  
**工期**：7天（2026-03-08 至 2026-03-15）  
**模式**：5个Agent并行开发  
**目标**：完成5个核心模块的开发和单元测试

---

## 1. BackendAgent #1: 动力系统模块

### 1.1 模块概述
- **模块名称**：PowertrainSystem
- **技术栈**：C++ 17 + pybind11
- **工作目录**：`src/powertrain/`
- **测试目录**：`tests/powertrain/`

### 1.2 子模块任务

#### Day 1-2: 电机模型（PMSM）
**任务描述**：
实现永磁同步电机（PMSM）模型，包括：
- 扭矩-转速特性曲线
- 效率Map图
- 温度影响模型
- 响应延迟模型

**接口定义**：
```cpp
class PMSMMotor {
public:
    void set_torque_request(double torque_nm);
    MotorState get_current_state() const;
    void update(double dt);
private:
    double torque_request_;
    double current_torque_;
    double rotor_speed_;
    double temperature_;
};
```

**验收标准**：
- [ ] 扭矩响应时间<10ms
- [ ] 效率误差<5%
- [ ] 单元测试覆盖率>90%

#### Day 3-4: 电池模型（SOC/SOH）
**任务描述**：
实现电池管理系统模型，包括：
- SOC（荷电状态）估算
- SOH（健康状态）估算
- 开路电压（OCV）模型
- 内阻模型
- 温度影响

**接口定义**：
```cpp
class BatteryPack {
public:
    double get_soc() const;
    double get_soh() const;
    double get_voltage() const;
    void consume_power(double power_kw, double dt);
    void update(double dt);
};
```

**验收标准**：
- [ ] SOC估算误差<3%
- [ ] 电压估算误差<2%
- [ ] 单元测试覆盖率>90%

#### Day 5-6: 变速器模型
**任务描述**：
实现单级减速器模型，包括：
- 传动效率
- 齿轮比
- 扭矩传递
- 能量损失

**接口定义**：
```cpp
class SingleSpeedReducer {
public:
    void set_input_torque(double torque_nm);
    void set_input_speed(double speed_rpm);
    double get_output_torque() const;
    double get_output_speed() const;
    void update(double dt);
};
```

**验收标准**：
- [ ] 扭矩传递误差<2%
- [ ] 效率模型准确
- [ ] 单元测试覆盖率>90%

#### Day 7: 集成测试
**任务描述**：
- 整合电机、电池、变速器
- 编写集成测试
- 性能基准测试

**验收标准**：
- [ ] 集成测试通过
- [ ] 性能满足要求（<500μs/step）
- [ ] 文档完整

### 1.3 技术要求
- 使用Eigen库进行矩阵运算
- 使用pybind11创建Python绑定
- 代码符合Google C++ Style Guide
- 使用Google Test编写测试

### 1.4 输出文件
```
src/powertrain/
├── motor/
│   ├── pmsm_motor.h
│   ├── pmsm_motor.cpp
│   └── pmsm_motor_test.cpp
├── battery/
│   ├── battery_pack.h
│   ├── battery_pack.cpp
│   └── battery_pack_test.cpp
├── transmission/
│   ├── single_speed_reducer.h
│   ├── single_speed_reducer.cpp
│   └── single_speed_reducer_test.cpp
├── powertrain_system.h
├── powertrain_system.cpp
└── powertrain_binding.cpp
```

---

## 2. BackendAgent #2: 底盘系统模块

### 2.1 模块概述
- **模块名称**：ChassisSystem
- **技术栈**：C++ 17 + pybind11
- **工作目录**：`src/chassis/`
- **测试目录**：`tests/chassis/`

### 2.2 子模块任务

#### Day 1-2: 悬架模型（主动/被动）
**任务描述**：
实现悬架系统模型，包括：
- 被动悬架（弹簧+阻尼）
- 主动悬架（可控阻尼+空气弹簧）
- 1/4车辆动力学
- 轮胎动载荷

**接口定义**：
```cpp
class Suspension {
public:
    void set_suspension_travel(double travel_m);
    void set_vertical_velocity(double velocity_m_s);
    void set_mode(SuspensionMode mode);
    SuspensionForce get_force() const;
    void update(double dt);
};
```

**验收标准**：
- [ ] 悬架动力学正确
- [ ] 主动悬架响应<20ms
- [ ] 单元测试覆盖率>90%

#### Day 3-4: 转向模型（EPS/SBW）
**任务描述**：
实现转向系统模型，包括：
- 电动助力转向（EPS）
- 线控转向（SBW）
- 转向比
- 转向力矩反馈

**接口定义**：
```cpp
class SteeringSystem {
public:
    void set_steering_angle(double angle_rad);
    void set_steering_velocity(double velocity_rad_s);
    void set_mode(SteeringMode mode);
    SteeringOutput get_output() const;
    void update(double dt);
};
```

**验收标准**：
- [ ] 转向响应正确
- [ ] 力矩反馈准确
- [ ] 单元测试覆盖率>90%

#### Day 5-6: 制动模型（EMB）
**任务描述**：
实现电子机械制动（EMB）模型，包括：
- 制动力分配
- 制动响应
- ABS功能
- 制动能量回收

**接口定义**：
```cpp
class BrakingSystem {
public:
    void set_brake_pressure(double pressure_bar);
    void set_brake_distribution(double front_ratio);
    BrakingOutput get_output() const;
    void update(double dt);
};
```

**验收标准**：
- [ ] 制动力分配准确
- [ ] 响应时间<50ms
- [ ] 单元测试覆盖率>90%

#### Day 7: 集成测试
**任务描述**：
- 整合悬架、转向、制动
- 编写集成测试
- 性能基准测试

### 2.3 输出文件
```
src/chassis/
├── suspension/
├── steering/
├── braking/
├── chassis_system.h
├── chassis_system.cpp
└── chassis_binding.cpp
```

---

## 3. BackendAgent #3: 车辆动力学模块

### 3.1 模块概述
- **模块名称**：VehicleDynamics
- **技术栈**：C++ 17 + pybind11 + Eigen
- **工作目录**：`src/dynamics/`
- **测试目录**：`tests/dynamics/`

### 3.2 核心任务

#### Day 1-3: 6自由度运动学方程
**任务描述**：
实现6自由度车辆动力学方程，包括：
- 纵向运动（X）
- 横向运动（Y）
- 垂向运动（Z）
- 侧倾运动（φ）
- 俯仰运动（θ）
- 横摆运动（ψ）

**接口定义**：
```cpp
class VehicleDynamics {
public:
    void set_state(const VehicleState& state);
    void set_input(const VehicleInput& input);
    VehicleStateDerivative compute_derivatives();
    void integrate(double dt);
    VehicleState get_state() const;
};
```

**验收标准**：
- [ ] 动力学方程正确
- [ ] 数值稳定性好
- [ ] 单元测试覆盖率>90%

#### Day 4-5: 数值积分（RK4）
**任务描述**：
实现4阶Runge-Kutta积分器，包括：
- RK4积分算法
- 自适应步长（可选）
- 误差控制

**接口定义**：
```cpp
class RK4Integrator {
public:
    void set_state(const VectorXd& state);
    void set_derivatives_func(DerivativesFunc func);
    VectorXd integrate(double dt);
    double get_error_estimate() const;
};
```

**验收标准**：
- [ ] 积分精度满足要求（误差<0.1%）
- [ ] 性能满足要求（<100μs/step）
- [ ] 单元测试覆盖率>90%

#### Day 6: 状态空间实现
**任务描述**：
- 状态空间模型
- 线性化（可选）
- 稳定性分析

#### Day 7: 集成测试
**任务描述**：
- 完整动力学仿真
- 标准工况测试
- 性能基准测试

### 3.3 输出文件
```
src/dynamics/
├── vehicle_dynamics.h
├── vehicle_dynamics.cpp
├── rk4_integrator.h
├── rk4_integrator.cpp
├── state_space.h
├── state_space.cpp
└── dynamics_binding.cpp
```

---

## 4. AIEngineerAgent: 轮胎模型模块

### 4.1 模块概述
- **模块名称**：TireModel
- **技术栈**：C++ 17 + pybind11
- **工作目录**：`src/tire/`
- **测试目录**：`tests/tire/`

### 4.2 核心任务

#### Day 1-3: Pacejka魔术公式
**任务描述**：
实现Pacejka轮胎模型，包括：
- 纵向力（Fx）
- 横向力（Fy）
- 回正力矩（Mz）
- 组合工况

**接口定义**：
```cpp
class PacejkaTire {
public:
    void set_slip_ratio(double kappa);
    void set_slip_angle(double alpha);
    void set_camber_angle(double gamma);
    void set_vertical_load(double fz);
    TireForces get_forces() const;
    void update(double dt);
};
```

**验收标准**：
- [ ] 魔术公式正确
- [ ] 力误差<5%
- [ ] 单元测试覆盖率>90%

#### Day 4-5: 参数拟合
**任务描述**：
- 参数拟合算法
- 测试数据拟合
- 参数验证

#### Day 6: 滑移率计算
**任务描述**：
- 纵向滑移率
- 横向滑移角
- 滑移速度计算

#### Day 7: 集成测试
**任务描述**：
- 完整轮胎模型测试
- 标准工况验证
- 性能基准测试

### 4.3 输出文件
```
src/tire/
├── pacejka_tire.h
├── pacejka_tire.cpp
├── slip_calculator.h
├── slip_calculator.cpp
└── tire_binding.cpp
```

---

## 5. DevOpsAgent: 实时调度模块

### 5.1 模块概述
- **模块名称**：RealtimeScheduler
- **技术栈**：C++ 17 + POSIX RT
- **工作目录**：`src/scheduler/`
- **测试目录**：`tests/scheduler/`

### 5.2 核心任务

#### Day 1-2: 任务调度器
**任务描述**：
实现实时任务调度器，包括：
- 周期性任务调度
- 优先级管理
- 截止时间监控
- 资源管理

**接口定义**：
```cpp
class RealtimeScheduler {
public:
    void add_task(Task task, int priority);
    void remove_task(int task_id);
    void start();
    void stop();
    SchedulerStats get_stats() const;
};
```

**验收标准**：
- [ ] 调度精度<1ms
- [ ] 截止时间满足率>99.9%
- [ ] 单元测试覆盖率>90%

#### Day 3-4: 优先级管理
**任务描述**：
- SCHED_FIFO配置
- CPU亲和性设置
- 优先级继承

#### Day 5-6: 资源管理+监控
**任务描述**：
- 内存管理
- CPU使用率监控
- 延迟统计
- 日志记录

#### Day 7: 性能测试+优化
**任务描述**：
- 压力测试
- 性能优化
- 基准测试

### 5.3 输出文件
```
src/scheduler/
├── realtime_scheduler.h
├── realtime_scheduler.cpp
├── task.h
├── task.cpp
├── monitor.h
├── monitor.cpp
└── scheduler_binding.cpp
```

---

## 6. 协作机制

### 6.1 同步点
- **Day 3**: 第一次集成测试（动力+底盘+轮胎）
- **Day 5**: 第二次集成测试（加入动力学）
- **Day 7**: 最终集成测试（所有模块+调度器）

### 6.2 依赖关系
- 轮胎模型 → 车辆动力学（需要轮胎力）
- 动力系统 → 车辆动力学（需要驱动力）
- 底盘系统 → 车辆动力学（需要悬架力）
- 所有模块 → 实时调度器（需要调度）

### 6.3 接口协议
- 使用`docs/MODULE_INTERFACE_SPEC.md`定义的接口
- 任何接口变更需要同步所有相关Agent
- 使用`shared/sync_status.json`记录同步状态

---

## 7. 验收标准

### 7.1 功能验收
- [ ] 5个模块全部开发完成
- [ ] 所有接口正确实现
- [ ] 单元测试覆盖率>90%
- [ ] 集成测试通过

### 7.2 性能验收
- [ ] 单步计算时间<500μs
- [ ] 实时调度延迟<1ms
- [ ] 内存占用<500MB

### 7.3 质量验收
- [ ] 代码符合规范
- [ ] 注释完整
- [ ] 文档完整

---

**准备就绪，2026-03-08启动Phase 2！**
