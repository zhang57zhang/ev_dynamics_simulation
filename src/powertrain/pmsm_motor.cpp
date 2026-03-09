/**
 * @file pmsm_motor.cpp
 * @brief 永磁同步电机（PMSM）模型实现
 * @author CodeCraft
 * @date 2026-03-08
 */

#include "pmsm_motor.h"
#include <algorithm>
#include <iostream>

namespace ev_dynamics {
namespace powertrain {

PMSMMotor::PMSMMotor(const PMSMConfig& config)
    : config_(config)
    , kp_current_(0.5)
    , ki_current_(10.0)
    , current_integral_error_d_(0.0)
    , current_integral_error_q_(0.0)
{
    initialize();
}

void PMSMMotor::initialize() {
    state_ = PMSMState();
    current_integral_error_d_ = 0.0;
    current_integral_error_q_ = 0.0;
    
    std::cout << "[PMSM] Motor initialized: " << config_.rated_power / 1000.0 
              << "kW, " << config_.rated_torque << "Nm, " 
              << config_.rated_speed << "rpm" << std::endl;
}

void PMSMMotor::update(double dt, const PMSMInput& input) {
    // 1. 磁场定向控制（FOC）
    fieldOrientedControl(input.target_torque);
    
    // 2. 电流限制
    limitCurrent();
    
    // 3. 计算电磁转矩
    calculateElectromagneticTorque();
    
    // 4. 逆Park变换（d-q → 三相）
    inverseParkTransform();
    
    // 5. 计算损耗
    calculateLosses();
    
    // 6. 更新温度
    updateTemperature(dt);
    
    // 7. 计算效率
    calculateEfficiency();
    
    // 8. 更新电气角度
    state_.electrical_angle += config_.pole_pairs * state_.rotor_speed * dt;
    
    // 保持角度在[0, 2π]范围内
    while (state_.electrical_angle >= 2.0 * M_PI) {
        state_.electrical_angle -= 2.0 * M_PI;
    }
    while (state_.electrical_angle < 0.0) {
        state_.electrical_angle += 2.0 * M_PI;
    }
}

void PMSMMotor::calculateElectromagneticTorque() {
    // PMSM电磁转矩公式：Te = 1.5 * p * [ψf * iq + (Ld - Lq) * id * iq]
    // 对于表贴式PMSM（Ld ≈ Lq），简化为：Te = 1.5 * p * ψf * iq
    
    double torque = 1.5 * config_.pole_pairs * (
        config_.permanent_flux * state_.iq +
        (config_.d_axis_inductance - config_.q_axis_inductance) * state_.id * state_.iq
    );
    
    state_.electromagnetic_torque = torque;
}

void PMSMMotor::inverseParkTransform() {
    // 逆Park变换：d-q → α-β
    double theta = state_.electrical_angle;
    double cos_theta = std::cos(theta);
    double sin_theta = std::sin(theta);
    
    // d-q → α-β
    double i_alpha = state_.id * cos_theta - state_.iq * sin_theta;
    double i_beta = state_.id * sin_theta + state_.iq * cos_theta;
    
    // α-β → a-b-c（逆Clarke变换）
    state_.ia = i_alpha;
    state_.ib = -0.5 * i_alpha + std::sqrt(3.0) / 2.0 * i_beta;
    state_.ic = -0.5 * i_alpha - std::sqrt(3.0) / 2.0 * i_beta;
}

void PMSMMotor::parkTransform() {
    // Park变换：三相 → d-q
    double theta = state_.electrical_angle;
    double cos_theta = std::cos(theta);
    double sin_theta = std::sin(theta);
    
    // a-b-c → α-β（Clarke变换）
    double i_alpha = state_.ia;
    double i_beta = (state_.ia + 2.0 * state_.ib) / std::sqrt(3.0);
    
    // α-β → d-q
    state_.id = i_alpha * cos_theta + i_beta * sin_theta;
    state_.iq = -i_alpha * sin_theta + i_beta * cos_theta;
}

void PMSMMotor::calculateLosses() {
    // 1. 铜损：Pcu = 1.5 * Rs * (id² + iq²)
    state_.copper_loss = 1.5 * config_.stator_resistance * 
                         (state_.id * state_.id + state_.iq * state_.iq);
    
    // 2. 铁损（简化模型）：Pfe = kfe * ω^1.5
    // kfe: 铁损系数（经验值）
    double iron_loss_coefficient = 0.01;  // W/(rad/s)^1.5
    state_.iron_loss = iron_loss_coefficient * 
                       std::pow(std::abs(state_.rotor_speed), 1.5);
    
    // 3. 总损耗
    state_.total_loss = state_.copper_loss + state_.iron_loss;
    
    // 4. 输出功率
    state_.output_power = state_.electromagnetic_torque * state_.rotor_speed;
}

void PMSMMotor::updateTemperature(double dt) {
    // 简化的热模型：dT/dt = (Ploss - (T - Tamb) / Rth) / Cth
    // Ploss: 损耗功率
    // Rth: 热阻
    // Cth: 热容
    // Tamb: 环境温度
    
    double heat_generated = state_.total_loss;
    double heat_dissipated = (state_.stator_temperature - 25.0) / 
                             config_.thermal_resistance;
    
    double temperature_change = (heat_generated - heat_dissipated) * dt / 
                                config_.thermal_capacitance;
    
    state_.stator_temperature += temperature_change;
    
    // 转子温度近似等于定子温度（简化模型）
    state_.rotor_temperature = state_.stator_temperature;
    
    // 温度限制
    if (state_.stator_temperature > config_.max_temperature) {
        std::cerr << "[PMSM] WARNING: Temperature exceeded limit: " 
                  << state_.stator_temperature << "°C > " 
                  << config_.max_temperature << "°C" << std::endl;
    }
}

void PMSMMotor::fieldOrientedControl(double target_torque) {
    // 磁场定向控制（FOC）：Id=0控制策略
    
    // 目标q轴电流（根据目标转矩计算）
    // Te = 1.5 * p * ψf * iq  =>  iq = Te / (1.5 * p * ψf)
    double target_iq = target_torque / (1.5 * config_.pole_pairs * 
                                        config_.permanent_flux);
    
    // 目标d轴电流（Id=0控制）
    double target_id = 0.0;
    
    // 电流环PI控制
    double error_id = target_id - state_.id;
    double error_iq = target_iq - state_.iq;
    
    current_integral_error_d_ += error_id * 0.001;  // 假设1ms控制周期
    current_integral_error_q_ += error_iq * 0.001;
    
    // 积分限幅（防止积分饱和）
    double max_integral = 100.0;
    current_integral_error_d_ = std::clamp(current_integral_error_d_, 
                                           -max_integral, max_integral);
    current_integral_error_q_ = std::clamp(current_integral_error_q_, 
                                           -max_integral, max_integral);
    
    // PI控制输出
    state_.id += (kp_current_ * error_id + 
                  ki_current_ * current_integral_error_d_) * 0.001;
    state_.iq += (kp_current_ * error_iq + 
                  ki_current_ * current_integral_error_q_) * 0.001;
}

void PMSMMotor::limitCurrent() {
    // 电流幅值限制
    double current_magnitude = std::sqrt(state_.id * state_.id + 
                                         state_.iq * state_.iq);
    
    if (current_magnitude > config_.max_current) {
        double scale = config_.max_current / current_magnitude;
        state_.id *= scale;
        state_.iq *= scale;
        
        std::cerr << "[PMSM] Current limited: " << current_magnitude 
                  << "A -> " << config_.max_current << "A" << std::endl;
    }
}

void PMSMMotor::calculateEfficiency() {
    // 效率 = 输出功率 / (输出功率 + 损耗)
    double input_power = state_.output_power + state_.total_loss;
    
    if (input_power > 0.0) {
        state_.efficiency = (state_.output_power / input_power) * 100.0;
    } else {
        state_.efficiency = 0.0;
    }
}

void PMSMMotor::reset() {
    state_ = PMSMState();
    current_integral_error_d_ = 0.0;
    current_integral_error_q_ = 0.0;
    
    std::cout << "[PMSM] Motor reset to initial state" << std::endl;
}

} // namespace powertrain
} // namespace ev_dynamics
