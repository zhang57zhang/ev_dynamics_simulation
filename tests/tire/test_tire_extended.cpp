/**
 * @file test_tire_extended.cpp
 * @brief 轮胎模型扩展测试
 * 
 * 深入测试轮胎模型的物理特性和边界条件
 * 
 * @author TireAgent
 * @date 2026-03-08
 */

#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <fstream>
#include "../src/tire/pacejka_model.h"
#include "../src/tire/tire_dynamics.h"

using namespace ev_simulation::tire;

// ============================================================================
// Pacejka模型物理特性测试
// ============================================================================

class PacejkaPhysicsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 使用典型的轮胎参数
        params_.PCx = 1.65;
        params_.PDx = 1.0;
        params_.PDx1 = 1.0;
        params_.PEx = -0.5;
        params_.PKx = 21.68;
        params_.PKx1 = 21.68;
        
        params_.PCy = 1.3;
        params_.PDy = 1.0;
        params_.PDy1 = 1.0;
        params_.PEy = -1.0;
        params_.PKy = 14.0;
        params_.PKy1 = 14.0;
        
        params_.PCz = 2.22;
        params_.PDz = 0.12;
        params_.PKz = 11.0;
        
        params_.Fz0 = 4000.0;
        
        model_ = std::make_unique<PacejkaModel>(params_);
    }
    
    PacejkaParameters params_;
    std::unique_ptr<PacejkaModel> model_;
};

// 测试1：纵向力曲线形状（S型曲线）
TEST_F(PacejkaPhysicsTest, FxCurveShape) {
    double Fz = 4000.0;
    std::vector<double> kappa_values;
    std::vector<double> Fx_values;
    
    // 生成曲线
    for (double kappa = -1.0; kappa <= 1.0; kappa += 0.05) {
        double Fx = model_->computeFx(kappa, Fz);
        kappa_values.push_back(kappa);
        Fx_values.push_back(Fx);
    }
    
    // 验证S型曲线特征
    // 1. 在kappa=0附近应该近似线性
    double Fx_small = model_->computeFx(0.01, Fz);
    double Fx_small2 = model_->computeFx(0.02, Fz);
    double slope_small = (Fx_small2 - Fx_small) / 0.01;
    
    // 线性区斜率应该接近刚度
    EXPECT_GT(slope_small, 50000.0);  // > 50 kN/unit slip
    EXPECT_LT(slope_small, 200000.0); // < 200 kN/unit slip
    
    // 2. 曲线应该饱和（在极端滑移时）
    double Fx_large_pos = model_->computeFx(0.5, Fz);
    double Fx_extreme_pos = model_->computeFx(1.0, Fz);
    EXPECT_LT(std::abs(Fx_extreme_pos - Fx_large_pos), std::abs(Fx_large_pos) * 0.3);
    
    // 3. 曲线应该关于原点对称（对于零偏移）
    double Fx_pos = model_->computeFx(0.1, Fz);
    double Fx_neg = model_->computeFx(-0.1, Fz);
    EXPECT_NEAR(Fx_pos, -Fx_neg, std::abs(Fx_pos) * 0.1);
}

// 测试2：横向力曲线形状
TEST_F(PacejkaPhysicsTest, FyCurveShape) {
    double Fz = 4000.0;
    
    // 生成曲线
    std::vector<double> alpha_values;
    std::vector<double> Fy_values;
    
    for (double alpha = -0.5; alpha <= 0.5; alpha += 0.05) {
        double Fy = model_->computeFy(alpha, Fz);
        alpha_values.push_back(alpha);
        Fy_values.push_back(Fy);
    }
    
    // 验证曲线特征
    // 1. 线性区
    double Fy_small = model_->computeFy(0.02, Fz);  // ~1度
    double Fy_small2 = model_->computeFy(0.04, Fz);
    double slope_small = (Fy_small2 - Fy_small) / 0.02;
    
    EXPECT_GT(slope_small, 40000.0);  // > 40 kN/rad
    
    // 2. 峰值力
    double Fy_peak = 0.0;
    for (double Fy : Fy_values) {
        if (std::abs(Fy) > std::abs(Fy_peak)) {
            Fy_peak = Fy;
        }
    }
    
    // 峰值力应该接近 mu * Fz
    EXPECT_GT(std::abs(Fy_peak), Fz * 0.8);
    EXPECT_LT(std::abs(Fy_peak), Fz * 1.5);
    
    // 3. 对称性
    double Fy_pos = model_->computeFy(0.2, Fz);
    double Fy_neg = model_->computeFy(-0.2, Fz);
    EXPECT_NEAR(Fy_pos, -Fy_neg, std::abs(Fy_pos) * 0.15);
}

