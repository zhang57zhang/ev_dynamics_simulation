/**
 * @file test_tire.cpp
 * @brief 轮胎模型单元测试
 * 
 * 测试Pacejka MF6.2模型和轮胎动力学
 * 
 * @author TireAgent
 * @date 2026-03-08
 */

#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "../src/tire/pacejka_model.h"
#include "../src/tire/tire_dynamics.h"

using namespace ev_simulation::tire;

// ============================================================================
// PacejkaModel 测试
// ============================================================================

class PacejkaModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 使用默认参数
        model_ = std::make_unique<PacejkaModel>();
    }
    
    std::unique_ptr<PacejkaModel> model_;
};

// 测试1：模型初始化
TEST_F(PacejkaModelTest, Initialization) {
    EXPECT_FALSE(model_->isInitialized());
    
    PacejkaParameters params;
    model_->setParameters(params);
    EXPECT_TRUE(model_->isInitialized());
}

// 测试2：纵向力计算（纯纵向滑移）
TEST_F(PacejkaModelTest, ComputeFxPureSlip) {
    PacejkaParameters params;
    params.PCx = 1.6;
    params.PDx = 1.0;
    params.PKx = 100000.0;  // 100 kN
    params.Fz0 = 4000.0;
    
    model_->setParameters(params);
    
    double Fz = 4000.0;  // N
    
    // 测试不同滑移率下的纵向力
    std::vector<double> kappa_values = {-0.1, -0.05, 0.0, 0.05, 0.1};
    
    for (double kappa : kappa_values) {
        double Fx = model_->computeFx(kappa, Fz);
        
        // 纵向力应该在合理范围内
        EXPECT_GT(std::abs(Fx), 0.0) << "Fx should not be zero at kappa=" << kappa;
        EXPECT_LT(std::abs(Fx), Fz * 2.0) << "Fx should not exceed 2 * Fz";
    }
    
    // 零滑移率应该产生零力（忽略偏移）
    double Fx_zero = model_->computeFx(0.0, Fz);
    EXPECT_NEAR(Fx_zero, 0.0, 1000.0) << "Fx should be near zero at kappa=0";
}

// 测试3：横向力计算（纯侧偏）
TEST_F(PacejkaModelTest, ComputeFyPureSlip) {
    PacejkaParameters params;
    params.PCy = 1.4;
    params.PDy = 1.0;
    params.PKy = 80000.0;  // 80 kN/rad
    params.Fz0 = 4000.0;
    
    model_->setParameters(params);
    
    double Fz = 4000.0;  // N
    
    // 测试不同侧偏角下的横向力
    std::vector<double> alpha_values = {-0.2, -0.1, 0.0, 0.1, 0.2};  // rad
    
    for (double alpha : alpha_values) {
        double Fy = model_->computeFy(alpha, Fz);
        
        // 横向力应该在合理范围内
        EXPECT_GT(std::abs(Fy), 0.0) << "Fy should not be zero at alpha=" << alpha;
        EXPECT_LT(std::abs(Fy), Fz * 2.0) << "Fy should not exceed 2 * Fz";
    }
    
    // 零侧偏角应该产生零力（忽略偏移）
    double Fy_zero = model_->computeFy(0.0, Fz);
    EXPECT_NEAR(Fy_zero, 0.0, 1000.0) << "Fy should be near zero at alpha=0";
}

// 测试4：回正力矩计算
TEST_F(PacejkaModelTest, ComputeMz) {
    PacejkaParameters params;
    params.PCz = 2.3;
    params.PDz = 0.15;  // m
    params.PKz = 2.0;   // m/rad
    params.Fz0 = 4000.0;
    
    model_->setParameters(params);
    
    double Fz = 4000.0;  // N
    
    // 测试不同侧偏角下的回正力矩
    std::vector<double> alpha_values = {-0.2, -0.1, 0.0, 0.1, 0.2};
    
    for (double alpha : alpha_values) {
        double Mz = model_->computeMz(alpha, Fz);
        
        // 回正力矩应该在合理范围内
        EXPECT_LT(std::abs(Mz), 1000.0) << "Mz should be reasonable";
    }
    
    // 零侧偏角应该产生零力矩（忽略偏移）
    double Mz_zero = model_->computeMz(0.0, Fz);
    EXPECT_NEAR(Mz_zero, 0.0, 100.0) << "Mz should be near zero at alpha=0";
}

