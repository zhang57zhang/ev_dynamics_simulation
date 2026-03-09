#!/bin/bash

# 安全配置检查脚本
# 用于验证生产环境安全配置是否正确

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 配置文件路径
CONFIG_FILE="config/production_security.yaml"
ENV_FILE=".env.production"

# 检查计数
PASS_COUNT=0
FAIL_COUNT=0
WARN_COUNT=0

# 打印函数
print_header() {
    echo ""
    echo "========================================="
    echo "$1"
    echo "========================================="
}

print_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}[✓]${NC} $2"
        ((PASS_COUNT++))
    else
        echo -e "${RED}[✗]${NC} $2"
        ((FAIL_COUNT++))
    fi
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
    ((WARN_COUNT++))
}

# 检查配置文件
check_config_file() {
    print_header "检查配置文件"
    
    if [ -f "$CONFIG_FILE" ]; then
        print_result 0 "安全配置文件存在"
    else
        print_result 1 "安全配置文件不存在"
        return 1
    fi
}

# 检查环境变量
check_environment_variables() {
    print_header "检查环境变量"
    
    local required_vars=(
        "JWT_SECRET_KEY"
        "OAUTH_CLIENT_ID"
        "OAUTH_CLIENT_SECRET"
        "API_KEY_PROD"
        "API_KEY_MONITORING"
        "REDIS_HOST"
        "REDIS_PASSWORD"
    )
    
    for var in "${required_vars[@]}"; do
        if [ -n "${!var}" ]; then
            print_result 0 "环境变量 $var 已设置"
        else
            print_result 1 "环境变量 $var 未设置"
        fi
    done
}

# 检查TLS配置
check_tls_config() {
    print_header "检查TLS配置"
    
    # 检查证书文件
    local cert_file=$(grep "cert_file:" $CONFIG_FILE | awk '{print $2}' | tr -d '"')
    local key_file=$(grep "key_file:" $CONFIG_FILE | awk '{print $2}' | tr -d '"')
    
    if [ -f "$cert_file" ]; then
        print_result 0 "TLS证书文件存在: $cert_file"
        
        # 检查证书有效期
        local expiry=$(openssl x509 -enddate -noout -in "$cert_file" | cut -d= -f2)
        local expiry_epoch=$(date -d "$expiry" +%s)
        local current_epoch=$(date +%s)
        local days_left=$(( (expiry_epoch - current_epoch) / 86400 ))
        
        if [ $days_left -gt 30 ]; then
            print_result 0 "证书有效期剩余 $days_left 天"
        elif [ $days_left -gt 0 ]; then
            print_warning "证书即将过期，剩余 $days_left 天"
        else
            print_result 1 "证书已过期"
        fi
    else
        print_result 1 "TLS证书文件不存在: $cert_file"
    fi
    
    if [ -f "$key_file" ]; then
        print_result 0 "TLS密钥文件存在: $key_file"
    else
        print_result 1 "TLS密钥文件不存在: $key_file"
    fi
    
    # 检查TLS版本
    local tls_version=$(grep "version:" $CONFIG_FILE | grep -A1 "tls:" | tail -1 | awk '{print $2}')
    if [ "$tls_version" = '"1.3"' ]; then
        print_result 0 "使用TLS 1.3（推荐）"
    elif [ "$tls_version" = '"1.2"' ]; then
        print_warning "使用TLS 1.2，建议升级到1.3"
    else
        print_result 1 "TLS版本配置错误"
    fi
}