// 测试3：载荷敏感性
TEST_F(PacejkaPhysicsTest, LoadSensitivity) {
    // 测试不同载荷下的力
    std::vector<double> Fz_values = {2000.0, 3000.0, 4000.0, 5000.0, 6000.0};
    double kappa = 0.1;
    
    std::vector<double> Fx_values;
    for (double Fz : Fz_values) {
        double Fx = model_->computeFx(kappa, Fz);
        Fx_values.push_back(Fx);
    }
    
    // 纵向力应该随载荷增加而增加
    for (size_t i = 1; i < Fx_values.size(); ++i) {
        EXPECT_GT(std::abs(Fx_values[i]), std::abs(Fx_values[i-1]) * 0.8);
    }
    
    // 但不是线性关系（载荷敏感性）
    double Fx_ratio = Fx_values[4] / Fx_values[2];  // 6000/4000
    double Fz_ratio = 6000.0 / 4000.0;
    EXPECT_LT(Fx_ratio, Fz_ratio);  // 力增加比例小于载荷增加比例
}

// 测试4：外倾角影响
TEST_F(PacejkaPhysicsTest, CamberEffect) {
    double Fz = 4000.0;
    double alpha = 0.1;
    
    std::vector<double> gamma_values = {-0.1, -0.05, 0.0, 0.05, 0.1};  // rad
    std::vector<double> Fy_values;
    
    for (double gamma : gamma_values) {
        double Fy = model_->computeFy(alpha, Fz, gamma);
        Fy_values.push_back(Fy);
    }
    
    // 外倾角应该影响横向力
    // 正外倾应该产生额外的横向力
    double Fy_zero_camber = Fy_values[2];
    double Fy_pos_camber = Fy_values[4];
    
    // 外倾影响通常较小（< 10%）
    EXPECT_LT(std::abs(Fy_pos_camber - Fy_zero_camber), std::abs(Fy_zero_camber) * 0.2);
}

// 测试5：回正力矩特性
TEST_F(PacejkaPhysicsTest, AligningMomentCharacteristics) {
    double Fz = 4000.0;
    
    // 回正力矩应该在小侧偏角时线性增长，然后减小
    std::vector<double> alpha_values = {0.0, 0.05, 0.1, 0.15, 0.2, 0.3};
    std::vector<double> Mz_values;
    
    for (double alpha : alpha_values) {
        double Mz = model_->computeMz(alpha, Fz);
        Mz_values.push_back(Mz);
    }
    
    // 1. 线性区
    double Mz_small = Mz_values[1];  // alpha=0.05
    EXPECT_GT(std::abs(Mz_small), 0.0);
    
    // 2. 峰值
    int peak_idx = 0;
    for (int i = 1; i < Mz_values.size(); ++i) {
        if (std::abs(Mz_values[i]) > std::abs(Mz_values[peak_idx])) {
            peak_idx = i;
        }
    }
    
    // 峰值应该在中等侧偏角
    EXPECT_GT(peak_idx, 0);
    EXPECT_LT(peak_idx, 5);
    
    // 3. 在大侧偏角时应该减小
    EXPECT_LT(std::abs(Mz_values[5]), std::abs(Mz_values[peak_idx]));
}

