/**
 * @file steering.cpp
 * @brief 电动助力转向系统实现
 * @author CodeCraft
 * @date 2026-03-08
 */

#include "steering.h"
#include <algorithm>
#include <cmath>

namespace ev_dynamics {
namespace chassis {

// ============================================================================
// 构造函数和初始化
// ============================================================================

Steering::Steering(const SteeringConfig& config)
    : config_(config)
    , active_return_enabled_(true)
    , manual_steering_ratio_(0.0)
    , motor_torque_command_(0.0)
    , prev_steering_angle_(0.0)
{
}

void Steering::initialize() {
    state_ = SteeringState();
    state_.current_steering_ratio = config_.base_steering_ratio;
    
    motor_torque_command_ = 0.0;
    prev_steering_angle_ = 0.0;
}

// ============================================================================
// 主更新函数
// ============================================================================

void Steering::update(double dt, const SteeringInput& input) {
    // 1. 更新转向盘角速度
    state_.steering_wheel_speed = (state_.steering_wheel_angle - prev_steering_angle_) / dt;
    prev_steering_angle_ = state_.steering_wheel_angle;
    
    // 2. 计算可变转向比
    state_.current_steering_ratio = calculateVariableRatio(input.vehicle_speed,
                                                            state_.steering_wheel_angle);
    
    // 3. 计算平均车轮转角
    state_.wheel_angle_average = state_.steering_wheel_angle / state_.current_steering_ratio;
    
    // 4. 计算Ackerman转向几何
    auto [left_angle, right_angle] = calculateAckermanAngles(state_.wheel_angle_average,
                                                              config_.wheelbase,
                                                              config_.track_width);
    
    state_.wheel_angle_left = left_angle;
    state_.wheel_angle_right = right_angle;
    state_.ackerman_angle_diff = std::abs(left_angle - right_angle);
    
    // 5. 限制车轮转角
    state_.wheel_angle_left = std::clamp(state_.wheel_angle_left,
                                         -config_.max_wheel_angle,
                                         config_.max_wheel_angle);
    state_.wheel_angle_right = std::clamp(state_.wheel_angle_right,
                                          -config_.max_wheel_angle,
                                          config_.max_wheel_angle);
    
    // 6. 计算助力扭矩
    state_.assist_torque = calculateAssistTorque(input.driver_torque, input.vehicle_speed);
    
    // 7. 计算反馈扭矩
    state_.feedback_torque = calculateFeedbackTorque(input);
    
    // 8. 计算回正扭矩
    state_.return_torque = calculateReturnTorque(input);
    
    // 9. 计算总电机扭矩指令
    motor_torque_command_ = state_.assist_torque 
                           + state_.feedback_torque
                           + state_.return_torque;
    
    // 添加摩擦补偿
    motor_torque_command_ += frictionCompensation(state_.steering_wheel_speed);
    
    // 限制电机扭矩
    motor_torque_command_ = std::clamp(motor_torque_command_,
                                       -config_.motor_torque_max,
                                       config_.motor_torque_max);
    
    // 10. 更新电机状态
    updateMotor(dt, motor_torque_command_);
    
    // 11. 更新转向柱状态（根据转向类型）
    if (config_.type == SteeringType::STEER_BY_WIRE) {
        // 线控转向：直接设置转向盘角度
        state_.steering_wheel_angle = input.driver_angle;
        state_.steering_wheel_torque = input.driver_torque;
    } else {
        // 传统转向：通过转向柱动力学
        updateSteeringColumn(dt, input.driver_torque);
    }
    
    // 12. 检查回正状态
    state_.is_returning = (std::abs(input.driver_torque) < 0.5 
                          && std::abs(state_.steering_wheel_angle) > 0.05);
}

// ============================================================================
// 可变转向比计算
// ============================================================================

double Steering::calculateVariableRatio(double vehicle_speed, 
                                        double steering_angle) const {
    // 如果手动设置了转向比，使用手动值
    if (manual_steering_ratio_ > 0.0) {
        return manual_steering_ratio_;
    }
    
    // 速度相关转向比
    double speed_ratio = 1.0;
    
    if (vehicle_speed < config_.variable_ratio_start_speed) {
        // 低速：最大转向比（轻便）
        speed_ratio = config_.max_steering_ratio / config_.base_steering_ratio;
    } else {
        // 高速：线性过渡到最小转向比（稳定）
        double speed_factor = (vehicle_speed - config_.variable_ratio_start_speed) / 20.0;
        speed_factor = std::clamp(speed_factor, 0.0, 1.0);
        speed_ratio = config_.max_steering_ratio / config_.base_steering_ratio
                     - speed_factor * (config_.max_steering_ratio - config_.min_steering_ratio) 
                       / config_.base_steering_ratio;
    }
    
    // 角度相关转向比（大角度时减小转向比，提高灵活性）
    double angle_factor = std::abs(steering_angle) / (2.0 * M_PI);  // 归一化到一圈
    angle_factor = std::clamp(angle_factor, 0.0, 1.0);
    
    double angle_ratio = 1.0 - 0.2 * angle_factor;  // 最大减小20%
    
    return config_.base_steering_ratio * speed_ratio * angle_ratio;
}

// ============================================================================
// Ackerman转向几何
// ============================================================================

std::pair<double, double> Steering::calculateAckermanAngles(double average_angle,
                                                             double wheelbase,
                                                             double track_width) const {
    // 完美Ackerman几何
    // tan(δ_inner) - tan(δ_outer) = track / wheelbase
    
    if (std::abs(average_angle) < 0.001) {
        return {0.0, 0.0};
    }
    
    // 简化计算：使用百分比Ackerman
    // 实际Ackerman = 完美Ackerman * percentage
    
    double R = wheelbase / std::tan(std::abs(average_angle));  // 转弯半径
    
    // 完美Ackerman内轮和外轮角度
    double inner_angle = std::atan(wheelbase / (R - track_width / 2.0));
    double outer_angle = std::atan(wheelbase / (R + track_width / 2.0));
    
    // 应用Ackerman百分比
    double perfect_diff = inner_angle - outer_angle;
    double actual_diff = perfect_diff * config_.ackerman_percentage;
    
    // 计算实际内外轮角度
    if (average_angle > 0) {
        // 左转：左轮为内轮
        return {average_angle + actual_diff / 2.0,
                average_angle - actual_diff / 2.0};
    } else {
        // 右转：右轮为内轮
        return {average_angle - actual_diff / 2.0,
                average_angle + actual_diff / 2.0};
    }
}

// ============================================================================
// 助力扭矩计算
// ============================================================================

double Steering::calculateAssistTorque(double driver_torque, double vehicle_speed) {
    // 基础助力增益
    double assist_gain = 1.0;
    
    switch (config_.assist_mode) {
        case AssistMode::COMFORT:
            assist_gain = 1.5;  // 轻便
            break;
        case AssistMode::NORMAL:
            assist_gain = 1.0;  // 正常
            break;
        case AssistMode::SPORT:
            assist_gain = 0.7;  // 沉重
            break;
        case AssistMode::ADAPTIVE:
            // 根据车速自适应
            // 低速轻便，高速沉稳
            assist_gain = 1.5 - 0.8 * std::min(vehicle_speed / 30.0, 1.0);
            break;
    }
    
    // 速度相关的助力曲线
    // 低速时助力大，高速时助力小
    double speed_factor = 1.0;
    if (vehicle_speed > 1.0) {
        speed_factor = 1.0 / (1.0 + 0.05 * vehicle_speed);
    }
    
    // 助力扭矩
    double assist_torque = driver_torque * assist_gain * speed_factor;
    
    return assist_torque;
}

// ============================================================================
// 反馈扭矩计算
// ============================================================================

double Steering::calculateFeedbackTorque(const SteeringInput& input) const {
    // 路感反馈基于：
    // 1. 轮胎回正力矩
    // 2. 轮胎反馈力
    // 3. 横向加速度
    
    double feedback = 0.0;
    
    // 轮胎回正力矩反馈
    feedback += input.tire_align_torque * config_.feedback_gain * 0.4;
    
    // 轮胎侧向力反馈
    feedback += input.tire_feedback_force * config_.feedback_gain * 0.005;
    
    // 横向加速度反馈（模拟离心力）
    feedback += input.lateral_acceleration * config_.feedback_gain * 0.02;
    
    return feedback;
}

// ============================================================================
// 回正扭矩计算
// ============================================================================

double Steering::calculateReturnTorque(const SteeringInput& input) {
    if (!active_return_enabled_) {
        return 0.0;
    }
    
    // 主动回正扭矩
    double return_torque = 0.0;
    
    // 基于主销后倾角的回正力矩
    // M_align = Fz * c * sin(caster) * sin(δ)
    double normal_force = 4000.0;  // 前轴载荷 (N)
    return_torque += normal_force * config_.kingpin_offset 
                     * std::sin(config_.caster_angle) 
                     * std::sin(state_.wheel_angle_average);
    
    // 基于车速的回正增益
    // 低速时回正弱，高速时回正强
    double speed_gain = std::min(input.vehicle_speed / 10.0, 1.0);
    return_torque *= (0.3 + 0.7 * speed_gain);
    
    // 回正速度限制
    double max_return_torque = config_.return_speed * config_.column_stiffness;
    return_torque = std::clamp(return_torque, -max_return_torque, max_return_torque);
    
    // 只在驾驶员松手时回正
    if (std::abs(input.driver_torque) < 0.5) {
        state_.is_returning = true;
        return return_torque;
    } else {
        state_.is_returning = false;
        return 0.0;
    }
}

// ============================================================================
// 电机更新
// ============================================================================

void Steering::updateMotor(double dt, double target_torque) {
    // 电机动力学模型（一阶惯性环节）
    double tau = 0.01;  // 电机时间常数 (s)
    double alpha = dt / (tau + dt);
    
    state_.motor_torque = state_.motor_torque 
                         + alpha * (target_torque - state_.motor_torque);
    
    // 电机角度（通过转向器传动比）
    state_.motor_angle = state_.steering_wheel_angle * config_.gear_ratio;
    
    // 电机角速度
    state_.motor_speed = state_.steering_wheel_speed * config_.gear_ratio;
}

// ============================================================================
// 转向柱更新
// ============================================================================

void Steering::updateSteeringColumn(double dt, double driver_torque) {
    // 转向柱动力学模型
    // J * θ̈ + c * θ̇ + k * θ = T_driver + T_motor / ratio
    
    double J = config_.motor_inertia;  // 等效转动惯量
    double c = config_.column_damping;  // 阻尼
    double k = config_.column_stiffness;  // 刚度
    
    // 扭矩
    double total_torque = driver_torque + state_.motor_torque / config_.gear_ratio;
    
    // 角加速度
    double angular_acceleration = (total_torque 
                                   - c * state_.steering_wheel_speed 
                                   - k * state_.steering_wheel_angle) / J;
    
    // 积分更新
    state_.steering_wheel_speed += angular_acceleration * dt;
    state_.steering_wheel_angle += state_.steering_wheel_speed * dt;
    
    // 限制转向盘角度
    double max_angle = 2.0 * M_PI;  // ±2圈
    state_.steering_wheel_angle = std::clamp(state_.steering_wheel_angle,
                                              -max_angle,
                                              max_angle);
    
    // 转向盘扭矩（反馈给驾驶员）
    state_.steering_wheel_torque = driver_torque;
}

// ============================================================================
// 摩擦补偿
// ============================================================================

double Steering::frictionCompensation(double speed) const {
    // 库伦摩擦 + 粘性摩擦
    double coulomb_friction = config_.friction_compensation * 
                              (speed > 0 ? 1.0 : (speed < 0 ? -1.0 : 0.0));
    
    double viscous_friction = 0.1 * speed;  // 粘性摩擦系数
    
    return coulomb_friction + viscous_friction;
}

// ============================================================================
// 设置函数
// ============================================================================

void Steering::setSteeringWheelAngle(double angle) {
    if (config_.type == SteeringType::STEER_BY_WIRE) {
        state_.steering_wheel_angle = std::clamp(angle, -2.0 * M_PI, 2.0 * M_PI);
    }
}

void Steering::setAssistMode(AssistMode mode) {
    config_.assist_mode = mode;
}

void Steering::setSteeringRatio(double ratio) {
    if (ratio > 0) {
        manual_steering_ratio_ = ratio;
    } else {
        manual_steering_ratio_ = 0.0;  // 恢复自动模式
    }
}

void Steering::enableActiveReturn(bool enable) {
    active_return_enabled_ = enable;
}

void Steering::reset() {
    initialize();
}

} // namespace chassis
} // namespace ev_dynamics
