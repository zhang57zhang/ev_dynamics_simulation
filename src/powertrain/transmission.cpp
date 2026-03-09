/**
 * @file transmission.cpp
 * @brief 单级减速器模型实现
 * @author CodeCraft
 * @date 2026-03-08
 */

#include "transmission.h"
#include <algorithm>
#include <iostream>

namespace ev_dynamics {
namespace powertrain {

Transmission::Transmission(const TransmissionConfig& config)
    : config_(config)
{
    initialize();
}

void Transmission::initialize() {
    state_ = TransmissionState();
    
    std::cout << "[Transmission] Initialized: ratio=" << config_.gear_ratio 
              << ":1, efficiency=" << (config_.gear_efficiency * 100) << "%"
              << std::endl;
}

void Transmission::update(double dt, const TransmissionInput& input) {
    // 更新输入
    state_.input_torque = input.input_torque;
    state_.input_speed = input.input_speed;
    
    // 1. 计算输出转速
    calculateOutputSpeed();
    
    // 2. 计算输出扭矩
    calculateOutputTorque();
    
    // 3. 计算效率
    calculateEfficiency();
    
    // 4. 计算功率损耗
    calculatePowerLoss();
    
    // 5. 更新扭转振动
    updateVibration(dt);
    
    // 6. 更新温度
    updateTemperature(dt);
}

void Transmission::calculateOutputSpeed() {
    // 输出转速 = 输入转速 / 传动比
    state_.output_speed = state_.input_speed / config_.gear_ratio;
    
    // 转速限制
    double max_input_rad = config_.max_input_speed * 2.0 * M_PI / 60.0;
    if (state_.input_speed > max_input_rad) {
        std::cerr << "[Transmission] WARNING: Input speed exceeded limit" << std::endl;
    }
}

void Transmission::calculateOutputTorque() {
    // 输出扭矩 = 输入扭矩 * 传动比 * 效率
    state_.output_torque = state_.input_torque * config_.gear_ratio * state_.efficiency;
    
    // 扭矩限制
    if (state_.output_torque > config_.max_output_torque) {
        std::cerr << "[Transmission] WARNING: Output torque exceeded limit" << std::endl;
        state_.output_torque = config_.max_output_torque;
    }
}

void Transmission::calculateEfficiency() {
    // 效率模型（简化）：
    // 基础效率 * 负载因子 * 速度因子
    // 负载因子：中等负载时效率最高
    // 速度因子：中等转速时效率最高
    
    double torque_ratio = state_.output_torque / config_.max_output_torque;
    double speed_ratio = state_.input_speed / (config_.max_input_speed * 2.0 * M_PI / 60.0);
    
    // 负载因子（抛物线，最优在0.5-0.7）
    double load_factor = 1.0 - 0.1 * std::pow(torque_ratio - 0.6, 2);
    
    // 速度因子（抛物线，最优在0.5-0.7）
    double speed_factor = 1.0 - 0.1 * std::pow(speed_ratio - 0.6, 2);
    
    state_.efficiency = config_.gear_efficiency * load_factor * speed_factor;
    
    // 效率限制
    state_.efficiency = std::clamp(state_.efficiency, 0.8, 1.0);
}

void Transmission::calculatePowerLoss() {
    // 功率损耗 = 输入功率 * (1 - 效率)
    double input_power = state_.input_torque * state_.input_speed;
    state_.power_loss = input_power * (1.0 - state_.efficiency);
}

void Transmission::updateVibration(double dt) {
    // 扭转振动模型（二阶系统）：
    // J * θ'' + c * θ' + k * θ = T
    // J: 惯量, c: 阻尼, k: 刚度, T: 扭矩, θ: 扭转角
    
    // 简化为单自由度振动
    double torque_deviation = state_.input_torque - state_.output_torque / config_.gear_ratio;
    
    // 振动加速度
    double vibration_acc = (torque_deviation - 
                           config_.damping * state_.vibration_velocity -
                           config_.stiffness * state_.vibration_angle) / config_.gear_inertia;
    
    // 更新振动状态（Euler积分）
    state_.vibration_velocity += vibration_acc * dt;
    state_.vibration_angle += state_.vibration_velocity * dt;
    
    // 振动衰减（阻尼）
    state_.vibration_velocity *= 0.99;
}

void Transmission::updateTemperature(double dt) {
    // 热模型：C * dT/dt = P_loss - h * (T - T_amb)
    // C: 热容, P_loss: 功率损耗, h: 散热系数, T_amb: 环境温度
    
    double thermal_capacitance = 1000.0;  // J/K（经验值）
    double heat_transfer_coeff = 5.0;     // W/K（经验值）
    
    // 温度变化
    double heat_generated = state_.power_loss;
    double heat_dissipated = heat_transfer_coeff * 
                            (state_.temperature - 25.0);
    
    double temperature_change = (heat_generated - heat_dissipated) * dt / thermal_capacitance;
    state_.temperature += temperature_change;
    
    // 温度限制
    if (state_.temperature > config_.max_temperature) {
        std::cerr << "[Transmission] WARNING: Temperature exceeded: " 
                  << state_.temperature << "°C" << std::endl;
    }
}

void Transmission::reset() {
    state_ = TransmissionState();
    std::cout << "[Transmission] Reset to initial state" << std::endl;
}

} // namespace powertrain
} // namespace ev_dynamics
