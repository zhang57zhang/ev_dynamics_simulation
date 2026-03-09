/**
 * @file suspension.cpp
 * @brief 主动悬架系统实现
 * @author CodeCraft
 * @date 2026-03-08
 */

#include "suspension.h"
#include <algorithm>
#include <cmath>

namespace ev_dynamics {
namespace chassis {

// ============================================================================
// 构造函数和初始化
// ============================================================================

Suspension::Suspension(const SuspensionConfig& config)
    : config_(config)
    , target_height_(config.height_normal)
    , current_damping_mode_(config.damping_mode)
    , adaptive_damping_factor_(0.5)
    , height_kp_(10000.0)
    , height_ki_(100.0)
    , height_kd_(500.0)
    , height_integral_(0.0)
    , height_prev_error_(0.0)
{
    prev_road_input_.fill(0.0);
    filtered_velocity_.fill(0.0);
}

void Suspension::initialize() {
    // 初始化状态
    state_ = SuspensionState();
    state_.body_height = config_.height_normal;
    
    // 设置初始阻尼模式
    current_damping_mode_ = config_.damping_mode;
    adaptive_damping_factor_ = config_.comfort_weight;
    
    // 重置控制器状态
    height_integral_ = 0.0;
    height_prev_error_ = 0.0;
    prev_road_input_.fill(0.0);
    filtered_velocity_.fill(0.0);
}

// ============================================================================
// 主更新函数
// ============================================================================

void Suspension::update(double dt, const SuspensionInput& input) {
    // 1. 更新路面输入速度
    std::array<double, 4> road_inputs = {
        input.road_fl, input.road_fr, input.road_rl, input.road_rr
    };
    
    // 2. 更新四个悬架角
    updateCorner(config_.front_left, state_.front_left, road_inputs[0], dt);
    updateCorner(config_.front_right, state_.front_right, road_inputs[1], dt);
    updateCorner(config_.rear_left, state_.rear_left, road_inputs[2], dt);
    updateCorner(config_.rear_right, state_.rear_right, road_inputs[3], dt);
    
    // 3. 更新车身姿态
    updateBodyPose();
    
    // 4. 自适应阻尼控制
    if (current_damping_mode_ == DampingMode::ADAPTIVE) {
        adaptiveDampingControl(input);
    }
    
    // 5. 高度调节控制
    if (config_.type == SuspensionType::ACTIVE) {
        heightControl(input.target_height, dt);
    }
    
    // 6. 计算性能指标
    state_.comfort_index = calculateComfortIndex();
    state_.handling_index = calculateHandlingIndex();
    
    // 7. 保存前一时刻状态
    prev_road_input_ = road_inputs;
}

// ============================================================================
// 悬架角更新
// ============================================================================

void Suspension::updateCorner(const SuspensionCornerConfig& corner_config,
                              SuspensionCornerState& corner_state,
                              double road_input,
                              double dt) {
    // 更新路面输入
    corner_state.road_displacement = road_input;
    corner_state.road_velocity = (road_input - corner_state.road_displacement) / dt;
    
    // 计算轮胎变形和力
    double tire_relative_displacement = road_input - corner_state.tire_deflection;
    corner_state.tire_force = corner_config.tire_stiffness * tire_relative_displacement
                              + corner_config.tire_damping * corner_state.road_velocity;
    corner_state.tire_deflection = tire_relative_displacement;
    
    // 计算弹簧力（考虑非线性）
    corner_state.spring_force = calculateSpringForce(corner_config,
                                                      corner_state.displacement);
    
    // 计算阻尼力
    corner_state.damping_force = calculateDampingForce(corner_config,
                                                        corner_state.velocity,
                                                        current_damping_mode_);
    
    // 计算主动作动器力
    if (config_.type == SuspensionType::ACTIVE) {
        corner_state.actuator_force = calculateActuatorForce(corner_state, dt);
    } else {
        corner_state.actuator_force = 0.0;
    }
    
    // 总力
    corner_state.total_force = corner_state.spring_force 
                               + corner_state.damping_force
                               + corner_state.actuator_force;
    
    // 簧上质量运动（二自由度模型）
    double sprung_acceleration = -corner_state.total_force / corner_config.sprung_mass;
    
    // 簧下质量运动
    double unsprung_force = corner_state.tire_force - corner_state.total_force;
    double unsprung_acceleration = unsprung_force / corner_config.unsprung_mass;
    
    // 积分更新（欧拉法）
    corner_state.velocity += unsprung_acceleration * dt;
    corner_state.displacement += corner_state.velocity * dt;
    corner_state.acceleration = unsprung_acceleration;
    
    // 限制位移范围
    double min_disp = corner_config.min_length - corner_config.installed_length;
    double max_disp = corner_config.max_length - corner_config.installed_length;
    corner_state.displacement = std::clamp(corner_state.displacement, min_disp, max_disp);
}

// ============================================================================
// 弹簧力计算
// ============================================================================

double Suspension::calculateSpringForce(const SuspensionCornerConfig& config,
                                        double displacement) const {
    // 非线性弹簧模型：F = k*x + k_nl*x^3
    double linear_force = config.spring_stiffness * displacement;
    double nonlinear_force = config.spring_nonlinear_coeff 
                             * std::pow(displacement, 3);
    
    // 考虑预紧力
    return linear_force + nonlinear_force + config.spring_preload;
}

// ============================================================================
// 阻尼力计算
// ============================================================================

double Suspension::calculateDampingForce(const SuspensionCornerConfig& config,
                                         double velocity,
                                         DampingMode mode) {
    // 基础阻尼系数
    double base_damping = (velocity > 0) ? config.damping_compression 
                                         : config.damping_rebound;
    
    // 根据模式调整阻尼
    double damping_multiplier = 1.0;
    
    switch (mode) {
        case DampingMode::COMFORT:
            damping_multiplier = 0.6;
            break;
        case DampingMode::NORMAL:
            damping_multiplier = 1.0;
            break;
        case DampingMode::SPORT:
            damping_multiplier = 1.5;
            break;
        case DampingMode::ADAPTIVE:
            damping_multiplier = 0.6 + 0.9 * (1.0 - adaptive_damping_factor_);
            break;
    }
    
    // 可变阻尼限制
    double effective_damping = std::clamp(base_damping * damping_multiplier,
                                          config.damping_min,
                                          config.damping_max);
    
    return effective_damping * velocity;
}

// ============================================================================
// 主动作动器力计算
// ============================================================================

double Suspension::calculateActuatorForce(SuspensionCornerState& corner_state, 
                                          double dt) {
    // 天棚阻尼控制 + 地面钩控制混合
    double skyhook_force = skyhookDampingControl(corner_state);
    double groundhook_force = groundhookDampingControl(corner_state);
    
    // 混合控制（根据性能权重）
    double target_force = config_.comfort_weight * skyhook_force
                         + config_.handling_weight * groundhook_force;
    
    // 作动器动力学（一阶滞后）
    double tau = 1.0 / (2.0 * M_PI * config_.front_left.actuator_bandwidth);
    double alpha = dt / (tau + dt);
    
    double actuator_force = corner_state.actuator_force 
                           + alpha * (target_force - corner_state.actuator_force);
    
    // 限制作动力
    return std::clamp(actuator_force,
                      -config_.front_left.actuator_force_max,
                      config_.front_left.actuator_force_max);
}

// ============================================================================
// 天棚阻尼控制
// ============================================================================

double Suspension::skyhookDampingControl(const SuspensionCornerState& corner_state) const {
    // 天棚阻尼：基于簧上质量绝对速度
    double skyhook_gain = 2000.0;  // 天棚阻尼增益
    
    // 估计簧上质量绝对速度（简化的低通滤波）
    double sprung_velocity = corner_state.velocity;
    
    return -skyhook_gain * sprung_velocity;
}

// ============================================================================
// 地面钩阻尼控制
// ============================================================================

double Suspension::groundhookDampingControl(const SuspensionCornerState& corner_state) const {
    // 地面钩阻尼：基于簧下质量绝对速度
    double groundhook_gain = 1500.0;  // 地面钩阻尼增益
    
    // 估计簧下质量绝对速度
    double unsprung_velocity = corner_state.road_velocity;
    
    return -groundhook_gain * unsprung_velocity;
}

// ============================================================================
// 车身姿态更新
// ============================================================================

void Suspension::updateBodyPose() {
    // 车身高度（四轮平均）
    double fl_height = config_.front_left.installed_length + state_.front_left.displacement;
    double fr_height = config_.front_right.installed_length + state_.front_right.displacement;
    double rl_height = config_.rear_left.installed_length + state_.rear_left.displacement;
    double rr_height = config_.rear_right.installed_length + state_.rear_right.displacement;
    
    state_.body_height = (fl_height + fr_height + rl_height + rr_height) / 4.0;
    
    // 计算轴距和轮距
    double wheelbase = 2.8;   // 轴距 (m)
    double track_width = 1.6; // 轮距 (m)
    
    // 俯仰角（前-后）
    double front_height = (fl_height + fr_height) / 2.0;
    double rear_height = (rl_height + rr_height) / 2.0;
    state_.body_pitch = std::atan((front_height - rear_height) / wheelbase);
    
    // 侧倾角（左-右）
    double left_height = (fl_height + rl_height) / 2.0;
    double right_height = (fr_height + rr_height) / 2.0;
    state_.body_roll = std::atan((left_height - right_height) / track_width);
    
    // 俯仰和侧倾角速度（简化估计）
    static double prev_pitch = 0.0;
    static double prev_roll = 0.0;
    
    // 使用数值微分（实际应用中应使用状态观测器）
    state_.pitch_rate = state_.body_pitch - prev_pitch;
    state_.roll_rate = state_.body_roll - prev_roll;
    
    prev_pitch = state_.body_pitch;
    prev_roll = state_.body_roll;
    
    // 垂向加速度（四轮平均）
    state_.vertical_acceleration = (state_.front_left.acceleration 
                                   + state_.front_right.acceleration
                                   + state_.rear_left.acceleration 
                                   + state_.rear_right.acceleration) / 4.0;
}

// ============================================================================
// 舒适性指标
// ============================================================================

double Suspension::calculateComfortIndex() const {
    // 基于ISO 2631标准的简化舒适性指标
    // 考虑垂向加速度RMS值
    
    double acc_rms = std::abs(state_.vertical_acceleration);
    
    // 归一化到0-1范围
    // 0.3 m/s² 以下为舒适，1.0 m/s² 以上为不舒适
    double comfort = 1.0 - std::clamp((acc_rms - 0.3) / 0.7, 0.0, 1.0);
    
    return comfort;
}

// ============================================================================
// 操控性指标
// ============================================================================

double Suspension::calculateHandlingIndex() const {
    // 操控性指标基于：
    // 1. 侧倾角大小
    // 2. 轮胎载荷变化
    
    // 侧倾角影响（小侧倾为好）
    double roll_factor = 1.0 - std::abs(state_.body_roll) / 0.1;  // 0.1 rad为阈值
    
    // 轮胎载荷变化（小变化为好）
    double load_variation = std::abs(state_.front_left.tire_force 
                                    - state_.front_right.tire_force)
                           + std::abs(state_.rear_left.tire_force 
                                    - state_.rear_right.tire_force);
    double load_factor = 1.0 - std::clamp(load_variation / 10000.0, 0.0, 1.0);
    
    return std::clamp((roll_factor + load_factor) / 2.0, 0.0, 1.0);
}

// ============================================================================
// 高度调节控制
// ============================================================================

void Suspension::heightControl(double target_height, double dt) {
    // 限制目标高度范围
    target_height = std::clamp(target_height,
                               config_.height_min,
                               config_.height_max);
    
    // PID控制
    double error = target_height - state_.body_height;
    height_integral_ += error * dt;
    height_integral_ = std::clamp(height_integral_, -0.01, 0.01);  // 抗积分饱和
    
    double derivative = (error - height_prev_error_) / dt;
    
    double control_force = height_kp_ * error 
                         + height_ki_ * height_integral_
                         + height_kd_ * derivative;
    
    // 分配到四个悬架角
    state_.front_left.actuator_force += control_force / 4.0;
    state_.front_right.actuator_force += control_force / 4.0;
    state_.rear_left.actuator_force += control_force / 4.0;
    state_.rear_right.actuator_force += control_force / 4.0;
    
    // 限制作动力
    double max_force = config_.front_left.actuator_force_max;
    state_.front_left.actuator_force = std::clamp(state_.front_left.actuator_force,
                                                   -max_force, max_force);
    state_.front_right.actuator_force = std::clamp(state_.front_right.actuator_force,
                                                    -max_force, max_force);
    state_.rear_left.actuator_force = std::clamp(state_.rear_left.actuator_force,
                                                  -max_force, max_force);
    state_.rear_right.actuator_force = std::clamp(state_.rear_right.actuator_force,
                                                   -max_force, max_force);
    
    height_prev_error_ = error;
    target_height_ = target_height;
}

// ============================================================================
// 自适应阻尼控制
// ============================================================================

void Suspension::adaptiveDampingControl(const SuspensionInput& input) {
    // 根据驾驶工况自适应调整阻尼系数
    
    // 1. 路面粗糙度估计（基于路面输入的RMS）
    double road_rms = std::sqrt((std::pow(input.road_fl, 2) 
                                + std::pow(input.road_fr, 2)
                                + std::pow(input.road_rl, 2) 
                                + std::pow(input.road_rr, 2)) / 4.0);
    
    // 2. 横向加速度（反映弯道工况）
    double lateral_factor = std::abs(input.lateral_acceleration) / 5.0;  // 5 m/s²为阈值
    
    // 3. 纵向加速度（反映加减速工况）
    double longitudinal_factor = std::abs(input.longitudinal_acceleration) / 3.0;  // 3 m/s²为阈值
    
    // 4. 车速因子
    double speed_factor = input.vehicle_speed / 30.0;  // 30 m/s为参考速度
    
    // 5. 综合自适应系数
    // - 路面粗糙：增加舒适性权重
    // - 横向加速度大：增加操控性权重
    // - 高速：增加操控性权重
    
    double comfort_weight = config_.comfort_weight;
    
    // 路面粗糙度影响
    comfort_weight += road_rms * 0.3;
    
    // 横向加速度影响（降低舒适性权重，提高操控性）
    comfort_weight -= lateral_factor * 0.2;
    
    // 纵向加速度影响
    comfort_weight -= longitudinal_factor * 0.1;
    
    // 车速影响
    comfort_weight -= speed_factor * 0.1;
    
    // 限制范围
    adaptive_damping_factor_ = std::clamp(comfort_weight, 0.0, 1.0);
}

// ============================================================================
// 设置函数
// ============================================================================

void Suspension::setTargetHeight(double height) {
    target_height_ = std::clamp(height, config_.height_min, config_.height_max);
}

void Suspension::setDampingMode(DampingMode mode) {
    current_damping_mode_ = mode;
}

void Suspension::setPerformanceWeights(double comfort, double handling) {
    config_.comfort_weight = std::clamp(comfort, 0.0, 1.0);
    config_.handling_weight = std::clamp(handling, 0.0, 1.0);
    
    // 归一化
    double total = config_.comfort_weight + config_.handling_weight;
    if (total > 0) {
        config_.comfort_weight /= total;
        config_.handling_weight /= total;
    }
}

void Suspension::reset() {
    initialize();
}

} // namespace chassis
} // namespace ev_dynamics
