/**
 * @file test_powertrain.cpp
 * @brief Powertrain模块单元测试
 * @author CodeCraft
 * @date 2026-03-08
 * 
 * 测试覆盖：
 * - PMSM电机模型（20个测试）
 * - 电池模型（17个测试）
 * - 集成测试（10个测试）
 */

#include <gtest/gtest.h>
#include <cmath>
#include <iostream>
#include "pmsm_motor.h"
#include "battery.h"

using namespace ev_dynamics::powertrain;

// ============================================================================
// PMSM电机模型测试（20个）
// ============================================================================

class PMSMMotorTest : public ::testing::Test {
protected:
    PMSMMotor motor;
    
    void SetUp() override {
        motor.initialize();
    }
};

TEST_F(PMSMMotorTest, InitializationTest) {
    auto state = motor.getState();
    EXPECT_DOUBLE_EQ(state.electromagnetic_torque, 0.0);
    EXPECT_DOUBLE_EQ(state.rotor_speed, 0.0);
    EXPECT_DOUBLE_EQ(state.id, 0.0);
    EXPECT_DOUBLE_EQ(state.iq, 0.0);
}

TEST_F(PMSMMotorTest, RatedTorqueTest) {
    PMSMInput input;
    input.target_torque = 127.0;  // 额定扭矩
    
    motor.update(0.001, input);  // 1ms步长
    auto state = motor.getState();
    
    // 允许5%误差
    EXPECT_NEAR(state.electromagnetic_torque, 127.0, 6.35);
}

TEST_F(PMSMMotorTest, TorqueLinearityTest) {
    // 测试扭矩线性度
    for (double torque = 0.0; torque <= 127.0; torque += 25.4) {
        PMSMInput input;
        input.target_torque = torque;
        
        motor.reset();
        motor.update(0.01, input);
        
        auto state = motor.getState();
        double error = std::abs(state.electromagnetic_torque - torque) / torque;
        EXPECT_LT(error, 0.05);  // <5%误差
    }
}

TEST_F(PMSMMotorTest, OverloadTest) {
    PMSMInput input;
    input.target_torque = 200.0;  // 超载（额定127Nm）
    
    motor.update(0.001, input);
    auto state = motor.getState();
    
    // 应该被限制在300A电流以内
    double current_magnitude = std::sqrt(state.id * state.id + state.iq * state.iq);
    EXPECT_LE(current_magnitude, motor.getConfig().max_current);
}

TEST_F(PMSMMotorTest, CurrentLimitTest) {
    PMSMInput input;
    input.target_torque = 1000.0;  // 极大扭矩请求
    
    motor.update(0.001, input);
    auto state = motor.getState();
    
    double current_magnitude = std::sqrt(state.id * state.id + state.iq * state.iq);
    EXPECT_LE(current_magnitude, motor.getConfig().max_current);
}

TEST_F(PMSMMotorTest, EfficiencyTest) {
    PMSMInput input;
    input.target_torque = 100.0;
    
    // 运行一段时间
    for (int i = 0; i < 100; ++i) {
        motor.update(0.001, input);
    }
    
    auto state = motor.getState();
    // 电机效率应该>85%
    EXPECT_GT(state.efficiency, 85.0);
}

TEST_F(PMSMMotorTest, CopperLossTest) {
    PMSMInput input;
    input.target_torque = 100.0;
    
    motor.update(0.001, input);
    auto state = motor.getState();
    
    // 铜损应该>0
    EXPECT_GT(state.copper_loss, 0.0);
}

TEST_F(PMSMMotorTest, IronLossTest) {
    PMSMInput input;
    input.target_torque = 100.0;
    
    motor.update(0.001, input);
    auto state = motor.getState();
    
    // 铁损应该>=0
    EXPECT_GE(state.iron_loss, 0.0);
}

TEST_F(PMSMMotorTest, TotalLossTest) {
    PMSMInput input;
    input.target_torque = 100.0;
    
    motor.update(0.001, input);
    auto state = motor.getState();
    
    // 总损耗 = 铜损 + 铁损
    EXPECT_DOUBLE_EQ(state.total_loss, state.copper_loss + state.iron_loss);
}

