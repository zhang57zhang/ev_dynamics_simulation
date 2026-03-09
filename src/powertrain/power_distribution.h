/**
 * @file power_distribution.h
 * @brief 功率分配策略
 * @author CodeCraft
 * @date 2026-03-08
 * 
 * 实现功率分配策略，包括：
 * - 前后扭矩分配
 * - 能量回收策略
 * - 动态分配算法
 * - 稳定性控制
 */

#ifndef EV_DYNAMICS_POWER_DISTRIBUTION_H
#define EV_DYNAMICS_POWER_DISTRIBUTION_H

#include <cmath>
#include <Eigen/Dense>

namespace ev_dynamics {
namespace powertrain {

using namespace Eigen;

/**
 * @brief 功率分配配置
 */
struct PowerDistributionConfig {
    // 车辆参数
    double front_weight_ratio;      // 前轴载荷比
    double rear_weight_ratio;       // 后轴载荷比
    double wheelbase;               // 轴距 (m)
    double track_width;             // 轮距 (m)
    
    // 分配策略
    double base_front_ratio;        // 基础前轴扭矩分配比
    double max_front_ratio;         // 最大前轴分配比
    double min_front_ratio;         // 最小前轴分配比
    
    // 能量回收
    double regen_base_ratio;        // 基础能量回收比例
    double regen_max_ratio;         // 最大能量回收比例
    double regen_min_speed;         // 最小回收速度 (m/s)
    
    // 稳定性控制
    double understeer_threshold;    // 转向不足阈值
    double oversteer_threshold;     // 转向过度阈值
    double stability_gain;          // 稳定性增益
    
    PowerDistributionConfig()
        : front_weight_ratio(0.5)
        , rear_weight_ratio(0.5)
        , wheelbase(2.7)
        , track_width(1.6)
        , base_front_ratio(0.5)
        , max_front_ratio(0.7)
        , min_front_ratio(0.3)
        , regen_base_ratio(0.3)
        , regen_max_ratio(0.5)
        , regen_min_speed(2.0)
        , understeer_threshold(0.1)
        , oversteer_threshold(-0.1)
        , stability_gain(0.5)
    {}
};

/**
 * @brief 功率分配状态
 */
struct PowerDistributionState {
    double front_torque;            // 前轴扭矩 (Nm)
    double rear_torque;             // 后轴扭矩 (Nm)
    double front_ratio;             // 前轴分配比
    double regen_torque;            // 回收扭矩 (Nm)
    double regen_ratio;             // 回收比例
    double stability_correction;    // 稳定性修正
    double total_torque;            // 总扭矩 (Nm)
    
    PowerDistributionState()
        : front_torque(0.0)
        , rear_torque(0.0)
        , front_ratio(0.5)
        , regen_torque(0.0)
        , regen_ratio(0.0)
        , stability_correction(0.0)
        , total_torque(0.0)
    {}
};

/**
 * @brief 功率分配输入
 */
struct PowerDistributionInput {
    double total_torque_request;    // 总扭矩请求 (Nm)
    double vehicle_speed;           // 车速 (m/s)
    double lateral_acceleration;    // 横向加速度 (m/s²)
    double yaw_rate;                // 横摆角速度 (rad/s)
    double steering_angle;          // 转向角 (rad)
    bool regenerative_braking;      // 是否能量回收
    
    PowerDistributionInput()
        : total_torque_request(0.0)
        , vehicle_speed(0.0)
        , lateral_acceleration(0.0)
        , yaw_rate(0.0)
        , steering_angle(0.0)
        , regenerative_braking(false)
    {}
};

/**
 * @brief 功率分配策略类
 */
class PowerDistribution {
public:
    explicit PowerDistribution(const PowerDistributionConfig& config = PowerDistributionConfig());
    
    void initialize();
    void update(double dt, const PowerDistributionInput& input);
    
    const PowerDistributionState& getState() const { return state_; }
    const PowerDistributionConfig& getConfig() const { return config_; }
    void reset();

private:
    void calculateTorqueDistribution(double total_torque);
    void calculateRegenerativeBraking(double total_torque, double vehicle_speed);
    void applyStabilityControl(double lateral_acc, double yaw_rate, double steering);
    void limitDistribution();

private:
    PowerDistributionConfig config_;
    PowerDistributionState state_;
};

} // namespace powertrain
} // namespace ev_dynamics

#endif // EV_DYNAMICS_POWER_DISTRIBUTION_H