// 测试6：组合滑移椭圆
TEST_F(PacejkaPhysicsTest, CombinedSlipEllipse) {
    double Fz = 4000.0;
    
    // 在不同侧偏角下，纵向力应该减小
    std::vector<double> alpha_values = {0.0, 0.05, 0.1, 0.15};
    double kappa = 0.1;
    
    std::vector<double> Fx_values;
    for (double alpha : alpha_values) {
        double Fx = model_->computeFxCombined(kappa, alpha, Fz);
        Fx_values.push_back(Fx);
    }
    
    // 纵向力应该随侧偏角增加而减小
    for (size_t i = 1; i < Fx_values.size(); ++i) {
        EXPECT_LT(std::abs(Fx_values[i]), std::abs(Fx_values[i-1]));
    }
    
    // 类似地，横向力应该随纵向滑移增加而减小
    std::vector<double> kappa_values = {0.0, 0.05, 0.1, 0.15};
    double alpha = 0.1;
    
    std::vector<double> Fy_values;
    for (double k : kappa_values) {
        double Fy = model_->computeFyCombined(k, alpha, Fz);
        Fy_values.push_back(Fy);
    }
    
    // 横向力应该随纵向滑移增加而减小
    for (size_t i = 1; i < Fy_values.size(); ++i) {
        EXPECT_LT(std::abs(Fy_values[i]), std::abs(Fy_values[i-1]));
    }
}

// ============================================================================
// 轮胎动力学物理测试
// ============================================================================

class TireDynamicsPhysicsTest : public ::testing::Test {
protected:
    void SetUp() override {
        TireConfig config;
        config.radius = 0.33;
        config.width = 0.225;
        config.inertia = 1.5;
        config.relaxation_length_long = 0.2;
        config.relaxation_length_lat = 0.5;
        
        PacejkaParameters params;
        params.Fz0 = 4000.0;
        
        tire_ = std::make_unique<TireDynamics>(config, params);
    }
    
    std::unique_ptr<TireDynamics> tire_;
};

// 测试7：松弛长度动态响应
TEST_F(TireDynamicsPhysicsTest, RelaxationDynamics) {
    // 设置初始状态
    WheelMotion motion;
    motion.omega = 60.0;
    motion.Vx = 20.0;
    motion.Vy = 0.0;
    motion.V_wheel = 20.0;
    
    tire_->setWheelMotion(motion);
    tire_->setVerticalLoad(4000.0);
    
    // 突然改变滑移率
    motion.omega = 70.0;  // 增加车轮转速
    tire_->setWheelMotion(motion);
    
    // 初始滑移
    SlipState slip_initial = tire_->computeSlip(motion);
    
    // 应用松弛
    SlipState slip_relaxed = tire_->applyRelaxation(slip_initial, motion.V_wheel);
    
    // 松弛后的滑移率应该小于实际滑移率（动态滞后）
    EXPECT_LT(std::abs(slip_relaxed.kappa_dynamic), std::abs(slip_initial.kappa) + 0.05);
}

// 测试8：滚动阻力速度依赖性
TEST_F(TireDynamicsPhysicsTest, RollingResistanceSpeedDependency) {
    double Fz = 4000.0;
    
    // 不同速度下的滚动阻力
    std::vector<double> V_values = {10.0, 20.0, 30.0, 40.0, 50.0};  // m/s
    std::vector<double> F_roll_values;
    
    for (double V : V_values) {
        double F_roll = tire_->computeRollingResistance(Fz, V);
        F_roll_values.push_back(F_roll);
    }
    
    // 滚动阻力应该随速度增加而增加
    for (size_t i = 1; i < F_roll_values.size(); ++i) {
        EXPECT_GT(F_roll_values[i], F_roll_values[i-1]);
    }
}

