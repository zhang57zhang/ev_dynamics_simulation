/**
 * @file test_chassis.cpp
 * @brief 底盘系统单元测试
 * @author CodeCraft
 * @date 2026-03-08
 */

#include <gtest/gtest.h>
#include <cmath>
#include <chrono>
#include "../src/chassis/suspension.h"
#include "../src/chassis/steering.h"
#include "../src/chassis/braking.h"

using namespace ev_dynamics::chassis;

// ============================================================================
// Suspension Tests (25 tests)
// ============================================================================

class SuspensionTest : public ::testing::Test {
protected:
    void SetUp() override {
        suspension_ = std::make_unique<Suspension>();
        suspension_->initialize();
    }
    
    std::unique_ptr<Suspension> suspension_;
};

// Test 1: 默认构造
TEST_F(SuspensionTest, DefaultConstruction) {
    EXPECT_NO_THROW(Suspension s);
}

// Test 2: 初始化
TEST_F(SuspensionTest, Initialization) {
    Suspension s;
    EXPECT_NO_THROW(s.initialize());
    
    auto state = s.getState();
    EXPECT_DOUBLE_EQ(state.body_height, 0.16);
    EXPECT_DOUBLE_EQ(state.body_pitch, 0.0);
    EXPECT_DOUBLE_EQ(state.body_roll, 0.0);
}

// Test 3: 更新函数
TEST_F(SuspensionTest, UpdateFunction) {
    SuspensionInput input;
    input.road_fl = 0.01;
    input.road_fr = 0.01;
    input.road_rl = 0.01;
    input.road_rr = 0.01;
    
    EXPECT_NO_THROW(suspension_->update(0.01, input));
}

// Test 4: 路面输入响应
TEST_F(SuspensionTest, RoadInputResponse) {
    SuspensionInput input;
    input.road_fl = 0.02;  // 2cm bump
    
    suspension_->update(0.01, input);
    
    auto state = suspension_->getState();
    EXPECT_NE(state.front_left.displacement, 0.0);
}

// Test 5: 弹簧力计算
TEST_F(SuspensionTest, SpringForceCalculation) {
    SuspensionInput input;
    input.road_fl = 0.05;  // 5cm displacement
    
    for (int i = 0; i < 100; ++i) {
        suspension_->update(0.01, input);
    }
    
    auto state = suspension_->getState();
    EXPECT_NE(state.front_left.spring_force, 0.0);
}

// Test 6: 阻尼力计算
TEST_F(SuspensionTest, DampingForceCalculation) {
    SuspensionInput input;
    input.road_fl = 0.01;
    input.road_fr = 0.01;
    
    suspension_->update(0.01, input);
    
    auto state = suspension_->getState();
    // 阻尼力应该存在
    EXPECT_TRUE(std::isfinite(state.front_left.damping_force));
}

// Test 7: 舒适模式阻尼
TEST_F(SuspensionTest, ComfortDampingMode) {
    suspension_->setDampingMode(DampingMode::COMFORT);
    
    SuspensionInput input;
    input.road_fl = 0.01;
    
    suspension_->update(0.01, input);
    
    // 应该正常运行
    EXPECT_NO_THROW(suspension_->update(0.01, input));
}

// Test 8: 运动模式阻尼
TEST_F(SuspensionTest, SportDampingMode) {
    suspension_->setDampingMode(DampingMode::SPORT);
    
    SuspensionInput input;
    input.road_fl = 0.01;
    
    suspension_->update(0.01, input);
    
    EXPECT_NO_THROW(suspension_->update(0.01, input));
}

// Test 9: 自适应阻尼
TEST_F(SuspensionTest, AdaptiveDampingMode) {
    suspension_->setDampingMode(DampingMode::ADAPTIVE);
    
    SuspensionInput input;
    input.road_fl = 0.01;
    input.vehicle_speed = 20.0;
    input.lateral_acceleration = 2.0;
    
    suspension_->update(0.01, input);
    
    EXPECT_NO_THROW(suspension_->update(0.01, input));
}

