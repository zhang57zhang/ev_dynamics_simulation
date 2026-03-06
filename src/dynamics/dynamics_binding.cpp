/**
 * @file dynamics_binding.cpp
 * @brief 车辆动力学Python绑定
 * 
 * 包括：
 * - VehicleDynamics: 车辆动力学模型
 * - RK4Integrator: 四阶龙格-库塔积分器
 * - StateSpace: 状态空间表示
 * 
 * @author BackendAgent #3
 * @date 2026-03-06
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include <Eigen/Dense>
#include <functional>

#include "vehicle_dynamics.h"
#include "rk4_integrator.h"
#include "state_space.h"

namespace py = pybind11;
using namespace ev_simulation::dynamics;

// ============================================================================
// Python模块定义
// ============================================================================
PYBIND11_MODULE(dynamics, m) {
    m.doc() = "Dynamics module: vehicle dynamics, RK4 integrator, and state space models";
    
    // ========================================================================
    // 数据结构
    // ========================================================================
    
    // VehicleConfig
    py::class_<VehicleConfig>(m, "VehicleConfig")
        .def(py::init<>())
        .def_readwrite("mass", &VehicleConfig::mass)
        .def_readwrite("inertia_xx", &VehicleConfig::inertia_xx)
        .def_readwrite("inertia_yy", &VehicleConfig::inertia_yy)
        .def_readwrite("inertia_zz", &VehicleConfig::inertia_zz)
        .def_readwrite("wheelbase", &VehicleConfig::wheelbase)
        .def_readwrite("track_width_front", &VehicleConfig::track_width_front)
        .def_readwrite("track_width_rear", &VehicleConfig::track_width_rear)
        .def_readwrite("cg_height", &VehicleConfig::cg_height)
        .def_readwrite("cg_to_front_axle", &VehicleConfig::cg_to_front_axle)
        .def_readwrite("cg_to_rear_axle", &VehicleConfig::cg_to_rear_axle)
        .def_readwrite("drag_coefficient", &VehicleConfig::drag_coefficient)
        .def_readwrite("frontal_area", &VehicleConfig::frontal_area)
        .def_readwrite("lift_coefficient", &VehicleConfig::lift_coefficient)
        .def_readwrite("air_density", &VehicleConfig::air_density)
        .def_readwrite("tire_radius", &VehicleConfig::tire_radius)
        .def_readwrite("wheel_inertia", &VehicleConfig::wheel_inertia)
        .def_readwrite("cornering_stiffness_front", &VehicleConfig::cornering_stiffness_front)
        .def_readwrite("cornering_stiffness_rear", &VehicleConfig::cornering_stiffness_rear)
        .def_readwrite("longitudinal_stiffness", &VehicleConfig::longitudinal_stiffness)
        .def_readwrite("g", &VehicleConfig::g)
        .def("validate", &VehicleConfig::validate);
    
    // TireForce
    py::class_<TireForce>(m, "TireForce")
        .def(py::init<>())
        .def_readwrite("Fx", &TireForce::Fx)
        .def_readwrite("Fy", &TireForce::Fy)
        .def_readwrite("Fz", &TireForce::Fz)
        .def_readwrite("Mz", &TireForce::Mz);
    
    // DynamicsInput
    py::class_<DynamicsInput>(m, "DynamicsInput")
        .def(py::init<>())
        .def_readwrite("drive_forces", &DynamicsInput::drive_forces)
        .def_readwrite("brake_torques", &DynamicsInput::brake_torques)
        .def_readwrite("steering_angle", &DynamicsInput::steering_angle)
        .def_readwrite("tire_forces", &DynamicsInput::tire_forces)
        .def_readwrite("road_grade_angle", &DynamicsInput::road_grade_angle)
        .def_readwrite("road_friction", &DynamicsInput::road_friction)
        .def_readwrite("wheel_speeds", &DynamicsInput::wheel_speeds);
    
    // VehicleState
    py::class_<VehicleState>(m, "VehicleState")
        .def(py::init<>())
        .def_readwrite("x", &VehicleState::x)
        .def_readwrite("y", &VehicleState::y)
        .def_readwrite("z", &VehicleState::z)
        .def_readwrite("roll", &VehicleState::roll)
        .def_readwrite("pitch", &VehicleState::pitch)
        .def_readwrite("yaw", &VehicleState::yaw)
        .def_readwrite("vx", &VehicleState::vx)
        .def_readwrite("vy", &VehicleState::vy)
        .def_readwrite("vz", &VehicleState::vz)
        .def_readwrite("omega_x", &VehicleState::omega_x)
        .def_readwrite("omega_y", &VehicleState::omega_y)
        .def_readwrite("omega_z", &VehicleState::omega_z)
        .def_readwrite("ax", &VehicleState::ax)
        .def_readwrite("ay", &VehicleState::ay)
        .def_readwrite("az", &VehicleState::az)
        .def_readwrite("alpha_x", &VehicleState::alpha_x)
        .def_readwrite("alpha_y", &VehicleState::alpha_y)
        .def_readwrite("alpha_z", &VehicleState::alpha_z)
        .def_readwrite("speed", &VehicleState::speed)
        .def_readwrite("speed_kmh", &VehicleState::speed_kmh)
        .def_readwrite("lateral_acceleration", &VehicleState::lateral_acceleration)
        .def_readwrite("longitudinal_acceleration", &VehicleState::longitudinal_acceleration)
        .def("to_dict", [](const VehicleState& s) {
            py::dict d;
            d["x"] = s.x; d["y"] = s.y; d["z"] = s.z;
            d["roll"] = s.roll; d["pitch"] = s.pitch; d["yaw"] = s.yaw;
            d["vx"] = s.vx; d["vy"] = s.vy; d["vz"] = s.vz;
            d["omega_x"] = s.omega_x; d["omega_y"] = s.omega_y; d["omega_z"] = s.omega_z;
            d["ax"] = s.ax; d["ay"] = s.ay; d["az"] = s.az;
            d["speed"] = s.speed; d["speed_kmh"] = s.speed_kmh;
            return d;
        });
    
    // ========================================================================
    // VehicleDynamics
    // ========================================================================
    
    py::class_<VehicleDynamics>(m, "VehicleDynamics")
        .def(py::init<>())
        .def("initialize", &VehicleDynamics::initialize, py::arg("config"))
        .def("reset", (void (VehicleDynamics::*)(const VehicleState&)) &VehicleDynamics::reset, 
             py::arg("initial_state") = VehicleState{})
        .def("set_input", &VehicleDynamics::setInput, py::arg("input"))
        .def("compute_derivatives", &VehicleDynamics::computeDerivatives)
        .def("update", &VehicleDynamics::update, py::arg("dt"))
        .def("get_state", &VehicleDynamics::getState, py::return_value_policy::reference_internal)
        .def("get_config", &VehicleDynamics::getConfig, py::return_value_policy::reference_internal)
        .def("get_wheel_loads", &VehicleDynamics::getWheelLoads)
        .def("get_body_acceleration", &VehicleDynamics::getBodyAcceleration)
        .def("get_wheel_positions", &VehicleDynamics::getWheelPositions)
        .def("is_initialized", &VehicleDynamics::isInitialized);
    
    // ========================================================================
    // RK4Integrator
    // ========================================================================
    
    py::class_<RK4Integrator>(m, "RK4Integrator")
        .def(py::init<>())
        .def("set_state", &RK4Integrator::setState, py::arg("state"))
        .def("set_time", &RK4Integrator::setTime, py::arg("time"))
        .def("get_state", &RK4Integrator::getState)
        .def("get_time", &RK4Integrator::getTime)
        .def("set_adaptive", &RK4Integrator::setAdaptive, py::arg("enabled"))
        .def("set_tolerance", &RK4Integrator::setTolerance, py::arg("tolerance"))
        .def("set_step_limits", &RK4Integrator::setStepLimits, 
             py::arg("min_step"), py::arg("max_step"))
        .def("step", &RK4Integrator::step, py::arg("f"), py::arg("t"), 
             py::arg("x"), py::arg("dt"))
        .def("integrate", &RK4Integrator::integrate, py::arg("f"), 
             py::arg("t_end"), py::arg("initial_dt"))
        .def("get_error_estimate", &RK4Integrator::getErrorEstimate)
        .def("get_step_count", &RK4Integrator::getStepCount)
        .def("reset", &RK4Integrator::reset);
    
    // ========================================================================
    // AdaptiveRK4Integrator
    // ========================================================================
    
    py::class_<AdaptiveRK4Integrator, RK4Integrator>(m, "AdaptiveRK4Integrator")
        .def(py::init<>())
        .def("set_history_size", &AdaptiveRK4Integrator::setHistorySize, py::arg("size"))
        .def("integrate_with_monitoring", &AdaptiveRK4Integrator::integrateWithMonitoring,
             py::arg("f"), py::arg("t_end"), py::arg("initial_dt"), py::arg("callback"))
        .def("get_average_error", &AdaptiveRK4Integrator::getAverageError)
        .def("get_max_error", &AdaptiveRK4Integrator::getMaxError)
        .def("get_average_step", &AdaptiveRK4Integrator::getAverageStep);
    
    // ========================================================================
    // VehicleRK4Integrator
    // ========================================================================
    
    py::class_<VehicleRK4Integrator, RK4Integrator>(m, "VehicleRK4Integrator")
        .def(py::init<VehicleDynamics*>(), py::arg("dynamics"))
        .def("initialize", &VehicleRK4Integrator::initialize)
        .def("integrate_step", &VehicleRK4Integrator::integrateStep, py::arg("dt"));
    
    // ========================================================================
    // LinearStateSpace
    // ========================================================================
    
    py::class_<LinearStateSpace>(m, "LinearStateSpace")
        .def(py::init<>())
        .def(py::init<int, int, int>(), py::arg("state_dim"), 
             py::arg("input_dim"), py::arg("output_dim"))
        .def("initialize", &LinearStateSpace::initialize, py::arg("config"))
        .def("reset", &LinearStateSpace::reset, py::arg("initial_state"))
        .def("set_matrices", &LinearStateSpace::setMatrices,
             py::arg("A"), py::arg("B"), py::arg("C"), py::arg("D"))
        .def("set_state", &LinearStateSpace::setState, py::arg("state"))
        .def("update", &LinearStateSpace::update, py::arg("u"), py::arg("dt"))
        .def("update_rk4", &LinearStateSpace::updateRK4, py::arg("u"), py::arg("dt"))
        .def("get_output", &LinearStateSpace::getOutput, py::arg("u"))
        .def("get_state", &LinearStateSpace::getState)
        .def("get_A", &LinearStateSpace::getA)
        .def("get_B", &LinearStateSpace::getB)
        .def("get_C", &LinearStateSpace::getC)
        .def("get_D", &LinearStateSpace::getD)
        .def("get_eigenvalues", &LinearStateSpace::getEigenvalues)
        .def("is_stable", &LinearStateSpace::isStable)
        .def("discretize", &LinearStateSpace::discretize, py::arg("dt"))
        .def("is_initialized", &LinearStateSpace::isInitialized);
    
    // ========================================================================
    // NonlinearStateSpace
    // ========================================================================
    
    py::class_<NonlinearStateSpace>(m, "NonlinearStateSpace")
        .def(py::init<>())
        .def("initialize", &NonlinearStateSpace::initialize, py::arg("config"))
        .def("set_state_function", &NonlinearStateSpace::setStateFunction)
        .def("set_output_function", &NonlinearStateSpace::setOutputFunction)
        .def("set_state", &NonlinearStateSpace::setState, py::arg("state"))
        .def("update", &NonlinearStateSpace::update, py::arg("u"), py::arg("dt"))
        .def("get_output", &NonlinearStateSpace::getOutput, py::arg("u"))
        .def("get_state", &NonlinearStateSpace::getState)
        .def("compute_jacobian", &NonlinearStateSpace::computeJacobian,
             py::arg("x_eq"), py::arg("u_eq"), py::arg("epsilon") = 1e-6)
        .def("find_equilibrium", &NonlinearStateSpace::findEquilibrium,
             py::arg("x0"), py::arg("u_eq"), 
             py::arg("tolerance") = 1e-6, py::arg("max_iterations") = 100)
        .def("is_initialized", &NonlinearStateSpace::isInitialized);
    
    // ========================================================================
    // VehicleStateSpace
    // ========================================================================
    
    py::class_<VehicleStateSpace>(m, "VehicleStateSpace")
        .def(py::init<>())
        .def(py::init<VehicleDynamics*>(), py::arg("dynamics"))
        .def("set_dynamics", &VehicleStateSpace::setDynamics, py::arg("dynamics"))
        .def("initialize", &VehicleStateSpace::initialize)
        .def("state_derivative", &VehicleStateSpace::stateDerivative,
             py::arg("state"), py::arg("input"))
        .def("linearize", &VehicleStateSpace::linearize,
             py::arg("x_eq"), py::arg("u_eq"), py::arg("epsilon") = 1e-6)
        .def("get_linearized_model", &VehicleStateSpace::getLinearizedModel)
        .def("compute_eigenvalues", &VehicleStateSpace::computeEigenvalues)
        .def("is_stable", &VehicleStateSpace::isStable)
        .def("compute_damping_ratios", &VehicleStateSpace::computeDampingRatios)
        .def("compute_natural_frequencies", &VehicleStateSpace::computeNaturalFrequencies);
    
    // ========================================================================
    // 模块级别常量
    // ========================================================================
    
    m.attr("VERSION") = "1.0.0";
    m.attr("STATE_DIM") = 18;
    m.attr("INPUT_DIM") = 6;
    
    // 辅助函数
    m.def("create_default_vehicle_config", []() {
        VehicleConfig config;
        config.mass = 2000.0;
        config.inertia_xx = 500.0;
        config.inertia_yy = 2500.0;
        config.inertia_zz = 2800.0;
        config.wheelbase = 2.8;
        config.track_width_front = 1.6;
        config.track_width_rear = 1.6;
        config.cg_height = 0.5;
        config.cg_to_front_axle = 1.4;
        config.cg_to_rear_axle = 1.4;
        config.drag_coefficient = 0.28;
        config.frontal_area = 2.3;
        config.lift_coefficient = 0.05;
        config.air_density = 1.225;
        config.tire_radius = 0.33;
        config.wheel_inertia = 1.5;
        config.cornering_stiffness_front = 80000.0;
        config.cornering_stiffness_rear = 80000.0;
        config.longitudinal_stiffness = 100000.0;
        config.g = 9.81;
        return config;
    });
}
