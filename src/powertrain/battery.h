/**
 * @file battery.h
 * @brief 锂离子电池等效电路模型
 * @author CodeCraft
 * @date 2026-03-08
 * 
 * 实现Thevenin等效电路模型，包括：
 * - 开路电压（OCV）特性
 * - 内阻模型
 * - 极化效应
 * - SOC估算
 * - 温度特性
 */

#ifndef EV_DYNAMICS_BATTERY_H
#define EV_DYNAMICS_BATTERY_H

#include <cmath>
#include <vector>
#include <Eigen/Dense>

namespace ev_dynamics {
namespace powertrain {

using namespace Eigen;

/**
 * @brief 电池参数配置
 */
struct BatteryConfig {
    // 基本参数
    double nominal_capacity;      // 额定容量 (Ah)
    double nominal_voltage;       // 额定电压 (V)
    double max_voltage;           // 最大电压 (V)
    double min_voltage;           // 最小电压 (V)
    double max_charge_current;    // 最大充电电流 (A)
    double max_discharge_current; // 最大放电电流 (A)
    
    // Thevenin模型参数
    double series_resistance;     // 串联电阻 R0 (Ohm)
    double polarization_resistance; // 极化电阻 R1 (Ohm)
    double polarization_capacitance; // 极化电容 C1 (F)
    
    // 温度参数
    double thermal_resistance;    // 热阻 (K/W)
    double thermal_capacitance;   // 热容 (J/K)
    double max_temperature;       // 最大温度 (°C)
    double min_temperature;       // 最小温度 (°C)
    
    // SOC-OCV曲线参数（多项式拟合）
    std::vector<double> ocv_coefficients;  // OCV = f(SOC)
    
    BatteryConfig()
        : nominal_capacity(100.0)           // 100Ah
        , nominal_voltage(400.0)            // 400V
        , max_voltage(420.0)                // 420V
        , min_voltage(300.0)                // 300V
        , max_charge_current(100.0)         // 100A充电
        , max_discharge_current(200.0)      // 200A放电
        , series_resistance(0.02)           // 20mΩ
        , polarization_resistance(0.01)     // 10mΩ
        , polarization_capacitance(2000.0)  // 2000F
        , thermal_resistance(0.2)           // 0.2 K/W
        , thermal_capacitance(10000.0)      // 10000 J/K
        , max_temperature(60.0)             // 60°C
        , min_temperature(-20.0)            // -20°C
        , ocv_coefficients({400.0, 20.0, -5.0, 0.5})  // 400 + 20*SOC - 5*SOC² + 0.5*SOC³
    {}
};

/**
 * @brief 电池状态
 */
struct BatteryState {
    double soc;                   // 荷电状态 SOC (0-1)
    double soh;                   // 健康状态 SOH (0-1)
    double terminal_voltage;      // 端电压 (V)
    double open_circuit_voltage;  // 开路电压 OCV (V)
    double current;               // 电流 (A，正为放电)
    double polarization_voltage;  // 极化电压 (V)
    double internal_temperature;  // 内部温度 (°C)
    double surface_temperature;   // 表面温度 (°C)
    double power;                 // 功率 (W)
    double energy_remaining;      // 剩余能量 (Wh)
    double capacity_remaining;    // 剩余容量 (Ah)
    
    BatteryState()
        : soc(0.8)  // 初始80% SOC
        , soh(1.0)  // 初始100% SOH
        , terminal_voltage(400.0)
        , open_circuit_voltage(400.0)
        , current(0.0)
        , polarization_voltage(0.0)
        , internal_temperature(25.0)
        , surface_temperature(25.0)
        , power(0.0)
        , energy_remaining(32.0)  // 100Ah * 400V * 0.8 = 32kWh
        , capacity_remaining(80.0)
    {}
};

/**
 * @brief 电池输入
 */
struct BatteryInput {
    double current;               // 电流 (A，正为放电)
    double ambient_temperature;   // 环境温度 (°C)
    
    BatteryInput()
        : current(0.0)
        , ambient_temperature(25.0)
    {}
};

/**
 * @brief 锂离子电池模型类
 */
class Battery {
public:
    /**
     * @brief 构造函数
     * @param config 电池配置参数
     */
    explicit Battery(const BatteryConfig& config = BatteryConfig());
    
    /**
     * @brief 初始化电池
     */
    void initialize();
    
    /**
     * @brief 更新电池状态（时间步进）
     * @param dt 时间步长 (s)
     * @param input 输入电流
     */
    void update(double dt, const BatteryInput& input);
    
    /**
     * @brief 获取当前状态
     * @return 电池状态
     */
    const BatteryState& getState() const { return state_; }
    
    /**
     * @brief 获取配置参数
     * @return 电池配置
     */
    const BatteryConfig& getConfig() const { return config_; }
    
    /**
     * @brief 设置SOC
     * @param soc 荷电状态 (0-1)
     */
    void setSOC(double soc);
    
    /**
     * @brief 重置电池状态
     */
    void reset();

private:
    /**
     * @brief 计算开路电压（OCV）
     * @param soc 荷电状态
     * @return 开路电压 (V)
     */
    double calculateOCV(double soc) const;
    
    /**
     * @brief 更新SOC（安时积分法）
     * @param dt 时间步长 (s)
     * @param current 电流 (A)
     */
    void updateSOC(double dt, double current);
    
    /**
     * @brief 更新极化电压
     * @param dt 时间步长 (s)
     * @param current 电流 (A)
     */
    void updatePolarization(double dt, double current);
    
    /**
     * @brief 计算端电压
     * @param current 电流 (A)
     */
    void calculateTerminalVoltage(double current);
    
    /**
     * @brief 更新温度
     * @param dt 时间步长 (s)
     * @param current 电流 (A)
     * @param ambient_temp 环境温度 (°C)
     */
    void updateTemperature(double dt, double current, double ambient_temp);
    
    /**
     * @brief 电流限制
     * @param current 电流 (A)
     * @return 限制后的电流 (A)
     */
    double limitCurrent(double current) const;
    
    /**
     * @brief 计算剩余能量和容量
     */
    void calculateRemaining();

private:
    BatteryConfig config_;  // 电池配置
    BatteryState state_;    // 电池状态
};

} // namespace powertrain
} // namespace ev_dynamics

#endif // EV_DYNAMICS_BATTERY_H
