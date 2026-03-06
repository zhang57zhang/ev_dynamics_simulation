/**
 * @file vehicle_dynamics.h
 * @brief 6自由度车辆动力学模型头文件
 * 
 * 定义车辆动力学所需的数据结构、状态和配置
 * 
 * @author BackendAgent #3
 * @date 2026-03-06
 */

#pragma once

#include <array>
#include <tuple>
#include <Eigen/Dense>

namespace ev_simulation {
namespace dynamics {

using Eigen::Vector3d;
using Eigen::Matrix3d;

/**
 * @brief 车辆配置参数
 */
struct VehicleConfig {
    // === 质量参数 ===
    double mass{2000.0};                    ///< 整车质量 (kg)
    double inertia_xx{500.0};               ///< 绕X轴转动惯量 (kg·m²) - 侧倾
    double inertia_yy{2500.0};              ///< 绕Y轴转动惯量 (kg·m²) - 俯仰
    double inertia_zz{2800.0};              ///< 绕Z轴转动惯量 (kg·m²) - 横摆
    
    // === 尺寸参数 ===
    double wheelbase{2.8};                  ///< 轴距 (m)
    double track_width_front{1.6};          ///< 前轮距 (m)
    double track_width_rear{1.6};           ///< 后轮距 (m)
    double cg_height{0.5};                  ///< 质心高度 (m)
    double cg_to_front_axle{1.4};           ///< 质心到前轴距离 (m)
    double cg_to_rear_axle{1.4};            ///< 质心到后轴距离 (m)
    
    // === 空气动力学 ===
    double drag_coefficient{0.28};          ///< 阻力系数 Cd
    double frontal_area{2.3};               ///< 迎风面积 (m²)
    double lift_coefficient{0.05};          ///< 升力系数 Cl
    double air_density{1.225};              ///< 空气密度 (kg/m³)
    
    // === 轮胎参数 ===
    double tire_radius{0.33};               ///< 轮胎半径 (m)
    double wheel_inertia{1.5};              ///< 车轮转动惯量 (kg·m²)
    double cornering_stiffness_front{80000.0};  ///< 前轮侧偏刚度 (N/rad)
    double cornering_stiffness_rear{80000.0};   ///< 后轮侧偏刚度 (N/rad)
    double longitudinal_stiffness{100000.0};    ///< 纵向刚度 (N)
    
    // === 物理常数 ===
    double g{9.81};                         ///< 重力加速度 (m/s²)
    
    /**
     * @brief 验证配置参数有效性
     * @throws std::invalid_argument 参数无效
     */
    void validate() const;
};

/**
 * @brief 轮胎力结构
 */
struct TireForce {
    double Fx{0.0};                         ///< 纵向力 (N)
    double Fy{0.0};                         ///< 侧向力 (N)
    double Fz{0.0};                         ///< 垂向载荷 (N)
    double Mz{0.0};                         ///< 回正力矩 (N·m)
};

/**
 * @brief 动力学输入
 */
struct DynamicsInput {
    // === 驱动/制动力 ===
    std::array<double, 4> drive_forces{{0, 0, 0, 0}};  ///< 四轮驱动力 (N)
    std::array<double, 4> brake_torques{{0, 0, 0, 0}}; ///< 四轮制动力矩 (N·m)
    
    // === 转向 ===
    double steering_angle{0.0};             ///< 前轮转角 (rad)
    
    // === 轮胎力（由轮胎模型计算）===
    std::array<TireForce, 4> tire_forces;  ///< 四轮轮胎力
    
    // === 道路条件 ===
    double road_grade_angle{0.0};           ///< 道路坡度角 (rad)
    double road_friction{1.0};              ///< 道路摩擦系数
    
    // === 车轮转速 ===
    std::array<double, 4> wheel_speeds{{0, 0, 0, 0}};  ///< 四轮角速度 (rad/s)
    
    // === 总力/力矩（计算结果）===
    Vector3d tire_force_total{Vector3d::Zero()};   ///< 总轮胎力
    Vector3d tire_torque_total{Vector3d::Zero()};  ///< 总轮胎力矩
};

/**
 * @brief 车辆状态（6-DOF）
 */
struct VehicleState {
    // === 位置（惯性系）===
    double x{0.0};                          ///< X坐标 (m)
    double y{0.0};                          ///< Y坐标 (m)
    double z{0.0};                          ///< Z坐标 (m)
    
    // === 姿态（欧拉角）===
    double roll{0.0};                       ///< 侧倾角 (rad)
    double pitch{0.0};                      ///< 俯仰角 (rad)
    double yaw{0.0};                        ///< 横摆角 (rad)
    
    // === 速度（车身系）===
    double vx{0.0};                         ///< 纵向速度 (m/s)
    double vy{0.0};                         ///< 横向速度 (m/s)
    double vz{0.0};                         ///< 垂向速度 (m/s)
    
    // === 角速度（车身系）===
    double omega_x{0.0};                    ///< 侧倾角速度 (rad/s)
    double omega_y{0.0};                    ///< 俯仰角速度 (rad/s)
    double omega_z{0.0};                    ///< 横摆角速度 (rad/s)
    
