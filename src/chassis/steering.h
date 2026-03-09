/**
 * @file steering.h
 * @brief 电动助力转向系统（EPS）模型
 * @author CodeCraft
 * @date 2026-03-08
 * 
 * 实现线控转向系统，包括：
 * - 电动助力转向（EPS）
 * - 可变转向比
 * - Ackerman转向几何
 * - 转向手感反馈
 * - 主动回正
 */

#ifndef EV_DYNAMICS_STEERING_H
#define EV_DYNAMICS_STEERING_H

#include <cmath>
#include <vector>
#include <Eigen/Dense>

namespace ev_dynamics {
namespace chassis {

using namespace Eigen;

/**
 * @brief 转向类型枚举
 */
enum class SteeringType {
    MANUAL,         ///< 机械转向
    HYDRAULIC,      ///< 液压助力
    ELECTRIC,       ///< 电动助力（EPS）
    STEER_BY_WIRE   ///< 线控转向
};

/**
 * @brief 助力模式枚举
 */
enum class AssistMode {
    COMFORT,        ///< 舒适模式（轻便）
    NORMAL,         ///< 正常模式
    SPORT,          ///< 运动模式（沉稳）
    ADAPTIVE        ///< 自适应模式
};

/**
 * @brief 转向系统配置
 */
struct SteeringConfig {
    // 基本几何参数
    double wheelbase;               // 轴距 (m)
    double track_width;             // 轮距 (m)
    double kingpin_offset;          // 主销偏移距 (m)
    double caster_angle;            // 主销后倾角 (rad)
    double steering_arm_length;     // 转向臂长度 (m)
    
    // 转向比参数
    double base_steering_ratio;     // 基础转向比
    double min_steering_ratio;      // 最小转向比（高速）
    double max_steering_ratio;      // 最大转向比（低速）
    double variable_ratio_start_speed; // 可变转向比启用速度 (m/s)
    
    // Ackerman几何参数
    double ackerman_percentage;     // Ackerman百分比 (0-1)
    double max_wheel_angle;         // 最大车轮转角 (rad)
    
    // EPS参数
    double motor_torque_max;        // 最大电机扭矩 (Nm)
    double motor_inertia;           // 电机转动惯量 (kg·m²)
    double motor_damping;           // 电机阻尼 (Nm·s/rad)
    double gear_ratio;              // 转向器传动比
    double column_stiffness;        // 转向柱刚度 (Nm/rad)
    double column_damping;          // 转向柱阻尼 (Nm·s/rad)
    
    // 反馈参数
    double feedback_gain;           // 路感反馈增益
    double return_speed;            // 回正速度 (rad/s)
    double friction_compensation;   // 摩擦补偿 (Nm)
    
    // 类型
    SteeringType type;              // 转向类型
    AssistMode assist_mode;         // 助力模式
    
    SteeringConfig()
        : wheelbase(2.8)
        , track_width(1.6)
        , kingpin_offset(0.05)
        , caster_angle(0.1)         // ~5.7度
        , steering_arm_length(0.12)
        , base_steering_ratio(16.0)
        , min_steering_ratio(12.0)
        , max_steering_ratio(20.0)
        , variable_ratio_start_speed(5.0)
        , ackerman_percentage(0.8)
        , max_wheel_angle(0.6)      // ~35度
        , motor_torque_max(8.0)
        , motor_inertia(0.001)
        , motor_damping(0.1)
        , gear_ratio(20.0)
        , column_stiffness(100.0)
        , column_damping(2.0)
        , feedback_gain(1.0)
        , return_speed(2.0)
        , friction_compensation(0.5)
        , type(SteeringType::ELECTRIC)
        , assist_mode(AssistMode::NORMAL)
    {}
};

/**
 * @brief 转向系统状态
 */
struct SteeringState {
    // 转向盘状态
    double steering_wheel_angle;    // 转向盘转角 (rad)
    double steering_wheel_torque;   // 转向盘扭矩 (Nm)
    double steering_wheel_speed;    // 转向盘角速度 (rad/s)
    
    // 车轮转角
    double wheel_angle_left;        // 左前轮转角 (rad)
    double wheel_angle_right;       // 右前轮转角 (rad)
    double wheel_angle_average;     // 平均车轮转角 (rad)
    
    // 助力系统
    double assist_torque;           // 助力扭矩 (Nm)
    double feedback_torque;         // 反馈扭矩 (Nm)
    double motor_angle;             // 电机转角 (rad)
    double motor_speed;             // 电机角速度 (rad/s)
    double motor_torque;            // 电机扭矩 (Nm)
    
    // 转向比
    double current_steering_ratio;  // 当前转向比
    
    // Ackerman角度差
    double ackerman_angle_diff;     // Ackerman角度差 (rad)
    
    // 回正状态
    double return_torque;           // 回正扭矩 (Nm)
    bool is_returning;              // 是否正在回正
    
