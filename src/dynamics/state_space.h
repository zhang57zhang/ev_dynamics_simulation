/**
 * @file state_space.h
 * @brief 状态空间模型头文件
 * 
 * @author BackendAgent #3
 * @date 2026-03-06
 */

#pragma once

#include <functional>
#include <vector>
#include <complex>
#include <Eigen/Dense>
#include "vehicle_dynamics.h"

namespace ev_simulation {
namespace dynamics {

using Eigen::VectorXd;
using Eigen::MatrixXd;

/**
 * @brief 状态空间配置
 */
struct StateSpaceConfig {
    int state_dim{0};       ///< 状态维度
    int input_dim{0};       ///< 输入维度
    int output_dim{0};      ///< 输出维度
    
    void validate() const;
};

/**
 * @brief 状态函数类型：dx/dt = f(x, u)
 */
using StateFunction = std::function<VectorXd(const VectorXd&, const VectorXd&)>;

/**
 * @brief 输出函数类型：y = h(x, u)
 */
using OutputFunction = std::function<VectorXd(const VectorXd&, const VectorXd&)>;

/**
 * @brief 线性状态空间模型
 * 
 * 连续时间系统：
 * dx/dt = A*x + B*u
 * y = C*x + D*u
 */
class LinearStateSpace {
public:
    LinearStateSpace();
    LinearStateSpace(int state_dim, int input_dim, int output_dim);
    ~LinearStateSpace();
    
    // === 初始化 ===
    
    /**
     * @brief 初始化状态空间模型
     * @param config 配置参数
     */
    void initialize(const StateSpaceConfig& config);
    
    /**
     * @brief 重置状态
     * @param initial_state 初始状态
     */
    void reset(const VectorXd& initial_state);
    
    /**
     * @brief 设置系统矩阵
     * @param A 状态矩阵
     * @param B 输入矩阵
     * @param C 输出矩阵
     * @param D 直通矩阵
     */
    void setMatrices(const MatrixXd& A, const MatrixXd& B, 
                     const MatrixXd& C, const MatrixXd& D);
    
    /**
     * @brief 设置当前状态
     * @param state 状态向量
     */
    void setState(const VectorXd& state);
    
    // === 更新 ===
    
    /**
     * @brief 更新状态（欧拉积分）
     * @param u 输入向量
     * @param dt 时间步长 (s)
     * @return 新状态
     */
    VectorXd update(const VectorXd& u, double dt);
    
    /**
     * @brief 更新状态（RK4积分）
     * @param u 输入向量
     * @param dt 时间步长 (s)
     * @return 新状态
     */
    VectorXd updateRK4(const VectorXd& u, double dt);
    
    // === 输出 ===
    
    /**
     * @brief 计算输出
     * @param u 输入向量
     * @return 输出向量
     */
    VectorXd getOutput(const VectorXd& u) const;
    
    // === 查询 ===
    
    VectorXd getState() const;
    MatrixXd getA() const;
    MatrixXd getB() const;
    MatrixXd getC() const;
    MatrixXd getD() const;
    
    /**
     * @brief 获取特征值
     * @return 特征值列表
     */
    std::vector<std::complex<double>> getEigenvalues() const;
    
    /**
     * @brief 检查系统稳定性
     * @return true: 稳定
     */
    bool isStable() const;
    
    /**
     * @brief 计算雅可比矩阵
     * @param x_eq 平衡点状态
     * @param u_eq 平衡点输入
     * @return 雅可比矩阵（对于线性系统就是A）
     */
    MatrixXd computeJacobian(const VectorXd& x_eq, const VectorXd& u_eq);
    
    /**
     * @brief 离散化系统
     * @param dt 采样时间 (s)
     * @return 离散状态矩阵 Ad
     */
    MatrixXd discretize(double dt) const;
    
    bool isInitialized() const { return initialized_; }
    
private:
    bool initialized_;
    StateSpaceConfig config_;
    VectorXd x_;
    MatrixXd A_, B_, C_, D_;
};

/**
 * @brief 非线性状态空间模型
 * 
 * dx/dt = f(x, u)
 * y = h(x, u)
 */
class NonlinearStateSpace {
public:
    NonlinearStateSpace();
    ~NonlinearStateSpace();
    