// 测试5：组合滑移
TEST_F(PacejkaModelTest, ComputeCombinedSlip) {
    PacejkaParameters params;
    params.Fz0 = 4000.0;
    
    model_->setParameters(params);
    
    double Fz = 4000.0;
    double kappa = 0.05;
    double alpha = 0.1;  // rad
    
    // 纯纵向力
    double Fx_pure = model_->computeFx(kappa, Fz);
    
    // 组合滑移纵向力
    double Fx_combined = model_->computeFxCombined(kappa, alpha, Fz);
    
    // 组合滑移应该减小纵向力
    EXPECT_LT(std::abs(Fx_combined), std::abs(Fx_pure)) 
        << "Combined slip should reduce Fx";
    
    // 纯横向力
    double Fy_pure = model_->computeFy(alpha, Fz);
    
    // 组合滑移横向力
    double Fy_combined = model_->computeFyCombined(kappa, alpha, Fz);
    
    // 组合滑移应该减小横向力
    EXPECT_LT(std::abs(Fy_combined), std::abs(Fy_pure)) 
        << "Combined slip should reduce Fy";
}

// 测试6：完整轮胎力计算
TEST_F(PacejkaModelTest, ComputeForces) {
    PacejkaParameters params;
    params.Fz0 = 4000.0;
    
    model_->setParameters(params);
    
    TireState state;
    state.kappa = 0.05;
    state.alpha = 0.1;
    state.gamma = 0.0;
    state.Fz = 4000.0;
    state.V_wheel = 20.0;
    
    TireForces forces = model_->computeForces(state);
    
    // 验证所有力都在合理范围内
    EXPECT_LT(std::abs(forces.Fx), 10000.0) << "Fx should be reasonable";
    EXPECT_LT(std::abs(forces.Fy), 10000.0) << "Fy should be reasonable";
    EXPECT_LT(std::abs(forces.Mz), 1000.0) << "Mz should be reasonable";
    EXPECT_EQ(forces.Fz, state.Fz) << "Fz should be preserved";
}

// 测试7：参数验证
TEST_F(PacejkaModelTest, ParameterValidation) {
    PacejkaParameters params;
    
    // 测试非法参数
    params.PCx = -1.0;  // 应该为正
    EXPECT_THROW(params.validate(), std::invalid_argument);
    
    params.PCx = 1.6;
    params.Fz0 = -1000.0;  // 应该为正
    EXPECT_THROW(params.validate(), std::invalid_argument);
    
    params.Fz0 = 4000.0;
    EXPECT_NO_THROW(params.validate());
}

// 测试8：参数文件IO
TEST_F(PacejkaModelTest, ParameterFileIO) {
    PacejkaParameters params;
    params.PCx = 1.7;
    params.PCy = 1.5;
    params.Fz0 = 5000.0;
    
    std::string filename = "test_tire_params.txt";
    
    // 保存参数
    EXPECT_NO_THROW(params.saveToFile(filename));
    
    // 加载参数
    PacejkaParameters loaded_params;
    EXPECT_NO_THROW(loaded_params = PacejkaParameters::loadFromFile(filename));
    
    // 验证参数一致性
    EXPECT_DOUBLE_EQ(loaded_params.PCx, params.PCx);
    EXPECT_DOUBLE_EQ(loaded_params.PCy, params.PCy);
    EXPECT_DOUBLE_EQ(loaded_params.Fz0, params.Fz0);
}

// 测试9：特性曲线生成
TEST_F(PacejkaModelTest, CharacteristicCurve) {
    PacejkaParameters params;
    params.Fz0 = 4000.0;
    
    model_->setParameters(params);
    
    double Fz = 4000.0;
    
    // 生成Fx-kappa曲线
    auto curve_fx = model_->getCharacteristicCurve(Fz, "Fx-kappa", -0.2, 0.2, 50);
    EXPECT_EQ(curve_fx.size(), 50);
    
    // 生成Fy-alpha曲线
    auto curve_fy = model_->getCharacteristicCurve(Fz, "Fy-alpha", -0.3, 0.3, 50);
    EXPECT_EQ(curve_fy.size(), 50);
    
    // 生成Mz-alpha曲线
    auto curve_mz = model_->getCharacteristicCurve(Fz, "Mz-alpha", -0.3, 0.3, 50);
    EXPECT_EQ(curve_mz.size(), 50);
}