    SteeringState()
        : steering_wheel_angle(0.0)
        , steering_wheel_torque(0.0)
        , steering_wheel_speed(0.0)
        , wheel_angle_left(0.0)
        , wheel_angle_right(0.0)
        , wheel_angle_average(0.0)
        , assist_torque(0.0)
        , feedback_torque(0.0)
        , motor_angle(0.0)
        , motor_speed(0.0)
        , motor_torque(0.0)
        , current_steering_ratio(16.0)
        , ackerman_angle_diff(0.0)
        , return_torque(0.0)
        , is_returning(false)
    {}
};

/**
 * @brief 转向系统输入
 */
struct SteeringInput {
    // 驾驶员输入
    double driver_torque;           // 驾驶员施加扭矩 (Nm)
    double driver_angle;            // 驾驶员转向角度 (rad) - 用于线控转向
    
    // 车辆状态
    double vehicle_speed;           // 车速 (m/s)
    double lateral_acceleration;    // 横向加速度 (m/s²)
    double yaw_rate;                // 横摆角速度 (rad/s)
    
    // 轮胎反馈
    double tire_align_torque;       // 轮胎回正力矩 (Nm)
    double tire_feedback_force;     // 轮胎反馈力 (N)
    
    SteeringInput()
        : driver_torque(0.0)
        , driver_angle(0.0)
        , vehicle_speed(0.0)
        , lateral_acceleration(0.0)
        , yaw_rate(0.0)
        , tire_align_torque(0.0)
        , tire_feedback_force(0.0)
    {}
};

/**
 * @brief 电动助力转向系统类
 */
class Steering {
public:
    /**
     * @brief 构造函数
     * @param config 转向配置参数
     */
    explicit Steering(const SteeringConfig& config = SteeringConfig());
    
    /**
     * @brief 初始化转向系统
     */
    void initialize();
    
    /**
     * @brief 更新转向状态（时间步进）
     * @param dt 时间步长 (s)
     * @param input 输入参数
     */
    void update(double dt, const SteeringInput& input);
    
    /**
     * @brief 获取当前状态
     * @return 转向状态
     */
    const SteeringState& getState() const { return state_; }
    
    /**
     * @brief 获取配置参数
     * @return 转向配置
     */
    const SteeringConfig& getConfig() const { return config_; }
    
    /**
     * @brief 设置转向盘角度（用于线控转向）
     * @param angle 转向盘角度 (rad)
     */
    void setSteeringWheelAngle(double angle);
    
    /**
     * @brief 设置助力模式
     * @param mode 助力模式
     */
    void setAssistMode(AssistMode mode);
    
    /**
     * @brief 设置转向比（手动模式）
     * @param ratio 转向比
     */
    void setSteeringRatio(double ratio);
    
    /**
     * @brief 启用/禁用主动回正
     * @param enable 是否启用
     */
    void enableActiveReturn(bool enable);
    
    /**
     * @brief 重置转向状态
     */
    void reset();

private:
    /**
     * @brief 计算可变转向比
     * @param vehicle_speed 车速
     * @param steering_angle 转向盘角度
     * @return 当前转向比
     */
    double calculateVariableRatio(double vehicle_speed, 
                                   double steering_angle) const;
    
    /**
     * @brief 计算Ackerman转向几何
     * @param average_angle 平均车轮转角
     * @param wheelbase 轴距
     * @param track_width 轮距
     * @return 左右车轮转角（pair<左, 右>）
     */
    std::pair<double, double> calculateAckermanAngles(double average_angle,
                                                       double wheelbase,
                                                       double track_width) const;
    
    /**
     * @brief 计算助力扭矩
     * @param driver_torque 驾驶员扭矩
     * @param vehicle_speed 车速
     * @return 助力扭矩 (Nm)
     */
    double calculateAssistTorque(double driver_torque, 
                                  double vehicle_speed);
    
    /**
     * @brief 计算路感反馈扭矩
     * @param input 输入参数
     * @return 反馈扭矩 (Nm)
     */
    double calculateFeedbackTorque(const SteeringInput& input) const;
    
    /**
     * @brief 计算回正扭矩
     * @param input 输入参数
     * @return 回正扭矩 (Nm)
     */
    double calculateReturnTorque(const SteeringInput& input);
    
    /**
     * @brief 更新电机状态
     * @param dt 时间步长
     * @param target_torque 目标扭矩
     */
    void updateMotor(double dt, double target_torque);
    
    /**
     * @brief 更新转向柱状态
     * @param dt 时间步长
     * @param driver_torque 驾驶员扭矩
     */
    void updateSteeringColumn(double dt, double driver_torque);
    
    /**
     * @brief 摩擦补偿
     * @param speed 角速度
     * @return 摩擦补偿扭矩 (Nm)
     */
    double frictionCompensation(double speed) const;

private:
    SteeringConfig config_;         // 转向配置
    SteeringState state_;           // 转向状态
    
    // 控制器状态
    bool active_return_enabled_;    // 主动回正使能
    double manual_steering_ratio_;  // 手动设置的转向比（0表示自动）
    
    // 电机动力学状态
    double motor_torque_command_;   // 电机扭矩指令
    double prev_steering_angle_;    // 前一时刻转向角
};

} // namespace chassis
} // namespace ev_dynamics

#endif // EV_DYNAMICS_STEERING_H
