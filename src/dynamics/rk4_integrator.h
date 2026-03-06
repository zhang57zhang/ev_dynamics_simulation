/**
 * @file rk4_integrator.h
 * @brief 4阶Runge-Kutta积分器头文件
 * 
 * @author BackendAgent #3
 * @date 2026-03-06
 */

#pragma once

#include <functional>
#include <deque>
#include <Eigen/Dense>
#include "vehicle_dynamics.h"

namespace ev_simulation {
namespace dynamics {

using Eigen::VectorXd;
using Eigen::MatrixXd;

/**
 * @brief ODE函数类型：dy/dt = f(t, y)
 */
using ODEFunction = std::function<VectorXd(double, const VectorXd&)>;

/**
 * @brief 监控回调函数类型
 */
using MonitoringCallback = std::function<void(
    double old_time, const VectorXd& old_state,
    double new_time, const VectorXd& new_state,
    double dt, double error
)>;

/**
 * @brief 4阶Runge-Kutta积分器
 * 
 * 支持固定步长和自适应步长积分
 */
class RK4Integrator {
public:
    RK4Integrator();
    virtual ~RK4Integrator();
    
    // === 状态管理 ===
    
    /**
     * @brief 设置当前状态
     * @param state 状态向量
     */
    void setState(const VectorXd& state);
    
    /**
     * @brief 设置当前时间
     * @param time 时间 (s)
     */
    void setTime(double time);
    
    /**
     * @brief 获取当前状态
     * @return 状态向量
     */
    VectorXd getState() const;
    
    /**
     * @brief 获取当前时间
     * @return 时间 (s)
     */
    double getTime() const;
    
    // === 配置 ===
    
    /**
     * @brief 启用/禁用自适应步长
     * @param enabled 是否启用
     */
    void setAdaptive(bool enabled);
    
    /**
     * @brief 设置误差容限
     * @param tolerance 容限
     */
    void setTolerance(double tolerance);
    
    /**
     * @brief 设置步长限制
     * @param min_step 最小步长 (s)
     * @param max_step 最大步长 (s)
     */
    void setStepLimits(double min_step, double max_step);
    
    // === 积分 ===
    
    /**
     * @brief 执行一步RK4积分（固定步长）
     * @param f ODE函数
     * @param dt 时间步长 (s)
     * @return 新状态
     */
    VectorXd step(ODEFunction f, double dt);
    
    /**
     * @brief 积分到指定时间
     * @param f ODE函数
     * @param t_end 结束时间 (s)
     * @param initial_dt 初始步长 (s)
     * @return 最终状态
     */
    VectorXd integrate(ODEFunction f, double t_end, double initial_dt);
    
    // === 查询 ===
    
    /**
     * @brief 获取误差估计
     * @return 误差估计
     */
    double getErrorEstimate() const;
    
    /**
     * @brief 获取步数
     * @return 步数
     */
    size_t getStepCount() const;
    
    /**
     * @brief 重置积分器
     */
    void reset();
    
protected:
    /**
     * @brief 自适应步长积分
     * @param f ODE函数
     * @param remaining 剩余时间 (s)
     * @return 实际使用的步长
     */
    double adaptiveStep(ODEFunction f, double remaining);
    
    // === 成员变量 ===
    VectorXd state_;
    double time_;
    double error_estimate_;
    size_t step_count_;
    
    // 自适应步长参数
    bool adaptive_enabled_;
    double min_step_;
    double max_step_;
    double tolerance_;
    double safety_factor_;
};

/**
 * @brief 带监控的自适应RK4积分器
 */
class AdaptiveRK4Integrator : public RK4Integrator {
public:
    AdaptiveRK4Integrator();
    ~AdaptiveRK4Integrator() override;
    
    /**
     * @brief 设置历史记录大小
     * @param size 最大历史记录数
     */
    void setHistorySize(size_t size);
    
    /**
     * @brief 带监控的积分
     * @param f ODE函数
     * @param t_end 结束时间 (s)
     * @param initial_dt 初始步长 (s)
     * @param callback 监控回调
     * @return 最终状态
     */
    VectorXd integrateWithMonitoring(
        ODEFunction f, 
        double t_end, 
        double initial_dt,
        MonitoringCallback callback
    );
    
    /**
     * @brief 获取平均误差
     * @return 平均误差
     */
    double getAverageError() const;
    
    /**
     * @brief 获取最大误差
     * @return 最大误差
     */
    double getMaxError() const;
    
    /**
     * @brief 获取平均步长
     * @return 平均步长
     */
    double getAverageStep() const;
    
private:
    std::deque<double> error_history_;
    std::deque<double> step_history_;
    size_t max_history_;
};

/**
 * @brief 车辆动力学专用RK4积分器
 */
class VehicleRK4Integrator : public RK4Integrator {
public:
    /**
     * @brief 构造函数
     * @param dynamics 车辆动力学模型指针
     */
    explicit VehicleRK4Integrator(VehicleDynamics* dynamics);
    ~VehicleRK4Integrator() override;
    
    /**
     * @brief 初始化积分器
     */
    void initialize();
    
    /**
     * @brief 执行一步积分
     * @param dt 时间步长 (s)
     * @return 新状态向量
     */
    VectorXd integrateStep(double dt);
    
private:
    /**
     * @brief 将车辆状态转换为向量
     * @param state 车辆状态
     * @return 状态向量
     */
    VectorXd vehicleStateToVector(const VehicleState& state) const;
    
    /**
     * @brief 将向量转换为车辆状态
     * @param vec 状态向量
     * @return 车辆状态
     */
    VehicleState vectorToVehicleState(const VectorXd& vec) const;
    
    /**
     * @brief 将状态导数转换为向量
     * @param deriv 状态导数
     * @return 导数向量
     */
    VectorXd stateDerivativeToVector(const VehicleStateDerivative& deriv) const;
    
    VehicleDynamics* dynamics_;
    VectorXd state_vector_;
};

} // namespace dynamics
} // namespace ev_simulation