    // === 初始化 ===
    
    void initialize(const StateSpaceConfig& config);
    
    /**
     * @brief 设置状态函数
     * @param f 状态函数 dx/dt = f(x, u)
     */
    void setStateFunction(StateFunction f);
    
    /**
     * @brief 设置输出函数
     * @param h 输出函数 y = h(x, u)
     */
    void setOutputFunction(OutputFunction h);
    
    void setState(const VectorXd& state);
    
    // === 更新 ===
    
    /**
     * @brief 更新状态（RK4积分）
     * @param u 输入向量
     * @param dt 时间步长 (s)
     * @return 新状态
     */
    VectorXd update(const VectorXd& u, double dt);
    
    // === 输出 ===
    
    VectorXd getOutput(const VectorXd& u) const;
    VectorXd getState() const;
    
    // === 线性化 ===
    
    /**
     * @brief 计算状态雅可比矩阵
     * @param x_eq 平衡点状态
     * @param u_eq 平衡点输入
     * @param epsilon 扰动大小
     * @return 雅可比矩阵 A = df/dx
     */
    MatrixXd computeJacobian(const VectorXd& x_eq, const VectorXd& u_eq, 
                             double epsilon = 1e-6);
    
    /**
     * @brief 计算输入雅可比矩阵
     * @param x_eq 平衡点状态
     * @param u_eq 平衡点输入
     * @param epsilon 扰动大小
     * @return 雅可比矩阵 B = df/du
     */
    MatrixXd computeInputJacobian(const VectorXd& x_eq, const VectorXd& u_eq, 
                                   double epsilon = 1e-6);
    
    /**
     * @brief 寻找平衡点
     * @param x0 初始猜测
     * @param u_eq 输入
     * @param tolerance 收敛容限
     * @param max_iterations 最大迭代次数
     * @return (平衡点状态, 残差)
     */
    std::pair<VectorXd, VectorXd> findEquilibrium(
        const VectorXd& x0, 
        const VectorXd& u_eq, 
        double tolerance = 1e-6, 
        int max_iterations = 100
    );
    
    bool isInitialized() const { return initialized_; }
    
private:
    bool initialized_;
    StateSpaceConfig config_;
    VectorXd x_;
    StateFunction state_func_;
    OutputFunction output_func_;
};

/**
 * @brief 车辆动力学状态空间模型
 * 
 * 封装车辆动力学模型为状态空间形式
 */
class VehicleStateSpace {
public:
    VehicleStateSpace();
    explicit VehicleStateSpace(VehicleDynamics* dynamics);
    ~VehicleStateSpace();
    
    /**
     * @brief 设置车辆动力学模型
     * @param dynamics 动力学模型指针
     */
    void setDynamics(VehicleDynamics* dynamics);
    
    /**
     * @brief 初始化状态空间模型
     */
    void initialize();
    
    /**
     * @brief 计算状态导数
     * @param state 状态向量（18维）
     * @param input 输入向量（6维）
     * @return 状态导数
     */
    VectorXd stateDerivative(const VectorXd& state, const VectorXd& input);
    
    /**
     * @brief 在平衡点线性化
     * @param x_eq 平衡点状态
     * @param u_eq 平衡点输入
     * @param epsilon 扰动大小
     */
    void linearize(const VectorXd& x_eq, const VectorXd& u_eq, double epsilon = 1e-6);
    
    /**
     * @brief 获取线性化模型
     * @return 线性状态空间模型
     */
    LinearStateSpace getLinearizedModel() const;
    
    /**
     * @brief 计算特征值
     * @return 特征值列表
     */
    std::vector<std::complex<double>> computeEigenvalues() const;
    
    /**
     * @brief 检查稳定性
     * @return true: 稳定
     */
    bool isStable() const;
    
    /**
     * @brief 计算阻尼比
     * @return 阻尼比列表
     */
    std::vector<double> computeDampingRatios() const;
    
    /**
     * @brief 计算自然频率
     * @return 自然频率列表 (Hz)
     */
    std::vector<double> computeNaturalFrequencies() const;
    
private:
    VehicleDynamics* dynamics_;
    LinearStateSpace linearized_;
};

} // namespace dynamics
} // namespace ev_simulation
