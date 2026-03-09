#!/bin/bash
# Powertrain模块构建脚本

set -e  # 遇错即停

echo "========================================"
echo "Powertrain模块 - 构建验证"
echo "========================================"
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查依赖
echo "步骤1: 检查依赖库..."
if command -v cmake &> /dev/null; then
    echo -e "${GREEN}✓${NC} CMake已安装"
else
    echo -e "${RED}✗${NC} CMake未安装，请先安装"
    exit 1
fi

if [ -d "/usr/include/eigen3" ] || [ -d "/usr/local/include/eigen3" ]; then
    echo -e "${GREEN}✓${NC} Eigen3已安装"
else
    echo -e "${YELLOW}!${NC} Eigen3未找到，尝试自动安装..."
    sudo apt-get install -y libeigen3-dev
fi

if [ -f "/usr/lib/libgtest.a" ] || [ -f "/usr/local/lib/libgtest.a" ]; then
    echo -e "${GREEN}✓${NC} Google Test已安装"
else
    echo -e "${YELLOW}!${NC} Google Test未找到，尝试自动安装..."
    sudo apt-get install -y libgtest-dev
fi

echo ""
echo "步骤2: 创建构建目录..."
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo "清理旧构建..."
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "步骤3: 配置CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON \
    -DENABLE_COVERAGE=OFF

echo ""
echo "步骤4: 编译项目..."
cmake --build . -j$(nproc)

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} 编译成功！"
else
    echo -e "${RED}✗${NC} 编译失败！"
    exit 1
fi

echo ""
echo "步骤5: 运行测试..."
ctest --output-on-failure

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} 所有测试通过！"
else
    echo -e "${RED}✗${NC} 测试失败！"
    exit 1
fi

echo ""
echo "========================================"
echo "构建验证完成！"
echo "========================================"
echo ""
echo "生成文件:"
echo "  - libpowertrain.a"
echo "  - test_powertrain"
echo ""
echo "测试结果:"
ctest -N | grep "Total Tests"
echo ""

# 生成测试报告
echo "生成测试报告..."
ctest -T Test --output-on-failure
echo ""
echo -e "${GREEN}✓${NC} Powertrain模块验证成功！"
