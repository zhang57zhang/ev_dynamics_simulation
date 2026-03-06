#!/bin/bash
# EV Dynamics Simulation - Deployment Script
# Version: 1.0
# Date: 2026-03-06

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}ℹ $1${NC}"
}

# Configuration
DEPLOY_DIR="${DEPLOY_DIR:-/opt/ev_dynamics}"
CONFIG_DIR="${CONFIG_DIR:-/etc/ev_dynamics}"
LOG_DIR="${LOG_DIR:-/var/log/ev_dynamics}"
USER="${EV_DYNAMICS_USER:-evdynamics}"
SERVICE_NAME="ev_dynamics"

print_header "EV Dynamics Simulation - Deployment"

# Step 1: Pre-deployment checks
print_info "[1/8] Running pre-deployment checks..."

if [[ $EUID -ne 0 ]]; then
   print_error "This script must be run as root"
   exit 1
fi

print_success "Pre-deployment checks passed"

# Step 2: Create deployment directories
print_info "[2/8] Creating deployment directories..."

mkdir -p "$DEPLOY_DIR"
mkdir -p "$DEPLOY_DIR/bin"
mkdir -p "$DEPLOY_DIR/lib"
mkdir -p "$DEPLOY_DIR/share"
mkdir -p "$CONFIG_DIR"
mkdir -p "$LOG_DIR"

print_success "Directories created"

# Step 3: Create user if not exists
print_info "[3/8] Creating service user..."

if ! id -u "$USER" >/dev/null 2>&1; then
    useradd -r -s /bin/false -d "$DEPLOY_DIR" "$USER"
    print_success "User $USER created"
else
    print_info "User $USER already exists"
fi

# Step 4: Copy binaries
print_info "[4/8] Copying binaries..."

cp build/bin/ev_dynamics_sim "$DEPLOY_DIR/bin/"
cp -r build/lib/* "$DEPLOY_DIR/lib/" 2>/dev/null || true

print_success "Binaries copied"

# Step 5: Copy configuration files
print_info "[5/8] Copying configuration files..."

cp config/production.yaml "$CONFIG_DIR/"
cp config/monitoring.yaml "$CONFIG_DIR/"
cp config/security.yaml "$CONFIG_DIR/"

# Set secure permissions
chmod 600 "$CONFIG_DIR/security.yaml"

print_success "Configuration files copied"

# Step 6: Install systemd service
print_info "[6/8] Installing systemd service..."

cat > /etc/systemd/system/${SERVICE_NAME}.service <<EOF
[Unit]
Description=EV Dynamics Simulation Service
After=network.target

[Service]
Type=simple
User=$USER
Group=$USER
WorkingDirectory=$DEPLOY_DIR
ExecStart=$DEPLOY_DIR/bin/ev_dynamics_sim --config $CONFIG_DIR/production.yaml
Restart=on-failure
RestartSec=5

# Security settings
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=$LOG_DIR

# Resource limits
LimitNOFILE=65536
LimitNPROC=4096

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable ${SERVICE_NAME}

print_success "Systemd service installed"

# Step 7: Set permissions
print_info "[7/8] Setting permissions..."

chown -R "$USER:$USER" "$DEPLOY_DIR"
chown -R "$USER:$USER" "$CONFIG_DIR"
chown -R "$USER:$USER" "$LOG_DIR"

chmod 755 "$DEPLOY_DIR/bin/ev_dynamics_sim"

print_success "Permissions set"

# Step 8: Post-deployment verification
print_info "[8/8] Running post-deployment verification..."

# Check binary
if [[ -x "$DEPLOY_DIR/bin/ev_dynamics_sim" ]]; then
    print_success "Binary executable"
else
    print_error "Binary not executable"
    exit 1
fi

# Check configuration
if [[ -f "$CONFIG_DIR/production.yaml" ]]; then
    print_success "Configuration file present"
else
    print_error "Configuration file missing"
    exit 1
fi

# Test service start
systemctl start ${SERVICE_NAME}
sleep 2

if systemctl is-active --quiet ${SERVICE_NAME}; then
    print_success "Service started successfully"
    systemctl stop ${SERVICE_NAME}
else
    print_error "Service failed to start"
    journalctl -u ${SERVICE_NAME} -n 20
    exit 1
fi

print_header "Deployment Completed Successfully"

echo ""
print_info "Deployment Information:"
echo "  Installation Directory: $DEPLOY_DIR"
echo "  Configuration Directory: $CONFIG_DIR"
echo "  Log Directory: $LOG_DIR"
echo "  Service Name: $SERVICE_NAME"
echo ""
print_info "To start the service:"
echo "  sudo systemctl start $SERVICE_NAME"
echo ""
print_info "To check service status:"
echo "  sudo systemctl status $SERVICE_NAME"
echo ""
print_info "To view logs:"
echo "  sudo journalctl -u $SERVICE_NAME -f"
echo ""
