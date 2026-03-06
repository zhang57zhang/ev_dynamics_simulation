# 故障排查指南

**版本**: 1.0  
**更新日期**: 2026-03-06

---

## 目录

1. [故障排查流程](#1-故障排查流程)
2. [编译故障](#2-编译故障)
3. [运行时故障](#3-运行时故障)
4. [性能故障](#4-性能故障)
5. [HIL通信故障](#5-hil通信故障)
6. [Python绑定故障](#6-python绑定故障)
7. [系统故障](#7-系统故障)
8. [故障诊断工具](#8-故障诊断工具)

---

## 1. 故障排查流程

### 1.1 标准排查流程

```
┌──────────────┐
│  发现故障     │
└──────┬───────┘
       │
       v
┌──────────────┐
│  收集信息     │ ← 错误日志、系统状态、复现步骤
└──────┬───────┘
       │
       v
┌──────────────┐
│  定位问题     │ ← 查看本文档、搜索FAQ
└──────┬───────┘
       │
       v
┌──────────────┐    NO
│  找到解决方案？├─────────> 联系技术支持
└──────┬───────┘
       │ YES
       v
┌──────────────┐
│  执行修复     │
└──────┬───────┘
       │
       v
┌──────────────┐    NO
│  问题解决？   ├─────────> 回滚 + 寻求帮助
└──────┬───────┘
       │ YES
       v
┌──────────────┐
│  记录文档     │ ← 更新故障库
└──────────────┘
```

### 1.2 信息收集清单

在报告故障前，请收集以下信息：

- [ ] 错误信息完整输出
- [ ] 系统环境（OS版本、内核版本）
- [ ] 编译器和CMake版本
- [ ] 依赖库版本
- [ ] 配置文件内容
- [ ] 复现步骤
- [ ] 最近更改记录

---

## 2. 编译故障

### 2.1 CMake配置失败

#### 错误：Could not find Eigen3

**症状**:
```
CMake Error at CMakeLists.txt:15 (find_package):
  Could not find a package configuration file provided by "Eigen3" with
  any of the following names:
    Eigen3Config.cmake
    eigen3-config.cmake
```

**诊断**:
```bash
# 检查Eigen3是否安装
dpkg -l | grep libeigen3-dev

# 查找Eigen3位置
find /usr -name "Eigen3Config.cmake" 2>/dev/null
```

**解决方案**:

**方法1**: 安装系统包
```bash
sudo apt update
sudo apt install libeigen3-dev
```

**方法2**: 手动指定路径
```bash
cmake .. -DEigen3_DIR=/usr/lib/cmake/eigen3-3.3.7
```

**方法3**: 使用vcpkg
```bash
vcpkg install eigen3:x64-linux
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

#### 错误：Python版本不匹配

**症状**:
```
CMake Error: Python interpreter version 3.8 does not match required version 3.10
```

**诊断**:
```bash
# 检查Python版本
python3 --version
which python3

# 检查可用Python版本
ls -la /usr/bin/python*
```

**解决方案**:

**方法1**: 安装Python 3.10+
```bash
sudo apt install python3.10 python3.10-dev
```

**方法2**: 指定Python路径
```bash
cmake .. -DPYTHON_EXECUTABLE=/usr/bin/python3.10
```

---

### 2.2 编译错误

#### 错误：undefined reference to `...

**症状**:
```
undefined reference to `ev_dynamics::VehicleDynamics::update(double)'
```

**诊断**:
```bash
# 检查库文件是否存在
find build -name "*.a" -o -name "*.so"

# 检查符号
nm build/lib/libev_dynamics.so | grep update
```

**解决方案**:

**方法1**: 重新编译
```bash
make clean
make -j$(nproc)
```

**方法2**: 检查链接顺序
```cmake
# 确保库的链接顺序正确
target_link_libraries(ev_dynamics
    PRIVATE
        powertrain
        chassis
        dynamics
        tire
        scheduler
)
```

---

#### 错误：内存不足（编译器崩溃）

**症状**:
```
c++: internal compiler error: Killed (program cc1plus)
```

**诊断**:
```bash
# 检查内存使用
free -h

# 检查交换空间
swapon --show
```

**解决方案**:

**方法1**: 限制并行编译
```bash
make -j2  # 只用2个核心
```

**方法2**: 增加交换空间
```bash
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

---

### 2.3 Python绑定编译失败

#### 错误：pybind11 not found

**症状**:
```
CMake Error: Could not find pybind11
```

**诊断**:
```bash
# 检查pybind11安装
pip show pybind11

# 获取CMake路径
python3 -c "import pybind11; print(pybind11.get_cmake_dir())"
```

**解决方案**:

**方法1**: 安装pybind11
```bash
pip install pybind11
```

**方法2**: 指定CMake路径
```bash
cmake .. -Dpybind11_DIR=$(python3 -c "import pybind11; print(pybind11.get_cmake_dir())")
```

---

## 3. 运行时故障

### 3.1 程序无法启动

#### 错误：找不到共享库

**症状**:
```
error while loading shared libraries: libev_dynamics.so: cannot open shared object file
```

**诊断**:
```bash
# 查找库文件
find /usr/local -name "libev_dynamics.so"

# 检查库路径
ldconfig -p | grep ev_dynamics
```

**解决方案**:

**方法1**: 临时设置
```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
./build/bin/ev_dynamics_simulation
```

**方法2**: 永久设置
```bash
# 更新链接器缓存
sudo ldconfig

# 或添加到配置
echo "/usr/local/lib" | sudo tee /etc/ld.so.conf.d/ev_dynamics.conf
sudo ldconfig
```

---

#### 错误：配置文件未找到

**症状**:
```
Error: Cannot open config file: simulation_config.yaml
```

**诊断**:
```bash
# 检查文件是否存在
ls -la config/simulation_config.yaml

# 检查当前目录
pwd
```

**解决方案**:

**方法1**: 使用绝对路径
```bash
./build/bin/ev_dynamics_simulation --config /absolute/path/to/simulation_config.yaml
```

**方法2**: 设置环境变量
```bash
export EV_DYNAMICS_CONFIG=/path/to/config
./build/bin/ev_dynamics_simulation
```

---

### 3.2 程序崩溃

#### 错误：Segmentation Fault

**症状**:
```
Segmentation fault (core dumped)
```

**诊断**:

**步骤1**: 使用调试版本
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
gdb ./build/bin/ev_dynamics_simulation
(gdb) run
# 崩溃时执行
(gdb) bt
```

**步骤2**: 检查核心转储
```bash
# 启用核心转储
ulimit -c unlimited

# 运行程序
./build/bin/ev_dynamics_simulation

# 分析核心文件
gdb ./build/bin/ev_dynamics_simulation core
```

**常见原因**:

1. **空指针访问**
```cpp
// 错误示例
VehicleState* state = nullptr;
state->speed;  // 崩溃
```

2. **数组越界**
```cpp
std::vector<double> data(10);
data[20];  // 崩溃
```

3. **栈溢出**
```cpp
// 递归过深
void infinite_recursion() {
    infinite_recursion();  // 崩溃
}
```

**解决方案**:
- 添加空指针检查
- 使用边界检查（`at()`而不是`[]`）
- 增加栈大小或改用堆分配

---

#### 错误：数值异常（NaN/Inf）

**症状**:
```
Warning: NaN detected in vehicle speed
```

**诊断**:
```python
import numpy as np

state = vehicle.get_state()

# 检查NaN
if np.isnan(state.speed):
    print("❌ 速度为NaN")

# 检查Inf
if np.isinf(state.acceleration):
    print("❌ 加速度为Inf")
```

**常见原因**:

1. **除零**
```python
x = 0.0
y = 1.0 / x  # Inf
```

2. **时间步长过大**
```python
dt = 1.0  # 太大！
vehicle.update(dt)
```

3. **参数不合理**
```python
vehicle.mass = -100  # 质量不能为负
```

**解决方案**:

```python
# 1. 添加数值检查
def safe_update(vehicle, dt):
    vehicle.update(dt)
    state = vehicle.get_state()
    
    if np.isnan(state.speed) or np.isinf(state.speed):
        raise ValueError(f"数值异常: speed={state.speed}")
    
    return state

# 2. 参数验证
assert vehicle.mass > 0, "质量必须大于0"
assert dt > 0 and dt < 0.1, "时间步长应在(0, 0.1)范围内"

# 3. 使用更小的时间步长
dt = 0.001  # 从2ms降到1ms
```

---

### 3.3 性能异常

#### 问题：仿真速度慢于实时

**症状**:
- 10秒仿真需要15秒实际时间
- CPU占用率100%

**诊断**:

```python
import time

start = time.time()
for i in range(5000):
    vehicle.update(dt)
elapsed = time.time() - start

realtime_factor = 10.0 / elapsed
print(f"实时倍率: {realtime_factor:.2f}x")
```

**解决方案**:

**优化1**: 使用Release模式
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**优化2**: 减少日志输出
```yaml
logging:
  level: WARNING
  console: false
```

**优化3**: 降低数据记录频率
```python
# 每10步记录一次
if i % 10 == 0:
    record_data()
```

**优化4**: 使用C++核心循环
```cpp
// 而不是Python循环
for (int i = 0; i < 10000; i++) {
    vehicle.update(dt);
}
```

---

## 4. 性能故障

### 4.1 实时性不满足

#### 问题：帧时间超过限制

**症状**:
```
Warning: Frame time 2.5ms exceeds limit 2.0ms
```

**诊断**:

```bash
# 检查系统负载
top -H -p $(pgrep ev_dynamics)

# 检查中断
cat /proc/interrupts

# 测试实时性能
sudo cyclictest -l100000 -m -Sp90 -i200 -h400 -q
```

**解决方案**:

**优化1**: 使用RT内核
```bash
sudo apt install linux-image-rt-amd64
sudo reboot
```

**优化2**: CPU隔离
```bash
# 编辑GRUB
GRUB_CMDLINE_LINUX="isolcpus=0,1 nohz_full=0,1"
```

**优化3**: 提高优先级
```bash
# 设置实时优先级
sudo chrt -rr 90 ./build/bin/ev_dynamics_simulation
```

**优化4**: 禁用不必要的服务
```bash
sudo systemctl disable bluetooth
sudo systemctl disable cups
```

---

### 4.2 内存泄漏

#### 问题：内存占用持续增长

**症状**:
- 程序运行一段时间后内存占用持续增加
- 最终导致OOM（Out of Memory）

**诊断**:

```bash
# 监控内存使用
watch -n 1 'ps aux | grep ev_dynamics'

# 使用Valgrind检测
valgrind --leak-check=full ./build/bin/ev_dynamics_simulation
```

**常见原因**:

1. **忘记释放内存**
```cpp
// 错误示例
while (true) {
    double* data = new double[1000];
    // 忘记 delete[] data
}
```

2. **Python引用循环**
```python
# 可能导致循环引用
class Node:
    def __init__(self):
        self.parent = None
        self.children = []

node1 = Node()
node2 = Node()
node1.children.append(node2)
node2.parent = node1  # 循环引用
```

**解决方案**:

```cpp
// C++: 使用智能指针
auto data = std::make_unique<double[]>(1000);
// 自动释放

// 或手动释放
double* data = new double[1000];
delete[] data;
```

```python
# Python: 使用弱引用
import weakref

class Node:
    def __init__(self):
        self.children = []
        self._parent = None
    
    @property
    def parent(self):
        return self._parent() if self._parent else None
    
    @parent.setter
    def parent(self, value):
        self._parent = weakref.ref(value) if value else None
```

---

## 5. HIL通信故障

### 5.1 CAN通信失败

#### 错误：CAN接口未找到

**症状**:
```
Error: CAN interface 'can0' not found
```

**诊断**:
```bash
# 检查CAN接口
ip link show can0

# 检查CAN驱动
lsmod | grep can
```

**解决方案**:

```bash
# 加载CAN驱动
sudo modprobe can
sudo modprobe can_raw
sudo modprobe vcan  # 虚拟CAN（测试用）

# 配置CAN接口
sudo ip link set can0 up type can bitrate 500000

# 验证
ip link show can0
```

---

#### 错误：CAN数据丢失

**症状**:
- 接收到的数据不完整
- 数据包丢失

**诊断**:
```bash
# 检查CAN错误统计
ip -s link show can0

# 查看CAN错误
dmesg | grep can
```

**解决方案**:

**优化1**: 增加缓冲区
```bash
# 增加接收缓冲区
echo 1000 | sudo tee /sys/class/net/can0/queues/rx-0/bytes
```

**优化2**: 提高优先级
```python
# 提高CAN线程优先级
import os
import threading

def can_thread():
    # 设置实时优先级
    os.sched_setscheduler(0, os.SCHED_RR, os.sched_param(90))
    
    # CAN通信代码
    while True:
        data = hil.read_can()
        # ...

thread = threading.Thread(target=can_thread)
thread.start()
```

---

### 5.2 延迟过大

#### 问题：CAN往返延迟超过1ms

**症状**:
- 发送CAN帧后，接收响应延迟过大
- 实时性不满足要求

**诊断**:
```python
import time

start = time.time()
hil.send_can({'test': 123})
data = hil.read_can(timeout=0.1)
elapsed = (time.time() - start) * 1000

print(f"往返延迟: {elapsed:.2f}ms")
```

**解决方案**:

**优化1**: 使用RT内核（见4.1）

**优化2**: 绑定CPU
```bash
# 绑定到隔离的CPU
taskset -c 0 ./build/bin/ev_dynamics_simulation
```

**优化3**: 减少数据量
```python
# 降低采样率
if i % 2 == 0:  # 每2步发送一次
    hil.send_can(data)
```

---

## 6. Python绑定故障

### 6.1 导入失败

#### 错误：ModuleNotFoundError

**症状**:
```python
>>> import powertrain
ModuleNotFoundError: No module named 'powertrain'
```

**诊断**:
```bash
# 检查模块是否存在
ls build/python_modules/

# 检查Python路径
python3 -c "import sys; print('\n'.join(sys.path))"
```

**解决方案**:

**方法1**: 添加路径
```python
import sys
sys.path.append('build/python_modules')
import powertrain
```

**方法2**: 安装Python包
```bash
cd python
pip install -e .
```

---

#### 错误：动态链接失败

**症状**:
```
ImportError: libev_dynamics.so: cannot open shared object file
```

**诊断**:
```bash
# 检查依赖
ldd build/python_modules/powertrain.*.so

# 查找缺失的库
find /usr/local -name "libev_dynamics.so"
```

**解决方案**:

```bash
# 设置库路径
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# 或重新安装
cd build
sudo make install
sudo ldconfig
```

---

### 6.2 类型转换错误

#### 错误：TypeError

**症状**:
```python
TypeError: set_pedal(): incompatible function arguments
```

**原因**: 参数类型不匹配

**解决方案**:

```python
# ❌ 错误：传入字符串
vehicle.set_pedal("0.5")

# ✅ 正确：传入浮点数
vehicle.set_pedal(0.5)

# 添加类型检查
def safe_set_pedal(vehicle, value):
    if not isinstance(value, (int, float)):
        raise TypeError(f"Expected float, got {type(value)}")
    if not 0 <= value <= 1:
        raise ValueError(f"Value must be in [0, 1], got {value}")
    vehicle.set_pedal(float(value))
```

---

## 7. 系统故障

### 7.1 权限问题

#### 错误：Permission denied

**症状**:
```
Permission denied: /dev/can0
```

**解决方案**:

```bash
# 添加用户到can组
sudo usermod -a -G can $USER

# 修改设备权限（临时）
sudo chmod 666 /dev/can0

# 添加udev规则（永久）
sudo nano /etc/udev/rules.d/99-can.rules
# 添加:
KERNEL=="can*", MODE="0666"

# 重新加载规则
sudo udevadm control --reload-rules
sudo udevadm trigger
```

---

### 7.2 资源限制

#### 错误：Too many open files

**症状**:
```
OSError: [Errno 24] Too many open files
```

**诊断**:
```bash
# 检查打开的文件数
lsof -p $(pgrep ev_dynamics) | wc -l

# 检查限制
ulimit -n
```

**解决方案**:

```bash
# 临时增加限制
ulimit -n 65536

# 永久增加（编辑limits.conf）
sudo nano /etc/security/limits.conf
# 添加:
* soft nofile 65536
* hard nofile 65536
```

---

## 8. 故障诊断工具

### 8.1 日志分析

```bash
# 查看实时日志
tail -f logs/ev_dynamics.log

# 搜索错误
grep -i error logs/ev_dynamics.log

# 统计错误类型
grep ERROR logs/ev_dynamics.log | awk '{print $5}' | sort | uniq -c
```

### 8.2 性能分析

```bash
# CPU分析
top -H -p $(pgrep ev_dynamics)

# 内存分析
pmap $(pgrep ev_dynamics)

# IO分析
iotop -p $(pgrep ev_dynamics)
```

### 8.3 网络诊断

```bash
# 检查网络连接
netstat -anp | grep ev_dynamics

# 抓包分析
tcpdump -i can0 -w can_capture.pcap
```

### 8.4 调试工具

```bash
# GDB调试
gdb ./build/bin/ev_dynamics_simulation
(gdb) run
(gdb) bt

# Valgrind内存检查
valgrind --leak-check=full ./build/bin/ev_dynamics_simulation

# strace系统调用跟踪
strace -f ./build/bin/ev_dynamics_simulation
```

---

## 附录

### A. 错误代码速查表

| 错误码 | 类型 | 说明 | 解决方案 |
|--------|------|------|---------|
| E001 | 配置 | 配置文件未找到 | 检查文件路径 |
| E002 | 参数 | 参数超出范围 | 验证参数取值 |
| E003 | 数值 | NaN/Inf检测 | 减小时间步长 |
| E004 | 通信 | CAN通信失败 | 检查硬件连接 |
| E005 | 内存 | 内存不足 | 减少数据记录 |
| E006 | 实时 | 实时性不满足 | 使用RT内核 |
| E007 | 权限 | 权限不足 | 修改权限/用户组 |
| E008 | 依赖 | 库文件缺失 | 安装依赖 |

### B. 快速诊断命令

```bash
# 系统信息
uname -a
cat /etc/os-release

# 编译环境
cmake --version
gcc --version
python3 --version

# 依赖检查
ldconfig -p | grep eigen
pip list | grep pybind

# 运行状态
systemctl status ev_dynamics
journalctl -u ev_dynamics -f

# 性能检查
top -H -p $(pgrep ev_dynamics)
iostat -x 1
```

### C. 联系支持

如果以上方法均无法解决问题，请提供以下信息联系技术支持：

1. **系统信息**
```bash
uname -a > system_info.txt
cmake --version >> system_info.txt
gcc --version >> system_info.txt
```

2. **错误日志**
```bash
journalctl -u ev_dynamics > error_log.txt
dmesg | tail -100 >> error_log.txt
```

3. **复现步骤**
- 详细的操作步骤
- 配置文件内容
- 预期结果 vs 实际结果

**联系方式**:
- **邮件**: support@example.com
- **GitHub**: 创建Issue并附上以上信息
- **紧急**: +86-xxx-xxxx-xxxx

---

**文档版本**: 1.0  
**最后更新**: 2026-03-06  
**维护者**: EV Dynamics Team
