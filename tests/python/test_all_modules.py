#!/usr/bin/env python3
"""
测试脚本 - 验证所有Python模块是否正确构建
"""

import sys
import os

# 添加构建目录到Python路径
build_dir = os.path.join(os.path.dirname(__file__), '..', 'build', 'python_modules')
sys.path.insert(0, os.path.abspath(build_dir))

def test_powertrain():
    """测试动力系统模块"""
    print("=" * 60)
    print("测试 Powertrain 模块")
    print("=" * 60)
    
    try:
        import powertrain
        
        # 创建电机
        motor = powertrain.Motor(max_torque=300.0, max_speed=15000.0)
        motor.set_torque_request(100.0)
        motor.update(0.001)
        print(f"✓ Motor创建成功")
        print(f"  当前扭矩: {motor.get_torque()} Nm")
        print(f"  当前转速: {motor.get_speed()} rpm")
        
        # 创建电池
        battery = powertrain.Battery(capacity=60.0, max_power=150.0)
        battery.update(dt=0.001, power_demand=50.0)
        print(f"✓ Battery创建成功")
        print(f"  SOC: {battery.get_soc():.2f}%")
        print(f"  温度: {battery.get_temperature()} °C")
        
        # 创建变速箱
        transmission = powertrain.Transmission(ratio=9.0, efficiency=0.98)
        output_torque = transmission.get_output_torque(100.0)
        print(f"✓ Transmission创建成功")
        print(f"  输出扭矩: {output_torque} Nm")
        
        print("\n✅ Powertrain模块测试通过\n")
        return True
        
    except Exception as e:
        print(f"\n❌ Powertrain模块测试失败: {e}\n")
        return False

def test_chassis():
    """测试底盘系统模块"""
    print("=" * 60)
    print("测试 Chassis 模块")
    print("=" * 60)
    
    try:
        import chassis
        
        # 创建悬架
        suspension = chassis.Suspension(stiffness=30000.0, damping=3000.0)
        suspension.update(dt=0.001, road_displacement=0.01)
        print(f"✓ Suspension创建成功")
        print(f"  位移: {suspension.get_displacement():.4f} m")
        print(f"  速度: {suspension.get_velocity():.4f} m/s")
        
        # 创建转向系统
        steering = chassis.Steering(ratio=15.0, max_angle=0.6)
        steering.set_steering_wheel_angle(90.0)
        print(f"✓ Steering创建成功")
        print(f"  车轮转角: {steering.get_wheel_angle():.4f} rad")
        
        # 创建制动系统
        braking = chassis.Braking(max_torque=5000.0)
        braking.set_brake_pressure(0.5)
        print(f"✓ Braking创建成功")
        print(f"  制动力矩: {braking.get_brake_torque():.2f} Nm")
        
        print("\n✅ Chassis模块测试通过\n")
        return True
        
    except Exception as e:
        print(f"\n❌ Chassis模块测试失败: {e}\n")
        return False

def test_dynamics():
    """测试车辆动力学模块"""
    print("=" * 60)
    print("测试 Dynamics 模块")
    print("=" * 60)
    
    try:
        import dynamics
        import numpy as np
        
        # 创建状态空间模型
        state_space = dynamics.StateSpace(state_dim=2, input_dim=1, output_dim=1)
        A = np.array([[0.0, 1.0], [-1.0, -0.5]])
        B = np.array([[0.0], [1.0]])
        C = np.array([[1.0, 0.0]])
        D = np.array([[0.0]])
        state_space.set_A(A)
        state_space.set_B(B)
        state_space.set_C(C)
        state_space.set_D(D)
        print(f"✓ StateSpace创建成功")
        
        # 创建RK4积分器
        rk4 = dynamics.RK4Integrator()
        print(f"✓ RK4Integrator创建成功")
        
        # 创建车辆动力学模型
        vehicle = dynamics.VehicleDynamics(mass=1500.0, inertia_z=2500.0)
        initial_state = np.zeros(6)
        vehicle.set_state(initial_state)
        vehicle.update(dt=0.002, steering_angle=0.0, traction_force=1000.0)
        state = vehicle.get_state()
        print(f"✓ VehicleDynamics创建成功")
        print(f"  初始状态: {initial_state}")
        print(f"  当前状态: {state}")
        
        print("\n✅ Dynamics模块测试通过\n")
        return True
        
    except Exception as e:
        print(f"\n❌ Dynamics模块测试失败: {e}\n")
        import traceback
        traceback.print_exc()
        return False

