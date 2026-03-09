/**
 * @file pacejka_model.h
 * @brief Pacejka Magic Formula 6.2 轮胎模型头文件
 * 
 * 实现行业标准Pacejka MF6.2轮胎模型，包括：
 * - 纵向力 Fx（驱动/制动力）
 * - 横向力 Fy（侧向力）
 * - 回正力矩 Mz
 * - 组合滑移
 * - 参数拟合
 * 
 * @author TireAgent
 * @date 2026-03-08
 */

#pragma once

#include <array>
#include <vector>
#include <string>
#include <Eigen/Dense>

namespace ev_simulation {
namespace tire {

using Eigen::VectorXd;
using Eigen::MatrixXd;

/**
 * @brief Pacejka MF6.2 轮胎参数
 * 
 * 包含所有魔术公式系数和物理参数
 */
struct PacejkaParameters {
    // === 纵向力 Fx 参数 ===
    double PCx{1.6};        ///< 形状因子 Cx
    double PDx{1.0};        ///< 峰值因子 Dx (摩擦系数)
    double PDx1{1.0};       ///< 载荷相关系数
    double PDx2{0.0};       ///< 载荷相关系数
    double PEx{-1.0};       ///< 曲率因子 Ex
    double PEx1{0.0};       ///< 载荷相关系数
    double PEx2{0.0};       ///< 载荷相关系数
    double PKx{20.0};       ///< 纵向滑移刚度 Kx (kN)
    double PKx1{20.0};      ///< 载荷相关系数
    double PKx2{0.0};       ///< 载荷相关系数
    double PHx{0.0};        ///< 水平偏移 Hx
    double PHx1{0.0};       ///< 载荷相关系数
    double PVx{0.0};        ///< 垂直偏移 Vx
    double PVx1{0.0};       ///< 载荷相关系数
    
    // === 横向力 Fy 参数 ===
    double PCy{1.4};        ///< 形状因子 Cy
    double PDy{1.0};        ///< 峰值因子 Dy (摩擦系数)
    double PDy1{1.0};       ///< 载荷相关系数
    double PDy2{0.0};       ///< 载荷相关系数
    double PDy3{0.0};       ///< 载荷相关系数
    double PEy{-1.0};       ///< 曲率因子 Ey
    double PEy1{0.0};       ///< 载荷相关系数
    double PEy2{0.0};       ///< 载荷相关系数
    double PEy3{0.0};       ///< 载荷相关系数
    double PEy4{0.0};       ///< 载荷相关系数
    double PKy{20.0};       ///< 侧偏刚度 Ky (kN/rad)
    double PKy1{20.0};      ///< 载荷相关系数
    double PKy2{0.0};       ///< 载荷相关系数
    double PKy3{0.0};       ///< 载荷相关系数
    double PHy{0.0};        ///< 水平偏移 Hy
    double PHy1{0.0};       ///< 载荷相关系数
    double PHy2{0.0};       ///< 载荷相关系数
    double PVy{0.0};        ///< 垂直偏移 Vy
    double PVy1{0.0};       ///< 载荷相关系数
    double PVy2{0.0};       ///< 载荷相关系数
    double PVy3{0.0};       ///< 载荷相关系数
    double PVy4{0.0};       ///< 载荷相关系数
    
    // === 回正力矩 Mz 参数 ===
    double PCz{2.3};        ///< 形状因子 Cz
    double PDz{0.15};       ///< 峰值因子 Dz (m)
    double PDz1{0.15};      ///< 载荷相关系数
    double PDz2{0.0};       ///< 载荷相关系数
    double PDz3{0.0};       ///< 载荷相关系数
    double PDz4{0.0};       ///< 载荷相关系数
    double PEz{-2.0};       ///< 曲率因子 Ez
    double PEz1{-2.0};      ///< 载荷相关系数
    double PEz2{0.0};       ///< 载荷相关系数
    double PEz3{0.0};       ///< 载荷相关系数
    double PEz4{0.0};       ///< 载荷相关系数
    double PKz{2.0};        ///< 回正刚度 Kz (m/rad)
    double PKz1{2.0};       ///< 载荷相关系数
    double PKz2{0.0};       ///< 载荷相关系数
    double PKz3{0.0};       ///< 载荷相关系数
    double PHz{0.0};        ///< 水平偏移 Hz
    double PHz1{0.0};       ///< 载荷相关系数
    double PHz2{0.0};       ///< 载荷相关系数
    double PHz3{0.0};       ///< 载荷相关系数
    double PVz{0.0};        ///< 垂直偏移 Vz
    double PVz1{0.0};       ///< 载荷相关系数
    double PVz2{0.0};       ///< 载荷相关系数
    double PVz3{0.0};       ///< 载荷相关系数
    
