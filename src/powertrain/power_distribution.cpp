/**
 * @file power_distribution.cpp
 * @brief 功率分配策略实现
 * @author CodeCraft
 * @date 2026-03-08
 */

#include "power_distribution.h"
#include <algorithm>
#include <iostream>

namespace ev_dynamics {
namespace powertrain {

PowerDistribution::PowerDistribution(const PowerDistributionConfig& config)
    : config_(config)
{
    initialize();
}

void PowerDistribution::initialize() {
    state_ = PowerDistributionState();
    state_.front_ratio = config_.base_front_ratio;
    
    std::cout << "[PowerDistribution] Initialized: front_ratio=" 
              << (config_.base_front_ratio * 100) << "%" << std::endl;
}

void PowerDistribution::update(double dt, const PowerDistributionInput& input) {
    state_.total_torque = input.total_torque_request;
    
    // 1. 应用稳定性控制
    applyStabilityControl(input.lateral_acceleration, 
                         input.yaw_rate, 
                         input.steering_angle);
    
    // 2. 计算扭矩分配
    if (input.regenerative_braking) {
        calculateRegenerativeBraking(input.total_torque_request, input.vehicle_speed);
    } else {
        calculateTorqueDistribution(input.total_torque_request);
    }
    
    // 3. 限制分配
    limitDistribution();
}

void PowerDistribution::calculateTorqueDistribution(double total_torque) {
    // 动态扭矩分配策略：
    // 1. 基础分配：根据载荷比
    // 2. 动态调整：根据稳定性修正
    // 3. 限制：在[min, max]范围内
    
    double adjusted_front_ratio = config_.base_front_ratio + state_.stability_correction;
    
    // 限制分配比
    adjusted_front_ratio = std::clamp(adjusted_front_ratio,
                                      config_.min_front_ratio,
                                      config_.max_front_ratio);
    
    state_.front_ratio = adjusted_front_ratio;
    
    // 计算前后扭矩
    state_.front_torque = total_torque * adjusted_front_ratio;
    state_.rear_torque = total_torque * (1.0 - adjusted_front_ratio);
}

void PowerDistribution::calculateRegenerativeBraking(double total_torque, double vehicle_speed) {
    // 能量回收策略：
    // 1. 低速时减少回收（避免电机效率低）
    // 2. 高速时增加回收
    // 3. 根据车速动态调整
    
    if (vehicle_speed < config_.regen_min_speed) {
        // 低速时不回收
        state_.regen_ratio = 0.0;
        state_.regen_torque = 0.0;
        state_.front_torque = total_torque * config_.base_front_ratio;
        state_.rear_torque = total_torque * (1.0 - config_.base_front_ratio);
        return;
    }
    
    // 根据车速计算回收比例（简化模型）
    double speed_factor = std::min(vehicle_speed / 20.0, 1.0);  // 20m/s达到最大
    state_.regen_ratio = config_.regen_base_ratio + 
                        (config_.regen_max_ratio - config_.regen_base_ratio) * speed_factor;
    
    // 回收扭矩（负值）
    state_.regen_torque = -std::abs(total_torque) * state_.regen_ratio;
    
    // 前后分配（回收时主要用前轮）
    double regen_front_ratio = 0.6;  // 60%前轮回收
    state_.front_torque = state_.regen_torque * regen_front_ratio;
    state_.rear_torque = state_.regen_torque * (1.0 - regen_front_ratio);
}

void PowerDistribution::applyStabilityControl(double lateral_acc, double yaw_rate, double steering) {
    // 稳定性控制：
    // 1. 检测转向不足/过度
    // 2. 调整扭矩分配
    // 3. 提高稳定性
    
    // 计算转向特性
    double steer_characteristic = lateral_acc / (std::abs(yaw_rate) + 0.001);
    
    if (steer_characteristic > config_.understeer_threshold) {
        // 转向不足：增加后轴扭矩
        state_.stability_correction = -config_.stability_gain * 0.1;
        std::cout << "[PowerDistribution] Understeer detected, adjusting torque" << std::endl;
    } else if (steer_characteristic < config_.oversteer_threshold) {
        // 转向过度：增加前轴扭矩
        state_.stability_correction = config_.stability_gain * 0.1;
        std::cout << "[PowerDistribution] Oversteer detected, adjusting torque" << std::endl;
    } else {
        // 正常：逐渐恢复基础分配
        state_.stability_correction *= 0.95;
    }
}

void PowerDistribution::limitDistribution() {
    // 确保扭矩分配在合理范围内
    state_.front_ratio = std::clamp(state_.front_ratio,
                                   config_.min_front_ratio,
                                   config_.max_front_ratio);
    
    state_.regen_ratio = std::clamp(state_.regen_ratio, 0.0, config_.regen_max_ratio);
}

void PowerDistribution::reset() {
    state_ = PowerDistributionState();
    state_.front_ratio = config_.base_front_ratio;
    std::cout << "[PowerDistribution] Reset to initial state" << std::endl;
}

} // namespace powertrain
} // namespace ev_dynamics
