#!/bin/bash
# =============================================================================
# 构建脚本 - Linux/macOS
# EV Dynamics Simulation Project
# =============================================================================

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 打印函数
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# =============================================================================
# 检查依赖
# =============================================================================
print_info "Checking dependencies..."

# 检查CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake not found! Please install cmake (3.16 or later)"
    exit 1
fi
CMAKE_VERSION=$(cmake --version | head -n1 | cut -d" " -f3)
print_info "CMake version: $CMAKE_VERSION"

# 检查编译器
if command -v g++ &> /dev/null; then
    GCC_VERSION=$(g++ --version | head -n1)
    print_info "GCC found: $GCC_VERSION"
elif command -v clang++ &> /dev/null; then
    CLANG_VERSION=$(clang++ --version | head -n1)
    print_info "Clang found: $CLANG_VERSION"
else
    print_error "No C++ compiler found! Please install g++ or clang++"
    exit 1
fi

# 检查Python
if ! command -v python3 &> /dev/null; then
    print_error "Python3 not found!"
    exit 1
fi
PYTHON_VERSION=$(python3 --version)
print_info "Python version: $PYTHON_VERSION"

# =============================================================================
# 配置选项
# =============================================================================
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR="build"
CLEAN_BUILD=false
RUN_TESTS=false
VERBOSE=false

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug       Build in Debug mode"
            echo "  --release     Build in Release mode (default)"
            echo "  --clean       Clean build directory before building"
            echo "  --test        Run tests after build"
            echo "  --verbose, -v Enable verbose output"
            echo "  --help, -h    Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# =============================================================================
# 清理构建目录
# =============================================================================
if [ "$CLEAN_BUILD" = true ]; then
    print_info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# =============================================================================
# 创建构建目录
# =============================================================================
print_info "Creating build directory..."
mkdir -p "$BUILD_DIR"

# =============================================================================
# 配置项目
# =============================================================================
print_info "Configuring project (Build type: $BUILD_TYPE)..."

CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)

if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

cd "$BUILD_DIR"
cmake .. "${CMAKE_ARGS[@]}"

# =============================================================================
# 构建项目
# =============================================================================
print_info "Building project..."

# 获取CPU核心数
if command -v nproc &> /dev/null; then
    NPROC=$(nproc)
elif command -v sysctl &> /dev/null; then
    NPROC=$(sysctl -n hw.ncpu)
else
    NPROC=4
fi

print_info "Using $NPROC parallel jobs"

if [ "$VERBOSE" = true ]; then
    make VERBOSE=1 -j"$NPROC"
else
    make -j"$NPROC"
fi

# =============================================================================
# 运行测试
# =============================================================================
if [ "$RUN_TESTS" = true ]; then
    print_info "Running tests..."
    ctest --output-on-failure
fi

# =============================================================================
# 完成
# =============================================================================
print_info "Build completed successfully!"
print_info "Build artifacts are in: $BUILD_DIR/"
print_info "Python modules are in: $BUILD_DIR/python_modules/"

echo ""
print_info "To use the Python modules:"
echo "  export PYTHONPATH=\"\$PYTHONPATH:$(pwd)/python_modules\""
echo "  python3 -c \"import powertrain; print('Powertrain module loaded!')\""
