/**
 * @file pmsm_motor.h
 * @brief 永磁同步电机（PMSM）模型
 * @author CodeCraft
 * @date 2026-03-08
 * 
 * 实现完整的PMSM电机模型，包括：
 * - 电磁转矩计算
 * - d-q坐标系变换
 * - 磁场定向控制（FOC）
 * - 温度模型
 * - 损耗模型
 */

#ifndef EV_DYNAMICS_PMSM_MOTOR_H
#define EV_DYNAMICS_PMSM_MOTOR_H

#include <cmath>
#include <vector>
#include <Eigen/Dense>

namespace ev_dynamics {
namespace powertrain {

using namespace Eigen;

/**
 * @brief PMSM电机参数配置
 */
struct PMSMConfig {
    // 基本参数
    double rated_power;          // 额定功率 (W)
    double rated_voltage;        // 额定电压 (V)
    double rated_current;        // 额定电流 (A)
    double rated_speed;          // 额定转速 (rpm)
    double rated_torque;         // 额定扭矩 (Nm)
    
    // 电磁参数
    double pole_pairs;           // 极对数
    double stator_resistance;    // 定子电阻 Rs (Ohm)
    double d_axis_inductance;    // d轴电感 Ld (H)
    double q_axis_inductance;    // q轴电感 Lq (H)
    double permanent_flux;       // 永磁体磁链 ψf (Wb)
    
    // 机械参数
    double rotor_inertia;        // 转子惯量 (kg·m²)
    double friction_coefficient; // 摩擦系数 (N·m·s/rad)
    
    // 热参数
    double thermal_resistance;   // 热阻 (K/W)
    double thermal_capacitance;  // 热容 (J/K)
    double max_temperature;      // 最大允许温度 (°C)
    
    // 限制参数
    double max_current;          // 最大电流限制 (A)
    double max_speed;            // 最大转速限制 (rpm)
    
    PMSMConfig()
        : rated_power(80000.0)           // 80kW
        , rated_voltage(400.0)           // 400V
        , rated_current(200.0)           // 200A
        , rated_speed(6000.0)            // 6000rpm
        , rated_torque(127.0)            // 127Nm
        , pole_pairs(4.0)                // 4极对
        , stator_resistance(0.01)        // 0.01 Ohm
        , d_axis_inductance(0.0005)      // 0.5mH
        , q_axis_inductance(0.0005)      // 0.5mH
        , permanent_flux(0.175)          // 0.175Wb
        , rotor_inertia(0.05)            // 0.05 kg·m²
        , friction_coefficient(0.001)    // 0.001 N·m·s/rad
        , thermal_resistance(0.15)       // 0.15 K/W
        , thermal_capacitance(5000.0)    // 5000 J/K
        , max_temperature(150.0)         // 150°C
        , max_current(300.0)             // 300A (1.5倍过载)
        , max_speed(12000.0)             // 12000rpm (2倍过速)
    {}
};

/**
 * @brief PMSM电机状态
 */
struct PMSMState {
    double electrical_angle;     // 电气角度 (rad)
    double rotor_speed;          // 转子转速 (rad/s)
    double rotor_position;       // 转子位置 (rad)
    
    // d-q坐标系电流
    double id;                   // d轴电流 (A)
    double iq;                   // q轴电流 (A)
    
    // 三相电流
    double ia;                   // A相电流 (A)
    double ib;                   // B相电流 (A)
    double ic;                   // C相电流 (A)
    
    // 输出
    double electromagnetic_torque;  // 电磁转矩 (Nm)
    double output_power;            // 输出功率 (W)
    double efficiency;              // 效率 (%)
    
    // 温度
    double stator_temperature;      // 定子温度 (°C)
    double rotor_temperature;       // 转子温度 (°C)
    
    // 损耗
    double copper_loss;             // 铜损 (W)
    double iron_loss;               // 铁损 (W)
    double total_loss;              // 总损耗 (W)
    
    PMSMState()
        : electrical_angle(0.0)
        , rotor_speed(0.0)
        , rotor_position(0.0)
        , id(0.0)
        , iq(0.0)
        , ia(0.0)
        , ib(0.0)
        , ic(0.0)
        , electromagnetic_torque(0.0)
        , output_power(0.0)
        , efficiency(0.0)
        , stator_temperature(25.0)
        , rotor_temperature(25.0)
        , copper_loss(0.0)
        , iron_loss(0.0)
        , total_loss(0.0)
    {}
};

/**
 * @brief PMSM电机控制输入
 */
struct PMSMInput {
    double target_torque;        // 目标扭矩 (Nm)
    double dc_bus_voltage;       // 直流母线电压 (V)
    double ambient_temperature;  // 环境温度 (°C)
    
    PMSMInput()
        : target_torque(0.0)
        , dc_bus_voltage(400.0)
        , ambient_temperature(25.0)
    {}
};

/**
 * @brief PMSM电机模型类
 */
class PMSMMotor {
public:
    /**
     * @brief 构造函数
     * @param config 电机配置参数
     */
    explicit PMSMMotor(const PMSMConfig& config = PMSMConfig());
    
    /**
     * @brief 初始化电机
     */
    void initialize();
    
    /**
     * @brief 更新电机状态（时间步进）
     * @param dt 时间步长 (s)
     * @param input 控制输入
     */
    void update(double dt, const PMSMInput& input);
    
    /**
     * @brief 获取当前状态
     * @return 电机状态
     */
    const PMSMState& getState() const { return state_; }
    
    /**
     * @brief 获取配置参数
     * @return 电机配置
     */
    const PMSMConfig& getConfig() const { return config_; }
    
    /**
     * @brief 重置电机状态
     */
    void reset();

private:
    /**
     * @brief 计算电磁转矩
     */
    void calculateElectromagneticTorque();
    
    /**
     * @brief d-q坐标系到三相坐标系变换（逆Park变换）
     */
    void inverseParkTransform();
    
    /**
     * @brief 三相坐标系到d-q坐标系变换（Park变换）
     */
    void parkTransform();
    
    /**
     * @brief 计算损耗
     */
    void calculateLosses();
    
    /**
     * @brief 更新温度模型
     */
    void updateTemperature(double dt);
    
    /**
     * @brief 磁场定向控制（FOC）
     */
    void fieldOrientedControl(double target_torque);
    
    /**
     * @brief 电流限制
     */
    void limitCurrent();
    
    /**
     * @brief 计算效率
     */
    void calculateEfficiency();

private:
    PMSMConfig config_;  // 电机配置
    PMSMState state_;    // 电机状态
    
    // 控制参数
    double kp_current_;  // 电流环比例增益
    double ki_current_;  // 电流环积分增益
    double current_integral_error_d_;  // d轴电流积分误差
    double current_integral_error_q_;  // q轴电流积分误差
};

} // namespace powertrain
} // namespace ev_dynamics

#endif // EV_DYNAMICS_PMSM_MOTOR_H
