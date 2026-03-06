# 快速使用指南

## 快速开始（3步）

### 1️⃣ 安装依赖

```bash
# 安装Python依赖
pip install psutil

# （可选）安装System V IPC支持
pip install sysv-ipc

# （Linux）安装rt-tests
sudo apt-get install rt-tests
```

### 2️⃣ 运行测试

```bash
# 方式1: 运行所有测试（推荐）
cd E:\workspace\ev_dynamics_simulation\tests
python run_all_tests.py

# 方式2: 单独运行测试
sudo python test_realtime_prerequisite.py  # 需要root权限
python test_shared_memory_perf.py
python test_message_queue_perf.py
```

### 3️⃣ 查看报告

```bash
# 测试报告保存在 test_results/ 目录
ls test_results/

# 查看最新的报告
cat test_results/*.txt
```

---

## 测试脚本说明

### 📋 test_realtime_prerequisite.py
**用途**: 验证PREEMPT_RT实时性前置条件  
**时间**: 约30秒  
**权限**: 需要root（sudo）  
**输出**: 
- `realtime_test_report_*.json`
- `realtime_test_report_*.txt`

### 📋 test_shared_memory_perf.py
**用途**: 测试共享内存性能  
**时间**: 约60秒  
**权限**: 普通用户  
**输出**: 
- `shm_performance_report_*.json`
- `shm_performance_report_*.txt`

### 📋 test_message_queue_perf.py
**用途**: 测试消息队列性能  
**时间**: 约45秒  
**权限**: 普通用户  
**输出**: 
- `mq_performance_report_*.json`
- `mq_performance_report_*.txt`

---

## 性能目标

| 测试项目 | 目标值 | 说明 |
|---------|--------|------|
| cyclictest最大延迟 | <200μs | 实时系统要求 |
| 共享内存读写延迟 | <10μs | 共享内存性能 |
| 消息队列延迟 | <100μs | IPC性能 |
| 吞吐量 | >10K ops/s | 处理能力 |

---

## 常见问题

**Q: cyclictest未找到？**  
A: `sudo apt-get install rt-tests`

**Q: 权限不足？**  
A: 使用 `sudo python run_all_tests.py`

**Q: Windows系统？**  
A: 部分测试仅支持Linux，但共享内存和消息队列测试可在Windows运行

**Q: 如何解读测试结果？**  
A: 查看生成的 `.txt` 报告文件，或使用 `jq` 工具解析JSON报告

---

## 文件清单

```
tests/
├── test_realtime_prerequisite.py   # PREEMPT_RT验证脚本
├── test_shared_memory_perf.py      # 共享内存性能测试
├── test_message_queue_perf.py      # 消息队列性能测试
├── run_all_tests.py                # 一键运行所有测试
├── README.md                       # 详细文档
├── QUICKSTART.md                   # 本文件
├── test_report_template.md         # 测试报告模板
└── test_results/                   # 测试结果目录（自动创建）
    ├── *.json                      # JSON格式报告
    ├── *.txt                       # 文本格式报告
    └── *.log                       # 执行日志
```

---

**立即开始测试！** 🚀

```bash
python run_all_tests.py
```
