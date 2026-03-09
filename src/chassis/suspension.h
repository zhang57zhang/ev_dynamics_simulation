/**
 * @file suspension.h
 * @brief 主动悬架系统模型
 * @author CodeCraft
 * @date 2026-03-08
 * 
 * 实现主动悬架系统，包括：
 * - 多体动力学模型
 * - 主动高度调节
 * - 可变阻尼控制（CDC）
 * - 舒适性算法
 * - 路面适应
 */

#ifndef EV_DYNAMICS_SUSPENSION_H
#define EV_DYNAMICS_SUSPENSION_H

#include <cmath>
#include <vector>
#include <array>
#include <Eigen/Dense>

namespace ev_dynamics {
namespace chassis {

using namespace Eigen;

/**
 * @brief 悬架类型枚举
 */
enum class SuspensionType {
    PASSIVE,        ///< 被动悬架
    SEMI_ACTIVE,    ///< 半主动悬架
    ACTIVE          ///< 主动悬架
};

/**
 * @brief 阻尼模式枚举
 */
enum class DampingMode {
    COMFORT,        ///< 舒适模式
    NORMAL,         ///< 正常模式
    SPORT,          ///< 运动模式
    ADAPTIVE        ///< 自适应模式
};

/**
 * @brief 单个悬架配置参数
 */
struct SuspensionCornerConfig {
    // 弹簧参数
    double spring_stiffness;         // 弹簧刚度 (N/m)
    double spring_preload;           // 预紧力 (N)
    double spring_nonlinear_coeff;   // 非线性系数
    
    // 阻尼参数
    double damping_compression;      // 压缩阻尼 (Ns/m)
    double damping_rebound;          // 回弹阻尼 (Ns/m)
    double damping_min;              // 最小阻尼 (Ns/m)
    double damping_max;              // 最大阻尼 (Ns/m)
    
    // 几何参数
    double motion_ratio;             // 运动比
    double installed_length;         // 安装长度 (m)
    double min_length;               // 最小长度 (m)
    double max_length;               // 最大长度 (m)
    
    // 主动系统参数
    double actuator_force_max;       // 最大作动力 (N)
    double actuator_bandwidth;       // 作动器带宽 (Hz)
    
    // 质量参数
    double sprung_mass;              // 簧上质量 (kg)
    double unsprung_mass;            // 簧下质量 (kg)
    
    // 轮胎参数
    double tire_stiffness;           // 轮胎刚度 (N/m)
    double tire_damping;             // 轮胎阻尼 (Ns/m)
    
    SuspensionCornerConfig()
        : spring_stiffness(25000.0)
        , spring_preload(0.0)
        , spring_nonlinear_coeff(100.0)
        , damping_compression(2500.0)
        , damping_rebound(3000.0)
        , damping_min(1000.0)
        , damping_max(6000.0)
        , motion_ratio(1.0)
        , installed_length(0.35)
        , min_length(0.25)
        , max_length(0.45)
        , actuator_force_max(5000.0)
        , actuator_bandwidth(20.0)
        , sprung_mass(400.0)
        , unsprung_mass(45.0)
        , tire_stiffness(200000.0)
        , tire_damping(200.0)
    {}
};

/**
 * @brief 完整悬架系统配置
 */
struct SuspensionConfig {
    SuspensionCornerConfig front_left;
    SuspensionCornerConfig front_right;
    SuspensionCornerConfig rear_left;
    SuspensionCornerConfig rear_right;
    
    // 高度调节参数
    double height_min;               // 最小离地间隙 (m)
    double height_max;               // 最大离地间隙 (m)
    double height_normal;            // 正常离地间隙 (m)
    double height_adjust_rate;       // 高度调节速率 (m/s)
    
    // 控制参数
    SuspensionType type;             // 悬架类型
    DampingMode damping_mode;        // 阻尼模式
    double comfort_weight;           // 舒适性权重 (0-1)
    double handling_weight;          // 操控性权重 (0-1)
    
    // 主动控制参数
    double preview_distance;         // 预瞄距离 (m)
    bool enable_preview;             // 启用预瞄控制
    
