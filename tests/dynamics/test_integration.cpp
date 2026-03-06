/**
 * @file test_integration.cpp
 * @brief 车辆动力学模块集成测试
 * 
 * 测试场景：
 * 1. 直线加速（0-100 km/h）
 * 2. 转向工况（阶跃转向）
 * 3. 制动工况（紧急制动）
 * 4. 组合工况（加速+转向）
 * 
 * @author BackendAgent #3
 * @date 2026-03-06
 */

#include <gtest/gtest.h>
#include <cmath>
#include <chrono>
#include <fstream>
#include "../src/dynamics/vehicle_dynamics.h"
#include "../src/dynamics/rk4_integrator.h"
#include "../src/dynamics/state_space.h"

using namespace ev_simulation::dynamics;
using Eigen::VectorXd;

// ============================================================================
// 测试夹具
// ============================================================================

class DynamicsIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 配置标准车辆参数
        config_.mass = 2000.0;
        config_.inertia_xx = 500.0;
        config_.inertia_yy = 2500.0;
        config_.inertia_zz = 2800.0;
        config_.wheelbase = 2.8;
        config_.track_width_front = 1.6;
        config_.track_width_rear = 1.6;
        config_.cg_height = 0.5;
        config_.cg_to_front_axle = 1.4;
        config_.cg_to_rear_axle = 1.4;
        config_.drag_coefficient = 0.28;
        config_.frontal_area = 2.3;
        config_.lift_coefficient = 0.05;
        config_.air_density = 1.225;
        config_.tire_radius = 0.33;
        config_.wheel_inertia = 1.5;
        config_.cornering_stiffness_front = 80000.0;
        config_.cornering_stiffness_rear = 80000.0;
        config_.longitudinal_stiffness = 100000.0;
        config_.g = 9.81;
        
        dynamics_.initialize(config_);
    }
    
    void TearDown() override {
    }
    
    VehicleConfig config_;
    VehicleDynamics dynamics_;
    
    // 辅助函数：转换为km/h
    double toKmh(double ms) const { return ms * 3.6; }
    
    // 辅助函数：转换为m/s
    double toMs(double kmh) const { return kmh / 3.6; }
};

// ============================================================================
// 测试1：直线加速（0-100 km/h）
// ============================================================================

