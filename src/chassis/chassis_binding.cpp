/**
 * @file chassis_binding.cpp
 * @brief 底盘系统Python绑定
 * @author CodeCraft
 * @date 2026-03-08
 * 
 * 包括：
 * - Suspension: 主动悬架系统
 * - Steering: 电动助力转向系统
 * - Braking: 线控制动系统
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include <Eigen/Dense>

#include "suspension.h"
#include "steering.h"
#include "braking.h"

namespace py = pybind11;
using namespace ev_dynamics::chassis;

// ============================================================================
// Python模块定义
// ============================================================================
PYBIND11_MODULE(chassis, m) {
    m.doc() = "Chassis module: active suspension, electric power steering, and brake-by-wire systems";
    
    // ========================================================================
    // 枚举类型
    // ========================================================================
    
    py::enum_<SuspensionType>(m, "SuspensionType")
        .value("PASSIVE", SuspensionType::PASSIVE)
        .value("SEMI_ACTIVE", SuspensionType::SEMI_ACTIVE)
        .value("ACTIVE", SuspensionType::ACTIVE)
        .export_values();
    
    py::enum_<DampingMode>(m, "DampingMode")
        .value("COMFORT", DampingMode::COMFORT)
        .value("NORMAL", DampingMode::NORMAL)
        .value("SPORT", DampingMode::SPORT)
        .value("ADAPTIVE", DampingMode::ADAPTIVE)
        .export_values();
    
    py::enum_<SteeringType>(m, "SteeringType")
        .value("MANUAL", SteeringType::MANUAL)
        .value("HYDRAULIC", SteeringType::HYDRAULIC)
        .value("ELECTRIC", SteeringType::ELECTRIC)
        .value("STEER_BY_WIRE", SteeringType::STEER_BY_WIRE)
        .export_values();
    
    py::enum_<AssistMode>(m, "AssistMode")
        .value("COMFORT", AssistMode::COMFORT)
        .value("NORMAL", AssistMode::NORMAL)
        .value("SPORT", AssistMode::SPORT)
        .value("ADAPTIVE", AssistMode::ADAPTIVE)
        .export_values();
    
    py::enum_<BrakingType>(m, "BrakingType")
        .value("HYDRAULIC", BrakingType::HYDRAULIC)
        .value("ELECTRO_HYDRAULIC", BrakingType::ELECTRO_HYDRAULIC)
        .value("ELECTRO_MECHANICAL", BrakingType::ELECTRO_MECHANICAL)
        .export_values();
    
    py::enum_<ABSState>(m, "ABSState")
        .value("INACTIVE", ABSState::INACTIVE)
        .value("PRESSURE_HOLD", ABSState::PRESSURE_HOLD)
        .value("PRESSURE_DECREASE", ABSState::PRESSURE_DECREASE)
        .value("PRESSURE_INCREASE", ABSState::PRESSURE_INCREASE)
        .export_values();
    
    // ========================================================================
    // Suspension配置和状态
    // ========================================================================
    
    py::class_<SuspensionCornerConfig>(m, "SuspensionCornerConfig")
        .def(py::init<>())
        .def_readwrite("spring_stiffness", &SuspensionCornerConfig::spring_stiffness)
        .def_readwrite("damping_compression", &SuspensionCornerConfig::damping_compression)
        .def_readwrite("damping_rebound", &SuspensionCornerConfig::damping_rebound)
        .def_readwrite("sprung_mass", &SuspensionCornerConfig::sprung_mass)
        .def_readwrite("unsprung_mass", &SuspensionCornerConfig::unsprung_mass);
    
    py::class_<SuspensionConfig>(m, "SuspensionConfig")
        .def(py::init<>())
        .def_readwrite("front_left", &SuspensionConfig::front_left)
        .def_readwrite("front_right", &SuspensionConfig::front_right)
        .def_readwrite("rear_left", &SuspensionConfig::rear_left)
        .def_readwrite("rear_right", &SuspensionConfig::rear_right)
        .def_readwrite("type", &SuspensionConfig::type)
        .def_readwrite("damping_mode", &SuspensionConfig::damping_mode);
    
    py::class_<SuspensionCornerState>(m, "SuspensionCornerState")
        .def(py::init<>())
        .def_readwrite("displacement", &SuspensionCornerState::displacement)
        .def_readwrite("velocity", &SuspensionCornerState::velocity)
        .def_readwrite("spring_force", &SuspensionCornerState::spring_force)
        .def_readwrite("damping_force", &SuspensionCornerState::damping_force)
        .def_readwrite("actuator_force", &SuspensionCornerState::actuator_force);
    
    py::class_<SuspensionState>(m, "SuspensionState")
        .def(py::init<>())
        .def_readwrite("front_left", &SuspensionState::front_left)
        .def_readwrite("front_right", &SuspensionState::front_right)
        .def_readwrite("rear_left", &SuspensionState::rear_left)
        .def_readwrite("rear_right", &SuspensionState::rear_right)
        .def_readwrite("body_height", &SuspensionState::body_height)
        .def_readwrite("body_pitch", &SuspensionState::body_pitch)
        .def_readwrite("body_roll", &SuspensionState::body_roll)
        .def_readwrite("comfort_index", &SuspensionState::comfort_index)
        .def_readwrite("handling_index", &SuspensionState::handling_index);
    
    py::class_<SuspensionInput>(m, "SuspensionInput")
        .def(py::init<>())
        .def_readwrite("road_fl", &SuspensionInput::road_fl)
        .def_readwrite("road_fr", &SuspensionInput::road_fr)
        .def_readwrite("road_rl", &SuspensionInput::road_rl)
        .def_readwrite("road_rr", &SuspensionInput::road_rr)
        .def_readwrite("vehicle_speed", &SuspensionInput::vehicle_speed)
        .def_readwrite("lateral_acceleration", &SuspensionInput::lateral_acceleration)
        .def_readwrite("target_height", &SuspensionInput::target_height)
        .def_readwrite("requested_damping_mode", &SuspensionInput::requested_damping_mode);
    
    // Suspension类
    py::class_<Suspension>(m, "Suspension")
        .def(py::init<const SuspensionConfig&>(), py::arg("config") = SuspensionConfig())
        .def("initialize", &Suspension::initialize)
        .def("update", &Suspension::update, py::arg("dt"), py::arg("input"))
        .def("getState", &Suspension::getState, py::return_value_policy::reference)
        .def("getConfig", &Suspension::getConfig, py::return_value_policy::reference)
        .def("setTargetHeight", &Suspension::setTargetHeight, py::arg("height"))
        .def("setDampingMode", &Suspension::setDampingMode, py::arg("mode"))
        .def("setPerformanceWeights", &Suspension::setPerformanceWeights,
             py::arg("comfort"), py::arg("handling"))
        .def("reset", &Suspension::reset);
    
    // ========================================================================
    // Steering配置和状态
    // ========================================================================
    
    py::class_<SteeringConfig>(m, "SteeringConfig")
        .def(py::init<>())
        .def_readwrite("wheelbase", &SteeringConfig::wheelbase)
        .def_readwrite("track_width", &SteeringConfig::track_width)
        .def_readwrite("base_steering_ratio", &SteeringConfig::base_steering_ratio)
        .def_readwrite("type", &SteeringConfig::type)
        .def_readwrite("assist_mode", &SteeringConfig::assist_mode);
    
    py::class_<SteeringState>(m, "SteeringState")
        .def(py::init<>())
        .def_readwrite("steering_wheel_angle", &SteeringState::steering_wheel_angle)
        .def_readwrite("wheel_angle_left", &SteeringState::wheel_angle_left)
        .def_readwrite("wheel_angle_right", &SteeringState::wheel_angle_right)
        .def_readwrite("assist_torque", &SteeringState::assist_torque)
        .def_readwrite("current_steering_ratio", &SteeringState::current_steering_ratio);
    
    py::class_<SteeringInput>(m, "SteeringInput")
        .def(py::init<>())
        .def_readwrite("driver_torque", &SteeringInput::driver_torque)
        .def_readwrite("driver_angle", &SteeringInput::driver_angle)
        .def_readwrite("vehicle_speed", &SteeringInput::vehicle_speed)
        .def_readwrite("lateral_acceleration", &SteeringInput::lateral_acceleration);
    
    // Steering类
    py::class_<Steering>(m, "Steering")
        .def(py::init<const SteeringConfig&>(), py::arg("config") = SteeringConfig())
        .def("initialize", &Steering::initialize)
        .def("update", &Steering::update, py::arg("dt"), py::arg("input"))
        .def("getState", &Steering::getState, py::return_value_policy::reference)
        .def("getConfig", &Steering::getConfig, py::return_value_policy::reference)
        .def("setSteeringWheelAngle", &Steering::setSteeringWheelAngle, py::arg("angle"))
        .def("setAssistMode", &Steering::setAssistMode, py::arg("mode"))
        .def("setSteeringRatio", &Steering::setSteeringRatio, py::arg("ratio"))
        .def("enableActiveReturn", &Steering::enableActiveReturn, py::arg("enable"))
        .def("reset", &Steering::reset);
    
    // ========================================================================
    // Braking配置和状态
    // ========================================================================
    
    py::class_<BrakeCornerConfig>(m, "BrakeCornerConfig")
        .def(py::init<>())
        .def_readwrite("max_brake_torque", &BrakeCornerConfig::max_brake_torque)
        .def_readwrite("brake_response_time", &BrakeCornerConfig::brake_response_time);
    
    py::class_<BrakingConfig>(m, "BrakingConfig")
        .def(py::init<>())
        .def_readwrite("front_left", &BrakingConfig::front_left)
        .def_readwrite("type", &BrakingConfig::type)
        .def_readwrite("abs_enabled", &BrakingConfig::abs_enabled)
        .def_readwrite("ebd_enabled", &BrakingConfig::ebd_enabled)
        .def_readwrite("regen_enabled", &BrakingConfig::regen_enabled);
    
    py::class_<BrakeCornerState>(m, "BrakeCornerState")
        .def(py::init<>())
        .def_readwrite("brake_pressure", &BrakeCornerState::brake_pressure)
        .def_readwrite("brake_torque", &BrakeCornerState::brake_torque)
        .def_readwrite("wheel_slip", &BrakeCornerState::wheel_slip)
        .def_readwrite("abs_state", &BrakeCornerState::abs_state);
    
    py::class_<BrakingState>(m, "BrakingState")
        .def(py::init<>())
        .def_readwrite("front_left", &BrakingState::front_left)
        .def_readwrite("front_right", &BrakingState::front_right)
        .def_readwrite("rear_left", &BrakingState::rear_left)
        .def_readwrite("rear_right", &BrakingState::rear_right)
        .def_readwrite("brake_pedal_position", &BrakingState::brake_pedal_position)
        .def_readwrite("total_brake_torque", &BrakingState::total_brake_torque)
        .def_readwrite("abs_active", &BrakingState::abs_active)
        .def_readwrite("regen_torque", &BrakingState::regen_torque);
    
    py::class_<BrakingInput>(m, "BrakingInput")
        .def(py::init<>())
        .def_readwrite("brake_pedal_position", &BrakingInput::brake_pedal_position)
        .def_readwrite("vehicle_speed", &BrakingInput::vehicle_speed)
        .def_readwrite("wheel_speed_fl", &BrakingInput::wheel_speed_fl)
        .def_readwrite("wheel_speed_fr", &BrakingInput::wheel_speed_fr)
        .def_readwrite("wheel_speed_rl", &BrakingInput::wheel_speed_rl)
        .def_readwrite("wheel_speed_rr", &BrakingInput::wheel_speed_rr)
        .def_readwrite("regen_request", &BrakingInput::regen_request);
    
    // Braking类
    py::class_<Braking>(m, "Braking")
        .def(py::init<const BrakingConfig&>(), py::arg("config") = BrakingConfig())
        .def("initialize", &Braking::initialize)
        .def("update", &Braking::update, py::arg("dt"), py::arg("input"))
        .def("getState", &Braking::getState, py::return_value_policy::reference)
        .def("getConfig", &Braking::getConfig, py::return_value_policy::reference)
        .def("enableABS", &Braking::enableABS, py::arg("enable"))
        .def("enableEBD", &Braking::enableEBD, py::arg("enable"))
        .def("enableRegen", &Braking::enableRegen, py::arg("enable"))
        .def("setRegenBlendFactor", &Braking::setRegenBlendFactor, py::arg("factor"))
        .def("emergencyBrake", &Braking::emergencyBrake)
        .def("releaseBrake", &Braking::releaseBrake)
        .def("reset", &Braking::reset);
    
    // 模块版本信息
    m.attr("__version__") = "1.0.0";
}
