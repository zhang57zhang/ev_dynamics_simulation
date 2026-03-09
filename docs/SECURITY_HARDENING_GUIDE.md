# 生产环境安全加固指南

## 概述

本文档提供六自由度车辆动力学仿真系统的生产环境安全加固指南，确保系统在生产环境中安全运行。

---

## 1. 认证与授权

### 1.1 启用强认证

```yaml
# config/production_security.yaml
authentication:
  enabled: true
  method: "jwt"
  
  jwt:
    secret_key: "${JWT_SECRET_KEY}"  # 从环境变量读取
    algorithm: "HS256"
    expiration: 3600  # 1小时
```

**环境变量设置：**
```bash
# 生成强密钥（32字节）
export JWT_SECRET_KEY=$(openssl rand -base64 32)
export OAUTH_CLIENT_SECRET="your-oauth-client-secret"
export API_KEY_PROD=$(openssl rand -hex 32)
```

### 1.2 RBAC配置

```yaml
access_control:
  rbac:
    enabled: true
    
    roles:
      - name: "admin"
        permissions: ["*"]
      - name: "operator"
        permissions: ["simulation:read", "simulation:write"]
      - name: "viewer"
        permissions: ["simulation:read"]
```

---

## 2. 网络安全

### 2.1 TLS配置

**启用TLS 1.3：**
```yaml
network:
  tls:
    enabled: true
    version: "1.3"
    cert_file: "/etc/ssl/certs/vehicle-sim.crt"
    key_file: "/etc/ssl/private/vehicle-sim.key"
```

**获取Let's Encrypt证书：**
```bash
# 安装certbot
sudo apt-get install certbot

# 获取证书
sudo certbot certonly --standalone -d your-domain.com

# 自动续期
sudo crontab -e
0 12 * * * /usr/bin/certbot renew --quiet
```

### 2.2 防火墙配置

**UFW（Ubuntu）：**
```bash
# 启用UFW
sudo ufw enable

# 默认拒绝入站
sudo ufw default deny incoming

# 默认允许出站
sudo ufw default allow outgoing

# 允许HTTPS
sudo ufw allow 443/tcp

# 允许SSH（仅内网）
sudo ufw allow from 10.0.0.0/8 to any port 22

# 允许Prometheus（仅内网）
sudo ufw allow from 10.0.0.0/8 to any port 9090

# 查看状态
sudo ufw status verbose
```

**Firewalld（CentOS/RHEL）：**
```bash
# 启用firewalld
sudo systemctl enable firewalld
sudo systemctl start firewalld

# 允许HTTPS
sudo firewall-cmd --permanent --add-service=https

# 允许SSH（仅内网）
sudo firewall-cmd --permanent --add-rich-rule='rule family="ipv4" source address="10.0.0.0/8" service name="ssh" accept'

# 重载配置
sudo firewall-cmd --reload
```

---

## 3. 数据安全

### 3.1 数据加密

**启用静态数据加密：**
```yaml
data_security:
  encryption:
    enabled: true
    algorithm: "AES-256-GCM"
    key_rotation_days: 90
```

**生成加密密钥：**
```bash
# 生成主密钥
export MASTER_KEY=$(openssl rand -base64 32)

# 存储到安全位置
echo $MASTER_KEY | sudo tee /etc/vehicle-sim/master.key
sudo chmod 600 /etc/vehicle-sim/master.key
```

### 3.2 数据脱敏

```yaml
data_security:
  data_masking:
    enabled: true
    rules:
      - field: "email"
        pattern: "^([^@]{3})[^@]*(@.*)$"
        replacement: "$1***$2"
```

---

## 4. 审计与日志

### 4.1 启用审计日志

```yaml
audit:
  enabled: true
  log_file: "/var/log/vehicle-sim/audit.log"
  max_size_mb: 100
  retention_days: 90
```

**日志轮转配置：**
```bash
# /etc/logrotate.d/vehicle-sim
/var/log/vehicle-sim/*.log {
    daily
    rotate 90
    compress
    delaycompress
    missingok
    notifempty
    create 0640 vehicle-sim vehicle-sim
    sharedscripts
    postrotate
        systemctl reload vehicle-sim > /dev/null 2>&1 || true
    endscript
}
```

### 4.2 集中式日志

**使用ELK Stack：**
```yaml
# config/logstash.conf
input {
  file {
    path => "/var/log/vehicle-sim/*.log"
    start_position => "beginning"
  }
}

output {
  elasticsearch {
    hosts => ["localhost:9200"]
    index => "vehicle-sim-%{+YYYY.MM.dd}"
  }
}
```

---

## 5. 速率限制

### 5.1 配置速率限制

```yaml
rate_limiting:
  enabled: true
  
  per_user:
    requests_per_minute: 100
    requests_per_hour: 1000
    
  endpoints:
    - path: "/api/v1/auth/login"
      limit: 5
      window: "minute"
```

### 5.2 使用Redis实现

```yaml
session:
  storage: "redis"
  redis:
    host: "${REDIS_HOST}"
    port: 6379
    password: "${REDIS_PASSWORD}"
```

---

## 6. 安全头配置

### 6.1 HTTP安全头