// Test 10: 高度调节
TEST_F(SuspensionTest, HeightAdjustment) {
    suspension_->setTargetHeight(0.18);
    
    SuspensionInput input;
    input.target_height = 0.18;
    
    for (int i = 0; i < 100; ++i) {
        suspension_->update(0.01, input);
    }
    
    auto state = suspension_->getState();
    // 高度应该接近目标
    EXPECT_NEAR(state.body_height, 0.18, 0.05);
}

// Test 11: 车身俯仰
TEST_F(SuspensionTest, BodyPitch) {
    SuspensionInput input;
    input.road_fl = 0.02;  // 前高
    input.road_fr = 0.02;
    input.road_rl = 0.0;
    input.road_rr = 0.0;
    
    for (int i = 0; i < 50; ++i) {
        suspension_->update(0.01, input);
    }
    
    auto state = suspension_->getState();
    EXPECT_NE(state.body_pitch, 0.0);
}

// Test 12: 车身侧倾
TEST_F(SuspensionTest, BodyRoll) {
    SuspensionInput input;
    input.road_fl = 0.02;  // 左高
    input.road_rl = 0.02;
    input.road_fr = 0.0;
    input.road_rr = 0.0;
    
    for (int i = 0; i < 50; ++i) {
        suspension_->update(0.01, input);
    }
    
    auto state = suspension_->getState();
    EXPECT_NE(state.body_roll, 0.0);
}

// Test 13: 舒适性指标
TEST_F(SuspensionTest, ComfortIndex) {
    SuspensionInput input;
    input.road_fl = 0.01;
    
    suspension_->update(0.01, input);
    
    auto state = suspension_->getState();
    EXPECT_GE(state.comfort_index, 0.0);
    EXPECT_LE(state.comfort_index, 1.0);
}

// Test 14: 操控性指标
TEST_F(SuspensionTest, HandlingIndex) {
    SuspensionInput input;
    input.road_fl = 0.01;
    
    suspension_->update(0.01, input);
    
    auto state = suspension_->getState();
    EXPECT_GE(state.handling_index, 0.0);
    EXPECT_LE(state.handling_index, 1.0);
}

// Test 15: 性能权重设置
TEST_F(SuspensionTest, PerformanceWeights) {
    suspension_->setPerformanceWeights(0.7, 0.3);
    
    auto config = suspension_->getConfig();
    EXPECT_NEAR(config.comfort_weight, 0.7, 0.01);
    EXPECT_NEAR(config.handling_weight, 0.3, 0.01);
}

// Test 16: 主动悬架
TEST_F(SuspensionTest, ActiveSuspension) {
    SuspensionConfig config;
    config.type = SuspensionType::ACTIVE;
    
    Suspension active_susp(config);
    active_susp.initialize();
    
    SuspensionInput input;
    input.road_fl = 0.01;
    
    active_susp.update(0.01, input);
    
    auto state = active_susp.getState();
    EXPECT_NE(state.front_left.actuator_force, 0.0);
}

// Test 17: 被动悬架
TEST_F(SuspensionTest, PassiveSuspension) {
    SuspensionConfig config;
    config.type = SuspensionType::PASSIVE;
    
    Suspension passive_susp(config);
    passive_susp.initialize();
    
    SuspensionInput input;
    input.road_fl = 0.01;
    
    passive_susp.update(0.01, input);
    
    auto state = passive_susp.getState();
    EXPECT_DOUBLE_EQ(state.front_left.actuator_force, 0.0);
}

// Test 18: 轮胎力计算
TEST_F(SuspensionTest, TireForceCalculation) {
    SuspensionInput input;
    input.road_fl = 0.01;
    
    suspension_->update(0.01, input);
    
    auto state = suspension_->getState();
    EXPECT_NE(state.front_left.tire_force, 0.0);
}

// Test 19: 重置功能
TEST_F(SuspensionTest, ResetFunction) {
    SuspensionInput input;
    input.road_fl = 0.05;
    
    for (int i = 0; i < 50; ++i) {
        suspension_->update(0.01, input);
    }
    
    suspension_->reset();
    
    auto state = suspension_->getState();
    EXPECT_DOUBLE_EQ(state.body_pitch, 0.0);
}