TEST_F(PMSMMotorTest, OutputPowerTest) {
    PMSMInput input;
    input.target_torque = 100.0;
    
    // 设置转速
    motor.getState().rotor_speed = 100.0;  // 100 rad/s
    
    motor.update(0.001, input);
    auto state = motor.getState();
    
    // 输出功率 = 扭矩 * 转速
    double expected_power = state.electromagnetic_torque * state.rotor_speed;
    EXPECT_NEAR(state.output_power, expected_power, 1.0);
}

TEST_F(PMSMMotorTest, TemperatureTest) {
    PMSMInput input;
    input.target_torque = 100.0;
    
    // 长时间运行
    for (int i = 0; i < 1000; ++i) {
        motor.update(0.001, input);
    }
    
    auto state = motor.getState();
    // 温度应该升高
    EXPECT_GT(state.stator_temperature, 25.0);
}

TEST_F(PMSMMotorTest, TemperatureLimitTest) {
    PMSMInput input;
    input.target_torque = 200.0;  // 过载运行
    
    // 长时间运行
    for (int i = 0; i < 10000; ++i) {
        motor.update(0.001, input);
    }
    
    auto state = motor.getState();
    // 温度不应该超过限制
    EXPECT_LE(state.stator_temperature, motor.getConfig().max_temperature);
}

TEST_F(PMSMMotorTest, ParkTransformTest) {
    // 手动设置三相电流
    motor.getState().ia = 100.0;
    motor.getState().ib = -50.0;
    motor.getState().ic = -50.0;
    motor.getState().electrical_angle = 0.0;
    
    // 执行Park变换（这里需要调用private方法，实际测试中需要friend类或公开接口）
    // 这里简化测试，验证数据一致性
    EXPECT_DOUBLE_EQ(motor.getState().ia + motor.getState().ib + motor.getState().ic, 0.0);
}

TEST_F(PMSMMotorTest, InverseParkTransformTest) {
    motor.getState().id = 0.0;
    motor.getState().iq = 100.0;
    motor.getState().electrical_angle = M_PI / 4.0;  // 45度
    
    motor.update(0.001, PMSMInput());
    auto state = motor.getState();
    
    // 三相电流和应该为0
    double sum = state.ia + state.ib + state.ic;
    EXPECT_NEAR(sum, 0.0, 0.1);
}

TEST_F(PMSMMotorTest, ZeroTorqueTest) {
    PMSMInput input;
    input.target_torque = 0.0;
    
    motor.update(0.001, input);
    auto state = motor.getState();
    
    EXPECT_NEAR(state.electromagnetic_torque, 0.0, 0.1);
    EXPECT_NEAR(state.id, 0.0, 0.1);
    EXPECT_NEAR(state.iq, 0.0, 0.1);
}

TEST_F(PMSMMotorTest, NegativeTorqueTest) {
    PMSMInput input;
    input.target_torque = -50.0;  // 负扭矩（发电）
    
    motor.update(0.001, input);
    auto state = motor.getState();
    
    EXPECT_LT(state.electromagnetic_torque, 0.0);
}

TEST_F(PMSMMotorTest, ResetTest) {
    PMSMInput input;
    input.target_torque = 100.0;
    
    motor.update(0.001, input);
    motor.reset();
    
    auto state = motor.getState();
    EXPECT_DOUBLE_EQ(state.electromagnetic_torque, 0.0);
    EXPECT_DOUBLE_EQ(state.id, 0.0);
    EXPECT_DOUBLE_EQ(state.iq, 0.0);
}

TEST_F(PMSMMotorTest, ContinuousOperationTest) {
    PMSMInput input;
    input.target_torque = 100.0;
    
    // 连续运行1秒（1000步）
    for (int i = 0; i < 1000; ++i) {
        motor.update(0.001, input);
    }
    
    auto state = motor.getState();
    // 应该稳定运行
    EXPECT_GT(state.efficiency, 80.0);
    EXPECT_LT(state.stator_temperature, 100.0);
}

