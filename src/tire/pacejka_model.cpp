/**
 * @file pacejka_model.cpp
 * @brief Pacejka Magic Formula 6.2 轮胎模型实现
 * 
 * 实现完整的Pacejka MF6.2模型，包括：
 * - 纵向力 Fx = D * sin(C * arctan(B * kappa - E * (B * kappa - arctan(B * kappa))))
 * - 横向力 Fy = D * sin(C * arctan(B * alpha - E * (B * alpha - arctan(B * alpha))))
 * - 回正力矩 Mz = D * sin(C * arctan(B * alpha - E * (B * alpha - arctan(B * alpha))))
 * - 组合滑移
 * - 参数拟合
 * 
 * 参考文献：
 * - Pacejka, H.B. (2012). Tire and Vehicle Dynamics. 3rd Edition. Elsevier.
 * - TNO MF-Tire/MF-Swift 6.2 Manual
 * 
 * @author TireAgent
 * @date 2026-03-08
 */

#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <Eigen/Dense>
#include "pacejka_model.h"

namespace ev_simulation {
namespace tire {

using Eigen::VectorXd;
using Eigen::MatrixXd;

// ============================================================================
// PacejkaParameters 实现
// ============================================================================

void PacejkaParameters::validate() const {
    if (PCx <= 0 || PCy <= 0 || PCz <= 0) {
        throw std::invalid_argument("Shape factors (C) must be positive");
    }
    if (PDx <= 0 || PDy <= 0 || PDz <= 0) {
        throw std::invalid_argument("Peak factors (D) must be positive");
    }
    if (PKx <= 0 || PKy <= 0 || PKz <= 0) {
        throw std::invalid_argument("Stiffness values (K) must be positive");
    }
    if (Fz0 <= 0) {
        throw std::invalid_argument("Nominal load Fz0 must be positive");
    }
    if (tire_radius <= 0) {
        throw std::invalid_argument("Tire radius must be positive");
    }
}

PacejkaParameters PacejkaParameters::loadFromFile(const std::string& filename) {
    PacejkaParameters params;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open parameter file: " + filename);
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // 跳过注释和空行
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string key;
        double value;
        
        if (iss >> key >> value) {
            // 纵向力参数
            if (key == "PCx") params.PCx = value;
            else if (key == "PDx") params.PDx = value;
            else if (key == "PDx1") params.PDx1 = value;
            else if (key == "PDx2") params.PDx2 = value;
            else if (key == "PEx") params.PEx = value;
            else if (key == "PEx1") params.PEx1 = value;
            else if (key == "PEx2") params.PEx2 = value;
            else if (key == "PKx") params.PKx = value;
            else if (key == "PKx1") params.PKx1 = value;
            else if (key == "PKx2") params.PKx2 = value;
            else if (key == "PHx") params.PHx = value;
            else if (key == "PHx1") params.PHx1 = value;
            else if (key == "PVx") params.PVx = value;
            else if (key == "PVx1") params.PVx1 = value;
            
            // 横向力参数
            else if (key == "PCy") params.PCy = value;
            else if (key == "PDy") params.PDy = value;
            else if (key == "PDy1") params.PDy1 = value;
            else if (key == "PDy2") params.PDy2 = value;
            else if (key == "PDy3") params.PDy3 = value;
            else if (key == "PEy") params.PEy = value;
            else if (key == "PEy1") params.PEy1 = value;
            else if (key == "PEy2") params.PEy2 = value;
            else if (key == "PEy3") params.PEy3 = value;
            else if (key == "PEy4") params.PEy4 = value;
            else if (key == "PKy") params.PKy = value;
            else if (key == "PKy1") params.PKy1 = value;
            else if (key == "PKy2") params.PKy2 = value;
            else if (key == "PKy3") params.PKy3 = value;
            else if (key == "PHy") params.PHy = value;
            else if (key == "PHy1") params.PHy1 = value;
            else if (key == "PHy2") params.PHy2 = value;
            else if (key == "PVy") params.PVy = value;
            else if (key == "PVy1") params.PVy1 = value;
            else if (key == "PVy2") params.PVy2 = value;
            else if (key == "PVy3") params.PVy3 = value;
            else if (key == "PVy4") params.PVy4 = value;
            
            // 回正力矩参数
            else if (key == "PCz") params.PCz = value;
            else if (key == "PDz") params.PDz = value;
            else if (key == "PDz1") params.PDz1 = value;
            else if (key == "PDz2") params.PDz2 = value;
            else if (key == "PDz3") params.PDz3 = value;
            else if (key == "PDz4") params.PDz4 = value;
            else if (key == "PEz") params.PEz = value;
            else if (key == "PEz1") params.PEz1 = value;
            else if (key == "PEz2") params.PEz2 = value;
            else if (key == "PEz3") params.PEz3 = value;
            else if (key == "PEz4") params.PEz4 = value;
            else if (key == "PKz") params.PKz = value;
            else if (key == "PKz1") params.PKz1 = value;
            else if (key == "PKz2") params.PKz2 = value;
            else if (key == "PKz3") params.PKz3 = value;
            else if (key == "PHz") params.PHz = value;
            else if (key == "PHz1") params.PHz1 = value;
            else if (key == "PHz2") params.PHz2 = value;
            else if (key == "PHz3") params.PHz3 = value;
            else if (key == "PVz") params.PVz = value;
            else if (key == "PVz1") params.PVz1 = value;
            else if (key == "PVz2") params.PVz2 = value;
            else if (key == "PVz3") params.PVz3 = value;
            
            // 组合滑移参数
            else if (key == "rBx1") params.rBx1 = value;
            else if (key == "rBx2") params.rBx2 = value;
            else if (key == "rCx") params.rCx = value;
            else if (key == "rEx") params.rEx = value;
            else if (key == "rHx") params.rHx = value;
            else if (key == "rBy1") params.rBy1 = value;
            else if (key == "rBy2") params.rBy2 = value;
            else if (key == "rBy3") params.rBy3 = value;
            else if (key == "rCy") params.rCy = value;
            else if (key == "rEy") params.rEy = value;
            else if (key == "rHy") params.rHy = value;
            
            // 物理参数
            else if (key == "Fz0") params.Fz0 = value;
            else if (key == "tire_radius") params.tire_radius = value;
            else if (key == "tire_width") params.tire_width = value;
        }
    }
    
