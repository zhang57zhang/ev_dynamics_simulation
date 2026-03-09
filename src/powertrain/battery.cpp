/**
 * @file battery.cpp
 * @brief 锂离子电池等效电路模型实现
 * @author CodeCraft
 * @date 2026-03-08
 */

#include "battery.h"
#include <algorithm>
#include <iostream>
#include <cmath>

namespace ev_dynamics {
namespace powertrain {

Battery::Battery(const BatteryConfig& config)
    : config_(config)
{
    initialize();
}

void Battery::initialize() {
    state_ = BatteryState();
    state_.soc = 0.8;  // 初始80% SOC
    state_.soh = 1.0;  // 初始100% SOH
    state_.open_circuit_voltage = calculateOCV(state_.soc);
    state_.terminal_voltage = state_.open_circuit_voltage;
    
    calculateRemaining();
    
    std::cout << "[Battery] Initialized: " << config_.nominal_capacity 
              << "Ah, " << config_.nominal_voltage << "V, SOC: " 
              << (state_.soc * 100) << "%" << std::endl;
}

void Battery::update(double dt, const BatteryInput& input) {
    // 1. 电流限制
    double limited_current = limitCurrent(input.current);
    state_.current = limited_current;
    
    // 2. 更新SOC（安时积分法）
    updateSOC(dt, limited_current);
    
    // 3. 更新开路电压
    state_.open_circuit_voltage = calculateOCV(state_.soc);
    
    // 4. 更新极化电压
    updatePolarization(dt, limited_current);
    
    // 5. 计算端电压
    calculateTerminalVoltage(limited_current);
    
    // 6. 更新温度
    updateTemperature(dt, limited_current, input.ambient_temperature);
    
    // 7. 计算功率
    state_.power = state_.terminal_voltage * state_.current;
    
    // 8. 计算剩余能量和容量
    calculateRemaining();
}

double Battery::calculateOCV(double soc) const {
    // 多项式拟合：OCV = a0 + a1*SOC + a2*SOC² + a3*SOC³
    double ocv = 0.0;
    
    if (config_.ocv_coefficients.size() >= 4) {
        double soc2 = soc * soc;
        double soc3 = soc2 * soc;
        
        ocv = config_.ocv_coefficients[0] +
              config_.ocv_coefficients[1] * soc +
              config_.ocv_coefficients[2] * soc2 +
              config_.ocv_coefficients[3] * soc3;
    } else {
        // 简化模型：线性插值
        ocv = config_.min_voltage + 
              (config_.max_voltage - config_.min_voltage) * soc;
    }
    
    return ocv;
}

void Battery::updateSOC(double dt, double current) {
    // 安时积分法：SOC(t) = SOC(t-1) - (I * dt) / (C * 3600)
    // I: 电流（A），dt: 时间（s），C: 容量（Ah）
    
    double delta_soc = (current * dt) / (config_.nominal_capacity * 3600.0);
    state_.soc -= delta_soc;  // 放电电流为正，SOC减少
    
    // SOC限制在[0, 1]
    state_.soc = std::clamp(state_.soc, 0.0, 1.0);
}

void Battery::updatePolarization(double dt, double current) {
    // Thevenin模型极化电压：dU1/dt = (I*R1 - U1) / (R1*C1)
    // U1: 极化电压，I: 电流，R1: 极化电阻，C1: 极化电容
    
    double tau = config_.polarization_resistance * config_.polarization_capacitance;
    double target_voltage = current * config_.polarization_resistance;
    
    // 一阶RC电路响应
    double voltage_change = (target_voltage - state_.polarization_voltage) * 
                           (1.0 - std::exp(-dt / tau));
    
    state_.polarization_voltage += voltage_change;
}

void Battery::calculateTerminalVoltage(double current) {
    // 端电压 = OCV - I*R0 - U1
    // OCV: 开路电压，I: 电流，R0: 串联电阻，U1: 极化电压
    
    double voltage_drop = current * config_.series_resistance + 
                         state_.polarization_voltage;
    
    state_.terminal_voltage = state_.open_circuit_voltage - voltage_drop;
    
    // 电压限制
    state_.terminal_voltage = std::clamp(state_.terminal_voltage,
                                        config_.min_voltage,
                                        config_.max_voltage);
}

void Battery::updateTemperature(double dt, double current, double ambient_temp) {
    // 热模型：Cth * dT/dt = I²*R - (T - Tamb) / Rth
    // Cth: 热容，I: 电流，R: 内阻，Rth: 热阻，Tamb: 环境温度
    
    // 计算发热功率（简化模型）
    double internal_resistance = config_.series_resistance + 
                                config_.polarization_resistance;
    double heat_generated = current * current * internal_resistance;
    
    // 散热功率
    double heat_dissipated = (state_.internal_temperature - ambient_temp) / 
                            config_.thermal_resistance;
    
    // 温度变化
    double temperature_change = (heat_generated - heat_dissipated) * dt / 
                               config_.thermal_capacitance;
    
    state_.internal_temperature += temperature_change;
    
    // 表面温度近似（略低于内部温度）
    state_.surface_temperature = state_.internal_temperature - 
                                (state_.internal_temperature - ambient_temp) * 0.3;
    
    // 温度限制检查
    if (state_.internal_temperature > config_.max_temperature) {
        std::cerr << "[Battery] WARNING: Temperature exceeded: " 
                  << state_.internal_temperature << "°C > " 
                  << config_.max_temperature << "°C" << std::endl;
    }
}

double Battery::limitCurrent(double current) const {
    double max_current;
    
    if (current > 0) {
        // 放电电流限制
        max_current = config_.max_discharge_current;
    } else {
        // 充电电流限制
        max_current = config_.max_charge_current;
    }
    
    // SOC相关限制（低SOC时限制放电，高SOC时限制充电）
    if (current > 0 && state_.soc < 0.1) {
        // SOC < 10%时，限制放电电流
        max_current *= state_.soc / 0.1;
    } else if (current < 0 && state_.soc > 0.9) {
        // SOC > 90%时，限制充电电流
        max_current *= (1.0 - state_.soc) / 0.1;
    }
    
    return std::clamp(current, -max_current, max_current);
}

void Battery::calculateRemaining() {
    // 剩余容量（Ah）
    state_.capacity_remaining = state_.soc * config_.nominal_capacity * state_.soh;
    
    // 剩余能量（kWh）
    // E = V * C * SOC
    state_.energy_remaining = (state_.terminal_voltage * 
                              state_.capacity_remaining) / 1000.0;
}

void Battery::setSOC(double soc) {
    state_.soc = std::clamp(soc, 0.0, 1.0);
    state_.open_circuit_voltage = calculateOCV(state_.soc);
    calculateRemaining();
}

void Battery::reset() {
    state_ = BatteryState();
    state_.open_circuit_voltage = calculateOCV(state_.soc);
    calculateRemaining();
    
    std::cout << "[Battery] Reset to initial state (SOC: " 
              << (state_.soc * 100) << "%)" << std::endl;
}

} // namespace powertrain
} // namespace ev_dynamics
