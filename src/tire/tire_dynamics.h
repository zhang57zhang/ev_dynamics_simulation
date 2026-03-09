/**
 * @file tire_dynamics.h
 * @brief 轮胎动力学模型头文件
 * 
 * 实现轮胎动力学，包括：
 * - 滑移率计算（纵向和横向）
 * - 松弛长度模型（动态响应）
 * - 轮胎转动惯量
 * - 滚动阻力
 * - 轮胎温度影响
 * 
 * @author TireAgent
 * @date 2026-03-08
 */

#pragma once

#include <array>
#include <memory>
#include "pacejka_model.h"

namespace ev_simulation {
namespace tire {

/**
 * @brief 轮胎配置参数
 */
struct TireConfig {
    // === 几何参数 ===
    double radius{0.33};            ///< 轮胎半径 (m)
    double width{0.225};            ///< 轮胎宽度 (m)
    double rim_radius{0.2032};      ///< 轮毂半径 (m)
    
    // === 惯性参数 ===
    double inertia{1.5};            ///< 轮胎转动惯量 (kg·m²)
    double mass{12.0};              ///< 轮胎质量 (kg)
    
    // === 松弛长度参数 ===
    double relaxation_length_long{0.2};   ///< 纵向松弛长度 (m)
    double relaxation_length_lat{0.5};    ///< 横向松弛长度 (m)
    
    // === 滚动阻力参数 ===
    double rolling_resistance_coeff{0.01}; ///< 滚动阻力系数
    double rolling_resistance_speed{0.0};   ///< 速度相关系数
    
    // === 温度参数 ===
    double temperature_nominal{80.0};       ///< 标称温度 (°C)
    double temperature_coefficient{-0.001}; ///< 温度影响系数 (1/°C)
    
    // === 摩擦参数 ===
    double friction_base{1.0};              ///< 基础摩擦系数
    double friction_load_sensitivity{0.0};  ///< 载荷敏感系数
    
    /**
     * @brief 验证参数有效性
     * @throws std::invalid_argument 参数无效
     */
    void validate() const;
};

/**
 * @brief 车轮运动状态
 */
struct WheelMotion {
    double omega{0.0};              ///< 车轮角速度 (rad/s)
    double V_wheel{0.0};            ///< 车轮中心速度 (m/s)
    double V_vehicle{0.0};          ///< 车速 (m/s)
    double steer_angle{0.0};        ///< 转向角 (rad)
    double camber_angle{0.0};       ///< 外倾角 (rad)
    
    // 车轮在车身坐标系中的速度分量
    double Vx{0.0};                 ///< 纵向速度 (m/s)
    double Vy{0.0};                 ///< 横向速度 (m/s)
};

/**
 * @brief 滑移状态
 */
struct SlipState {
    double kappa{0.0};              ///< 纵向滑移率 (-1 to 1)
    double alpha{0.0};              ///< 侧偏角 (rad)
    double gamma{0.0};              ///< 外倾角 (rad)
    
    // 动态滑移（考虑松弛长度）
    double kappa_dynamic{0.0};      ///< 动态纵向滑移率
    double alpha_dynamic{0.0};      ///< 动态侧偏角
    
    // 滑移速度
    double slip_velocity_x{0.0};    ///< 纵向滑移速度 (m/s)
    double slip_velocity_y{0.0};    ///< 横向滑移速度 (m/s)
};

/**
 * @brief 轮胎温度状态
 */
struct TireTemperature {
    double surface_temp{80.0};      ///< 表面温度 (°C)
    double bulk_temp{80.0};         ///< 体积温度 (°C)
    double gradient{0.0};           ///< 温度梯度 (°C/m)
};

/**
 * @brief 轮胎动态状态
 */
struct TireDynamicState {
    // 滑移状态
    SlipState slip;
    
    // 垂直载荷
    double Fz{4000.0};              ///< 垂直载荷 (N)
    
    // 温度
    TireTemperature temperature;
    