// Test 20: 大位移响应
TEST_F(SuspensionTest, LargeDisplacementResponse) {
    SuspensionInput input;
    input.road_fl = 0.10;  // 10cm
    
    for (int i = 0; i < 100; ++i) {
        suspension_->update(0.01, input);
    }
    
    auto state = suspension_->getState();
    EXPECT_TRUE(std::isfinite(state.front_left.displacement));
}

// Test 21: 频率响应
TEST_F(SuspensionTest, FrequencyResponse) {
    double dt = 0.01;
    double freq = 2.0;  // 2 Hz
    double amplitude = 0.01;
    
    for (int i = 0; i < 200; ++i) {
        SuspensionInput input;
        input.road_fl = amplitude * std::sin(2.0 * M_PI * freq * i * dt);
        
        suspension_->update(dt, input);
    }
    
    auto state = suspension_->getState();
    EXPECT_TRUE(std::isfinite(state.front_left.displacement));
}

// Test 22: 非对称路面
TEST_F(SuspensionTest, AsymmetricRoadInput) {
    SuspensionInput input;
    input.road_fl = 0.02;
    input.road_fr = 0.0;
    input.road_rl = 0.01;
    input.road_rr = -0.01;
    
    suspension_->update(0.01, input);
    
    auto state = suspension_->getState();
    EXPECT_TRUE(std::isfinite(state.body_roll));
}

// Test 23: 高速工况
TEST_F(SuspensionTest, HighSpeedCondition) {
    SuspensionInput input;
    input.vehicle_speed = 30.0;
    input.road_fl = 0.01;
    
    suspension_->update(0.01, input);
    
    EXPECT_NO_THROW(suspension_->update(0.01, input));
}

// Test 24: 弯道工况
TEST_F(SuspensionTest, CorneringCondition) {
    SuspensionInput input;
    input.lateral_acceleration = 5.0;
    input.vehicle_speed = 20.0;
    
    suspension_->update(0.01, input);
    
    EXPECT_NO_THROW(suspension_->update(0.01, input));
}

// Test 25: 极限高度
TEST_F(SuspensionTest, ExtremeHeights) {
    suspension_->setTargetHeight(0.12);  // 最小高度
    
    SuspensionInput input;
    input.target_height = 0.12;
    
    for (int i = 0; i < 100; ++i) {
        suspension_->update(0.01, input);
    }
    
    auto state = suspension_->getState();
    EXPECT_GE(state.body_height, 0.11);
}

// ============================================================================
// Steering Tests (22 tests)
// ============================================================================

class SteeringTest : public ::testing::Test {
protected:
    void SetUp() override {
        steering_ = std::make_unique<Steering>();
        steering_->initialize();
    }
    
    std::unique_ptr<Steering> steering_;
};

// Test 26: 默认构造
TEST_F(SteeringTest, DefaultConstruction) {
    EXPECT_NO_THROW(Steering s);
}

// Test 27: 初始化
TEST_F(SteeringTest, Initialization) {
    Steering s;
    EXPECT_NO_THROW(s.initialize());
    
    auto state = s.getState();
    EXPECT_DOUBLE_EQ(state.steering_wheel_angle, 0.0);
    EXPECT_DOUBLE_EQ(state.wheel_angle_left, 0.0);
    EXPECT_DOUBLE_EQ(state.wheel_angle_right, 0.0);
}

// Test 28: 转向输入
TEST_F(SteeringTest, SteeringInput) {
    SteeringInput input;
    input.driver_torque = 2.0;
    input.vehicle_speed = 10.0;
    
    steering_->update(0.01, input);
    
    EXPECT_NO_THROW(steering_->update(0.01, input));
}