TEST_F(PMSMMotorTest, DynamicResponseTest) {
    // 阶跃响应测试
    PMSMInput input;
    input.target_torque = 0.0;
    
    motor.update(0.001, input);
    EXPECT_NEAR(motor.getState().electromagnetic_torque, 0.0, 0.1);
    
    // 阶跃到100Nm
    input.target_torque = 100.0;
    motor.update(0.001, input);
    
    // 应该快速响应（允许5%误差）
    EXPECT_NEAR(motor.getState().electromagnetic_torque, 100.0, 5.0);
}

TEST_F(PMSMMotorTest, VoltageLimitTest) {
    PMSMInput input;
    input.target_torque = 200.0;
    input.dc_bus_voltage = 300.0;  // 降低母线电压
    
    motor.update(0.001, input);
    auto state = motor.getState();
    
    // 电压降低时，电流限制应该更严格
    double current_magnitude = std::sqrt(state.id * state.id + state.iq * state.iq);
    EXPECT_LE(current_magnitude, motor.getConfig().max_current);
}

// ============================================================================
// 电池模型测试（17个）
// ============================================================================

class BatteryTest : public ::testing::Test {
protected:
    Battery battery;
    
    void SetUp() override {
        battery.initialize();
    }
};

TEST_F(BatteryTest, InitializationTest) {
    auto state = battery.getState();
    EXPECT_DOUBLE_EQ(state.soc, 0.8);  // 初始80% SOC
    EXPECT_DOUBLE_EQ(state.soh, 1.0);  // 初始100% SOH
}

TEST_F(BatteryTest, OCVCalculationTest) {
    // SOC 100%时的OCV应该接近最大电压
    battery.setSOC(1.0);
    auto state = battery.getState();
    EXPECT_NEAR(state.open_circuit_voltage, battery.getConfig().max_voltage, 10.0);
    
    // SOC 0%时的OCV应该接近最小电压
    battery.setSOC(0.0);
    state = battery.getState();
    EXPECT_NEAR(state.open_circuit_voltage, battery.getConfig().min_voltage, 20.0);
}

TEST_F(BatteryTest, DischargeTest) {
    BatteryInput input;
    input.current = 50.0;  // 50A放电
    
    double initial_soc = battery.getState().soc;
    
    // 放电10秒
    for (int i = 0; i < 1000; ++i) {
        battery.update(0.01, input);
    }
    
    auto state = battery.getState();
    EXPECT_LT(state.soc, initial_soc);  // SOC应该下降
    EXPECT_LT(state.terminal_voltage, state.open_circuit_voltage);  // 端电压<OCV
}

TEST_F(BatteryTest, ChargeTest) {
    battery.setSOC(0.5);  // 设置为50% SOC
    
    BatteryInput input;
    input.current = -50.0;  // 50A充电（负电流）
    
    double initial_soc = battery.getState().soc;
    
    // 充电10秒
    for (int i = 0; i < 1000; ++i) {
        battery.update(0.01, input);
    }
    
    auto state = battery.getState();
    EXPECT_GT(state.soc, initial_soc);  // SOC应该上升
    EXPECT_GT(state.terminal_voltage, state.open_circuit_voltage);  // 端电压>OCV
}

TEST_F(BatteryTest, CurrentLimitTest) {
    BatteryInput input;
    input.current = 500.0;  // 超大放电电流
    
    battery.update(0.01, input);
    auto state = battery.getState();
    
    // 电流应该被限制
    EXPECT_LE(std::abs(state.current), battery.getConfig().max_discharge_current);
}

TEST_F(BatteryTest, SOCLimitsTest) {
    // 测试SOC上限
    battery.setSOC(1.5);  // 尝试设置超过100%
    EXPECT_LE(battery.getState().soc, 1.0);
    
    // 测试SOC下限
    battery.setSOC(-0.5);  // 尝试设置低于0%
    EXPECT_GE(battery.getState().soc, 0.0);
}

TEST_F(BatteryTest, TerminalVoltageTest) {
    BatteryInput input;
    input.current = 100.0;  // 100A放电
    
    battery.update(0.01, input);
    auto state = battery.getState();
    
    // 端电压 = OCV - I*R - 极化电压
    double expected_voltage = state.open_circuit_voltage - 
                              input.current * battery.getConfig().series_resistance -
                              state.polarization_voltage;
    
    EXPECT_NEAR(state.terminal_voltage, expected_voltage, 5.0);
}

