/**
 * @file braking.h
 * @brief 线控制动系统（EMB）模型
 * @author CodeCraft
 * @date 2026-03-08
 * 
 * 实现线控制动系统，包括：
 * - 电控机械制动（EMB）
 * - 电控液压制动（EHB）
 * - ABS防抱死算法
 * - EBD制动力分配
 * - 制动能量回收协调
 */

#ifndef EV_DYNAMICS_BRAKING_H
#define EV_DYNAMICS_BRAKING_H

#include <cmath>
#include <vector>
#include <array>
#include <Eigen/Dense>

namespace ev_dynamics {
namespace chassis {

using namespace Eigen;

/**
 * @brief 制动类型枚举
 */
enum class BrakingType {
    HYDRAULIC,          ///< 传统液压制动
    ELECTRO_HYDRAULIC,  ///< 电控液压制动（EHB）
    ELECTRO_MECHANICAL  ///< 电控机械制动（EMB）
};

/**
 * @brief ABS状态枚举
 */
enum class ABSState {
    INACTIVE,           ///< 未激活
    PRESSURE_HOLD,      ///< 保压
    PRESSURE_DECREASE,  ///< 减压
    PRESSURE_INCREASE   ///< 增压
};

/**
 * @brief 单轮制动配置
 */
struct BrakeCornerConfig {
    // 制动器参数
    double max_brake_torque;        // 最大制动力矩 (Nm)
    double brake_response_time;     // 制动响应时间 (s)
    double brake_gain;              // 制动增益 (Nm/MPa for hydraulic, Nm/A for EMB)
    
    // 盘式制动器参数
    double disc_radius;             // 制动盘半径 (m)
    double pad_friction_coeff;      // 摩擦片摩擦系数
    double piston_area;             // 活塞面积 (m²)
    double max_pressure;            // 最大制动压力 (MPa)
    
    // EMB电机参数
    double motor_torque_constant;   // 电机扭矩常数 (Nm/A)
    double motor_max_current;       // 电机最大电流 (A)
    float gear_ratio;               // 减速比
    float ball_screw_pitch;         // 滚珠丝杠导程 (m)
    
    // 热参数
    double thermal_mass;            // 热容 (J/K)
    double thermal_resistance;      // 热阻 (K/W)
    double max_temperature;         // 最高温度 (°C)
    
    BrakeCornerConfig()
        : max_brake_torque(3000.0)
        , brake_response_time(0.05)
        , brake_gain(100.0)
        , disc_radius(0.15)
        , pad_friction_coeff(0.4)
        , piston_area(0.003)
        , max_pressure(15.0)
        , motor_torque_constant(0.1)
        , motor_max_current(30.0)
        , gear_ratio(10.0f)
        , ball_screw_pitch(0.005f)
        , thermal_mass(500.0)
        , thermal_resistance(0.05)
        , max_temperature(800.0)
    {}
};

/**
 * @brief 完整制动系统配置
 */
struct BrakingConfig {
    BrakeCornerConfig front_left;
    BrakeCornerConfig front_right;
    BrakeCornerConfig rear_left;
    BrakeCornerConfig rear_right;
    
    // 系统参数
    BrakingType type;               // 制动类型
    double front_rear_distribution; // 前后制动力分配 (0-1, 前轴比例)
    double pedal_ratio;             // 制动踏板传动比
    
    // ABS参数
    bool abs_enabled;               // ABS使能
    double slip_ratio_min;          // 最小滑移率
    double slip_ratio_max;          // 最大滑移率
    double slip_ratio_target;       // 目标滑移率
    double abs_cycle_frequency;     // ABS循环频率 (Hz)
    
    // EBD参数
    bool ebd_enabled;               // EBD使能
    double ebd_gain;                // EBD调节增益
    
    // 制动能量回收
    bool regen_enabled;             // 再生制动使能
    double regen_blend_factor;      // 再生制动比例 (0-1)
    double max_regen_torque;        // 最大再生制动扭矩 (Nm)
    
    // 车辆参数
    double vehicle_mass;            // 整车质量 (kg)
    double wheelbase;               // 轴距 (m)
    double cg_height;               // 质心高度 (m)
    double wheel_radius;            // 车轮半径 (m)
    