    // === 组合滑移参数 ===
    double rBx1{10.0};      ///< 组合滑移参数
    double rBx2{0.0};       ///< 组合滑移参数
    double rCx{1.0};        ///< 组合滑移参数
    double rEx{0.0};        ///< 组合滑移参数
    double rHx{0.0};        ///< 组合滑移参数
    
    double rBy1{10.0};      ///< 组合滑移参数
    double rBy2{0.0};       ///< 组合滑移参数
    double rBy3{0.0};       ///< 组合滑移参数
    double rCy{1.0};        ///< 组合滑移参数
    double rEy{0.0};        ///< 组合滑移参数
    double rHy{0.0};        ///< 组合滑移参数
    
    // === 物理参数 ===
    double Fz0{4000.0};     ///< 额定垂直载荷 (N)
    double tire_radius{0.33}; ///< 轮胎半径 (m)
    double tire_width{0.225}; ///< 轮胎宽度 (m)
    
    /**
     * @brief 验证参数有效性
     * @throws std::invalid_argument 参数无效
     */
    void validate() const;
    
    /**
     * @brief 从文件加载参数
     * @param filename 参数文件路径
     * @return PacejkaParameters
     */
    static PacejkaParameters loadFromFile(const std::string& filename);
    
    /**
     * @brief 保存参数到文件
     * @param filename 参数文件路径
     */
    void saveToFile(const std::string& filename) const;
};

/**
 * @brief 轮胎状态
 */
struct TireState {
    double kappa{0.0};      ///< 纵向滑移率 (-1 to 1)
    double alpha{0.0};      ///< 侧偏角 (rad)
    double gamma{0.0};      ///< 外倾角 (rad)
    double Fz{4000.0};      ///< 垂直载荷 (N)
    double V_wheel{20.0};   ///< 车轮速度 (m/s)
};

/**
 * @brief 轮胎力输出
 */
struct TireForces {
    double Fx{0.0};         ///< 纵向力 (N)
    double Fy{0.0};         ///< 侧向力 (N)
    double Fz{0.0};         ///< 垂向力 (N) (输入)
    double Mx{0.0};         ///< 翻转力矩 (N·m)
    double My{0.0};         ///< 滚动阻力矩 (N·m)
    double Mz{0.0};         ///< 回正力矩 (N·m)
};

/**
 * @brief Pacejka Magic Formula 6.2 轮胎模型
 * 
 * 实现完整的MF6.2模型，包括：
 * - 纯纵向滑移
 * - 纯侧偏
 * - 组合滑移
 * - 外倾影响
 * - 参数拟合
 */
class PacejkaModel {
public:
    /**
     * @brief 构造函数
     */
    PacejkaModel();
    
    /**
     * @brief 带参数构造函数
     * @param params Pacejka参数
     */
    explicit PacejkaModel(const PacejkaParameters& params);
    
    /**
     * @brief 析构函数
     */
    ~PacejkaModel();
    
    /**
     * @brief 设置Pacejka参数
     * @param params 参数结构
     */
    void setParameters(const PacejkaParameters& params);
    
    /**
     * @brief 获取Pacejka参数
     * @return 参数结构
     */
    const PacejkaParameters& getParameters() const { return params_; }
    
    /**
     * @brief 计算轮胎力（主接口）
     * @param state 轮胎状态
     * @return 轮胎力
     */
    TireForces computeForces(const TireState& state);
    
    /**
     * @brief 计算纵向力 Fx（纯纵向滑移）
     * @param kappa 纵向滑移率
     * @param Fz 垂直载荷 (N)
     * @return 纵向力 (N)
     */
    double computeFx(double kappa, double Fz);
    