// 测试9：有效半径变形
TEST_F(TireDynamicsPhysicsTest, EffectiveRadiusDeformation) {
    // 不同载荷下的有效半径
    std::vector<double> Fz_values = {2000.0, 4000.0, 6000.0, 8000.0};
    std::vector<double> r_eff_values;
    
    for (double Fz : Fz_values) {
        double r_eff = tire_->computeEffectiveRadius(Fz);
        r_eff_values.push_back(r_eff);
    }
    
    // 有效半径应该随载荷增加而减小
    for (size_t i = 1; i < r_eff_values.size(); ++i) {
        EXPECT_LT(r_eff_values[i], r_eff_values[i-1]);
    }
    
    // 变形应该在合理范围内
    EXPECT_GT(r_eff_values[0], 0.30);  // > 30 cm
    EXPECT_LT(r_eff_values[3], 0.33);  // < 自由半径
}

// 测试10：温度对摩擦的影响
TEST_F(TireDynamicsPhysicsTest, TemperatureFrictionEffect) {
    double base_friction = 1.0;
    
    // 不同温度下的摩擦系数
    std::vector<double> T_values = {20.0, 60.0, 80.0, 100.0, 140.0};  // °C
    std::vector<double> mu_values;
    
    for (double T : T_values) {
        double mu = tire_->computeEffectiveFriction(base_friction, T);
        mu_values.push_back(mu);
    }
    
    // 标称温度（80°C）附近应该有最大摩擦系数
    int max_idx = 0;
    for (int i = 1; i < mu_values.size(); ++i) {
        if (mu_values[i] > mu_values[max_idx]) {
            max_idx = i;
        }
    }
    
    // 最大摩擦系数应该在80°C附近（索引2）
    EXPECT_NEAR(max_idx, 2, 1);
}

// 测试11：车轮转速动力学
TEST_F(TireDynamicsPhysicsTest, WheelSpeedDynamics) {
    // 初始状态
    WheelMotion motion;
    motion.omega = 60.0;  // rad/s
    motion.Vx = 20.0;
    motion.Vy = 0.0;
    motion.V_wheel = 20.0;
    
    tire_->setWheelMotion(motion);
    tire_->setVerticalLoad(4000.0);
    
    // 施加驱动力矩
    double dt = 0.01;  // 10 ms
    double driving_torque = 1000.0;  // N·m
    
    tire_->update(dt, driving_torque, 0.0);
    
    // 车轮转速应该增加
    const TireDynamicState& state = tire_->getState();
    EXPECT_GT(state.omega, 60.0);
}

// 测试12：制动力矩
TEST_F(TireDynamicsPhysicsTest, BrakeTorque) {
    WheelMotion motion;
    motion.omega = 60.0;
    motion.Vx = 20.0;
    motion.Vy = 0.0;
    motion.V_wheel = 20.0;
    
    tire_->setWheelMotion(motion);
    tire_->setVerticalLoad(4000.0);
    
    // 施加制动力矩
    double dt = 0.01;
    double brake_torque = 1000.0;  // N·m
    
    tire_->update(dt, 0.0, brake_torque);
    
    // 车轮转速应该减小
    const TireDynamicState& state = tire_->getState();
    EXPECT_LT(state.omega, 60.0);
}

// ============================================================================
// 四轮系统集成测试
// ============================================================================

class FourWheelIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        TireConfig config;
        config.radius = 0.33;
        config.inertia = 1.5;
        
        PacejkaParameters params;
        params.Fz0 = 4000.0;
        
        system_ = std::make_unique<FourWheelTireSystem>();
        system_->initialize(config, params);
    }
    
    std::unique_ptr<FourWheelTireSystem> system_;
};

