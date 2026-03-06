/**
 * @file vehicle_dynamics.cpp
 * @brief 6自由度车辆动力学模型实现
 * 
 * 实现完整的6自由度（6-DOF）车辆动力学方程，包括：
 * - 牛顿-欧拉方程（力平衡和力矩平衡）
 * - 空气动力学（阻力、升力、侧向力）
 * - 重力分量（坡度、侧倾影响）
 * - 陀螺力矩（惯性耦合）
 * - 轮胎力计算
 * 
 * @author BackendAgent #3
 * @date 2026-03-06
 */

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <Eigen/Dense>
#include "vehicle_dynamics.h"

namespace ev_simulation {
namespace dynamics {

using Eigen::Vector3d;
using Eigen::Matrix3d;

// ============================================================================
// VehicleConfig 实现
// ============================================================================

void VehicleConfig::validate() const {
    if (mass <= 0) {
        throw std::invalid_argument("Vehicle mass must be positive");
    }
    if (inertia_xx <= 0 || inertia_yy <= 0 || inertia_zz <= 0) {
        throw std::invalid_argument("Inertia values must be positive");
    }
    if (wheelbase <= 0) {
        throw std::invalid_argument("Wheelbase must be positive");
    }
    if (track_width_front <= 0 || track_width_rear <= 0) {
        throw std::invalid_argument("Track widths must be positive");
    }
    if (cg_height < 0) {
        throw std::invalid_argument("CG height cannot be negative");
    }
    if (drag_coefficient < 0) {
        throw std::invalid_argument("Drag coefficient cannot be negative");
    }
    if (frontal_area <= 0) {
        throw std::invalid_argument("Frontal area must be positive");
    }
}

// ============================================================================
// VehicleDynamics 实现
// ============================================================================

VehicleDynamics::VehicleDynamics()
    : initialized_(false)
    , params_()
    , state_()
    , input_()
    , aero_force_(Vector3d::Zero())
    , gravity_force_(Vector3d::Zero())
    , gyro_torque_(Vector3d::Zero())
    , wheel_loads_{{0.0, 0.0, 0.0, 0.0}}
{
}

VehicleDynamics::~VehicleDynamics() = default;

void VehicleDynamics::initialize(const VehicleConfig& config) {
    config.validate();
    params_ = config;
    initialized_ = true;
    reset();
}

void VehicleDynamics::reset(const VehicleState& initial_state) {
    if (!initialized_) {
        throw std::runtime_error("VehicleDynamics not initialized");
    }
    state_ = initial_state;
    input_ = DynamicsInput{};
    aero_force_ = Vector3d::Zero();
    gravity_force_ = Vector3d::Zero();
    gyro_torque_ = Vector3d::Zero();
    
    // 初始化车轮载荷（静态载荷分配）
    double static_load = params_.mass * params_.g / 4.0;
    wheel_loads_.fill(static_load);
    
    // 根据质心位置调整前后载荷分配
    double front_ratio = params_.cg_to_rear_axle / params_.wheelbase;
    double rear_ratio = params_.cg_to_front_axle / params_.wheelbase;
    
    wheel_loads_[0] = static_load * 2 * front_ratio / 2;  // FL
    wheel_loads_[1] = static_load * 2 * front_ratio / 2;  // FR
    wheel_loads_[2] = static_load * 2 * rear_ratio / 2;   // RL
    wheel_loads_[3] = static_load * 2 * rear_ratio / 2;   // RR
}

void VehicleDynamics::setInput(const DynamicsInput& input) {
    input_ = input;
}

VehicleStateDerivative VehicleDynamics::computeDerivatives() {
    if (!initialized_) {
        throw std::runtime_error("VehicleDynamics not initialized");
    }
    
    VehicleStateDerivative deriv;
    
    // 1. 计算空气动力学力
    computeAerodynamics();
    
    // 2. 计算重力分量
    computeGravityComponents();
    
    // 3. 计算陀螺力矩
    computeGyroscopicTorque();
    
    // 4. 计算轮胎力
    computeTireForces();
    
    // 5. 更新车轮载荷
    updateWheelLoads();
    
    // 6. 牛顿-欧拉方程（6-DOF）
    
    // 总外力
    Vector3d total_force = input_.tire_force_total + aero_force_ + gravity_force_;
    
    // 总外力矩
    Vector3d total_torque = input_.tire_torque_total + gyro_torque_;
    
    // 纵向加速度 (m/s²)
    // m * ax = Fx_total - m * (vy * omega_z - vz * omega_y)
    deriv.ax = (total_force.x() - params_.mass * (state_.vy * state_.omega_z - state_.vz * state_.omega_y)) / params_.mass;
    
    // 横向加速度 (m/s²)
    // m * ay = Fy_total - m * (vz * omega_x - vx * omega_z)
    deriv.ay = (total_force.y() - params_.mass * (state_.vz * state_.omega_x - state_.vx * state_.omega_z)) / params_.mass;
    
    // 垂向加速度 (m/s²)
    // m * az = Fz_total - m * (vx * omega_y - vy * omega_x)
    deriv.az = (total_force.z() - params_.mass * (state_.vx * state_.omega_y - state_.vy * state_.omega_x)) / params_.mass;
    
    // 侧倾角加速度 (rad/s²)
    // Ix * alpha_x = Mx - (Iz - Iy) * omega_y * omega_z
    deriv.alpha_x = (total_torque.x() - (params_.inertia_zz - params_.inertia_yy) * state_.omega_y * state_.omega_z) / params_.inertia_xx;
    
    // 俯仰角加速度 (rad/s²)
    // Iy * alpha_y = My - (Ix - Iz) * omega_x * omega_z
    deriv.alpha_y = (total_torque.y() - (params_.inertia_xx - params_.inertia_zz) * state_.omega_x * state_.omega_z) / params_.inertia_yy;
    
    // 横摆角加速度 (rad/s²)
    // Iz * alpha_z = Mz - (Iy - Ix) * omega_x * omega_y
    deriv.alpha_z = (total_torque.z() - (params_.inertia_yy - params_.inertia_xx) * state_.omega_x * state_.omega_y) / params_.inertia_zz;
    
    // 7. 速度导数
    deriv.vx = state_.ax;
    deriv.vy = state_.ay;
    deriv.vz = state_.az;
    
    // 8. 角速度导数
    deriv.omega_x = state_.alpha_x;
    deriv.omega_y = state_.alpha_y;
    deriv.omega_z = state_.alpha_z;
    
    // 9. 位置导数（从车身系转换到惯性系）
    Matrix3d rotation = computeRotationMatrix(state_.roll, state_.pitch, state_.yaw);
    Vector3d velocity_body(state_.vx, state_.vy, state_.vz);
    Vector3d velocity_inertial = rotation * velocity_body;
    
    deriv.x = velocity_inertial.x();
    deriv.y = velocity_inertial.y();
    deriv.z = velocity_inertial.z();
    
    // 10. 姿态导数（欧拉角速率）
    // 欧拉角导数 = R_euler^-1 * omega_body
    Matrix3d euler_rate_matrix = computeEulerRateMatrix(state_.roll, state_.pitch);
    Vector3d omega_body(state_.omega_x, state_.omega_y, state_.omega_z);
    Vector3d euler_rates = euler_rate_matrix * omega_body;
    
    deriv.roll = euler_rates.x();
    deriv.pitch = euler_rates.y();
    deriv.yaw = euler_rates.z();
    
    return deriv;
}

void VehicleDynamics::update(double dt) {
    // 计算导数
    VehicleStateDerivative deriv = computeDerivatives();
    
    // 欧拉积分（简单实现，实际应使用RK4）
    state_.x += deriv.x * dt;
    state_.y += deriv.y * dt;
    state_.z += deriv.z * dt;
    
    state_.roll += deriv.roll * dt;
    state_.pitch += deriv.pitch * dt;
    state_.yaw += deriv.yaw * dt;
    
    state_.vx += deriv.ax * dt;
    state_.vy += deriv.ay * dt;
    state_.vz += deriv.az * dt;
    
    state_.omega_x += deriv.alpha_x * dt;
    state_.omega_y += deriv.alpha_y * dt;
    state_.omega_z += deriv.alpha_z * dt;
    
    state_.ax = deriv.ax;
    state_.ay = deriv.ay;
    state_.az = deriv.az;
    
    state_.alpha_x = deriv.alpha_x;
    state_.alpha_y = deriv.alpha_y;
    state_.alpha_z = deriv.alpha_z;
    
    // 更新派生量
    updateDerivedQuantities();
}

void VehicleDynamics::computeAerodynamics() {
    // 计算总速度
    double v_total = std::sqrt(state_.vx * state_.vx + 
                               state_.vy * state_.vy + 
                               state_.vz * state_.vz);
    
    // 动压：q = 0.5 * rho * v²
    double dynamic_pressure = 0.5 * params_.air_density * v_total * v_total;
    
    // 空气阻力（纵向，与运动方向相反）
    double drag_force = dynamic_pressure * params_.drag_coefficient * params_.frontal_area;
    if (v_total > 1e-6) {
        aero_force_.x() = -drag_force * state_.vx / v_total;
        aero_force_.y() = -drag_force * state_.vy / v_total;
    } else {
        aero_force_.x() = 0;
        aero_force_.y() = 0;
    }
    
    // 空气升力（垂向，向上为正）
    double lift_force = dynamic_pressure * params_.lift_coefficient * params_.frontal_area;
    aero_force_.z() = -lift_force;  // 负值表示向下（增加下压力）
    
    // 侧向气动力（简化模型，与侧滑角相关）
    double sideslip_angle = std::atan2(state_.vy, std::abs(state_.vx) + 1e-6);
    double side_force_coefficient = -2.0 * sideslip_angle;  // 简化的侧向力系数
    double side_force = dynamic_pressure * side_force_coefficient * params_.frontal_area;
    aero_force_.y() += side_force;
}

void VehicleDynamics::computeGravityComponents() {
    // 重力在车身系中的分量
    // 取决于道路坡度和车辆侧倾角
    
    // 道路坡度角（从输入获取，或默认为0）
    double road_grade = input_.road_grade_angle;
    
    // 重力纵向分量（坡道阻力）
    gravity_force_.x() = -params_.mass * params_.g * std::sin(road_grade);
    
    // 重力垂向分量
    gravity_force_.z() = -params_.mass * params_.g * std::cos(road_grade) * std::cos(state_.roll);
    
    // 重力侧向分量（侧倾引起的）
    gravity_force_.y() = -params_.mass * params_.g * std::cos(road_grade) * std::sin(state_.roll);
}

void VehicleDynamics::computeGyroscopicTorque() {
    // 陀螺力矩 = omega x (I * omega)
    // 这里简化处理，主要考虑车轮旋转产生的陀螺效应
    
    Vector3d omega(state_.omega_x, state_.omega_y, state_.omega_z);
    Vector3d inertia_omega(
        params_.inertia_xx * state_.omega_x,
        params_.inertia_yy * state_.omega_y,
        params_.inertia_zz * state_.omega_z
    );
    
    gyro_torque_ = omega.cross(inertia_omega);
    
    // 添加车轮旋转产生的陀螺效应
    // 假设四个车轮的转动惯量相同
    double wheel_inertia = params_.wheel_inertia;
    double wheel_radius = params_.tire_radius;
    
    for (int i = 0; i < 4; ++i) {
        double wheel_omega = input_.wheel_speeds[i];
        // 车轮旋转产生的陀螺力矩
        gyro_torque_.x() += wheel_inertia * wheel_omega * state_.omega_z * wheel_radius;
        gyro_torque_.z() -= wheel_inertia * wheel_omega * state_.omega_x * wheel_radius;
    }
}

void VehicleDynamics::computeTireForces() {
    // 简化的轮胎力计算
    // 实际应使用Pacejka魔术公式或其他轮胎模型
    
    // 这里使用线性轮胎模型作为占位符
    double cornering_stiffness_front = params_.cornering_stiffness_front;
    double cornering_stiffness_rear = params_.cornering_stiffness_rear;
    double longitudinal_stiffness = params_.longitudinal_stiffness;
    
    // 前轮侧偏角
    double vfx = state_.vx + state_.omega_z * params_.cg_to_front_axle;
    double vfy = state_.vy + state_.omega_z * params_.track_width_front / 2;
    double alpha_fl = std::atan2(vfy, std::abs(vfx) + 1e-6) - input_.steering_angle;
    
    vfy = state_.vy - state_.omega_z * params_.track_width_front / 2;
    double alpha_fr = std::atan2(vfy, std::abs(vfx) + 1e-6) - input_.steering_angle;
    
    // 后轮侧偏角
    double vrx = state_.vx - state_.omega_z * params_.cg_to_rear_axle;
    double vry = state_.vy + state_.omega_z * params_.track_width_rear / 2;
    double alpha_rl = std::atan2(vry, std::abs(vrx) + 1e-6);
    
    vry = state_.vy - state_.omega_z * params_.track_width_rear / 2;
    double alpha_rr = std::atan2(vry, std::abs(vrx) + 1e-6);
    
    // 侧向力（线性模型）
    input_.tire_forces[0].Fy = -cornering_stiffness_front * alpha_fl * wheel_loads_[0] / (params_.mass * params_.g / 4);
    input_.tire_forces[1].Fy = -cornering_stiffness_front * alpha_fr * wheel_loads_[1] / (params_.mass * params_.g / 4);
    input_.tire_forces[2].Fy = -cornering_stiffness_rear * alpha_rl * wheel_loads_[2] / (params_.mass * params_.g / 4);
    input_.tire_forces[3].Fy = -cornering_stiffness_rear * alpha_rr * wheel_loads_[3] / (params_.mass * params_.g / 4);
    
    // 纵向力（从驱动/制动输入获取）
    input_.tire_forces[0].Fx = input_.drive_forces[0];
    input_.tire_forces[1].Fx = input_.drive_forces[1];
    input_.tire_forces[2].Fx = input_.drive_forces[2];
    input_.tire_forces[3].Fx = input_.drive_forces[3];
    
    // 垂向载荷
    input_.tire_forces[0].Fz = wheel_loads_[0];
    input_.tire_forces[1].Fz = wheel_loads_[1];
    input_.tire_forces[2].Fz = wheel_loads_[2];
    input_.tire_forces[3].Fz = wheel_loads_[3];
    
    // 计算总轮胎力
    input_.tire_force_total = Vector3d::Zero();
    input_.tire_torque_total = Vector3d::Zero();
    
    // 车轮位置（车身系）
    double xf = params_.cg_to_front_axle;
    double xr = -params_.cg_to_rear_axle;
    double wf = params_.track_width_front / 2;
    double wr = params_.track_width_rear / 2;
    
    // 前左轮
    input_.tire_force_total.x() += input_.tire_forces[0].Fx * std::cos(input_.steering_angle) 
                                  - input_.tire_forces[0].Fy * std::sin(input_.steering_angle);
    input_.tire_force_total.y() += input_.tire_forces[0].Fx * std::sin(input_.steering_angle) 
                                  + input_.tire_forces[0].Fy * std::cos(input_.steering_angle);
    input_.tire_force_total.z() += input_.tire_forces[0].Fz;
    
    input_.tire_torque_total.x() += input_.tire_forces[0].Fz * wf - input_.tire_forces[0].Fy * params_.cg_height;
    input_.tire_torque_total.y() += -input_.tire_forces[0].Fz * xf + input_.tire_forces[0].Fx * params_.cg_height;
    input_.tire_torque_total.z() += input_.tire_forces[0].Fy * xf - input_.tire_forces[0].Fx * wf;
    
    // 前右轮
    input_.tire_force_total.x() += input_.tire_forces[1].Fx * std::cos(input_.steering_angle) 
                                  - input_.tire_forces[1].Fy * std::sin(input_.steering_angle);
    input_.tire_force_total.y() += input_.tire_forces[1].Fx * std::sin(input_.steering_angle) 
                                  + input_.tire_forces[1].Fy * std::cos(input_.steering_angle);
    input_.tire_force_total.z() += input_.tire_forces[1].Fz;
    
    input_.tire_torque_total.x() -= input_.tire_forces[1].Fz * wf + input_.tire_forces[1].Fy * params_.cg_height;
    input_.tire_torque_total.y() += -input_.tire_forces[1].Fz * xf + input_.tire_forces[1].Fx * params_.cg_height;
    input_.tire_torque_total.z() += input_.tire_forces[1].Fy * xf + input_.tire_forces[1].Fx * wf;
    
    // 后左轮
    input_.tire_force_total.x() += input_.tire_forces[2].Fx;
    input_.tire_force_total.y() += input_.tire_forces[2].Fy;
    input_.tire_force_total.z() += input_.tire_forces[2].Fz;
    
    input_.tire_torque_total.x() += input_.tire_forces[2].Fz * wr - input_.tire_forces[2].Fy * params_.cg_height;
    input_.tire_torque_total.y() += -input_.tire_forces[2].Fz * xr + input_.tire_forces[2].Fx * params_.cg_height;
    input_.tire_torque_total.z() += input_.tire_forces[2].Fy * xr - input_.tire_forces[2].Fx * wr;
    
    // 后右轮
    input_.tire_force_total.x() += input_.tire_forces[3].Fx;
    input_.tire_force_total.y() += input_.tire_forces[3].Fy;
    input_.tire_force_total.z() += input_.tire_forces[3].Fz;
    
    input_.tire_torque_total.x() -= input_.tire_forces[3].Fz * wr + input_.tire_forces[3].Fy * params_.cg_height;
    input_.tire_torque_total.y() += -input_.tire_forces[3].Fz * xr + input_.tire_forces[3].Fx * params_.cg_height;
    input_.tire_torque_total.z() += input_.tire_forces[3].Fy * xr + input_.tire_forces[3].Fx * wr;
}

void VehicleDynamics::updateWheelLoads() {
    // 静态载荷
    double static_load_front = params_.mass * params_.g * params_.cg_to_rear_axle / params_.wheelbase / 2;
    double static_load_rear = params_.mass * params_.g * params_.cg_to_front_axle / params_.wheelbase / 2;
    
    // 纵向载荷转移（制动/加速）
    double longitudinal_transfer = params_.mass * state_.ax * params_.cg_height / params_.wheelbase;
    
    // 侧向载荷转移
    double lateral_transfer_front = params_.mass * state_.ay * params_.cg_height / params_.track_width_front 
                                    * params_.cg_to_rear_axle / params_.wheelbase;
    double lateral_transfer_rear = params_.mass * state_.ay * params_.cg_height / params_.track_width_rear 
                                   * params_.cg_to_front_axle / params_.wheelbase;
    
    // 更新车轮载荷
    wheel_loads_[0] = static_load_front + longitudinal_transfer / 2 - lateral_transfer_front;  // FL
    wheel_loads_[1] = static_load_front + longitudinal_transfer / 2 + lateral_transfer_front;  // FR
    wheel_loads_[2] = static_load_rear - longitudinal_transfer / 2 - lateral_transfer_rear;    // RL
    wheel_loads_[3] = static_load_rear - longitudinal_transfer / 2 + lateral_transfer_rear;    // RR
    
    // 确保载荷非负
    for (int i = 0; i < 4; ++i) {
        wheel_loads_[i] = std::max(0.0, wheel_loads_[i]);
    }
}

Matrix3d VehicleDynamics::computeRotationMatrix(double roll, double pitch, double yaw) const {
    double cr = std::cos(roll), sr = std::sin(roll);
    double cp = std::cos(pitch), sp = std::sin(pitch);
    double cy = std::cos(yaw), sy = std::sin(yaw);
    
    Matrix3d R;
    R << cy*cp, cy*sp*sr - sy*cr, cy*sp*cr + sy*sr,
         sy*cp, sy*sp*sr + cy*cr, sy*sp*cr - cy*sr,
         -sp,   cp*sr,            cp*cr;
    
    return R;
}

Matrix3d VehicleDynamics::computeEulerRateMatrix(double roll, double pitch) const {
    double cr = std::cos(roll), sr = std::sin(roll);
    double cp = std::cos(pitch), sp = std::sin(pitch);
    double tp = sp / (cp + 1e-10);  // tan(pitch) with protection
    
    Matrix3d T;
    T << 1,  sr*tp,  cr*tp,
         0,  cr,     -sr,
         0,  sr/cp,  cr/cp;
    
    return T;
}

void VehicleDynamics::updateDerivedQuantities() {
    state_.speed = std::sqrt(state_.vx * state_.vx + state_.vy * state_.vy);
    state_.speed_kmh = state_.speed * 3.6;
    
    // 侧向加速度（向心加速度）
    state_.lateral_acceleration = state_.ay + state_.vx * state_.omega_z;
    state_.longitudinal_acceleration = state_.ax;
}

std::array<double, 4> VehicleDynamics::getWheelLoads() const {
    return wheel_loads_;
}

Vector3d VehicleDynamics::getBodyAcceleration() const {
    return Vector3d(state_.ax, state_.ay, state_.az);
}

std::array<std::tuple<double, double, double>, 4> VehicleDynamics::getWheelPositions() const {
    std::array<std::tuple<double, double, double>, 4> positions;
    
    double xf = params_.cg_to_front_axle;
    double xr = -params_.cg_to_rear_axle;
    double wf = params_.track_width_front / 2;
    double wr = params_.track_width_rear / 2;
    
    // 前左
    positions[0] = std::make_tuple(xf, wf, 0);
    // 前右
    positions[1] = std::make_tuple(xf, -wf, 0);
    // 后左
    positions[2] = std::make_tuple(xr, wr, 0);
    // 后右
    positions[3] = std::make_tuple(xr, -wr, 0);
    
    return positions;
}

} // namespace dynamics
} // namespace ev_simulation
