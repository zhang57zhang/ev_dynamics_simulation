# Grafana仪表板部署指南

## 概述

本指南说明如何部署和使用六自由度车辆动力学仿真系统的Grafana监控仪表板。

---

## 前置要求

- Prometheus已安装并运行
- Grafana 9.0+已安装
- 仿真系统已配置Prometheus指标导出

---

## 快速部署

### 1. 导入仪表板

**方式1：通过UI导入**

1. 打开Grafana Web界面（默认：http://localhost:3000）
2. 登录（默认：admin/admin）
3. 点击左侧菜单 → Dashboards → Import
4. 上传JSON文件：
   - `monitoring/grafana_dashboard_vehicle.json` - 车辆仿真监控
   - `monitoring/grafana_dashboard_system.json` - API和安全监控
5. 选择Prometheus数据源
6. 点击Import

**方式2：通过API导入**

```bash
# 导入车辆仿真仪表板
curl -X POST http://admin:admin@localhost:3000/api/dashboards/db \
  -H "Content-Type: application/json" \
  -d @monitoring/grafana_dashboard_vehicle.json

# 导入API安全仪表板
curl -X POST http://admin:admin@localhost:3000/api/dashboards/db \
  -H "Content-Type: application/json" \
  -d @monitoring/grafana_dashboard_system.json
```

### 2. 配置数据源

**Prometheus数据源：**

1. 点击左侧菜单 → Configuration → Data Sources
2. 点击"Add data source"
3. 选择"Prometheus"
4. 配置：
   - Name: Prometheus
   - URL: http://localhost:9090
   - Access: Server (default)
5. 点击"Save & Test"

---

## 仪表板说明

### 1. 车辆仿真监控仪表板

**文件：** `monitoring/grafana_dashboard_vehicle.json`

**包含面板：**
1. **仿真步长延迟** - P99/P95/平均延迟
2. **CPU使用率** - 实时CPU占用
3. **内存使用率** - 实时内存占用
4. **车辆状态** - 速度和加速度曲线
5. **轮胎力** - 四轮轮胎力曲线
6. **P99延迟** - 数值显示
7. **仿真步数/秒** - 数值显示
8. **实时因子** - 数值显示
9. **运行时间** - 数值显示

**刷新频率：** 5秒

### 2. API和安全监控仪表板

**文件：** `monitoring/grafana_dashboard_system.json`

**包含面板：**
1. **API请求速率** - 请求/秒曲线
2. **API响应时间** - P99/P95/平均响应时间
3. **HTTP状态码分布** - 饼图
4. **安全事件** - 认证失败、速率限制

**刷新频率：** 30秒

---

## 告警规则配置

### 1. 配置告警规则

```bash
# 复制告警规则到Prometheus配置目录
sudo cp monitoring/prometheus_rules.yml /etc/prometheus/alert_rules.yml

# 重启Prometheus
sudo systemctl restart prometheus
```

### 2. 配置告警通知

**邮件通知：**

编辑 `/etc/grafana/grafana.ini`:

```ini
[smtp]
enabled = true
host = smtp.gmail.com:587
user = your-email@gmail.com
password = your-app-password
from_address = alert@vehicle-sim.com
from_name = Vehicle Simulation Alert
```

**Slack通知：**

1. Configuration → Alerting → Contact points
2. Add contact point → Slack
3. 输入Webhook URL
4. Test and Save

**PagerDuty通知：**

1. Configuration → Alerting → Contact points
2. Add contact point → PagerDuty
3. 输入Integration Key
4. Test and Save

---

## 监控指标

### 仿真性能指标

| 指标名称 | 描述 | 单位 | 目标 |
|---------|------|------|------|
| `vehicle_simulation_step_duration_seconds` | 仿真步长延迟 | 秒 | <0.001s |
| `vehicle_simulation_realtime_factor` | 实时因子 | - | >1.0 |
| `vehicle_simulation_steps_total` | 总仿真步数 | - | - |
| `vehicle_speed_meters_per_second` | 车速 | m/s | [0, 100] |
| `vehicle_acceleration_meters_per_second_squared` | 加速度 | m/s² | - |

### 系统资源指标

| 指标名称 | 描述 | 单位 | 目标 |
|---------|------|------|------|
| `process_cpu_seconds_total` | CPU使用时间 | 秒 | <80% |
| `node_memory_MemAvailable_bytes` | 可用内存 | 字节 | >15% |
| `node_filesystem_avail_bytes` | 可用磁盘 | 字节 | >15% |

### API指标

| 指标名称 | 描述 | 单位 | 目标 |
|---------|------|------|------|
| `http_requests_total` | HTTP请求总数 | - | - |
| `http_request_duration_seconds` | 请求延迟 | 秒 | P95<0.5s |
| `auth_failures_total` | 认证失败数 | - | - |
| `rate_limit_exceeded_total` | 速率限制触发数 | - | - |

---

## 告警级别

### Critical（严重）
- 仿真延迟 >10ms
- 实时因子 <1.0
- 暴力破解攻击
- 服务宕机
- 数值不稳定

**响应时间：** 立即

### Warning（警告）
- 仿真延迟 >1ms
- CPU使用率 >80%
- 内存使用率 >85%
- 磁盘空间 <15%
- API错误率 >5%

**响应时间：** 1小时内

---

## 自定义仪表板

### 创建新面板

1. 打开仪表板
2. 点击"Add panel"
3. 选择可视化类型
4. 输入PromQL查询
5. 配置面板选项
6. 保存仪表板

### 常用PromQL查询

**P99延迟：**
```promql
histogram_quantile(0.99, rate(vehicle_simulation_step_duration_seconds_bucket[5m]))
```

**平均CPU使用率：**
```promql
rate(process_cpu_seconds_total{job="vehicle-simulation"}[5m]) * 100
```

**HTTP请求速率：**
```promql
rate(http_requests_total{job="vehicle-simulation"}[5m])
```

**错误率：**
```promql
rate(http_requests_total{job="vehicle-simulation",status=~"5.."}[5m]) 
/ 
rate(http_requests_total{job="vehicle-simulation"}[5m])
```

---

## 最佳实践

### 1. 仪表板组织
- 按功能分类（仿真/系统/安全）
- 使用变量实现动态过滤
- 添加文档和说明

### 2. 告警配置
- 避免告警疲劳（合理设置阈值）
- 分级告警（Critical/Warning）
- 配置静默规则

### 3. 性能优化
- 合理设置刷新频率
- 使用Recording Rules
- 限制数据范围

---

## 故障排查

### 仪表板无数据

**检查清单：**
1. Prometheus是否运行：`curl http://localhost:9090/-/healthy`
2. 数据源配置是否正确
3. 指标是否导出：`curl http://localhost:9090/api/v1/targets`
4. 查询语法是否正确

### 告警不触发

**检查清单：**
1. 告警规则是否加载：`curl http://localhost:9090/api/v1/rules`
2. 告警条件是否满足
3. 通知渠道是否配置
4. 静默规则是否生效

---

## 维护

### 定期检查
- 每周检查仪表板可用性
- 每月检查告警规则有效性
- 每季度优化查询性能

### 更新
- 新增功能时更新仪表板
- 调整告警阈值
- 优化可视化效果

---

**Grafana监控配置完成！** ✅

访问仪表板：http://localhost:3000