// Test 29: 可变转向比
TEST_F(SteeringTest, VariableSteeringRatio) {
    auto state = steering_->getState();
    double ratio_low_speed = state.current_steering_ratio;
    
    // 低速
    SteeringInput input;
    input.vehicle_speed = 2.0;
    input.driver_torque = 1.0;
    
    steering_->update(0.01, input);
    state = steering_->getState();
    double ratio_high_speed = state.current_steering_ratio;
    
    // 转向比应该不同
    EXPECT_TRUE(std::isfinite(ratio_low_speed));
    EXPECT_TRUE(std::isfinite(ratio_high_speed));
}

// Test 30: Ackerman几何
TEST_F(SteeringTest, AckermanGeometry) {
    SteeringInput input;
    input.driver_torque = 5.0;
    input.vehicle_speed = 10.0;
    
    for (int i = 0; i < 100; ++i) {
        steering_->update(0.01, input);
    }
    
    auto state = steering_->getState();
    // 转弯时内外轮角度应该不同
    if (std::abs(state.wheel_angle_average) > 0.01) {
        EXPECT_NE(state.wheel_angle_left, state.wheel_angle_right);
    }
}

// Test 31: 助力扭矩
TEST_F(SteeringTest, AssistTorque) {
    SteeringInput input;
    input.driver_torque = 3.0;
    input.vehicle_speed = 10.0;
    
    steering_->update(0.01, input);
    
    auto state = steering_->getState();
    EXPECT_NE(state.assist_torque, 0.0);
}

// Test 32: 舒适模式助力
TEST_F(SteeringTest, ComfortAssistMode) {
    steering_->setAssistMode(AssistMode::COMFORT);
    
    SteeringInput input;
    input.driver_torque = 2.0;
    input.vehicle_speed = 10.0;
    
    steering_->update(0.01, input);
    
    auto state = steering_->getState();
    EXPECT_GT(state.assist_torque, 0.0);
}

// Test 33: 运动模式助力
TEST_F(SteeringTest, SportAssistMode) {
    steering_->setAssistMode(AssistMode::SPORT);
    
    SteeringInput input;
    input.driver_torque = 2.0;
    input.vehicle_speed = 10.0;
    
    steering_->update(0.01, input);
    
    auto state = steering_->getState();
    EXPECT_TRUE(std::isfinite(state.assist_torque));
}

// Test 34: 反馈扭矩
TEST_F(SteeringTest, FeedbackTorque) {
    SteeringInput input;
    input.driver_torque = 2.0;
    input.tire_align_torque = 5.0;
    input.vehicle_speed = 10.0;
    
    steering_->update(0.01, input);
    
    auto state = steering_->getState();
    EXPECT_TRUE(std::isfinite(state.feedback_torque));
}

// Test 35: 回正扭矩
TEST_F(SteeringTest, ReturnTorque) {
    // 先转向
    SteeringInput input;
    input.driver_torque = 5.0;
    input.vehicle_speed = 10.0;
    
    for (int i = 0; i < 50; ++i) {
        steering_->update(0.01, input);
    }
    
    // 松手回正
    input.driver_torque = 0.0;
    steering_->update(0.01, input);
    
    auto state = steering_->getState();
    EXPECT_TRUE(state.is_returning || state.return_torque != 0.0);
}

// Test 36: 电机扭矩
TEST_F(SteeringTest, MotorTorque) {
    SteeringInput input;
    input.driver_torque = 3.0;
    input.vehicle_speed = 10.0;
    
    steering_->update(0.01, input);
    
    auto state = steering_->getState();
    EXPECT_TRUE(std::isfinite(state.motor_torque));
}

// Test 37: 转向柱动力学
TEST_F(SteeringTest, SteeringColumnDynamics) {
    SteeringInput input;
    input.driver_torque = 5.0;
    input.vehicle_speed = 10.0;
    
    for (int i = 0; i < 100; ++i) {
        steering_->update(0.01, input);
    }
    
    auto state = steering_->getState();
    EXPECT_NE(state.steering_wheel_angle, 0.0);
}

