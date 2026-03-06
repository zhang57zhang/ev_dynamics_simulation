# EV Dynamics Simulation - 实时性验证测试套件

## 概述

本测试套件用于验证电动汽车动力学仿真系统的实时性能，包括PREEMPT_RT内核验证、共享内存性能测试和消息队列延迟测试。

**作者**: BackendAgent  
**日期**: 2026-03-06  
**版本**: 1.0

---

## 测试脚本列表

### 1. `test_realtime_prerequisite.py` - PREEMPT_RT实时性前置验证

**用途**: 验证系统是否满足实时性要求的前置条件

**测试项目**:
- ✅ 内核版本和PREEMPT_RT补丁检查
- ✅ cyclictest延迟测试（目标<200μs）
- ✅ CPU亲和性配置测试
- ✅ 内存锁定功能测试

**运行方式**:
```bash
# Linux系统（推荐使用sudo）
sudo python3 test_realtime_prerequisite.py

# 或指定输出目录
sudo python3 test_realtime_prerequisite.py --output /path/to/results
```

**输出文件**:
- `realtime_test_report_YYYYMMDD_HHMMSS.json` - JSON格式详细报告
- `realtime_test_report_YYYYMMDD_HHMMSS.txt` - 文本格式摘要报告
- `test_execution.log` - 执行日志

**性能目标**:
| 指标 | 目标值 | 说明 |
|------|--------|------|
| cyclictest最大延迟 | <200μs | 实时系统延迟要求 |
| cyclictest平均延迟 | <50μs | 实时系统平均延迟 |

**前置要求**:
- Linux操作系统
- PREEMPT_RT内核（推荐）
- rt-tests包（用于cyclictest）
- psutil库

**安装依赖**:
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install rt-tests python3-psutil

# CentOS/RHEL
sudo yum install rt-tests python3-psutil
```

---

### 2. `test_shared_memory_perf.py` - 共享内存性能测试

**用途**: 测试共享内存的读写延迟和吞吐量

**测试项目**:
- ✅ mmap共享内存读写延迟（目标<10μs）
- ✅ 吞吐量测试（目标>10K ops/s）
- ✅ 并发访问性能测试
- ✅ 跨进程共享内存测试（仅Linux）

**运行方式**:
```bash
# 基本运行
python3 test_shared_memory_perf.py

# 指定输出目录
python3 test_shared_memory_perf.py --output /path/to/results
```

**输出文件**:
- `shm_performance_report_YYYYMMDD_HHMMSS.json` - JSON格式详细报告
- `shm_performance_report_YYYYMMDD_HHMMSS.txt` - 文本格式摘要报告
- `shm_perf_test.log` - 执行日志

**性能目标**:
| 指标 | 目标值 | 说明 |
|------|--------|------|
| 读延迟 | <10μs | 共享内存读取延迟 |
| 写延迟 | <10μs | 共享内存写入延迟 |
| 吞吐量 | >10K ops/s | 每秒操作数 |

**测试配置**:
- 数据块大小: 64B, 256B, 1KB, 4KB, 16KB
- 迭代次数: 10,000次
- 并发线程数: 1, 2, 4, 8

**前置要求**:
- Python 3.6+
- psutil库

---

### 3. `test_message_queue_perf.py` - 消息队列性能测试

**用途**: 测试消息队列的延迟和吞吐量

**测试项目**:
- ✅ Python Queue延迟测试（目标<100μs）
- ✅ 吞吐量测试（目标>10K msgs/s）
- ✅ 生产者-消费者模式性能
- ✅ 多进程队列性能
- ✅ System V消息队列测试（仅Linux）

**运行方式**:
```bash
# 基本运行
python3 test_message_queue_perf.py

