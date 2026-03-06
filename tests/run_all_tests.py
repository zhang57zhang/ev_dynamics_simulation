#!/usr/bin/env python3
"""
运行所有实时性验证测试的主脚本

作者：BackendAgent
日期：2026-03-06
"""

import os
import sys
import subprocess
import time
from pathlib import Path
from datetime import datetime


def print_banner(text: str):
    """打印横幅"""
    print("\n" + "=" * 80)
    print(text.center(80))
    print("=" * 80 + "\n")


def run_test(test_script: str, test_name: str, use_sudo: bool = False):
    """
    运行单个测试脚本
    
    Args:
        test_script: 测试脚本路径
        test_name: 测试名称
        use_sudo: 是否使用sudo运行
        
    Returns:
        bool: 测试是否成功
    """
    print(f"\n正在运行: {test_name}")
    print("-" * 80)
    
    start_time = time.time()
    
    try:
        # 构建命令
        if use_sudo and os.name != 'nt':  # Linux/Unix系统
            cmd = ["sudo", sys.executable, test_script]
        else:
            cmd = [sys.executable, test_script]
        
        # 运行测试
        result = subprocess.run(
            cmd,
            capture_output=False,
            text=True,
            timeout=300  # 5分钟超时
        )
        
        elapsed_time = time.time() - start_time
        
        if result.returncode == 0:
            print(f"\n✓ {test_name} 完成 (耗时: {elapsed_time:.2f}秒)")
            return True
        else:
            print(f"\n✗ {test_name} 失败 (返回码: {result.returncode})")
            return False
            
    except subprocess.TimeoutExpired:
        print(f"\n✗ {test_name} 超时")
        return False
    except Exception as e:
        print(f"\n✗ {test_name} 异常: {e}")
        return False


def main():
    """主函数"""
    print_banner("EV动力学仿真系统 - 实时性验证测试套件")
    
    # 获取当前目录
    current_dir = Path(__file__).parent
    
    # 创建测试结果目录
    results_dir = current_dir / "test_results"
    results_dir.mkdir(exist_ok=True)
    
    print(f"测试开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"测试结果目录: {results_dir}")
    
    # 定义测试列表
    tests = [
        {
            "script": "test_realtime_prerequisite.py",
            "name": "PREEMPT_RT实时性前置验证",
            "use_sudo": True  # 需要root权限
        },
        {
            "script": "test_shared_memory_perf.py",
            "name": "共享内存性能测试",
            "use_sudo": False
        },
        {
            "script": "test_message_queue_perf.py",
            "name": "消息队列性能测试",
            "use_sudo": False
        }
    ]
    
    # 运行所有测试
    results = {}
    overall_start = time.time()
    
    for i, test in enumerate(tests, 1):
        print(f"\n[{i}/{len(tests)}] {test['name']}")
        script_path = current_dir / test["script"]
        
        if script_path.exists():
            success = run_test(
                str(script_path),
                test["name"],
                test["use_sudo"]
            )
            results[test["name"]] = success
        else:
            print(f"✗ 脚本不存在: {script_path}")
            results[test["name"]] = False
    
    overall_time = time.time() - overall_start
    
    # 打印测试摘要
    print_banner("测试摘要")
    
    passed = sum(1 for success in results.values() if success)
    failed = len(results) - passed
    
    for test_name, success in results.items():
        status = "✓ 通过" if success else "✗ 失败"
        print(f"{test_name}: {status}")
    
    print(f"\n总计: {len(results)}个测试")
    print(f"通过: {passed}个")
    print(f"失败: {failed}个")
    print(f"总耗时: {overall_time:.2f}秒")
    
    print(f"\n测试完成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"\n详细报告请查看: {results_dir}")
    
    # 返回退出码
    if failed > 0:
        print("\n⚠ 部分测试失败，请检查日志和报告")
        sys.exit(1)
    else:
        print("\n✓ 所有测试通过！")
        sys.exit(0)


if __name__ == "__main__":
    main()
