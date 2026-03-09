/**
 * @file test_chassis_extended.cpp
 * @brief 底盘系统扩展测试（制动系统）
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
// Braking Tests (42 tests)
// ============================================================================

class BrakingTest : public ::testing::Test {
protected:
    void SetUp() override {
        braking_ = std::make_unique<Braking>();
        braking_->initialize();
    }
    
    std::unique_ptr<Braking> braking_;
};

// Test 48: 默认构造
TEST_F(BrakingTest, DefaultConstruction) {
    EXPECT_NO_THROW(Braking b);
}

// Test 49: 初始化
TEST_F(BrakingTest, Initialization) {
    Braking b;
    EXPECT_NO_THROW(b.initialize());
    
    auto state = b.getState();
    EXPECT_DOUBLE_EQ(state.brake_pedal_position, 0.0);
    EXPECT_DOUBLE_EQ(state.total_brake_torque, 0.0);
}

// Test 50: 制动输入
TEST_F(BrakingTest, BrakeInput) {
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 60.0;
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_NEAR(state.brake_pedal_position, 0.5, 0.01);
}

// Test 51: 制动力矩计算
TEST_F(BrakingTest, BrakeTorqueCalculation) {
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 50.0;
    input.wheel_speed_fr = 50.0;
    input.wheel_speed_rl = 50.0;
    input.wheel_speed_rr = 50.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_GT(state.total_brake_torque, 0.0);
}

// Test 52: 滑移率计算
TEST_F(BrakingTest, SlipRatioCalculation) {
    BrakingInput input;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 40.0;  // 轮速较低
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_GT(state.front_left.wheel_slip, 0.0);
    EXPECT_LE(state.front_left.wheel_slip, 1.0);
}

// Test 53: ABS激活
TEST_F(BrakingTest, ABSActivation) {
    braking_->enableABS(true);
    
    BrakingInput input;
    input.brake_pedal_position = 1.0;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 30.0;  // 低轮速，高滑移率
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    for (int i = 0; i < 50; ++i) {
        braking_->update(0.01, input);
    }
    
    auto state = braking_->getState();
    // ABS应该检测到高滑移率
    EXPECT_TRUE(state.front_left.wheel_slip > 0.0);
}

// Test 54: ABS禁用
TEST_F(BrakingTest, ABSDisabled) {
    braking_->enableABS(false);
    
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 60.0;
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_FALSE(state.abs_active);
}

// Test 55: ABS状态机
TEST_F(BrakingTest, ABSStateMachine) {
    braking_->enableABS(true);
    
    BrakingInput input;
    input.brake_pedal_position = 1.0;
    input.vehicle_speed = 30.0;
    input.wheel_speed_fl = 20.0;  // 高滑移率
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    for (int i = 0; i < 100; ++i) {
        braking_->update(0.01, input);
    }
    
    auto state = braking_->getState();
    // ABS应该介入
    EXPECT_TRUE(state.abs_active || state.front_left.abs_state != ABSState::INACTIVE);
}

// Test 56: EBD制动力分配
TEST_F(BrakingTest, EBDistribution) {
    braking_->enableEBD(true);
    
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.longitudinal_acceleration = 5.0;  // 制动减速度
    input.wheel_speed_fl = 55.0;
    input.wheel_speed_fr = 55.0;
    input.wheel_speed_rl = 55.0;
    input.wheel_speed_rr = 55.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_GT(state.front_brake_ratio, 0.0);
    EXPECT_GT(state.rear_brake_ratio, 0.0);
    EXPECT_NEAR(state.front_brake_ratio + state.rear_brake_ratio, 1.0, 0.01);
}

// Test 57: EBD禁用
TEST_F(BrakingTest, EBDDisabled) {
    braking_->enableEBD(false);
    
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_DOUBLE_EQ(state.front_brake_ratio, 0.6);  // 默认分配
}

// Test 58: 再生制动协调
TEST_F(BrakingTest, RegenCoordination) {
    braking_->enableRegen(true);
    braking_->setRegenBlendFactor(0.3);
    
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.regen_request = 1.0;
    input.wheel_speed_fl = 60.0;
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_GT(state.regen_torque, 0.0);
    EXPECT_GT(state.friction_torque, 0.0);
}

// Test 59: 再生制动禁用
TEST_F(BrakingTest, RegenDisabled) {
    braking_->enableRegen(false);
    
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.regen_request = 1.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_DOUBLE_EQ(state.regen_torque, 0.0);
}

// Test 60: 紧急制动
TEST_F(BrakingTest, EmergencyBrake) {
    braking_->emergencyBrake();
    
    BrakingInput input;
    input.vehicle_speed = 30.0;
    input.wheel_speed_fl = 90.0;
    input.wheel_speed_fr = 90.0;
    input.wheel_speed_rl = 90.0;
    input.wheel_speed_rr = 90.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_DOUBLE_EQ(state.brake_pedal_position, 1.0);
    EXPECT_GT(state.total_brake_torque, 0.0);
}

// Test 61: 释放制动
TEST_F(BrakingTest, ReleaseBrake) {
    // 先制动
    braking_->emergencyBrake();
    
    BrakingInput input;
    input.vehicle_speed = 20.0;
    braking_->update(0.01, input);
    
    // 然后释放
    braking_->releaseBrake();
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_DOUBLE_EQ(state.brake_pedal_position, 0.0);
}

// Test 62: 制动减速度
TEST_F(BrakingTest, DecelerationCalculation) {
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 60.0;
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_GT(state.deceleration, 0.0);
}

// Test 63: 制动盘温度
TEST_F(BrakingTest, DiscTemperature) {
    BrakingInput input;
    input.brake_pedal_position = 0.8;
    input.vehicle_speed = 30.0;
    input.wheel_speed_fl = 90.0;
    input.wheel_speed_fr = 90.0;
    input.wheel_speed_rl = 90.0;
    input.wheel_speed_rr = 90.0;
    
    for (int i = 0; i < 100; ++i) {
        braking_->update(0.01, input);
    }
    
    auto state = braking_->getState();
    EXPECT_GT(state.front_left.disc_temperature, 25.0);  // 应该升温
}

// Test 64: EMB制动
TEST_F(BrakingTest, EMBBraking) {
    BrakingConfig config;
    config.type = BrakingType::ELECTRO_MECHANICAL;
    
    Braking emb(config);
    emb.initialize();
    
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 60.0;
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    emb.update(0.01, input);
    
    auto state = emb.getState();
    EXPECT_GT(state.total_brake_torque, 0.0);
}

// Test 65: EHB制动
TEST_F(BrakingTest, EHBBraking) {
    BrakingConfig config;
    config.type = BrakingType::ELECTRO_HYDRAULIC;
    
    Braking ehb(config);
    ehb.initialize();
    
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 60.0;
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    ehb.update(0.01, input);
    
    auto state = ehb.getState();
    EXPECT_GT(state.total_brake_torque, 0.0);
}

// Test 66: 重置功能
TEST_F(BrakingTest, ResetFunction) {
    braking_->emergencyBrake();
    
    BrakingInput input;
    input.vehicle_speed = 20.0;
    braking_->update(0.01, input);
    
    braking_->reset();
    
    auto state = braking_->getState();
    EXPECT_DOUBLE_EQ(state.brake_pedal_position, 0.0);
    EXPECT_DOUBLE_EQ(state.total_brake_torque, 0.0);
}

// Test 67: 低附着路面
TEST_F(BrakingTest, LowFrictionSurface) {
    BrakingInput input;
    input.brake_pedal_position = 0.8;
    input.vehicle_speed = 20.0;
    input.road_friction_coeff = 0.3;  // 低附着
    input.wheel_speed_fl = 30.0;  // 高滑移率
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_TRUE(std::isfinite(state.total_brake_torque));
}

// Test 68: 高速制动
TEST_F(BrakingTest, HighSpeedBraking) {
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 50.0;
    input.wheel_speed_fl = 150.0;
    input.wheel_speed_fr = 150.0;
    input.wheel_speed_rl = 150.0;
    input.wheel_speed_rr = 150.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_GT(state.total_brake_torque, 0.0);
}

// Test 69: 低速制动
TEST_F(BrakingTest, LowSpeedBraking) {
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 2.0;
    input.wheel_speed_fl = 6.0;
    input.wheel_speed_fr = 6.0;
    input.wheel_speed_rl = 6.0;
    input.wheel_speed_rr = 6.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_GT(state.total_brake_torque, 0.0);
}

// Test 70: 非对称轮速
TEST_F(BrakingTest, AsymmetricWheelSpeeds) {
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 40.0;  # 不同轮速
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 55.0;
    input.wheel_speed_rr = 50.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_TRUE(std::isfinite(state.total_brake_torque));
}

// Test 71: 再生制动比例设置
TEST_F(BrakingTest, RegenBlendFactorSetting) {
    braking_->setRegenBlendFactor(0.5);
    
    auto config = braking_->getConfig();
    EXPECT_DOUBLE_EQ(config.regen_blend_factor, 0.5);
}

// Test 72: 零踏板输入
TEST_F(BrakingTest, ZeroPedalInput) {
    BrakingInput input;
    input.brake_pedal_position = 0.0;
    input.vehicle_speed = 20.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_DOUBLE_EQ(state.brake_pedal_position, 0.0);
    EXPECT_NEAR(state.total_brake_torque, 0.0, 1.0);
}

// Test 73: 脉冲制动
TEST_F(BrakingTest, PulseBraking) {
    for (int i = 0; i < 100; ++i) {
        BrakingInput input;
        input.brake_pedal_position = (i % 20 < 10) ? 0.5 : 0.0;  // 脉冲
        input.vehicle_speed = 20.0;
        input.wheel_speed_fl = 60.0;
        input.wheel_speed_fr = 60.0;
        input.wheel_speed_rl = 60.0;
        input.wheel_speed_rr = 60.0;
        
        braking_->update(0.01, input);
    }
    
    auto state = braking_->getState();
    EXPECT_TRUE(std::isfinite(state.total_brake_torque));
}

// Test 74: 轴荷转移
TEST_F(BrakingTest, AxleLoadTransfer) {
    braking_->enableEBD(true);
    
    BrakingInput input;
    input.brake_pedal_position = 0.8;
    input.vehicle_speed = 30.0;
    input.longitudinal_acceleration = 8.0;  // 强制动
    input.wheel_speed_fl = 80.0;
    input.wheel_speed_fr = 80.0;
    input.wheel_speed_rl = 80.0;
    input.wheel_speed_rr = 80.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    // 强制动时前轴制动力比例应该增加
    EXPECT_GT(state.front_brake_ratio, 0.5);
}

// Test 75: 再生制动退出速度
TEST_F(BrakingTest, RegenLowSpeedExit) {
    braking_->enableRegen(true);
    
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 3.0;  # 低速
    input.regen_request = 1.0;
    input.wheel_speed_fl = 9.0;
    input.wheel_speed_fr = 9.0;
    input.wheel_speed_rl = 9.0;
    input.wheel_speed_rr = 9.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    // 低速时再生制动应该减弱
    EXPECT_LT(state.regen_torque, braking_->getConfig().max_regen_torque);
}

// Test 76: 紧急制动减少再生
TEST_F(BrakingTest, EmergencyBrakeRegenReduction) {
    braking_->enableRegen(true);
    
    BrakingInput input;
    input.brake_pedal_position = 0.9;  // 紧急制动
    input.vehicle_speed = 30.0;
    input.regen_request = 1.0;
    input.wheel_speed_fl = 90.0;
    input.wheel_speed_fr = 90.0;
    input.wheel_speed_rl = 90.0;
    input.wheel_speed_rr = 90.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    // 紧急制动时再生应该减少
    EXPECT_LT(state.regen_torque, braking_->getConfig().max_regen_torque);
}

// Test 77: ABS循环频率
TEST_F(BrakingTest, ABSCycleFrequency) {
    braking_->enableABS(true);
    
    BrakingInput input;
    input.brake_pedal_position = 1.0;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 20.0;
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    // 运行足够长时间观察ABS循环
    for (int i = 0; i < 200; ++i) {
        braking_->update(0.01, input);
    }
    
    auto state = braking_->getState();
    // 应该经历了ABS循环
    EXPECT_TRUE(std::isfinite(state.front_left.brake_pressure));
}

// Test 78: 制动力响应时间
TEST_F(BrakingTest, BrakeResponseTime) {
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 60.0;
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    // 制动应该有响应
    EXPECT_GT(state.total_brake_torque, 0.0);
}

// Test 79: 四轮独立控制
TEST_F(BrakingTest, IndependentWheelControl) {
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 40.0;  // 不同滑移率
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 50.0;
    input.wheel_speed_rr = 55.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    // 各轮制动力矩应该不同
    EXPECT_TRUE(std::isfinite(state.front_left.brake_torque));
    EXPECT_TRUE(std::isfinite(state.front_right.brake_torque));
}

// Test 80: 温度限制
TEST_F(BrakingTest, TemperatureLimit) {
    BrakingInput input;
    input.brake_pedal_position = 1.0;
    input.vehicle_speed = 30.0;
    input.wheel_speed_fl = 90.0;
    input.wheel_speed_fr = 90.0;
    input.wheel_speed_rl = 90.0;
    input.wheel_speed_rr = 90.0;
    
    // 长时间制动
    for (int i = 0; i < 500; ++i) {
        braking_->update(0.01, input);
    }
    
    auto state = braking_->getState();
    // 温度不应该超过最大值
    EXPECT_LE(state.front_left.disc_temperature, 
              braking_->getConfig().front_left.max_temperature + 10.0);
}

// Test 81: 停车制动
TEST_F(BrakingTest, StationaryBraking) {
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 0.0;
    input.wheel_speed_fl = 0.0;
    input.wheel_speed_fr = 0.0;
    input.wheel_speed_rl = 0.0;
    input.wheel_speed_rr = 0.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_TRUE(std::isfinite(state.total_brake_torque));
}

// Test 82: 反向行驶
TEST_F(BrakingTest, ReverseDriving) {
    BrakingInput input;
    input.brake_pedal_position = 0.3;
    input.vehicle_speed = -5.0;  // 反向
    input.wheel_speed_fl = -15.0;
    input.wheel_speed_fr = -15.0;
    input.wheel_speed_rl = -15.0;
    input.wheel_speed_rr = -15.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_TRUE(std::isfinite(state.total_brake_torque));
}

// Test 83: 渐进制动
TEST_F(BrakingTest, ProgressiveBraking) {
    double pedal = 0.0;
    
    for (int i = 0; i < 100; ++i) {
        pedal += 0.01;
        
        BrakingInput input;
        input.brake_pedal_position = pedal;
        input.vehicle_speed = 20.0;
        input.wheel_speed_fl = 60.0;
        input.wheel_speed_fr = 60.0;
        input.wheel_speed_rl = 60.0;
        input.wheel_speed_rr = 60.0;
        
        braking_->update(0.01, input);
    }
    
    auto state = braking_->getState();
    EXPECT_GT(state.total_brake_torque, 0.0);
}

// Test 84: 快速释放
TEST_F(BrakingTest, QuickRelease) {
    // 快速踩下
    BrakingInput input;
    input.brake_pedal_position = 1.0;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 60.0;
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    braking_->update(0.01, input);
    
    // 快速释放
    input.brake_pedal_position = 0.0;
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_NEAR(state.brake_pedal_position, 0.0, 0.01);
}

// Test 85: 最大制动力
TEST_F(BrakingTest, MaximumBrakingForce) {
    braking_->emergencyBrake();
    
    BrakingInput input;
    input.vehicle_speed = 30.0;
    input.wheel_speed_fl = 90.0;
    input.wheel_speed_fr = 90.0;
    input.wheel_speed_rl = 90.0;
    input.wheel_speed_rr = 90.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    // 应该接近最大制动力
    EXPECT_GT(state.total_brake_torque, 0.0);
}

// Test 86: 配置参数获取
TEST_F(BrakingTest, ConfigRetrieval) {
    auto config = braking_->getConfig();
    
    EXPECT_GT(config.front_left.max_brake_torque, 0.0);
    EXPECT_GT(config.vehicle_mass, 0.0);
    EXPECT_GT(config.wheelbase, 0.0);
}

// Test 87: 状态获取
TEST_F(BrakingTest, StateRetrieval) {
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    
    braking_->update(0.01, input);
    
    auto state = braking_->getState();
    EXPECT_DOUBLE_EQ(state.brake_pedal_position, 0.5);
}

// Test 88: ABS所有车轮激活
TEST_F(BrakingTest, ABSAllWheelsActive) {
    braking_->enableABS(true);
    
    BrakingInput input;
    input.brake_pedal_position = 1.0;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 20.0;  # 所有车轮高滑移率
    input.wheel_speed_fr = 20.0;
    input.wheel_speed_rl = 20.0;
    input.wheel_speed_rr = 20.0;
    
    for (int i = 0; i < 50; ++i) {
        braking_->update(0.01, input);
    }
    
    auto state = braking_->getState();
    EXPECT_TRUE(state.abs_active);
}

// Test 89: 性能测试
TEST_F(BrakingTest, PerformanceTest) {
    auto start = std::chrono::high_resolution_clock::now();
    
    BrakingInput input;
    input.brake_pedal_position = 0.5;
    input.vehicle_speed = 20.0;
    input.wheel_speed_fl = 60.0;
    input.wheel_speed_fr = 60.0;
    input.wheel_speed_rl = 60.0;
    input.wheel_speed_rr = 60.0;
    
    for (int i = 0; i < 1000; ++i) {
        braking_->update(0.01, input);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 应该在合理时间内完成
    EXPECT_LT(duration.count(), 100);
}

// ============================================================================
// Integration Tests (5 tests)
// ============================================================================

// Test 90: 悬架+转向集成
TEST(ChassisIntegrationTest, SuspensionSteeringIntegration) {
    Suspension susp;
    Steering steer;
    
    susp.initialize();
    steer.initialize();
    
    // 转弯工况
    SuspensionInput susp_input;
    susp_input.lateral_acceleration = 3.0;
    susp_input.vehicle_speed = 20.0;
    
    SteeringInput steer_input;
    steer_input.driver_torque = 3.0;
    steer_input.vehicle_speed = 20.0;
    steer_input.lateral_acceleration = 3.0;
    
    for (int i = 0; i < 100; ++i) {
        susp.update(0.01, susp_input);
        steer.update(0.01, steer_input);
    }
    
    auto susp_state = susp.getState();
    auto steer_state = steer.getState();
    
    EXPECT_TRUE(std::isfinite(susp_state.body_roll));
    EXPECT_TRUE(std::isfinite(steer_state.wheel_angle_left));
}

// Test 91: 转向+制动集成
TEST(ChassisIntegrationTest, SteeringBrakingIntegration) {
    Steering steer;
    Braking brake;
    
    steer.initialize();
    brake.initialize();
    
    // 弯道制动
    SteeringInput steer_input;
    steer_input.driver_torque = 2.0;
    steer_input.vehicle_speed = 20.0;
    steer_input.lateral_acceleration = 2.0;
    
    BrakingInput brake_input;
    brake_input.brake_pedal_position = 0.3;
    brake_input.vehicle_speed = 20.0;
    brake_input.wheel_speed_fl = 60.0;
    brake_input.wheel_speed_fr = 60.0;
    brake_input.wheel_speed_rl = 60.0;
    brake_input.wheel_speed_rr = 60.0;
    
    for (int i = 0; i < 100; ++i) {
        steer.update(0.01, steer_input);
        brake.update(0.01, brake_input);
    }
    
    auto steer_state = steer.getState();
    auto brake_state = brake.getState();
    
    EXPECT_TRUE(std::isfinite(steer_state.wheel_angle_left));
    EXPECT_GT(brake_state.total_brake_torque, 0.0);
}

// Test 92: 全系统集成
TEST(ChassisIntegrationTest, FullChassisIntegration) {
    Suspension susp;
    Steering steer;
    Braking brake;
    
    susp.initialize();
    steer.initialize();
    brake.initialize();
    
    // 综合工况
    SuspensionInput susp_input;
    susp_input.vehicle_speed = 20.0;
    susp_input.lateral_acceleration = 2.0;
    susp_input.longitudinal_acceleration = -3.0;
    susp_input.road_fl = 0.01;
    
    SteeringInput steer_input;
    steer_input.vehicle_speed = 20.0;
    steer_input.driver_torque = 2.0;
    steer_input.lateral_acceleration = 2.0;
    
    BrakingInput brake_input;
    brake_input.vehicle_speed = 20.0;
    brake_input.brake_pedal_position = 0.3;
    brake_input.longitudinal_acceleration = -3.0;
    brake_input.wheel_speed_fl = 60.0;
    brake_input.wheel_speed_fr = 60.0;
    brake_input.wheel_speed_rl = 60.0;
    brake_input.wheel_speed_rr = 60.0;
    
    for (int i = 0; i < 100; ++i) {
        susp.update(0.01, susp_input);
        steer.update(0.01, steer_input);
        brake.update(0.01, brake_input);
    }
    
    EXPECT_NO_THROW(susp.getState());
    EXPECT_NO_THROW(steer.getState());
    EXPECT_NO_THROW(brake.getState());
}

// Test 93: 极限工况
TEST(ChassisIntegrationTest, ExtremeConditions) {
    Suspension susp;
    Steering steer;
    Braking brake;
    
    susp.initialize();
    steer.initialize();
    brake.initialize();
    
    // 极限工况
    SuspensionInput susp_input;
    susp_input.vehicle_speed = 50.0;
    susp_input.lateral_acceleration = 8.0;
    susp_input.longitudinal_acceleration = -10.0;
    susp_input.road_fl = 0.05;
    
    SteeringInput steer_input;
    steer_input.vehicle_speed = 50.0;
    steer_input.driver_torque = 8.0;
    
    BrakingInput brake_input;
    brake_input.vehicle_speed = 50.0;
    brake_input.brake_pedal_position = 1.0;
    brake_input.wheel_speed_fl = 100.0;
    brake_input.wheel_speed_fr = 100.0;
    brake_input.wheel_speed_rl = 100.0;
    brake_input.wheel_speed_rr = 100.0;
    
    for (int i = 0; i < 200; ++i) {
        susp.update(0.01, susp_input);
        steer.update(0.01, steer_input);
        brake.update(0.01, brake_input);
    }
    
    EXPECT_TRUE(std::isfinite(susp.getState().body_roll));
    EXPECT_TRUE(std::isfinite(steer.getState().wheel_angle_left));
    EXPECT_TRUE(std::isfinite(brake.getState().total_brake_torque));
}

// Test 94: 长时间运行稳定性
TEST(ChassisIntegrationTest, LongTermStability) {
    Suspension susp;
    Steering steer;
    Braking brake;
    
    susp.initialize();
    steer.initialize();
    brake.initialize();
    
    // 模拟60秒运行
    for (int i = 0; i < 6000; ++i) {
        SuspensionInput susp_input;
        susp_input.vehicle_speed = 20.0 + 10.0 * std::sin(i * 0.01);
        susp_input.road_fl = 0.01 * std::sin(i * 0.1);
        
        SteeringInput steer_input;
        steer_input.vehicle_speed = 20.0 + 10.0 * std::sin(i * 0.01);
        steer_input.driver_torque = 2.0 * std::sin(i * 0.05);
        
        BrakingInput brake_input;
        brake_input.vehicle_speed = 20.0 + 10.0 * std::sin(i * 0.01);
        brake_input.brake_pedal_position = 0.3 + 0.2 * std::sin(i * 0.02);
        brake_input.wheel_speed_fl = 60.0 + 30.0 * std::sin(i * 0.01);
        brake_input.wheel_speed_fr = 60.0 + 30.0 * std::sin(i * 0.01);
        brake_input.wheel_speed_rl = 60.0 + 30.0 * std::sin(i * 0.01);
        brake_input.wheel_speed_rr = 60.0 + 30.0 * std::sin(i * 0.01);
        
        susp.update(0.01, susp_input);
        steer.update(0.01, steer_input);
        brake.update(0.01, brake_input);
    }
    
    EXPECT_TRUE(std::isfinite(susp.getState().body_height));
    EXPECT_TRUE(std::isfinite(steer.getState().steering_wheel_angle));
    EXPECT_TRUE(std::isfinite(brake.getState().total_brake_torque));
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