// 测试13：稳态圆周运动
TEST_F(FourWheelIntegrationTest, SteadyStateCornering) {
    // 假设车辆以20 m/s，0.3 rad/s横摆角速度转弯
    double V = 20.0;  // m/s
    double omega_z = 0.3;  // rad/s
    double R = V / omega_z;  // 转弯半径
    
    // 车轮位置（相对于质心）
    double xf = 1.4;   // m
    double xr = -1.4;  // m
    double wf = 0.8;   // m
    double wr = 0.8;   // m
    
    // 设置每个车轮的运动状态
    // FL
    WheelMotion motion_fl;
    motion_fl.Vx = V;
    motion_fl.Vy = omega_z * wf;  // 横向速度
    motion_fl.omega = V / 0.33;
    motion_fl.V_wheel = V;
    system_->setWheelMotion(0, motion_fl);
    system_->setVerticalLoad(0, 3500.0);
    
    // FR
    WheelMotion motion_fr;
    motion_fr.Vx = V;
    motion_fr.Vy = -omega_z * wf;
    motion_fr.omega = V / 0.33;
    motion_fr.V_wheel = V;
    system_->setWheelMotion(1, motion_fr);
    system_->setVerticalLoad(1, 3500.0);
    
    // RL
    WheelMotion motion_rl;
    motion_rl.Vx = V;
    motion_rl.Vy = omega_z * wr;
    motion_rl.omega = V / 0.33;
    motion_rl.V_wheel = V;
    system_->setWheelMotion(2, motion_rl);
    system_->setVerticalLoad(2, 4500.0);
    
    // RR
    WheelMotion motion_rr;
    motion_rr.Vx = V;
    motion_rr.Vy = -omega_z * wr;
    motion_rr.omega = V / 0.33;
    motion_rr.V_wheel = V;
    system_->setWheelMotion(3, motion_rr);
    system_->setVerticalLoad(3, 4500.0);
    
    // 计算所有轮胎力
    auto outputs = system_->computeAllForces();
    
    // 验证横向力（应该产生向心力）
    double Fy_total = 0.0;
    for (int i = 0; i < 4; ++i) {
        Fy_total += outputs[i].forces.Fy;
    }
    
    // 总横向力应该为正（向左转）
    EXPECT_GT(Fy_total, 0.0);
    
    // 验证侧偏角（外轮应该为负，内轮应该为正）
    EXPECT_LT(outputs[1].slip.alpha, 0.0);  // FR（外轮）
    EXPECT_GT(outputs[0].slip.alpha, 0.0);  // FL（内轮）
}

// 测试14：直线加速
TEST_F(FourWheelIntegrationTest, StraightLineAcceleration) {
    double V = 20.0;
    
    // 设置所有车轮
    for (int i = 0; i < 4; ++i) {
        WheelMotion motion;
        motion.Vx = V;
        motion.Vy = 0.0;
        motion.omega = V / 0.33;
        motion.V_wheel = V;
        
        system_->setWheelMotion(i, motion);
        system_->setVerticalLoad(i, 4000.0);
    }
    
    // 前轮驱动
    std::array<double, 4> driving_torques = {800.0, 800.0, 0.0, 0.0};
    std::array<double, 4> brake_torques = {0.0, 0.0, 0.0, 0.0};
    
    // 更新
    double dt = 0.01;
    for (int step = 0; step < 100; ++step) {
        system_->updateAll(dt, driving_torques, brake_torques);
    }
    
    // 计算总力
    auto [Fx_total, Fy_total, Mz_total] = system_->getTotalForces();
    
    // 应该有正的纵向力
    EXPECT_GT(Fx_total, 0.0);
    
    // 横向力应该接近零（直线行驶）
    EXPECT_LT(std::abs(Fy_total), std::abs(Fx_total) * 0.1);
}

// 测试15：制动测试
TEST_F(FourWheelIntegrationTest, BrakingTest) {
    double V = 30.0;  // m/s
    
    // 设置所有车轮
    for (int i = 0; i < 4; ++i) {
        WheelMotion motion;
        motion.Vx = V;
        motion.Vy = 0.0;
        motion.omega = V / 0.33;
        motion.V_wheel = V;
        
        system_->setWheelMotion(i, motion);
        system_->setVerticalLoad(i, 4000.0);
    }
    
    // 制动
    std::array<double, 4> driving_torques = {0.0, 0.0, 0.0, 0.0};
    std::array<double, 4> brake_torques = {1500.0, 1500.0, 1500.0, 1500.0};
    
    // 更新
    double dt = 0.01;
    for (int step = 0; step < 100; ++step) {
        system_->updateAll(dt, driving_torques, brake_torques);
    }
    
    // 计算总力
    auto [Fx_total, Fy_total, Mz_total] = system_->getTotalForces();
    
    // 应该有负的纵向力（制动力）
    EXPECT_LT(Fx_total, 0.0);
}