    params.validate();
    return params;
}

void PacejkaParameters::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create parameter file: " + filename);
    }
    
    file << "# Pacejka MF6.2 Tire Parameters\n";
    file << "# Generated by ev_dynamics_simulation\n\n";
    
    file << "# === Longitudinal Force Fx ===\n";
    file << "PCx " << PCx << "\n";
    file << "PDx " << PDx << "\n";
    file << "PDx1 " << PDx1 << "\n";
    file << "PDx2 " << PDx2 << "\n";
    file << "PEx " << PEx << "\n";
    file << "PEx1 " << PEx1 << "\n";
    file << "PEx2 " << PEx2 << "\n";
    file << "PKx " << PKx << "\n";
    file << "PKx1 " << PKx1 << "\n";
    file << "PKx2 " << PKx2 << "\n";
    file << "PHx " << PHx << "\n";
    file << "PHx1 " << PHx1 << "\n";
    file << "PVx " << PVx << "\n";
    file << "PVx1 " << PVx1 << "\n\n";
    
    file << "# === Lateral Force Fy ===\n";
    file << "PCy " << PCy << "\n";
    file << "PDy " << PDy << "\n";
    file << "PDy1 " << PDy1 << "\n";
    file << "PDy2 " << PDy2 << "\n";
    file << "PDy3 " << PDy3 << "\n";
    file << "PEy " << PEy << "\n";
    file << "PEy1 " << PEy1 << "\n";
    file << "PEy2 " << PEy2 << "\n";
    file << "PEy3 " << PEy3 << "\n";
    file << "PEy4 " << PEy4 << "\n";
    file << "PKy " << PKy << "\n";
    file << "PKy1 " << PKy1 << "\n";
    file << "PKy2 " << PKy2 << "\n";
    file << "PKy3 " << PKy3 << "\n";
    file << "PHy " << PHy << "\n";
    file << "PHy1 " << PHy1 << "\n";
    file << "PHy2 " << PHy2 << "\n";
    file << "PVy " << PVy << "\n";
    file << "PVy1 " << PVy1 << "\n";
    file << "PVy2 " << PVy2 << "\n";
    file << "PVy3 " << PVy3 << "\n";
    file << "PVy4 " << PVy4 << "\n\n";
    
    file << "# === Aligning Moment Mz ===\n";
    file << "PCz " << PCz << "\n";
    file << "PDz " << PDz << "\n";
    file << "PDz1 " << PDz1 << "\n";
    file << "PDz2 " << PDz2 << "\n";
    file << "PDz3 " << PDz3 << "\n";
    file << "PDz4 " << PDz4 << "\n";
    file << "PEz " << PEz << "\n";
    file << "PEz1 " << PEz1 << "\n";
    file << "PEz2 " << PEz2 << "\n";
    file << "PEz3 " << PEz3 << "\n";
    file << "PEz4 " << PEz4 << "\n";
    file << "PKz " << PKz << "\n";
    file << "PKz1 " << PKz1 << "\n";
    file << "PKz2 " << PKz2 << "\n";
    file << "PKz3 " << PKz3 << "\n";
    file << "PHz " << PHz << "\n";
    file << "PHz1 " << PHz1 << "\n";
    file << "PHz2 " << PHz2 << "\n";
    file << "PHz3 " << PHz3 << "\n";
    file << "PVz " << PVz << "\n";
    file << "PVz1 " << PVz1 << "\n";
    file << "PVz2 " << PVz2 << "\n";
    file << "PVz3 " << PVz3 << "\n\n";
    
    file << "# === Combined Slip ===\n";
    file << "rBx1 " << rBx1 << "\n";
    file << "rBx2 " << rBx2 << "\n";
    file << "rCx " << rCx << "\n";
    file << "rEx " << rEx << "\n";
    file << "rHx " << rHx << "\n";
    file << "rBy1 " << rBy1 << "\n";
    file << "rBy2 " << rBy2 << "\n";
    file << "rBy3 " << rBy3 << "\n";
    file << "rCy " << rCy << "\n";
    file << "rEy " << rEy << "\n";
    file << "rHy " << rHy << "\n\n";
    
    file << "# === Physical Parameters ===\n";
    file << "Fz0 " << Fz0 << "\n";
    file << "tire_radius " << tire_radius << "\n";
    file << "tire_width " << tire_width << "\n";
}