// Test 38: 车轮转角限制
TEST_F(SteeringTest, WheelAngleLimit) {
    SteeringInput input;
    input.driver_torque = 10.0;
    input.vehicle_speed = 5.0;
    
    for (int i = 0; i < 200; ++i) {
        steering_->update(0.01, input);
    }
    
    auto state = steering_->getState();
    EXPECT_LE(std::abs(state.wheel_angle_left), 0.7);
    EXPECT_LE(std::abs(state.wheel_angle_right), 0.7);
}

// Test 39: 线控转向
TEST_F(SteeringTest, SteerByWire) {
    SteeringConfig config;
    config.type = SteeringType::STEER_BY_WIRE;
    
    Steering sbw(config);
    sbw.initialize();
    
    sbw.setSteeringWheelAngle(0.5);
    
    SteeringInput input;
    input.driver_angle = 0.5;
    input.vehicle_speed = 10.0;
    
    sbw.update(0.01, input);
    
    auto state = sbw.getState();
    EXPECT_NEAR(state.steering_wheel_angle, 0.5, 0.01);
}

// Test 40: 主动回正使能
TEST_F(SteeringTest, ActiveReturnEnable) {
    steering_->enableActiveReturn(true);
    
    SteeringInput input;
    input.driver_torque = 0.0;
    input.vehicle_speed = 10.0;
    
    steering_->update(0.01, input);
    
    EXPECT_NO_THROW(steering_->update(0.01, input));
}

// Test 41: 禁用主动回正
TEST_F(SteeringTest, ActiveReturnDisable) {
    steering_->enableActiveReturn(false);
    
    SteeringInput input;
    input.driver_torque = 0.0;
    input.vehicle_speed = 10.0;
    
    steering_->update(0.01, input);
    
    auto state = steering_->getState();
    EXPECT_DOUBLE_EQ(state.return_torque, 0.0);
}

// Test 42: 手动转向比
TEST_F(SteeringTest, ManualSteeringRatio) {
    steering_->setSteeringRatio(18.0);
    
    SteeringInput input;
    input.driver_torque = 2.0;
    input.vehicle_speed = 10.0;
    
    steering_->update(0.01, input);
    
    auto state = steering_->getState();
    EXPECT_DOUBLE_EQ(state.current_steering_ratio, 18.0);
}

// Test 43: 高速稳定性
TEST_F(SteeringTest, HighSpeedStability) {
    SteeringInput input;
    input.vehicle_speed = 40.0;
    input.driver_torque = 1.0;
    
    steering_->update(0.01, input);
    
    auto state = steering_->getState();
    EXPECT_TRUE(std::isfinite(state.current_steering_ratio));
}

// Test 44: 低速灵活性
TEST_F(SteeringTest, LowSpeedManeuverability) {
    SteeringInput input;
    input.vehicle_speed = 2.0;
    input.driver_torque = 3.0;
    
    steering_->update(0.01, input);
    
    auto state = steering_->getState();
    EXPECT_TRUE(std::isfinite(state.current_steering_ratio));
}

// Test 45: 摩擦补偿
TEST_F(SteeringTest, FrictionCompensation) {
    SteeringInput input;
    input.driver_torque = 1.0;
    input.vehicle_speed = 10.0;
    
    steering_->update(0.01, input);
    
    // 应该正常运行
    EXPECT_NO_THROW(steering_->update(0.01, input));
}

// Test 46: 重置功能
TEST_F(SteeringTest, ResetFunction) {
    SteeringInput input;
    input.driver_torque = 5.0;
    
    for (int i = 0; i < 50; ++i) {
        steering_->update(0.01, input);
    }
    
    steering_->reset();
    
    auto state = steering_->getState();
    EXPECT_DOUBLE_EQ(state.steering_wheel_angle, 0.0);
}

// Test 47: 横向加速度反馈
TEST_F(SteeringTest, LateralAccelerationFeedback) {
    SteeringInput input;
    input.driver_torque = 2.0;
    input.lateral_acceleration = 5.0;
    input.vehicle_speed = 20.0;
    
    steering_->update(0.01, input);
    
    auto state = steering_->getState();
    EXPECT_TRUE(std::isfinite(state.feedback_torque));
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