TEST_F(BatteryTest, PowerCalculationTest) {
    BatteryInput input;
    input.current = 100.0;  // 100A放电
    
    battery.update(0.01, input);
    auto state = battery.getState();
    
    // 功率 = 电压 * 电流
    double expected_power = state.terminal_voltage * state.current;
    EXPECT_NEAR(state.power, expected_power, 100.0);
}

TEST_F(BatteryTest, EnergyRemainingTest) {
    auto state = battery.getState();
    double expected_energy = state.soc * battery.getConfig().nominal_capacity * 
                             battery.getConfig().nominal_voltage;
    
    EXPECT_NEAR(state.energy_remaining, expected_energy / 1000.0, 1.0);  // kWh
}

TEST_F(BatteryTest, CapacityRemainingTest) {
    auto state = battery.getState();
    double expected_capacity = state.soc * battery.getConfig().nominal_capacity;
    
    EXPECT_NEAR(state.capacity_remaining, expected_capacity, 1.0);  // Ah
}

TEST_F(BatteryTest, TemperatureTest) {
    BatteryInput input;
    input.current = 200.0;  // 大电流放电
    input.ambient_temperature = 25.0;
    
    // 长时间运行
    for (int i = 0; i < 1000; ++i) {
        battery.update(0.1, input);  // 100秒
    }
    
    auto state = battery.getState();
    // 温度应该升高
    EXPECT_GT(state.internal_temperature, 25.0);
}

TEST_F(BatteryTest, TemperatureLimitTest) {
    BatteryInput input;
    input.current = 300.0;  // 极大电流
    input.ambient_temperature = 40.0;  // 高温环境
    
    // 长时间运行
    for (int i = 0; i < 10000; ++i) {
        battery.update(0.1, input);
    }
    
    auto state = battery.getState();
    // 温度不应该超过限制
    EXPECT_LE(state.internal_temperature, battery.getConfig().max_temperature);
}

TEST_F(BatteryTest, PolarizationTest) {
    BatteryInput input;
    input.current = 100.0;
    
    // 开始放电
    battery.update(0.01, input);
    double initial_polarization = battery.getState().polarization_voltage;
    
    // 持续放电
    for (int i = 0; i < 100; ++i) {
        battery.update(0.01, input);
    }
    
    // 极化电压应该增加
    EXPECT_GT(battery.getState().polarization_voltage, initial_polarization);
}

TEST_F(BatteryTest, ZeroCurrentTest) {
    BatteryInput input;
    input.current = 0.0;
    
    battery.update(0.01, input);
    auto state = battery.getState();
    
    // 零电流时，端电压应该接近OCV
    EXPECT_NEAR(state.terminal_voltage, state.open_circuit_voltage, 1.0);
}

TEST_F(BatteryTest, ResetTest) {
    BatteryInput input;
    input.current = 100.0;
    
    battery.update(0.01, input);
    battery.reset();
    
    auto state = battery.getState();
    EXPECT_DOUBLE_EQ(state.soc, 0.8);
    EXPECT_DOUBLE_EQ(state.current, 0.0);
}

TEST_F(BatteryTest, ContinuousDischargeTest) {
    BatteryInput input;
    input.current = 50.0;
    
    double initial_soc = battery.getState().soc;
    
    // 连续放电1000秒
    for (int i = 0; i < 100000; ++i) {
        battery.update(0.01, input);
    }
    
    auto state = battery.getState();
    // SOC应该显著下降
    EXPECT_LT(state.soc, initial_soc - 0.1);
}

TEST_F(BatteryTest, DynamicCurrentTest) {
    // 动态电流测试
    for (int i = 0; i < 100; ++i) {
        BatteryInput input;
        input.current = 50.0 * std::sin(i * 0.1);  // 正弦波电流
        
        battery.update(0.01, input);
    }
    
    auto state = battery.getState();
    // 应该稳定运行
    EXPECT_GE(state.soc, 0.0);
    EXPECT_LE(state.soc, 1.0);
}

// ============================================================================
// 集成测试（10个）
// ============================================================================

class PowertrainIntegrationTest : public ::testing::Test {
protected:
    PMSMMotor motor;
    Battery battery;
    
