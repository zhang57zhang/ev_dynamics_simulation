/**
 * @file rk4_integrator.cpp
 * @brief 4阶Runge-Kutta积分器实现
 * 
 * 实现高精度RK4积分器，支持：
 * - 固定步长积分
 * - 自适应步长控制（Richardson外推）
 * - 误差估计
 * - 步长自动调整
 * 
 * @author BackendAgent #3
 * @date 2026-03-06
 */

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include "rk4_integrator.h"

namespace ev_simulation {
namespace dynamics {

using Eigen::VectorXd;
using Eigen::MatrixXd;

// ============================================================================
// RK4Integrator 实现
// ============================================================================

RK4Integrator::RK4Integrator()
    : state_()
    , time_(0.0)
    , error_estimate_(0.0)
    , step_count_(0)
    , adaptive_enabled_(true)
    , min_step_(1e-6)
    , max_step_(0.1)
    , tolerance_(1e-6)
    , safety_factor_(0.9)
{
}

RK4Integrator::~RK4Integrator() = default;

void RK4Integrator::setState(const VectorXd& state) {
    state_ = state;
}

void RK4Integrator::setTime(double time) {
    time_ = time;
}

VectorXd RK4Integrator::getState() const {
    return state_;
}

double RK4Integrator::getTime() const {
    return time_;
}

void RK4Integrator::setAdaptive(bool enabled) {
    adaptive_enabled_ = enabled;
}

void RK4Integrator::setTolerance(double tolerance) {
    if (tolerance <= 0) {
        throw std::invalid_argument("Tolerance must be positive");
    }
    tolerance_ = tolerance;
}

void RK4Integrator::setStepLimits(double min_step, double max_step) {
    if (min_step <= 0 || max_step <= 0 || min_step > max_step) {
        throw std::invalid_argument("Invalid step limits");
    }
    min_step_ = min_step;
    max_step_ = max_step;
}

VectorXd RK4Integrator::step(ODEFunction f, double dt) {
    int n = state_.size();
    
    if (n == 0) {
        throw std::runtime_error("State not initialized");
    }
    
    // k1 = f(t, y)
    VectorXd k1 = f(time_, state_);
    
    // k2 = f(t + dt/2, y + dt*k1/2)
    VectorXd k2 = f(time_ + dt/2, state_ + dt/2 * k1);
    
    // k3 = f(t + dt/2, y + dt*k2/2)
    VectorXd k3 = f(time_ + dt/2, state_ + dt/2 * k2);
    
    // k4 = f(t + dt, y + dt*k3)
    VectorXd k4 = f(time_ + dt, state_ + dt * k3);
    
    // y_new = y + dt/6 * (k1 + 2*k2 + 2*k3 + k4)
    VectorXd new_state = state_ + dt/6 * (k1 + 2*k2 + 2*k3 + k4);
    
    // 误差估计（使用Richardson外推的简化版本）
    error_estimate_ = (k1 - k2 - k3 + k4).norm() * dt / 6;
    
    // 更新状态
    state_ = new_state;
    time_ += dt;
    step_count_++;
    
    return state_;
}

VectorXd RK4Integrator::integrate(ODEFunction f, double t_end, double initial_dt) {
    double dt = initial_dt;
    double remaining = t_end - time_;
    
    while (remaining > 0) {
        if (adaptive_enabled_) {
            // 自适应步长积分
            dt = adaptiveStep(f, remaining);
        } else {
            // 固定步长积分
            if (dt > remaining) {
                dt = remaining;
            }
            step(f, dt);
        }
        
        remaining = t_end - time_;
    }
    
    return state_;
}

double RK4Integrator::adaptiveStep(ODEFunction f, double remaining) {
    double dt = std::min(max_step_, remaining);
    bool step_accepted = false;
    int attempts = 0;
    const int max_attempts = 20;
    
    while (!step_accepted && attempts < max_attempts) {
        attempts++;
        
        // 尝试一步积分
        VectorXd state_backup = state_;
        double time_backup = time_;
        
        // 使用当前步长进行RK4
        VectorXd result_full = step(f, dt);
        VectorXd state_full = state_;
        double error_full = error_estimate_;
        
        // 恢复状态
        state_ = state_backup;
        time_ = time_backup;
        
        // 使用半步长进行两步RK4（用于误差估计）
        double half_dt = dt / 2;
        step(f, half_dt);
        VectorXd result_half = step(f, half_dt);
        VectorXd state_half = state_;
        
        // 计算误差（Richardson外推）
        double error = (state_full - state_half).norm();
        
        // 相对误差
        double relative_error = error / (state_half.norm() + 1e-10);
        
        // 判断步长是否可接受
        if (relative_error < tolerance_ || dt <= min_step_) {
            // 接受步长
            state_ = state_half;  // 使用更高精度的半步结果
            time_ = time_backup + dt;
            step_count_++;
            error_estimate_ = error;
            step_accepted = true;
            
            // 如果误差很小，可以增加步长
            if (relative_error < tolerance_ / 10) {
                dt = std::min(dt * 2.0, max_step_);
            }
        } else {
            // 拒绝步长，减小步长重试
            double factor = safety_factor_ * std::pow(tolerance_ / relative_error, 0.2);
            factor = std::max(0.1, std::min(0.5, factor));  // 限制减小幅度
            dt = std::max(dt * factor, min_step_);
            
            // 恢复状态
            state_ = state_backup;
            time_ = time_backup;
        }
    }
    
    if (!step_accepted) {
        throw std::runtime_error("RK4 integrator failed to converge after " + 
                                std::to_string(max_attempts) + " attempts");
    }
    
    return dt;
}

double RK4Integrator::getErrorEstimate() const {
    return error_estimate_;
}

size_t RK4Integrator::getStepCount() const {
    return step_count_;
}

void RK4Integrator::reset() {
    state_ = VectorXd();
    time_ = 0.0;
    error_estimate_ = 0.0;
    step_count_ = 0;
}

// ============================================================================
// AdaptiveRK4Integrator 实现
// ============================================================================

AdaptiveRK4Integrator::AdaptiveRK4Integrator()
    : RK4Integrator()
    , error_history_()
    , step_history_()
    , max_history_(100)
{
}

AdaptiveRK4Integrator::~AdaptiveRK4Integrator() = default;

void AdaptiveRK4Integrator::setHistorySize(size_t size) {
    max_history_ = size;
    while (error_history_.size() > max_history_) {
        error_history_.pop_front();
        step_history_.pop_front();
    }
}

VectorXd AdaptiveRK4Integrator::integrateWithMonitoring(
    ODEFunction f, 
    double t_end, 
    double initial_dt,
    MonitoringCallback callback
) {
    double dt = initial_dt;
    double remaining = t_end - getTime();
    
    while (remaining > 0) {
        double old_time = getTime();
        VectorXd old_state = getState();
        
        // 执行一步
        dt = adaptiveStep(f, remaining);
        
        // 记录历史
        error_history_.push_back(getErrorEstimate());
        step_history_.push_back(dt);
        
        // 限制历史大小
        if (error_history_.size() > max_history_) {
            error_history_.pop_front();
            step_history_.pop_front();
        }
        
        // 回调
        if (callback) {
            callback(old_time, old_state, getTime(), getState(), dt, getErrorEstimate());
        }
        
        remaining = t_end - getTime();
    }
    
    return getState();
}

double AdaptiveRK4Integrator::getAverageError() const {
    if (error_history_.empty()) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (double e : error_history_) {
        sum += e;
    }
    return sum / error_history_.size();
}

double AdaptiveRK4Integrator::getMaxError() const {
    if (error_history_.empty()) {
        return 0.0;
    }
    
    double max_err = 0.0;
    for (double e : error_history_) {
        max_err = std::max(max_err, e);
    }
    return max_err;
}

double AdaptiveRK4Integrator::getAverageStep() const {
    if (step_history_.empty()) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (double s : step_history_) {
        sum += s;
    }
    return sum / step_history_.size();
}

// ============================================================================
// VehicleRK4Integrator 实现
// ============================================================================

VehicleRK4Integrator::VehicleRK4Integrator(VehicleDynamics* dynamics)
    : dynamics_(dynamics)
{
    if (!dynamics_) {
        throw std::invalid_argument("VehicleDynamics pointer cannot be null");
    }
}

VehicleRK4Integrator::~VehicleRK4Integrator() = default;

void VehicleRK4Integrator::initialize() {
    if (!dynamics_->isInitialized()) {
        throw std::runtime_error("VehicleDynamics not initialized");
    }
    
    // 将车辆状态转换为向量形式
    state_vector_ = vehicleStateToVector(dynamics_->getState());
    setTime(0.0);
}

VectorXd VehicleRK4Integrator::integrateStep(double dt) {
    // 定义ODE函数
    auto ode_func = [this](double t, const VectorXd& y) -> VectorXd {
        // 从向量恢复车辆状态
        VehicleState vs = vectorToVehicleState(y);
        dynamics_->reset(vs);
        
        // 计算导数
        VehicleStateDerivative deriv = dynamics_->computeDerivatives();
        
        // 转换为向量
        return stateDerivativeToVector(deriv);
    };
    
    // 执行RK4步进
    state_vector_ = step(ode_func, dt);
    
    // 更新车辆动力学模型状态
    VehicleState new_state = vectorToVehicleState(state_vector_);
    dynamics_->reset(new_state);
    
    return state_vector_;
}

VectorXd VehicleRK4Integrator::vehicleStateToVector(const VehicleState& state) const {
    VectorXd vec(18);
    
    // 位置
    vec(0) = state.x;
    vec(1) = state.y;
    vec(2) = state.z;
    
    // 姿态
    vec(3) = state.roll;
    vec(4) = state.pitch;
    vec(5) = state.yaw;
    
    // 速度
    vec(6) = state.vx;
    vec(7) = state.vy;
    vec(8) = state.vz;
    
    // 角速度
    vec(9) = state.omega_x;
    vec(10) = state.omega_y;
    vec(11) = state.omega_z;
    
    // 加速度（存储用于输出）
    vec(12) = state.ax;
    vec(13) = state.ay;
    vec(14) = state.az;
    
    // 角加速度
    vec(15) = state.alpha_x;
    vec(16) = state.alpha_y;
    vec(17) = state.alpha_z;
    
    return vec;
}

VehicleState VehicleRK4Integrator::vectorToVehicleState(const VectorXd& vec) const {
    VehicleState state;
    
    state.x = vec(0);
    state.y = vec(1);
    state.z = vec(2);
    
    state.roll = vec(3);
    state.pitch = vec(4);
    state.yaw = vec(5);
    
    state.vx = vec(6);
    state.vy = vec(7);
    state.vz = vec(8);
    
    state.omega_x = vec(9);
    state.omega_y = vec(10);
    state.omega_z = vec(11);
    
    state.ax = vec(12);
    state.ay = vec(13);
    state.az = vec(14);
    
    state.alpha_x = vec(15);
    state.alpha_y = vec(16);
    state.alpha_z = vec(17);
    
    return state;
}

VectorXd VehicleRK4Integrator::stateDerivativeToVector(const VehicleStateDerivative& deriv) const {
    VectorXd vec(18);
    
    vec(0) = deriv.x;
    vec(1) = deriv.y;
    vec(2) = deriv.z;
    
    vec(3) = deriv.roll;
    vec(4) = deriv.pitch;
    vec(5) = deriv.yaw;
    
    vec(6) = deriv.vx;
    vec(7) = deriv.vy;
    vec(8) = deriv.vz;
    
    vec(9) = deriv.omega_x;
    vec(10) = deriv.omega_y;
    vec(11) = deriv.omega_z;
    
    vec(12) = deriv.ax;
    vec(13) = deriv.ay;
    vec(14) = deriv.az;
    
    vec(15) = deriv.alpha_x;
    vec(16) = deriv.alpha_y;
    vec(17) = deriv.alpha_z;
    
    return vec;
}

} // namespace dynamics
} // namespace ev_simulation