// ============================================================================
// PacejkaModel 实现
// ============================================================================

PacejkaModel::PacejkaModel()
    : params_()
    , initialized_(false)
{
}

PacejkaModel::PacejkaModel(const PacejkaParameters& params)
    : params_(params)
    , initialized_(true)
{
    params_.validate();
}

PacejkaModel::~PacejkaModel() = default;

void PacejkaModel::setParameters(const PacejkaParameters& params) {
    params.validate();
    params_ = params;
    initialized_ = true;
}

TireForces PacejkaModel::computeForces(const TireState& state) {
    if (!initialized_) {
        throw std::runtime_error("PacejkaModel not initialized");
    }
    
    TireForces forces;
    
    // 计算组合滑移力
    forces.Fx = computeFxCombined(state.kappa, state.alpha, state.Fz);
    forces.Fy = computeFyCombined(state.kappa, state.alpha, state.Fz, state.gamma);
    forces.Fz = state.Fz;  // 垂向力作为输入
    forces.Mz = computeMz(state.alpha, state.Fz, state.gamma);
    
    // 翻转力矩和滚动阻力矩（简化模型）
    forces.Mx = forces.Fy * params_.tire_radius * 0.1;  // 近似
    forces.My = forces.Fx * params_.tire_radius * 0.01;  // 滚动阻力近似
    
    return forces;
}

double PacejkaModel::computeFx(double kappa, double Fz) {
    if (!initialized_) {
        throw std::runtime_error("PacejkaModel not initialized");
    }
    
    // 归一化载荷
    double dfz = normalizeLoad(Fz);
    
    // 计算参数
    auto [C, D, E, K, H, V] = computeFxParams(dfz);
    
    // 魔术公式
    double x = kappa + H;
    return magicFormula(x, C, D, E, K, 0.0, V);
}

