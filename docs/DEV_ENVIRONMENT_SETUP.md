# 开发环境配置指南

本文档详细说明如何搭建EV Dynamics Simulation项目的开发环境。

## 目录

1. [系统要求](#系统要求)
2. [依赖安装](#依赖安装)
3. [构建步骤](#构建步骤)
4. [IDE配置](#ide配置)
5. [故障排查](#故障排查)
6. [开发工作流](#开发工作流)

---

## 系统要求

### 操作系统
- **Linux**: Ubuntu 20.04 或更高版本（推荐）
- **Windows**: Windows 10 或更高版本
- **macOS**: macOS 10.15 或更高版本

### 编译器
- **Linux**: GCC 9+ 或 Clang 10+
- **Windows**: Visual Studio 2019 或更高版本（MSVC）
- **macOS**: Xcode 12+ (Clang)

### 其他要求
- **Python**: 3.10 或更高版本
- **CMake**: 3.16 或更高版本
- **Git**: 2.20 或更高版本

---

## 依赖安装

### Linux (Ubuntu/Debian)

```bash
# 更新包列表
sudo apt-get update

# 安装编译工具
sudo apt-get install -y build-essential cmake git

# 安装Eigen3 (线性代数库)
sudo apt-get install -y libeigen3-dev

# 安装Google Test
sudo apt-get install -y libgtest-dev

# 编译Google Test (某些发行版需要)
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib/
cd -

# 安装Python依赖
pip install pybind11 pytest
```

### Windows

#### 方式1: 使用vcpkg（推荐）

```powershell
# 安装vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 安装依赖
.\vcpkg install eigen3:x64-windows
.\vcpkg install gtest:x64-windows
.\vcpkg install pybind11:x64-windows

# 集成到Visual Studio
.\vcpkg integrate install

# 设置环境变量
set VCPKG_ROOT=%CD%
```

#### 方式2: 手动安装

```powershell
# 安装Eigen3
git clone https://gitlab.com/libeigen/eigen.git
cd eigen
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=C:\Libraries\eigen3
cmake --build . --target INSTALL

# 安装Google Test
git clone https://github.com/google/googletest.git
cd googletest
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=C:\Libraries\gtest
cmake --build . --target INSTALL

# 安装Python依赖
pip install pybind11 pytest
```

### macOS

```bash
# 安装Homebrew（如果未安装）
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装编译工具
xcode-select --install

# 安装依赖
brew install cmake eigen googletest

# 安装Python依赖
pip install pybind11 pytest
```

---

## 构建步骤

### Linux/macOS

```bash
# 克隆项目
cd E:\workspace
git clone <repository-url> ev_dynamics_simulation
cd ev_dynamics_simulation

# 标准构建
./build.sh

# 调试构建
./build.sh --debug

# 清理构建
./build.sh --clean

# 构建并运行测试
./build.sh --test

# 详细输出
./build.sh --verbose
```

### Windows

```cmd
REM 克隆项目
cd E:\workspace
git clone <repository-url> ev_dynamics_simulation
cd ev_dynamics_simulation

REM 标准构建
build.bat

REM 调试构建
build.bat --debug

REM 清理构建
build.bat --clean

REM 构建并运行测试
build.bat --test

REM 详细输出
build.bat --verbose
```

### 手动构建（高级）

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 构建
cmake --build . --config Release

# 运行测试
ctest --output-on-failure

# 安装（可选）
cmake --install . --prefix /path/to/install
```

---

## IDE配置

### Visual Studio Code

1. **安装扩展**
   ```
   - C/C++ (Microsoft)
   - CMake Tools
   - Python
   - GitLens
   ```

2. **配置文件 (.vscode/settings.json)**
   ```json
   {
       "cmake.configureOnOpen": true,
       "cmake.buildDirectory": "${workspaceFolder}/build",
       "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
       "python.analysis.extraPaths": [
           "${workspaceFolder}/build/python_modules"
       ]
   }
   ```

3. **配置文件 (.vscode/launch.json)**
   ```json
   {
       "version": "0.2.0",
       "configurations": [
           {
               "name": "Python: Current File",
               "type": "python",
               "request": "launch",
               "program": "${file}",
               "console": "integratedTerminal",
               "env": {
                   "PYTHONPATH": "${workspaceFolder}/build/python_modules"
               }
           }
       ]
   }
   ```

### CLion

1. **打开项目**
   - File → Open → 选择项目根目录

2. **配置CMake**
   - File → Settings → Build → CMake
   - Build type: Release
   - CMake options: `-DCMAKE_BUILD_TYPE=Release`

3. **配置Python解释器**
   - File → Settings → Project → Python Interpreter
   - 添加项目虚拟环境

4. **运行配置**
   - Run → Edit Configurations
   - 添加Python运行配置
   - 设置环境变量: `PYTHONPATH=<项目路径>/build/python_modules`

### Visual Studio 2019/2022

1. **打开项目**
   - File → Open → CMake → 选择 `CMakeLists.txt`

2. **配置项目**
   - Project → CMake Settings
   - 添加x64-Release配置
   - 设置CMake变量

3. **构建项目**
   - Build → Build All

4. **调试**
   - 设置Python脚本为启动项
   - 调试 → 启动调试

---

## 故障排查

### 问题1: CMake找不到Eigen3

**Linux解决方案**:
```bash
sudo apt-get install libeigen3-dev
```

**Windows解决方案**:
```powershell
# 使用vcpkg
vcpkg install eigen3:x64-windows
vcpkg integrate install

# 或设置CMAKE_PREFIX_PATH
cmake .. -DCMAKE_PREFIX_PATH=C:\Libraries\eigen3
```

### 问题2: pybind11导入错误

**解决方案**:
```bash
# 确保Python版本兼容
python --version  # 应该是3.10+

# 重新安装pybind11
pip uninstall pybind11
pip install pybind11

# 检查Python路径
python -c "import pybind11; print(pybind11.get_cmake_dir())"
```

### 问题3: Python模块加载失败

**解决方案**:
```bash
# 检查模块路径
ls build/python_modules/

# 设置PYTHONPATH
export PYTHONPATH=$PYTHONPATH:$(pwd)/build/python_modules

# Windows
set PYTHONPATH=%PYTHONPATH%;%CD%\build\python_modules
```

### 问题4: 编译错误 - 找不到头文件

**解决方案**:
```bash
# 检查包含路径
cmake --system-information | grep CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES

# 清理并重新构建
./build.sh --clean
```

### 问题5: 测试失败

**解决方案**:
```bash
# 查看详细测试输出
cd build
ctest --output-on-failure --verbose

# 运行特定测试
ctest -R <test_name> --verbose
```

---

## 开发工作流

### 1. 创建新模块

```bash
# 创建模块目录
mkdir -p src/new_module/submodule1

# 创建CMakeLists.txt
cat > src/new_module/CMakeLists.txt << 'EOF'
# new_module/CMakeLists.txt
message(STATUS "Configuring new_module module...")

set(NEW_MODULE_SOURCES
    new_module_binding.cpp
)

add_python_module(new_module ${NEW_MODULE_SOURCES})

message(STATUS "New module configured")
EOF

# 创建绑定文件
touch src/new_module/new_module_binding.cpp
```

### 2. 添加新类

参考 `src/pybind11_template.cpp` 文件中的示例。

### 3. 编写测试

```cpp
// tests/test_new_module.cpp
#include <gtest/gtest.h>
#include "new_module/new_class.h"

TEST(NewClassTest, BasicTest) {
    NewClass obj;
    EXPECT_TRUE(obj.is_valid());
}
```

### 4. 提交代码

```bash
# 创建功能分支
git checkout -b feature/new-feature

# 添加修改
git add .

# 提交
git commit -m "feat: add new feature"

# 推送
git push origin feature/new-feature
```

---

## 性能优化建议

1. **编译优化**
   - Release模式: `-O3 -march=native`
   - Debug模式: `-O0 -g`

2. **并行编译**
   - Linux: `make -j$(nproc)`
   - Windows: `cmake --build . -- /m`

3. **CCache（可选）**
   ```bash
   sudo apt-get install ccache
   cmake .. -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
   ```

---

## 参考资料

- [CMake文档](https://cmake.org/documentation/)
- [pybind11文档](https://pybind11.readthedocs.io/)
- [Eigen3文档](https://eigen.tuxfamily.org/dox/)
- [Google Test文档](https://google.github.io/googletest/)

---

## 联系支持

如有问题，请联系项目维护者或查看GitHub Issues。

最后更新: 2025-03-06
