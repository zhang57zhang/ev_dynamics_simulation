/**
 * @file transmission.h
 * @brief 单级减速器模型
 * @author CodeCraft
 * @date 2026-03-08
 * 
 * 实现单级减速器模型，包括：
 * - 齿轮传动
 * - 传动效率
 * - 齿轮动力学
 * - 振动模型
 */

#ifndef EV_DYNAMICS_TRANSMISSION_H
#define EV_DYNAMICS_TRANSMISSION_H

#include <cmath>
#include <Eigen/Dense>

namespace ev_dynamics {
namespace powertrain {

using namespace Eigen;

/**
 * @brief 变速器参数配置
 */
struct TransmissionConfig {
    // 基本参数
    double gear_ratio;             // 传动比（减速比）
    double gear_efficiency;        // 传动效率
    double gear_inertia;           // 齿轮惯量 (kg·m²)
    
    // 齿轮参数
    int pinion_teeth;              // 主动齿轮齿数
    int gear_teeth;                // 从动齿轮齿数
    double module;                 // 模数 (mm)
    double pressure_angle;         // 压力角 (rad)
    
    // 振动参数
    double stiffness;              // 扭转刚度 (N·m/rad)
    double damping;                // 扭转阻尼 (N·m·s/rad)
    
    // 限制参数
    double max_input_speed;        // 最大输入转速 (rpm)
    double max_output_torque;      // 最大输出扭矩 (Nm)
    double max_temperature;        // 最大温度 (°C)
    
    TransmissionConfig()
        : gear_ratio(9.0)              // 9:1减速比
        , gear_efficiency(0.97)        // 97%效率
        , gear_inertia(0.01)           // 0.01 kg·m²
        , pinion_teeth(15)             // 15齿主动轮
        , gear_teeth(135)              // 135齿从动轮
        , module(3.0)                  // 模数3mm
        , pressure_angle(M_PI / 9.0)   // 20度压力角
        , stiffness(10000.0)           // 10000 N·m/rad
        , damping(10.0)                // 10 N·m·s/rad
        , max_input_speed(12000.0)     // 12000 rpm
        , max_output_torque(3000.0)    // 3000 Nm
        , max_temperature(120.0)       // 120°C
    {}
};

/**
 * @brief 变速器状态
 */
struct TransmissionState {
    double input_speed;            // 输入转速 (rad/s)
    double output_speed;           // 输出转速 (rad/s)
    double input_torque;           // 输入扭矩 (Nm)
    double output_torque;          // 输出扭矩 (Nm)
    double power_loss;             // 功率损耗 (W)
    double efficiency;             // 实时效率
    double temperature;            // 温度 (°C)
    double vibration_angle;        // 扭转振动角度 (rad)
    double vibration_velocity;     // 扭转振动角速度 (rad/s)
    
    TransmissionState()
        : input_speed(0.0)
        , output_speed(0.0)
        , input_torque(0.0)
        , output_torque(0.0)
        , power_loss(0.0)
        , efficiency(0.97)
        , temperature(25.0)
        , vibration_angle(0.0)
        , vibration_velocity(0.0)
    {}
};

/**
 * @brief 变速器输入
 */
struct TransmissionInput {
    double input_torque;           // 输入扭矩 (Nm)
    double input_speed;            // 输入转速 (rad/s)
    double load_torque;            // 负载扭矩 (Nm)
    double ambient_temperature;    // 环境温度 (°C)
    
    TransmissionInput()
        : input_torque(0.0)
        , input_speed(0.0)
        , load_torque(0.0)
        , ambient_temperature(25.0)
    {}
};

/**
 * @brief 单级减速器模型类
 */
class Transmission {
public:
    /**
     * @brief 构造函数
     * @param config 变速器配置参数
     */
    explicit Transmission(const TransmissionConfig& config = TransmissionConfig());
    
    /**
     * @brief 初始化变速器
     */
    void initialize();
    
    /**
     * @brief 更新变速器状态（时间步进）
     * @param dt 时间步长 (s)
     * @param input 输入参数
     */
    void update(double dt, const TransmissionInput& input);
    
    /**
     * @brief 获取当前状态
     * @return 变速器状态
     */
    const TransmissionState& getState() const { return state_; }
    
    /**
     * @brief 获取配置参数
     * @return 变速器配置
     */
    const TransmissionConfig& getConfig() const { return config_; }
    
    /**
     * @brief 重置变速器状态
     */
    void reset();

private:
    /**
     * @brief 计算输出转速
     */
    void calculateOutputSpeed();
    
    /**
     * @brief 计算输出扭矩
     */
    void calculateOutputTorque();
    
    /**
     * @brief 计算传动效率
     */
    void calculateEfficiency();
    
    /**
     * @brief 计算功率损耗
     */
    void calculatePowerLoss();
    
    /**
     * @brief 更新扭转振动
     */
    void updateVibration(double dt);
    
    /**
     * @brief 更新温度
     */
    void updateTemperature(double dt);

private:
    TransmissionConfig config_;  // 变速器配置
    TransmissionState state_;    // 变速器状态
};

} // namespace powertrain
} // namespace ev_dynamics

#endif // EV_DYNAMICS_TRANSMISSION_H