    // 动态变量（松弛长度模型）
    double sigma_x{0.0};            ///< 纵向松弛状态
    double sigma_y{0.0};            ///< 横向松弛状态
    
    // 车轮转速
    double omega{0.0};              ///< 车轮角速度 (rad/s)
};

/**
 * @brief 轮胎动力学输出
 */
struct TireDynamicsOutput {
    TireForces forces;              ///< 轮胎力
    SlipState slip;                 ///< 滑移状态
    double omega{0.0};              ///< 车轮角速度 (rad/s)
    double omega_dot{0.0};          ///< 车轮角加速度 (rad/s²)
    double effective_radius{0.33};  ///< 有效半径 (m)
};

/**
 * @brief 轮胎动力学模型
 * 
 * 实现完整的轮胎动力学，包括：
 * - 滑移率计算
 * - 松弛长度模型
 * - 动态响应
 * - 温度影响
 * - 滚动阻力
 */
class TireDynamics {
public:
    /**
     * @brief 构造函数
     */
    TireDynamics();
    
    /**
     * @brief 带参数构造函数
     * @param tire_config 轮胎配置
     * @param pacejka_params Pacejka参数
     */
    TireDynamics(const TireConfig& tire_config, const PacejkaParameters& pacejka_params);
    
    /**
     * @brief 析构函数
     */
    ~TireDynamics();
    
    /**
     * @brief 初始化轮胎动力学
     * @param tire_config 轮胎配置
     * @param pacejka_params Pacejka参数
     */
    void initialize(const TireConfig& tire_config, const PacejkaParameters& pacejka_params);
    
    /**
     * @brief 重置状态
     */
    void reset();
    
    /**
     * @brief 设置车轮运动状态
     * @param motion 车轮运动状态
     */
    void setWheelMotion(const WheelMotion& motion);
    
    /**
     * @brief 设置垂直载荷
     * @param Fz 垂直载荷 (N)
     */
    void setVerticalLoad(double Fz);
    
    /**
     * @brief 设置温度
     * @param temperature 温度状态
     */
    void setTemperature(const TireTemperature& temperature);
    
    /**
     * @brief 计算轮胎力（主接口）
     * @return 轮胎动力学输出
     */
    TireDynamicsOutput computeForces();
    
    /**
     * @brief 更新轮胎状态（时间步进）
     * @param dt 时间步长 (s)
     * @param driving_torque 驱动力矩 (N·m)
     * @param brake_torque 制动力矩 (N·m)
     */
    void update(double dt, double driving_torque = 0.0, double brake_torque = 0.0);
    
    /**
     * @brief 计算滑移率
     * @param motion 车轮运动状态
     * @return 滑移状态
     */
    SlipState computeSlip(const WheelMotion& motion);
    
    /**
     * @brief 计算松弛长度
     * @param slip 滑移状态
     * @param V_wheel 车轮速度 (m/s)
     * @return 松弛后的滑移状态
     */
    SlipState applyRelaxation(const SlipState& slip, double V_wheel);
    
    /**
     * @brief 计算滚动阻力
     * @param Fz 垂直载荷 (N)
     * @param V 车速 (m/s)
     * @return 滚动阻力 (N)
     */
    double computeRollingResistance(double Fz, double V) const;
    
    /**
     * @brief 计算有效轮胎半径
     * @param Fz 垂直载荷 (N)
     * @return 有效半径 (m)
     */
    double computeEffectiveRadius(double Fz) const;
    
    /**
     * @brief 计算摩擦系数（温度影响）
     * @param base_friction 基础摩擦系数
     * @param temperature 温度 (°C)
     * @return 有效摩擦系数
     */
    double computeEffectiveFriction(double base_friction, double temperature) const;
    
    /**
     * @brief 获取当前状态
     * @return 轮胎动态状态
     */
    const TireDynamicState& getState() const { return state_; }
    