// ============================================================================
// TireDynamics 测试
// ============================================================================

class TireDynamicsTest : public ::testing::Test {
protected:
    void SetUp() override {
        TireConfig tire_config;
        tire_config.radius = 0.33;
        tire_config.inertia = 1.5;
        
        PacejkaParameters pacejka_params;
        pacejka_params.Fz0 = 4000.0;
        
        tire_dynamics_ = std::make_unique<TireDynamics>(tire_config, pacejka_params);
    }
    
    std::unique_ptr<TireDynamics> tire_dynamics_;
};

// 测试10：轮胎动力学初始化
TEST_F(TireDynamicsTest, Initialization) {
    EXPECT_TRUE(tire_dynamics_->isInitialized());
    
    // 重置状态
    EXPECT_NO_THROW(tire_dynamics_->reset());
}

// 测试11：滑移率计算
TEST_F(TireDynamicsTest, ComputeSlip) {
    WheelMotion motion;
    motion.omega = 60.0;      // rad/s
    motion.Vx = 20.0;         // m/s
    motion.Vy = 0.0;
    
    tire_dynamics_->setWheelMotion(motion);
    tire_dynamics_->setVerticalLoad(4000.0);
    
    SlipState slip = tire_dynamics_->computeSlip(motion);
    
    // 滑移率应该在[-1, 1]范围内
    EXPECT_GE(slip.kappa, -1.0);
    EXPECT_LE(slip.kappa, 1.0);
    
    // 侧偏角应该在合理范围内
    EXPECT_LT(std::abs(slip.alpha), M_PI / 2);
}

// 测试12：侧偏角计算
TEST_F(TireDynamicsTest, ComputeSlipAngle) {
    WheelMotion motion;
    motion.omega = 60.0;
    motion.Vx = 20.0;
    motion.Vy = 2.0;  // 横向速度
    
    tire_dynamics_->setWheelMotion(motion);
    tire_dynamics_->setVerticalLoad(4000.0);
    
    SlipState slip = tire_dynamics_->computeSlip(motion);
    
    // 侧偏角应该大约为 atan(2/20) = 0.0997 rad
    EXPECT_NEAR(slip.alpha, std::atan2(2.0, 20.0), 0.01);
}

// 测试13：松弛长度模型
TEST_F(TireDynamicsTest, RelaxationLength) {
    WheelMotion motion;
    motion.omega = 60.0;
    motion.Vx = 20.0;
    motion.Vy = 0.0;
    motion.V_wheel = 20.0;
    
    tire_dynamics_->setWheelMotion(motion);
    tire_dynamics_->setVerticalLoad(4000.0);
    
    SlipState slip = tire_dynamics_->computeSlip(motion);
    SlipState relaxed_slip = tire_dynamics_->applyRelaxation(slip, motion.V_wheel);
    
    // 松弛后的滑移率应该与输入滑移率接近（初始状态为0）
    EXPECT_LT(std::abs(relaxed_slip.kappa_dynamic), std::abs(slip.kappa) + 0.1);
}

// 测试14：滚动阻力
TEST_F(TireDynamicsTest, RollingResistance) {
    double Fz = 4000.0;
    double V = 20.0;  // m/s
    
    double F_roll = tire_dynamics_->computeRollingResistance(Fz, V);
    
    // 滚动阻力应该是正的（阻碍运动）
    EXPECT_GT(F_roll, 0.0);
    
    // 滚动阻力应该在合理范围内（通常 < 0.02 * Fz）
    EXPECT_LT(F_roll, 0.05 * Fz);
}

// 测试15：有效半径
TEST_F(TireDynamicsTest, EffectiveRadius) {
    double Fz = 4000.0;
    
    double r_eff = tire_dynamics_->computeEffectiveRadius(Fz);
    
    // 有效半径应该小于自由半径
    EXPECT_LT(r_eff, 0.33);
    
    // 有效半径应该为正
    EXPECT_GT(r_eff, 0.0);
    
    // 有效半径应该在合理范围内
    EXPECT_GT(r_eff, 0.30);
}

