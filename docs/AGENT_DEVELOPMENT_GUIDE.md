# Agent开发指南

本文档为参与EV Dynamics Simulation项目的5个Agent提供开发指南。

## 角色分配

### Agent 1: Powertrain Specialist (动力系统专家)
**负责模块**: `src/powertrain/`

**任务**:
- 完善电机模型（考虑温度、效率MAP）
- 实现电池等效电路模型（Thevenin/PNGV）
- 添加变速箱动力学

**文件**:
- `src/powertrain/motor/motor_model.cpp`
- `src/powertrain/battery/battery_model.cpp`
- `src/powertrain/transmission/transmission_model.cpp`

### Agent 2: Chassis Specialist (底盘系统专家)
**负责模块**: `src/chassis/`

**任务**:
- 实现多体悬架动力学
- 添加转向系统动力学（Ackerman几何）
- 完善制动系统（EBD、ABS逻辑）

**文件**:
- `src/chassis/suspension/suspension_model.cpp`
- `src/chassis/steering/steering_model.cpp`
- `src/chassis/braking/braking_model.cpp`

### Agent 3: Dynamics Specialist (车辆动力学专家)
**负责模块**: `src/dynamics/`

**任务**:
- 实现完整的车辆动力学模型（14自由度）
- 优化RK4积分器（自适应步长）
- 添加状态空间模型库

**文件**:
- `src/dynamics/vehicle_dynamics/vehicle_model.cpp`
- `src/dynamics/rk4_integrator/rk4.cpp`
- `src/dynamics/state_space/state_space.cpp`

### Agent 4: Tire Specialist (轮胎模型专家)
**负责模块**: `src/tire/`

**任务**:
- 实现完整的Pacejka魔术公式（MF6.2）
- 添加轮胎松弛长度模型
- 实现滑移率计算器

**文件**:
- `src/tire/pacejka/pacejka_model.cpp`
- `src/tire/slip_calculator/slip_calc.cpp`

### Agent 5: Scheduler Specialist (调度系统专家)
**负责模块**: `src/scheduler/`

**任务**:
- 实现实时调度器（POSIX RT）
- 添加任务管理器（优先级队列）
- 实现性能监控（CPU、内存、延迟）

**文件**:
- `src/scheduler/realtime_scheduler/scheduler.cpp`
- `src/scheduler/task_manager/task_mgr.cpp`
- `src/scheduler/monitor/perf_monitor.cpp`

## 开发流程

### 1. 环境准备
```bash
# 每个Agent克隆项目后
cd E:\workspace\ev_dynamics_simulation
./build.sh --debug  # 或 build.bat --debug (Windows)
```

### 2. 创建功能分支
```bash
git checkout -b feature/<module-name>-<feature-description>
# 例如: feature/powertrain-motor-thermal-model
```

### 3. 开发循环
```bash
# 修改代码
vim src/powertrain/motor/motor_model.cpp

# 增量构建
./build.sh

# 运行测试
cd build
ctest -R test_motor --output-on-failure

# 提交代码
git add .
git commit -m "feat(powertrain): add motor thermal model"
```

### 4. 代码审查
- 推送到远程分支
- 创建Pull Request
- 等待CI通过和其他Agent审查

## 代码规范

### C++ 代码规范
```cpp
// 命名空间: 小写，下划线分隔
namespace vehicle_dynamics {

// 类名: 大驼峰
class VehicleModel {
public:
    // 构造函数
    VehicleModel(double mass, double inertia);
    
    // 公共方法: 小驼峰
    void update(double dt);
    double getSpeed() const;
    
private:
    // 成员变量: 小驼峰，后缀下划线
    double mass_;
    double inertia_;
    Eigen::VectorXd state_;
};

// 常量: 全大写，下划线分隔
constexpr double GRAVITY = 9.81;

// 枚举: 大驼峰
enum class VehicleType {
    Sedan,
    SUV,
    Truck
};

} // namespace vehicle_dynamics
```