    BrakingConfig()
        : type(BrakingType::ELECTRO_HYDRAULIC)
        , front_rear_distribution(0.6)
        , pedal_ratio(4.0)
        , abs_enabled(true)
        , slip_ratio_min(0.08)
        , slip_ratio_max(0.30)
        , slip_ratio_target(0.15)
        , abs_cycle_frequency(15.0)
        , ebd_enabled(true)
        , ebd_gain(0.1)
        , regen_enabled(true)
        , regen_blend_factor(0.3)
        , max_regen_torque(500.0)
        , vehicle_mass(1800.0)
        , wheelbase(2.8)
        , cg_height(0.55)
        , wheel_radius(0.33)
    {}
};

/**
 * @brief 单轮制动状态
 */
struct BrakeCornerState {
    double brake_pressure;          // 制动压力 (MPa) 或电流 (A)
    double brake_torque;            // 制动力矩 (Nm)
    double brake_force;             // 制动力 (N)
    double wheel_slip;              // 滑移率
    double wheel_speed;             // 轮速 (rad/s)
    double disc_temperature;        // 制动盘温度 (°C)
    
    // ABS状态
    ABSState abs_state;             // ABS状态
    double abs_cycle_timer;         // ABS循环计时器
    double target_pressure;         // 目标压力
    
    // EMB状态
    double motor_current;           // 电机电流 (A)
    double pad_wear;                // 摩擦片磨损 (%)
    
    BrakeCornerState()
        : brake_pressure(0.0)
        , brake_torque(0.0)
        , brake_force(0.0)
        , wheel_slip(0.0)
        , wheel_speed(0.0)
        , disc_temperature(25.0)
        , abs_state(ABSState::INACTIVE)
        , abs_cycle_timer(0.0)
        , target_pressure(0.0)
        , motor_current(0.0)
        , pad_wear(0.0)
    {}
};

/**
 * @brief 完整制动系统状态
 */
struct BrakingState {
    BrakeCornerState front_left;
    BrakeCornerState front_right;
    BrakeCornerState rear_left;
    BrakeCornerState rear_right;
    
    // 系统状态
    double brake_pedal_position;    // 制动踏板位置 (0-1)
    double total_brake_torque;      // 总制动力矩 (Nm)
    double total_brake_force;       // 总制动力 (N)
    double deceleration;            // 减速度 (m/s²)
    
    // 制动分配
    double front_brake_ratio;       // 前轴制动力比例
    double rear_brake_ratio;        // 后轴制动力比例
    
    // 再生制动
    double regen_torque;            // 再生制动力矩 (Nm)
    double friction_torque;         // 摩擦制动力矩 (Nm)
    
    // ABS状态
    bool abs_active;                // ABS是否激活
    int abs_active_wheels;          // ABS激活的车轮数量
    
    // EBD状态
    double ebd_correction;          // EBD修正量
    
    BrakingState()
        : brake_pedal_position(0.0)
        , total_brake_torque(0.0)
        , total_brake_force(0.0)
        , deceleration(0.0)
        , front_brake_ratio(0.6)
        , rear_brake_ratio(0.4)
        , regen_torque(0.0)
        , friction_torque(0.0)
        , abs_active(false)
        , abs_active_wheels(0)
        , ebd_correction(0.0)
    {}
};

/**
 * @brief 制动系统输入
 */
struct BrakingInput {
    // 驾驶员输入
    double brake_pedal_position;    // 制动踏板位置 (0-1)
    double brake_pedal_force;       // 制动踏板力 (N)
    
    // 车辆状态
    double vehicle_speed;           // 车速 (m/s)
    double longitudinal_acceleration; // 纵向加速度 (m/s²)
    
    // 轮速（四轮）
    double wheel_speed_fl;          // 前左轮速 (rad/s)
    double wheel_speed_fr;          // 前右轮速 (rad/s)
    double wheel_speed_rl;          // 后左轮速 (rad/s)
    double wheel_speed_rr;          // 后右轮速 (rad/s)
    
    // 路面条件
    double road_friction_coeff;     // 路面附着系数
    
    // 再生制动请求
    double regen_request;           // 再生制动请求 (0-1)
    