// 测试16：温度影响
TEST_F(TireDynamicsTest, TemperatureEffect) {
    double base_friction = 1.0;
    
    // 标称温度下的摩擦系数
    double mu_nominal = tire_dynamics_->computeEffectiveFriction(base_friction, 80.0);
    EXPECT_NEAR(mu_nominal, base_friction, 0.1);
    
    // 高温下的摩擦系数（应该降低）
    double mu_hot = tire_dynamics_->computeEffectiveFriction(base_friction, 120.0);
    EXPECT_LT(mu_hot, mu_nominal);
    
    // 低温下的摩擦系数（应该增加）
    double mu_cold = tire_dynamics_->computeEffectiveFriction(base_friction, 40.0);
    EXPECT_GT(mu_cold, mu_nominal);
}

// 测试17：完整轮胎力计算
TEST_F(TireDynamicsTest, ComputeForces) {
    WheelMotion motion;
    motion.omega = 60.0;
    motion.Vx = 20.0;
    motion.Vy = 0.5;
    motion.V_wheel = 20.0;
    
    tire_dynamics_->setWheelMotion(motion);
    tire_dynamics_->setVerticalLoad(4000.0);
    
    TireDynamicsOutput output = tire_dynamics_->computeForces();
    
    // 验证所有力都在合理范围内
    EXPECT_LT(std::abs(output.forces.Fx), 15000.0);
    EXPECT_LT(std::abs(output.forces.Fy), 15000.0);
    EXPECT_LT(std::abs(output.forces.Mz), 2000.0);
    
    // 验证滑移状态
    EXPECT_GE(output.slip.kappa, -1.0);
    EXPECT_LE(output.slip.kappa, 1.0);
}

// 测试18：时间步进更新
TEST_F(TireDynamicsTest, Update) {
    WheelMotion motion;
    motion.omega = 60.0;
    motion.Vx = 20.0;
    motion.Vy = 0.0;
    motion.V_wheel = 20.0;
    
    tire_dynamics_->setWheelMotion(motion);
    tire_dynamics_->setVerticalLoad(4000.0);
    
    double dt = 0.001;  // 1 ms
    double driving_torque = 500.0;  // N·m
    double brake_torque = 0.0;
    
    // 更新状态
    EXPECT_NO_THROW(tire_dynamics_->update(dt, driving_torque, brake_torque));
    
    // 车轮转速应该增加
    const TireDynamicState& state = tire_dynamics_->getState();
    EXPECT_GT(state.omega, 0.0);
}

// ============================================================================
// FourWheelTireSystem 测试
// ============================================================================

class FourWheelTireSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        TireConfig tire_config;
        tire_config.radius = 0.33;
        tire_config.inertia = 1.5;
        
        PacejkaParameters pacejka_params;
        pacejka_params.Fz0 = 4000.0;
        
        system_ = std::make_unique<FourWheelTireSystem>();
        system_->initialize(tire_config, pacejka_params);
    }
    
    std::unique_ptr<FourWheelTireSystem> system_;
};

// 测试19：四轮系统初始化
TEST_F(FourWheelTireSystemTest, Initialization) {
    EXPECT_TRUE(system_->isInitialized());
    
    EXPECT_NO_THROW(system_->reset());
}

// 测试20：设置车轮载荷
TEST_F(FourWheelTireSystemTest, SetWheelLoads) {
    std::array<double, 4> Fz = {3500.0, 3500.0, 4500.0, 4500.0};
    
    EXPECT_NO_THROW(system_->setAllWheelLoads(Fz));
    
    // 验证单个车轮
    EXPECT_NO_THROW(system_->setVerticalLoad(0, 3000.0));
}

