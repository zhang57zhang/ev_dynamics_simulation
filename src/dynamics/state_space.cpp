/**
 * @file state_space.cpp
 * @brief 状态空间模型实现
 * 
 * 实现线性和非线性状态空间模型，包括：
 * - 线性系统：dx/dt = Ax + Bu, y = Cx + Du
 * - 非线性系统：dx/dt = f(x,u), y = h(x,u)
 * - 数值线性化（雅可比矩阵）
 * - 系统分析（特征值、稳定性）
 * 
 * @author BackendAgent #3
 * @date 2026-03-06
 */

#include <cmath>
#include <algorithm>
#include <complex>
#include <stdexcept>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include "state_space.h"

namespace ev_simulation {
namespace dynamics {

using Eigen::VectorXd;
using Eigen::MatrixXd;
using Eigen::EigenSolver;

// ============================================================================
// StateSpaceConfig 实现
// ============================================================================

void StateSpaceConfig::validate() const {
    if (state_dim <= 0 || input_dim <= 0 || output_dim <= 0) {
        throw std::invalid_argument("Dimensions must be positive");
    }
}

// ============================================================================
// LinearStateSpace 实现
// ============================================================================

LinearStateSpace::LinearStateSpace()
    : initialized_(false)
    , config_()
    , x_()
    , A_()
    , B_()
    , C_()
    , D_()
{
}

LinearStateSpace::LinearStateSpace(int state_dim, int input_dim, int output_dim)
    : LinearStateSpace()
{
    config_.state_dim = state_dim;
    config_.input_dim = input_dim;
    config_.output_dim = output_dim;
    config_.validate();
    
    initialize(config_);
}

LinearStateSpace::~LinearStateSpace() = default;

void LinearStateSpace::initialize(const StateSpaceConfig& config) {
    config.validate();
    config_ = config;
    
    // 初始化矩阵
    A_ = MatrixXd::Zero(config.state_dim, config.state_dim);
    B_ = MatrixXd::Zero(config.state_dim, config.input_dim);
    C_ = MatrixXd::Zero(config.output_dim, config.state_dim);
    D_ = MatrixXd::Zero(config.output_dim, config.input_dim);
    
    // 初始化状态向量
    x_ = VectorXd::Zero(config.state_dim);
    
    initialized_ = true;
}

void LinearStateSpace::reset(const VectorXd& initial_state) {
    if (!initialized_) {
        throw std::runtime_error("LinearStateSpace not initialized");
    }
    
    if (initial_state.size() != config_.state_dim) {
        throw std::invalid_argument("Initial state dimension mismatch");
    }
    
    x_ = initial_state;
}

void LinearStateSpace::setMatrices(const MatrixXd& A, const MatrixXd& B, 
                                    const MatrixXd& C, const MatrixXd& D) {
    if (!initialized_) {
        throw std::runtime_error("LinearStateSpace not initialized");
    }
    
    // 验证矩阵维度
    if (A.rows() != config_.state_dim || A.cols() != config_.state_dim) {
        throw std::invalid_argument("A matrix dimension mismatch");
    }
    if (B.rows() != config_.state_dim || B.cols() != config_.input_dim) {
        throw std::invalid_argument("B matrix dimension mismatch");
    }
    if (C.rows() != config_.output_dim || C.cols() != config_.state_dim) {
        throw std::invalid_argument("C matrix dimension mismatch");
    }
    if (D.rows() != config_.output_dim || D.cols() != config_.input_dim) {
        throw std::invalid_argument("D matrix dimension mismatch");
    }
    
    A_ = A;
    B_ = B;
    C_ = C;
    D_ = D;
}

void LinearStateSpace::setState(const VectorXd& state) {
    if (!initialized_) {
        throw std::runtime_error("LinearStateSpace not initialized");
    }
    if (state.size() != config_.state_dim) {
        throw std::invalid_argument("State dimension mismatch");
    }
    x_ = state;
}

VectorXd LinearStateSpace::update(const VectorXd& u, double dt) {
    if (!initialized_) {
        throw std::runtime_error("LinearStateSpace not initialized");
    }
    if (u.size() != config_.input_dim) {
        throw std::invalid_argument("Input dimension mismatch");
    }
    
    // 连续时间状态方程：dx/dt = Ax + Bu
    // 离散化（欧拉法）：x(k+1) = x(k) + (A*x(k) + B*u(k)) * dt
    x_ = x_ + (A_ * x_ + B_ * u) * dt;
    
    return x_;
}

VectorXd LinearStateSpace::updateRK4(const VectorXd& u, double dt) {
    if (!initialized_) {
        throw std::runtime_error("LinearStateSpace not initialized");
    }
    
    // RK4积分
    VectorXd k1 = A_ * x_ + B_ * u;
    VectorXd k2 = A_ * (x_ + dt/2 * k1) + B_ * u;
    VectorXd k3 = A_ * (x_ + dt/2 * k2) + B_ * u;
    VectorXd k4 = A_ * (x_ + dt * k3) + B_ * u;
    
    x_ = x_ + dt/6 * (k1 + 2*k2 + 2*k3 + k4);
    
    return x_;
}

VectorXd LinearStateSpace::getOutput(const VectorXd& u) const {
    if (!initialized_) {
        throw std::runtime_error("LinearStateSpace not initialized");
    }
    
    // 输出方程：y = Cx + Du
    return C_ * x_ + D_ * u;
}

VectorXd LinearStateSpace::getState() const {
    return x_;
}

MatrixXd LinearStateSpace::getA() const { return A_; }
MatrixXd LinearStateSpace::getB() const { return B_; }
MatrixXd LinearStateSpace::getC() const { return C_; }
MatrixXd LinearStateSpace::getD() const { return D_; }

std::vector<std::complex<double>> LinearStateSpace::getEigenvalues() const {
    EigenSolver<MatrixXd> solver(A_);
    auto eigenvalues = solver.eigenvalues();
    
    std::vector<std::complex<double>> result;
    for (int i = 0; i < eigenvalues.size(); ++i) {
        result.push_back(eigenvalues[i]);
    }
    return result;
}

bool LinearStateSpace::isStable() const {
    auto eigenvalues = getEigenvalues();
    for (const auto& ev : eigenvalues) {
        if (ev.real() >= 0) {
            return false;  // 存在非负实部的特征值
        }
    }
    return true;
}

MatrixXd LinearStateSpace::computeJacobian(const VectorXd& x_eq, const VectorXd& u_eq) {
    // 对于线性系统，雅可比矩阵就是A
    return A_;
}

MatrixXd LinearStateSpace::discretize(double dt) const {
    // 零阶保持离散化
    // Ad = e^(A*dt)
    // 简化版本：使用泰勒展开
    int n = config_.state_dim;
    MatrixXd Ad = MatrixXd::Identity(n, n);
    MatrixXd A_power = A_;
    double factorial = 1.0;
    
    for (int k = 1; k <= 20; ++k) {
        factorial *= k;
        Ad += A_power * std::pow(dt, k) / factorial;
        A_power *= A_;
    }
    
    return Ad;
}

// ============================================================================
// NonlinearStateSpace 实现
// ============================================================================

NonlinearStateSpace::NonlinearStateSpace()
    : initialized_(false)
    , config_()
    , x_()
    , state_func_(nullptr)
    , output_func_(nullptr)
{
}

NonlinearStateSpace::~NonlinearStateSpace() = default;

void NonlinearStateSpace::initialize(const StateSpaceConfig& config) {
    config.validate();
    config_ = config;
    x_ = VectorXd::Zero(config.state_dim);
    initialized_ = true;
}

void NonlinearStateSpace::setStateFunction(StateFunction f) {
    state_func_ = f;
}

void NonlinearStateSpace::setOutputFunction(OutputFunction f) {
    output_func_ = f;
}

void NonlinearStateSpace::setState(const VectorXd& state) {
    if (!initialized_) {
        throw std::runtime_error("NonlinearStateSpace not initialized");
    }
    if (state.size() != config_.state_dim) {
        throw std::invalid_argument("State dimension mismatch");
    }
    x_ = state;
}

VectorXd NonlinearStateSpace::update(const VectorXd& u, double dt) {
    if (!initialized_) {
        throw std::runtime_error("NonlinearStateSpace not initialized");
    }
    if (!state_func_) {
        throw std::runtime_error("State function not set");
    }
    
    // RK4积分
    VectorXd k1 = state_func_(x_, u);
    VectorXd k2 = state_func_(x_ + dt/2 * k1, u);
    VectorXd k3 = state_func_(x_ + dt/2 * k2, u);
    VectorXd k4 = state_func_(x_ + dt * k3, u);
    
    x_ = x_ + dt/6 * (k1 + 2*k2 + 2*k3 + k4);
    
    return x_;
}

VectorXd NonlinearStateSpace::getOutput(const VectorXd& u) const {
    if (!initialized_) {
        throw std::runtime_error("NonlinearStateSpace not initialized");
    }
    
    if (output_func_) {
        return output_func_(x_, u);
    } else {
        // 默认输出为状态本身
        return x_.head(config_.output_dim);
    }
}

VectorXd NonlinearStateSpace::getState() const {
    return x_;
}

MatrixXd NonlinearStateSpace::computeJacobian(
    const VectorXd& x_eq, 
    const VectorXd& u_eq, 
    double epsilon
) {
    int n = config_.state_dim;
    int m = config_.input_dim;
    
    if (!state_func_) {
        throw std::runtime_error("State function not set");
    }
    
    // 计算状态雅可比矩阵 A = df/dx
    MatrixXd A(n, n);
    VectorXd f0 = state_func_(x_eq, u_eq);
    
    for (int j = 0; j < n; ++j) {
        VectorXd x_pert = x_eq;
        x_pert(j) += epsilon;
        VectorXd f_pert = state_func_(x_pert, u_eq);
        A.col(j) = (f_pert - f0) / epsilon;
    }
    
    return A;
}

MatrixXd NonlinearStateSpace::computeInputJacobian(
    const VectorXd& x_eq, 
    const VectorXd& u_eq, 
    double epsilon
) {
    int n = config_.state_dim;
    int m = config_.input_dim;
    
    if (!state_func_) {
        throw std::runtime_error("State function not set");
    }
    
    // 计算输入雅可比矩阵 B = df/du
    MatrixXd B(n, m);
    VectorXd f0 = state_func_(x_eq, u_eq);
    
    for (int j = 0; j < m; ++j) {
        VectorXd u_pert = u_eq;
        u_pert(j) += epsilon;
        VectorXd f_pert = state_func_(x_eq, u_pert);
        B.col(j) = (f_pert - f0) / epsilon;
    }
    
    return B;
}

std::pair<VectorXd, VectorXd> NonlinearStateSpace::findEquilibrium(
    const VectorXd& x0, 
    const VectorXd& u_eq, 
    double tolerance, 
    int max_iterations
) {
    if (!state_func_) {
        throw std::runtime_error("State function not set");
    }
    
    VectorXd x = x0;
    
    for (int iter = 0; iter < max_iterations; ++iter) {
        // 计算dx/dt
        VectorXd dx = state_func_(x, u_eq);
        
        // 检查是否收敛
        if (dx.norm() < tolerance) {
            return {x, dx};
        }
        
        // 计算雅可比矩阵
        MatrixXd A = computeJacobian(x, u_eq);
        
        // 牛顿法更新：dx/dt = 0 => x_new = x - A^{-1} * dx/dt
        // 使用伪逆处理奇异矩阵
        VectorXd delta = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(-dx);
        x = x + delta;
    }
    
    // 未收敛，返回最后状态
    return {x, state_func_(x, u_eq)};
}

// ============================================================================
// VehicleStateSpace 实现
// ============================================================================

VehicleStateSpace::VehicleStateSpace()
    : dynamics_(nullptr)
    , linearized_()
{
}

VehicleStateSpace::VehicleStateSpace(VehicleDynamics* dynamics)
    : VehicleStateSpace()
{
    setDynamics(dynamics);
}

VehicleStateSpace::~VehicleStateSpace() = default;

void VehicleStateSpace::setDynamics(VehicleDynamics* dynamics) {
    if (!dynamics) {
        throw std::invalid_argument("VehicleDynamics pointer cannot be null");
    }
    dynamics_ = dynamics;
}

void VehicleStateSpace::initialize() {
    if (!dynamics_) {
        throw std::runtime_error("VehicleDynamics not set");
    }
    if (!dynamics_->isInitialized()) {
        throw std::runtime_error("VehicleDynamics not initialized");
    }
    
    // 初始化线性化模型（6状态 + 6辅助状态）
    linearized_.initialize(StateSpaceConfig{18, 6, 18});
}

VectorXd VehicleStateSpace::stateDerivative(const VectorXd& state, const VectorXd& input) {
    // 从向量恢复车辆状态
    VehicleState vs;
    vs.x = state(0); vs.y = state(1); vs.z = state(2);
    vs.roll = state(3); vs.pitch = state(4); vs.yaw = state(5);
    vs.vx = state(6); vs.vy = state(7); vs.vz = state(8);
    vs.omega_x = state(9); vs.omega_y = state(10); vs.omega_z = state(11);
    vs.ax = state(12); vs.ay = state(13); vs.az = state(14);
    vs.alpha_x = state(15); vs.alpha_y = state(16); vs.alpha_z = state(17);
    
    // 设置输入
    DynamicsInput din;
    for (int i = 0; i < 4; ++i) {
        din.drive_forces[i] = input(i);
        din.wheel_speeds[i] = input(i) / (dynamics_->getConfig().tire_radius * dynamics_->getConfig().mass);
    }
    din.steering_angle = input(4);
    din.road_grade_angle = input(5);
    
    dynamics_->reset(vs);
    dynamics_->setInput(din);
    
    // 计算导数
    VehicleStateDerivative deriv = dynamics_->computeDerivatives();
    
    // 转换为向量
    VectorXd dstate(18);
    dstate(0) = deriv.x; dstate(1) = deriv.y; dstate(2) = deriv.z;
    dstate(3) = deriv.roll; dstate(4) = deriv.pitch; dstate(5) = deriv.yaw;
    dstate(6) = deriv.vx; dstate(7) = deriv.vy; dstate(8) = deriv.vz;
    dstate(9) = deriv.omega_x; dstate(10) = deriv.omega_y; dstate(11) = deriv.omega_z;
    dstate(12) = deriv.ax; dstate(13) = deriv.ay; dstate(14) = deriv.az;
    dstate(15) = deriv.alpha_x; dstate(16) = deriv.alpha_y; dstate(17) = deriv.alpha_z;
    
    return dstate;
}

void VehicleStateSpace::linearize(const VectorXd& x_eq, const VectorXd& u_eq, double epsilon) {
    // 数值计算雅可比矩阵
    int n = 18;  // 状态维度
    int m = 6;   // 输入维度
    
    MatrixXd A(n, n);
    MatrixXd B(n, m);
    
    VectorXd f0 = stateDerivative(x_eq, u_eq);
    
    // 计算A = df/dx
    for (int j = 0; j < n; ++j) {
        VectorXd x_pert = x_eq;
        x_pert(j) += epsilon;
        VectorXd f_pert = stateDerivative(x_pert, u_eq);
        A.col(j) = (f_pert - f0) / epsilon;
    }
    
    // 计算B = df/du
    for (int j = 0; j < m; ++j) {
        VectorXd u_pert = u_eq;
        u_pert(j) += epsilon;
        VectorXd f_pert = stateDerivative(x_eq, u_pert);
        B.col(j) = (f_pert - f0) / epsilon;
    }
    
    // 设置线性化矩阵
    MatrixXd C = MatrixXd::Identity(n, n);
    MatrixXd D = MatrixXd::Zero(n, m);
    linearized_.setMatrices(A, B, C, D);
}

LinearStateSpace VehicleStateSpace::getLinearizedModel() const {
    return linearized_;
}

std::vector<std::complex<double>> VehicleStateSpace::computeEigenvalues() const {
    return linearized_.getEigenvalues();
}

bool VehicleStateSpace::isStable() const {
    return linearized_.isStable();
}

std::vector<double> VehicleStateSpace::computeDampingRatios() const {
    auto eigenvalues = computeEigenvalues();
    std::vector<double> damping_ratios;
    
    for (const auto& ev : eigenvalues) {
        // 阻尼比 = -real(ev) / |ev|
        double real = ev.real();
        double imag = ev.imag();
        double magnitude = std::sqrt(real*real + imag*imag);
        
        if (magnitude > 1e-10) {
            damping_ratios.push_back(-real / magnitude);
        } else {
            damping_ratios.push_back(0.0);
        }
    }
    
    return damping_ratios;
}

std::vector<double> VehicleStateSpace::computeNaturalFrequencies() const {
    auto eigenvalues = computeEigenvalues();
    std::vector<double> frequencies;
    
    for (const auto& ev : eigenvalues) {
        // 自然频率 = |ev| / (2*pi)
        double magnitude = std::abs(ev);
        frequencies.push_back(magnitude / (2 * M_PI));
    }
    
    return frequencies;
}

} // namespace dynamics
} // namespace ev_simulation
