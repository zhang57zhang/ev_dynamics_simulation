#!/usr/bin/env python3
"""
消息队列性能测试脚本

测试项目：
1. 消息队列延迟（目标<100μs）
2. 吞吐量（目标>10K msgs/s）
3. 不同消息大小的性能
4. 性能报告生成

作者：BackendAgent
日期：2026-03-06
版本：1.0
"""

import os
import sys
import time
import json
import queue
import platform
import threading
import multiprocessing as mp
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Tuple, Optional
from statistics import mean, median, stdev
from dataclasses import dataclass, asdict
import ctypes


@dataclass
class MessageQueueMetrics:
    """消息队列性能指标数据类"""
    test_name: str
    message_count: int
    message_size_bytes: int
    total_time_sec: float
    throughput_msgs_per_sec: float
    avg_latency_us: float
    min_latency_us: float
    max_latency_us: float
    median_latency_us: float
    p95_latency_us: float
    p99_latency_us: float
    bandwidth_mbps: float


class MessageQueuePerformanceTest:
    """消息队列性能测试类"""
    
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
                "latency_us": 100,
                "throughput_msgs_per_sec": 10000
            }
        }
        
        # 测试配置
        self.test_config = {
            "message_sizes": [64, 256, 1024, 4096],  # 消息大小（字节）
            "message_count": 10000,  # 消息数量
            "warmup_count": 100,  # 预热消息数
            "queue_sizes": [0, 100, 1000],  # 队列大小（0表示无限）
            "producer_consumer_pairs": [1, 2, 4]  # 生产者-消费者对数
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
        log_file = self.output_dir / "mq_perf_test.log"
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
    
    def test_queue_latency(self) -> Dict:
        """
        测试Python Queue延迟
        
        Returns:
            测试结果字典
        """
        self.log("=" * 60)
        self.log("测试1: Python Queue延迟测试")
        self.log("=" * 60)
        
        result = {
            "test_name": "queue_latency",
            "status": "UNKNOWN",
            "metrics": {},
            "details": {},
            "messages": []
        }
        
        try:
            message_count = self.test_config["message_count"]
            warmup_count = self.test_config["warmup_count"]
            
            # 测试不同消息大小
            for msg_size in self.test_config["message_sizes"]:
                self.log(f"\n--- 消息大小: {msg_size} bytes ---")
                
                # 创建队列
                test_queue = queue.Queue()
                
                # 准备测试消息
                test_message = os.urandom(msg_size)
                
                # 预热
                for _ in range(warmup_count):
                    test_queue.put(test_message)
                    _ = test_queue.get()
                
                # 正式测试 - 测量put+get的往返延迟
                latencies = []
                start_total = time.perf_counter()
                
                for _ in range(message_count):
                    # 测量put延迟
                    start_put = time.perf_counter()
                    test_queue.put(test_message)
                    end_put = time.perf_counter()
                    put_latency = (end_put - start_put) * 1_000_000
                    
                    # 测量get延迟
                    start_get = time.perf_counter()
                    _ = test_queue.get()
                    end_get = time.perf_counter()
                    get_latency = (end_get - start_get) * 1_000_000
                    
                    # 总延迟
                    latencies.append(put_latency + get_latency)
                
                total_time = time.perf_counter() - start_total
                
                # 计算指标
                metrics = MessageQueueMetrics(
                    test_name=f"queue_latency_{msg_size}B",
                    message_count=message_count,
                    message_size_bytes=msg_size,
                    total_time_sec=total_time,
                    throughput_msgs_per_sec=message_count / total_time,
                    avg_latency_us=mean(latencies),
                    min_latency_us=min(latencies),
                    max_latency_us=max(latencies),
                    median_latency_us=median(latencies),
                    p95_latency_us=self.calculate_percentile(latencies, 95),
                    p99_latency_us=self.calculate_percentile(latencies, 99),
                    bandwidth_mbps=(msg_size * message_count / total_time) / (1024 * 1024)
                )
                
                result["metrics"][f"size_{msg_size}"] = asdict(metrics)
                
                self.log(f"吞吐量: {metrics.throughput_msgs_per_sec:.2f} msgs/s")
                self.log(f"平均延迟: {metrics.avg_latency_us:.2f} μs")
                self.log(f"最小延迟: {metrics.min_latency_us:.2f} μs")
                self.log(f"最大延迟: {metrics.max_latency_us:.2f} μs")
                self.log(f"中位数延迟: {metrics.median_latency_us:.2f} μs")
                self.log(f"P95延迟: {metrics.p95_latency_us:.2f} μs")
                self.log(f"P99延迟: {metrics.p99_latency_us:.2f} μs")
                self.log(f"带宽: {metrics.bandwidth_mbps:.2f} MB/s")
            
            # 检查是否达到性能目标
            target_latency = self.results["targets"]["latency_us"]
            target_throughput = self.results["targets"]["throughput_msgs_per_sec"]
            
            # 使用1KB消息的结果作为基准
            benchmark_metrics = result["metrics"].get("size_1024", {})
            
            if (benchmark_metrics.get("avg_latency_us", float('inf')) < target_latency and
                benchmark_metrics.get("throughput_msgs_per_sec", 0) >= target_throughput):
                result["status"] = "PASSED"
                result["messages"].append(
                    f"性能达标 (延迟<{target_latency}μs, 吞吐量>{target_throughput} msgs/s)"
                )
                self.log(f"\n✓ 性能达标", "INFO")
            else:
                result["status"] = "FAILED"
                result["messages"].append(
                    f"性能未达标 (延迟<{target_latency}μs, 吞吐量>{target_throughput} msgs/s)"
                )
                self.log(f"\n✗ 性能未达标", "ERROR")
            
        except Exception as e:
            result["status"] = "ERROR"
            result["messages"].append(f"测试异常: {str(e)}")
            self.log(f"错误: 测试异常: {e}", "ERROR")
            import traceback
            self.log(traceback.format_exc(), "ERROR")
        
        return result
    
    def producer_worker(self, msg_queue: queue.Queue, message: bytes, 
                        count: int, result_dict: Dict, worker_id: int):
        """
        生产者工作线程
        
        Args:
            msg_queue: 消息队列
            message: 测试消息
            count: 消息数量
            result_dict: 结果字典
            worker_id: 工作线程ID
        """
        try:
            latencies = []
            start_total = time.perf_counter()
            
            for _ in range(count):
                start = time.perf_counter()
                msg_queue.put(message)
                end = time.perf_counter()
                latencies.append((end - start) * 1_000_000)
            
            total_time = time.perf_counter() - start_total
            
            result_dict[worker_id] = {
                "role": "producer",
                "message_count": count,
                "total_time_sec": total_time,
                "avg_latency_us": mean(latencies),
                "throughput_msgs_per_sec": count / total_time
            }
        except Exception as e:
            result_dict[worker_id] = {
                "role": "producer",
                "error": str(e)
            }
    
    def consumer_worker(self, msg_queue: queue.Queue, count: int, 
                       result_dict: Dict, worker_id: int):
        """
        消费者工作线程
        
        Args:
            msg_queue: 消息队列
            count: 消息数量
            result_dict: 结果字典
            worker_id: 工作线程ID
        """
        try:
            latencies = []
            start_total = time.perf_counter()
            
            for _ in range(count):
                start = time.perf_counter()
                _ = msg_queue.get()
                end = time.perf_counter()
                latencies.append((end - start) * 1_000_000)
            
            total_time = time.perf_counter() - start_total
            
            result_dict[worker_id] = {
                "role": "consumer",
                "message_count": count,
                "total_time_sec": total_time,
                "avg_latency_us": mean(latencies),
                "throughput_msgs_per_sec": count / total_time
            }
        except Exception as e:
            result_dict[worker_id] = {
                "role": "consumer",
                "error": str(e)
            }
    
    def test_producer_consumer(self) -> Dict:
        """
        测试生产者-消费者模式性能
        
        Returns:
            测试结果字典
        """
        self.log("\n" + "=" * 60)
        self.log("测试2: 生产者-消费者模式性能测试")
        self.log("=" * 60)
        
        result = {
            "test_name": "producer_consumer",
            "status": "UNKNOWN",
            "metrics": {},
            "details": {},
            "messages": []
        }
        
        try:
            msg_size = 1024  # 1KB消息
            messages_per_pair = 2000
            test_message = os.urandom(msg_size)
            
            concurrent_results = {}
            
            for pair_count in self.test_config["producer_consumer_pairs"]:
                self.log(f"\n--- 生产者-消费者对数: {pair_count} ---")
                
                # 创建队列
                test_queue = queue.Queue(maxsize=1000)
                
                # 结果收集
                result_dict = {}
                threads = []
                
                # 创建生产者和消费者线程
                for i in range(pair_count):
                    # 生产者
                    producer = threading.Thread(
                        target=self.producer_worker,
                        args=(test_queue, test_message, messages_per_pair, 
                              result_dict, f"producer_{i}")
                    )
                    threads.append(producer)
                    
                    # 消费者
                    consumer = threading.Thread(
                        target=self.consumer_worker,
                        args=(test_queue, messages_per_pair, 
                              result_dict, f"consumer_{i}")
                    )
                    threads.append(consumer)
                
                start_total = time.perf_counter()
                
                # 启动所有线程
                for t in threads:
                    t.start()
                
                # 等待所有线程完成
                for t in threads:
                    t.join(timeout=30)
                
                total_time = time.perf_counter() - start_total
                
                # 统计结果
                total_messages = messages_per_pair * pair_count
                producer_results = [r for k, r in result_dict.items() 
                                   if k.startswith("producer_") and "error" not in r]
                consumer_results = [r for k, r in result_dict.items() 
                                   if k.startswith("consumer_") and "error" not in r]
                
                if producer_results and consumer_results:
                    avg_producer_latency = mean([r["avg_latency_us"] for r in producer_results])
                    avg_consumer_latency = mean([r["avg_latency_us"] for r in consumer_results])
                    
                    concurrent_results[pair_count] = {
                        "pair_count": pair_count,
                        "total_messages": total_messages,
                        "total_time_sec": total_time,
                        "throughput_msgs_per_sec": total_messages / total_time,
                        "avg_producer_latency_us": avg_producer_latency,
                        "avg_consumer_latency_us": avg_consumer_latency,
                        "total_threads": len(threads)
                    }
                    
                    self.log(f"总消息数: {total_messages}")
                    self.log(f"总时间: {total_time:.2f} s")
                    self.log(f"吞吐量: {total_messages / total_time:.2f} msgs/s")
                    self.log(f"平均生产者延迟: {avg_producer_latency:.2f} μs")
                    self.log(f"平均消费者延迟: {avg_consumer_latency:.2f} μs")
                else:
                    self.log("错误: 部分线程执行失败", "ERROR")
            
            result["details"]["concurrent_results"] = concurrent_results
            
            # 检查并发性能
            if concurrent_results:
                max_throughput = max(r["throughput_msgs_per_sec"] 
                                    for r in concurrent_results.values())
                
                target_throughput = self.results["targets"]["throughput_msgs_per_sec"]
                
                if max_throughput >= target_throughput:
                    result["status"] = "PASSED"
                    result["messages"].append(
                        f"并发吞吐量达标 ({max_throughput:.2f} msgs/s)"
                    )
                    self.log(f"\n✓ 并发吞吐量达标", "INFO")
                else:
                    result["status"] = "WARNING"
                    result["messages"].append(
                        f"并发吞吐量偏低 ({max_throughput:.2f} msgs/s)"
                    )
                    self.log(f"\n⚠ 并发吞吐量偏低", "WARNING")
            else:
                result["status"] = "FAILED"
                result["messages"].append("并发测试失败")
            
        except Exception as e:
            result["status"] = "ERROR"
            result["messages"].append(f"测试异常: {str(e)}")
            self.log(f"错误: 测试异常: {e}", "ERROR")
            import traceback
            self.log(traceback.format_exc(), "ERROR")
        
        return result
    
    def test_multiprocessing_queue(self) -> Dict:
        """
        测试多进程队列性能
        
        Returns:
            测试结果字典
        """
        self.log("\n" + "=" * 60)
        self.log("测试3: 多进程队列性能测试")
        self.log("=" * 60)
        
        result = {
            "test_name": "multiprocessing_queue",
            "status": "UNKNOWN",
            "metrics": {},
            "details": {},
            "messages": []
        }
        
        try:
            msg_size = 1024  # 1KB
            message_count = 2000
            
            self.log(f"消息大小: {msg_size} bytes")
            self.log(f"消息数量: {message_count}")
            
            def mp_producer(queue_obj, msg, count, result_queue):
                """多进程生产者"""
                try:
                    latencies = []
                    start_total = time.perf_counter()
                    
                    for _ in range(count):
                        start = time.perf_counter()
                        queue_obj.put(msg)
                        end = time.perf_counter()
                        latencies.append((end - start) * 1_000_000)
                    
                    total_time = time.perf_counter() - start_total
                    
                    result_queue.put({
                        "role": "producer",
                        "count": count,
                        "total_time": total_time,
                        "avg_latency": mean(latencies),
                        "throughput": count / total_time
                    })
                except Exception as e:
                    result_queue.put({"role": "producer", "error": str(e)})
            
            def mp_consumer(queue_obj, count, result_queue):
                """多进程消费者"""
                try:
                    latencies = []
                    start_total = time.perf_counter()
                    
                    for _ in range(count):
                        start = time.perf_counter()
                        _ = queue_obj.get()
                        end = time.perf_counter()
                        latencies.append((end - start) * 1_000_000)
                    
                    total_time = time.perf_counter() - start_total
                    
                    result_queue.put({
                        "role": "consumer",
                        "count": count,
                        "total_time": total_time,
                        "avg_latency": mean(latencies),
                        "throughput": count / total_time
                    })
                except Exception as e:
                    result_queue.put({"role": "consumer", "error": str(e)})
            
            # 创建多进程队列
            mp_queue = mp.Queue()
            result_queue = mp.Queue()
            test_message = os.urandom(msg_size)
            
            # 启动生产者和消费者进程
            producer_proc = mp.Process(
                target=mp_producer,
                args=(mp_queue, test_message, message_count, result_queue)
            )
            consumer_proc = mp.Process(
                target=mp_consumer,
                args=(mp_queue, message_count, result_queue)
            )
            
            start_total = time.perf_counter()
            
            producer_proc.start()
            time.sleep(0.1)  # 让生产者先启动
            consumer_proc.start()
            
            producer_proc.join(timeout=30)
            consumer_proc.join(timeout=30)
            
            total_time = time.perf_counter() - start_total
            
            # 收集结果
            process_results = []
            while not result_queue.empty():
                process_results.append(result_queue.get())
            
            result["details"]["process_results"] = process_results
            result["details"]["total_time_sec"] = total_time
            
            # 分析结果
            for proc_result in process_results:
                if "error" not in proc_result:
                    self.log(f"{proc_result['role']}: "
                            f"平均延迟 {proc_result['avg_latency']:.2f} μs, "
                            f"吞吐量 {proc_result['throughput']:.2f} msgs/s")
                else:
                    self.log(f"{proc_result['role']}: 错误 - {proc_result['error']}", "ERROR")
            
            # 检查测试结果
            successful_results = [r for r in process_results if "error" not in r]
            
            if len(successful_results) == 2:
                avg_latency = mean([r["avg_latency"] for r in successful_results])
                target_latency = self.results["targets"]["latency_us"]
                
                if avg_latency < target_latency:
                    result["status"] = "PASSED"
                    result["messages"].append(
                        f"多进程队列延迟达标 ({avg_latency:.2f} μs < {target_latency} μs)"
                    )
                    self.log(f"\n✓ 多进程队列性能达标", "INFO")
                else:
                    result["status"] = "WARNING"
                    result["messages"].append(
                        f"多进程队列延迟偏高 ({avg_latency:.2f} μs >= {target_latency} μs)"
                    )
                    self.log(f"\n⚠ 多进程队列延迟偏高", "WARNING")
            else:
                result["status"] = "FAILED"
                result["messages"].append("多进程队列测试失败")
                self.log("\n✗ 多进程队列测试失败", "ERROR")
            
        except Exception as e:
            result["status"] = "ERROR"
            result["messages"].append(f"测试异常: {str(e)}")
            self.log(f"错误: 测试异常: {e}", "ERROR")
            import traceback
            self.log(traceback.format_exc(), "ERROR")
        
        return result
    
    def test_system_v_mq(self) -> Dict:
        """
        测试System V消息队列性能（仅Linux）
        
        Returns:
            测试结果字典
        """
        self.log("\n" + "=" * 60)
        self.log("测试4: System V消息队列性能测试")
        self.log("=" * 60)
        
        result = {
            "test_name": "system_v_mq",
            "status": "UNKNOWN",
            "metrics": {},
            "details": {},
            "messages": []
        }
        
        # 检查是否为Linux系统
        if platform.system() != "Linux":
            result["status"] = "SKIPPED"
            result["messages"].append("System V消息队列仅支持Linux")
            self.log("提示: System V消息队列仅支持Linux系统", "INFO")
            return result
        
        try:
            import sysv_ipc
            
            # 创建System V消息队列
            key = 0x1234
            mq = sysv_ipc.MessageQueue(key, flags=sysv_ipc.IPC_CREX)
            
            message_count = 1000
            msg_size = 1024
            test_message = os.urandom(msg_size)
            
            self.log(f"消息大小: {msg_size} bytes")
            self.log(f"消息数量: {message_count}")
            
            # 测试发送延迟
            send_latencies = []
            start_total = time.perf_counter()
            
            for i in range(message_count):
                start = time.perf_counter()
                mq.send(test_message, type=1)
                end = time.perf_counter()
                send_latencies.append((end - start) * 1_000_000)
            
            send_time = time.perf_counter() - start_total
            
            # 测试接收延迟
            recv_latencies = []
            start_total = time.perf_counter()
            
            for _ in range(message_count):
                start = time.perf_counter()
                _, _ = mq.receive(type=1)
                end = time.perf_counter()
                recv_latencies.append((end - start) * 1_000_000)
            
            recv_time = time.perf_counter() - start_total
            
            # 计算指标
            result["metrics"]["send"] = {
                "avg_latency_us": mean(send_latencies),
                "min_latency_us": min(send_latencies),
                "max_latency_us": max(send_latencies),
                "throughput_msgs_per_sec": message_count / send_time
            }
            
            result["metrics"]["receive"] = {
                "avg_latency_us": mean(recv_latencies),
                "min_latency_us": min(recv_latencies),
                "max_latency_us": max(recv_latencies),
                "throughput_msgs_per_sec": message_count / recv_time
            }
            
            self.log(f"发送平均延迟: {result['metrics']['send']['avg_latency_us']:.2f} μs")
            self.log(f"发送吞吐量: {result['metrics']['send']['throughput_msgs_per_sec']:.2f} msgs/s")
            self.log(f"接收平均延迟: {result['metrics']['receive']['avg_latency_us']:.2f} μs")
            self.log(f"接收吞吐量: {result['metrics']['receive']['throughput_msgs_per_sec']:.2f} msgs/s")
            
            # 检查性能
            avg_latency = (result['metrics']['send']['avg_latency_us'] + 
                          result['metrics']['receive']['avg_latency_us']) / 2
            target_latency = self.results["targets"]["latency_us"]
            
            if avg_latency < target_latency:
                result["status"] = "PASSED"
                result["messages"].append(
                    f"System V MQ延迟达标 ({avg_latency:.2f} μs < {target_latency} μs)"
                )
                self.log(f"\n✓ System V MQ性能达标", "INFO")
            else:
                result["status"] = "WARNING"
                result["messages"].append(
                    f"System V MQ延迟偏高 ({avg_latency:.2f} μs >= {target_latency} μs)"
                )
                self.log(f"\n⚠ System V MQ延迟偏高", "WARNING")
            
            # 清理
            mq.remove()
            
        except ImportError:
            result["status"] = "SKIPPED"
            result["messages"].append("需要安装sysv_ipc模块")
            self.log("提示: 需要安装sysv_ipc模块 (pip install sysv-ipc)", "WARNING")
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
        self.log("消息队列性能测试开始")
        self.log("=" * 80)
        
        # 执行各项测试
        self.results["tests"]["latency"] = self.test_queue_latency()
        self.results["tests"]["producer_consumer"] = self.test_producer_consumer()
        self.results["tests"]["multiprocessing"] = self.test_multiprocessing_queue()
        self.results["tests"]["system_v"] = self.test_system_v_mq()
        
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
            output_file = f"mq_performance_report_{timestamp}.json"
        
        report_path = self.output_dir / output_file
        
        # 保存JSON报告
        with open(report_path, "w", encoding="utf-8") as f:
            json.dump(self.results, f, indent=2, ensure_ascii=False)
        
        self.log(f"JSON报告已保存: {report_path}")
        
        # 生成文本摘要
        summary_path = report_path.with_suffix(".txt")
        
        with open(summary_path, "w", encoding="utf-8") as f:
            f.write("=" * 80 + "\n")
            f.write("消息队列性能测试报告\n")
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
        print("消息队列性能测试摘要")
        print("=" * 80)
        for test_name, test_result in self.results["tests"].items():
            print(f"{test_result['test_name']}: {test_result['status']}")
        print("=" * 80)
        
        return str(report_path)


def main():
    """主函数"""
    # 创建测试实例
    tester = MessageQueuePerformanceTest(output_dir="./test_results")
    
    # 运行所有测试
    tester.run_all_tests()
    
    # 生成报告
    report_path = tester.generate_report()
    
    print(f"\n测试完成！报告路径: {report_path}")


if __name__ == "__main__":
    main()