// 测试21：计算所有轮胎力
TEST_F(FourWheelTireSystemTest, ComputeAllForces) {
    // 设置所有车轮的运动状态
    for (int i = 0; i < 4; ++i) {
        WheelMotion motion;
        motion.omega = 60.0;
        motion.Vx = 20.0;
        motion.Vy = 0.0;
        motion.V_wheel = 20.0;
        
        system_->setWheelMotion(i, motion);
        system_->setVerticalLoad(i, 4000.0);
    }
    
    std::array<TireDynamicsOutput, 4> outputs = system_->computeAllForces();
    
    // 验证所有输出
    for (int i = 0; i < 4; ++i) {
        EXPECT_LT(std::abs(outputs[i].forces.Fx), 15000.0);
        EXPECT_LT(std::abs(outputs[i].forces.Fy), 15000.0);
    }
}

// 测试22：总力计算
TEST_F(FourWheelTireSystemTest, GetTotalForces) {
    // 设置所有车轮的运动状态
    for (int i = 0; i < 4; ++i) {
        WheelMotion motion;
        motion.omega = 60.0;
        motion.Vx = 20.0;
        motion.Vy = 0.0;
        motion.V_wheel = 20.0;
        
        system_->setWheelMotion(i, motion);
        system_->setVerticalLoad(i, 4000.0);
    }
    
    auto [Fx_total, Fy_total, Mz_total] = system_->getTotalForces();
    
    // 验证总力
    EXPECT_LT(std::abs(Fx_total), 60000.0);  // 4轮 * 15000 N
    EXPECT_LT(std::abs(Fy_total), 60000.0);
    EXPECT_LT(std::abs(Mz_total), 50000.0);
}

// 测试23：四轮更新
TEST_F(FourWheelTireSystemTest, UpdateAll) {
    // 设置所有车轮的运动状态
    for (int i = 0; i < 4; ++i) {
        WheelMotion motion;
        motion.omega = 60.0;
        motion.Vx = 20.0;
        motion.Vy = 0.0;
        motion.V_wheel = 20.0;
        
        system_->setWheelMotion(i, motion);
        system_->setVerticalLoad(i, 4000.0);
    }
    
    double dt = 0.001;
    std::array<double, 4> driving_torques = {500.0, 500.0, 0.0, 0.0};  // 前驱
    std::array<double, 4> brake_torques = {0.0, 0.0, 0.0, 0.0};
    
    EXPECT_NO_THROW(system_->updateAll(dt, driving_torques, brake_torques));
}

// 测试24：获取单个轮胎
TEST_F(FourWheelTireSystemTest, GetTire) {
    EXPECT_NO_THROW(system_->getTire(0));
    EXPECT_NO_THROW(system_->getTire(3));
    
    // 非法索引
    EXPECT_THROW(system_->getTire(-1), std::invalid_argument);
    EXPECT_THROW(system_->getTire(4), std::invalid_argument);
}

// 测试25：边界条件测试
TEST_F(TireDynamicsTest, BoundaryConditions) {
    WheelMotion motion;
    
    // 零速度
    motion.omega = 0.0;
    motion.Vx = 0.0;
    motion.Vy = 0.0;
    motion.V_wheel = 0.0;
    
    tire_dynamics_->setWheelMotion(motion);
    tire_dynamics_->setVerticalLoad(4000.0);
    
    EXPECT_NO_THROW(tire_dynamics_->computeForces());
    
    // 极端载荷
    tire_dynamics_->setVerticalLoad(100.0);  // 很小
    EXPECT_NO_THROW(tire_dynamics_->computeForces());
    
    tire_dynamics_->setVerticalLoad(20000.0);  // 很大
    EXPECT_NO_THROW(tire_dynamics_->computeForces());
}

// 测试26：参数拟合（简化测试）
TEST_F(PacejkaModelTest, ParameterFitting) {
    PacejkaParameters params;
    params.Fz0 = 4000.0;
    
    model_->setParameters(params);
    
    // 生成模拟数据
    Eigen::VectorXd kappa_data(20);
    Eigen::VectorXd Fx_data(20);
    
    for (int i = 0; i < 20; ++i) {
        kappa_data(i) = -0.1 + i * 0.01;
        Fx_data(i) = model_->computeFx(kappa_data(i), 4000.0);
    }
    
    // 拟合参数（使用添加噪声的数据）
    EXPECT_NO_THROW(model_->fitParameters(kappa_data, Fx_data, "Fx"));
}

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