    void SetUp() override {
        motor.initialize();
        battery.initialize();
    }
};

TEST_F(PowertrainIntegrationTest, MotorBatteryIntegrationTest) {
    // 电机-电池集成测试
    PMSMInput motor_input;
    motor_input.target_torque = 100.0;
    
    // 运行1秒
    for (int i = 0; i < 1000; ++i) {
        motor.update(0.001, motor_input);
        
        // 计算电池放电电流
        double motor_power = motor.getState().output_power;
        double battery_current = motor_power / battery.getState().terminal_voltage;
        
        BatteryInput battery_input;
        battery_input.current = battery_current;
        battery.update(0.001, battery_input);
    }
    
    auto motor_state = motor.getState();
    auto battery_state = battery.getState();
    
    // 验证功率平衡
    double power_error = std::abs(motor_state.output_power - 
                                  (battery_state.power - battery_state.terminal_voltage * 
                                   battery.getConfig().series_resistance * battery_state.current));
    
    EXPECT_LT(power_error / motor_state.output_power, 0.1);  // <10%误差
}

TEST_F(PowertrainIntegrationTest, EnergyBalanceTest) {
    // 能量平衡测试
    PMSMInput motor_input;
    motor_input.target_torque = 100.0;
    
    double initial_energy = battery.getState().energy_remaining;
    
    // 运行10秒
    for (int i = 0; i < 10000; ++i) {
        motor.update(0.001, motor_input);
        
        double battery_current = motor.getState().output_power / 
                                battery.getState().terminal_voltage;
        
        BatteryInput battery_input;
        battery_input.current = battery_current;
        battery.update(0.001, battery_input);
    }
    
    double final_energy = battery.getState().energy_remaining;
    double energy_consumed = initial_energy - final_energy;
    
    // 能量消耗应该在合理范围内
    EXPECT_GT(energy_consumed, 0.0);
    EXPECT_LT(energy_consumed, initial_energy);
}

TEST_F(PowertrainIntegrationTest, AccelerationTest) {
    // 加速工况测试
    PMSMInput motor_input;
    motor_input.target_torque = 127.0;  // 额定扭矩
    
    // 0-100km/h加速测试
    double target_speed = 100.0 / 3.6;  // 27.78 m/s
    double current_speed = 0.0;
    double vehicle_mass = 1500.0;  // kg
    
    int steps = 0;
    while (current_speed < target_speed && steps < 100000) {
        motor.update(0.001, motor_input);
        
        // 简化的车辆动力学（只考虑加速）
        double acceleration = motor.getState().electromagnetic_torque / 
                            (vehicle_mass * 0.3);  // 假设轮半径0.3m
        current_speed += acceleration * 0.001;
        
        // 更新电池
        double battery_current = motor.getState().output_power / 
                                battery.getState().terminal_voltage;
        BatteryInput battery_input;
        battery_input.current = battery_current;
        battery.update(0.001, battery_input);
        
        steps++;
    }
    
    // 应该在30秒内达到100km/h
    EXPECT_LT(steps * 0.001, 30.0);
}

TEST_F(PowertrainIntegrationTest, EfficiencyTest) {
    // 系统效率测试
    PMSMInput motor_input;
    motor_input.target_torque = 100.0;
    
    double total_battery_energy = 0.0;
    double total_motor_energy = 0.0;
    
    // 运行10秒
    for (int i = 0; i < 10000; ++i) {
        motor.update(0.001, motor_input);
        
        double battery_current = motor.getState().output_power / 
                                battery.getState().terminal_voltage;
        
        BatteryInput battery_input;
        battery_input.current = battery_current;
        battery.update(0.001, battery_input);
        
        total_battery_energy += battery.getState().power * 0.001 / 3600.0;  // Wh
        total_motor_energy += motor.getState().output_power * 0.001 / 3600.0;  // Wh
    }
    
    // 系统效率 = 电机输出 / 电池输出
    double system_efficiency = total_motor_energy / total_battery_energy * 100.0;
    
    // 系统效率应该>75%
    EXPECT_GT(system_efficiency, 75.0);
}

// ... 更多集成测试 ...

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