double PacejkaModel::computeFy(double alpha, double Fz, double gamma) {
    if (!initialized_) {
        throw std::runtime_error("PacejkaModel not initialized");
    }
    
    // 归一化载荷
    double dfz = normalizeLoad(Fz);
    
    // 计算参数
    auto [C, D, E, K, H, V] = computeFyParams(dfz, gamma);
    
    // 魔术公式
    double x = alpha + H;
    return magicFormula(x, C, D, E, K, 0.0, V);
}

double PacejkaModel::computeMz(double alpha, double Fz, double gamma) {
    if (!initialized_) {
        throw std::runtime_error("PacejkaModel not initialized");
    }
    
    // 归一化载荷
    double dfz = normalizeLoad(Fz);
    
    // 计算参数
    auto [C, D, E, K, H, V] = computeMzParams(dfz, gamma);
    
    // 魔术公式
    double x = alpha + H;
    return magicFormula(x, C, D, E, K, 0.0, V);
}

double PacejkaModel::computeFxCombined(double kappa, double alpha, double Fz) {
    // 纯纵向滑移力
    double Fx0 = computeFx(kappa, Fz);
    
    // 计算组合滑移权重因子
    double Gxa = computeCombinationWeight(kappa, alpha, "Fx");
    
    // 组合滑移力
    return Fx0 * Gxa;
}

double PacejkaModel::computeFyCombined(double kappa, double alpha, double Fz, double gamma) {
    // 纯侧偏力
    double Fy0 = computeFy(alpha, Fz, gamma);
    
    // 计算组合滑移权重因子
    double Gyk = computeCombinationWeight(kappa, alpha, "Fy");
    
    // 组合滑移力
    return Fy0 * Gyk;
}

VectorXd PacejkaModel::fitParameters(const VectorXd& slip_data, 
                                    const VectorXd& force_data,
                                    const std::string& force_type) {
    if (slip_data.size() != force_data.size()) {
        throw std::invalid_argument("Slip and force data must have same size");
    }
    
    if (slip_data.size() < 10) {
        throw std::invalid_argument("Need at least 10 data points for fitting");
    }
    
    // 简化的参数拟合（最小二乘法）
    // 这里实现一个基本的拟合算法
    
    int n = slip_data.size();
    int num_params = 6;  // C, D, E, K, H, V
    
    // 构建雅可比矩阵
    MatrixXd J(n, num_params);
    VectorXd residuals = force_data;
    
    // 初始猜测
    VectorXd params_vec(num_params);
    params_vec << 1.6, 1.0, -1.0, 20.0, 0.0, 0.0;
    
    // 迭代优化（高斯-牛顿法）
    const int max_iter = 50;
    const double tolerance = 1e-6;
    
    for (int iter = 0; iter < max_iter; ++iter) {
        // 计算雅可比矩阵
        for (int i = 0; i < n; ++i) {
            double x = slip_data(i);
            double C = params_vec(0);
            double D = params_vec(1);
            double E = params_vec(2);
            double K = params_vec(3);
            double H = params_vec(4);
            double V = params_vec(5);
            
            // 魔术公式值
            double Bx = K / (C * D + 1e-10);
            double y = D * std::sin(C * std::atan(Bx * (x + H) - E * (Bx * (x + H) - std::atan(Bx * (x + H))))) + V;
            
            // 数值微分计算雅可比
            const double delta = 1e-6;
            for (int j = 0; j < num_params; ++j) {
                VectorXd params_perturbed = params_vec;
                params_perturbed(j) += delta;
                
                double C_p = params_perturbed(0);
                double D_p = params_perturbed(1);
                double E_p = params_perturbed(2);
                double K_p = params_perturbed(3);
                double H_p = params_perturbed(4);
                double V_p = params_perturbed(5);
                
                double Bx_p = K_p / (C_p * D_p + 1e-10);
                double y_p = D_p * std::sin(C_p * std::atan(Bx_p * (x + H_p) - E_p * (Bx_p * (x + H_p) - std::atan(Bx_p * (x + H_p))))) + V_p;
                
                J(i, j) = (y_p - y) / delta;
            }
            
            residuals(i) = force_data(i) - y;
        }
        
        // 最小二乘求解
        VectorXd delta_params = (J.transpose() * J).ldlt().solve(J.transpose() * residuals);
        
        // 更新参数
        params_vec += delta_params;
        
        // 检查收敛
        if (delta_params.norm() < tolerance) {
            break;
        }
    }
    
    // 更新参数
    if (force_type == "Fx") {
        params_.PCx = params_vec(0);
        params_.PDx = params_vec(1);
        params_.PEx = params_vec(2);
        params_.PKx = params_vec(3);
        params_.PHx = params_vec(4);
        params_.PVx = params_vec(5);
    } else if (force_type == "Fy") {
        params_.PCy = params_vec(0);
        params_.PDy = params_vec(1);
        params_.PEy = params_vec(2);
        params_.PKy = params_vec(3);
        params_.PHy = params_vec(4);
        params_.PVy = params_vec(5);
    } else if (force_type == "Mz") {
        params_.PCz = params_vec(0);
        params_.PDz = params_vec(1);
        params_.PEz = params_vec(2);
        params_.PKz = params_vec(3);
        params_.PHz = params_vec(4);
        params_.PVz = params_vec(5);
    }
    
    return params_vec;
}

