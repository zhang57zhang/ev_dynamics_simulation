/**
 * @file braking.cpp
 * @brief 线控制动系统实现
 * @author CodeCraft
 * @date 2026-03-08
 */

#include "braking.h"
#include <algorithm>
#include <cmath>

namespace ev_dynamics {
namespace chassis {

// ============================================================================
// 构造函数和初始化
// ============================================================================

Braking::Braking(const BrakingConfig& config)
    : config_(config)
    , reference_speed_(0.0)
{
    prev_slip_ratio_.fill(0.0);
}

void Braking::initialize() {
    state_ = BrakingState();
    reference_speed_ = 0.0;
    prev_slip_ratio_.fill(0.0);
}

// ============================================================================
// 主更新函数
// ============================================================================

void Braking::update(double dt, const BrakingInput& input) {
    // 1. 保存踏板位置
    state_.brake_pedal_position = input.brake_pedal_position;
    
    // 2. 计算参考车速（取四轮最大）
    reference_speed_ = calculateReferenceSpeed(input);
    
    // 3. 更新轮速
    state_.front_left.wheel_speed = input.wheel_speed_fl;
    state_.front_right.wheel_speed = input.wheel_speed_fr;
    state_.rear_left.wheel_speed = input.wheel_speed_rl;
    state_.rear_right.wheel_speed = input.wheel_speed_rr;
    
    // 4. 计算滑移率
    state_.front_left.wheel_slip = calculateSlipRatio(input.wheel_speed_fl, reference_speed_);
    state_.front_right.wheel_slip = calculateSlipRatio(input.wheel_speed_fr, reference_speed_);
    state_.rear_left.wheel_slip = calculateSlipRatio(input.wheel_speed_rl, reference_speed_);
    state_.rear_right.wheel_slip = calculateSlipRatio(input.wheel_speed_rr, reference_speed_);
    
    // 5. EBD制动力分配
    if (config_.ebd_enabled) {
        ebdControl(input);
    } else {
        state_.front_brake_ratio = config_.front_rear_distribution;
        state_.rear_brake_ratio = 1.0 - config_.front_rear_distribution;
    }
    
    // 6. 计算总制动力矩请求
    double total_torque_request = input.brake_pedal_position 
                                  * config_.front_left.max_brake_torque * 4.0;
    
    // 7. 制动能量回收协调
    if (config_.regen_enabled) {
        regenCoordination(total_torque_request, input);
    } else {
        state_.regen_torque = 0.0;
        state_.friction_torque = total_torque_request;
    }
    
    // 8. 分配制动力矩到各轮
    double front_torque = state_.friction_torque * state_.front_brake_ratio / 2.0;
    double rear_torque = state_.friction_torque * state_.rear_brake_ratio / 2.0;
    
    double target_pressure_fl = front_torque / config_.front_left.brake_gain;
    double target_pressure_fr = front_torque / config_.front_right.brake_gain;
    double target_pressure_rl = rear_torque / config_.rear_left.brake_gain;
    double target_pressure_rr = rear_torque / config_.rear_right.brake_gain;
    
    // 9. ABS控制
    state_.abs_active_wheels = 0;
    
    if (config_.abs_enabled) {
        absControl(state_.front_left, config_.front_left, 
                   state_.front_left.wheel_slip, dt);
        absControl(state_.front_right, config_.front_right,
                   state_.front_right.wheel_slip, dt);
        absControl(state_.rear_left, config_.rear_left,
                   state_.rear_left.wheel_slip, dt);
        absControl(state_.rear_right, config_.rear_right,
                   state_.rear_right.wheel_slip, dt);
        
        // 使用ABS目标压力
        target_pressure_fl = state_.front_left.target_pressure;
        target_pressure_fr = state_.front_right.target_pressure;
        target_pressure_rl = state_.rear_left.target_pressure;
        target_pressure_rr = state_.rear_right.target_pressure;
        
        state_.abs_active = (state_.abs_active_wheels > 0);
    } else {
        state_.abs_active = false;
        state_.front_left.abs_state = ABSState::INACTIVE;
        state_.front_right.abs_state = ABSState::INACTIVE;
        state_.rear_left.abs_state = ABSState::INACTIVE;
        state_.rear_right.abs_state = ABSState::INACTIVE;
    }
    
    // 10. 更新各轮制动
    updateCorner(state_.front_left, config_.front_left, target_pressure_fl, dt);
    updateCorner(state_.front_right, config_.front_right, target_pressure_fr, dt);
    updateCorner(state_.rear_left, config_.rear_left, target_pressure_rl, dt);
    updateCorner(state_.rear_right, config_.rear_right, target_pressure_rr, dt);
    
    // 11. 更新温度
    updateTemperature(state_.front_left, config_.front_left, dt);
    updateTemperature(state_.front_right, config_.front_right, dt);
    updateTemperature(state_.rear_left, config_.rear_left, dt);
    updateTemperature(state_.rear_right, config_.rear_right, dt);
    
    // 12. 计算总制动力和减速度
    state_.total_brake_torque = state_.front_left.brake_torque
                               + state_.front_right.brake_torque
                               + state_.rear_left.brake_torque
                               + state_.rear_right.brake_torque
                               + state_.regen_torque;
    
    state_.total_brake_force = state_.total_brake_torque / config_.wheel_radius;
    
    calculateDeceleration();
    
    // 13. 保存前一时刻滑移率
    prev_slip_ratio_ = {
        state_.front_left.wheel_slip,
        state_.front_right.wheel_slip,
        state_.rear_left.wheel_slip,
        state_.rear_right.wheel_slip
    };
}

// ============================================================================
// 参考车速计算
// ============================================================================

double Braking::calculateReferenceSpeed(const BrakingInput& input) const {
    // 参考车速 = max(四轮轮速对应的车速, 车辆速度)
    double wheel_speed_to_vehicle = config_.wheel_radius;
    
    double ref_from_wheels = std::max({
        input.wheel_speed_fl * wheel_speed_to_vehicle,
        input.wheel_speed_fr * wheel_speed_to_vehicle,
        input.wheel_speed_rl * wheel_speed_to_vehicle,
        input.wheel_speed_rr * wheel_speed_to_vehicle
    });
    
    // 如果有车速信号，取较大值
    return std::max(ref_from_wheels, input.vehicle_speed);
}

// ============================================================================
// 滑移率计算
// ============================================================================

double Braking::calculateSlipRatio(double wheel_speed, double vehicle_speed) const {
    if (vehicle_speed < 0.1) {
        return 0.0;
    }
    
    double wheel_velocity = wheel_speed * config_.wheel_radius;
    double slip = (vehicle_speed - wheel_velocity) / vehicle_speed;
    
    return std::clamp(slip, 0.0, 1.0);
}

// ============================================================================
// ABS控制
// ============================================================================

void Braking::absControl(BrakeCornerState& corner_state,
                         const BrakeCornerConfig& corner_config,
                         double slip_ratio,
                         double dt) {
    // ABS状态机
    
    // 判断是否需要ABS介入
    bool need_abs = (slip_ratio > config_.slip_ratio_min);
    
    if (!need_abs) {
        corner_state.abs_state = ABSState::INACTIVE;
        corner_state.abs_cycle_timer = 0.0;
        return;
    }
    
    // ABS激活计数
    if (corner_state.abs_state != ABSState::INACTIVE) {
        state_.abs_active_wheels++;
    }
    
    // ABS循环周期
    double cycle_time = 1.0 / config_.abs_cycle_frequency;
    corner_state.abs_cycle_timer += dt;
    
    // 滑移率变化率
    double slip_rate = (slip_ratio - prev_slip_ratio_[0]) / dt;  // 简化，实际应对应各轮
    
    // 状态转换逻辑
    switch (corner_state.abs_state) {
        case ABSState::INACTIVE:
            if (slip_ratio > config_.slip_ratio_max) {
                // 滑移率过大，开始减压
                corner_state.abs_state = ABSState::PRESSURE_DECREASE;
                corner_state.abs_cycle_timer = 0.0;
                state_.abs_active_wheels++;
            } else if (slip_ratio > config_.slip_ratio_min) {
                // 滑移率进入ABS范围，开始保压
                corner_state.abs_state = ABSState::PRESSURE_HOLD;
                corner_state.abs_cycle_timer = 0.0;
                state_.abs_active_wheels++;
            }
            break;
            
        case ABSState::PRESSURE_HOLD:
            if (corner_state.abs_cycle_timer >= cycle_time) {
                if (slip_ratio > config_.slip_ratio_target) {
                    // 滑移率偏高，继续减压
                    corner_state.abs_state = ABSState::PRESSURE_DECREASE;
                } else {
                    // 滑移率偏低，开始增压
                    corner_state.abs_state = ABSState::PRESSURE_INCREASE;
                }
                corner_state.abs_cycle_timer = 0.0;
            }
            break;
            
        case ABSState::PRESSURE_DECREASE:
            // 减压阶段
            corner_state.target_pressure *= 0.8;  // 减压20%
            corner_state.target_pressure = std::max(corner_state.target_pressure, 0.0);
            
            if (corner_state.abs_cycle_timer >= cycle_time) {
                corner_state.abs_state = ABSState::PRESSURE_HOLD;
                corner_state.abs_cycle_timer = 0.0;
            }
            break;
            
        case ABSState::PRESSURE_INCREASE:
            // 增压阶段
            corner_state.target_pressure *= 1.1;  // 增压10%
            corner_state.target_pressure = std::min(corner_state.target_pressure,
                                                    corner_config.max_pressure);
            
            if (corner_state.abs_cycle_timer >= cycle_time) {
                if (slip_ratio > config_.slip_ratio_max) {
                    corner_state.abs_state = ABSState::PRESSURE_DECREASE;
                } else {
                    corner_state.abs_state = ABSState::PRESSURE_HOLD;
                }
                corner_state.abs_cycle_timer = 0.0;
            }
            break;
    }
}

// ============================================================================
// EBD制动力分配
// ============================================================================

void Braking::ebdControl(const BrakingInput& input) {
    // EBD根据轴荷转移动态调整前后制动力分配
    
    // 计算轴荷转移
    // ΔFz = m * a * h / L
    double axle_load_transfer = config_.vehicle_mass 
                               * std::abs(input.longitudinal_acceleration)
                               * config_.cg_height 
                               / config_.wheelbase;
    
    // 静态轴荷
    double static_front_load = config_.vehicle_mass * 9.81 * 0.5;  // 假设50:50
    double static_rear_load = static_front_load;
    
    // 动态轴荷（制动时前轴增载，后轴减载）
    double dynamic_front_load = static_front_load + axle_load_transfer;
    double dynamic_rear_load = static_rear_load - axle_load_transfer;
    
    // 理想制动力分配（基于附着率）
    double ideal_front_ratio = dynamic_front_load / (dynamic_front_load + dynamic_rear_load);
    
    // EBD修正（逐渐向理想分配靠近）
    double ebd_correction = config_.ebd_gain * (ideal_front_ratio - config_.front_rear_distribution);
    state_.ebd_correction = ebd_correction;
    
    // 应用修正
    state_.front_brake_ratio = config_.front_rear_distribution + ebd_correction;
    state_.rear_brake_ratio = 1.0 - state_.front_brake_ratio;
    
    // 限制范围（避免后轮抱死）
    state_.front_brake_ratio = std::clamp(state_.front_brake_ratio, 0.5, 0.8);
    state_.rear_brake_ratio = 1.0 - state_.front_brake_ratio;
}

// ============================================================================
// 制动能量回收协调
// ============================================================================

void Braking::regenCoordination(double total_torque_request, const BrakingInput& input) {
    // 制动能量回收协调算法
    // 优先使用再生制动，不足部分由摩擦制动补充
    
    // 1. 计算可用再生制动力矩
    double available_regen = std::min(config_.max_regen_torque * input.regen_request,
                                      config_.max_regen_torque);
    
    // 2. 低速时逐渐退出再生制动（避免车轮抱死）
    if (input.vehicle_speed < 5.0) {
        double speed_factor = input.vehicle_speed / 5.0;
        available_regen *= speed_factor;
    }
    
    // 3. 紧急制动时减少再生制动（优先保证制动安全）
    if (input.brake_pedal_position > 0.8) {
        double emergency_factor = 1.0 - (input.brake_pedal_position - 0.8) / 0.2;
        available_regen *= emergency_factor;
    }
    
    // 4. 分配再生和摩擦制动
    double max_regen_torque = total_torque_request * config_.regen_blend_factor;
    state_.regen_torque = std::min(available_regen, max_regen_torque);
    state_.friction_torque = total_torque_request - state_.regen_torque;
    
    // 5. 确保摩擦制动非负
    state_.friction_torque = std::max(state_.friction_torque, 0.0);
}

// ============================================================================
// 单轮制动更新
// ============================================================================

void Braking::updateCorner(BrakeCornerState& corner_state,
                           const BrakeCornerConfig& corner_config,
                           double target_pressure,
                           double dt) {
    // 保存目标压力（用于ABS）
    corner_state.target_pressure = target_pressure;
    
    // 制动器响应（一阶惯性）
    double tau = corner_config.brake_response_time;
    double alpha = dt / (tau + dt);
    
    corner_state.brake_pressure = corner_state.brake_pressure
                                  + alpha * (target_pressure - corner_state.brake_pressure);
    
    // 限制压力范围
    corner_state.brake_pressure = std::clamp(corner_state.brake_pressure,
                                             0.0,
                                             corner_config.max_pressure);
    
    // 计算制动力矩
    calculateBrakeTorque(corner_state, corner_config);
}

// ============================================================================
// 制动力矩计算
// ============================================================================

void Braking::calculateBrakeTorque(BrakeCornerState& corner_state,
                                   const BrakeCornerConfig& corner_config) {
    if (config_.type == BrakingType::ELECTRO_MECHANICAL) {
        // EMB：电流 -> 夹紧力 -> 制动力矩
        corner_state.motor_current = corner_state.brake_pressure;  // 这里pressure实际是电流
        
        double clamp_force = corner_state.motor_current 
                           * corner_config.motor_torque_constant
                           * corner_config.gear_ratio
                           / corner_config.ball_screw_pitch;
        
        corner_state.brake_torque = clamp_force 
                                   * corner_config.pad_friction_coeff
                                   * corner_config.disc_radius * 2.0;
    } else {
        // 液压/EHB：压力 -> 夹紧力 -> 制动力矩
        double clamp_force = corner_state.brake_pressure * 1e6  // MPa -> Pa
                           * corner_config.piston_area;
        
        corner_state.brake_torque = clamp_force 
                                   * corner_config.pad_friction_coeff
                                   * corner_config.disc_radius * 2.0;
    }
    
    // 限制最大力矩
    corner_state.brake_torque = std::min(corner_state.brake_torque,
                                         corner_config.max_brake_torque);
    
    // 计算制动力
    corner_state.brake_force = corner_state.brake_torque / config_.wheel_radius;
}

// ============================================================================
// 温度更新
// ============================================================================

void Braking::updateTemperature(BrakeCornerState& corner_state,
                                const BrakeCornerConfig& corner_config,
                                double dt) {
    // 制动功率（热量产生）
    double heat_power = corner_state.brake_torque 
                       * corner_state.wheel_speed;  // W
    
    // 热动力学模型
    // dT/dt = (P_heat - (T - T_amb) / R_th) / C_th
    double ambient_temp = 25.0;  // 环境温度
    double heat_loss = (corner_state.disc_temperature - ambient_temp) 
                      / corner_config.thermal_resistance;
    
    double temp_rate = (heat_power - heat_loss) / corner_config.thermal_mass;
    
    corner_state.disc_temperature += temp_rate * dt;
    
    // 限制温度范围
    corner_state.disc_temperature = std::max(corner_state.disc_temperature, ambient_temp);
    corner_state.disc_temperature = std::min(corner_state.disc_temperature,
                                            corner_config.max_temperature);
}

// ============================================================================
// 减速度计算
// ============================================================================

void Braking::calculateDeceleration() {
    // 减速度 = 总制动力 / 整车质量
    state_.deceleration = state_.total_brake_force / config_.vehicle_mass;
}

// ============================================================================
// 设置函数
// ============================================================================

void Braking::enableABS(bool enable) {
    config_.abs_enabled = enable;
}

void Braking::enableEBD(bool enable) {
    config_.ebd_enabled = enable;
}

void Braking::enableRegen(bool enable) {
    config_.regen_enabled = enable;
}

void Braking::setRegenBlendFactor(double factor) {
    config_.regen_blend_factor = std::clamp(factor, 0.0, 1.0);
}

void Braking::emergencyBrake() {
    // 紧急制动：最大制动力
    state_.brake_pedal_position = 1.0;
    state_.front_left.target_pressure = config_.front_left.max_pressure;
    state_.front_right.target_pressure = config_.front_right.max_pressure;
    state_.rear_left.target_pressure = config_.rear_left.max_pressure;
    state_.rear_right.target_pressure = config_.rear_right.max_pressure;
}

void Braking::releaseBrake() {
    // 释放制动
    state_.brake_pedal_position = 0.0;
    state_.front_left.target_pressure = 0.0;
    state_.front_right.target_pressure = 0.0;
    state_.rear_left.target_pressure = 0.0;
    state_.rear_right.target_pressure = 0.0;
}

void Braking::reset() {
    initialize();
}

} // namespace chassis
} // namespace ev_dynamics