### 文档注释
```cpp
/**
 * @brief 更新车辆状态
 * 
 * 使用RK4积分器更新车辆动力学状态
 * 
 * @param dt 时间步长（秒）
 * @param steering_angle 前轮转角（弧度）
 * @param traction_force 牵引力（牛顿）
 * 
 * @return 更新后的状态向量
 * 
 * @throws std::invalid_argument 如果dt <= 0
 */
Eigen::VectorXd update(double dt, double steering_angle, double traction_force);
```

## 测试要求

### 单元测试
每个模块必须包含单元测试：

```cpp
// tests/test_motor.cpp
#include <gtest/gtest.h>
#include "powertrain/motor/motor_model.h"

TEST(MotorTest, TorqueLimit) {
    Motor motor(300.0, 15000.0);
    motor.setTorqueRequest(500.0);  // 超过最大扭矩
    EXPECT_NEAR(motor.getTorque(), 300.0, 1e-6);
}

TEST(MotorTest, ThermalResponse) {
    Motor motor(300.0, 15000.0);
    for (int i = 0; i < 1000; i++) {
        motor.update(0.001);
    }
    EXPECT_GT(motor.getTemperature(), 25.0);
}
```

### Python测试
```python
# tests/python/test_motor.py
import pytest
import powertrain

def test_motor_torque_limit():
    motor = powertrain.Motor(300.0, 15000.0)
    motor.set_torque_request(500.0)
    assert abs(motor.get_torque() - 300.0) < 1e-6
```

## 性能指标

### 计算性能
- **仿真频率**: 500 Hz（实时）
- **单步耗时**: < 2 ms
- **内存占用**: < 100 MB

### 代码质量
- **测试覆盖率**: > 90%
- **静态分析**: 0 warnings
- **代码格式**: clang-format检查通过

## 集成接口

### Python API
```python
# 所有模块必须遵循这个接口模式
import <module_name>

# 创建对象
obj = <module_name>.ClassName(param1, param2)

# 设置输入
obj.set_input(value)

# 更新状态
obj.update(dt)

# 获取输出
output = obj.get_output()
```

### 配置文件
```yaml
# config/module_config.yaml
module_name:
  param1: value1
  param2: value2
```

## 常见问题

### Q1: 如何调试Python绑定？
```bash
# 设置环境变量
export PYTHONPATH=$PYTHONPATH:$(pwd)/build/python_modules

# 启用调试输出
export PYBIND11_DEBUG=1

# 运行Python
python -c "import powertrain; print(dir(powertrain))"
```

### Q2: 如何处理依赖冲突？
```bash
# 使用虚拟环境
python -m venv venv
source venv/bin/activate  # Linux
.\venv\Scripts\activate   # Windows

# 重新安装依赖
pip install pybind11 pytest
```

### Q3: 如何优化构建速度？
```bash
# 使用ccache
sudo apt-get install ccache
cmake .. -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

# 并行编译
make -j$(nproc)
```

## 联系与协作

- **每日站会**: 每天早上10:00
- **代码审查**: Pull Request创建后24小时内
- **问题讨论**: GitHub Issues
- **紧急问题**: 直接联系项目维护者

## 里程碑

### Week 1 (3月6日 - 3月12日)
- [ ] 完成开发环境搭建
- [ ] 实现基础模型框架
- [ ] 编写单元测试

### Week 2 (3月13日 - 3月19日)
- [ ] 完善核心算法
- [ ] 集成测试
- [ ] 性能优化

### Week 3 (3月20日 - 3月26日)
- [ ] 模块集成
- [ ] 端到端测试
- [ ] 文档完善

### Week 4 (3月27日 - 4月2日)
- [ ] 代码审查
- [ ] 性能调优
- [ ] 发布准备

---

**最后更新**: 2025-03-06
**维护者**: DevOpsAgent