TEST_F(DynamicsIntegrationTest, StraightLineAcceleration_0_to_100kmh) {
    // 初始化
    dynamics_.reset();
    
    // 设置恒定驱动力（简化模型：四轮均匀分配）
    double total_drive_force = 8000.0;  // 总驱动力 8kN
    DynamicsInput input;
    input.drive_forces = {total_drive_force/4, total_drive_force/4, 
                          total_drive_force/4, total_drive_force/4};
    input.steering_angle = 0.0;
    
    dynamics_.setInput(input);
    
    // 仿真参数
    double dt = 0.001;  // 1ms时间步
    double max_time = 30.0;  // 最大仿真时间30秒
    double target_speed = toMs(100.0);  // 目标速度 100 km/h
    
    // 记录时间
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 仿真循环
    int steps = 0;
    while (dynamics_.getState().speed < target_speed && steps * dt < max_time) {
        dynamics_.update(dt);
        steps++;
        
        // 更新车轮转速
        for (int i = 0; i < 4; ++i) {
            input.wheel_speeds[i] = dynamics_.getState().vx / config_.tire_radius;
        }
        dynamics_.setInput(input);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // 验证结果
    double final_speed = dynamics_.getState().speed;
    double acceleration_time = steps * dt;
    
    std::cout << "=== 直线加速测试 ===" << std::endl;
    std::cout << "最终速度: " << toKmh(final_speed) << " km/h" << std::endl;
    std::cout << "加速时间: " << acceleration_time << " s" << std::endl;
    std::cout << "仿真步数: " << steps << std::endl;
    std::cout << "计算时间: " << duration.count() << " μs" << std::endl;
    std::cout << "平均单步时间: " << (double)duration.count() / steps << " μs/step" << std::endl;
    
    // 断言
    EXPECT_GE(final_speed, target_speed) << "未能达到目标速度";
    EXPECT_LT(acceleration_time, 15.0) << "加速时间过长";
    EXPECT_GT(acceleration_time, 3.0) << "加速时间过短（可能数值不稳定）";
    
    // 性能要求：单步计算时间 < 100μs
    double avg_step_time = (double)duration.count() / steps;
    EXPECT_LT(avg_step_time, 100.0) << "单步计算时间超过100μs";
}

// ============================================================================
// 测试2：转向工况（阶跃转向）
// ============================================================================

TEST_F(DynamicsIntegrationTest, StepSteerManeuver) {
    // 初始化：从60 km/h开始
    dynamics_.reset();
    VehicleState initial_state;
    initial_state.vx = toMs(60.0);
    dynamics_.reset(initial_state);
    
    // 仿真参数
    double dt = 0.001;
    double steer_time = 1.0;  // 1秒后开始转向
    double steer_angle = 0.05;  // 约3度转向角
    double total_time = 10.0;
    
    DynamicsInput input;
    input.drive_forces = {0, 0, 0, 0};  // 无驱动
    input.steering_angle = 0.0;
    
    // 初始化车轮转速
    for (int i = 0; i < 4; ++i) {
        input.wheel_speeds[i] = initial_state.vx / config_.tire_radius;
    }
    
    dynamics_.setInput(input);
    
    // 记录轨迹
    std::vector<std::pair<double, double>> trajectory;
    
    // 仿真循环
    for (int step = 0; step * dt < total_time; ++step) {
        double t = step * dt;
        
        // 阶跃转向
        if (t >= steer_time) {
            input.steering_angle = steer_angle;
        }
        
        dynamics_.setInput(input);
        dynamics_.update(dt);
        
        // 记录轨迹
        trajectory.push_back({dynamics_.getState().x, dynamics_.getState().y});
    }
    
    // 验证结果
    const auto& final_state = dynamics_.getState();
    
    std::cout << "=== 阶跃转向测试 ===" << std::endl;
    std::cout << "初始速度: 60 km/h" << std::endl;
    std::cout << "转向角: " << steer_angle * 180 / M_PI << " 度" << std::endl;
    std::cout << "最终位置: (" << final_state.x << ", " << final_state.y << ") m" << std::endl;
    std::cout << "最终横摆角: " << final_state.yaw * 180 / M_PI << " 度" << std::endl;
    std::cout << "最终横摆角速度: " << final_state.omega_z << " rad/s" << std::endl;
    
    // 断言：车辆应该转弯
    EXPECT_NE(final_state.y, 0.0) << "车辆未发生横向位移";
    EXPECT_NE(final_state.yaw, 0.0) << "车辆未发生横摆";
    
    // 断言：横摆角速度应该在合理范围内
    EXPECT_LT(std::abs(final_state.omega_z), 1.0) << "横摆角速度过大（可能数值不稳定）";
}

// ============================================================================
// 测试3：制动工况（紧急制动）
// ============================================================================

TEST_F(DynamicsIntegrationTest, EmergencyBraking) {
    // 初始化：从100 km/h开始
    dynamics_.reset();
    VehicleState initial_state;
    initial_state.vx = toMs(100.0);
    dynamics_.reset(initial_state);
    
    // 设置制动力
    double brake_decel = 8.0;  // 8 m/s² 减速度（约0.8g）
    double total_brake_force = config_.mass * brake_decel;
    
    DynamicsInput input;
    input.drive_forces = {-total_brake_force/4, -total_brake_force/4, 
                          -total_brake_force/4, -total_brake_force/4};
    input.steering_angle = 0.0;
    
    // 初始化车轮转速
    for (int i = 0; i < 4; ++i) {
        input.wheel_speeds[i] = initial_state.vx / config_.tire_radius;
    }
    
    dynamics_.setInput(input);
    
    // 仿真参数
    double dt = 0.001;
    double max_time = 10.0;
    
    // 仿真循环
    int steps = 0;
    double initial_x = dynamics_.getState().x;
    
    while (dynamics_.getState().vx > 0.1 && steps * dt < max_time) {
        dynamics_.update(dt);
        steps++;
        
        // 更新车轮转速
        for (int i = 0; i < 4; ++i) {
            input.wheel_speeds[i] = std::max(0.0, dynamics_.getState().vx / config_.tire_radius);
        }
        dynamics_.setInput(input);
    }
    
    // 验证结果
    const auto& final_state = dynamics_.getState();
    double braking_distance = final_state.x - initial_x;
    double braking_time = steps * dt;
    double avg_decel = (toMs(100.0) - final_state.vx) / braking_time;
    
    std::cout << "=== 紧急制动测试 ===" << std::endl;
    std::cout << "初始速度: 100 km/h" << std::endl;
    std::cout << "制动距离: " << braking_distance << " m" << std::endl;
    std::cout << "制动时间: " << braking_time << " s" << std::endl;
    std::cout << "平均减速度: " << avg_decel << " m/s²" << std::endl;
    std::cout << "最终速度: " << toKmh(final_state.vx) << " km/h" << std::endl;
    
    // 断言
    EXPECT_LT(final_state.vx, toMs(5.0)) << "未能有效减速";
    EXPECT_LT(braking_distance, 100.0) << "制动距离过长";
    EXPECT_GT(braking_distance, 30.0) << "制动距离过短（可能数值问题）";
    EXPECT_GT(avg_decel, 4.0) << "平均减速度过低";
}

// ============================================================================
// 测试4：组合工况（加速+转向）
// ============================================================================

TEST_F(DynamicsIntegrationTest, CombinedAccelerationAndSteering) {
    // 初始化
    dynamics_.reset();
    
    // 仿真参数
    double dt = 0.001;
    double total_time = 10.0;
    
    DynamicsInput input;
    input.drive_forces = {2000, 2000, 2000, 2000};  // 总驱动力8kN
    input.steering_angle = 0.0;
    
    dynamics_.setInput(input);
    
    // 仿真循环
    std::vector<double> speeds;
    std::vector<double> lateral_accels;
    
    for (int step = 0; step * dt < total_time; ++step) {
        double t = step * dt;
        
        // 3秒后开始正弦转向
        if (t >= 3.0) {
            input.steering_angle = 0.05 * std::sin(0.5 * (t - 3.0));  // 正弦转向
        }
        
        // 更新车轮转速
        for (int i = 0; i < 4; ++i) {
            input.wheel_speeds[i] = dynamics_.getState().vx / config_.tire_radius;
        }
        
        dynamics_.setInput(input);
        dynamics_.update(dt);
        
        speeds.push_back(dynamics_.getState().speed);
        lateral_accels.push_back(dynamics_.getState().lateral_acceleration);
    }
    
    // 验证结果
    const auto& final_state = dynamics_.getState();
    
    double max_speed = *std::max_element(speeds.begin(), speeds.end());
    double max_lateral_accel = 0;
    for (double la : lateral_accels) {
        max_lateral_accel = std::max(max_lateral_accel, std::abs(la));
    }
    
    std::cout << "=== 组合工况测试 ===" << std::endl;
    std::cout << "最终速度: " << toKmh(final_state.speed) << " km/h" << std::endl;
    std::cout << "最大速度: " << toKmh(max_speed) << " km/h" << std::endl;
    std::cout << "最大侧向加速度: " << max_lateral_accel << " m/s²" << std::endl;
    std::cout << "最终位置: (" << final_state.x << ", " << final_state.y << ") m" << std::endl;
    
    // 断言
    EXPECT_GT(toKmh(max_speed), 60.0) << "最大速度过低";
    EXPECT_GT(max_lateral_accel, 0.5) << "侧向加速度过小";
    EXPECT_LT(max_lateral_accel, 15.0) << "侧向加速度过大（可能数值不稳定）";
}

// ============================================================================
// 测试5：RK4积分器精度测试
// ============================================================================

TEST_F(DynamicsIntegrationTest, RK4IntegratorAccuracy) {
    // 使用简谐振荡器测试RK4精度
    // dx/dt = v
    // dv/dt = -k/m * x
    
    double k = 100.0;  // 弹簧刚度
    double m = 1.0;    // 质量
    double omega = std::sqrt(k / m);  // 自然频率
    
    // ODE函数
    auto harmonic_oscillator = [k, m](double t, const VectorXd& y) -> VectorXd {
        VectorXd dydt(2);
        dydt(0) = y(1);              // dx/dt = v
        dydt(1) = -k / m * y(0);     // dv/dt = -k/m * x
        return dydt;
    };
    
    RK4Integrator integrator;
    VectorXd initial_state(2);
    initial_state << 1.0, 0.0;  // 初始位移1m，初始速度0
    integrator.setState(initial_state);
    
    // 积分参数
    double dt = 0.001;
    double total_time = 10.0;  // 10个周期左右
    double exact_period = 2 * M_PI / omega;
    
    // 精确解
    auto exact_solution = [omega](double t) -> std::pair<double, double> {
        return {std::cos(omega * t), -omega * std::sin(omega * t)};
    };
    
    // 仿真
    double max_position_error = 0;
    double max_velocity_error = 0;
    
    for (int step = 0; step * dt < total_time; ++step) {
        integrator.step(harmonic_oscillator, dt);
        
        double t = integrator.getTime();
        auto [exact_x, exact_v] = exact_solution(t);
        
        VectorXd state = integrator.getState();
        double pos_error = std::abs(state(0) - exact_x);
        double vel_error = std::abs(state(1) - exact_v);
        
        max_position_error = std::max(max_position_error, pos_error);
        max_velocity_error = std::max(max_velocity_error, vel_error);
    }
    
    std::cout << "=== RK4精度测试 ===" << std::endl;
    std::cout << "最大位置误差: " << max_position_error << " m" << std::endl;
    std::cout << "最大速度误差: " << max_velocity_error << " m/s" << std::endl;
    std::cout << "相对位置误差: " << max_position_error / 1.0 * 100 << " %" << std::endl;
    
    // 断言：RK4误差应小于0.1%
    EXPECT_LT(max_position_error, 0.001) << "RK4位置误差过大";
    EXPECT_LT(max_velocity_error, 0.01) << "RK4速度误差过大";
}

// ============================================================================
// 测试6：自适应步长RK4测试
// ============================================================================

TEST_F(DynamicsIntegrationTest, AdaptiveRK4Test) {
    // 使用刚性问题测试自适应步长
    // dx1/dt = -0.5 * x1
    // dx2/dt = -100 * x2  (快速模态)
    
    auto stiff_system = [](double t, const VectorXd& y) -> VectorXd {
        VectorXd dydt(2);
        dydt(0) = -0.5 * y(0);
        dydt(1) = -100.0 * y(1);
        return dydt;
    };
    
    AdaptiveRK4Integrator integrator;
    VectorXd initial_state(2);
    initial_state << 1.0, 1.0;
    integrator.setState(initial_state);
    integrator.setAdaptive(true);
    integrator.setTolerance(1e-6);
    
    // 积分到t=1
    double dt = 0.01;
    integrator.integrate(stiff_system, 1.0, dt);
    
    // 精确解
    double exact_x1 = std::exp(-0.5);
    double exact_x2 = std::exp(-100.0);
    
    VectorXd final_state = integrator.getState();
    double error_x1 = std::abs(final_state(0) - exact_x1);
    double error_x2 = std::abs(final_state(1) - exact_x2);
    
    std::cout << "=== 自适应RK4测试 ===" << std::endl;
    std::cout << "步数: " << integrator.getStepCount() << std::endl;
    std::cout << "平均步长: " << 1.0 / integrator.getStepCount() << " s" << std::endl;
    std::cout << "x1误差: " << error_x1 << std::endl;
    std::cout << "x2误差: " << error_x2 << std::endl;
    std::cout << "平均误差估计: " << integrator.getAverageError() << std::endl;
    
    // 断言
    EXPECT_LT(error_x1, 1e-4) << "x1误差过大";
    EXPECT_LT(integrator.getAverageError(), 1e-3) << "平均误差估计过大";
}

// ============================================================================
// 测试7：状态空间模型测试
// ============================================================================

TEST_F(DynamicsIntegrationTest, StateSpaceModelTest) {
    // 创建线性状态空间模型
    LinearStateSpace ss(2, 1, 1);  // 2状态，1输入，1输出
    
    // 设置简谐振荡器参数
    Eigen::Matrix2d A;
    A << 0, 1,
        -100, 0;  // omega^2 = 100
    
    Eigen::Vector2d B;
    B << 0, 1;
    
    Eigen::RowVector2d C;
    C << 1, 0;
    
    double D = 0;
    
    ss.setMatrices(A, B, C, Eigen::Matrix<double, 1, 1>::Zero());
    
    // 初始状态
    Eigen::Vector2d x0;
    x0 << 1.0, 0.0;
    ss.reset(x0);
    
    // 仿真
    double dt = 0.001;
    double total_time = 5.0;
    Eigen::VectorXd u(1);
    u << 0.0;  // 无外部输入
    
    std::vector<double> positions;
    for (int step = 0; step * dt < total_time; ++step) {
        ss.updateRK4(u, dt);
        positions.push_back(ss.getState()(0));
    }
    
    // 检查稳定性（简谐振荡器应该保持能量守恒）
    double initial_amplitude = 1.0;
    double final_amplitude = std::abs(positions.back());
    double amplitude_drift = std::abs(final_amplitude - initial_amplitude);
    
    std::cout << "=== 状态空间模型测试 ===" << std::endl;
    std::cout << "初始振幅: " << initial_amplitude << std::endl;
    std::cout << "最终振幅: " << final_amplitude << std::endl;
    std::cout << "振幅漂移: " << amplitude_drift << std::endl;
    
    // 检查特征值
    auto eigenvalues = ss.getEigenvalues();
    std::cout << "特征值: ";
    for (const auto& ev : eigenvalues) {
        std::cout << ev << " ";
    }
    std::cout << std::endl;
    
    // 断言
    EXPECT_LT(amplitude_drift, 0.01) << "振幅漂移过大（能量不守恒）";
    EXPECT_FALSE(ss.isStable()) << "简谐振荡器不应该是渐近稳定的";
}

// ============================================================================
// 测试8：车辆状态空间线性化测试
// ============================================================================

TEST_F(DynamicsIntegrationTest, VehicleStateSpaceLinearization) {
    // 初始化车辆状态空间模型
    VehicleStateSpace vss(&dynamics_);
    vss.initialize();
    
    // 平衡点：匀速直线行驶
    VectorXd x_eq(18);
    x_eq.setZero();
    x_eq(6) = toMs(60.0);  // 60 km/h 纵向速度
    
    VectorXd u_eq(6);
    u_eq.setZero();
    u_eq(0) = 500;  // 小驱动力克服阻力
    u_eq(1) = 500;
    u_eq(2) = 500;
    u_eq(3) = 500;
    
    // 线性化
    vss.linearize(x_eq, u_eq, 1e-5);
    
    // 获取特征值
    auto eigenvalues = vss.computeEigenvalues();
    auto damping_ratios = vss.computeDampingRatios();
    auto natural_freqs = vss.computeNaturalFrequencies();
    
    std::cout << "=== 车辆状态空间线性化测试 ===" << std::endl;
    std::cout << "特征值数量: " << eigenvalues.size() << std::endl;
    
    // 打印前几个主要特征值
    std::cout << "主要特征值（按实部排序）：" << std::endl;
    std::vector<size_t> indices(eigenvalues.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
        return eigenvalues[i].real() < eigenvalues[j].real();
    });
    
    for (int i = 0; i < std::min(6, (int)indices.size()); ++i) {
        size_t idx = indices[i];
        std::cout << "  λ" << i << ": " << eigenvalues[idx] 
                  << ", ζ=" << damping_ratios[idx]
                  << ", fn=" << natural_freqs[idx] << " Hz" << std::endl;
    }
    
    // 断言
    EXPECT_EQ(eigenvalues.size(), 18) << "特征值数量应为18";
    
    // 检查是否有不稳定模态（实部大于0）
    bool has_unstable = false;
    for (const auto& ev : eigenvalues) {
        if (ev.real() > 0.1) {
            has_unstable = true;
            break;
        }
    }
    
    // 对于正常车辆配置，不应有明显不稳定模态
    EXPECT_FALSE(has_unstable) << "存在不稳定模态";
}