double PacejkaModel::validateModel(const std::vector<TireState>& test_states,
                                  const std::vector<TireForces>& test_forces) {
    if (test_states.size() != test_forces.size()) {
        throw std::invalid_argument("Test states and forces must have same size");
    }
    
    double sum_squared_error = 0.0;
    int count = 0;
    
    for (size_t i = 0; i < test_states.size(); ++i) {
        TireForces predicted = computeForces(test_states[i]);
        
        double fx_error = predicted.Fx - test_forces[i].Fx;
        double fy_error = predicted.Fy - test_forces[i].Fy;
        double mz_error = predicted.Mz - test_forces[i].Mz;
        
        sum_squared_error += fx_error * fx_error + fy_error * fy_error + mz_error * mz_error;
        count += 3;
    }
    
    // RMSE
    return std::sqrt(sum_squared_error / count);
}

std::vector<std::pair<double, double>> PacejkaModel::getCharacteristicCurve(
    double Fz,
    const std::string& curve_type,
    double range_min,
    double range_max,
    int num_points
) {
    std::vector<std::pair<double, double>> curve;
    curve.reserve(num_points);
    
    double step = (range_max - range_min) / (num_points - 1);
    
    for (int i = 0; i < num_points; ++i) {
        double x = range_min + i * step;
        double y = 0.0;
        
        if (curve_type == "Fx-kappa") {
            y = computeFx(x, Fz);
        } else if (curve_type == "Fy-alpha") {
            y = computeFy(x, Fz);
        } else if (curve_type == "Mz-alpha") {
            y = computeMz(x, Fz);
        } else {
            throw std::invalid_argument("Unknown curve type: " + curve_type);
        }
        
        curve.push_back({x, y});
    }
    
    return curve;
}

// ============================================================================
// 私有方法
// ============================================================================

double PacejkaModel::normalizeLoad(double Fz) const {
    // 归一化载荷：dfz = (Fz - Fz0) / Fz0
    return (Fz - params_.Fz0) / params_.Fz0;
}

std::tuple<double, double, double, double, double, double> 
PacejkaModel::computeFxParams(double dfz) {
    // 形状因子 C
    double C = params_.PCx;
    
    // 峰值因子 D
    double D = params_.PDx * (params_.PDx1 * dfz + params_.PDx2) * Fz0;
    
    // 曲率因子 E
    double E = params_.PEx * (params_.PEx1 * dfz + params_.PEx2);
    
    // 纵向滑移刚度 K
    double K = params_.PKx * (params_.PKx1 * dfz + params_.PKx2) * 1000;  // kN -> N
    
    // 水平偏移 H
    double H = params_.PHx * (params_.PHx1 * dfz);
    
    // 垂直偏移 V
    double V = params_.PVx * (params_.PVx1 * dfz);
    
    return {C, D, E, K, H, V};
}

