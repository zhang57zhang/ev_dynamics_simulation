# 项目概览

## 快速导航

### 📁 目录结构

```
ev_dynamics_simulation/
├── 📂 src/                    # 源代码（5个核心模块）
│   ├── powertrain/            # 动力系统（Agent 1）
│   ├── chassis/               # 底盘系统（Agent 2）
│   ├── dynamics/              # 车辆动力学（Agent 3）
│   ├── tire/                  # 轮胎模型（Agent 4）
│   ├── scheduler/             # 实时调度（Agent 5）
│   └── pybind11_template.cpp  # Python绑定模板
│
├── 📂 include/                # 公共头文件
├── 📂 tests/                  # 测试代码
│   └── python/                # Python测试
│
├── 📂 config/                 # 配置文件
│   └── simulation_config.yaml # 仿真配置模板
│
├── 📂 docs/                   # 文档
│   ├── DEV_ENVIRONMENT_SETUP.md    # 环境配置指南
│   └── AGENT_DEVELOPMENT_GUIDE.md  # Agent开发指南
│
├── 📂 .github/workflows/      # CI/CD配置
│   └── ci.yml                 # 持续集成
│
├── 📄 CMakeLists.txt          # 顶层CMake配置
├── 📄 dependencies.cmake      # 依赖管理
├── 📄 build.sh                # Linux构建脚本
├── 📄 build.bat               # Windows构建脚本
├── 📄 setup.sh                # Linux环境检查
├── 📄 setup.bat               # Windows环境检查
├── 📄 Makefile                # Make快捷命令
├── 📄 Doxyfile                # 文档生成配置
├── 📄 README.md               # 项目说明
└── 📄 .gitignore              # Git忽略规则
```

### 🚀 快速开始

#### 1. 环境检查
```bash
# Linux/macOS
./setup.sh

# Windows
setup.bat
```

#### 2. 构建项目
```bash
# Linux/macOS
./build.sh

# Windows
build.bat
```

#### 3. 测试模块
```bash
# Python测试
python tests/python/test_all_modules.py

# C++测试
cd build && ctest --output-on-failure
```

### 📚 文档导航

| 文档 | 描述 | 适用对象 |
|------|------|---------|
| [README.md](README.md) | 项目简介和快速开始 | 所有人 |
| [docs/DEV_ENVIRONMENT_SETUP.md](docs/DEV_ENVIRONMENT_SETUP.md) | 开发环境配置详解 | 新开发者 |
| [docs/AGENT_DEVELOPMENT_GUIDE.md](docs/AGENT_DEVELOPMENT_GUIDE.md) | Agent开发指南 | 5个Agent |
| [config/simulation_config.yaml](config/simulation_config.yaml) | 仿真配置模板 | 配置人员 |

### 🛠️ 构建选项

```bash
# 调试构建
./build.sh --debug

# 清理构建
./build.sh --clean

# 构建并测试
./build.sh --test

# 详细输出
./build.sh --verbose
```

### 🧪 测试

```bash
# 运行所有测试
make test

# 运行特定测试
cd build
ctest -R test_powertrain --output-on-failure

# Python测试
python -m pytest tests/python/ -v
```

### 📦 依赖列表

| 依赖 | 版本要求 | 用途 |
|------|---------|------|
| CMake | ≥ 3.16 | 构建系统 |
| GCC/MSVC | GCC 9+ / MSVC 2019+ | C++编译器 |
| Python | ≥ 3.10 | Python环境 |
| Eigen3 | ≥ 3.3 | 线性代数 |
| pybind11 | 最新 | Python绑定 |
| Google Test | 最新 | 单元测试 |

### 🤝 Agent分工

| Agent | 模块 | 主要文件 |
|-------|------|---------|
| Agent 1 | powertrain | `src/powertrain/` |
| Agent 2 | chassis | `src/chassis/` |
| Agent 3 | dynamics | `src/dynamics/` |
| Agent 4 | tire | `src/tire/` |
| Agent 5 | scheduler | `src/scheduler/` |

### 🔗 重要链接

- **CI/CD**: `.github/workflows/ci.yml`
- **配置模板**: `config/simulation_config.yaml`
- **代码模板**: `src/pybind11_template.cpp`
- **Make快捷命令**: `Makefile`

### 💡 常用命令

```bash
# 快速构建
make build

# 清理
make clean

# 测试
make test

# 生成文档
make docs

# 代码格式化
make format

# 静态分析
make analyze
```

### 📊 项目状态

- ✅ 开发环境配置完成
- ✅ CMake构建系统就绪
- ✅ Python绑定模板就绪
- ✅ CI/CD流程配置完成
- ⏳ 核心模型实现中

### 🎯 下一步

1. **各Agent**: 阅读 [Agent开发指南](docs/AGENT_DEVELOPMENT_GUIDE.md)
2. **开发环境**: 按照 [环境配置指南](docs/DEV_ENVIRONMENT_SETUP.md) 安装依赖
3. **开始开发**: 参考模板文件开始实现各自模块

---

**最后更新**: 2025-03-06
**维护者**: DevOpsAgent