// ============================================================================
// 测试9：性能基准测试
// ============================================================================

TEST_F(DynamicsIntegrationTest, PerformanceBenchmark) {
    dynamics_.reset();
    
    DynamicsInput input;
    input.drive_forces = {2000, 2000, 2000, 2000};
    input.steering_angle = 0.02;
    
    for (int i = 0; i < 4; ++i) {
        input.wheel_speeds[i] = 10.0;
    }
    
    dynamics_.setInput(input);
    
    // 预热
    for (int i = 0; i < 100; ++i) {
        dynamics_.update(0.001);
    }
    
    // 基准测试
    const int iterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        dynamics_.update(0.001);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double total_time_us = duration.count();
    double avg_time_us = total_time_us / iterations;
    
    std::cout << "=== 性能基准测试 ===" << std::endl;
    std::cout << "总迭代次数: " << iterations << std::endl;
    std::cout << "总计算时间: " << total_time_us << " μs" << std::endl;
    std::cout << "平均单步时间: " << avg_time_us << " μs/step" << std::endl;
    std::cout << "等效仿真频率: " << 1e6 / avg_time_us << " Hz" << std::endl;
    
    // 性能要求：单步时间 < 100μs
    EXPECT_LT(avg_time_us, 100.0) << "性能不满足要求（<100μs/step）";
}

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
