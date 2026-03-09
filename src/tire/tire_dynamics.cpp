/**
 * @file tire_dynamics.cpp
 * @brief 轮胎动力学模型实现
 * 
 * 实现轮胎动力学，包括：
 * - 滑移率计算：kappa = (V_wheel - V_vehicle) / V_vehicle
 * - 松弛长度模型：d(sigma)/dt = (V - V0) / sigma_relaxation
 * - 动态响应：考虑轮胎变形的延迟
 * - 温度影响：mu_eff = mu * (1 + coeff * (T - T0))
 * - 滚动阻力：F_roll = f * Fz * (1 + v/v0)
 * 
 * 参考文献：
 * - Pacejka, H.B. (2012). Tire and Vehicle Dynamics. 3rd Edition.
 * - Gent, A.N., Walter, J.D. (2006). The Pneumatic Tire.
 * 
 * @author TireAgent
 * @date 2026-03-08
 */

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <Eigen/Dense>
#include "tire_dynamics.h"

namespace ev_simulation {
namespace tire {

using Eigen::Vector3d;

// ============================================================================
// TireConfig 实现
// ============================================================================

void TireConfig::validate() const {
    if (radius <= 0) {
        throw std::invalid_argument("Tire radius must be positive");
    }
    if (width <= 0) {
        throw std::invalid_argument("Tire width must be positive");
    }
    if (inertia <= 0) {
        throw std::invalid_argument("Tire inertia must be positive");
    }
    if (mass <= 0) {
        throw std::invalid_argument("Tire mass must be positive");
    }
    if (relaxation_length_long <= 0) {
        throw std::invalid_argument("Longitudinal relaxation length must be positive");
    }
    if (relaxation_length_lat <= 0) {
        throw std::invalid_argument("Lateral relaxation length must be positive");
    }
    if (rolling_resistance_coeff < 0) {
        throw std::invalid_argument("Rolling resistance coefficient cannot be negative");
    }
}

// ============================================================================
// TireDynamics 实现
// ============================================================================

TireDynamics::TireDynamics()
    : pacejka_model_(std::make_unique<PacejkaModel>())
    , state_()
    , motion_()
    , initialized_(false)
{
}

TireDynamics::TireDynamics(const TireConfig& tire_config, const PacejkaParameters& pacejka_params)
    : pacejka_model_(std::make_unique<PacejkaModel>(pacejka_params))
    , config_(tire_config)
    , state_()
    , motion_()
    , initialized_(false)
{
    config_.validate();
    initialized_ = true;
}

TireDynamics::~TireDynamics() = default;

void TireDynamics::initialize(const TireConfig& tire_config, const PacejkaParameters& pacejka_params) {
    config_ = tire_config;
    config_.validate();
    
    pacejka_model_ = std::make_unique<PacejkaModel>(pacejka_params);
    
    initialized_ = true;
    reset();
}

void TireDynamics::reset() {
    if (!initialized_) {
        throw std::runtime_error("TireDynamics not initialized");
    }
    
    state_ = TireDynamicState{};
    motion_ = WheelMotion{};
}

void TireDynamics::setWheelMotion(const WheelMotion& motion) {
    motion_ = motion;
}

void TireDynamics::setVerticalLoad(double Fz) {
    state_.Fz = Fz;
}

void TireDynamics::setTemperature(const TireTemperature& temperature) {
    state_.temperature = temperature;
}

TireDynamicsOutput TireDynamics::computeForces() {
    if (!initialized_) {
        throw std::runtime_error("TireDynamics not initialized");
    }
    
    TireDynamicsOutput output;
    
    // 1. 计算滑移率
    state_.slip = computeSlip(motion_);
    
    // 2. 应用松弛长度模型
    SlipState relaxed_slip = applyRelaxation(state_.slip, motion_.V_wheel);
    
    // 3. 计算有效摩擦系数（温度影响）
    double effective_friction = computeEffectiveFriction(
        config_.friction_base + config_.friction_load_sensitivity * state_.Fz / 4000.0,
        state_.temperature.surface_temp
    );
    
    // 4. 准备Pacejka输入
    TireState tire_state;
    tire_state.kappa = relaxed_slip.kappa_dynamic;
    tire_state.alpha = relaxed_slip.alpha_dynamic;
    tire_state.gamma = state_.slip.gamma;
    tire_state.Fz = state_.Fz;
    tire_state.V_wheel = motion_.V_wheel;
    
    // 5. 计算Pacejka力
    output.forces = pacejka_model_->computeForces(tire_state);
    
    // 6. 应用摩擦系数修正
    output.forces.Fx *= effective_friction;
    output.forces.Fy *= effective_friction;
    output.forces.Mz *= effective_friction;
    
    // 7. 添加滚动阻力
    double rolling_resistance = computeRollingResistance(state_.Fz, motion_.V_vehicle);
    output.forces.Fx -= rolling_resistance;
    
    // 8. 更新输出
    output.slip = relaxed_slip;
    output.omega = state_.omega;
    output.effective_radius = computeEffectiveRadius(state_.Fz);
    
    return output;
}

void TireDynamics::update(double dt, double driving_torque, double brake_torque) {
    if (!initialized_) {
        throw std::runtime_error("TireDynamics not initialized");
    }
    
    // 1. 更新松弛长度模型
    updateRelaxation(dt);
    
    // 2. 更新车轮转速
    updateWheelSpeed(dt, driving_torque, brake_torque);
    
    // 3. 更新温度
    updateTemperature(dt, state_.slip);
}

SlipState TireDynamics::computeSlip(const WheelMotion& motion) {
    SlipState slip;
    
    // === 纵向滑移率 kappa ===
    // kappa = (V_wheel - V_vehicle) / V_vehicle
    // 其中 V_wheel = omega * r_effective
    
    double effective_radius = computeEffectiveRadius(state_.Fz);
    double V_wheel = motion.omega * effective_radius;
    double V_vehicle_long = motion.Vx;
    
    // 防止除零
    if (std::abs(V_vehicle_long) > 0.1) {
        slip.kappa = (V_wheel - V_vehicle_long) / std::abs(V_vehicle_long);
    } else {
        slip.kappa = 0.0;
    }
    
    // 限制滑移率范围
    slip.kappa = std::clamp(slip.kappa, -1.0, 1.0);
    
    // === 侧偏角 alpha ===
    // alpha = atan(Vy / Vx)
    
    if (std::abs(motion.Vx) > 0.1) {
        slip.alpha = std::atan2(motion.Vy, std::abs(motion.Vx));
    } else {
        slip.alpha = 0.0;
    }
    
    // === 外倾角 gamma ===
    slip.gamma = motion.camber_angle;
    
    // === 滑移速度 ===
    slip.slip_velocity_x = V_wheel - V_vehicle_long;
    slip.slip_velocity_y = motion.Vy;
    
    return slip;
}

SlipState TireDynamics::applyRelaxation(const SlipState& slip, double V_wheel) {
    SlipState relaxed_slip = slip;
    
    // 松弛长度模型：
    // d(sigma_x)/dt = (kappa - sigma_x) * V / sigma_relaxation_long
    // d(sigma_y)/dt = (alpha - sigma_y) * V / sigma_relaxation_lat
    //
    // 这里使用一阶滞后模型：
    // tau = sigma_relaxation / V
    // sigma_dynamic = sigma + (sigma_input - sigma) * (1 - exp(-dt / tau))
    
    if (V_wheel > 0.1) {
        double tau_x = config_.relaxation_length_long / V_wheel;
        double tau_y = config_.relaxation_length_lat / V_wheel;
        
        // 纵向松弛
        relaxed_slip.kappa_dynamic = state_.sigma_x;
        
        // 横向松弛
        relaxed_slip.alpha_dynamic = state_.sigma_y;
    } else {
        // 低速时忽略松弛效应
        relaxed_slip.kappa_dynamic = slip.kappa;
        relaxed_slip.alpha_dynamic = slip.alpha;
    }
    
    return relaxed_slip;
}

double TireDynamics::computeRollingResistance(double Fz, double V) const {
    // 滚动阻力模型：
    // F_roll = f0 * Fz * (1 + (V/V0)^2)
    // 其中 f0 是基础滚动阻力系数，V0 是参考速度
    
    double V0 = 100.0 / 3.6;  // 100 km/h in m/s
    double speed_factor = 1.0 + config_.rolling_resistance_speed * (V * V) / (V0 * V0);
    
    return config_.rolling_resistance_coeff * Fz * speed_factor;
}

double TireDynamics::computeEffectiveRadius(double Fz) const {
    // 有效半径模型：
    // r_eff = r0 - delta_r
    // delta_r = Fz / K_radial
    // 简化模型：假设径向刚度约为 200 kN/m
    
    double K_radial = 200000.0;  // N/m
    double delta_r = Fz / K_radial;
    
    return config_.radius - delta_r;
}

double TireDynamics::computeEffectiveFriction(double base_friction, double temperature) const {
    // 温度影响模型：
    // mu_eff = mu * (1 + coeff * (T - T0))
    
    double temp_diff = temperature - config_.temperature_nominal;
    double temp_factor = 1.0 + config_.temperature_coefficient * temp_diff;
    
    return base_friction * temp_factor;
}

void TireDynamics::updateRelaxation(double dt) {
    // 更新松弛状态变量
    // 使用一阶低通滤波器
    
    if (motion_.V_wheel > 0.1) {
        // 纵向松弛
        double tau_x = config_.relaxation_length_long / motion_.V_wheel;
        double alpha_x = dt / (tau_x + dt);
        state_.sigma_x = state_.sigma_x + alpha_x * (state_.slip.kappa - state_.sigma_x);
        
        // 横向松弛
        double tau_y = config_.relaxation_length_lat / motion_.V_wheel;
        double alpha_y = dt / (tau_y + dt);
        state_.sigma_y = state_.sigma_y + alpha_y * (state_.slip.alpha - state_.sigma_y);
    } else {
        // 低速时直接使用滑移值
        state_.sigma_x = state_.slip.kappa;
        state_.sigma_y = state_.slip.alpha;
    }
}

void TireDynamics::updateWheelSpeed(double dt, double driving_torque, double brake_torque) {
    // 车轮转速更新：
    // I * d(omega)/dt = T_drive - T_brake - Fx * r_eff - M_roll
    //
    // 其中：
    // - T_drive: 驱动力矩
    // - T_brake: 制动力矩
    // - Fx: 纵向力
    // - r_eff: 有效半径
    // - M_roll: 滚动阻力矩
    
    // 计算轮胎力
    TireDynamicsOutput output = computeForces();
    
    // 力矩平衡
    double effective_radius = output.effective_radius;
    double total_torque = driving_torque - brake_torque - output.forces.Fx * effective_radius;
    
    // 角加速度
    double omega_dot = total_torque / config_.inertia;
    
    // 更新角速度（欧拉积分）
    state_.omega += omega_dot * dt;
    
    // 防止车轮反转（除非是倒车）
    if (motion_.V_vehicle > 0.1) {
        state_.omega = std::max(0.0, state_.omega);
    }
}

void TireDynamics::updateTemperature(double dt, const SlipState& slip) {
    // 简化的温度模型：
    // dT/dt = k1 * P_slip - k2 * (T - T_ambient)
    // 其中 P_slip 是滑移功率 = F * V_slip
    
    double ambient_temp = 25.0;  // °C
    
    // 计算滑移功率
    double slip_power = std::abs(slip.slip_velocity_x * state_.Fz * slip.kappa) + 
                       std::abs(slip.slip_velocity_y * state_.Fz * slip.alpha / 1000.0);
    
    // 温度变化率
    double k1 = 0.001;   // 加热系数
    double k2 = 0.01;    // 冷却系数
    
    double dT = k1 * slip_power - k2 * (state_.temperature.surface_temp - ambient_temp);
    
    // 更新温度（简化模型）
    state_.temperature.surface_temp += dT * dt;
    
    // 限制温度范围
    state_.temperature.surface_temp = std::clamp(state_.temperature.surface_temp, 0.0, 200.0);
    
    // 体积温度跟随表面温度（简化）
    double tau_bulk = 10.0;  // s
    state_.temperature.bulk_temp += (state_.temperature.surface_temp - state_.temperature.bulk_temp) * dt / tau_bulk;
}

// ============================================================================
// FourWheelTireSystem 实现
// ============================================================================

FourWheelTireSystem::FourWheelTireSystem()
    : initialized_(false)
{
    for (int i = 0; i < 4; ++i) {
        tires_[i] = std::make_unique<TireDynamics>();
    }
}

FourWheelTireSystem::~FourWheelTireSystem() = default;

void FourWheelTireSystem::initialize(const TireConfig& tire_config, const PacejkaParameters& pacejka_params) {
    for (int i = 0; i < 4; ++i) {
        tires_[i]->initialize(tire_config, pacejka_params);
    }
    initialized_ = true;
}

void FourWheelTireSystem::reset() {
    if (!initialized_) {
        throw std::runtime_error("FourWheelTireSystem not initialized");
    }
    
    for (int i = 0; i < 4; ++i) {
        tires_[i]->reset();
    }
}

void FourWheelTireSystem::setWheelMotion(int wheel_index, const WheelMotion& motion) {
    if (wheel_index < 0 || wheel_index >= 4) {
        throw std::invalid_argument("Wheel index must be 0-3");
    }
    
    tires_[wheel_index]->setWheelMotion(motion);
}

void FourWheelTireSystem::setVerticalLoad(int wheel_index, double Fz) {
    if (wheel_index < 0 || wheel_index >= 4) {
        throw std::invalid_argument("Wheel index must be 0-3");
    }
    
    tires_[wheel_index]->setVerticalLoad(Fz);
}

void FourWheelTireSystem::setAllWheelLoads(const std::array<double, 4>& Fz) {
    for (int i = 0; i < 4; ++i) {
        tires_[i]->setVerticalLoad(Fz[i]);
    }
}

std::array<TireDynamicsOutput, 4> FourWheelTireSystem::computeAllForces() {
    std::array<TireDynamicsOutput, 4> outputs;
    
    for (int i = 0; i < 4; ++i) {
        outputs[i] = tires_[i]->computeForces();
    }
    
    return outputs;
}

void FourWheelTireSystem::updateAll(double dt, 
                                   const std::array<double, 4>& driving_torques,
                                   const std::array<double, 4>& brake_torques) {
    for (int i = 0; i < 4; ++i) {
        tires_[i]->update(dt, driving_torques[i], brake_torques[i]);
    }
}

const TireDynamics& FourWheelTireSystem::getTire(int wheel_index) const {
    if (wheel_index < 0 || wheel_index >= 4) {
        throw std::invalid_argument("Wheel index must be 0-3");
    }
    
    return *tires_[wheel_index];
}

std::tuple<double, double, double> FourWheelTireSystem::getTotalForces() const {
    double Fx_total = 0.0;
    double Fy_total = 0.0;
    double Mz_total = 0.0;
    
    // 假设车辆参数（用于计算力矩）
    double wheelbase = 2.8;
    double track_width = 1.6;
    double xf = 1.4;
    double xr = -1.4;
    double wf = track_width / 2;
    
    for (int i = 0; i < 4; ++i) {
        TireDynamicsOutput output = tires_[i]->computeForces();
        
        Fx_total += output.forces.Fx;
        Fy_total += output.forces.Fy;
        
        // 计算横摆力矩（简化）
        double x = (i < 2) ? xf : xr;
        double y = (i % 2 == 0) ? wf : -wf;
        
        Mz_total += output.forces.Fy * x - output.forces.Fx * y;
    }
    
    return {Fx_total, Fy_total, Mz_total};
}

} // namespace tire
} // namespace ev_simulation
