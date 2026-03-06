#!/bin/bash
# EV Dynamics Simulation - Start Script
# Version: 1.0
# Date: 2026-03-06

SERVICE_NAME="ev_dynamics"
CONFIG_DIR="${CONFIG_DIR:-/etc/ev_dynamics}"

echo "Starting EV Dynamics Simulation..."

# Check if service is already running
if systemctl is-active --quiet ${SERVICE_NAME}; then
    echo "Service is already running"
    systemctl status ${SERVICE_NAME}
    exit 0
fi

# Start service
sudo systemctl start ${SERVICE_NAME}

# Wait for service to start
sleep 2

# Check status
if systemctl is-active --quiet ${SERVICE_NAME}; then
    echo "✓ Service started successfully"
    systemctl status ${SERVICE_NAME}
else
    echo "✗ Service failed to start"
    journalctl -u ${SERVICE_NAME} -n 20
    exit 1
fi