    /**
     * @brief 获取配置
     * @return 轮胎配置
     */
    const TireConfig& getConfig() const { return config_; }
    
    /**
     * @brief 获取Pacejka模型
     * @return Pacejka模型引用
     */
    const PacejkaModel& getPacejkaModel() const { return *pacejka_model_; }
    
    /**
     * @brief 检查是否已初始化
     * @return true: 已初始化
     */
    bool isInitialized() const { return initialized_; }

private:
    /**
     * @brief 更新松弛长度模型
     * @param dt 时间步长 (s)
     */
    void updateRelaxation(double dt);
    
    /**
     * @brief 更新车轮转速
     * @param dt 时间步长 (s)
     * @param driving_torque 驱动力矩 (N·m)
     * @param brake_torque 制动力矩 (N·m)
     */
    void updateWheelSpeed(double dt, double driving_torque, double brake_torque);
    
    /**
     * @brief 更新温度
     * @param dt 时间步长 (s)
     * @param slip 滑移状态
     */
    void updateTemperature(double dt, const SlipState& slip);
    
    TireConfig config_;                     ///< 轮胎配置
    std::unique_ptr<PacejkaModel> pacejka_model_;  ///< Pacejka模型
    TireDynamicState state_;                ///< 轮胎动态状态
    WheelMotion motion_;                    ///< 车轮运动状态
    bool initialized_;                      ///< 初始化标志
};

/**
 * @brief 四轮轮胎动力学系统
 * 
 * 管理四个车轮的轮胎动力学
 */
class FourWheelTireSystem {
public:
    /**
     * @brief 构造函数
     */
    FourWheelTireSystem();
    
    /**
     * @brief 析构函数
     */
    ~FourWheelTireSystem();
    
    /**
     * @brief 初始化四轮系统
     * @param tire_config 轮胎配置
     * @param pacejka_params Pacejka参数
     */
    void initialize(const TireConfig& tire_config, const PacejkaParameters& pacejka_params);
    
    /**
     * @brief 重置所有轮胎状态
     */
    void reset();
    
    /**
     * @brief 设置车轮运动状态
     * @param wheel_index 车轮索引 (0:FL, 1:FR, 2:RL, 3:RR)
     * @param motion 车轮运动状态
     */
    void setWheelMotion(int wheel_index, const WheelMotion& motion);
    
    /**
     * @brief 设置垂直载荷
     * @param wheel_index 车轮索引
     * @param Fz 垂直载荷 (N)
     */
    void setVerticalLoad(int wheel_index, double Fz);
    
    /**
     * @brief 设置所有车轮载荷
     * @param Fz 四轮载荷数组
     */
    void setAllWheelLoads(const std::array<double, 4>& Fz);
    
    /**
     * @brief 计算所有轮胎力
     * @return 四轮轮胎力
     */
    std::array<TireDynamicsOutput, 4> computeAllForces();
    
    /**
     * @brief 更新所有轮胎状态
     * @param dt 时间步长 (s)
     * @param driving_torques 四轮驱动力矩
     * @param brake_torques 四轮制动力矩
     */
    void updateAll(double dt, 
                   const std::array<double, 4>& driving_torques,
                   const std::array<double, 4>& brake_torques);
    
    /**
     * @brief 获取轮胎动力学实例
     * @param wheel_index 车轮索引
     * @return 轮胎动力学引用
     */
    const TireDynamics& getTire(int wheel_index) const;
    
    /**
     * @brief 获取总轮胎力
     * @return (Fx_total, Fy_total, Mz_total)
     */
    std::tuple<double, double, double> getTotalForces() const;
    
    /**
     * @brief 检查是否已初始化
     * @return true: 已初始化
     */
    bool isInitialized() const { return initialized_; }

private:
    std::array<std::unique_ptr<TireDynamics>, 4> tires_;  ///< 四轮轮胎
    bool initialized_;                                     ///< 初始化标志
};

} // namespace tire
} // namespace ev_simulation
