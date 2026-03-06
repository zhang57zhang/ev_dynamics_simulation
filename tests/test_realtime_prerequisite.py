#!/usr/bin/env python3
"""
PREEMPT_RT实时性前置验证脚本

验证项目：
1. 内核版本和PREEMPT_RT补丁
2. cyclictest延迟测试（目标<200μs）
3. CPU亲和性配置
4. 内存锁定功能

作者：BackendAgent
日期：2026-03-06
版本：1.0
"""

import os
import sys
import subprocess
import time
import psutil
import json
import platform
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Tuple, Optional


class RealtimePrerequisiteTest:
    """PREEMPT_RT实时性前置验证测试类"""
    
    def __init__(self, output_dir: str = "./test_results"):
        """
        初始化测试类
        
        Args:
            output_dir: 测试结果输出目录
        """
        self.results = {
            "test_time": datetime.now().isoformat(),
            "platform": platform.platform(),
            "python_version": platform.python_version(),
            "tests": {},
            "summary": {
                "total": 0,
                "passed": 0,
                "failed": 0,
                "warnings": 0
            }
        }
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # 性能目标
        self.targets = {
            "cyclictest_max_latency_us": 200,  # 最大延迟目标 200μs
            "cyclictest_avg_latency_us": 50,   # 平均延迟目标 50μs
        }
        
    def log(self, message: str, level: str = "INFO"):
        """
        记录日志
        
        Args:
            message: 日志消息
            level: 日志级别（INFO, WARNING, ERROR）
        """
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_message = f"[{timestamp}] [{level}] {message}"
        print(log_message)
        
        # 同时写入日志文件
        log_file = self.output_dir / "test_execution.log"
        with open(log_file, "a", encoding="utf-8") as f:
            f.write(log_message + "\n")
    
    def check_kernel_rt(self) -> Dict:
        """
        检查是否为PREEMPT_RT内核
        
        Returns:
            包含检查结果的字典
        """
        self.log("=" * 60)
        self.log("测试1: 检查内核版本和PREEMPT_RT补丁")
        self.log("=" * 60)
        
        result = {
            "test_name": "kernel_rt_check",
            "status": "UNKNOWN",
            "details": {},
            "messages": []
        }
        
        try:
            # 获取内核版本
            kernel_version = platform.release()
            result["details"]["kernel_version"] = kernel_version
            self.log(f"内核版本: {kernel_version}")
            
            # 检查是否为Linux系统
            if platform.system() != "Linux":
                result["status"] = "FAILED"
                result["messages"].append("此测试仅适用于Linux系统")
                self.log("错误: 此测试仅适用于Linux系统", "ERROR")
                return result
            
            # 方法1: 检查/proc/version
            try:
                with open("/proc/version", "r") as f:
                    proc_version = f.read()
                    result["details"]["proc_version"] = proc_version.strip()
                    self.log(f"/proc/version: {proc_version.strip()}")
            except Exception as e:
                result["messages"].append(f"无法读取/proc/version: {e}")
                self.log(f"警告: 无法读取/proc/version: {e}", "WARNING")
            
            # 方法2: 检查内核配置文件
            kernel_config_paths = [
                "/boot/config-{}".format(kernel_version),
                "/proc/config.gz",
                "/usr/src/linux/.config",
                "/lib/modules/{}/build/.config".format(kernel_version)
            ]
            
            kernel_config = None
            for config_path in kernel_config_paths:
                if os.path.exists(config_path):
                    kernel_config = config_path
                    break
            
            if kernel_config:
                result["details"]["kernel_config_path"] = kernel_config
                self.log(f"找到内核配置文件: {kernel_config}")
                
                # 读取并检查PREEMPT_RT配置
                try:
                    # 如果是.gz文件，需要解压
                    if kernel_config.endswith(".gz"):
                        import gzip
                        with gzip.open(kernel_config, "rt") as f:
                            config_content = f.read()
                    else:
                        with open(kernel_config, "r") as f:
                            config_content = f.read()
                    
                    # 检查PREEMPT相关配置
                    preempt_configs = {
                        "CONFIG_PREEMPT_RT": "完全抢占实时内核",
                        "CONFIG_PREEMPT": "抢占式内核",
                        "CONFIG_PREEMPT_VOLUNTARY": "自愿抢占",
                        "CONFIG_PREEMPT_NONE": "无抢占",
                        "CONFIG_HZ": "时钟频率",
                        "CONFIG_NO_HZ": "无滴答模式",
                        "CONFIG_HIGH_RES_TIMERS": "高精度定时器"
                    }
                    
                    detected_preempt = []
                    for config_key, config_desc in preempt_configs.items():
                        if config_key + "=y" in config_content:
                            result["details"][config_key] = True
                            detected_preempt.append(f"{config_key} ({config_desc})")
                            self.log(f"✓ 发现配置: {config_key} - {config_desc}")
                        elif config_key in config_content:
                            # 提取配置值
                            for line in config_content.split("\n"):
                                if line.startswith(config_key + "="):
                                    value = line.split("=")[1]
                                    result["details"][config_key] = value
                                    self.log(f"  {config_key} = {value}")
                                    break
                    
                    # 判断是否为PREEMPT_RT内核
                    if result["details"].get("CONFIG_PREEMPT_RT"):
                        result["status"] = "PASSED"
                        result["messages"].append("检测到PREEMPT_RT内核补丁")
                        self.log("✓ 检测到PREEMPT_RT实时内核!", "INFO")
                    elif result["details"].get("CONFIG_PREEMPT"):
                        result["status"] = "WARNING"
                        result["messages"].append("检测到CONFIG_PREEMPT，但不是完整的PREEMPT_RT")
                        self.log("⚠ 检测到CONFIG_PREEMPT，但不是完整的PREEMPT_RT", "WARNING")
                    else:
                        result["status"] = "FAILED"
                        result["messages"].append("未检测到PREEMPT_RT或PREEMPT配置")
                        self.log("✗ 未检测到实时内核配置", "ERROR")
                    
                    result["details"]["detected_preempt_configs"] = detected_preempt
                    
                except Exception as e:
                    result["status"] = "ERROR"
                    result["messages"].append(f"读取内核配置失败: {e}")
                    self.log(f"错误: 读取内核配置失败: {e}", "ERROR")
            else:
                result["status"] = "WARNING"
                result["messages"].append("未找到内核配置文件")
                self.log("警告: 未找到内核配置文件，无法确认PREEMPT_RT状态", "WARNING")
                
                # 尝试通过uname检查
                if "rt" in kernel_version.lower() or "PREEMPT_RT" in kernel_version:
                    result["status"] = "PASSED"
                    result["messages"].append("内核版本字符串包含'rt'标识")
                    self.log("✓ 内核版本包含'rt'标识", "INFO")
            
            # 方法3: 检查/sys/kernel/realtime
            realtime_path = "/sys/kernel/realtime"
            if os.path.exists(realtime_path):
                try:
                    with open(realtime_path, "r") as f:
                        is_realtime = f.read().strip()
                        result["details"]["sys_kernel_realtime"] = is_realtime
                        if is_realtime == "1":
                            self.log("✓ /sys/kernel/realtime = 1 (实时内核)", "INFO")
                            if result["status"] != "PASSED":
                                result["status"] = "PASSED"
                                result["messages"].append("/sys/kernel/realtime确认实时内核")
                        else:
                            self.log(f"  /sys/kernel/realtime = {is_realtime}", "INFO")
                except Exception as e:
                    self.log(f"警告: 无法读取{realtime_path}: {e}", "WARNING")
            
        except Exception as e:
            result["status"] = "ERROR"
            result["messages"].append(f"检查过程发生异常: {e}")
            self.log(f"错误: 检查过程发生异常: {e}", "ERROR")
        
        return result
    
    def test_cyclictest(self, duration: int = 10) -> Dict:
        """
        测试cyclictest延迟
        
        Args:
            duration: 测试持续时间（秒）
            
        Returns:
            包含测试结果的字典
        """
        self.log("=" * 60)
        self.log(f"测试2: cyclictest延迟测试（持续{duration}秒）")
        self.log("=" * 60)
        
        result = {
            "test_name": "cyclictest",
            "status": "UNKNOWN",
            "details": {},
            "messages": []
        }
        
        try:
            # 检查cyclictest是否可用
            try:
                subprocess.run(["which", "cyclictest"], 
                             check=True, capture_output=True, text=True)
                self.log("✓ cyclictest已安装")
            except subprocess.CalledProcessError:
                result["status"] = "SKIPPED"
                result["messages"].append("cyclictest未安装")
                self.log("警告: cyclictest未安装，跳过此测试", "WARNING")
                self.log("提示: 安装命令 - sudo apt-get install rt-tests", "INFO")
                return result
            
            # 检查是否有root权限（cyclictest通常需要）
            if os.geteuid() != 0:
                result["status"] = "WARNING"
                result["messages"].append("需要root权限才能运行cyclictest")
                self.log("警告: 建议使用sudo运行以获得准确的延迟测量", "WARNING")
            
            # 运行cyclictest
            # -m: 锁定内存
            # -p90: 优先级90
            # -n: 使用clock_nanosleep
            # -i10000: 间隔10000微秒（10ms）
            # -l<count>: 循环次数，根据duration计算
            interval_us = 10000  # 10ms
            loops = int((duration * 1000000) / interval_us)
            
            cmd = [
                "cyclictest",
                "-m",
                "-Sp90",
                "-i10000",
                "-h4000",
                f"-l{loops}",
                "-q"
            ]
            
            self.log(f"执行命令: {' '.join(cmd)}")
            self.log(f"测试将持续约{duration}秒，请稍候...")
            
            start_time = time.time()
            
            try:
                process = subprocess.Popen(
                    cmd,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True
                )
                
                stdout, stderr = process.communicate(timeout=duration + 10)
                
                execution_time = time.time() - start_time
                result["details"]["execution_time"] = execution_time
                
                if process.returncode != 0:
                    result["status"] = "ERROR"
                    result["messages"].append(f"cyclictest执行失败: {stderr}")
                    self.log(f"错误: cyclictest执行失败: {stderr}", "ERROR")
                    return result
                
                # 解析cyclictest输出
                # 输出格式通常为：
                # # /dev/cpu_dma_latency set to 0us
                # T: 0 ( 1234) P:90 I:10000 C:   1000 Min:      5 Act:   10 Avg:   12 Max:     200
                lines = stdout.strip().split("\n")
                
                for line in lines:
                    if line.startswith("T:"):
                        parts = line.split()
                        # 提取延迟值（单位：微秒）
                        min_lat = int(parts[6])
                        act_lat = int(parts[8])
                        avg_lat = int(parts[10])
                        max_lat = int(parts[12])
                        
                        result["details"]["min_latency_us"] = min_lat
                        result["details"]["act_latency_us"] = act_lat
                        result["details"]["avg_latency_us"] = avg_lat
                        result["details"]["max_latency_us"] = max_lat
                        
                        self.log(f"  最小延迟: {min_lat} μs")
                        self.log(f"  当前延迟: {act_lat} μs")
                        self.log(f"  平均延迟: {avg_lat} μs")
                        self.log(f"  最大延迟: {max_lat} μs")
                        
                        # 检查是否达到性能目标
                        if max_lat <= self.targets["cyclictest_max_latency_us"]:
                            result["status"] = "PASSED"
                            result["messages"].append(
                                f"最大延迟{max_lat}μs 达到目标<{self.targets['cyclictest_max_latency_us']}μs"
                            )
                            self.log(f"✓ 最大延迟达标 (<{self.targets['cyclictest_max_latency_us']}μs)", "INFO")
                        else:
                            result["status"] = "FAILED"
                            result["messages"].append(
                                f"最大延迟{max_lat}μs 超过目标{self.targets['cyclictest_max_latency_us']}μs"
                            )
                            self.log(f"✗ 最大延迟超标 (>={self.targets['cyclictest_max_latency_us']}μs)", "ERROR")
                        
                        if avg_lat <= self.targets["cyclictest_avg_latency_us"]:
                            result["messages"].append(
                                f"平均延迟{avg_lat}μs 达到目标<{self.targets['cyclictest_avg_latency_us']}μs"
                            )
                            self.log(f"✓ 平均延迟达标 (<{self.targets['cyclictest_avg_latency_us']}μs)", "INFO")
                        else:
                            result["messages"].append(
                                f"平均延迟{avg_lat}μs 超过建议值{self.targets['cyclictest_avg_latency_us']}μs"
                            )
                            self.log(f"⚠ 平均延迟偏高 (>={self.targets['cyclictest_avg_latency_us']}μs)", "WARNING")
                        
                        break
                else:
                    result["status"] = "ERROR"
                    result["messages"].append("无法解析cyclictest输出")
                    self.log("错误: 无法解析cyclictest输出", "ERROR")
                    self.log(f"原始输出: {stdout}", "INFO")
                
            except subprocess.TimeoutExpired:
                process.kill()
                result["status"] = "ERROR"
                result["messages"].append("cyclictest执行超时")
                self.log("错误: cyclictest执行超时", "ERROR")
            
        except Exception as e:
            result["status"] = "ERROR"
            result["messages"].append(f"测试过程发生异常: {e}")
            self.log(f"错误: 测试过程发生异常: {e}", "ERROR")
        
        return result
    
    def test_cpu_affinity(self) -> Dict:
        """
        测试CPU亲和性设置
        
        Returns:
            包含测试结果的字典
        """
        self.log("=" * 60)
        self.log("测试3: CPU亲和性配置测试")
        self.log("=" * 60)
        
        result = {
            "test_name": "cpu_affinity",
            "status": "UNKNOWN",
            "details": {},
            "messages": []
        }
        
        try:
            # 获取CPU信息
            cpu_count = psutil.cpu_count(logical=True)
            cpu_count_physical = psutil.cpu_count(logical=False)
            
            result["details"]["cpu_count_logical"] = cpu_count
            result["details"]["cpu_count_physical"] = cpu_count_physical
            
            self.log(f"逻辑CPU数量: {cpu_count}")
            self.log(f"物理CPU核心数: {cpu_count_physical}")
            
            # 获取当前进程的CPU亲和性
            current_process = psutil.Process()
            
            try:
                affinity = current_process.cpu_affinity()
                result["details"]["current_affinity"] = affinity
                self.log(f"当前进程CPU亲和性: {affinity}")
            except AttributeError:
                # Windows系统可能不支持cpu_affinity
                result["status"] = "SKIPPED"
                result["messages"].append("当前系统不支持CPU亲和性设置")
                self.log("警告: 当前系统不支持CPU亲和性设置（可能是Windows）", "WARNING")
                return result
            
            # 测试设置CPU亲和性
            if platform.system() == "Linux":
                # 尝试设置到CPU 0
                try:
                    current_process.cpu_affinity([0])
                    new_affinity = current_process.cpu_affinity()
                    
                    if new_affinity == [0]:
                        result["details"]["test_affinity_set"] = [0]
                        result["details"]["test_affinity_result"] = "SUCCESS"
                        self.log("✓ 成功设置CPU亲和性到[0]")
                        
                        # 恢复原始亲和性
                        current_process.cpu_affinity(affinity)
                        self.log("✓ 已恢复原始CPU亲和性")
                        
                        result["status"] = "PASSED"
                        result["messages"].append("CPU亲和性设置功能正常")
                    else:
                        result["status"] = "FAILED"
                        result["messages"].append("CPU亲和性设置失败")
                        self.log("✗ CPU亲和性设置失败", "ERROR")
                        
                except Exception as e:
                    result["status"] = "ERROR"
                    result["messages"].append(f"CPU亲和性设置异常: {e}")
                    self.log(f"错误: CPU亲和性设置异常: {e}", "ERROR")
            else:
                result["status"] = "SKIPPED"
                result["messages"].append("非Linux系统，跳过CPU亲和性测试")
                self.log("提示: 非Linux系统，跳过CPU亲和性测试", "INFO")
            
            # 检查CPU隔离配置（isolcpus）
            if platform.system() == "Linux":
                try:
                    with open("/proc/cmdline", "r") as f:
                        cmdline = f.read()
                        result["details"]["kernel_cmdline"] = cmdline.strip()
                        
                        if "isolcpus" in cmdline:
                            # 提取隔离的CPU
                            for param in cmdline.split():
                                if param.startswith("isolcpus="):
                                    isolated_cpus = param.split("=")[1]
                                    result["details"]["isolated_cpus"] = isolated_cpus
                                    self.log(f"✓ 检测到CPU隔离配置: {isolated_cpus}")
                                    result["messages"].append(f"检测到CPU隔离: {isolated_cpus}")
                                    break
                        else:
                            self.log("  未检测到CPU隔离配置（isolcpus）")
                            result["messages"].append("建议配置CPU隔离以提高实时性能")
                except Exception as e:
                    self.log(f"警告: 无法读取/proc/cmdline: {e}", "WARNING")
            
        except Exception as e:
            result["status"] = "ERROR"
            result["messages"].append(f"测试过程发生异常: {e}")
            self.log(f"错误: 测试过程发生异常: {e}", "ERROR")
        
        return result
    
    def test_memory_lock(self) -> Dict:
        """
        测试内存锁定功能
        
        Returns:
            包含测试结果的字典
        """
        self.log("=" * 60)
        self.log("测试4: 内存锁定功能测试")
        self.log("=" * 60)
        
        result = {
            "test_name": "memory_lock",
            "status": "UNKNOWN",
            "details": {},
            "messages": []
        }
        
        try:
            # 检查系统是否支持mlock
            if platform.system() != "Linux":
                result["status"] = "SKIPPED"
                result["messages"].append("此测试仅适用于Linux系统")
                self.log("警告: 此测试仅适用于Linux系统", "WARNING")
                return result
            
            # 检查当前内存锁定限制
            try:
                import resource
                
                # RLIMIT_MEMLOCK: 最大锁定内存字节数
                soft, hard = resource.getrlimit(resource.RLIMIT_MEMLOCK)
                
                result["details"]["memlock_limit_soft"] = soft
                result["details"]["memlock_limit_hard"] = hard
                
                self.log(f"内存锁定限制 (soft): {soft} bytes ({soft / 1024:.2f} KB)")
                self.log(f"内存锁定限制 (hard): {hard} bytes ({hard / 1024:.2f} KB)")
                
                if soft == 0:
                    result["status"] = "FAILED"
                    result["messages"].append("内存锁定限制为0，需要增加RLIMIT_MEMLOCK")
                    self.log("✗ 内存锁定限制为0，无法锁定内存", "ERROR")
                    self.log("提示: 在/etc/security/limits.conf中添加:", "INFO")
                    self.log("  * hard memlock unlimited", "INFO")
                    self.log("  * soft memlock unlimited", "INFO")
                elif soft < 1024 * 1024 * 100:  # 小于100MB
                    result["status"] = "WARNING"
                    result["messages"].append(f"内存锁定限制较低({soft}字节)，建议增加")
                    self.log(f"⚠ 内存锁定限制较低({soft / 1024 / 1024:.2f} MB)", "WARNING")
                else:
                    result["status"] = "PASSED"
                    result["messages"].append(f"内存锁定限制充足({soft}字节)")
                    self.log(f"✓ 内存锁定限制充足 ({soft / 1024 / 1024:.2f} MB)", "INFO")
                
            except Exception as e:
                result["status"] = "ERROR"
                result["messages"].append(f"无法获取内存锁定限制: {e}")
                self.log(f"错误: 无法获取内存锁定限制: {e}", "ERROR")
                return result
            
            # 测试实际的内存锁定功能
            try:
                import ctypes
                import mmap
                
                # 创建测试内存区域
                test_size = 1024 * 1024  # 1MB
                test_memory = mmap.mmap(-1, test_size)
                
                # 尝试锁定内存
                libc = ctypes.CDLL("libc.so.6", use_errno=True)
                
                # mlock(const void *addr, size_t len)
                ret = libc.mlock(ctypes.c_void_p(id(test_memory)), test_size)
                
                if ret == 0:
                    result["details"]["mlock_test"] = "SUCCESS"
                    self.log("✓ 内存锁定测试成功（1MB）")
                    
                    # 解锁
                    libc.munlock(ctypes.c_void_p(id(test_memory)), test_size)
                    self.log("✓ 内存解锁成功")
                    
                    if result["status"] != "PASSED":
                        result["status"] = "PASSED"
                    result["messages"].append("内存锁定功能测试成功")
                else:
                    errno = ctypes.get_errno()
                    result["details"]["mlock_test"] = f"FAILED (errno={errno})"
                    result["status"] = "FAILED"
                    result["messages"].append(f"内存锁定失败 (errno={errno})")
                    self.log(f"✗ 内存锁定失败 (errno={errno})", "ERROR")
                
                test_memory.close()
                
            except Exception as e:
                result["status"] = "ERROR"
                result["messages"].append(f"内存锁定测试异常: {e}")
                self.log(f"错误: 内存锁定测试异常: {e}", "ERROR")
            
            # 检查当前进程的内存锁定状态
            try:
                current_process = psutil.Process()
                memory_info = current_process.memory_info()
                
                result["details"]["process_memory_rss"] = memory_info.rss
                result["details"]["process_memory_vms"] = memory_info.vms
                
                self.log(f"进程内存使用 (RSS): {memory_info.rss / 1024 / 1024:.2f} MB")
                self.log(f"进程内存使用 (VMS): {memory_info.vms / 1024 / 1024:.2f} MB")
                
            except Exception as e:
                self.log(f"警告: 无法获取进程内存信息: {e}", "WARNING")
            
        except Exception as e:
            result["status"] = "ERROR"
            result["messages"].append(f"测试过程发生异常: {e}")
            self.log(f"错误: 测试过程发生异常: {e}", "ERROR")
        
        return result
    
    def run_all_tests(self) -> Dict:
        """
        运行所有测试
        
        Returns:
            包含所有测试结果的字典
        """
        self.log("=" * 60)
        self.log("开始PREEMPT_RT实时性前置验证测试")
        self.log("=" * 60)
        
        # 执行各项测试
        self.results["tests"]["kernel_rt"] = self.check_kernel_rt()
        self.results["tests"]["cyclictest"] = self.test_cyclictest(duration=10)
        self.results["tests"]["cpu_affinity"] = self.test_cpu_affinity()
        self.results["tests"]["memory_lock"] = self.test_memory_lock()
        
        # 统计测试结果
        for test_name, test_result in self.results["tests"].items():
            self.results["summary"]["total"] += 1
            status = test_result.get("status", "UNKNOWN")
            
            if status == "PASSED":
                self.results["summary"]["passed"] += 1
            elif status == "FAILED" or status == "ERROR":
                self.results["summary"]["failed"] += 1
            elif status == "WARNING":
                self.results["summary"]["warnings"] += 1
        
        self.log("=" * 60)
        self.log("所有测试完成")
        self.log("=" * 60)
        
        return self.results
    
    def generate_report(self, output_file: str = None) -> str:
        """
        生成测试报告
        
        Args:
            output_file: 报告输出文件名（如果为None，则自动生成）
            
        Returns:
            报告文件路径
        """
        if output_file is None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            output_file = f"realtime_test_report_{timestamp}.json"
        
        report_path = self.output_dir / output_file
        
        # 保存JSON格式的报告
        with open(report_path, "w", encoding="utf-8") as f:
            json.dump(self.results, f, indent=2, ensure_ascii=False)
        
        self.log(f"测试报告已保存: {report_path}")
        
        # 生成文本格式的摘要报告
        summary_path = report_path.with_suffix(".txt")
        
        with open(summary_path, "w", encoding="utf-8") as f:
            f.write("=" * 80 + "\n")
            f.write("PREEMPT_RT实时性前置验证测试报告\n")
            f.write("=" * 80 + "\n\n")
            
            f.write(f"测试时间: {self.results['test_time']}\n")
            f.write(f"平台: {self.results['platform']}\n")
            f.write(f"Python版本: {self.results['python_version']}\n\n")
            
            f.write("-" * 80 + "\n")
            f.write("测试摘要\n")
            f.write("-" * 80 + "\n")
            f.write(f"总测试数: {self.results['summary']['total']}\n")
            f.write(f"通过: {self.results['summary']['passed']}\n")
            f.write(f"失败: {self.results['summary']['failed']}\n")
            f.write(f"警告: {self.results['summary']['warnings']}\n\n")
            
            f.write("-" * 80 + "\n")
            f.write("详细测试结果\n")
            f.write("-" * 80 + "\n\n")
            
            for test_name, test_result in self.results["tests"].items():
                f.write(f"[{test_result['test_name']}]\n")
                f.write(f"状态: {test_result['status']}\n")
                
                if test_result.get("messages"):
                    f.write("消息:\n")
                    for msg in test_result["messages"]:
                        f.write(f"  - {msg}\n")
                
                if test_result.get("details"):
                    f.write("详细信息:\n")
                    for key, value in test_result["details"].items():
                        if isinstance(value, list):
                            f.write(f"  {key}: {', '.join(map(str, value))}\n")
                        else:
                            f.write(f"  {key}: {value}\n")
                
                f.write("\n")
            
            f.write("=" * 80 + "\n")
            f.write("报告生成完成\n")
            f.write("=" * 80 + "\n")
        
        self.log(f"摘要报告已保存: {summary_path}")
        
        # 在控制台输出摘要
        print("\n" + "=" * 80)
        print("测试摘要")
        print("=" * 80)
        print(f"总测试数: {self.results['summary']['total']}")
        print(f"通过: {self.results['summary']['passed']}")
        print(f"失败: {self.results['summary']['failed']}")
        print(f"警告: {self.results['summary']['warnings']}")
        print("=" * 80)
        
        return str(report_path)


def main():
    """主函数"""
    # 创建测试实例
    tester = RealtimePrerequisiteTest(output_dir="./test_results")
    
    # 运行所有测试
    tester.run_all_tests()
    
    # 生成报告
    report_path = tester.generate_report()
    
    # 返回退出码
    if tester.results["summary"]["failed"] > 0:
        sys.exit(1)
    else:
        sys.exit(0)


if __name__ == "__main__":
    main()
