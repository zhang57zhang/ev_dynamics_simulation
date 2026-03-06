#!/bin/bash
# EV Dynamics Simulation - Stop Script
# Version: 1.0
# Date: 2026-03-06

SERVICE_NAME="ev_dynamics"

echo "Stopping EV Dynamics Simulation..."

# Check if service is running
if ! systemctl is-active --quiet ${SERVICE_NAME}; then
    echo "Service is not running"
    exit 0
fi

# Stop service
sudo systemctl stop ${SERVICE_NAME}

# Wait for service to stop
sleep 2

# Check status
if ! systemctl is-active --quiet ${SERVICE_NAME}; then
    echo "✓ Service stopped successfully"
else
    echo "✗ Service failed to stop"
    systemctl status ${SERVICE_NAME}
    exit 1
fi