def test_tire():
    """测试轮胎模型模块"""
    print("=" * 60)
    print("测试 Tire 模块")
    print("=" * 60)
    
    try:
        import tire
        
        # 创建滑移率计算器
        slip_calc = tire.SlipCalculator()
        slip = slip_calc.calculate_longitudinal_slip(10.0, 9.0)
        alpha = slip_calc.calculate_slip_angle(1.0, 10.0)
        print(f"✓ SlipCalculator创建成功")
        print(f"  纵向滑移率: {slip:.4f}")
        print(f"  侧偏角: {alpha:.4f} rad")
        
        # 创建Pacejka轮胎模型
        pacejka = tire.PacejkaTire(B=10.0, C=1.9, D=1.0, E=0.97)
        fx = pacejka.calculate_longitudinal_force(0.1)
        fy = pacejka.calculate_lateral_force(0.05)
        print(f"✓ PacejkaTire创建成功")
        print(f"  纵向力: {fx:.4f}")
        print(f"  侧向力: {fy:.4f}")
        
        print("\n✅ Tire模块测试通过\n")
        return True
        
    except Exception as e:
        print(f"\n❌ Tire模块测试失败: {e}\n")
        return False

def test_scheduler():
    """测试实时调度器模块"""
    print("=" * 60)
    print("测试 Scheduler 模块")
    print("=" * 60)
    
    try:
        import scheduler
        
        # 创建任务管理器
        task_mgr = scheduler.TaskManager()
        task_mgr.add_task("task1", 100.0, lambda dt: None)
        print(f"✓ TaskManager创建成功")
        print(f"  任务数量: {task_mgr.get_task_count()}")
        
        # 创建监控器
        monitor = scheduler.Monitor()
        monitor.record_frame(0.001)
        print(f"✓ Monitor创建成功")
        print(f"  平均帧时间: {monitor.get_average_frame_time():.6f} s")
        print(f"  FPS: {monitor.get_fps():.2f}")
        
        # 创建实时调度器
        sched = scheduler.RealtimeScheduler(frequency=500.0)
        sched.add_task("dynamics", 500.0, lambda dt: None)
        sched.start()
        sched.step()
        print(f"✓ RealtimeScheduler创建成功")
        print(f"  频率: {sched.get_frequency()} Hz")
        print(f"  时间步长: {sched.get_dt()*1000:.2f} ms")
        print(f"  运行状态: {sched.is_running()}")
        
        print("\n✅ Scheduler模块测试通过\n")
        return True
        
    except Exception as e:
        print(f"\n❌ Scheduler模块测试失败: {e}\n")
        import traceback
        traceback.print_exc()
        return False

def main():
    """主测试函数"""
    print("\n" + "=" * 60)
    print("EV Dynamics Simulation - 模块测试")
    print("=" * 60 + "\n")
    
    results = {
        "powertrain": test_powertrain(),
        "chassis": test_chassis(),
        "dynamics": test_dynamics(),
        "tire": test_tire(),
        "scheduler": test_scheduler(),
    }
    
    print("\n" + "=" * 60)
    print("测试总结")
    print("=" * 60)
    
    passed = sum(1 for v in results.values() if v)
    total = len(results)
    
    for module, result in results.items():
        status = "✅ 通过" if result else "❌ 失败"
        print(f"{module:15s}: {status}")
    
    print(f"\n总计: {passed}/{total} 模块通过测试")
    
    if passed == total:
        print("\n🎉 所有模块测试通过！")
        return 0
    else:
        print("\n⚠️  部分模块测试失败，请检查构建")
        return 1

if __name__ == "__main__":
    sys.exit(main())