    SuspensionConfig()
        : height_min(0.12)
        , height_max(0.22)
        , height_normal(0.16)
        , height_adjust_rate(0.01)
        , type(SuspensionType::ACTIVE)
        , damping_mode(DampingMode::ADAPTIVE)
        , comfort_weight(0.5)
        , handling_weight(0.5)
        , preview_distance(5.0)
        , enable_preview(false)
    {}
};

/**
 * @brief 单个悬架状态
 */
struct SuspensionCornerState {
    double displacement;             // 悬架位移 (m)
    double velocity;                 // 悬架速度 (m/s)
    double acceleration;             // 悬架加速度 (m/s²)
    double spring_force;             // 弹簧力 (N)
    double damping_force;            // 阻尼力 (N)
    double actuator_force;           // 作动器力 (N)
    double total_force;              // 总力 (N)
    double tire_deflection;          // 轮胎变形 (m)
    double tire_force;               // 轮胎力 (N)
    double road_displacement;        // 路面位移 (m)
    double road_velocity;            // 路面速度 (m/s)
    
    SuspensionCornerState()
        : displacement(0.0)
        , velocity(0.0)
        , acceleration(0.0)
        , spring_force(0.0)
        , damping_force(0.0)
        , actuator_force(0.0)
        , total_force(0.0)
        , tire_deflection(0.0)
        , tire_force(0.0)
        , road_displacement(0.0)
        , road_velocity(0.0)
    {}
};

/**
 * @brief 完整悬架系统状态
 */
struct SuspensionState {
    SuspensionCornerState front_left;
    SuspensionCornerState front_right;
    SuspensionCornerState rear_left;
    SuspensionCornerState rear_right;
    
    double body_height;              // 车身高度 (m)
    double body_pitch;               // 车身俯仰角 (rad)
    double body_roll;                // 车身侧倾角 (rad)
    double pitch_rate;               // 俯仰角速度 (rad/s)
    double roll_rate;                // 侧倾角速度 (rad/s)
    double vertical_acceleration;    // 垂向加速度 (m/s²)
    
    double comfort_index;            // 舒适性指标
    double handling_index;           // 操控性指标
    
    SuspensionState()
        : body_height(0.16)
        , body_pitch(0.0)
        , body_roll(0.0)
        , pitch_rate(0.0)
        , roll_rate(0.0)
        , vertical_acceleration(0.0)
        , comfort_index(1.0)
        , handling_index(1.0)
    {}
};

/**
 * @brief 悬架输入
 */
struct SuspensionInput {
    // 四轮路面输入
    double road_fl;                  // 前左路面位移 (m)
    double road_fr;                  // 前右路面位移 (m)
    double road_rl;                  // 后左路面位移 (m)
    double road_rr;                  // 后右路面位移 (m)
    
    // 车辆状态
    double vehicle_speed;            // 车速 (m/s)
    double lateral_acceleration;     // 横向加速度 (m/s²)
    double longitudinal_acceleration; // 纵向加速度 (m/s²)
    
    // 驾驶员请求
    double target_height;            // 目标高度 (m)
    DampingMode requested_damping_mode; // 请求的阻尼模式
    
    SuspensionInput()
        : road_fl(0.0)
        , road_fr(0.0)
        , road_rl(0.0)
        , road_rr(0.0)
        , vehicle_speed(0.0)
        , lateral_acceleration(0.0)
        , longitudinal_acceleration(0.0)
        , target_height(0.16)
        , requested_damping_mode(DampingMode::NORMAL)
    {}
};

/**
 * @brief 主动悬架系统类
 */
class Suspension {
public:
    /**
     * @brief 构造函数
     * @param config 悬架配置参数
     */
    explicit Suspension(const SuspensionConfig& config = SuspensionConfig());
    
    /**
     * @brief 初始化悬架
     */
    void initialize();
    
    /**
     * @brief 更新悬架状态（时间步进）
     * @param dt 时间步长 (s)
     * @param input 输入参数
     */
    void update(double dt, const SuspensionInput& input);
    