    BrakingInput()
        : brake_pedal_position(0.0)
        , brake_pedal_force(0.0)
        , vehicle_speed(0.0)
        , longitudinal_acceleration(0.0)
        , wheel_speed_fl(0.0)
        , wheel_speed_fr(0.0)
        , wheel_speed_rl(0.0)
        , wheel_speed_rr(0.0)
        , road_friction_coeff(1.0)
        , regen_request(0.0)
    {}
};

/**
 * @brief 线控制动系统类
 */
class Braking {
public:
    /**
     * @brief 构造函数
     * @param config 制动配置参数
     */
    explicit Braking(const BrakingConfig& config = BrakingConfig());
    
    /**
     * @brief 初始化制动系统
     */
    void initialize();
    
    /**
     * @brief 更新制动状态（时间步进）
     * @param dt 时间步长 (s)
     * @param input 输入参数
     */
    void update(double dt, const BrakingInput& input);
    
    /**
     * @brief 获取当前状态
     * @return 制动状态
     */
    const BrakingState& getState() const { return state_; }
    
    /**
     * @brief 获取配置参数
     * @return 制动配置
     */
    const BrakingConfig& getConfig() const { return config_; }
    
    /**
     * @brief 启用/禁用ABS
     * @param enable 是否启用
     */
    void enableABS(bool enable);
    
    /**
     * @brief 启用/禁用EBD
     * @param enable 是否启用
     */
    void enableEBD(bool enable);
    
    /**
     * @brief 启用/禁用再生制动
     * @param enable 是否启用
     */
    void enableRegen(bool enable);
    
    /**
     * @brief 设置再生制动比例
     * @param factor 再生制动比例 (0-1)
     */
    void setRegenBlendFactor(double factor);
    
    /**
     * @brief 紧急制动
     */
    void emergencyBrake();
    
    /**
     * @brief 释放制动
     */
    void releaseBrake();
    
    /**
     * @brief 重置制动状态
     */
    void reset();

private:
    /**
     * @brief 计算参考车速
     * @param input 输入参数
     * @return 参考车速 (m/s)
     */
    double calculateReferenceSpeed(const BrakingInput& input) const;
    
    /**
     * @brief 计算滑移率
     * @param wheel_speed 轮速 (rad/s)
     * @param vehicle_speed 车速 (m/s)
     * @return 滑移率
     */
    double calculateSlipRatio(double wheel_speed, double vehicle_speed) const;
    
    /**
     * @brief ABS控制算法
     * @param corner_state 车轮制动状态
     * @param corner_config 车轮制动配置
     * @param slip_ratio 滑移率
     * @param dt 时间步长
     */
    void absControl(BrakeCornerState& corner_state,
                    const BrakeCornerConfig& corner_config,
                    double slip_ratio,
                    double dt);
    
    /**
     * @brief EBD制动力分配
     * @param input 输入参数
     */
    void ebdControl(const BrakingInput& input);
    
    /**
     * @brief 制动能量回收协调
     * @param total_torque_request 总制动力矩请求
     * @param input 输入参数
     */
    void regenCoordination(double total_torque_request, const BrakingInput& input);
    
    /**
     * @brief 更新单个车轮制动
     * @param corner_state 车轮制动状态
     * @param corner_config 车轮制动配置
     * @param target_pressure 目标压力
     * @param dt 时间步长
     */
    void updateCorner(BrakeCornerState& corner_state,
                      const BrakeCornerConfig& corner_config,
                      double target_pressure,
                      double dt);
    
    /**
     * @brief 更新制动盘温度
     * @param corner_state 车轮制动状态
     * @param corner_config 车轮制动配置
     * @param dt 时间步长
     */
    void updateTemperature(BrakeCornerState& corner_state,
                           const BrakeCornerConfig& corner_config,
                           double dt);
    
    /**
     * @brief 计算制动力矩
     * @param corner_state 车轮制动状态
     * @param corner_config 车轮制动配置
     */
    void calculateBrakeTorque(BrakeCornerState& corner_state,
                               const BrakeCornerConfig& corner_config);
    
    /**
     * @brief 计算制动减速度
     */
    void calculateDeceleration();

private:
    BrakingConfig config_;          // 制动配置
    BrakingState state_;            // 制动状态
    
    // 控制器状态
    double reference_speed_;        // 参考车速
    std::array<double, 4> prev_slip_ratio_;  // 前一时刻滑移率
};

} // namespace chassis
} // namespace ev_dynamics

#endif // EV_DYNAMICS_BRAKING_H
