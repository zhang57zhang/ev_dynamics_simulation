# Windows 10 安装总结

**最简化的安装步骤清单**

---

## ✅ 必需软件（全部免费）

### 1. Visual Studio 2022 Community
```
下载: https://visualstudio.microsoft.com/downloads/
选择: Desktop development with C++
大小: ~20GB
时间: ~30分钟
```

### 2. CMake 3.16+
```
下载: https://cmake.org/download/
选择: cmake-3.25.x-windows-x86_64.msi
大小: ~50MB
时间: ~2分钟
```

### 3. Python 3.8+
```
下载: https://www.python.org/downloads/
选择: Python 3.10.x 或 3.11.x
大小: ~30MB
时间: ~2分钟
```

### 4. Git
```
下载: https://git-scm.com/download/win
大小: ~50MB
时间: ~2分钟
```

---

## 🚀 安装步骤

### 步骤 1: 安装 Visual Studio
1. 下载并运行安装程序
2. 选择 "使用 C++ 的桌面开发"
3. 点击安装（等待约 30 分钟）

### 步骤 2: 安装 CMake
1. 下载 MSI 安装包
2. 勾选 "Add CMake to PATH"
3. 完成安装

### 步骤 3: 安装 Python
1. 下载 Python 安装包
2. 勾选 "Add Python to PATH"
3. 完成安装

### 步骤 4: 安装 Python 依赖
```powershell
pip install pybind11 pytest numpy
```

### 步骤 5: 克隆项目
```powershell
git clone https://github.com/your-username/ev_dynamics_simulation.git
cd ev_dynamics_simulation
```

### 步骤 6: 检查环境
```powershell
.\scripts\check_environment.bat
```

### 步骤 7: 构建项目
```powershell
.\build.bat
```

### 步骤 8: 运行测试
```powershell
cd build
ctest -C Release
```

---

## 📊 时间估算

| 步骤 | 时间 |
|------|------|
| Visual Studio | 30 分钟 |
| CMake + Python + Git | 10 分钟 |
| 克隆项目 | 2 分钟 |
| 构建项目 | 5 分钟 |
| **总计** | **~50 分钟** |

---

## 🔍 验证安装

```powershell
# 1. 检查 CMake
cmake --version
# 应显示: cmake version 3.16+

# 2. 检查 Python
python --version
# 应显示: Python 3.8+

# 3. 检查 pybind11
python -c "import pybind11; print('OK')"
# 应显示: OK

# 4. 运行项目测试
ctest -C Release
# 应显示: 100% tests passed
```

---

## ❓ 遇到问题？

### 常见问题快速解决

**Q: CMake 找不到编译器**
```powershell
# 使用 Developer Command Prompt
# 开始菜单 → Visual Studio 2022 → Developer Command Prompt
```

**Q: Python 模块导入失败**
```powershell
# 设置 PYTHONPATH
$env:PYTHONPATH = "$PWD\build\python_modules"
```

**Q: 测试失败**
```powershell
# 清理并重新构建
.\build.bat --clean
```

---

## 📚 完整文档

- **详细指南:** [WINDOWS10_USER_GUIDE.md](docs/WINDOWS10_USER_GUIDE.md)
- **快速参考:** [WINDOWS10_QUICK_REFERENCE.md](WINDOWS10_QUICK_REFERENCE.md)
- **常见问题:** [FAQ.md](docs/FAQ.md)
- **故障排查:** [TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)

---

## 🆘 获取帮助

- **GitHub Issues:** https://github.com/your-username/ev_dynamics_simulation/issues
- **邮件支持:** support@example.com

---

*安装完成后，查看 [快速开始指南](QUICKSTART_GUIDE.md) 开始使用！*
