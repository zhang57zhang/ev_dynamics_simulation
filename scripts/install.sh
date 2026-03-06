#!/bin/bash
# EV Dynamics Simulation - Installation Script (Linux/macOS)
# Version: 1.0
# Date: 2026-03-06

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Print colored message
print_msg() {
    echo -e "${2}${1}${NC}"
}

print_success() {
    print_msg "$1" "$GREEN"
}

print_error() {
    print_msg "$1" "$RED"
}

print_warning() {
    print_msg "$1" "$YELLOW"
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    else
        echo "unknown"
    fi
}

OS=$(detect_os)
INSTALL_DIR="/usr/local"
BUILD_DIR="build"

print_msg "========================================" "$GREEN"
print_msg "EV Dynamics Simulation - Installer" "$GREEN"
print_msg "========================================" "$GREEN"
echo ""

# Step 1: Check prerequisites
print_msg "[1/7] Checking prerequisites..." "$YELLOW"

if ! command_exists cmake; then
    print_error "Error: CMake not found. Please install CMake 3.16+"
    exit 1
fi

if ! command_exists python3; then
    print_error "Error: Python 3 not found. Please install Python 3.10+"
    exit 1
fi

print_success "✓ Prerequisites check passed"

# Step 2: Install dependencies
print_msg "[2/7] Installing dependencies..." "$YELLOW"

if [ "$OS" == "linux" ]; then
    if command_exists apt-get; then
        sudo apt-get update
        sudo apt-get install -y build-essential cmake git \
            libeigen3-dev libgtest-dev libyaml-cpp-dev
    elif command_exists yum; then
        sudo yum groupinstall "Development Tools"
        sudo yum install -y cmake git eigen3-devel gtest-devel yaml-cpp-devel
    else
        print_warning "Package manager not detected. Please install dependencies manually."
    fi
elif [ "$OS" == "macos" ]; then
    if command_exists brew; then
        brew install cmake eigen googletest yaml-cpp
    else
        print_warning "Homebrew not found. Please install dependencies manually."
    fi
fi

print_success "✓ Dependencies installed"

# Step 3: Create build directory
print_msg "[3/7] Creating build directory..." "$YELLOW"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

print_success "✓ Build directory created"

# Step 4: Configure CMake
print_msg "[4/7] Configuring CMake..." "$YELLOW"

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DBUILD_TESTS=ON \
    -DBUILD_PYTHON_BINDINGS=ON

print_success "✓ CMake configured"

# Step 5: Build
print_msg "[5/7] Building project..." "$YELLOW"

NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
make -j"$NPROC"

print_success "✓ Build completed"

# Step 6: Run tests
print_msg "[6/7] Running tests..." "$YELLOW"

ctest --output-on-failure

if [ $? -eq 0 ]; then
    print_success "✓ All tests passed"
else
    print_error "✗ Some tests failed"
    exit 1
fi

# Step 7: Install
print_msg "[7/7] Installing..." "$YELLOW"

sudo make install

# Install Python package
cd ..
pip3 install -e .

print_success "✓ Installation completed"

echo ""
print_msg "========================================" "$GREEN"
print_success "✓ EV Dynamics Simulation installed successfully!"
print_msg "========================================" "$GREEN"
echo ""
print_msg "Installation directory: $INSTALL_DIR" "$GREEN"
print_msg "Binary: $INSTALL_DIR/bin/ev_dynamics_sim" "$GREEN"
print_msg "Python package: ev_dynamics" "$GREEN"
echo ""
print_msg "To get started, run:" "$YELLOW"
print_msg "  ev_dynamics_sim --help" "$YELLOW"
print_msg "  python3 -c \"import ev_dynamics; print(ev_dynamics.__version__)\"" "$YELLOW"
echo ""
