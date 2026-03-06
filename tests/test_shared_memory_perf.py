#!/usr/bin/env python3
"""
共享内存性能测试脚本

测试项目：
1. 共享内存读写延迟（目标<10μs）
2. 吞吐量（目标>10K ops/s）
3. 并发访问性能
4. 性能报告生成

作者：BackendAgent
日期：2026-03-06
版本：1.0
"""

import os
import sys
import time
import json
import platform
import threading
import multiprocessing as mp
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Tuple, Optional
from statistics import mean, median, stdev
from dataclasses import dataclass, asdict
import ctypes
import struct


@dataclass
class PerformanceMetrics:
    """性能指标数据类"""
    operation: str
    count: int
    total_time_sec: float
    ops_per_sec: float
    avg_latency_us: float
    min_latency_us: float
    max_latency_us: float
    median_latency_us: float
    p95_latency_us: float
    p99_latency_us: float


class SharedMemoryPerformanceTest:
    """共享内存性能测试类"""
    
    def __init__(self, output_dir: str = "./test_results"):
        """
        初始化测试类
        
        Args:
            output_dir: 测试结果输出目录
        """
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        self.results = {
            "test_time": datetime.now().isoformat(),
            "platform": platform.platform(),
            "python_version": platform.python_version(),
            "tests": {},
            "targets": {
                "read_latency_us": 10,
                "write_latency_us": 10,
                "throughput_ops_per_sec": 10000
            }
        }
        
        # 测试配置
        self.test_config = {
            "data_size_bytes": 1024,  # 1KB数据块
            "iteration_count": 10000,  # 迭代次数
            "warmup_iterations": 100,  # 预热迭代
            "concurrent_threads": [1, 2, 4, 8],  # 并发线程数
            "concurrent_processes": [1, 2, 4]  # 并发进程数
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
        
        # 写入日志文件
        log_file = self.output_dir / "shm_perf_test.log"
        with open(log_file, "a", encoding="utf-8") as f:
            f.write(log_message + "\n")
    
    def calculate_percentile(self, data: List[float], percentile: float) -> float:
        """
        计算百分位数
        
        Args:
            data: 数据列表
            percentile: 百分位数（0-100）
            
        Returns:
            百分位数值
        """
        if not data:
            return 0.0
        sorted_data = sorted(data)
        index = int(len(sorted_data) * percentile / 100)
        index = min(index, len(sorted_data) - 1)
        return sorted_data[index]
    
    def test_mmap_read_write_latency(self) -> Dict:
        """
        测试mmap共享内存读写延迟
        
        Returns:
            测试结果字典
        """
        self.log("=" * 60)
        self.log("测试1: mmap共享内存读写延迟测试")
        self.log("=" * 60)
        
        result = {
            "test_name": "mmap_read_write_latency",
            "status": "UNKNOWN",
            "metrics": {},
            "details": {},
            "messages": []
        }
        
        try:
            import mmap
            
            # 创建共享内存区域
            data_size = self.test_config["data_size_bytes"]
            iteration_count = self.test_config["iteration_count"]
            warmup_iterations = self.test_config["warmup_iterations"]
            
            self.log(f"数据块大小: {data_size} bytes")
            self.log(f"迭代次数: {iteration_count}")
            self.log(f"预热次数: {warmup_iterations}")
            
            # 创建匿名内存映射
            shm = mmap.mmap(-1, data_size)
            
            # 准备测试数据
            test_data = os.urandom(data_size)
            
            # ===== 写入测试 =====
            self.log("\n--- 写入延迟测试 ---")
            
            # 预热
            for _ in range(warmup_iterations):
                shm.seek(0)
                shm.write(test_data)
            
            # 正式测试
            write_latencies = []
            start_total = time.perf_counter()
            
            for _ in range(iteration_count):
                start = time.perf_counter()
                shm.seek(0)
                shm.write(test_data)
                end = time.perf_counter()
                write_latencies.append((end - start) * 1_000_000)  # 转换为微秒
            
            total_write_time = time.perf_counter() - start_total
            
            # 计算写入性能指标
            write_metrics = PerformanceMetrics(
                operation="write",
                count=iteration_count,
                total_time_sec=total_write_time,
                ops_per_sec=iteration_count / total_write_time,
                avg_latency_us=mean(write_latencies),
                min_latency_us=min(write_latencies),
                max_latency_us=max(write_latencies),
                median_latency_us=median(write_latencies),
                p95_latency_us=self.calculate_percentile(write_latencies, 95),
                p99_latency_us=self.calculate_percentile(write_latencies, 99)
            )
            
            result["metrics"]["write"] = asdict(write_metrics)
            
            self.log(f"写入吞吐量: {write_metrics.ops_per_sec:.2f} ops/s")
            self.log(f"平均延迟: {write_metrics.avg_latency_us:.2f} μs")
            self.log(f"最小延迟: {write_metrics.min_latency_us:.2f} μs")
            self.log(f"最大延迟: {write_metrics.max_latency_us:.2f} μs")
            self.log(f"中位数延迟: {write_metrics.median_latency_us:.2f} μs")
            self.log(f"P95延迟: {write_metrics.p95_latency_us:.2f} μs")
            self.log(f"P99延迟: {write_metrics.p99_latency_us:.2f} μs")
            
            # ===== 读取测试 =====
            self.log("\n--- 读取延迟测试 ---")
            
            # 预热
            for _ in range(warmup_iterations):
                shm.seek(0)
                _ = shm.read(data_size)
            
            # 正式测试
            read_latencies = []
            start_total = time.perf_counter()
            
            for _ in range(iteration_count):
                start = time.perf_counter()
                shm.seek(0)
                _ = shm.read(data_size)
                end = time.perf_counter()
                read_latencies.append((end - start) * 1_000_000)  # 转换为微秒
            
            total_read_time = time.perf_counter() - start_total
            
            # 计算读取性能指标
            read_metrics = PerformanceMetrics(
                operation="read",
                count=iteration_count,
                total_time_sec=total_read_time,
                ops_per_sec=iteration_count / total_read_time,
                avg_latency_us=mean(read_latencies),
                min_latency_us=min(read_latencies),
                max_latency_us=max(read_latencies),
                median_latency_us=median(read_latencies),
                p95_latency_us=self.calculate_percentile(read_latencies, 95),
                p99_latency_us=self.calculate_percentile(read_latencies, 99)
            )
            
            result["metrics"]["read"] = asdict(read_metrics)
            
            self.log(f"读取吞吐量: {read_metrics.ops_per_sec:.2f} ops/s")
            self.log(f"平均延迟: {read_metrics.avg_latency_us:.2f} μs")
            self.log(f"最小延迟: {read_metrics.min_latency_us:.2f} μs")
            self.log(f"最大延迟: {read_metrics.max_latency_us:.2f} μs")
            self.log(f"中位数延迟: {read_metrics.median_latency_us:.2f} μs")
            self.log(f"P95延迟: {read_metrics.p95_latency_us:.2f} μs")
            self.log(f"P99延迟: {read_metrics.p99_latency_us:.2f} μs")
            
            # 检查性能目标
            target_latency = self.results["targets"]["read_latency_us"]
            
            if (read_metrics.avg_latency_us < target_latency and 
                write_metrics.avg_latency_us < target_latency):
                result["status"] = "PASSED"
                result["messages"].append(
                    f"读写延迟达标 (目标<{target_latency}μs)"
                )
                self.log(f"\n✓ 读写延迟达标 (目标<{target_latency}μs)", "INFO")
            else:
                result["status"] = "FAILED"
                result["messages"].append(
                    f"读写延迟超标 (目标<{target_latency}μs)"
                )
                self.log(f"\n✗ 读写延迟超标 (目标<{target_latency}μs)", "ERROR")
            
            # 清理
            shm.close()
            
        except Exception as e:
            result["status"] = "ERROR"
            result["messages"].append(f"测试异常: {str(e)}")
            self.log(f"错误: 测试异常: {e}", "ERROR")
            import traceback
            self.log(traceback.format_exc(), "ERROR")
        
        return result
    
    def test_throughput(self) -> Dict:
        """
        测试共享内存吞吐量
        
        Returns:
            测试结果字典
        """
        self.log("\n" + "=" * 60)
        self.log("测试2: 共享内存吞吐量测试")
        self.log("=" * 60)
        
        result = {
            "test_name": "throughput",
            "status": "UNKNOWN",
            "metrics": {},
            "details": {},
            "messages": []
        }
        
        try:
            import mmap
            
            # 测试不同大小的数据块
            data_sizes = [64, 256, 1024, 4096, 16384]  # 64B, 256B, 1KB, 4KB, 16KB
            iteration_count = 5000
            
            throughput_results = {}
            
            for data_size in data_sizes:
                self.log(f"\n--- 数据块大小: {data_size} bytes ---")
                
                # 创建共享内存
                shm = mmap.mmap(-1, data_size)
                test_data = os.urandom(data_size)
                
                # 吞吐量测试（混合读写）
                operations = []
                start_total = time.perf_counter()
                
                for i in range(iteration_count):
                    # 交替读写
                    if i % 2 == 0:
                        shm.seek(0)
                        shm.write(test_data)
                    else:
                        shm.seek(0)
                        _ = shm.read(data_size)
                    operations.append(time.perf_counter())
                
                total_time = operations[-1] - start_total
                ops_per_sec = iteration_count / total_time
                
                throughput_results[data_size] = {
                    "data_size_bytes": data_size,
                    "iteration_count": iteration_count,
                    "total_time_sec": total_time,
                    "ops_per_sec": ops_per_sec,
                    "throughput_mbps": (data_size * iteration_count / total_time) / (1024 * 1024)
                }
                
                self.log(f"吞吐量: {ops_per_sec:.2f} ops/s")
                self.log(f"带宽: {throughput_results[data_size]['throughput_mbps']:.2f} MB/s")
                
                shm.close()
            
            result["details"]["throughput_by_size"] = throughput_results
            
            # 检查是否达到吞吐量目标
            target_ops = self.results["targets"]["throughput_ops_per_sec"]
            max_throughput = max(r["ops_per_sec"] for r in throughput_results.values())
            
            if max_throughput >= target_ops:
                result["status"] = "PASSED"
                result["messages"].append(
                    f"吞吐量达标 (目标>{target_ops} ops/s, 实际{max_throughput:.2f} ops/s)"
                )
                self.log(f"\n✓ 吞吐量达标 (>{target_ops} ops/s)", "INFO")
            else:
                result["status"] = "FAILED"
                result["messages"].append(
                    f"吞吐量不达标 (目标>{target_ops} ops/s, 实际{max_throughput:.2f} ops/s)"
                )
                self.log(f"\n✗ 吞吐量不达标 (<{target_ops} ops/s)", "ERROR")
            
        except Exception as e:
            result["status"] = "ERROR"
            result["messages"].append(f"测试异常: {str(e)}")
            self.log(f"错误: 测试异常: {e}", "ERROR")
        
        return result
    
    def worker_thread(self, shm_name: str, data_size: int, iteration_count: int, 
                      result_queue: mp.Queue, thread_id: int):
        """
        工作线程函数
        
        Args:
            shm_name: 共享内存名称
            data_size: 数据大小
            iteration_count: 迭代次数
            result_queue: 结果队列
            thread_id: 线程ID
        """
        try:
            import mmap
            
            # 打开共享内存
            shm = mmap.mmap(-1, data_size)
            test_data = os.urandom(data_size)
            
            latencies = []
            start_total = time.perf_counter()
            
            for _ in range(iteration_count):
                start = time.perf_counter()
                shm.seek(0)
                shm.write(test_data)
                shm.seek(0)
                _ = shm.read(data_size)
                end = time.perf_counter()
                latencies.append((end - start) * 1_000_000)
            
            total_time = time.perf_counter() - start_total
            
            result_queue.put({
                "thread_id": thread_id,
                "iteration_count": iteration_count,
                "total_time_sec": total_time,
                "avg_latency_us": mean(latencies),
                "ops_per_sec": iteration_count / total_time
            })
            
            shm.close()
            
        except Exception as e:
            result_queue.put({
                "thread_id": thread_id,
                "error": str(e)
            })
    
    def test_concurrent_access(self) -> Dict:
        """
        测试并发访问性能
        
        Returns:
            测试结果字典
        """
        self.log("\n" + "=" * 60)
        self.log("测试3: 并发访问性能测试")
        self.log("=" * 60)
        
        result = {
            "test_name": "concurrent_access",
            "status": "UNKNOWN",
            "metrics": {},
            "details": {},
            "messages": []
        }
        
        try:
            data_size = 1024  # 1KB
            iterations_per_thread = 2000
            
            concurrent_results = {}
            
            for thread_count in self.test_config["concurrent_threads"]:
                self.log(f"\n--- 并发线程数: {thread_count} ---")
                
                result_queue = mp.Queue()
                threads = []
                
                start_total = time.perf_counter()
                
                # 创建并启动线程
                for i in range(thread_count):
                    t = threading.Thread(
                        target=self.worker_thread,
                        args=(f"shm_test_{i}", data_size, iterations_per_thread, 
                              result_queue, i)
                    )
                    threads.append(t)
                    t.start()
                
                # 等待所有线程完成
                for t in threads:
                    t.join()
                
                total_time = time.perf_counter() - start_total
                
                # 收集结果
                thread_results = []
                while not result_queue.empty():
                    thread_results.append(result_queue.get())
                
                # 计算聚合指标
                total_ops = sum(r["iteration_count"] for r in thread_results if "error" not in r)
                avg_latencies = [r["avg_latency_us"] for r in thread_results if "error" not in r]
                
                concurrent_results[thread_count] = {
                    "thread_count": thread_count,
                    "total_time_sec": total_time,
                    "total_ops": total_ops,
                    "total_ops_per_sec": total_ops / total_time,
                    "avg_thread_latency_us": mean(avg_latencies) if avg_latencies else 0,
                    "thread_results": thread_results
                }
                
                self.log(f"总操作数: {total_ops}")
                self.log(f"总吞吐量: {total_ops / total_time:.2f} ops/s")
                self.log(f"平均线程延迟: {mean(avg_latencies):.2f} μs")
            
            result["details"]["concurrent_results"] = concurrent_results
            
            # 检查并发性能
            max_concurrent_throughput = max(
                r["total_ops_per_sec"] for r in concurrent_results.values()
            )
            
            target_throughput = self.results["targets"]["throughput_ops_per_sec"]
            
            if max_concurrent_throughput >= target_throughput:
                result["status"] = "PASSED"
                result["messages"].append(
                    f"并发吞吐量达标 ({max_concurrent_throughput:.2f} ops/s)"
                )
                self.log(f"\n✓ 并发吞吐量达标", "INFO")
            else:
                result["status"] = "WARNING"
                result["messages"].append(
                    f"并发吞吐量偏低 ({max_concurrent_throughput:.2f} ops/s)"
                )
                self.log(f"\n⚠ 并发吞吐量偏低", "WARNING")
            
        except Exception as e:
            result["status"] = "ERROR"
            result["messages"].append(f"测试异常: {str(e)}")
            self.log(f"错误: 测试异常: {e}", "ERROR")
            import traceback
            self.log(traceback.format_exc(), "ERROR")
        
        return result
    
    def test_cross_process_shm(self) -> Dict:
        """
        测试跨进程共享内存性能
        
        Returns:
            测试结果字典
        """
        self.log("\n" + "=" * 60)
        self.log("测试4: 跨进程共享内存性能测试")
        self.log("=" * 60)
        
        result = {
            "test_name": "cross_process_shm",
            "status": "UNKNOWN",
            "metrics": {},
            "details": {},
            "messages": []
        }
        
        # 检查是否在Linux系统
        if platform.system() != "Linux":
            result["status"] = "SKIPPED"
            result["messages"].append("跨进程共享内存测试仅支持Linux")
            self.log("提示: 跨进程共享内存测试仅支持Linux系统", "INFO")
            return result
        
        try:
            from multiprocessing import shared_memory
            
            data_size = 1024
            iteration_count = 5000
            
            # 创建共享内存
            shm = shared_memory.SharedMemory(create=True, size=data_size)
            shm_name = shm.name
            
            self.log(f"共享内存名称: {shm_name}")
            self.log(f"数据大小: {data_size} bytes")
            
            def writer_process(name, size, iterations, queue):
                """写入进程"""
                try:
                    existing_shm = shared_memory.SharedMemory(name=name)
                    test_data = b'A' * size
                    
                    latencies = []
                    for _ in range(iterations):
                        start = time.perf_counter()
                        existing_shm.buf[:size] = test_data
                        end = time.perf_counter()
                        latencies.append((end - start) * 1_000_000)
                    
                    queue.put({
                        "role": "writer",
                        "avg_latency_us": mean(latencies),
                        "ops_per_sec": iterations / sum(latencies) * 1_000_000
                    })
                    
                    existing_shm.close()
                except Exception as e:
                    queue.put({"role": "writer", "error": str(e)})
            
            def reader_process(name, size, iterations, queue):
                """读取进程"""
                try:
                    existing_shm = shared_memory.SharedMemory(name=name)
                    
                    latencies = []
                    for _ in range(iterations):
                        start = time.perf_counter()
                        _ = bytes(existing_shm.buf[:size])
                        end = time.perf_counter()
                        latencies.append((end - start) * 1_000_000)
                    
                    queue.put({
                        "role": "reader",
                        "avg_latency_us": mean(latencies),
                        "ops_per_sec": iterations / sum(latencies) * 1_000_000
                    })
                    
                    existing_shm.close()
                except Exception as e:
                    queue.put({"role": "reader", "error": str(e)})
            
            # 启动读写进程
            queue = mp.Queue()
            
            writer = mp.Process(
                target=writer_process,
                args=(shm_name, data_size, iteration_count, queue)
            )
            reader = mp.Process(
                target=reader_process,
                args=(shm_name, data_size, iteration_count, queue)
            )
            
            start_time = time.perf_counter()
            
            writer.start()
            reader.start()
            
            writer.join()
            reader.join()
            
            total_time = time.perf_counter() - start_time
            
            # 收集结果
            process_results = []
            while not queue.empty():
                process_results.append(queue.get())
            
            result["details"]["process_results"] = process_results
            result["details"]["total_time_sec"] = total_time
            
            for proc_result in process_results:
                if "error" not in proc_result:
                    self.log(f"{proc_result['role']}: "
                            f"平均延迟 {proc_result['avg_latency_us']:.2f} μs, "
                            f"吞吐量 {proc_result['ops_per_sec']:.2f} ops/s")
                else:
                    self.log(f"{proc_result['role']}: 错误 - {proc_result['error']}", "ERROR")
            
            # 检查结果
            successful_results = [r for r in process_results if "error" not in r]
            if len(successful_results) == 2:
                result["status"] = "PASSED"
                result["messages"].append("跨进程共享内存测试成功")
                self.log("\n✓ 跨进程共享内存测试成功", "INFO")
            else:
                result["status"] = "FAILED"
                result["messages"].append("跨进程共享内存测试失败")
                self.log("\n✗ 跨进程共享内存测试失败", "ERROR")
            
            # 清理
            shm.close()
            shm.unlink()
            
        except ImportError:
            result["status"] = "SKIPPED"
            result["messages"].append("需要Python 3.8+的shared_memory模块")
            self.log("提示: 需要Python 3.8+的shared_memory模块", "WARNING")
        except Exception as e:
            result["status"] = "ERROR"
            result["messages"].append(f"测试异常: {str(e)}")
            self.log(f"错误: 测试异常: {e}", "ERROR")
            import traceback
            self.log(traceback.format_exc(), "ERROR")
        
        return result
    
    def run_all_tests(self) -> Dict:
        """
        运行所有测试
        
        Returns:
            测试结果字典
        """
        self.log("=" * 80)
        self.log("共享内存性能测试开始")
        self.log("=" * 80)
        
        # 执行各项测试
        self.results["tests"]["latency"] = self.test_mmap_read_write_latency()
        self.results["tests"]["throughput"] = self.test_throughput()
        self.results["tests"]["concurrent"] = self.test_concurrent_access()
        self.results["tests"]["cross_process"] = self.test_cross_process_shm()
        
        self.log("\n" + "=" * 80)
        self.log("所有测试完成")
        self.log("=" * 80)
        
        return self.results
    
    def generate_report(self, output_file: str = None) -> str:
        """
        生成测试报告
        
        Args:
            output_file: 报告文件名（如果为None，自动生成）
            
        Returns:
            报告文件路径
        """
        if output_file is None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            output_file = f"shm_performance_report_{timestamp}.json"
        
        report_path = self.output_dir / output_file
        
        # 保存JSON报告
        with open(report_path, "w", encoding="utf-8") as f:
            json.dump(self.results, f, indent=2, ensure_ascii=False)
        
        self.log(f"JSON报告已保存: {report_path}")
        
        # 生成文本摘要
        summary_path = report_path.with_suffix(".txt")
        
        with open(summary_path, "w", encoding="utf-8") as f:
            f.write("=" * 80 + "\n")
            f.write("共享内存性能测试报告\n")
            f.write("=" * 80 + "\n\n")
            
            f.write(f"测试时间: {self.results['test_time']}\n")
            f.write(f"平台: {self.results['platform']}\n")
            f.write(f"Python版本: {self.results['python_version']}\n\n")
            
            # 性能目标
            f.write("-" * 80 + "\n")
            f.write("性能目标\n")
            f.write("-" * 80 + "\n")
            for key, value in self.results["targets"].items():
                f.write(f"{key}: {value}\n")
            f.write("\n")
            
            # 测试结果
            f.write("-" * 80 + "\n")
            f.write("测试结果摘要\n")
            f.write("-" * 80 + "\n\n")
            
            for test_name, test_result in self.results["tests"].items():
                f.write(f"[{test_result['test_name']}]\n")
                f.write(f"状态: {test_result['status']}\n")
                
                if test_result.get("messages"):
                    f.write("消息:\n")
                    for msg in test_result["messages"]:
                        f.write(f"  - {msg}\n")
                
                # 写入关键指标
                if "metrics" in test_result:
                    f.write("关键指标:\n")
                    for metric_name, metric_data in test_result["metrics"].items():
                        f.write(f"  {metric_name}:\n")
                        if isinstance(metric_data, dict):
                            for k, v in metric_data.items():
                                f.write(f"    {k}: {v}\n")
                
                f.write("\n")
            
            f.write("=" * 80 + "\n")
            f.write("报告生成完成\n")
            f.write("=" * 80 + "\n")
        
        self.log(f"摘要报告已保存: {summary_path}")
        
        # 控制台输出摘要
        print("\n" + "=" * 80)
        print("共享内存性能测试摘要")
        print("=" * 80)
        for test_name, test_result in self.results["tests"].items():
            print(f"{test_result['test_name']}: {test_result['status']}")
        print("=" * 80)
        
        return str(report_path)


def main():
    """主函数"""
    # 创建测试实例
    tester = SharedMemoryPerformanceTest(output_dir="./test_results")
    
    # 运行所有测试
    tester.run_all_tests()
    
    # 生成报告
    report_path = tester.generate_report()
    
    print(f"\n测试完成！报告路径: {report_path}")


if __name__ == "__main__":
    main()