std::tuple<double, double, double, double, double, double> 
PacejkaModel::computeFyParams(double dfz, double gamma) {
    // 形状因子 C
    double C = params_.PCy;
    
    // 峰值因子 D
    double D = params_.PDy * (params_.PDy1 * dfz + params_.PDy2) * 
               (1 - params_.PDy3 * gamma * gamma) * params_.Fz0;
    
    // 曲率因子 E
    double E = params_.PEy * (params_.PEy1 * dfz + params_.PEy2) * 
               (1 - params_.PEy3 * gamma * gamma + params_.PEy4 * gamma);
    
    // 侧偏刚度 K
    double K = params_.PKy * (params_.PKy1 * dfz + params_.PKy2) * 
               (1 - params_.PKy3 * gamma * gamma) * 1000;  // kN -> N
    
    // 水平偏移 H
    double H = params_.PHy * (params_.PHy1 * dfz + params_.PHy2 * gamma);
    
    // 垂直偏移 V
    double V = params_.PVy * (params_.PVy1 * dfz + params_.PVy2 * gamma + 
                              params_.PVy3 * gamma * gamma + params_.PVy4 * gamma * dfz);
    
    return {C, D, E, K, H, V};
}

std::tuple<double, double, double, double, double, double> 
PacejkaModel::computeMzParams(double dfz, double gamma) {
    // 形状因子 C
    double C = params_.PCz;
    
    // 峰值因子 D
    double D = params_.PDz * (params_.PDz1 * dfz + params_.PDz2) * 
               (1 + params_.PDz3 * gamma + params_.PDz4 * gamma * gamma);
    
    // 曲率因子 E
    double E = params_.PEz * (params_.PEz1 * dfz + params_.PEz2) * 
               (1 + params_.PEz3 * gamma + params_.PEz4 * gamma * gamma);
    
    // 回正刚度 K
    double K = params_.PKz * (params_.PKz1 * dfz + params_.PKz2) * 
               (1 + params_.PKz3 * gamma);
    
    // 水平偏移 H
    double H = params_.PHz * (params_.PHz1 * dfz + params_.PHz2 * gamma + params_.PHz3 * gamma * gamma);
    
    // 垂直偏移 V
    double V = params_.PVz * (params_.PVz1 * dfz + params_.PVz2 * gamma + params_.PVz3 * gamma * gamma);
    
    return {C, D, E, K, H, V};
}

double PacejkaModel::magicFormula(double x, double C, double D, double E, 
                                 double K, double H, double V) const {
    // Pacejka魔术公式：
    // y = D * sin(C * arctan(B * x - E * (B * x - arctan(B * x)))) + V
    // 其中 B = K / (C * D)
    
    double B = K / (C * D + 1e-10);  // 防止除零
    double Bx = B * (x + H);
    
    // 魔术公式核心
    double y = D * std::sin(C * std::atan(Bx - E * (Bx - std::atan(Bx)))) + V;
    
    return y;
}

double PacejkaModel::computeCombinationWeight(double kappa, double alpha, 
                                              const std::string& type) {
    // 组合滑移权重因子 G
    // G = cos(C * arctan(B * sqrt(alpha² / (1 + alpha²)) - E * ...))
    
    double B = 0.0;
    double C = 0.0;
    double E = 0.0;
    double H = 0.0;
    
    if (type == "Fx") {
        B = params_.rBx1 * std::sqrt(alpha * alpha) + params_.rBx2;
        C = params_.rCx;
        E = params_.rEx;
        H = params_.rHx;
    } else if (type == "Fy") {
        B = params_.rBy1 * std::sqrt(kappa * kappa) + params_.rBy2;
        C = params_.rCy;
        E = params_.rEy;
        H = params_.rHy;
    }
    
    // 计算权重因子
    double s = (type == "Fx") ? std::abs(alpha) : std::abs(kappa);
    double G = std::cos(C * std::atan(B * s - E * (B * s - std::atan(B * s)) + H));
    
    return G;
}

} // namespace tire
} // namespace ev_simulation
