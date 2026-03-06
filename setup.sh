#!/bin/bash
# =============================================================================
# 快速启动脚本 - 验证开发环境
# =============================================================================

set -e

# 颜色
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

print_success() { echo -e "${GREEN}✓${NC} $1"; }
print_warning() { echo -e "${YELLOW}!${NC} $1"; }
print_error()   { echo -e "${RED}✗${NC} $1"; }
print_info()    { echo -e "ℹ $1"; }

echo "========================================"
echo "EV Dynamics Simulation - 环境检查"
echo "========================================"
echo ""

# 检查操作系统
OS=$(uname -s)
print_info "操作系统: $OS"

# 检查CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d" " -f3)
    print_success "CMake: $CMAKE_VERSION"
else
    print_error "CMake未安装"
    exit 1
fi

# 检查编译器
if command -v g++ &> /dev/null; then
    GCC_VERSION=$(g++ --version | head -n1)
    print_success "GCC: $GCC_VERSION"
elif command -v clang++ &> /dev/null; then
    CLANG_VERSION=$(clang++ --version | head -n1)
    print_success "Clang: $CLANG_VERSION"
else
    print_error "未找到C++编译器"
    exit 1
fi

# 检查Python
if command -v python3 &> /dev/null; then
    PYTHON_VERSION=$(python3 --version | cut -d" " -f2)
    print_success "Python: $PYTHON_VERSION"
else
    print_error "Python3未安装"
    exit 1
fi

# 检查pybind11
if python3 -c "import pybind11" 2>/dev/null; then
    PYBIND_VERSION=$(python3 -c "import pybind11; print(pybind11.__version__)")
    print_success "pybind11: $PYBIND_VERSION"
else
    print_warning "pybind11未安装 (pip install pybind11)"
fi

# 检查Eigen3
if [ -d "/usr/include/eigen3" ] || [ -d "/usr/local/include/eigen3" ]; then
    print_success "Eigen3: 已安装"
else
    print_warning "Eigen3未找到 (sudo apt-get install libeigen3-dev)"
fi

# 检查Google Test
if [ -f "/usr/include/gtest/gtest.h" ] || [ -f "/usr/local/include/gtest/gtest.h" ]; then
    print_success "Google Test: 已安装"
else
    print_warning "Google Test未找到 (sudo apt-get install libgtest-dev)"
fi

# 检查项目文件
echo ""
echo "========================================"
echo "项目文件检查"
echo "========================================"

FILES=(
    "CMakeLists.txt"
    "dependencies.cmake"
    "build.sh"
    "README.md"
    "src/powertrain/CMakeLists.txt"
    "src/chassis/CMakeLists.txt"
    "src/dynamics/CMakeLists.txt"
    "src/tire/CMakeLists.txt"
    "src/scheduler/CMakeLists.txt"
)

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        print_success "$file"
    else
        print_error "$file (缺失)"
    fi
done

# 检查构建目录
echo ""
echo "========================================"
echo "构建状态"
echo "========================================"

if [ -d "build" ]; then
    print_info "构建目录已存在"
    
    if [ -f "build/python_modules/powertrain.cpython-*.so" ] || \
       [ -f "build/python_modules/powertrain.*.pyd" ]; then
        print_success "Python模块已构建"
        
        # 尝试导入模块
        echo ""
        print_info "测试模块导入..."
        export PYTHONPATH=$PYTHONPATH:$(pwd)/build/python_modules
        
        if python3 -c "import powertrain" 2>/dev/null; then
            print_success "powertrain模块导入成功"
        else
            print_warning "powertrain模块导入失败"
        fi
        
        if python3 -c "import chassis" 2>/dev/null; then
            print_success "chassis模块导入成功"
        else
            print_warning "chassis模块导入失败"
        fi
    else
        print_info "Python模块未构建"
        print_info "运行 './build.sh' 进行构建"
    fi
else
    print_info "构建目录不存在"
    print_info "运行 './build.sh' 进行首次构建"
fi

# 总结
echo ""
echo "========================================"
echo "环境检查完成"
echo "========================================"
print_info "下一步操作:"
echo "  1. 安装缺失的依赖（如有）"
echo "  2. 运行 './build.sh' 构建项目"
echo "  3. 运行 'python tests/python/test_all_modules.py' 测试模块"
echo "  4. 查看 'docs/DEV_ENVIRONMENT_SETUP.md' 了解更多"