    /**
     * @brief 计算横向力 Fy（纯侧偏）
     * @param alpha 侧偏角 (rad)
     * @param Fz 垂直载荷 (N)
     * @param gamma 外倾角 (rad)
     * @return 横向力 (N)
     */
    double computeFy(double alpha, double Fz, double gamma = 0.0);
    
    /**
     * @brief 计算回正力矩 Mz
     * @param alpha 侧偏角 (rad)
     * @param Fz 垂直载荷 (N)
     * @param gamma 外倾角 (rad)
     * @return 回正力矩 (N·m)
     */
    double computeMz(double alpha, double Fz, double gamma = 0.0);
    
    /**
     * @brief 计算组合滑移纵向力
     * @param kappa 纵向滑移率
     * @param alpha 侧偏角 (rad)
     * @param Fz 垂直载荷 (N)
     * @return 纵向力 (N)
     */
    double computeFxCombined(double kappa, double alpha, double Fz);
    
    /**
     * @brief 计算组合滑移横向力
     * @param kappa 纵向滑移率
     * @param alpha 侧偏角 (rad)
     * @param Fz 垂直载荷 (N)
     * @param gamma 外倾角 (rad)
     * @return 横向力 (N)
     */
    double computeFyCombined(double kappa, double alpha, double Fz, double gamma = 0.0);
    
    /**
     * @brief 参数拟合（最小二乘法）
     * @param slip_data 滑移率/侧偏角数据
     * @param force_data 力数据
     * @param force_type 力类型 ("Fx", "Fy", "Mz")
     * @return 拟合的参数向量
     */
    VectorXd fitParameters(const VectorXd& slip_data, 
                          const VectorXd& force_data,
                          const std::string& force_type);
    
    /**
     * @brief 验证模型精度
     * @param test_data 测试数据
     * @return RMSE误差
     */
    double validateModel(const std::vector<TireState>& test_states,
                        const std::vector<TireForces>& test_forces);
    
    /**
     * @brief 获取轮胎特性曲线数据
     * @param Fz 垂直载荷 (N)
     * @param curve_type 曲线类型 ("Fx-kappa", "Fy-alpha", "Mz-alpha")
     * @param range_min 范围最小值
     * @param range_max 范围最大值
     * @param num_points 点数
     * @return (x, y) 数据点
     */
    std::vector<std::pair<double, double>> getCharacteristicCurve(
        double Fz,
        const std::string& curve_type,
        double range_min,
        double range_max,
        int num_points = 100
    );

private:
    /**
     * @brief 归一化垂直载荷
     * @param Fz 垂直载荷 (N)
     * @return 归一化载荷 dfz
     */
    double normalizeLoad(double Fz) const;
    
    /**
     * @brief 计算纵向力参数
     * @param dfz 归一化载荷
     * @return (C, D, E, K, H, V) 参数
     */
    std::tuple<double, double, double, double, double, double> 
    computeFxParams(double dfz);
    
    /**
     * @brief 计算横向力参数
     * @param dfz 归一化载荷
     * @param gamma 外倾角 (rad)
     * @return (C, D, E, K, H, V) 参数
     */
    std::tuple<double, double, double, double, double, double> 
    computeFyParams(double dfz, double gamma);
    
    /**
     * @brief 计算回正力矩参数
     * @param dfz 归一化载荷
     * @param gamma 外倾角 (rad)
     * @return (C, D, E, K, H, V) 参数
     */
    std::tuple<double, double, double, double, double, double> 
    computeMzParams(double dfz, double gamma);
    
    /**
     * @brief 魔术公式核心函数
     * @param x 输入变量
     * @param C 形状因子
     * @param D 峰值因子
     * @param E 曲率因子
     * @param K 刚度
     * @param H 水平偏移
     * @param V 垂直偏移
     * @return 力/力矩
     */
    double magicFormula(double x, double C, double D, double E, 
                       double K, double H, double V) const;
    
    /**
     * @brief 计算组合滑移权重因子
     * @param kappa 纵向滑移率
     * @param alpha 侧偏角 (rad)
     * @param type 权重类型 ("Fx", "Fy")
     * @return 权重因子
     */
    double computeCombinationWeight(double kappa, double alpha, 
                                    const std::string& type);
    
    PacejkaParameters params_;      ///< Pacejka参数
    bool initialized_;               ///< 初始化标志
};

} // namespace tire
} // namespace ev_simulation
