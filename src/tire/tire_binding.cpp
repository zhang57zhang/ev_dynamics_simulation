/**
 * @file tire_binding.cpp
 * @brief 轮胎模型Python绑定
 * 
 * 包括：
 * - Pacejka: 魔术公式轮胎模型
 * - SlipCalculator: 滑移率计算器
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <Eigen/Dense>
#include <cmath>

namespace py = pybind11;

// ============================================================================
// 滑移率计算器（占位符）
// ============================================================================
class SlipCalculator {
public:
    SlipCalculator() = default;
    
    // 计算纵向滑移率
    double calculate_longitudinal_slip(double wheel_speed, double vehicle_speed) {
        if (std::abs(vehicle_speed) < 0.01) {
            return 0.0;
        }
        return (wheel_speed - vehicle_speed) / vehicle_speed;
    }
    
    // 计算侧偏角
    double calculate_slip_angle(double vy, double vx) {
        if (std::abs(vx) < 0.01) {
            return 0.0;
        }
        return std::atan2(vy, vx);
    }
    
private:
};

// ============================================================================
// Pacejka轮胎模型（占位符）
// ============================================================================
class PacejkaTire {
public:
    PacejkaTire(double B, double C, double D, double E)
        : B_(B), C_(C), D_(D), E_(E) {}
    
    // 魔术公式: y = D * sin(C * atan(B * x - E * (B * x - atan(B * x))))
    double calculate_force(double slip) {
        double Bx = B_ * slip;
        double term = Bx - E_ * (Bx - std::atan(Bx));
        return D_ * std::sin(C_ * std::atan(term));
    }
    
    // 计算纵向力
    double calculate_longitudinal_force(double kappa) {
        // kappa: 纵向滑移率 [-1, 1]
        return calculate_force(kappa);
    }
    
    // 计算侧向力
    double calculate_lateral_force(double alpha) {
        // alpha: 侧偏角 [rad]
        return calculate_force(alpha);
    }
    
private:
    double B_; // 刚度因子
    double C_; // 形状因子
    double D_; // 峰值因子
    double E_; // 曲率因子
};

// ============================================================================
// Python模块定义
// ============================================================================
PYBIND11_MODULE(tire, m) {
    m.doc() = "Tire module: Pacejka magic formula and slip calculator";
    
    // SlipCalculator类
    py::class_<SlipCalculator>(m, "SlipCalculator")
        .def(py::init<>())
        .def("calculate_longitudinal_slip", &SlipCalculator::calculate_longitudinal_slip,
             py::arg("wheel_speed"), py::arg("vehicle_speed"))
        .def("calculate_slip_angle", &SlipCalculator::calculate_slip_angle,
             py::arg("vy"), py::arg("vx"));
    
    // PacejkaTire类
    py::class_<PacejkaTire>(m, "PacejkaTire")
        .def(py::init<double, double, double, double>(),
             py::arg("B"), py::arg("C"), py::arg("D"), py::arg("E"))
        .def("calculate_force", &PacejkaTire::calculate_force, py::arg("slip"))
        .def("calculate_longitudinal_force", &PacejkaTire::calculate_longitudinal_force,
             py::arg("kappa"))
        .def("calculate_lateral_force", &PacejkaTire::calculate_lateral_force,
             py::arg("alpha"));
}
