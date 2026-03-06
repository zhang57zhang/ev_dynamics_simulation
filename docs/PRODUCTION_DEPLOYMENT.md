# 生产环境部署指南

**版本**: 1.0  
**更新日期**: 2026-03-06  
**适用场景**: 生产环境、HIL测试、实时仿真

---

## 目录

1. [硬件要求](#1-硬件要求生产环境)
2. [操作系统配置](#2-操作系统配置)
3. [安全配置](#3-安全配置)
4. [监控配置](#4-监控配置)
5. [备份策略](#5-备份策略)
6. [高可用配置](#6-高可用配置)
7. [性能调优](#7-性能调优)
8. [灾难恢复](#8-灾难恢复)

---

## 1. 硬件要求（生产环境）

### 1.1 推荐配置

#### 计算节点

| 组件 | 规格 | 用途 |
|------|------|------|
| **CPU** | Intel Xeon E5-2600系列 或 AMD EPYC 7002系列 | 高性能计算 |
| **核心数** | 8核16线程（最低）<br>16核32线程（推荐） | 并行计算 |
| **内存** | 32 GB ECC DDR4（最低）<br>64 GB ECC DDR4（推荐） | 数据缓存 |
| **存储** | 500 GB NVMe SSD（系统）<br>1 TB NVMe SSD（数据） | 快速IO |
| **网络** | 双千兆网卡（冗余） | HIL通信 |

#### HIL硬件（可选）

| 硬件 | 型号示例 | 用途 |
|------|----------|------|
| **实时仿真器** | dSPACE DS1006<br>NI PXI系统 | 硬件在环 |
| **CAN接口** | Vector VN1630<br>NI CAN卡 | 车载网络 |
| **模拟IO** | dSPACE DS2201<br>NI DAQ卡 | 传感器信号 |
| **数字IO** | dSPACE DS4002<br>NI DIO卡 | 开关信号 |

### 1.2 网络拓扑

```
┌─────────────────┐
│  管理网络        │ (1 Gbps)
│  192.168.1.0/24 │
└────────┬────────┘
         │
    ┌────┴────┐
    │ 交换机   │
    └────┬────┘
         │
    ┌────┴─────────────────────┐
    │                          │
┌───┴────┐                ┌───┴────┐
│ 主节点  │                │ 备节点  │
│  Master │                │ Backup │
└────────┘                └────────┘
    │                          │
    └────────┬─────────────────┘
             │
      ┌──────┴──────┐
      │  HIL网络     │ (实时以太网)
      │ 10.0.0.0/24  │
      └──────┬──────┘
             │
      ┌──────┴──────┐
      │  HIL设备     │
      └─────────────┘
```

---

## 2. 操作系统配置

### 2.1 PREEMPT_RT内核配置

#### 安装RT内核

```bash
# Ubuntu 20.04
sudo apt update
sudo apt install linux-image-rt-amd64 linux-headers-rt-amd64

# 查看已安装内核
dpkg --list | grep linux-image

# 重启并选择RT内核
sudo reboot
```

#### 验证RT内核

```bash
# 检查当前内核
uname -v
# 输出应包含: PREEMPT RT

# 检查RT性能
sudo apt install rt-tests
sudo cyclictest -l100000 -m -Sp90 -i200 -h400 -q
# 期望: 最大延迟 <100μs
```

#### 配置内核参数

```bash
# 编辑GRUB配置
sudo nano /etc/default/grub

# 添加内核参数（在GRUB_CMDLINE_LINUX_DEFAULT中）
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash isolcpus=0,1 nohz_full=0,1 rcu_nocbs=0,1 intel_iommu=off"

# 更新GRUB
sudo update-grub

# 重启
sudo reboot
```

**内核参数说明**:

| 参数 | 说明 |
|------|------|
| `isolcpus=0,1` | 隔离CPU 0和1，专用于实时任务 |
| `nohz_full=0,1` | 禁用tick，减少中断 |
| `rcu_nocbs=0,1` | RCU回调迁移到其他CPU |
| `intel_iommu=off` | 禁用IOMMU（降低延迟） |

### 2.2 系统优化

#### CPU配置

```bash
# 安装cpupower工具
sudo apt install linux-tools-common linux-tools-$(uname -r)

# 设置性能模式
sudo cpupower frequency-set -g performance

# 查看当前配置
cpupower frequency-info

# 禁用超线程（可选，根据应用场景）
echo 0 | sudo tee /sys/devices/system/cpu/cpu*/online
```

#### 内存配置

```bash
# 禁用交换分区
sudo swapoff -a
sudo sed -i '/ swap / s/^\(.*\)$/#\1/g' /etc/fstab

# 配置透明大页
echo never | sudo tee /sys/kernel/mm/transparent_hugepage/enabled
echo never | sudo tee /sys/kernel/mm/transparent_hugepage/defrag

# 配置内存锁定
sudo bash -c "cat >> /etc/security/limits.conf << EOF
* hard memlock unlimited
* soft memlock unlimited
root hard memlock unlimited
root soft memlock unlimited
EOF"
```

#### 网络配置

```bash
# 禁用中断平衡
sudo systemctl stop irqbalance
sudo systemctl disable irqbalance

# 配置网卡中断亲和性（示例：eth0）
sudo apt install irqbalance
sudo systemctl stop irqbalance

# 查看网卡中断
grep eth0 /proc/interrupts

# 设置中断亲和性（绑定到CPU 2-3）
echo 0c | sudo tee /proc/irq/$(grep eth0 /proc/interrupts | awk '{print $1}' | tr -d ':')/smp_affinity
```

#### 系统服务优化

```bash
# 禁用不必要的服务
sudo systemctl disable bluetooth
sudo systemctl disable cups
sudo systemctl disable avahi-daemon
sudo systemctl disable ModemManager

# 配置系统日志（减少IO）
sudo nano /etc/systemd/journald.conf
# 设置:
# Storage=volatile
# RuntimeMaxUse=100M
# RuntimeMaxFileSize=10M

sudo systemctl restart systemd-journald
```

---

## 3. 安全配置

### 3.1 防火墙配置

```bash
# 启用UFW防火墙
sudo ufw enable

# 允许SSH（管理网络）
sudo ufw allow from 192.168.1.0/24 to any port 22

# 允许Web接口（监控）
sudo ufw allow from 192.168.1.0/24 to any port 8080

# 允许HIL网络通信
sudo ufw allow from 10.0.0.0/24 to any port 5000
sudo ufw allow from 10.0.0.0/24 to any port 9090

# 查看规则
sudo ufw status verbose
```

### 3.2 用户权限管理

```bash
# 创建专用用户
sudo useradd -r -m -s /bin/bash evdynamics

# 配置sudo权限（限制性）
sudo visudo
# 添加:
evdynamics ALL=(ALL) NOPASSWD: /usr/local/bin/ev_dynamics_*
evdynamics ALL=(ALL) NOPASSWD: /bin/systemctl start ev_dynamics
evdynamics ALL=(ALL) NOPASSWD: /bin/systemctl stop ev_dynamics
evdynamics ALL=(ALL) NOPASSWD: /bin/systemctl restart ev_dynamics

# 配置文件权限
sudo chown -R evdynamics:evdynamics /opt/ev_dynamics
sudo chmod 750 /opt/ev_dynamics
sudo chmod 640 /opt/ev_dynamics/config/*.yaml
```

### 3.3 SSH安全配置

```bash
# 编辑SSH配置
sudo nano /etc/ssh/sshd_config

# 推荐配置:
Port 22
PermitRootLogin no
PasswordAuthentication no
PubkeyAuthentication yes
AllowUsers evdynamics@192.168.1.0/24
MaxAuthTries 3
ClientAliveInterval 300
ClientAliveCountMax 2

# 重启SSH服务
sudo systemctl restart sshd
```

### 3.4 文件系统安全

```bash
# 配置SELinux/AppArmor（Ubuntu默认AppArmor）
sudo apt install apparmor-utils

# 创建自定义配置
sudo aa-genprof /usr/local/bin/ev_dynamics_simulation

# 或禁用（不推荐生产环境）
# sudo systemctl disable apparmor
```

---

## 4. 监控配置

### 4.1 Prometheus监控

#### 安装Prometheus

```bash
# 下载Prometheus
wget https://github.com/prometheus/prometheus/releases/download/v2.45.0/prometheus-2.45.0.linux-amd64.tar.gz
tar xzf prometheus-2.45.0.linux-amd64.tar.gz
sudo mv prometheus-2.45.0.linux-amd64 /opt/prometheus
sudo useradd -r -s /bin/false prometheus
sudo chown -R prometheus:prometheus /opt/prometheus
```

#### 配置Prometheus

```yaml
# /opt/prometheus/prometheus.yml
global:
  scrape_interval: 15s
  evaluation_interval: 15s

scrape_configs:
  - job_name: 'prometheus'
    static_configs:
      - targets: ['localhost:9090']

  - job_name: 'ev_dynamics'
    static_configs:
      - targets: ['localhost:9091']
    metrics_path: '/metrics'
    scrape_interval: 5s

  - job_name: 'node_exporter'
    static_configs:
      - targets: ['localhost:9100']
```

#### 创建Systemd服务

```ini
# /etc/systemd/system/prometheus.service
[Unit]
Description=Prometheus Monitoring System
After=network.target

[Service]
Type=simple
User=prometheus
ExecStart=/opt/prometheus/prometheus \
  --config.file=/opt/prometheus/prometheus.yml \
  --storage.tsdb.path=/opt/prometheus/data \
  --web.listen-address=:9090

Restart=on-failure

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl daemon-reload
sudo systemctl enable prometheus
sudo systemctl start prometheus
```

### 4.2 Grafana可视化

#### 安装Grafana

```bash
# 添加Grafana仓库
sudo apt install -y software-properties-common
sudo add-apt-repository "deb https://packages.grafana.com/oss/deb stable main"
wget -q -O - https://packages.grafana.com/gpg.key | sudo apt-key add -

# 安装Grafana
sudo apt update
sudo apt install grafana

# 启动服务
sudo systemctl enable grafana-server
sudo systemctl start grafana-server
```

#### 配置仪表板

1. 访问 `http://<server>:3000`（默认用户名/密码: admin/admin）
2. 添加数据源: Prometheus (`http://localhost:9090`)
3. 导入仪表板: `monitoring/grafana_dashboard.json`
4. 配置告警规则

**关键监控指标**:

| 指标 | 说明 | 告警阈值 |
|------|------|---------|
| `ev_dynamics_frame_time_ms` | 帧时间 | >2.5ms |
| `ev_dynamics_latency_us` | 延迟 | >100μs |
| `ev_dynamics_cpu_usage` | CPU使用率 | >80% |
| `ev_dynamics_memory_mb` | 内存使用 | >2GB |
| `node_cpu_seconds_total` | 系统CPU | >90% |

### 4.3 日志管理

#### 配置rsyslog

```bash
# 创建日志配置
sudo nano /etc/rsyslog.d/ev_dynamics.conf
```

```
# EV Dynamics日志
if $programname == 'ev_dynamics' then /var/log/ev_dynamics/ev_dynamics.log
& stop

# 按级别分离
if $programname == 'ev_dynamics' and $syslogseverity <= 4 then /var/log/ev_dynamics/error.log
& stop
```

```bash
# 创建日志目录
sudo mkdir -p /var/log/ev_dynamics
sudo chown evdynamics:evdynamics /var/log/ev_dynamics

# 重启rsyslog
sudo systemctl restart rsyslog
```

#### 配置日志轮转

```
# /etc/logrotate.d/ev_dynamics
/var/log/ev_dynamics/*.log {
    daily
    rotate 30
    compress
    delaycompress
    missingok
    notifempty
    create 0640 evdynamics evdynamics
    sharedscripts
    postrotate
        systemctl reload ev_dynamics > /dev/null 2>&1 || true
    endscript
}
```

---

## 5. 备份策略

### 5.1 自动备份脚本

```bash
#!/bin/bash
# /opt/ev_dynamics/scripts/backup.sh

BACKUP_DIR="/opt/ev_dynamics/backups"
DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_FILE="ev_dynamics_backup_${DATE}.tar.gz"

# 创建备份目录
mkdir -p $BACKUP_DIR

# 备份内容
tar -czf $BACKUP_DIR/$BACKUP_FILE \
  /opt/ev_dynamics/config \
  /opt/ev_dynamics/data \
  /var/log/ev_dynamics

# 保留最近30天的备份
find $BACKUP_DIR -name "*.tar.gz" -mtime +30 -delete

# 同步到远程服务器（可选）
rsync -avz $BACKUP_DIR/$BACKUP_FILE backup-server:/backups/ev_dynamics/

echo "Backup completed: $BACKUP_FILE"
```

```bash
# 设置权限
chmod +x /opt/ev_dynamics/scripts/backup.sh

# 配置cron（每日凌晨2点备份）
crontab -e
0 2 * * * /opt/ev_dynamics/scripts/backup.sh >> /var/log/ev_dynamics/backup.log 2>&1
```

### 5.2 配置文件版本控制

```bash
# 初始化Git仓库（配置目录）
cd /opt/ev_dynamics/config
git init
git add .
git commit -m "Initial config"

# 配置远程仓库
git remote add origin git@github.com:your-org/ev_dynamics_config.git
git push -u origin main

# 创建配置变更钩子
cat > .git/hooks/post-commit << 'EOF'
#!/bin/bash
git push origin main
systemctl reload ev_dynamics
EOF
chmod +x .git/hooks/post-commit
```

---

## 6. 高可用配置

### 6.1 主备切换

#### 使用Pacemaker + Corosync

```bash
# 安装Pacemaker和Corosync
sudo apt install pacemaker corosync pcs

# 配置集群（主备两台服务器）
sudo pcs cluster auth node1 node2 -u hacluster -p password
sudo pcs cluster setup --name ev_dynamics_cluster node1 node2
sudo pcs cluster start --all
sudo pcs cluster enable --all

# 创建资源
sudo pcs resource create ev_dynamics systemd:ev_dynamics \
  op monitor interval=30s \
  meta target-role=Started

# 配置故障转移
sudo pcs constraint colocation add ev_dynamics with node1 INFINITY
sudo pcs constraint order start node1 then ev_dynamics
```

### 6.2 负载均衡（可选）

```bash
# 安装HAProxy
sudo apt install haproxy

# 配置负载均衡
sudo nano /etc/haproxy/haproxy.cfg
```

```
frontend ev_dynamics_front
    bind *:8080
    mode http
    default_backend ev_dynamics_back

backend ev_dynamics_back
    mode http
    balance roundrobin
    option httpchk GET /health
    server node1 192.168.1.10:8080 check
    server node2 192.168.1.11:8080 check
```

---

## 7. 性能调优

### 7.1 实时性能基准测试

```bash
# 运行性能测试
python3 /opt/ev_dynamics/tests/test_realtime_prerequisite.py

# 查看性能报告
cat /opt/ev_dynamics/logs/performance_report.txt
```

**性能目标**:

| 指标 | 目标值 | 生产要求 |
|------|--------|----------|
| 仿真频率 | 500 Hz | ≥500 Hz |
| 延迟 | <80 μs | <100 μs |
| 抖动 | <10 μs | <20 μs |
| CPU占用 | <30% | <50% |
| 内存占用 | <1 GB | <2 GB |

### 7.2 性能调优参数

#### C++运行时调优

```bash
# 设置线程优先级（systemd服务中配置）
CPUSchedulingPolicy=rr
CPUSchedulingPriority=90

# 设置CPU亲和性
CPUAffinity=0,1

# 内存锁定
LimitMEMLOCK=infinity
LimitRTPRIO=95
```

#### 网络调优

```bash
# 配置网卡缓冲区大小
sudo ethtool -G eth0 rx 4096 tx 4096

# 配置TCP参数
sudo sysctl -w net.core.rmem_max=134217728
sudo sysctl -w net.core.wmem_max=134217728
sudo sysctl -w net.ipv4.tcp_rmem="4096 87380 134217728"
sudo sysctl -w net.ipv4.tcp_wmem="4096 65536 134217728"
```

---

## 8. 灾难恢复

### 8.1 恢复流程

#### 从备份恢复

```bash
# 停止服务
sudo systemctl stop ev_dynamics

# 恢复配置
cd /opt/ev_dynamics/backups
tar -xzf ev_dynamics_backup_YYYYMMDD_HHMMSS.tar.gz -C /

# 验证配置
/opt/ev_dynamics/scripts/health_check.sh

# 启动服务
sudo systemctl start ev_dynamics
```

#### 从Git恢复配置

```bash
cd /opt/ev_dynamics/config
git checkout <commit-hash>
sudo systemctl reload ev_dynamics
```

### 8.2 应急预案

| 故障类型 | 应急措施 | 恢复时间 |
|---------|---------|---------|
| 服务崩溃 | 自动重启（systemd） | <30秒 |
| 配置错误 | Git回滚 | <5分钟 |
| 硬件故障 | 主备切换 | <1分钟 |
| 数据损坏 | 备份恢复 | <30分钟 |

### 8.3 故障演练

```bash
# 模拟服务崩溃
sudo kill -9 $(pgrep ev_dynamics)

# 模拟配置错误
cd /opt/ev_dynamics/config
git revert HEAD

# 模拟网络故障
sudo ifdown eth0

# 记录演练结果
echo "$(date): 故障演练完成" >> /var/log/ev_dynamics/drill.log
```

---

## 附录

### A. 生产环境检查清单

#### 部署前检查

- [ ] RT内核已安装并验证
- [ ] 所有依赖库已安装
- [ ] 防火墙规则已配置
- [ ] 用户权限已配置
- [ ] 监控系统已部署
- [ ] 备份策略已配置
- [ ] 日志轮转已配置

#### 部署后验证

- [ ] 服务正常启动
- [ ] 性能指标达标
- [ ] 监控数据正常
- [ ] 日志正常记录
- [ ] 备份正常执行
- [ ] 告警规则生效

### B. 常用运维命令

```bash
# 查看服务状态
sudo systemctl status ev_dynamics

# 查看实时日志
sudo journalctl -u ev_dynamics -f

# 查看性能指标
curl http://localhost:9091/metrics

# 手动触发备份
/opt/ev_dynamics/scripts/backup.sh

# 健康检查
/opt/ev_dynamics/scripts/health_check.sh

# 性能测试
python3 /opt/ev_dynamics/tests/test_realtime_prerequisite.py
```

### C. 联系方式

- **技术支持**: support@example.com
- **紧急联系**: +86-xxx-xxxx-xxxx
- **文档仓库**: https://github.com/your-org/ev_dynamics_docs

---

**文档版本**: 1.0  
**最后更新**: 2026-03-06  
**维护者**: EV Dynamics DevOps Team