# 指定输出目录
python3 test_message_queue_perf.py --output /path/to/results
```

**输出文件**:
- `mq_performance_report_YYYYMMDD_HHMMSS.json` - JSON格式详细报告
- `mq_performance_report_YYYYMMDD_HHMMSS.txt` - 文本格式摘要报告
- `mq_perf_test.log` - 执行日志

**性能目标**:
| 指标 | 目标值 | 说明 |
|------|--------|------|
| 队列延迟 | <100μs | 消息队列往返延迟 |
| 吞吐量 | >10K msgs/s | 每秒消息数 |

**测试配置**:
- 消息大小: 64B, 256B, 1KB, 4KB
- 消息数量: 10,000条
- 生产者-消费者对数: 1, 2, 4

**前置要求**:
- Python 3.6+
- psutil库
- sysv-ipc库（可选，仅用于System V消息队列测试）

**安装可选依赖**:
```bash
# 安装sysv-ipc（用于System V消息队列测试）
pip install sysv-ipc
```

---

## 快速开始

### 1. 环境准备

```bash
# 克隆或下载项目
cd ev_dynamics_simulation/tests

# 安装Python依赖
pip install psutil

# （可选）安装System V IPC支持
pip install sysv-ipc

# （Linux）安装rt-tests
sudo apt-get install rt-tests
```

### 2. 运行完整测试套件

```bash
# 创建测试结果目录
mkdir -p test_results

# 1. 运行PREEMPT_RT验证（需要root权限）
sudo python3 test_realtime_prerequisite.py

# 2. 运行共享内存性能测试
python3 test_shared_memory_perf.py