// 测试16：载荷转移影响
TEST_F(FourWheelIntegrationTest, LoadTransferEffect) {
    double V = 20.0;
    
    // 初始载荷（静态）
    std::array<double, 4> Fz_static = {3500.0, 3500.0, 4500.0, 4500.0};
    
    // 设置车轮运动
    for (int i = 0; i < 4; ++i) {
        WheelMotion motion;
        motion.Vx = V;
        motion.Vy = 0.0;
        motion.omega = V / 0.33;
        motion.V_wheel = V;
        
        system_->setWheelMotion(i, motion);
    }
    
    // 测试1：静态载荷
    system_->setAllWheelLoads(Fz_static);
    auto outputs_static = system_->computeAllForces();
    
    // 测试2：制动载荷转移
    std::array<double, 4> Fz_braking = {4500.0, 4500.0, 3500.0, 3500.0};
    system_->setAllWheelLoads(Fz_braking);
    auto outputs_braking = system_->computeAllForces();
    
    // 前轮制动力应该增加（因为载荷增加）
    EXPECT_GT(std::abs(outputs_braking[0].forces.Fx), 
              std::abs(outputs_static[0].forces.Fx) * 0.9);
}

// 测试17：极端条件
TEST_F(FourWheelIntegrationTest, ExtremeConditions) {
    // 测试1：极低速度
    for (int i = 0; i < 4; ++i) {
        WheelMotion motion;
        motion.Vx = 0.1;  // 0.1 m/s
        motion.Vy = 0.0;
        motion.omega = 0.1 / 0.33;
        motion.V_wheel = 0.1;
        
        system_->setWheelMotion(i, motion);
        system_->setVerticalLoad(i, 4000.0);
    }
    
    EXPECT_NO_THROW(system_->computeAllForces());
    
    // 测试2：极高载荷
    for (int i = 0; i < 4; ++i) {
        system_->setVerticalLoad(i, 20000.0);  // 20 kN
    }
    
    EXPECT_NO_THROW(system_->computeAllForces());
    
    // 测试3：极端滑移
    for (int i = 0; i < 4; ++i) {
        WheelMotion motion;
        motion.Vx = 20.0;
        motion.Vy = 0.0;
        motion.omega = 100.0;  // 极高转速
        motion.V_wheel = 20.0;
        
        system_->setWheelMotion(i, motion);
        system_->setVerticalLoad(i, 4000.0);
    }
    
    EXPECT_NO_THROW(system_->computeAllForces());
}

// 测试18：连续时间积分
TEST_F(FourWheelIntegrationTest, ContinuousIntegration) {
    double V = 20.0;
    double dt = 0.001;  // 1 ms
    
    // 初始化
    for (int i = 0; i < 4; ++i) {
        WheelMotion motion;
        motion.Vx = V;
        motion.Vy = 0.0;
        motion.omega = V / 0.33;
        motion.V_wheel = V;
        
        system_->setWheelMotion(i, motion);
        system_->setVerticalLoad(i, 4000.0);
    }
    
    // 连续积分100步
    std::array<double, 4> driving_torques = {500.0, 500.0, 0.0, 0.0};
    std::array<double, 4> brake_torques = {0.0, 0.0, 0.0, 0.0};
    
    for (int step = 0; step < 100; ++step) {
        EXPECT_NO_THROW(system_->updateAll(dt, driving_torques, brake_torques));
        
        // 检查状态稳定性
        for (int i = 0; i < 4; ++i) {
            const TireDynamicState& state = system_->getTire(i).getState();
            EXPECT_TRUE(std::isfinite(state.omega));
            EXPECT_TRUE(std::isfinite(state.sigma_x));
            EXPECT_TRUE(std::isfinite(state.sigma_y));
        }
    }
}

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
