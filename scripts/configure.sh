#!/bin/bash
# EV Dynamics Simulation - Configuration Script
# Version: 1.0
# Date: 2026-03-06

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

CONFIG_DIR="${CONFIG_DIR:-/etc/ev_dynamics}"
CONFIG_FILE="$CONFIG_DIR/production.yaml"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}EV Dynamics - Configuration Wizard${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check if config exists
if [[ ! -f "$CONFIG_FILE" ]]; then
    echo -e "${YELLOW}Configuration file not found. Creating default...${NC}"
    mkdir -p "$CONFIG_DIR"
    cat > "$CONFIG_FILE" <<EOF
# EV Dynamics Simulation Configuration
# Generated: $(date)

simulation:
  # Simulation parameters
  dt: 0.001  # Time step (1ms)
  duration: 100.0  # Simulation duration (seconds)
  realtime: true  # Enable real-time simulation
  
vehicle:
  # Vehicle parameters
  mass: 1800.0  # Vehicle mass (kg)
  wheelbase: 2.7  # Wheelbase (m)
  track_width: 1.6  # Track width (m)
  cg_height: 0.5  # Center of gravity height (m)
  
powertrain:
  # Motor parameters
  motor_peak_power: 150000  # Peak power (W)
  motor_max_torque: 350  # Max torque (Nm)
  motor_max_speed: 15000  # Max speed (rpm)
  
  # Battery parameters
  battery_capacity: 75.0  # Capacity (kWh)
  battery_max_voltage: 400  # Max voltage (V)
  battery_min_voltage: 300  # Min voltage (V)
  
chassis:
  # Suspension parameters
  suspension_stiffness: 25000  # Spring stiffness (N/m)
  suspension_damping: 2500  # Damping coefficient (Ns/m)
  
  # Steering parameters
  steering_ratio: 15.0  # Steering ratio
  
  # Braking parameters
  brake_max_torque: 10000  # Max brake torque (Nm)

tire:
  # Tire model parameters (Pacejka)
  tire_radius: 0.33  # Tire radius (m)
  tire_width: 0.225  # Tire width (m)

realtime:
  # Real-time configuration
  cpu_affinity: [0, 1, 2, 3]  # CPU cores to use
  priority: 90  # Real-time priority (1-99)
  scheduler: "fifo"  # Scheduler: fifo, rr
  
logging:
  # Logging configuration
  level: "info"  # Log level: debug, info, warning, error
  file: "/var/log/ev_dynamics/simulation.log"
  max_size: 100  # Max log file size (MB)
  max_files: 10  # Max number of log files
EOF
    echo -e "${GREEN}✓ Default configuration created${NC}"
fi

echo ""
echo -e "${YELLOW}Configuration file location: $CONFIG_FILE${NC}"
echo ""
echo -e "${YELLOW}To edit configuration:${NC}"
echo "  nano $CONFIG_FILE"
echo "  vim $CONFIG_FILE"
echo ""
echo -e "${YELLOW}To validate configuration:${NC}"
echo "  ev_dynamics_sim --validate-config $CONFIG_FILE"
echo ""