    // === 加速度（车身系）===
    double ax{0.0};                         ///< 纵向加速度 (m/s²)
    double ay{0.0};                         ///< 横向加速度 (m/s²)
    double az{0.0};                         ///< 垂向加速度 (m/s²)
    
    // === 角加速度（车身系）===
    double alpha_x{0.0};                    ///< 侧倾角加速度 (rad/s²)
    double alpha_y{0.0};                    ///< 俯仰角加速度 (rad/s²)
    double alpha_z{0.0};                    ///< 横摆角加速度 (rad/s²)
    
    // === 派生量 ===
    double speed{0.0};                      ///< 车速 (m/s)
    double speed_kmh{0.0};                  ///< 车速 (km/h)
    double lateral_acceleration{0.0};       ///< 侧向加速度 (m/s²)
    double longitudinal_acceleration{0.0};  ///< 纵向加速度 (m/s²)
};

/**
 * @brief 车辆状态导数
 */
struct VehicleStateDerivative {
    // === 位置导数 ===
    double x{0.0};
    double y{0.0};
    double z{0.0};
    
    // === 姿态导数 ===
    double roll{0.0};
    double pitch{0.0};
    double yaw{0.0};
    
    // === 速度导数（加速度）===
    double vx{0.0};
    double vy{0.0};
    double vz{0.0};
    
    // === 角速度导数（角加速度）===
    double omega_x{0.0};
    double omega_y{0.0};
    double omega_z{0.0};
    
    // === 加速度导数 ===
    double ax{0.0};
    double ay{0.0};
    double az{0.0};
    
    // === 角加速度导数 ===
    double alpha_x{0.0};
    double alpha_y{0.0};
    double alpha_z{0.0};
};

/**
 * @brief 6自由度车辆动力学模型
 * 
 * 实现完整的车辆动力学计算，包括：
 * - 牛顿-欧拉方程
 * - 空气动力学
 * - 重力分量
 * - 陀螺力矩
 * - 轮胎力
 */
class VehicleDynamics {
public:
    VehicleDynamics();
    ~VehicleDynamics();
    
    /**
     * @brief 初始化车辆动力学模型
     * @param config 车辆配置参数
     */
    void initialize(const VehicleConfig& config);
    
    /**
     * @brief 重置车辆状态
     * @param initial_state 初始状态
     */
    void reset(const VehicleState& initial_state = VehicleState{});
    
    /**
     * @brief 设置动力学输入
     * @param input 动力学输入
     */
    void setInput(const DynamicsInput& input);
    
    /**
     * @brief 计算状态导数
     * @return 状态导数
     */
    VehicleStateDerivative computeDerivatives();
    
    /**
     * @brief 更新车辆状态（欧拉积分）
     * @param dt 时间步长 (s)
     */
    void update(double dt);
    
    /**
     * @brief 获取当前状态
     * @return 车辆状态
     */
    const VehicleState& getState() const { return state_; }
    
    /**
     * @brief 获取配置参数
     * @return 配置参数
     */
    const VehicleConfig& getConfig() const { return params_; }
    
    /**
     * @brief 获取四轮垂直载荷
     * @return 四轮载荷 [FL, FR, RL, RR]
     */
    std::array<double, 4> getWheelLoads() const;
    
    /**
     * @brief 获取车身加速度
     * @return 加速度向量 (ax, ay, az)
     */
    Vector3d getBodyAcceleration() const;
    
    /**
     * @brief 获取四轮位置（车身系）
     * @return 四轮位置 [(x, y, z), ...]
     */
    std::array<std::tuple<double, double, double>, 4> getWheelPositions() const;
    
    /**
     * @brief 检查是否已初始化
     * @return true: 已初始化
     */
    bool isInitialized() const { return initialized_; }
    
private:
    /**
     * @brief 计算空气动力学力
     */
    void computeAerodynamics();
    
    /**
     * @brief 计算重力分量
     */
    void computeGravityComponents();
    
    /**
     * @brief 计算陀螺力矩
     */
    void computeGyroscopicTorque();
    
    /**
     * @brief 计算轮胎力
     */
    void computeTireForces();
    
    /**
     * @brief 更新车轮载荷
     */
    void updateWheelLoads();
    
    /**
     * @brief 计算旋转矩阵（从车身系到惯性系）
     * @param roll 侧倾角
     * @param pitch 俯仰角
     * @param yaw 横摆角
     * @return 旋转矩阵
     */
    Matrix3d computeRotationMatrix(double roll, double pitch, double yaw) const;
    
    /**
     * @brief 计算欧拉角速率转换矩阵
     * @param roll 侧倾角
     * @param pitch 俯仰角
     * @return 欧拉角速率矩阵
     */
    Matrix3d computeEulerRateMatrix(double roll, double pitch) const;
    
    /**
     * @brief 更新派生量
     */
    void updateDerivedQuantities();
    
    // === 成员变量 ===
    bool initialized_;
    VehicleConfig params_;
    VehicleState state_;
    DynamicsInput input_;
    
    // 计算中间量
    Vector3d aero_force_;
    Vector3d gravity_force_;
    Vector3d gyro_torque_;
    std::array<double, 4> wheel_loads_;
};

} // namespace dynamics
} // namespace ev_simulation