```yaml
security_headers:
  hsts:
    enabled: true
    max_age: 31536000
    include_subdomains: true
    
  csp:
    enabled: true
    policy: "default-src 'self'"
    
  frame_options:
    enabled: true
    value: "DENY"
```

### 6.2 Nginx配置

```nginx
# /etc/nginx/conf.d/security-headers.conf
add_header Strict-Transport-Security "max-age=31536000; includeSubDomains; preload" always;
add_header Content-Security-Policy "default-src 'self'" always;
add_header X-Frame-Options "DENY" always;
add_header X-Content-Type-Options "nosniff" always;
add_header X-XSS-Protection "1; mode=block" always;
```

---

## 7. 依赖安全

### 7.1 依赖扫描

**安装扫描工具：**
```bash
# Node.js
npm install -g npm-audit

# Python
pip install pip-audit
```

**定期扫描：**
```bash
# 每日扫描脚本
#!/bin/bash
cd /opt/vehicle-sim
npm audit --production
pip-audit check
```

### 7.2 依赖更新策略

```yaml
dependency_security:
  scan_enabled: true
  scan_frequency: "daily"
  
  vulnerability_check:
    enabled: true
    fail_on_high: true
    fail_on_critical: true
```

---

## 8. 备份与恢复

### 8.1 自动备份

```yaml
backup:
  enabled: true
  frequency: "daily"
  retention_days: 30
  
  encryption:
    enabled: true
    algorithm: "AES-256"
```

**备份脚本：**
```bash
#!/bin/bash
# /opt/vehicle-sim/scripts/backup.sh

BACKUP_DIR="/backup/vehicle-sim"
DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_FILE="$BACKUP_DIR/backup_$DATE.tar.gz"

# 创建备份
tar -czf $BACKUP_FILE \
  config/ \
  data/ \
  logs/

# 加密备份
openssl enc -aes-256-cbc \
  -salt -in $BACKUP_FILE \
  -out $BACKUP_FILE.enc \
  -pass file:/etc/vehicle-sim/backup.key

# 上传到S3
aws s3 cp $BACKUP_FILE.enc s3://backup-bucket/vehicle-sim/

# 清理旧备份
find $BACKUP_DIR -name "backup_*.tar.gz*" -mtime +30 -delete
```

### 8.2 Cron任务

```bash
# /etc/cron.d/vehicle-sim-backup
0 2 * * * vehicle-sim /opt/vehicle-sim/scripts/backup.sh >> /var/log/vehicle-sim/backup.log 2>&1
```

---

## 9. 监控与告警

### 9.1 安全监控

```yaml
security_monitoring:
  real_time:
    enabled: true
    alert_on:
      - "multiple_failed_logins"
      - "unusual_access_pattern"
```

### 9.2 Prometheus告警规则

```yaml
# /etc/prometheus/alert_rules.yml
groups:
  - name: security_alerts
    rules:
      - alert: HighFailedLogins
        expr: rate(auth_failures_total[5m]) > 10
        for: 5m
        annotations:
          summary: "High rate of failed login attempts"
          
      - alert: UnusualAPIAccess
        expr: rate(api_requests_total[5m]) > 1000
        for: 5m
        annotations:
          summary: "Unusual API access pattern detected"
```

---

## 10. 安全检查清单

### 部署前检查

- [ ] 认证已启用
- [ ] TLS已启用（版本1.3）
- [ ] 防火墙已配置
- [ ] 审计日志已启用
- [ ] 速率限制已配置
- [ ] 安全头已配置
- [ ] 依赖已扫描
- [ ] 备份已配置
- [ ] 监控已配置
- [ ] 环境变量已设置

### 定期检查（每周）

- [ ] 证书有效期
- [ ] 依赖漏洞扫描
- [ ] 备份验证
- [ ] 日志审计
- [ ] 权限检查

### 安全事件响应

- [ ] 立即隔离受影响系统
- [ ] 保存日志和证据
- [ ] 通知安全团队
- [ ] 分析根本原因
- [ ] 修复漏洞
- [ ] 更新文档

---

## 11. 安全加固脚本

运行安全检查脚本：
```bash
./scripts/security_check.sh
```

预期输出：
```
========================================
  生产环境安全配置检查
  时间: 2026-03-06 22:30:00
========================================

[✓] 安全配置文件存在
[✓] 环境变量 JWT_SECRET_KEY 已设置
[✓] TLS证书文件存在
[✓] 认证已启用
[✓] UFW防火墙已启用
[✓] 配置文件权限安全: 640
[✓] 审计日志已启用
[✓] NPM依赖无已知漏洞
[✓] 备份已启用

检查结果统计：
通过: 15
失败: 0
警告: 2

✅ 所有安全检查通过！
```

---

## 12. 应急响应

### 安全事件处理流程

1. **检测** - 监控系统检测到异常
2. **响应** - 自动告警通知安全团队
3. **隔离** - 隔离受影响的系统
4. **分析** - 分析日志和证据
5. **修复** - 修复漏洞
6. **恢复** - 恢复服务
7. **复盘** - 总结经验教训

### 联系方式

- **安全团队**：security@company.com
- **应急响应**：+86-xxx-xxxx-xxxx
- **Slack频道**：#security-alerts

---

**安全加固完成！** ✅

生产环境安全配置已完善，所有安全检查通过。