# 检查认证配置
check_authentication() {
    print_header "检查认证配置"
    
    # 检查认证是否启用
    local auth_enabled=$(grep "enabled:" $CONFIG_FILE | grep -A1 "authentication:" | tail -1 | awk '{print $2}')
    if [ "$auth_enabled" = "true" ]; then
        print_result 0 "认证已启用"
    else
        print_result 1 "认证未启用（生产环境必须启用）"
    fi
    
    # 检查JWT密钥强度
    if [ -n "$JWT_SECRET_KEY" ]; then
        local key_length=${#JWT_SECRET_KEY}
        if [ $key_length -ge 32 ]; then
            print_result 0 "JWT密钥长度足够（$key_length字符）"
        else
            print_result 1 "JWT密钥长度不足（当前$key_length，建议≥32）"
        fi
    fi
}

# 检查防火墙配置
check_firewall() {
    print_header "检查防火墙配置"
    
    # 检查防火墙是否启用
    if command -v ufw &> /dev/null; then
        if ufw status | grep -q "Status: active"; then
            print_result 0 "UFW防火墙已启用"
        else
            print_result 1 "UFW防火墙未启用"
        fi
    elif command -v firewall-cmd &> /dev/null; then
        if firewall-cmd --state &> /dev/null; then
            print_result 0 "Firewalld防火墙已启用"
        else
            print_result 1 "Firewalld防火墙未启用"
        fi
    else
        print_warning "未检测到防火墙，请手动确认"
    fi
    
    # 检查开放端口
    print_header "检查开放端口"
    local open_ports=$(netstat -tuln | grep LISTEN | awk '{print $4}' | cut -d: -f2 | sort -n | uniq)
    
    echo "当前开放的端口："
    echo "$open_ports"
    
    # 检查是否有不安全的端口
    local unsafe_ports=("21" "23" "80" "8080")
    for port in "${unsafe_ports[@]}"; do
        if echo "$open_ports" | grep -q "^${port}$"; then
            print_warning "端口 $port 开放（建议关闭或使用安全替代）"
        fi
    done
}

# 检查文件权限
check_file_permissions() {
    print_header "检查文件权限"
    
    # 检查配置文件权限
    if [ -f "$CONFIG_FILE" ]; then
        local perms=$(stat -c %a "$CONFIG_FILE")
        if [ "$perms" -le "640" ]; then
            print_result 0 "配置文件权限安全: $perms"
        else
            print_result 1 "配置文件权限过于宽松: $perms（建议≤640）"
        fi
    fi
    
    # 检查密钥文件权限
    local key_file=$(grep "key_file:" $CONFIG_FILE | awk '{print $2}' | tr -d '"')
    if [ -f "$key_file" ]; then
        local key_perms=$(stat -c %a "$key_file")
        if [ "$key_perms" -le "600" ]; then
            print_result 0 "密钥文件权限安全: $key_perms"
        else
            print_result 1 "密钥文件权限过于宽松: $key_perms（建议≤600）"
        fi
    fi
}

# 检查日志配置
check_logging() {
    print_header "检查日志配置"
    
    # 检查审计日志是否启用
    local audit_enabled=$(grep "enabled:" $CONFIG_FILE | grep -A1 "audit:" | tail -1 | awk '{print $2}')
    if [ "$audit_enabled" = "true" ]; then
        print_result 0 "审计日志已启用"
    else
        print_result 1 "审计日志未启用（生产环境必须启用）"
    fi
    
    # 检查日志目录
    local log_dir="/var/log/vehicle-sim"
    if [ -d "$log_dir" ]; then
        print_result 0 "日志目录存在: $log_dir"
    else
        print_warning "日志目录不存在: $log_dir（将自动创建）"
    fi
}

# 检查依赖安全
check_dependencies() {
    print_header "检查依赖安全"
    
    # 检查是否有已知漏洞的依赖
    if command -v npm &> /dev/null; then
        if [ -f "package.json" ]; then
            npm audit --production > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                print_result 0 "NPM依赖无已知漏洞"
            else
                print_result 1 "NPM依赖存在已知漏洞，请运行 npm audit"
            fi
        fi
    fi
    
    if command -v pip &> /dev/null; then
        if [ -f "requirements.txt" ]; then
            if pip-audit check > /dev/null 2>&1; then
                print_result 0 "Python依赖无已知漏洞"
            else
                print_warning "Python依赖检查失败，请手动检查"
            fi
        fi
    fi
}

# 检查备份配置
check_backup() {
    print_header "检查备份配置"
    
    # 检查备份是否启用
    local backup_enabled=$(grep "enabled:" $CONFIG_FILE | grep -A1 "backup:" | tail -1 | awk '{print $2}')
    if [ "$backup_enabled" = "true" ]; then
        print_result 0 "备份已启用"
        
        # 检查备份目录
        local backup_dir="/backup/vehicle-sim"
        if [ -d "$backup_dir" ]; then
            print_result 0 "备份目录存在: $backup_dir"
            
            # 检查最近的备份
            local latest_backup=$(ls -t "$backup_dir" | head -1)
            if [ -n "$latest_backup" ]; then
                local backup_age=$(( ($(date +%s) - $(date -d "$latest_backup" +%s)) / 86400 ))
                if [ $backup_age -le 1 ]; then
                    print_result 0 "备份是最新的（$backup_age天前）"
                else
                    print_warning "备份已过期（$backup_age天前）"
                fi
            fi
        else
            print_warning "备份目录不存在: $backup_dir"
        fi
    else
        print_result 1 "备份未启用"
    fi
}

# 生成报告
generate_report() {
    print_header "安全检查报告"
    
    echo ""
    echo "检查结果统计："
    echo -e "${GREEN}通过: $PASS_COUNT${NC}"
    echo -e "${RED}失败: $FAIL_COUNT${NC}"
    echo -e "${YELLOW}警告: $WARN_COUNT${NC}"
    echo ""
    
    if [ $FAIL_COUNT -eq 0 ]; then
        echo -e "${GREEN}✅ 所有安全检查通过！${NC}"
        exit 0
    else
        echo -e "${RED}❌ 存在 $FAIL_COUNT 个安全问题，请立即修复！${NC}"
        exit 1
    fi
}

# 主函数
main() {
    echo ""
    echo "========================================="
    echo "  生产环境安全配置检查"
    echo "  时间: $(date '+%Y-%m-%d %H:%M:%S')"
    echo "========================================="
    
    check_config_file
    check_environment_variables
    check_tls_config
    check_authentication
    check_firewall
    check_file_permissions
    check_logging
    check_dependencies
    check_backup
    
    generate_report
}

# 执行主函数
main "$@"
