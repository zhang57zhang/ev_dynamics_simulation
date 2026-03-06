# 六自由度新能源车辆动力学仿真系统 - 开发路线图

**文档版本**: v1.0  
**创建日期**: 2026-03-06  
**作者**: ArchitectAgent  
**状态**: Phase 1 - 架构设计  

---

## 目录

1. [项目总览](#1-项目总览)
2. [Phase 1: 架构设计](#2-phase-1-架构设计3天2026-03-05至2026-03-08)
3. [Phase 2: 核心模块开发](#3-phase-2-核心模块开发7天2026-03-08至2026-03-15)
4. [Phase 3: 集成测试](#4-phase-3-集成测试3天2026-03-16至2026-03-18)
5. [Phase 4: 文档编写](#5-phase-4-文档编写2天2026-03-19至2026-03-20)
6. [Phase 5: 质量评估](#6-phase-5-质量评估1天2026-03-21)
7. [里程碑与验收标准](#7-里程碑与验收标准)
8. [风险管理](#8-风险管理)
9. [资源分配](#9-资源分配)
10. [成功标准](#10-成功标准)

---

## 1. 项目总览

### 1.1 项目背景

本项目旨在开发一个**六自由度新能源车辆动力学仿真系统**，用于HIL（Hardware-in-the-Loop）测试和验证。系统需要满足以下核心要求：

- **实时性**: 仿真步长≤1ms，总延迟<1ms
- **精度**: 与商业软件（CarSim/AVL）对比误差<5%
- **完整性**: 包含动力系统、底盘系统、轮胎模型、车辆动力学
- **可扩展性**: 支持多种车型和配置

### 1.2 项目时间表

```
总工期：16天（2026-03-05 至 2026-03-21）

Week 1 (03-05 ~ 03-08):
├─ Phase 1: 架构设计 (3天)
│  ├─ Day 1: 系统架构 + 模块接口 ✅
│  ├─ Day 2: 技术选型 + 路线图 🔄
│  └─ Day 3: 原型验证 + 评审

Week 2 (03-08 ~ 03-15):
├─ Phase 2: 核心模块开发 (7天)
│  ├─ BackendAgent #1: 动力系统
│  ├─ BackendAgent #2: 底盘系统
│  ├─ BackendAgent #3: 车辆动力学
│  ├─ AIEngineerAgent: 轮胎模型
│  └─ DevOpsAgent: 实时调度

Week 3 (03-16 ~ 03-18):
├─ Phase 3: 集成测试 (3天)
│  ├─ Day 1: 模块集成
│  ├─ Day 2: 实时性验证
│  └─ Day 3: 性能压力测试

Week 3 (03-19 ~ 03-20):
├─ Phase 4: 文档编写 (2天)
│  ├─ Day 1: API文档 + 用户指南
│  └─ Day 2: 部署文档 + 示例

Week 3 (03-21):
└─ Phase 5: 质量评估 (1天)
   └─ 代码质量 + 性能 + 最终交付
```

### 1.3 开发团队（5个Agent并行）

| Agent | 职责 | 技能栈 | 产出物 |
|-------|------|--------|--------|
| **ArchitectAgent** | 架构设计、技术选型 | 系统设计、性能优化 | 架构文档、技术选型 |
| **BackendAgent #1** | 动力系统模块 | 电机控制、电池建模 | PMSM、电池、变速器模型 |
| **BackendAgent #2** | 底盘系统模块 | 悬架、转向、制动 | 悬架、EPS、EMB模型 |
| **BackendAgent #3** | 车辆动力学模块 | 多体动力学、数值计算 | 6自由度求解器 |
| **AIEngineerAgent** | 轮胎模型模块 | 参数拟合、机器学习 | Pacejka模型 |
| **DevOpsAgent** | 实时调度模块 | 实时系统、性能优化 | 调度器、监控 |
| **TestAgent** | 集成测试 | 自动化测试、性能测试 | 测试报告 |
| **DocAgent** | 文档编写 | 技术写作、示例开发 | 用户手册、API文档 |
| **EvaluatorAgent** | 质量评估 | 代码审查、性能分析 | 质量报告 |

### 1.4 关键约束

| 约束类型 | 描述 | 应对策略 |
|---------|------|---------|
| **时间约束** | 16天总工期，极其紧张 | Agent并行开发 + 敏捷迭代 |
| **技术约束** | 实时性<1ms，难度高 | PREEMPT_RT + C++关键路径 |
| **资源约束** | 团队规模有限 | 清晰分工 + 高效协作 |
| **质量约束** | 覆盖率>90%，精度>95% | 持续测试 + 代码审查 |

---

## 2. Phase 1: 架构设计（3天，2026-03-05至2026-03-08）

### 2.1 Day 1（2026-03-05）：✅ 已完成75%

#### 2.1.1 上午（08:00-12:00）：系统架构设计

**任务清单**:
- [x] 确定系统架构风格（分层架构）
- [x] 定义核心模块（5个模块）
- [x] 设计模块间通信机制
- [x] 绘制系统架构图

**产出物**:
```
docs/ARCHITECTURE.md (121KB)
├─ 1. 系统总览
├─ 2. 架构设计原则
├─ 3. 分层架构
├─ 4. 模块划分
├─ 5. 通信机制
├─ 6. 数据流图
├─ 7. 部署架构
└─ 8. 技术栈
```

**关键决策**:
1. 采用**分层架构**（应用层 → 业务层 → 核心层 → 驱动层）
2. 模块间通信使用**共享内存 + 消息队列**混合方案
3. 实时任务绑定到**隔离CPU**（CPU 2-3）

**验收标准**:
- [x] 架构图清晰完整
- [x] 模块职责明确
- [x] 通信机制合理

---

#### 2.1.2 下午（14:00-18:00）：模块接口规范

**任务清单**:
- [x] 定义各模块输入/输出接口
- [x] 设计数据结构（状态、配置、日志）
- [x] 规划API命名规范
- [x] 编写接口文档

**产出物**:
```
docs/MODULE_INTERFACE_SPEC.md (52KB)
├─ 1. 接口设计原则
├─ 2. 动力系统模块接口
├─ 3. 底盘系统模块接口
├─ 4. 车辆动力学模块接口
├─ 5. 轮胎模型模块接口
├─ 6. 实时调度模块接口
├─ 7. HIL接口
└─ 8. 错误处理规范
```

**关键接口定义**:
```python
# 动力系统接口
class PowertrainInterface:
    def calculate_torque(throttle, speed, temperature) -> float
    def get_motor_state() -> MotorState
    def get_battery_state() -> BatteryState

# 轮胎模型接口
class TireModelInterface:
    def calculate_forces(slip_ratio, slip_angle, load) -> TireForces
    def update_parameters(new_params: dict) -> bool

# 动力学求解器接口
class DynamicsSolverInterface:
    def update(state: VehicleState, dt: float) -> VehicleState
    def reset(initial_state: VehicleState) -> None
```

**验收标准**:
- [x] 所有模块接口定义完整
- [x] 数据结构类型安全
- [x] API命名规范统一

---

#### 2.1.3 晚上（19:00-22:00）：数据结构定义

**任务清单**:
- [x] 定义核心数据结构（车辆状态、控制指令）
- [x] 设计配置文件格式（YAML）
- [x] 规划日志数据格式
- [x] 编写数据结构文档

**产出物**:
```
docs/DATA_STRUCTURES.md (53KB)
├─ 1. 数据结构设计原则
├─ 2. 车辆状态数据结构
├─ 3. 控制指令数据结构
├─ 4. 配置数据结构
├─ 5. 日志数据结构
├─ 6. 通信消息格式
└─ 7. 数据验证规范
```

**核心数据结构**:
```cpp
// 车辆状态（6自由度）
struct VehicleState {
    // 位置和姿态
    Eigen::Vector3d position;      // [x, y, z] m
    Eigen::Vector3d euler_angles;  // [phi, theta, psi] rad
    
    // 速度和角速度
    Eigen::Vector3d velocity;      // [vx, vy, vz] m/s
    Eigen::Vector3d angular_rate;  // [p, q, r] rad/s
    
    // 轮胎状态
    std::array<WheelState, 4> wheels;
    
    // 动力系统状态
    MotorState motor;
    BatteryState battery;
};

// 控制指令
struct ControlCommand {
    double throttle;    // 0.0-1.0
    double brake;       // 0.0-1.0
    double steering;    // -1.0 to 1.0
};
```

**验收标准**:
- [x] 数据结构覆盖所有场景
- [x] 内存布局优化（缓存友好）
- [x] 序列化/反序列化方案

---

### 2.2 Day 2（2026-03-06）：🔄 进行中

#### 2.2.1 上午（08:00-12:00）：技术选型文档

**任务清单**:
- [x] 编程语言选型（Python + C++）
- [x] 实时系统选型（Linux + PREEMPT_RT）
- [x] 通信机制选型（共享内存 + 消息队列）
- [x] 数值计算选型（RK4 + Eigen）
- [x] 物理模型选型（Pacejka）
- [x] HIL接口选型（SocketCAN + UDP）
- [x] 开发工具选型（CMake + pytest）
- [x] 部署方案选型（裸机 + YAML）
- [x] 性能优化选型（OpenMP + SIMD）
- [x] 风险评估

**产出物**:
```
docs/TECHNICAL_DECISIONS.md (65KB)
├─ 1. 编程语言选型
├─ 2. 实时系统选型
├─ 3. 通信机制选型
├─ 4. 数值计算选型
├─ 5. 物理模型选型
├─ 6. HIL接口选型
├─ 7. 开发工具选型
├─ 8. 部署方案选型
├─ 9. 性能优化选型
└─ 10. 风险评估
```

**关键决策**:
1. **Python + C++**: Python负责业务逻辑，C++负责性能关键路径
2. **PREEMPT_RT**: 比Xenomai易于调试，满足<1ms延迟要求
3. **Pacejka**: 工业标准，精度高
4. **RK4**: 精度和性能平衡，工程标准

**验收标准**:
- [x] 所有技术选型有对比分析
- [x] 选型理由充分
- [x] 风险评估完整

---

#### 2.2.2 下午（14:00-18:00）：开发路线图

**任务清单**:
- [x] 制定16天详细计划
- [x] 分配Agent任务
- [x] 定义里程碑和验收标准
- [x] 规划风险管理策略

**产出物**:
```
docs/DEVELOPMENT_ROADMAP.md (本文档)
├─ 1. 项目总览
├─ 2. Phase 1: 架构设计
├─ 3. Phase 2: 核心模块开发
├─ 4. Phase 3: 集成测试
├─ 5. Phase 4: 文档编写
├─ 6. Phase 5: 质量评估
├─ 7. 里程碑与验收标准
├─ 8. 风险管理
├─ 9. 资源分配
└─ 10. 成功标准
```

**验收标准**:
- [x] 每天任务明确
- [x] 里程碑可度量
- [x] 风险应对方案完整

---

#### 2.2.3 晚上（19:00-22:00）：实时性原型验证

**任务清单**:
- [ ] 验证PREEMPT_RT内核配置
- [ ] 测试实时调度器原型
- [ ] 测量基础延迟
- [ ] 验证C++性能基准

**产出物**:
```
prototypes/realtime_benchmark/
├─ CMakeLists.txt
├─ src/
│   ├── rt_scheduler.cpp       # 实时调度器原型
│   ├── latency_test.cpp       # 延迟测试
│   └── performance_benchmark.cpp  # 性能基准
├─ scripts/
│   ├── setup_rt_kernel.sh     # 内核配置脚本
│   └── run_tests.sh           # 测试脚本
└─ results/
    └─ latency_report.txt      # 测试结果
```

**测试内容**:
```bash
# 1. 内核配置验证
uname -v  # 确认PREEMPT_RT
cat /sys/kernel/realtime  # 应为1

# 2. 延迟测试
sudo cyclictest -l100000 -m -Sp90 -i200 -h400 -q
# 预期结果：Max < 200μs

# 3. 性能基准
./performance_benchmark
# 预期结果：
# - RK4单步计算 < 200μs
# - 轮胎力计算 < 150μs
# - 总仿真步长 < 1ms
```

**验收标准**:
- [ ] cyclictest最大延迟 < 200μs
- [ ] C++核心模块单步计算 < 500μs
- [ ] 总仿真步长 < 1ms

---

### 2.3 Day 3（2026-03-07）：Phase 1验收

#### 2.3.1 上午（08:00-12:00）：架构评审

**评审内容**:
1. **架构文档评审**（1.5小时）
   - ARCHITECTURE.md完整性
   - 模块划分合理性
   - 通信机制可行性
   - 部署架构可落地性

2. **接口规范评审**（1.5小时）
   - MODULE_INTERFACE_SPEC.md一致性
   - 接口设计合理性
   - 数据结构完整性
   - API命名规范性

3. **技术选型评审**（1小时）
   - TECHNICAL_DECISIONS.md合理性
   - 技术风险评估
   - 备选方案可行性

**评审清单**:
```markdown
## 架构评审清单

### 1. 架构设计
- [ ] 分层架构是否清晰？
- [ ] 模块职责是否单一？
- [ ] 模块间耦合度是否合理？
- [ ] 扩展性是否足够？

### 2. 接口设计
- [ ] 接口是否完整？
- [ ] 参数类型是否明确？
- [ ] 返回值是否清晰？
- [ ] 错误处理是否规范？

### 3. 数据结构
- [ ] 数据结构是否覆盖所有场景？
- [ ] 内存布局是否优化？
- [ ] 序列化方案是否可行？

### 4. 技术选型
- [ ] 选型理由是否充分？
- [ ] 是否考虑替代方案？
- [ ] 风险评估是否完整？
```

**产出物**:
```
reviews/
└─ phase1_architecture_review.md
   ├─ 评审结论
   ├─ 改进建议
   └─ 待决策事项
```

---

#### 2.3.2 下午（14:00-18:00）：技术原型验证

**验证内容**:
1. **实时性验证**（1小时）
   - cyclictest结果分析
   - 延迟分布统计
   - 最坏情况分析

2. **性能基准验证**（1.5小时）
   - C++核心模块性能测试
   - Python接口性能测试
   - 内存使用分析

3. **集成验证**（1.5小时）
   - Python调用C++模块
   - 共享内存通信测试
   - 消息队列通信测试

**验证清单**:
```bash
# 1. 实时性验证
./scripts/verify_realtime.sh
# 输出：latency_report.txt

# 2. 性能验证
./scripts/verify_performance.sh
# 输出：performance_report.txt

# 3. 集成验证
./scripts/verify_integration.sh
# 输出：integration_report.txt
```

**验收标准**:
- [ ] 实时延迟 < 200μs（99.9%）
- [ ] C++核心模块性能 < 500μs
- [ ] Python-C++集成开销 < 50μs

---

#### 2.3.3 晚上（19:00-22:00）：Phase 2启动准备

**准备内容**:
1. **开发环境搭建**（1小时）
   - 配置Git仓库
   - 设置CI/CD流水线
   - 配置代码规范工具（black, clang-format）

2. **任务分配**（1小时）
   - 创建各Agent任务看板
   - 定义任务依赖关系
   - 设置里程碑提醒

3. **启动会议**（1小时）
   - 向团队介绍架构
   - 讲解技术选型
   - 明确开发规范

**产出物**:
```
.phase2/
├─ task_board.md          # 任务看板
├─ development_guide.md   # 开发指南
└─ coding_standards.md    # 编码规范
```

**Phase 1验收标准**:
- [ ] 所有架构文档完成
- [ ] 技术原型验证通过
- [ ] 实时性达标（<1ms）
- [ ] 团队准备就绪

---

## 3. Phase 2: 核心模块开发（7天，2026-03-08至2026-03-15）

### 3.1 开发策略

**并行开发模式**:
```
5个Agent同时开发，每日同步

BackendAgent #1 ──┐
BackendAgent #2 ──┤
BackendAgent #3 ──┼──> 每日站会（30分钟）
AIEngineerAgent ──┤    ├─ 进度汇报
DevOpsAgent ──────┘    ├─ 问题讨论
                       └─ 协调依赖
```

**每日节奏**:
```
09:00-09:30  每日站会（同步进度）
09:30-12:00  开发时间（编码）
14:00-18:00  开发时间（编码）
19:00-21:00  代码审查 + 测试
21:00-22:00  提交代码 + 更新文档
```

**持续集成**:
```yaml
# .github/workflows/ci.yml
name: Continuous Integration

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential cmake python3 python3-pip
          pip3 install -r requirements.txt
      
      - name: Build C++ modules
        run: |
          mkdir build && cd build
          cmake .. -DCMAKE_BUILD_TYPE=Release
          make -j$(nproc)
      
      - name: Run Python tests
        run: pytest tests/ -v --cov=src
      
      - name: Run C++ tests
        run: cd build && ctest --verbose
      
      - name: Code quality check
        run: |
          black --check src/
          cppcheck --enable=all src/
```

---

### 3.2 BackendAgent #1: 动力系统模块（7天）

#### 3.2.1 Day 1-2（03-08 ~ 03-09）：电机模型（PMSM）

**开发内容**:

**Day 1（03-08）**: PMSM理论基础与模型搭建
```cpp
// src/powertrain/pmsm_motor.hpp
class PMSMMotor {
public:
    PMSMMotor(const MotorConfig& config);
    
    // 计算电磁扭矩
    double calculate_torque(
        double id, double iq,  // d-q轴电流
        double rotor_flux
    );
    
    // 计算d-q轴电流（给定扭矩）
    DQCurrent calculate_current(
        double target_torque,
        double rotor_speed
    );
    
    // 更新电机状态
    MotorState update(
        double throttle,
        double rotor_speed,
        double temperature,
        double dt
    );
    
private:
    MotorConfig config_;
    MotorState state_;
    
    // 内部方法
    double calculate_back_emf(double rotor_speed);
    double calculate_iron_loss(double frequency);
    double calculate_copper_loss(double id, double iq);
};
```

**Day 2（03-09）**: 模型参数与验证
```cpp
// tests/test_pmsm_motor.cpp
TEST(PMSMMotorTest, TorqueCalculation) {
    MotorConfig config{
        .pole_pairs = 4,
        .rated_power = 150000,  // 150kW
        .max_torque = 300,      // 300Nm
        .base_speed = 4000,     // 4000rpm
        .max_speed = 12000      // 12000rpm
    };
    
    PMSMMotor motor(config);
    
    // 测试额定工况
    auto state = motor.update(1.0, 4000, 60, 0.001);
    EXPECT_NEAR(state.torque, 300, 10);  // ±10Nm误差
    
    // 测试高速弱磁
    state = motor.update(1.0, 10000, 60, 0.001);
    EXPECT_LT(state.torque, 300);  // 弱磁降扭
}
```

**产出物**:
```
src/powertrain/
├─ pmsm_motor.hpp
├─ pmsm_motor.cpp
└─ motor_config.hpp

tests/powertrain/
└─ test_pmsm_motor.cpp
```

**验收标准**:
- [ ] 扭矩计算误差 < 5%
- [ ] 单步计算时间 < 100μs
- [ ] 单元测试覆盖率 > 90%

---

#### 3.2.2 Day 3-4（03-10 ~ 03-11）：电池模型（SOC/SOH）

**开发内容**:

**Day 3（03-10）**: 等效电路模型
```cpp
// src/powertrain/battery.hpp
class BatteryModel {
public:
    BatteryModel(const BatteryConfig& config);
    
    // 计算端电压
    double calculate_terminal_voltage(
        double current,
        double temperature
    );
    
    // 更新SOC
    void update_soc(double current, double dt);
    
    // 更新SOH
    void update_soh(double cycle_count, double depth_of_discharge);
    
    // 获取电池状态
    BatteryState get_state() const;
    
private:
    BatteryConfig config_;
    BatteryState state_;
    
    // Thevenin等效电路
    double R_ohm_;      // 欧姆内阻
    double R_polar_;    // 极化电阻
    double C_polar_;    // 极化电容
    
    // 参数辨识
    void identify_parameters(double temperature);
};
```

**Day 4（03-11）**: SOC估计与热管理
```cpp
// src/powertrain/soc_estimator.hpp
class SOCEstimator {
public:
    // 安时积分法
    double coulomb_counting(
        double current,
        double dt,
        double initial_soc
    );
    
    // 扩展卡尔曼滤波
    double ekf_estimate(
        double voltage,
        double current,
        double temperature
    );
    
private:
    Eigen::Vector2d x_;  // [SOC, U_polar]
    Eigen::Matrix2d P_;  // 协方差矩阵
    
    void predict(double current, double dt);
    void update(double voltage);
};

// 热管理模型
class ThermalModel {
public:
    double calculate_temperature(
        double power_loss,
        double ambient_temp,
        double dt
    );
    
private:
    double thermal_mass_;
    double cooling_coefficient_;
};
```

**产出物**:
```
src/powertrain/
├─ battery.hpp
├─ battery.cpp
├─ soc_estimator.hpp
├─ soc_estimator.cpp
└─ thermal_model.hpp

tests/powertrain/
└─ test_battery.cpp
```

**验收标准**:
- [ ] SOC估计误差 < 3%
- [ ] 端电压误差 < 2%
- [ ] 单步计算时间 < 50μs

---

#### 3.2.3 Day 5-6（03-12 ~ 03-13）：变速器模型

**开发内容**:

**Day 5（03-12）**: 单级减速器模型
```cpp
// src/powertrain/gearbox.hpp
class GearboxModel {
public:
    GearboxModel(const GearboxConfig& config);
    
    // 计算输出扭矩
    double calculate_output_torque(double input_torque);
    
    // 计算输出转速
    double calculate_output_speed(double input_speed);
    
    // 计算传动效率
    double calculate_efficiency(
        double input_torque,
        double input_speed,
        int gear
    );
    
    // 更新状态
    GearboxState update(
        double input_torque,
        double input_speed,
        int gear,
        double dt
    );
    
private:
    GearboxConfig config_;
    double gear_ratio_;
    double efficiency_;
    
    // 齿轮损失模型
    double calculate_gear_loss(double torque, double speed);
};
```

**Day 6（03-13）**: 差速器与传动轴
```cpp
// src/powertrain/differential.hpp
class DifferentialModel {
public:
    DifferentialModel(const DifferentialConfig& config);
    
    // 计算左右轮扭矩分配
    std::pair<double, double> distribute_torque(
        double input_torque,
        double left_wheel_speed,
        double right_wheel_speed
    );
    
private:
    double limited_slip_ratio_;  // LSD限滑比
};

// 传动轴模型
class DriveshaftModel {
public:
    double calculate_output_torque(
        double input_torque,
        double angular_velocity
    );
    
private:
    double stiffness_;
    double damping_;
};
```

**产出物**:
```
src/powertrain/
├─ gearbox.hpp
├─ gearbox.cpp
├─ differential.hpp
└─ driveshaft.hpp

tests/powertrain/
└─ test_gearbox.cpp
```

**验收标准**:
- [ ] 传动效率误差 < 2%
- [ ] 扭矩分配误差 < 3%
- [ ] 单步计算时间 < 30μs

---

#### 3.2.4 Day 7（03-14）：单元测试 + 集成

**开发内容**:

**Day 7（03-14）**: 集成测试与性能优化
```cpp
// tests/powertrain/test_powertrain_integration.cpp
TEST(PowertrainIntegrationTest, FullPowerCycle) {
    // 创建动力系统
    PMSMMotor motor(motor_config);
    BatteryModel battery(battery_config);
    GearboxModel gearbox(gearbox_config);
    
    // 模拟全油门加速
    for (int i = 0; i < 10000; ++i) {  // 10秒
        // 电机输出
        auto motor_state = motor.update(1.0, rotor_speed, 60, 0.001);
        
        // 电池放电
        battery.update_soc(motor_state.current, 0.001);
        
        // 变速器输出
        auto gearbox_state = gearbox.update(
            motor_state.torque, 
            rotor_speed, 
            1, 
            0.001
        );
        
        rotor_speed += gearbox_state.output_torque / inertia * 0.001;
    }
    
    // 验证结果
    EXPECT_GT(final_speed, 100 * 1000 / 3600);  // >100km/h
    EXPECT_LT(battery.get_soc(), 0.9);  // SOC下降
}

// 性能基准测试
TEST(PowertrainBenchmark, PerformanceTest) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000000; ++i) {  // 100万次
        powertrain.update(1.0, 4000, 60, 0.001);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double avg_time = duration.count() / 1000000.0;
    EXPECT_LT(avg_time, 100);  // <100μs
}
```

**产出物**:
```
tests/powertrain/
├─ test_powertrain_integration.cpp
└─ performance_report.txt

docs/powertrain/
└─ powertrain_module_guide.md
```

**验收标准**:
- [ ] 集成测试通过
- [ ] 单步计算时间 < 200μs（含电机+电池+变速器）
- [ ] 代码覆盖率 > 90%

---

### 3.3 BackendAgent #2: 底盘系统模块（7天）

#### 3.3.1 Day 1-2（03-08 ~ 03-09）：悬架模型

**开发内容**:

**Day 1（03-08）**: 悬架运动学与动力学
```cpp
// src/chassis/suspension.hpp
class SuspensionModel {
public:
    SuspensionModel(const SuspensionConfig& config);
    
    // 计算悬架力
    double calculate_suspension_force(
        double spring_compression,
        double damper_velocity
    );
    
    // 计算侧倾中心高度
    double calculate_roll_center_height(double roll_angle);
    
    // 计算车轮外倾角变化
    double calculate_camber_change(double suspension_travel);
    
    // 更新状态
    SuspensionState update(
        double vertical_force,
        double lateral_force,
        double dt
    );
    
private:
    SuspensionConfig config_;
    SuspensionState state_;
    
    // 弹簧-阻尼模型
    double spring_stiffness_;
    double damping_coefficient_;
    
    // 非线性特性
    double calculate_progressive_spring(double compression);
    double calculate_nonlinear_damper(double velocity);
};
```

**Day 2（03-09）**: 多连杆悬架与稳定杆
```cpp
// src/chassis/multilink_suspension.hpp
class MultilinkSuspension : public SuspensionModel {
public:
    // 计算硬点位置
    Eigen::Vector3d calculate_hardpoint(
        const std::string& point_name,
        double suspension_travel
    );
    
    // 计算侧倾刚度
    double calculate_roll_stiffness(double roll_angle);
    
private:
    std::map<std::string, Eigen::Vector3d> hardpoints_;
};

// 稳定杆模型
class StabilizerBar {
public:
    double calculate_anti_roll_moment(
        double left_suspension_travel,
        double right_suspension_travel
    );
    
private:
    double stiffness_;
    double length_;
};
```

**产出物**:
```
src/chassis/
├─ suspension.hpp
├─ suspension.cpp
├─ multilink_suspension.hpp
└─ stabilizer_bar.hpp

tests/chassis/
└─ test_suspension.cpp
```

**验收标准**:
- [ ] 悬架力计算误差 < 5%
- [ ] 侧倾刚度误差 < 3%
- [ ] 单步计算时间 < 80μs

---

#### 3.3.2 Day 3-4（03-10 ~ 03-11）：转向模型（EPS/SBW）

**开发内容**:

**Day 3（03-10）**: 电动助力转向（EPS）
```cpp
// src/chassis/eps.hpp
class EPSModel {
public:
    EPSModel(const EPSConfig& config);
    
    // 计算助力扭矩
    double calculate_assist_torque(
        double steering_torque,
        double vehicle_speed
    );
    
    // 计算转向阻力
    double calculate_steering_resistance(
        double steering_angle,
        double lateral_acceleration
    );
    
    // 更新状态
    SteeringState update(
        double steering_input,
        double vehicle_speed,
        double dt
    );
    
private:
    EPSConfig config_;
    SteeringState state_;
    
    // 助力曲线
    std::map<double, double> assist_map_;
    
    // 电机模型
    double calculate_motor_torque(double command, double speed);
};
```

**Day 4（03-11）**: 线控转向（SBW）
```cpp
// src/chassis/sbw.hpp
class SBWModel : public SteeringModel {
public:
    // 计算转向执行器扭矩
    double calculate_actuator_torque(
        double target_angle,
        double current_angle,
        double vehicle_speed
    );
    
    // 故障检测与容错
    bool detect_fault();
    void activate_redundant_channel();
    
private:
    // 双电机冗余
    std::array<MotorModel, 2> motors_;
    int active_channel_;
    
    // 角度控制
    PIDController angle_controller_;
};
```

**产出物**:
```
src/chassis/
├─ eps.hpp
├─ eps.cpp
├─ sbw.hpp
└─ steering_model.hpp

tests/chassis/
└─ test_steering.cpp
```

**验收标准**:
- [ ] 转向角度响应延迟 < 50ms
- [ ] 助力扭矩误差 < 5%
- [ ] 单步计算时间 < 60μs

---

#### 3.3.3 Day 5-6（03-12 ~ 03-13）：制动模型（EMB）

**开发内容**:

**Day 5（03-12）**: 电子机械制动（EMB）
```cpp
// src/chassis/emb.hpp
class EMBModel {
public:
    EMBModel(const EMBConfig& config);
    
    // 计算制动力
    double calculate_brake_force(
        double brake_command,
        double wheel_speed
    );
    
    // 计算夹紧力
    double calculate_clamp_force(double motor_torque);
    
    // 计算制动力矩
    double calculate_brake_torque(
        double clamp_force,
        double brake_pad_friction
    );
    
    // 更新状态
    BrakeState update(
        double brake_command,
        double wheel_speed,
        double dt
    );
    
private:
    EMBConfig config_;
    BrakeState state_;
    
    // 电机模型
    MotorModel motor_;
    
    // 丝杠传动
    double screw_pitch_;
    double screw_efficiency_;
};
```

**Day 6（03-13）**: 制动力分配与ABS
```cpp
// src/chassis/brake_distribution.hpp
class BrakeDistribution {
public:
    // 计算前后制动力分配
    std::pair<double, double> distribute_brake_force(
        double total_brake_command,
        double longitudinal_acceleration
    );
    
private:
    double brake_bias_;  // 制动力分配比
    double ebd_threshold_;  // EBD介入阈值
};

// ABS控制
class ABSController {
public:
    double calculate_brake_pressure(
        double wheel_speed,
        double vehicle_speed,
        double current_pressure
    );
    
private:
    double target_slip_ratio_;
    PIDController slip_controller_;
};
```

**产出物**:
```
src/chassis/
├─ emb.hpp
├─ emb.cpp
├─ brake_distribution.hpp
└─ abs_controller.hpp

tests/chassis/
└─ test_brake.cpp
```

**验收标准**:
- [ ] 制动力误差 < 3%
- [ ] ABS响应时间 < 10ms
- [ ] 单步计算时间 < 70μs

---

#### 3.3.4 Day 7（03-14）：单元测试 + 集成

**开发内容**:

**Day 7（03-14）**: 底盘系统集成测试
```cpp
// tests/chassis/test_chassis_integration.cpp
TEST(ChassisIntegrationTest, FullManeuver) {
    // 创建底盘系统
    SuspensionModel fl_suspension(config);
    SuspensionModel fr_suspension(config);
    EPSModel steering(eps_config);
    EMBModel brakes[4] = {EMBModel(config), ...};
    
    // 模拟双移线工况
    for (int i = 0; i < 10000; ++i) {
        double t = i * 0.001;
        
        // 转向输入
        double steering_angle = generate_double_lane_change(t);
        auto steering_state = steering.update(steering_angle, speed, 0.001);
        
        // 悬架响应
        auto fl_state = fl_suspension.update(vertical_force, lateral_force, 0.001);
        auto fr_state = fr_suspension.update(vertical_force, lateral_force, 0.001);
        
        // 制动（如需要）
        auto brake_state = brakes[0].update(brake_command, wheel_speed, 0.001);
        
        // 更新车辆状态
        ...
    }
    
    // 验证结果
    EXPECT_LT(max_roll_angle, 5 * M_PI / 180);  // 侧倾角 < 5度
    EXPECT_LT(max_yaw_rate, 30 * M_PI / 180);   // 横摆角速度 < 30deg/s
}

// 性能基准
TEST(ChassisBenchmark, PerformanceTest) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000000; ++i) {
        chassis_system.update(forces, dt);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double avg_time = duration.count() / 1000000.0;
    EXPECT_LT(avg_time, 200);  // <200μs
}
```

**产出物**:
```
tests/chassis/
├─ test_chassis_integration.cpp
└─ performance_report.txt

docs/chassis/
└─ chassis_module_guide.md
```

**验收标准**:
- [ ] 集成测试通过
- [ ] 单步计算时间 < 200μs（含悬架+转向+制动）
- [ ] 代码覆盖率 > 90%

---

### 3.4 BackendAgent #3: 车辆动力学模块（7天）

#### 3.4.1 Day 1-3（03-08 ~ 03-10）：6自由度运动学方程

**开发内容**:

**Day 1（03-08）**: 坐标系定义与变换
```cpp
// src/dynamics/coordinate_systems.hpp
class CoordinateSystems {
public:
    // 坐标系定义
    enum class Frame {
        INERTIAL,      // 惯性系（大地）
        BODY,          // 车身系
        WHEEL_HUB,     // 轮毂系
        TIRE_CONTACT   // 轮胎接地点系
    };
    
    // 旋转矩阵
    static Eigen::Matrix3d rotation_matrix(
        const Eigen::Vector3d& euler_angles
    );
    
    // 四元数转换
    static Eigen::Quaterniond euler_to_quaternion(
        const Eigen::Vector3d& euler_angles
    );
    
    // 坐标变换
    static Eigen::Vector3d transform(
        const Eigen::Vector3d& point,
        const Eigen::Matrix3d& rotation,
        const Eigen::Vector3d& translation
    );
    
private:
    // 欧拉角顺序：Z-Y-X（偏航-俯仰-侧倾）
    static constexpr int EULER_ORDER_ZYX = 0;
};
```

**Day 2（03-09）**: 牛顿-欧拉方程
```cpp
// src/dynamics/newton_euler.hpp
class NewtonEulerSolver {
public:
    NewtonEulerSolver(double mass, const Eigen::Matrix3d& inertia);
    
    // 计算加速度（牛顿第二定律）
    Eigen::Vector3d calculate_acceleration(
        const Eigen::Vector3d& total_force
    );
    
    // 计算角加速度（欧拉方程）
    Eigen::Vector3d calculate_angular_acceleration(
        const Eigen::Vector3d& total_torque,
        const Eigen::Vector3d& angular_velocity
    );
    
    // 更新状态
    VehicleState update(
        const VehicleState& current_state,
        const AppliedForces& forces,
        double dt
    );
    
private:
    double mass_;
    Eigen::Matrix3d inertia_;
    Eigen::Matrix3d inertia_inverse_;
    
    // 欧拉方程：I * ω_dot + ω × (I * ω) = τ
    Eigen::Vector3d solve_euler_equation(
        const Eigen::Vector3d& torque,
        const Eigen::Vector3d& omega
    );
};
```

**Day 3（03-10）**: 6自由度运动学
```cpp
// src/dynamics/kinematics_6dof.hpp
class Kinematics6DOF {
public:
    Kinematics6DOF(const VehicleConfig& config);
    
    // 计算轮胎接地点位置
    std::array<Eigen::Vector3d, 4> calculate_contact_points(
        const VehicleState& state
    );
    
    // 计算轮胎速度（含旋转）
    std::array<Eigen::Vector3d, 4> calculate_wheel_velocities(
        const VehicleState& state
    );
    
    // 计算滑移角和滑移率
    SlipState calculate_slip(
        const VehicleState& state,
        const std::array<double, 4>& wheel_speeds
    );
    
    // 计算空气动力
    AerodynamicForces calculate_aerodynamic_forces(
        const Eigen::Vector3d& velocity
    );
    
private:
    VehicleConfig config_;
    
    // 轮胎位置
    std::array<Eigen::Vector3d, 4> wheel_positions_;
};
```

**产出物**:
```
src/dynamics/
├─ coordinate_systems.hpp
├─ newton_euler.hpp
├─ newton_euler.cpp
└─ kinematics_6dof.hpp

tests/dynamics/
└─ test_kinematics.cpp
```

**验收标准**:
- [ ] 坐标变换精度 < 1e-6
- [ ] 运动学计算误差 < 1%
- [ ] 单步计算时间 < 100μs

---

#### 3.4.2 Day 4-5（03-11 ~ 03-12）：数值积分（RK4）

**开发内容**:

**Day 4（03-11）**: RK4积分器
```cpp
// src/dynamics/rk4_integrator.hpp
class RK4Integrator {
public:
    using StateVector = Eigen::Matrix<double, 12, 1>;
    using DynamicsFunction = std::function<StateVector(double, const StateVector&)>;
    
    // 单步RK4积分
    StateVector step(
        const DynamicsFunction& dynamics,
        double t,
        const StateVector& state,
        double dt
    ) {
        StateVector k1 = dynamics(t, state);
        StateVector k2 = dynamics(t + dt/2, state + dt/2 * k1);
        StateVector k3 = dynamics(t + dt/2, state + dt/2 * k2);
        StateVector k4 = dynamics(t + dt, state + dt * k3);
        
        return state + dt/6 * (k1 + 2*k2 + 2*k3 + k4);
    }
    
    // 自适应步长
    StateVector adaptive_step(
        const DynamicsFunction& dynamics,
        double t,
        const StateVector& state,
        double dt_initial,
        double tolerance = 1e-6
    );
    
private:
    // 误差估计
    double estimate_error(
        const StateVector& state_full,
        const StateVector& state_half
    );
};
```

**Day 5（03-12）**: 状态空间实现
```cpp
// src/dynamics/state_space.hpp
class StateSpaceModel {
public:
    StateSpaceModel(const VehicleConfig& config);
    
    // 定义状态向量：[x, y, z, phi, theta, psi, vx, vy, vz, p, q, r]
    using StateVector = Eigen::Matrix<double, 12, 1>;
    
    // 状态方程：dx/dt = f(x, u)
    StateVector state_derivative(
        const StateVector& state,
        const ControlInput& input
    );
    
    // 输出方程：y = h(x, u)
    OutputVector output(
        const StateVector& state,
        const ControlInput& input
    );
    
    // 离散化（用于实时仿真）
    StateVector discretize(
        const StateVector& state,
        const ControlInput& input,
        double dt
    );
    
private:
    VehicleConfig config_;
    
    // 子系统模型
    std::unique_ptr<TireModel> tire_model_;
    std::unique_ptr<AerodynamicModel> aero_model_;
    std::unique_ptr<PowertrainModel> powertrain_;
    
    // 组合动力学方程
    StateVector assemble_equations_of_motion(
        const StateVector& state,
        const AppliedForces& forces
    );
};
```

**产出物**:
```
src/dynamics/
├─ rk4_integrator.hpp
├─ rk4_integrator.cpp
├─ state_space.hpp
└─ state_space.cpp

tests/dynamics/
└─ test_integrator.cpp
```

**验收标准**:
- [ ] RK4积分误差 < 1e-6（与MATLAB对比）
- [ ] 自适应步长收敛
- [ ] 单步计算时间 < 200μs

---

#### 3.4.3 Day 6（03-13）：状态空间实现

**开发内容**:

**Day 6（03-13）**: 完整动力学系统
```cpp
// src/dynamics/vehicle_dynamics.hpp
class VehicleDynamics {
public:
    VehicleDynamics(const VehicleConfig& config);
    
    // 主更新函数
    VehicleState update(
        const VehicleState& current_state,
        const ControlCommand& command,
        double dt
    );
    
    // 重置到初始状态
    void reset(const VehicleState& initial_state);
    
    // 获取当前状态
    VehicleState get_state() const;
    
private:
    VehicleConfig config_;
    VehicleState state_;
    
    // 子系统
    std::unique_ptr<Kinematics6DOF> kinematics_;
    std::unique_ptr<NewtonEulerSolver> solver_;
    std::unique_ptr<RK4Integrator> integrator_;
    std::unique_ptr<TireModel> tires_[4];
    
    // 计算流程
    AppliedForces calculate_all_forces(
        const VehicleState& state,
        const ControlCommand& command
    );
    
    StateVector dynamics_function(
        double t,
        const StateVector& state
    );
};
```

**产出物**:
```
src/dynamics/
├─ vehicle_dynamics.hpp
└─ vehicle_dynamics.cpp

tests/dynamics/
└─ test_vehicle_dynamics.cpp
```

**验收标准**:
- [ ] 动力学响应正确
- [ ] 与CarSim对比误差 < 5%
- [ ] 单步计算时间 < 300μs

---

#### 3.4.4 Day 7（03-14）：单元测试 + 集成

**开发内容**:

**Day 7（03-14）**: 集成测试与验证
```cpp
// tests/dynamics/test_dynamics_integration.cpp
TEST(DynamicsIntegrationTest, StraightLineAcceleration) {
    VehicleDynamics dynamics(config);
    VehicleState state;
    state.velocity = Eigen::Vector3d::Zero();
    
    // 全油门加速10秒
    ControlCommand cmd = {1.0, 0.0, 0.0};
    
    for (int i = 0; i < 10000; ++i) {
        state = dynamics.update(state, cmd, 0.001);
    }
    
    // 验证：0-100km/h加速时间约6-8秒
    double final_speed = state.velocity.x() * 3.6;  // m/s -> km/h
    EXPECT_GT(final_speed, 100);
}

TEST(DynamicsIntegrationTest, DoubleLaneChange) {
    VehicleDynamics dynamics(config);
    VehicleState state;
    state.velocity = Eigen::Vector3d(30, 0, 0);  // 30m/s = 108km/h
    
    // 双移线工况
    for (int i = 0; i < 10000; ++i) {
        double t = i * 0.001;
        double steering = generate_dlc_steering(t);
        
        ControlCommand cmd = {0.5, 0.0, steering};
        state = dynamics.update(state, cmd, 0.001);
    }
    
    // 验证：侧向加速度 < 0.8g
    double max_lat_accel = ...;  // 从日志提取
    EXPECT_LT(max_lat_accel, 0.8 * 9.81);
}

// 性能基准
TEST(DynamicsBenchmark, PerformanceTest) {
    VehicleDynamics dynamics(config);
    VehicleState state;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000000; ++i) {
        state = dynamics.update(state, cmd, 0.001);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double avg_time = duration.count() / 1000000.0;
    EXPECT_LT(avg_time, 500);  // <500μs
}
```

**产出物**:
```
tests/dynamics/
├─ test_dynamics_integration.cpp
├─ car_sim_validation/     # CarSim对比数据
└─ performance_report.txt

docs/dynamics/
└─ dynamics_module_guide.md
```

**验收标准**:
- [ ] 集成测试通过
- [ ] 与CarSim对比误差 < 5%
- [ ] 单步计算时间 < 500μs
- [ ] 代码覆盖率 > 90%

---

### 3.5 AIEngineerAgent: 轮胎模型模块（7天）

#### 3.5.1 Day 1-3（03-08 ~ 03-10）：Pacejka魔术公式实现

**开发内容**:

**Day 1（03-08）**: 魔术公式基础实现
```cpp
// src/tire/pacejka.hpp
class PacejkaTireModel {
public:
    PacejkaTireModel(const PacejkaCoefficients& coef);
    
    // 纯纵向滑移（制动力/驱动力）
    double calculate_longitudinal_force(double kappa);
    
    // 纯侧偏（侧向力）
    double calculate_lateral_force(double alpha);
    
    // 回正力矩
    double calculate_aligning_moment(double alpha);
    
    // 组合滑移
    TireForces calculate_combined_forces(
        double kappa,
        double alpha,
        double Fz
    );
    
private:
    PacejkaCoefficients coef_;
    
    // 魔术公式核心
    double magic_formula(
        double x,
        double B, double C, double D, double E
    );
    
    // 载荷依赖
    double calculate_load_dependent_coefficient(
        double Fz,
        const std::string& coef_name
    );
};
```

**Day 2（03-09）**: 组合滑移与摩擦圆
```cpp
// src/tire/combined_slip.hpp
class CombinedSlipModel {
public:
    CombinedSlipModel(PacejkaTireModel* pure_slip_model);
    
    // 计算组合滑移力
    TireForces calculate_forces(
        double kappa,
        double alpha,
        double Fz,
        double gamma = 0.0  // 外倾角
    );
    
private:
    PacejkaTireModel* pure_model_;
    
    // 摩擦椭圆方法
    TireForces friction_ellipse_method(
        double Fx0,
        double Fy0,
        double Fz
    );
    
    // 联合滑移方法
    TireForces combined_slip_method(
        double kappa,
        double alpha,
        double Fz
    );
};
```

**Day 3（03-10）**: 轮胎热模型
```cpp
// src/tire/tire_thermal.hpp
class TireThermalModel {
public:
    TireThermalModel(const ThermalConfig& config);
    
    // 计算轮胎温度
    double calculate_temperature(
        double work_done,
        double ambient_temp,
        double dt
    );
    
    // 温度对摩擦系数的影响
    double get_friction_multiplier(double temperature);
    
private:
    ThermalConfig config_;
    double current_temp_;
    
    // 热动力学方程
    double heat_generation(double slip, double force, double velocity);
    double heat_dissipation(double temp, double ambient_temp);
};
```

**产出物**:
```
src/tire/
├─ pacejka.hpp
├─ pacejka.cpp
├─ combined_slip.hpp
├─ combined_slip.cpp
└─ tire_thermal.hpp

tests/tire/
└─ test_pacejka.cpp
```

**验收标准**:
- [ ] 纯滑移力计算误差 < 2%
- [ ] 组合滑移力计算误差 < 5%
- [ ] 单步计算时间 < 150μs

---

#### 3.5.2 Day 4-5（03-11 ~ 03-12）：参数拟合算法

**开发内容**:

**Day 4（03-11）**: 非线性最小二乘拟合
```cpp
// src/tire/parameter_fitting.hpp
class TireParameterFitting {
public:
    // 从试验数据拟合参数
    PacejkaCoefficients fit_coefficients(
        const std::vector<TireTestData>& test_data
    );
    
private:
    // Levenberg-Marquardt算法
    Eigen::VectorXd levenberg_marquardt(
        const Eigen::VectorXd& initial_params,
        const std::function<double(const Eigen::VectorXd&)>& residual
    );
    
    // 雅可比矩阵计算
    Eigen::MatrixXd calculate_jacobian(
        const Eigen::VectorXd& params,
        double epsilon = 1e-6
    );
};
```

**Day 5（03-12）**: 数据处理与验证
```python
# tools/tire_data_processor.py
import numpy as np
from scipy.optimize import curve_fit

class TireDataProcessor:
    def load_test_data(self, file_path):
        """加载轮胎试验台数据"""
        # 解析CSV/Excel文件
        # 数据清洗
        # 异常值剔除
        pass
    
    def fit_pacejka_coefficients(self, alpha_data, Fy_data):
        """拟合Pacejka系数"""
        def magic_formula(alpha, B, C, D, E):
            argument = B * alpha
            term = np.arctan(argument)
            return D * np.sin(C * np.arctan(argument - E * (argument - term)))
        
        popt, pcov = curve_fit(
            magic_formula, 
            alpha_data, 
            Fy_data,
            p0=[10, 1.5, 5000, 0.5]
        )
        
        return popt, pcov
    
    def validate_coefficients(self, coef, test_data):
        """验证拟合参数"""
        # 计算拟合误差
        # 生成对比图
        pass
```

**产出物**:
```
src/tire/
├─ parameter_fitting.hpp
└─ parameter_fitting.cpp

tools/
├─ tire_data_processor.py
└─ tire_validation.ipynb

data/tire/
└─ sample_test_data.csv
```

**验收标准**:
- [ ] 拟合误差 < 3%
- [ ] 参数物理意义合理
- [ ] 工具可用

---

#### 3.5.3 Day 6（03-13）：滑移率计算

**开发内容**:

**Day 6（03-13）**: 滑移率与滑移角
```cpp
// src/tire/slip_calculation.hpp
class SlipCalculator {
public:
    // 计算滑移率
    double calculate_slip_ratio(
        double wheel_speed,
        double vehicle_speed
    );
    
    // 计算滑移角
    double calculate_slip_angle(
        const Eigen::Vector3d& wheel_velocity,
        double wheel_angle
    );
    
    // 计算所有轮胎的滑移状态
    std::array<SlipState, 4> calculate_all_slips(
        const VehicleState& vehicle_state,
        const std::array<double, 4>& wheel_speeds,
        const std::array<double, 4>& wheel_angles
    );
    
private:
    // 滤波（去除噪声）
    double low_pass_filter(double raw_value, double prev_value, double alpha);
    
    // 速度阈值（避免零速除法）
    static constexpr double SPEED_THRESHOLD = 0.1;  // m/s
};
```

**产出物**:
```
src/tire/
├─ slip_calculation.hpp
└─ slip_calculation.cpp

tests/tire/
└─ test_slip_calculation.cpp
```

**验收标准**:
- [ ] 滑移率计算准确
- [ ] 处理边界情况（零速）
- [ ] 单步计算时间 < 30μs

---

#### 3.5.4 Day 7（03-14）：单元测试 + 集成

**开发内容**:

**Day 7（03-14）**: 轮胎模型集成测试
```cpp
// tests/tire/test_tire_integration.cpp
TEST(TireIntegrationTest, FullTraction) {
    PacejkaTireModel tire(coefficients);
    SlipCalculator slip_calc;
    
    // 模拟全油门工况
    for (int i = 0; i < 10000; ++i) {
        double wheel_speed = ...;
        double vehicle_speed = ...;
        
        double kappa = slip_calc.calculate_slip_ratio(wheel_speed, vehicle_speed);
        double Fx = tire.calculate_longitudinal_force(kappa);
        
        // 更新车轮转速
        wheel_speed += (Fx - resistance) / wheel_inertia * 0.001;
    }
    
    // 验证：滑移率稳定在0.1-0.15之间
    EXPECT_NEAR(final_slip_ratio, 0.12, 0.05);
}

// 与Tire Test Data对比
TEST(TireValidationTest, CompareWithTestData) {
    PacejkaTireModel tire(fitted_coefficients);
    
    // 加载试验数据
    auto test_data = load_test_data("tire_test_data.csv");
    
    // 对比计算值与试验值
    for (const auto& point : test_data) {
        double calculated = tire.calculate_lateral_force(point.alpha);
        double error = abs(calculated - point.Fy) / point.Fy;
        
        EXPECT_LT(error, 0.05);  // <5%误差
    }
}
```

**产出物**:
```
tests/tire/
├─ test_tire_integration.cpp
└─ validation_report.txt

docs/tire/
└─ tire_module_guide.md
```

**验收标准**:
- [ ] 与试验数据对比误差 < 5%
- [ ] 单步计算时间 < 150μs
- [ ] 代码覆盖率 > 90%

---

### 3.6 DevOpsAgent: 实时调度模块（7天）

#### 3.6.1 Day 1-2（03-08 ~ 03-09）：任务调度器实现

**开发内容**:

**Day 1（03-08）**: 实时任务调度器
```cpp
// src/realtime/rt_scheduler.hpp
class RealtimeScheduler {
public:
    RealtimeScheduler();
    ~RealtimeScheduler();
    
    // 添加周期任务
    void add_periodic_task(
        const std::string& name,
        std::function<void()> task,
        int priority,
        std::chrono::microseconds period
    );
    
    // 启动调度器
    void start();
    
    // 停止调度器
    void stop();
    
    // 获取统计信息
    SchedulerStats get_stats() const;
    
private:
    struct Task {
        std::string name;
        std::function<void()> function;
        int priority;
        std::chrono::microseconds period;
        std::chrono::microseconds execution_time;
        pthread_t thread;
        bool running;
    };
    
    std::vector<Task> tasks_;
    bool running_;
    
    // 任务线程函数
    static void* task_thread_func(void* arg);
    
    // 设置实时属性
    void configure_realtime_thread(pthread_t thread, int priority, int cpu);
};
```

**Day 2（03-09）**: 优先级管理与资源管理
```cpp
// src/realtime/priority_manager.hpp
class PriorityManager {
public:
    // 优先级分配策略
    int assign_priority(const std::string& task_name);
    
    // 优先级继承（避免优先级反转）
    void set_priority_inheritance(pthread_mutex_t* mutex);
    
    // 优先级天花板
    void set_priority_ceiling(
        pthread_mutex_t* mutex,
        int ceiling_priority
    );
    
private:
    std::map<std::string, int> priority_map_;
    
    // 默认优先级分配
    static const std::map<std::string, int> DEFAULT_PRIORITIES;
};

// src/realtime/resource_manager.hpp
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();
    
    // 锁定内存
    void lock_memory();
    
    // 设置CPU亲和性
    void set_cpu_affinity(int cpu);
    
    // 预分配栈空间
    void preallocate_stack(size_t size);
    
    // 禁用交换
    void disable_swapping();
    
private:
    bool memory_locked_;
    std::vector<int> isolated_cpus_;
};
```

**产出物**:
```
src/realtime/
├─ rt_scheduler.hpp
├─ rt_scheduler.cpp
├─ priority_manager.hpp
└─ resource_manager.hpp

tests/realtime/
└─ test_scheduler.cpp
```

**验收标准**:
- [ ] 任务调度抖动 < 10μs
- [ ] 优先级正确执行
- [ ] 单元测试通过

---

#### 3.6.2 Day 3-4（03-10 ~ 03-11）：监控与性能分析

**开发内容**:

**Day 3（03-10）**: 实时监控
```cpp
// src/realtime/monitor.hpp
class RealtimeMonitor {
public:
    RealtimeMonitor();
    
    // 记录任务执行时间
    void record_execution_time(
        const std::string& task_name,
        std::chrono::microseconds duration
    );
    
    // 记录延迟
    void record_latency(
        const std::string& task_name,
        std::chrono::microseconds latency
    );
    
    // 生成报告
    MonitorReport generate_report();
    
    // 导出数据
    void export_to_csv(const std::string& filename);
    
private:
    struct TaskStats {
        std::chrono::microseconds min_time;
        std::chrono::microseconds max_time;
        std::chrono::microseconds avg_time;
        std::chrono::microseconds jitter;
        int deadline_misses;
    };
    
    std::map<std::string, TaskStats> stats_;
    std::mutex mutex_;
};

// src/realtime/latency_tracer.hpp
class LatencyTracer {
public:
    // 开始追踪
    void start_trace(const std::string& event_name);
    
    // 结束追踪
    void end_trace(const std::string& event_name);
    
    // 获取追踪数据
    std::vector<LatencyEvent> get_trace_data();
    
private:
    std::map<std::string, std::chrono::high_resolution_clock::time_point> active_events_;
};
```

**Day 4（03-11）**: 性能分析与可视化
```python
# tools/rt_performance_analyzer.py
import pandas as pd
import matplotlib.pyplot as plt

class RTPerformanceAnalyzer:
    def __init__(self, monitor_data_file):
        self.data = pd.read_csv(monitor_data_file)
    
    def plot_latency_distribution(self):
        """绘制延迟分布图"""
        plt.figure(figsize=(10, 6))
        plt.hist(self.data['latency_us'], bins=100, alpha=0.7)
        plt.xlabel('Latency (μs)')
        plt.ylabel('Frequency')
        plt.title('Latency Distribution')
        plt.savefig('latency_distribution.png')
    
    def calculate_deadline_miss_rate(self, deadline_us):
        """计算截止时间错过率"""
        misses = (self.data['latency_us'] > deadline_us).sum()
        total = len(self.data)
        return misses / total
    
    def generate_report(self):
        """生成性能报告"""
        report = {
            'avg_latency': self.data['latency_us'].mean(),
            'max_latency': self.data['latency_us'].max(),
            'jitter': self.data['latency_us'].std(),
            'deadline_miss_rate': self.calculate_deadline_miss_rate(1000)
        }
        return report
```

**产出物**:
```
src/realtime/
├─ monitor.hpp
├─ monitor.cpp
└─ latency_tracer.hpp

tools/
└─ rt_performance_analyzer.py

scripts/
└─ visualize_latency.py
```

**验收标准**:
- [ ] 监控开销 < 5μs
- [ ] 可视化工具可用
- [ ] 报告生成正确

---

#### 3.6.3 Day 5-6（03-12 ~ 03-13）：资源管理 + 监控

**开发内容**:

**Day 5（03-12）**: 内存管理与错误处理
```cpp
// src/realtime/memory_pool.hpp
template<typename T, size_t PoolSize>
class RealtimeMemoryPool {
public:
    RealtimeMemoryPool() {
        // 预分配并锁定内存
        pool_ = static_cast<T*>(mmap(
            nullptr,
            PoolSize * sizeof(T),
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_LOCKED,
            -1, 0
        ));
        
        // 初始化空闲链表
        for (size_t i = 0; i < PoolSize - 1; ++i) {
            pool_[i].next_free = &pool_[i + 1];
        }
        free_list_ = &pool_[0];
    }
    
    T* allocate() {
        if (free_list_ == nullptr) {
            throw std::bad_alloc();
        }
        
        T* obj = free_list_;
        free_list_ = free_list_->next_free;
        return new(obj) T();
    }
    
    void deallocate(T* obj) {
        obj->~T();
        obj->next_free = free_list_;
        free_list_ = obj;
    }
    
private:
    struct Block {
        union {
            T data;
            Block* next_free;
        };
    };
    
    Block* pool_;
    Block* free_list_;
};

// src/realtime/error_handler.hpp
class RealtimeErrorHandler {
public:
    // 注册错误处理器
    void register_handler(
        const std::string& error_type,
        std::function<void(const std::string&)> handler
    );
    
    // 处理错误
    void handle_error(const std::string& error_type, const std::string& message);
    
    // 截止时间错过处理
    void on_deadline_miss(const std::string& task_name);
    
private:
    std::map<std::string, std::function<void(const std::string&)>> handlers_;
};
```

**Day 6（03-13）**: 集成与系统测试
```cpp
// tests/realtime/test_rt_integration.cpp
TEST(RTIntegrationTest, FullSystemTest) {
    // 创建调度器
    RealtimeScheduler scheduler;
    
    // 添加任务
    scheduler.add_periodic_task(
        "dynamics",
        []() { dynamics.update(cmd, 0.001); },
        95,  // 优先级
        std::chrono::microseconds(1000)  // 1ms周期
    );
    
    scheduler.add_periodic_task(
        "hil_comm",
        []() { hil.send_data(); },
        99,  // 最高优先级
        std::chrono::microseconds(1000)
    );
    
    // 启动调度器
    scheduler.start();
    
    // 运行10秒
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // 停止
    scheduler.stop();
    
    // 验证统计
    auto stats = scheduler.get_stats();
    EXPECT_LT(stats.max_jitter.count(), 50);  // <50μs抖动
    EXPECT_EQ(stats.deadline_misses, 0);  // 无截止时间错过
}
```

**产出物**:
```
src/realtime/
├─ memory_pool.hpp
├─ error_handler.hpp
└─ rt_integration.hpp

tests/realtime/
└─ test_rt_integration.cpp
```

**验收标准**:
- [ ] 内存池分配时间 < 100ns
- [ ] 错误处理正确
- [ ] 集成测试通过

---

#### 3.6.4 Day 7（03-14）：性能测试 + 优化

**开发内容**:

**Day 7（03-14）**: 性能基准测试与优化
```bash
# scripts/run_rt_benchmark.sh
#!/bin/bash

# 1. cyclictest
sudo cyclictest -l100000 -m -Sp90 -i200 -h400 -q > results/cyclictest.txt

# 2. 自定义基准
./build/tests/realtime_benchmark > results/benchmark.txt

# 3. 压力测试
stress --cpu 4 --io 2 --vm 2 --vm-bytes 128M --timeout 60 &
./build/tests/realtime_stress_test > results/stress_test.txt

# 4. 生成报告
python3 tools/rt_performance_analyzer.py results/benchmark.txt
```

**性能优化清单**:
```markdown
## 性能优化清单

### 1. 内核优化
- [ ] 禁用透明大页
- [ ] 隔离CPU
- [ ] 禁用RCU回调

### 2. 代码优化
- [ ] 内联关键函数
- [ ] SIMD指令
- [ ] 缓存友好设计

### 3. 内存优化
- [ ] 内存池
- [ ] 预分配
- [ ] 锁定内存

### 4. 调度优化
- [ ] 优先级调整
- [ ] CPU亲和性
- [ ] 避免锁竞争
```

**产出物**:
```
scripts/
├─ run_rt_benchmark.sh
└─ optimize_rt.sh

results/
├─ cyclictest.txt
├─ benchmark.txt
└─ stress_test.txt

docs/realtime/
└─ rt_module_guide.md
```

**验收标准**:
- [ ] cyclictest最大延迟 < 200μs
- [ ] 系统抖动 < 50μs
- [ ] 无截止时间错过
- [ ] 代码覆盖率 > 90%

---

## 4. Phase 3: 集成测试（3天，2026-03-16至2026-03-18）

### 4.1 Day 1（03-16）：模块集成测试

#### 4.1.1 上午（08:00-12:00）：模块接口集成

**测试内容**:
```cpp
// tests/integration/test_module_integration.cpp

// 测试1：动力系统 → 动力学
TEST(IntegrationTest, PowertrainToDynamics) {
    PowertrainModel powertrain(config);
    VehicleDynamics dynamics(vehicle_config);
    
    ControlCommand cmd = {1.0, 0.0, 0.0};
    VehicleState state;
    
    for (int i = 0; i < 10000; ++i) {
        // 动力系统计算输出扭矩
        double wheel_torque = powertrain.calculate_wheel_torque(cmd.throttle, state.velocity.x());
        
        // 传递给动力学模块
        dynamics.apply_wheel_torque(wheel_torque);
        state = dynamics.update(state, cmd, 0.001);
    }
    
    // 验证
    EXPECT_GT(state.velocity.x(), 20);  // >20m/s
}

// 测试2：轮胎 → 动力学
TEST(IntegrationTest, TireToDynamics) {
    TireModel tire(tire_config);
    VehicleDynamics dynamics(vehicle_config);
    
    VehicleState state;
    state.velocity = Eigen::Vector3d(20, 0, 0);
    
    for (int i = 0; i < 10000; ++i) {
        // 计算滑移
        double alpha = calculate_slip_angle(state);
        double kappa = calculate_slip_ratio(state);
        
        // 轮胎力
        auto forces = tire.calculate_forces(kappa, alpha, 5000);
        
        // 传递给动力学
        dynamics.apply_tire_forces(forces);
        state = dynamics.update(state, cmd, 0.001);
    }
    
    // 验证
    EXPECT_TRUE(std::isfinite(state.position.x()));
}

// 测试3：底盘 → 动力学
TEST(IntegrationTest, ChassisToDynamics) {
    SuspensionModel suspension(config);
    SteeringModel steering(config);
    VehicleDynamics dynamics(vehicle_config);
    
    // 模拟转向工况
    ControlCommand cmd = {0.5, 0.0, 0.1};  // 转向10%
    
    for (int i = 0; i < 10000; ++i) {
        // 悬架力
        auto susp_forces = suspension.calculate_forces(...);
        
        // 转向角
        double steering_angle = steering.calculate_angle(cmd.steering);
        
        // 传递给动力学
        dynamics.apply_suspension_forces(susp_forces);
        dynamics.set_steering_angle(steering_angle);
        
        state = dynamics.update(state, cmd, 0.001);
    }
    
    // 验证
    EXPECT_NEAR(state.euler_angles.z(), expected_yaw, 0.1);
}
```

**验收标准**:
- [ ] 所有模块接口集成成功
- [ ] 数据流正确
- [ ] 无内存泄漏

---

#### 4.1.2 下午（14:00-18:00）：全系统集成

**测试内容**:
```cpp
// tests/integration/test_full_system.cpp

class FullSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化所有模块
        powertrain_ = std::make_unique<PowertrainModel>(powertrain_config);
        chassis_ = std::make_unique<ChassisModel>(chassis_config);
        dynamics_ = std::make_unique<VehicleDynamics>(vehicle_config);
        tires_ = std::make_unique<TireModel>(tire_config);
        scheduler_ = std::make_unique<RealtimeScheduler>();
        
        // 配置调度器
        scheduler_->add_periodic_task(
            "simulation_step",
            [this]() { simulation_step(); },
            95,
            std::chrono::microseconds(1000)
        );
    }
    
    void simulation_step() {
        // 1. 获取控制指令
        ControlCommand cmd = get_control_command();
        
        // 2. 动力系统
        double wheel_torque = powertrain_->calculate_wheel_torque(cmd.throttle, state_.velocity.x());
        
        // 3. 底盘系统
        double steering_angle = chassis_->get_steering_angle(cmd.steering);
        auto suspension_forces = chassis_->get_suspension_forces(state_);
        
        // 4. 轮胎模型
        for (int i = 0; i < 4; ++i) {
            double alpha = calculate_slip_angle(state_, i);
            double kappa = calculate_slip_ratio(state_, i);
            tire_forces_[i] = tires_->calculate_forces(kappa, alpha, 5000);
        }
        
        // 5. 动力学求解
        dynamics_->apply_wheel_torque(wheel_torque);
        dynamics_->apply_tire_forces(tire_forces_);
        dynamics_->apply_suspension_forces(suspension_forces);
        state_ = dynamics_->update(state_, cmd, 0.001);
    }
    
    std::unique_ptr<PowertrainModel> powertrain_;
    std::unique_ptr<ChassisModel> chassis_;
    std::unique_ptr<VehicleDynamics> dynamics_;
    std::unique_ptr<TireModel> tires_;
    std::unique_ptr<RealtimeScheduler> scheduler_;
    
    VehicleState state_;
    std::array<TireForces, 4> tire_forces_;
};

TEST_F(FullSystemTest, StraightLineAcceleration) {
    state_.velocity = Eigen::Vector3d::Zero();
    ControlCommand cmd = {1.0, 0.0, 0.0};
    
    scheduler_->start();
    
    // 运行10秒
    for (int i = 0; i < 10000; ++i) {
        set_control_command(cmd);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    scheduler_->stop();
    
    // 验证
    EXPECT_GT(state_.velocity.x() * 3.6, 100);  // >100km/h
}

TEST_F(FullSystemTest, DoubleLaneChange) {
    state_.velocity = Eigen::Vector3d(30, 0, 0);
    
    scheduler_->start();
    
    // 双移线工况
    for (int i = 0; i < 10000; ++i) {
        double t = i * 0.001;
        ControlCommand cmd = {
            0.5,  // 油门
            0.0,  // 制动
            generate_dlc_steering(t)  // 转向
        };
        
        set_control_command(cmd);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    scheduler_->stop();
    
    // 验证
    EXPECT_LT(max_lateral_acceleration, 0.8 * 9.81);
    EXPECT_TRUE(course_completed);
}
```

**验收标准**:
- [ ] 全系统集成成功
- [ ] 仿真结果合理
- [ ] 性能达标（<1ms/step）

---

### 4.2 Day 2（03-17）：实时性验证测试

#### 4.2.1 上午（08:00-12:00）：延迟测试

**测试内容**:
```cpp
// tests/realtime/test_latency.cpp

TEST(LatencyTest, TaskExecutionTime) {
    VehicleDynamics dynamics(config);
    VehicleState state;
    ControlCommand cmd = {0.5, 0.0, 0.0};
    
    std::vector<double> execution_times;
    
    for (int i = 0; i < 100000; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        state = dynamics.update(state, cmd, 0.001);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        execution_times.push_back(duration.count());
    }
    
    // 统计
    double avg = std::accumulate(execution_times.begin(), execution_times.end(), 0.0) / execution_times.size();
    double max = *std::max_element(execution_times.begin(), execution_times.end());
    
    std::cout << "Average: " << avg << " μs" << std::endl;
    std::cout << "Max: " << max << " μs" << std::endl;
    
    EXPECT_LT(avg, 500);  // 平均<500μs
    EXPECT_LT(max, 1000);  // 最大<1ms
}

TEST(LatencyTest, SchedulingJitter) {
    RealtimeScheduler scheduler;
    
    std::vector<double> jitters;
    auto last_time = std::chrono::high_resolution_clock::now();
    
    scheduler.add_periodic_task(
        "test_task",
        [&]() {
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - last_time);
            
            double jitter = abs(duration.count() - 1000);  // 期望1000μs
            jitters.push_back(jitter);
            
            last_time = now;
        },
        95,
        std::chrono::microseconds(1000)
    );
    
    scheduler.start();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    scheduler.stop();
    
    // 统计
    double avg_jitter = std::accumulate(jitters.begin(), jitters.end(), 0.0) / jitters.size();
    double max_jitter = *std::max_element(jitters.begin(), jitters.end());
    
    std::cout << "Average Jitter: " << avg_jitter << " μs" << std::endl;
    std::cout << "Max Jitter: " << max_jitter << " μs" << std::endl;
    
    EXPECT_LT(avg_jitter, 20);  // 平均抖动<20μs
    EXPECT_LT(max_jitter, 100);  // 最大抖动<100μs
}
```

**验收标准**:
- [ ] 平均执行时间 < 500μs
- [ ] 最大执行时间 < 1ms
- [ ] 平均调度抖动 < 20μs
- [ ] 最大调度抖动 < 100μs

---

#### 4.2.2 下午（14:00-18:00）：压力测试

**测试内容**:
```bash
# scripts/stress_test.sh
#!/bin/bash

echo "Starting stress test..."

# 1. CPU压力
stress --cpu 4 --timeout 60 &

# 2. 内存压力
stress --vm 2 --vm-bytes 512M --timeout 60 &

# 3. I/O压力
stress --io 2 --timeout 60 &

# 4. 运行实时测试
./build/tests/realtime_stress_test

# 5. 收集结果
echo "Collecting results..."
python3 tools/analyze_stress_test.py results/

echo "Stress test completed."
```

```cpp
// tests/realtime/test_stress.cpp

TEST(StressTest, UnderCpuLoad) {
    // 在CPU高负载下测试
    RealtimeScheduler scheduler;
    VehicleDynamics dynamics(config);
    
    std::atomic<int> deadline_misses{0};
    
    scheduler.add_periodic_task(
        "simulation",
        [&]() {
            auto start = std::chrono::high_resolution_clock::now();
            
            dynamics.update(state, cmd, 0.001);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            if (duration.count() > 1000) {
                deadline_misses++;
            }
        },
        95,
        std::chrono::microseconds(1000)
    );
    
    scheduler.start();
    std::this_thread::sleep_for(std::chrono::seconds(60));
    scheduler.stop();
    
    std::cout << "Deadline misses: " << deadline_misses << std::endl;
    
    EXPECT_EQ(deadline_misses, 0);  // 无截止时间错过
}

TEST(StressTest, LongRunningStability) {
    // 长时间运行稳定性测试（24小时）
    RealtimeScheduler scheduler;
    VehicleDynamics dynamics(config);
    
    std::atomic<bool> running{true};
    std::atomic<int> errors{0};
    
    scheduler.add_periodic_task(
        "simulation",
        [&]() {
            try {
                dynamics.update(state, cmd, 0.001);
                
                // 检查数值稳定性
                if (!std::isfinite(state.position.x())) {
                    errors++;
                }
            } catch (...) {
                errors++;
            }
        },
        95,
        std::chrono::microseconds(1000)
    );
    
    scheduler.start();
    
    // 运行24小时
    for (int hour = 0; hour < 24; ++hour) {
        std::this_thread::sleep_for(std::chrono::hours(1));
        std::cout << "Hour " << hour << ": errors = " << errors << std::endl;
    }
    
    scheduler.stop();
    
    EXPECT_EQ(errors, 0);
}
```

**验收标准**:
- [ ] CPU压力下无截止时间错过
- [ ] 24小时运行无错误
- [ ] 内存无泄漏

---

### 4.3 Day 3（03-18）：性能压力测试

#### 4.3.1 上午（08:00-12:00）：性能基准

**测试内容**:
```python
# tools/performance_benchmark.py
import time
import numpy as np

class PerformanceBenchmark:
    def __init__(self):
        # 加载C++模块
        import vehicle_sim_python
        self.dynamics = vehicle_sim_python.Vehicle6DOFDynamics(1500, inertia)
    
    def benchmark_single_step(self, iterations=1000000):
        """单步性能基准"""
        times = []
        
        for _ in range(iterations):
            start = time.perf_counter_ns()
            
            self.dynamics.update(state, cmd, 0.001)
            
            end = time.perf_counter_ns()
            times.append((end - start) / 1000)  # 转换为μs
        
        return {
            'avg': np.mean(times),
            'std': np.std(times),
            'min': np.min(times),
            'max': np.max(times),
            'p99': np.percentile(times, 99)
        }
    
    def benchmark_full_system(self, duration=60):
        """全系统性能基准"""
        start_time = time.time()
        step_count = 0
        
        while time.time() - start_time < duration:
            self.dynamics.update(state, cmd, 0.001)
            step_count += 1
        
        return {
            'total_steps': step_count,
            'steps_per_second': step_count / duration,
            'avg_step_time_us': duration * 1e6 / step_count
        }
    
    def generate_report(self):
        """生成报告"""
        single_step = self.benchmark_single_step()
        full_system = self.benchmark_full_system()
        
        report = f"""
# 性能基准测试报告

## 单步性能
- 平均时间: {single_step['avg']:.2f} μs
- 标准差: {single_step['std']:.2f} μs
- 最小时间: {single_step['min']:.2f} μs
- 最大时间: {single_step['max']:.2f} μs
- P99: {single_step['p99']:.2f} μs

## 全系统性能（60秒）
- 总步数: {full_system['total_steps']}
- 每秒步数: {full_system['steps_per_second']:.0f}
- 平均步长时间: {full_system['avg_step_time_us']:.2f} μs

## 验收标准
- ✅ 平均单步时间 < 500 μs: {single_step['avg'] < 500}
- ✅ 最大单步时间 < 1000 μs: {single_step['max'] < 1000}
- ✅ 每秒步数 > 1000: {full_system['steps_per_second'] > 1000}
"""
        
        with open('performance_report.md', 'w') as f:
            f.write(report)
        
        return report

# 运行基准测试
if __name__ == '__main__':
    benchmark = PerformanceBenchmark()
    report = benchmark.generate_report()
    print(report)
```

**验收标准**:
- [ ] 平均单步时间 < 500μs
- [ ] P99 < 800μs
- [ ] 每秒步数 > 1000

---

#### 4.3.2 下午（14:00-18:00）：HIL集成测试

**测试内容**:
```cpp
// tests/hil/test_hil_integration.cpp

class HILIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化HIL接口
        can_interface_ = std::make_unique<SocketCAN>();
        can_interface_->open("can0");
        
        udp_interface_ = std::make_unique<UDPSocket>();
        udp_interface_->open(5000);
        
        // 初始化仿真系统
        system_ = std::make_unique<VehicleSimulationSystem>();
    }
    
    std::unique_ptr<SocketCAN> can_interface_;
    std::unique_ptr<UDPSocket> udp_interface_;
    std::unique_ptr<VehicleSimulationSystem> system_;
};

TEST_F(HILIntegrationTest, CANCommunication) {
    // 测试CAN通信
    ControlCommand cmd;
    
    for (int i = 0; i < 10000; ++i) {
        // 接收CAN消息
        uint32_t can_id;
        uint8_t data[8];
        size_t len;
        
        can_interface_->receive(can_id, data, len);
        
        // 解析控制指令
        cmd = parse_can_message(can_id, data);
        
        // 仿真步进
        system_->update(cmd, 0.001);
        
        // 发送CAN消息
        uint8_t response[8];
        encode_vehicle_state(system_->get_state(), response);
        can_interface_->send(0x100, response, 8);
    }
    
    // 验证通信延迟
    EXPECT_LT(avg_communication_latency, 100);  // <100μs
}

TEST_F(HILIntegrationTest, UDPCommunication) {
    // 测试UDP通信
    for (int i = 0; i < 10000; ++i) {
        // 接收UDP数据包
        uint8_t buffer[1024];
        ssize_t len = udp_interface_->receive(buffer, sizeof(buffer));
        
        // 解析
        ControlCommand cmd = parse_udp_command(buffer, len);
        
        // 仿真
        system_->update(cmd, 0.001);
        
        // 响应
        uint8_t response[1024];
        size_t resp_len = encode_vehicle_state(system_->get_state(), response);
        udp_interface_->send_to("192.168.1.200", 6000, response, resp_len);
    }
    
    EXPECT_LT(avg_communication_latency, 200);  // <200μs
}

TEST_F(HILIntegrationTest, FullHILLoop) {
    // 完整HIL闭环测试
    RealtimeScheduler scheduler;
    
    scheduler.add_periodic_task(
        "hil_comm",
        [this]() {
            // CAN接收
            can_interface_->receive(...);
            
            // 仿真
            system_->update(cmd, 0.001);
            
            // CAN发送
            can_interface_->send(...);
        },
        99,  // 最高优先级
        std::chrono::microseconds(1000)
    );
    
    scheduler.start();
    std::this_thread::sleep_for(std::chrono::seconds(60));
    scheduler.stop();
    
    // 验证
    auto stats = scheduler.get_stats();
    EXPECT_EQ(stats.deadline_misses, 0);
    EXPECT_LT(stats.max_jitter.count(), 50);
}
```

**验收标准**:
- [ ] CAN通信延迟 < 100μs
- [ ] UDP通信延迟 < 200μs
- [ ] 完整HIL闭环无截止时间错过

---

## 5. Phase 4: 文档编写（2天，2026-03-19至2026-03-20）

### 5.1 Day 1（03-19）：API文档 + 用户指南

#### 5.1.1 上午（08:00-12:00）：API文档生成

**内容**:
```python
# 使用Sphinx生成API文档
# docs/api/conf.py

project = 'Vehicle Dynamics Simulation'
copyright = '2026, EV Dynamics Team'
author = 'ArchitectAgent'

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.napoleon',
    'sphinx.ext.viewcode',
    'breathe'  # C++文档
]

# 生成Python API文档
# sphinx-apidoc -o docs/api/source src/

# 生成C++ API文档（使用Doxygen + Breathe）
# doxygen Doxyfile
# breathe-apidoc -o docs/api/source/doxygen xml/
```

**API文档结构**:
```
docs/api/
├─ index.rst
├─ python/
│  ├─ powertrain.rst
│  ├─ chassis.rst
│  ├─ dynamics.rst
│  ├─ tire.rst
│  └─ realtime.rst
└─ cpp/
   ├─ class_vehicle_dynamics.rst
   ├─ class_tire_model.rst
   └─ class_realtime_scheduler.rst
```

**示例文档**:
```rst
Vehicle6DOFDynamics
====================

.. class:: Vehicle6DOFDynamics

   六自由度车辆动力学求解器

   **参数**:
      - mass (float): 车辆质量 (kg)
      - inertia (np.ndarray): 转动惯量矩阵 (3x3)

   **示例**:

   .. code-block:: python

      from vehicle_sim_python import Vehicle6DOFDynamics
      import numpy as np

      # 创建动力学求解器
      mass = 1500.0  # kg
      inertia = np.diag([500, 2000, 2000])  # kg*m^2
      dynamics = Vehicle6DOFDynamics(mass, inertia)

      # 更新状态
      state = dynamics.update(current_state, command, 0.001)

   .. method:: update(state, command, dt)

      更新车辆状态

      **参数**:
         - state (VehicleState): 当前状态
         - command (ControlCommand): 控制指令
         - dt (float): 时间步长 (s)

      **返回**:
         VehicleState: 新状态
```

---

#### 5.1.2 下午（14:00-18:00）：用户指南

**内容**:
```markdown
# 用户指南

## 1. 快速开始

### 1.1 环境要求

- 操作系统: Ubuntu 22.04 + PREEMPT_RT内核
- Python: 3.10+
- C++编译器: GCC 11+
- 硬件: 4核CPU, 8GB内存

### 1.2 安装

```bash
# 1. 克隆仓库
git clone https://github.com/your-org/ev_dynamics_simulation.git
cd ev_dynamics_simulation

# 2. 安装Python依赖
pip install -r requirements.txt

# 3. 编译C++模块
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 4. 安装
make install
```

### 1.3 第一个仿真

```python
from vehicle_sim_python import (
    Vehicle6DOFDynamics,
    ControlCommand,
    VehicleState
)

# 1. 创建仿真器
dynamics = Vehicle6DOFDynamics(
    mass=1500.0,
    inertia=[500, 2000, 2000]
)

# 2. 设置初始状态
state = VehicleState()
state.velocity = [0, 0, 0]  # 静止

# 3. 运行仿真
for i in range(10000):  # 10秒
    # 控制指令：全油门
    cmd = ControlCommand(throttle=1.0, brake=0.0, steering=0.0)
    
    # 更新状态
    state = dynamics.update(state, cmd, 0.001)
    
    # 打印速度
    if i % 1000 == 0:
        print(f"Speed: {state.velocity[0] * 3.6:.1f} km/h")

# 4. 结果
print(f"Final speed: {state.velocity[0] * 3.6:.1f} km/h")
```

## 2. 配置文件

### 2.1 车辆配置

```yaml
# config/vehicle.yaml
vehicle:
  name: "TestVehicle"
  mass: 1500.0  # kg
  
  inertia:
    Ixx: 500
    Iyy: 2000
    Izz: 2000

powertrain:
  motor:
    type: "PMSM"
    rated_power: 150000  # W
    max_torque: 300  # Nm

tire:
  model: "Pacejka"
  coefficients:
    Bx: 12.0
    Cx: 1.65
    Dx: 1.0
```

### 2.2 实时配置

```yaml
# config/realtime.yaml
realtime:
  enabled: true
  priority: 95
  cpu_affinity: [2, 3]
  
  memory:
    locked: true
    pool_size: 1048576  # 1MB
```

## 3. 高级用法

### 3.1 自定义轮胎模型

```python
from vehicle_sim_python import TireModelBase

class MyTireModel(TireModelBase):
    def calculate_forces(self, kappa, alpha, Fz):
        # 自定义轮胎力计算
        Fx = ...
        Fy = ...
        return Fx, Fy
```

### 3.2 HIL集成

```python
from vehicle_sim_python import HILInterface

# 创建HIL接口
hil = HILInterface(
    can_interface="can0",
    udp_port=5000
)

# 运行实时仿真
hil.run_realtime_simulation()
```

## 4. 故障排查

### 4.1 实时性不达标

```bash
# 检查内核
uname -v  # 应显示PREEMPT_RT

# 测试延迟
sudo cyclictest -l100000 -m -Sp90 -i200 -h400 -q
# Max应<200μs
```

### 4.2 性能问题

```bash
# 性能分析
perf record -g python3 main.py
perf report
```
```

**产出物**:
```
docs/
├─ api/
│  ├─ index.html
│  └─ python/
├─ user_guide/
│  ├─ quick_start.md
│  ├─ configuration.md
│  └─ advanced_usage.md
└─ examples/
   ├─ basic_simulation.py
   ├─ custom_tire.py
   └─ hil_integration.py
```

---

### 5.2 Day 2（03-20）：部署文档 + 示例代码

#### 5.2.1 上午（08:00-12:00）：部署文档

**内容**:
```markdown
# 部署指南

## 1. 系统要求

### 1.1 硬件要求

| 组件 | 最低要求 | 推荐配置 |
|------|---------|---------|
| CPU | 4核 2.0GHz | 8核 3.0GHz |
| 内存 | 8GB | 16GB |
| 存储 | 50GB SSD | 100GB NVMe |
| 网络 | 千兆以太网 | 万兆以太网 |
| CAN | SocketCAN兼容 | PCAN-USB |

### 1.2 软件要求

- Ubuntu 22.04 LTS
- Linux内核 5.15+ (PREEMPT_RT)
- Python 3.10+
- GCC 11+
- CMake 3.15+

## 2. 操作系统配置

### 2.1 安装PREEMPT_RT内核

```bash
# 1. 安装RT内核
sudo apt update
sudo apt install linux-image-rt-amd64 linux-headers-rt-amd64

# 2. 更新GRUB
sudo update-grub

# 3. 重启
sudo reboot

# 4. 验证
uname -v
# 应包含"PREEMPT_RT"
```

### 2.2 内核参数优化

```bash
# 编辑/etc/default/grub
GRUB_CMDLINE_LINUX="isolcpus=2,3 nohz_full=2,3 rcu_nocbs=2,3"

# 更新并重启
sudo update-grub
sudo reboot
```

### 2.3 实时权限配置

```bash
# 编辑/etc/security/limits.conf
* soft rtprio 99
* hard rtprio 99
* soft memlock unlimited
* hard memlock unlimited

# 注销并重新登录
```

## 3. 软件安装

### 3.1 依赖安装

```bash
# 系统依赖
sudo apt install -y \
    build-essential cmake git \
    python3 python3-pip python3-dev \
    libeigen3-dev \
    can-utils \
    net-tools

# Python依赖
pip3 install -r requirements.txt
```

### 3.2 编译与安装

```bash
# 1. 克隆代码
git clone https://github.com/your-org/ev_dynamics_simulation.git
cd ev_dynamics_simulation

# 2. 编译
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 3. 测试
ctest --verbose

# 4. 安装
sudo make install

# 5. 安装Python包
cd ../
pip3 install .
```

## 4. HIL硬件配置

### 4.1 CAN接口配置

```bash
# 1. 加载驱动
sudo modprobe can
sudo modprobe can_raw
sudo modprobe vcan  # 虚拟CAN（测试用）

# 2. 配置物理CAN
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up

# 3. 验证
ip link show can0
```

### 4.2 网络配置

```bash
# 1. 配置静态IP
sudo nmcli con mod eth0 ipv4.addresses 192.168.1.100/24
sudo nmcli con mod eth0 ipv4.gateway 192.168.1.1
sudo nmcli con mod eth0 ipv4.method manual

# 2. 优化网络参数
sudo sysctl -w net.core.rmem_max=12582912
sudo sysctl -w net.core.wmem_max=12582912
```

## 5. 运行仿真

### 5.1 非实时模式

```bash
python3 main.py --config config/vehicle.yaml
```

### 5.2 实时模式

```bash
# 设置CPU亲和性
taskset -c 2,3 python3 main.py --realtime --config config/realtime.yaml
```

### 5.3 HIL模式

```bash
sudo python3 main.py --hil --config config/hil.yaml
```

## 6. 监控与维护

### 6.1 性能监控

```bash
# CPU使用率
htop

# 实时性能
sudo cyclictest -l100000 -m -Sp90 -i200 -h400 -q

# 内存使用
free -h
```

### 6.2 日志管理

```bash
# 查看日志
tail -f logs/simulation.log

# 日志轮转
sudo logrotate /etc/logrotate.d/vehicle_sim
```

## 7. 故障排查

### 7.1 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|---------|
| 实时性不达标 | 内核未配置RT | 重新安装RT内核 |
| CAN通信失败 | 接口未启动 | `ip link set can0 up` |
| 内存不足 | 未锁定内存 | 配置`memlock` |
| 权限错误 | 无实时权限 | 配置`limits.conf` |

### 7.2 联系支持

- GitHub Issues: https://github.com/your-org/ev_dynamics_simulation/issues
- Email: support@your-company.com
```

---

#### 5.2.2 下午（14:00-18:00）：示例代码

**示例1: 基础仿真**
```python
# examples/basic_simulation.py
"""
基础仿真示例：直线加速
"""
from vehicle_sim_python import Vehicle6DOFDynamics, ControlCommand
import numpy as np
import matplotlib.pyplot as plt

def main():
    # 创建仿真器
    dynamics = Vehicle6DOFDynamics(
        mass=1500.0,
        inertia=np.diag([500, 2000, 2000])
    )
    
    # 初始状态
    state = dynamics.get_initial_state()
    
    # 数据记录
    time_data = []
    speed_data = []
    acceleration_data = []
    
    # 仿真循环
    cmd = ControlCommand(throttle=1.0, brake=0.0, steering=0.0)
    
    for i in range(10000):  # 10秒
        # 更新状态
        state = dynamics.update(state, cmd, 0.001)
        
        # 记录数据
        time_data.append(i * 0.001)
        speed_data.append(state.velocity[0] * 3.6)  # m/s -> km/h
        acceleration_data.append(state.acceleration[0] / 9.81)  # g
    
    # 绘图
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
    
    ax1.plot(time_data, speed_data)
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Speed (km/h)')
    ax1.set_title('Vehicle Speed')
    ax1.grid(True)
    
    ax2.plot(time_data, acceleration_data)
    ax2.set_xlabel('Time (s)')
    ax2.set_ylabel('Acceleration (g)')
    ax2.set_title('Longitudinal Acceleration')
    ax2.grid(True)
    
    plt.tight_layout()
    plt.savefig('basic_simulation.png')
    plt.show()

if __name__ == '__main__':
    main()
```

**示例2: 双移线工况**
```python
# examples/double_lane_change.py
"""
双移线工况示例
"""
from vehicle_sim_python import Vehicle6DOFDynamics, ControlCommand
import numpy as np

def generate_dlc_steering(t):
    """生成双移线转向输入"""
    # ISO 3888-2标准双移线
    if t < 1.0:
        return 0.0
    elif t < 2.0:
        return 0.05 * np.sin(2 * np.pi * (t - 1.0))
    elif t < 3.0:
        return -0.05 * np.sin(2 * np.pi * (t - 2.0))
    else:
        return 0.0

def main():
    dynamics = Vehicle6DOFDynamics(
        mass=1500.0,
        inertia=np.diag([500, 2000, 2000])
    )
    
    state = dynamics.get_initial_state()
    state.velocity = [30, 0, 0]  # 30 m/s = 108 km/h
    
    for i in range(10000):
        t = i * 0.001
        
        # 生成控制指令
        steering = generate_dlc_steering(t)
        cmd = ControlCommand(throttle=0.5, brake=0.0, steering=steering)
        
        # 更新
        state = dynamics.update(state, cmd, 0.001)
        
        # 打印
        if i % 100 == 0:
            print(f"t={t:.1f}s, yaw_rate={state.angular_rate[2] * 180 / np.pi:.1f} deg/s")

if __name__ == '__main__':
    main()
```

**示例3: HIL集成**
```python
# examples/hil_integration.py
"""
HIL集成示例
"""
from vehicle_sim_python import Vehicle6DOFDynamics, HILInterface
import time

def main():
    # 创建HIL接口
    hil = HILInterface(
        can_interface="can0",
        udp_port=5000
    )
    
    # 创建仿真器
    dynamics = Vehicle6DOFDynamics(
        mass=1500.0,
        inertia=[500, 2000, 2000]
    )
    
    # 启动实时仿真
    hil.start_realtime_simulation(dynamics, dt=0.001)
    
    try:
        # 运行60秒
        time.sleep(60)
    except KeyboardInterrupt:
        pass
    finally:
        hil.stop()

if __name__ == '__main__':
    main()
```

**产出物**:
```
examples/
├─ basic_simulation.py
├─ double_lane_change.py
├─ custom_tire_model.py
├─ hil_integration.py
├─ parameter_sweep.py
└─ data_recording.py

docs/
├─ deployment_guide.md
└─ troubleshooting.md
```

---

## 6. Phase 5: 质量评估（1天，2026-03-21）

### 6.1 上午（08:00-12:00）：代码质量评估

#### 6.1.1 静态代码分析

**工具**:
```bash
# Python静态分析
black --check src/
pylint src/
mypy src/

# C++静态分析
cppcheck --enable=all src/
clang-tidy src/*.cpp -- -std=c++17
```

**评估标准**:
```markdown
## 代码质量标准

### 1. Python代码
- [ ] Black格式化通过
- [ ] Pylint评分 > 9.0/10
- [ ] MyPy类型检查通过
- [ ] 无安全漏洞（Bandit）

### 2. C++代码
- [ ] Clang-Format格式化通过
- [ ] Clang-Tidy警告 < 10个
- [ ] Cppcheck错误 = 0
- [ ] 符合MISRA C++规范（关键模块）

### 3. 测试覆盖率
- [ ] Python覆盖率 > 90%
- [ ] C++覆盖率 > 90%
- [ ] 关键路径覆盖率 = 100%
```

#### 6.1.2 代码审查

**审查清单**:
```markdown
## 代码审查清单

### 1. 架构设计
- [ ] 模块划分合理
- [ ] 接口设计清晰
- [ ] 依赖关系正确
- [ ] 扩展性良好

### 2. 代码质量
- [ ] 命名规范
- [ ] 注释充分
- [ ] 无重复代码
- [ ] 错误处理完善

### 3. 性能
- [ ] 无明显性能瓶颈
- [ ] 内存管理正确
- [ ] 并发安全
- [ ] 资源释放及时

### 4. 测试
- [ ] 单元测试充分
- [ ] 集成测试完整
- [ ] 边界条件覆盖
- [ ] 异常情况处理
```

**产出物**:
```
reviews/
├─ code_quality_report.md
├─ static_analysis_report.txt
└─ code_review_checklist.md
```

---

### 6.2 下午（14:00-18:00）：性能与可行性评估

#### 6.2.1 性能评估

**评估内容**:
```python
# tools/final_performance_evaluation.py

class FinalPerformanceEvaluation:
    def evaluate_all(self):
        results = {}
        
        # 1. 实时性
        results['realtime'] = self.evaluate_realtime()
        
        # 2. 精度
        results['accuracy'] = self.evaluate_accuracy()
        
        # 3. 稳定性
        results['stability'] = self.evaluate_stability()
        
        # 4. 资源使用
        results['resources'] = self.evaluate_resources()
        
        # 生成报告
        self.generate_final_report(results)
        
        return results
    
    def evaluate_realtime(self):
        """实时性评估"""
        # cyclictest
        max_latency = run_cyclictest()
        
        # 调度抖动
        max_jitter = measure_scheduling_jitter()
        
        return {
            'max_latency_us': max_latency,
            'max_jitter_us': max_jitter,
            'deadline_miss_rate': deadline_miss_rate,
            'pass': max_latency < 1000 and max_jitter < 100
        }
    
    def evaluate_accuracy(self):
        """精度评估"""
        # 与CarSim对比
        error = compare_with_carsim()
        
        return {
            'max_error_percent': error,
            'avg_error_percent': avg_error,
            'pass': error < 5
        }
    
    def evaluate_stability(self):
        """稳定性评估"""
        # 长时间运行
        errors = run_long_term_test(duration=24 * 3600)
        
        return {
            'runtime_errors': errors,
            'memory_leaks': detect_memory_leaks(),
            'pass': errors == 0
        }
    
    def evaluate_resources(self):
        """资源使用评估"""
        return {
            'cpu_usage_percent': measure_cpu_usage(),
            'memory_usage_mb': measure_memory_usage(),
            'disk_usage_mb': measure_disk_usage(),
            'pass': cpu_usage < 50 and memory_usage < 2048
        }
```

**性能报告**:
```markdown
# 最终性能评估报告

## 1. 实时性
- ✅ 最大延迟: 180 μs (< 1ms)
- ✅ 最大抖动: 45 μs (< 100μs)
- ✅ 截止时间错过率: 0%

## 2. 精度
- ✅ 与CarSim对比误差: 3.2% (< 5%)
- ✅ 平均误差: 1.8%

## 3. 稳定性
- ✅ 24小时运行错误: 0
- ✅ 内存泄漏: 0

## 4. 资源使用
- ✅ CPU使用率: 35% (< 50%)
- ✅ 内存使用: 1.2 GB (< 2GB)
- ✅ 磁盘使用: 50 MB

## 总体评估
✅ **所有性能指标达标，系统可交付**
```

---

#### 6.2.2 可行性评估

**评估内容**:
```markdown
# 可行性评估报告

## 1. 技术可行性

### 1.1 实时性
- ✅ PREEMPT_RT内核满足<1ms要求
- ✅ C++关键路径优化充分
- ⚠️ 极端工况可能需要进一步优化

### 1.2 精度
- ✅ Pacejka模型精度满足要求
- ✅ RK4积分器稳定性好
- ✅ 与商业软件对比误差<5%

### 1.3 完整性
- ✅ 6自由度模型完整
- ✅ 动力系统、底盘、轮胎模块齐全
- ✅ HIL接口支持CAN和UDP

## 2. 工程可行性

### 2.1 开发难度
- ✅ 团队具备所需技能
- ✅ 技术选型成熟
- ⚠️ 16天工期紧张

### 2.2 资源需求
- ✅ 硬件要求合理
- ✅ 软件依赖可控
- ✅ 开发工具齐全

### 2.3 风险控制
- ✅ 风险识别充分
- ✅ 缓解措施可行
- ✅ 降级方案明确

## 3. 经济可行性

### 3.1 成本
- ✅ 开源软件，无许可费用
- ✅ 硬件成本可控
- ✅ 维护成本低

### 3.2 收益
- ✅ 替代商业软件，节省许可费
- ✅ 支持定制开发
- ✅ 知识产权自主

## 4. 时间可行性

### 4.1 进度
- ✅ 16天计划合理
- ⚠️ 风险缓冲不足
- ✅ Agent并行开发提高效率

### 4.2 里程碑
- ✅ 阶段划分清晰
- ✅ 验收标准明确
- ✅ 交付时间可控

## 总体结论
✅ **项目技术可行、工程可控、经济合理，建议按计划执行**
```

---

#### 6.2.3 最终交付

**交付清单**:
```markdown
# 项目交付清单

## 1. 代码
- [x] 源代码（Python + C++）
- [x] 单元测试
- [x] 集成测试
- [x] 性能测试

## 2. 文档
- [x] 架构设计文档
- [x] 技术选型文档
- [x] 开发路线图
- [x] API文档
- [x] 用户指南
- [x] 部署指南

## 3. 配置
- [x] 车辆配置文件
- [x] 实时配置文件
- [x] HIL配置文件

## 4. 示例
- [x] 基础仿真示例
- [x] 双移线工况示例
- [x] HIL集成示例
- [x] 自定义模型示例

## 5. 工具
- [x] 性能分析工具
- [x] 数据可视化工具
- [x] 参数拟合工具

## 6. 测试报告
- [x] 单元测试报告
- [x] 集成测试报告
- [x] 性能测试报告
- [x] 实时性测试报告

## 7. 部署包
- [x] Docker镜像（开发环境）
- [x] 安装脚本
- [x] 配置脚本

## 验收签字
- 架构师: _______________  日期: _______
- 开发负责人: ___________  日期: _______
- 测试负责人: ___________  日期: _______
- 项目经理: ______________  日期: _______
```

---

## 7. 里程碑与验收标准

### 7.1 里程碑总览

| 里程碑 | 日期 | 验收标准 | 负责人 |
|--------|------|---------|--------|
| **M1: 架构完成** | 2026-03-08 | 文档完整，原型验证通过 | ArchitectAgent |
| **M2: 核心模块完成** | 2026-03-15 | 5个模块开发完成，单元测试通过 | 各BackendAgent |
| **M3: 集成完成** | 2026-03-18 | 集成测试通过，实时性达标 | TestAgent |
| **M4: 文档完成** | 2026-03-20 | 所有文档完成，示例可运行 | DocAgent |
| **M5: 项目交付** | 2026-03-21 | 质量评估通过，可实际部署 | EvaluatorAgent |

### 7.2 详细验收标准

#### M1: 架构完成（2026-03-08）

**文档验收**:
- [ ] ARCHITECTURE.md完整（>100KB）
- [ ] MODULE_INTERFACE_SPEC.md完整（>50KB）
- [ ] DATA_STRUCTURES.md完整（>50KB）
- [ ] TECHNICAL_DECISIONS.md完整（>60KB）
- [ ] DEVELOPMENT_ROADMAP.md完整（>40KB）

**原型验收**:
- [ ] PREEMPT_RT内核配置正确
- [ ] cyclictest最大延迟 < 200μs
- [ ] C++核心模块单步计算 < 500μs
- [ ] Python-C++集成开销 < 50μs

---

#### M2: 核心模块完成（2026-03-15）

**动力系统模块**:
- [ ] 电机模型（PMSM）完成
- [ ] 电池模型（SOC/SOH）完成
- [ ] 变速器模型完成
- [ ] 单元测试覆盖率 > 90%
- [ ] 单步计算时间 < 200μs

**底盘系统模块**:
- [ ] 悬架模型完成
- [ ] 转向模型（EPS/SBW）完成
- [ ] 制动模型（EMB）完成
- [ ] 单元测试覆盖率 > 90%
- [ ] 单步计算时间 < 200μs

**车辆动力学模块**:
- [ ] 6自由度运动学方程完成
- [ ] RK4积分器完成
- [ ] 状态空间实现完成
- [ ] 单元测试覆盖率 > 90%
- [ ] 单步计算时间 < 500μs

**轮胎模型模块**:
- [ ] Pacejka魔术公式完成
- [ ] 参数拟合算法完成
- [ ] 滑移率计算完成
- [ ] 单元测试覆盖率 > 90%
- [ ] 单步计算时间 < 150μs

**实时调度模块**:
- [ ] 任务调度器完成
- [ ] 优先级管理完成
- [ ] 资源管理完成
- [ ] 单元测试覆盖率 > 90%
- [ ] 调度抖动 < 50μs

---

#### M3: 集成完成（2026-03-18）

**模块集成**:
- [ ] 所有模块接口集成成功
- [ ] 数据流正确
- [ ] 无内存泄漏

**实时性验证**:
- [ ] cyclictest最大延迟 < 200μs
- [ ] 系统抖动 < 50μs
- [ ] 无截止时间错过

**性能压力**:
- [ ] 平均单步时间 < 500μs
- [ ] P99 < 800μs
- [ ] 每秒步数 > 1000
- [ ] 24小时运行无错误

---

#### M4: 文档完成（2026-03-20）

**API文档**:
- [ ] Python API文档完整
- [ ] C++ API文档完整
- [ ] 示例代码可运行

**用户文档**:
- [ ] 快速开始指南
- [ ] 配置指南
- [ ] 高级用法

**部署文档**:
- [ ] 系统要求明确
- [ ] 安装步骤清晰
- [ ] 故障排查完整

---

#### M5: 项目交付（2026-03-21）

**质量评估**:
- [ ] 代码质量评分 > 9.0/10
- [ ] 测试覆盖率 > 90%
- [ ] 静态分析无严重问题

**性能评估**:
- [ ] 实时性达标
- [ ] 精度达标（<5%）
- [ ] 稳定性达标

**可行性评估**:
- [ ] 技术可行
- [ ] 工程可控
- [ ] 经济合理

**交付物完整**:
- [ ] 代码、文档、配置、示例、工具齐全
- [ ] 验收签字完成

---

## 8. 风险管理

### 8.1 风险识别

| 风险ID | 风险描述 | 可能性 | 影响 | 风险等级 |
|--------|---------|--------|------|---------|
| R1 | 实时性不达标 | 中 | 高 | 🔴 高 |
| R2 | 集成复杂度高 | 高 | 中 | 🟡 中 |
| R3 | 性能瓶颈 | 中 | 中 | 🟡 中 |
| R4 | 工期延误 | 高 | 高 | 🔴 高 |
| R5 | 团队技能不足 | 中 | 高 | 🔴 高 |
| R6 | 需求变更 | 中 | 中 | 🟡 中 |
| R7 | 依赖库问题 | 低 | 低 | 🟢 低 |
| R8 | 硬件故障 | 低 | 中 | 🟡 中 |

### 8.2 风险应对策略

#### R1: 实时性不达标

**应对策略**:
1. **预防**:
   - 使用PREEMPT_RT内核
   - C++实现关键路径
   - 严格实时测试

2. **缓解**:
   - CPU隔离
   - 内存锁定
   - 优先级优化

3. **应急**:
   - 迁移至Xenomai
   - 使用专用实时硬件

---

#### R2: 集成复杂度高

**应对策略**:
1. **预防**:
   - 清晰接口定义
   - 持续集成
   - 模块化设计

2. **缓解**:
   - 逐步集成
   - 接口适配器
   - 回滚机制

3. **应急**:
   - 简化集成范围
   - 延长集成时间

---

#### R4: 工期延误

**应对策略**:
1. **预防**:
   - 详细计划
   - 每日跟踪
   - 风险缓冲

2. **缓解**:
   - Agent并行开发
   - 代码复用
   - 外包非核心

3. **应急**:
   - 削减功能
   - 延长工期
   - 增加资源

---

### 8.3 风险监控

**每日监控**:
- 进度燃尽图
- 技术指标监控
- 团队状态检查

**每周评审**:
- 风险状态更新
- 应对策略调整
- 经验教训总结

---

## 9. 资源分配

### 9.1 人力资源

| Agent | 工作量 | 专注度 | 关键技能 |
|-------|--------|--------|---------|
| ArchitectAgent | 3天（Phase 1） | 100% | 系统设计、性能优化 |
| BackendAgent #1 | 7天（Phase 2） | 100% | 电机控制、电池建模 |
| BackendAgent #2 | 7天（Phase 2） | 100% | 悬架、转向、制动 |
| BackendAgent #3 | 7天（Phase 2） | 100% | 多体动力学、数值计算 |
| AIEngineerAgent | 7天（Phase 2） | 100% | 参数拟合、机器学习 |
| DevOpsAgent | 7天（Phase 2） | 100% | 实时系统、性能优化 |
| TestAgent | 3天（Phase 3） | 100% | 自动化测试、性能测试 |
| DocAgent | 2天（Phase 4） | 100% | 技术写作、示例开发 |
| EvaluatorAgent | 1天（Phase 5） | 100% | 代码审查、性能分析 |

### 9.2 硬件资源

**开发环境**:
- CPU: Intel i7-12700K (12核)
- 内存: 32GB DDR4
- 存储: 1TB NVMe SSD
- 网络: 千兆以太网
- CAN: PCAN-USB

**测试环境**:
- CPU: Intel i7-12700K (12核)
- 内存: 16GB DDR4
- 存储: 512GB NVMe SSD
- CAN: Vector VN1630

### 9.3 软件资源

**操作系统**:
- Ubuntu 22.04 LTS
- Linux内核 5.15+ (PREEMPT_RT)

**开发工具**:
- GCC 11+
- CMake 3.15+
- Python 3.10+
- VS Code / CLion

**测试工具**:
- pytest
- Google Test
- cyclictest
- perf

---

## 10. 成功标准

### 10.1 功能标准

| 功能项 | 标准 | 验证方法 |
|--------|------|---------|
| **6自由度模型** | 正确实现 | 与CarSim对比 |
| **动力系统** | 扭矩误差<5% | 试验数据验证 |
| **底盘系统** | 响应延迟<50ms | 阶跃响应测试 |
| **轮胎模型** | 力误差<5% | 试验台对比 |
| **HIL接口** | 通信正常 | CAN/UDP测试 |

### 10.2 性能标准

| 性能项 | 标准 | 验证方法 |
|--------|------|---------|
| **实时延迟** | <1ms | cyclictest |
| **计算精度** | >95% | 与商业软件对比 |
| **代码覆盖率** | >90% | pytest-cov |
| **稳定性** | 24h无错误 | 长时间测试 |

### 10.3 质量标准

| 质量项 | 标准 | 验证方法 |
|--------|------|---------|
| **代码规范** | Black/Clang-Format | 自动检查 |
| **静态分析** | 无严重问题 | Cppcheck |
| **文档完整** | 100% | 人工审查 |
| **示例可运行** | 100% | 实际运行 |

### 10.4 交付标准

| 交付物 | 标准 | 验证方法 |
|--------|------|---------|
| **源代码** | 编译通过 | CI构建 |
| **测试报告** | 全部通过 | ctest |
| **文档** | 完整清晰 | 评审 |
| **部署包** | 可安装 | 实际部署 |

---

## 附录

### A. 每日站会模板

```markdown
## 每日站会（YYYY-MM-DD）

### 1. 昨天完成
- [ ] 任务1
- [ ] 任务2

### 2. 今天计划
- [ ] 任务3
- [ ] 任务4

### 3. 阻塞问题
- 问题1: ...
- 问题2: ...

### 4. 需要帮助
- ...
```

### B. 代码提交规范

```
<type>(<scope>): <subject>

<body>

<footer>

类型（type）：
- feat: 新功能
- fix: 修复bug
- docs: 文档
- style: 格式
- refactor: 重构
- test: 测试
- chore: 构建/工具

示例：
feat(dynamics): 实现6自由度运动学方程

- 添加牛顿-欧拉求解器
- 实现RK4积分器
- 添加单元测试

Closes #123
```

### C. 测试报告模板

```markdown
# 测试报告

## 1. 测试概述
- 测试日期: YYYY-MM-DD
- 测试人员: XXX
- 测试环境: ...

## 2. 测试结果
- 通过: X
- 失败: Y
- 跳过: Z

## 3. 详细结果
| 测试用例 | 结果 | 备注 |
|---------|------|------|
| ... | ✅ | ... |

## 4. 性能数据
- 平均执行时间: ... μs
- 最大执行时间: ... μs
- 内存使用: ... MB

## 5. 问题和建议
- ...
```

---

**文档结束**

*本开发路线图将根据项目进展持续更新和调整。*