# 3. 运行消息队列性能测试
python3 test_message_queue_perf.py
```

### 3. 查看测试报告

```bash
# 查看最新的测试报告
ls -lt test_results/*.txt | head -n 3

# 查看JSON报告（使用jq格式化）
cat test_results/realtime_test_report_*.json | jq .
```

---

## 测试报告说明

### JSON报告结构

所有测试脚本生成的JSON报告遵循以下结构：

```json
{
  "test_time": "2026-03-06T11:21:00",
  "platform": "Linux-5.15.0-rt-x86_64",
  "python_version": "3.10.6",
  "tests": {
    "test_name": {
      "test_name": "...",
      "status": "PASSED|FAILED|WARNING|SKIPPED|ERROR",
      "metrics": { ... },
      "details": { ... },
      "messages": [ ... ]
    }
  },
  "targets": { ... }
}
```

### 测试状态说明

| 状态 | 说明 |
|------|------|
| PASSED | 测试通过，达到性能目标 |
| FAILED | 测试失败，未达到性能目标 |
| WARNING | 测试完成，但存在警告 |
| SKIPPED | 测试被跳过（如不支持的系统） |
| ERROR | 测试过程中发生错误 |

---

## 性能优化建议

### 提高实时性能

1. **使用PREEMPT_RT内核**
   ```bash
   # Ubuntu安装RT内核
   sudo apt-get install linux-image-rt-amd64
   sudo reboot
   ```

2. **配置CPU隔离**
   ```bash
   # 编辑/etc/default/grub
   GRUB_CMDLINE_LINUX="isolcpus=2,3"
   
   # 更新grub
   sudo update-grub
   sudo reboot
   ```

3. **设置内存锁定限制**
   ```bash
   # 编辑/etc/security/limits.conf
   * hard memlock unlimited
   * soft memlock unlimited
   ```

4. **禁用CPU频率调节**
   ```bash
   # 设置性能模式
   sudo cpupower frequency-set -g performance
   ```

### 提高共享内存性能

1. **使用大页内存（Huge Pages）**
   ```bash
   # 配置大页内存
   echo 1024 > /proc/sys/vm/nr_hugepages
   ```

2. **绑定CPU亲和性**
   ```python
   import psutil
   p = psutil.Process()
   p.cpu_affinity([2])  # 绑定到CPU 2
   ```

### 提高消息队列性能

1. **调整队列大小**
   ```python
   # 根据实际需求调整队列大小
   q = queue.Queue(maxsize=10000)
   ```

2. **使用多进程代替多线程**（CPU密集型任务）

---

## 故障排查

### 问题1: cyclictest未找到

**错误**: `cyclictest: command not found`

**解决方案**:
```bash
sudo apt-get install rt-tests
```

### 问题2: 权限不足

**错误**: `Permission denied` 或延迟测试结果不准确

**解决方案**:
```bash
# 使用sudo运行
sudo python3 test_realtime_prerequisite.py

# 或配置capabilities
sudo setcap cap_sys_nice+ep /usr/bin/python3
```

### 问题3: 内存锁定失败

**错误**: `mlock failed: Cannot allocate memory`

**解决方案**:
```bash
# 增加内存锁定限制
ulimit -l unlimited

# 或编辑/etc/security/limits.conf
* hard memlock unlimited
* soft memlock unlimited
```

### 问题4: System V消息队列测试失败

**错误**: `ImportError: No module named 'sysv_ipc'`

**解决方案**:
```bash
# 安装sysv-ipc
pip install sysv-ipc

# 或跳过此测试（不影响主要功能）
```

---

## 测试报告模板

### 实时性验证报告模板

```
============================================================
EV动力学仿真系统 - 实时性验证报告
============================================================

测试时间: YYYY-MM-DD HH:MM:SS
测试人员: [姓名]
系统配置: [硬件/软件配置]

------------------------------------------------------------
1. PREEMPT_RT内核验证
------------------------------------------------------------
[ ] 内核版本: _______________
[ ] PREEMPT_RT补丁: [已安装/未安装]
[ ] 配置项检查:
    - CONFIG_PREEMPT_RT: [Yes/No]
    - CONFIG_HIGH_RES_TIMERS: [Yes/No]

------------------------------------------------------------
2. 延迟测试结果
------------------------------------------------------------
[ ] cyclictest最大延迟: _____ μs (目标<200μs)
[ ] cyclictest平均延迟: _____ μs (目标<50μs)

------------------------------------------------------------
3. 资源配置验证
------------------------------------------------------------
[ ] CPU亲和性: [通过/失败]
[ ] 内存锁定: [通过/失败]
[ ] CPU隔离: [已配置/未配置]

------------------------------------------------------------
4. 共享内存性能
------------------------------------------------------------
[ ] 读延迟: _____ μs (目标<10μs)
[ ] 写延迟: _____ μs (目标<10μs)
[ ] 吞吐量: _____ ops/s (目标>10K ops/s)

------------------------------------------------------------
5. 消息队列性能
------------------------------------------------------------
[ ] 队列延迟: _____ μs (目标<100μs)
[ ] 吞吐量: _____ msgs/s (目标>10K msgs/s)

------------------------------------------------------------
6. 总体评估
------------------------------------------------------------
[ ] 通过 - 满足所有性能目标
[ ] 条件通过 - 满足大部分性能目标
[ ] 失败 - 未达到性能要求

备注:
____________________________________________________________
____________________________________________________________

签名: _______________ 日期: _______________
```

---

## 持续集成

### GitHub Actions配置示例

```yaml
name: Realtime Performance Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Set up Python
      uses: actions/setup-python@v2
      with:
        python-version: 3.9
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y rt-tests
        pip install psutil
    
    - name: Run shared memory tests
      run: |
        cd tests
        python3 test_shared_memory_perf.py
    
    - name: Run message queue tests
      run: |
        cd tests
        python3 test_message_queue_perf.py
    
    - name: Upload test results
      uses: actions/upload-artifact@v2
      with:
        name: test-results
        path: tests/test_results/
```

---

## 参考资料

1. **PREEMPT_RT官方文档**: https://wiki.linuxfoundation.org/realtime/
2. **cyclictest使用指南**: https://wiki.linuxfoundation.org/realtime/documentation/howto/tools/cyclictest
3. **Python性能优化**: https://wiki.python.org/moin/PythonSpeed/PerformanceTips
4. **实时Linux系统调优**: https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/

---

## 联系方式

如有问题或建议，请联系:
- 项目维护者: BackendAgent
- 问题反馈: [GitHub Issues]

---

**最后更新**: 2026-03-06
