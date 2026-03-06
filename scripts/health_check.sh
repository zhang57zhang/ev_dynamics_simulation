#!/bin/bash
# EV Dynamics Simulation - Health Check Script
# Version: 1.0
# Date: 2026-03-06

SERVICE_NAME="ev_dynamics"
DEPLOY_DIR="${DEPLOY_DIR:-/opt/ev_dynamics}"
LOG_DIR="${LOG_DIR:-/var/log/ev_dynamics}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Health check functions
check_service() {
    if systemctl is-active --quiet ${SERVICE_NAME}; then
        echo -e "${GREEN}✓ Service is running${NC}"
        return 0
    else
        echo -e "${RED}✗ Service is not running${NC}"
        return 1
    fi
}

check_binary() {
    if [[ -x "$DEPLOY_DIR/bin/ev_dynamics_sim" ]]; then
        echo -e "${GREEN}✓ Binary exists and is executable${NC}"
        return 0
    else
        echo -e "${RED}✗ Binary not found or not executable${NC}"
        return 1
    fi
}

check_config() {
    if [[ -f "/etc/ev_dynamics/production.yaml" ]]; then
        echo -e "${GREEN}✓ Configuration file exists${NC}"
        return 0
    else
        echo -e "${RED}✗ Configuration file missing${NC}"
        return 1
    fi
}

check_logs() {
    if [[ -d "$LOG_DIR" ]]; then
        local log_size=$(du -sm "$LOG_DIR" | cut -f1)
        echo -e "${GREEN}✓ Log directory exists (Size: ${log_size}MB)${NC}"
        return 0
    else
        echo -e "${YELLOW}⚠ Log directory not found${NC}"
        return 0
    fi
}

check_memory() {
    local mem_usage=$(ps aux | grep ev_dynamics_sim | grep -v grep | awk '{sum+=$4} END {print sum}')
    if [[ -n "$mem_usage" ]]; then
        echo -e "${GREEN}✓ Memory usage: ${mem_usage}%${NC}"
        return 0
    else
        echo -e "${YELLOW}⚠ No memory usage data${NC}"
        return 0
    fi
}

check_cpu() {
    local cpu_usage=$(ps aux | grep ev_dynamics_sim | grep -v grep | awk '{sum+=$3} END {print sum}')
    if [[ -n "$cpu_usage" ]]; then
        echo -e "${GREEN}✓ CPU usage: ${cpu_usage}%${NC}"
        return 0
    else
        echo -e "${YELLOW}⚠ No CPU usage data${NC}"
        return 0
    fi
}

check_realtime() {
    if command -v rt-tests >/dev/null 2>&1; then
        local latency=$($DEPLOY_DIR/bin/ev_dynamics_sim --test-latency 2>/dev/null || echo "N/A")
        echo -e "${GREEN}✓ Real-time latency: ${latency}${NC}"
    else
        echo -e "${YELLOW}⚠ Real-time test tools not available${NC}"
    fi
    return 0
}

# Main health check
echo "========================================"
echo "EV Dynamics - Health Check"
echo "========================================"
echo ""

FAILED=0

echo "Checking service status..."
check_service || FAILED=$((FAILED + 1))
echo ""

echo "Checking binary..."
check_binary || FAILED=$((FAILED + 1))
echo ""

echo "Checking configuration..."
check_config || FAILED=$((FAILED + 1))
echo ""

echo "Checking logs..."
check_logs || FAILED=$((FAILED + 1))
echo ""

echo "Checking resource usage..."
check_memory || FAILED=$((FAILED + 1))
check_cpu || FAILED=$((FAILED + 1))
echo ""

echo "Checking real-time performance..."
check_realtime || FAILED=$((FAILED + 1))
echo ""

echo "========================================"
if [[ $FAILED -eq 0 ]]; then
    echo -e "${GREEN}✓ All health checks passed${NC}"
    exit 0
else
    echo -e "${RED}✗ $FAILED health check(s) failed${NC}"
    exit 1
fi