    /**
     * @brief 获取当前状态
     * @return 悬架状态
     */
    const SuspensionState& getState() const { return state_; }
    
    /**
     * @brief 获取配置参数
     * @return 悬架配置
     */
    const SuspensionConfig& getConfig() const { return config_; }
    
    /**
     * @brief 设置目标高度
     * @param height 目标高度 (m)
     */
    void setTargetHeight(double height);
    
    /**
     * @brief 设置阻尼模式
     * @param mode 阻尼模式
     */
    void setDampingMode(DampingMode mode);
    
    /**
     * @brief 设置舒适性/操控性权重
     * @param comfort 舒适性权重 (0-1)
     * @param handling 操控性权重 (0-1)
     */
    void setPerformanceWeights(double comfort, double handling);
    
    /**
     * @brief 重置悬架状态
     */
    void reset();

private:
    /**
     * @brief 更新单个悬架角
     * @param corner_config 悬架角配置
     * @param corner_state 悬架角状态
     * @param road_input 路面输入
     * @param dt 时间步长
     */
    void updateCorner(const SuspensionCornerConfig& corner_config,
                      SuspensionCornerState& corner_state,
                      double road_input,
                      double dt);
    
    /**
     * @brief 计算弹簧力（非线性）
     * @param config 配置参数
     * @param displacement 位移
     * @return 弹簧力 (N)
     */
    double calculateSpringForce(const SuspensionCornerConfig& config,
                                double displacement) const;
    
    /**
     * @brief 计算阻尼力（可变阻尼）
     * @param config 配置参数
     * @param velocity 速度
     * @param mode 阻尼模式
     * @return 阻尼力 (N)
     */
    double calculateDampingForce(const SuspensionCornerConfig& config,
                                 double velocity,
                                 DampingMode mode);
    
    /**
     * @brief 计算主动作动器力
     * @param corner_state 悬架角状态
     * @param dt 时间步长
     * @return 作动器力 (N)
     */
    double calculateActuatorForce(SuspensionCornerState& corner_state, double dt);
    
    /**
     * @brief 更新车身姿态
     */
    void updateBodyPose();
    
    /**
     * @brief 计算舒适性指标
     * @return 舒适性指标 (0-1，越大越好)
     */
    double calculateComfortIndex() const;
    
    /**
     * @brief 计算操控性指标
     * @return 操控性指标 (0-1，越大越好)
     */
    double calculateHandlingIndex() const;
    
    /**
     * @brief 高度调节控制
     * @param target_height 目标高度
     * @param dt 时间步长
     */
    void heightControl(double target_height, double dt);
    
    /**
     * @brief 自适应阻尼控制
     * @param input 输入参数
     */
    void adaptiveDampingControl(const SuspensionInput& input);
    
    /**
     * @brief 天棚阻尼控制
     * @param corner_state 悬架角状态
     * @return 天棚阻尼力 (N)
     */
    double skyhookDampingControl(const SuspensionCornerState& corner_state) const;
    
    /**
     * @brief 地面钩阻尼控制
     * @param corner_state 悬架角状态
     * @return 地面钩阻尼力 (N)
     */
    double groundhookDampingControl(const SuspensionCornerState& corner_state) const;

private:
    SuspensionConfig config_;        // 悬架配置
    SuspensionState state_;          // 悬架状态
    
    // 控制器状态
    double target_height_;           // 目标高度
    DampingMode current_damping_mode_; // 当前阻尼模式
    double adaptive_damping_factor_; // 自适应阻尼系数
    
    // 滤波器状态
    std::array<double, 4> prev_road_input_;  // 前一时刻路面输入
    std::array<double, 4> filtered_velocity_; // 滤波后的速度
    
    // PID控制器参数（高度调节）
    double height_kp_;
    double height_ki_;
    double height_kd_;
    double height_integral_;
    double height_prev_error_;
};

} // namespace chassis
} // namespace ev_dynamics

#endif // EV_DYNAMICS_SUSPENSION_H
