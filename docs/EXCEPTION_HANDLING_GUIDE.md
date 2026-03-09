# 异常处理机制完善指南

## 概述

本文档说明六自由度车辆动力学仿真系统的异常处理机制，包括异常类型、处理策略和最佳实践。

---

## 异常层次结构

### 基础异常类

```cpp
namespace ev_simulation {

// 基础异常（所有异常的父类）
class VehicleException : public std::runtime_error

// 配置异常
class ConfigurationException : public VehicleException

// 参数验证异常
class ValidationException : public VehicleException

// 数值计算异常
class NumericalException : public VehicleException

// 实时调度异常
class RealtimeException : public VehicleException

// 初始化异常
class InitializationException : public VehicleException

// 边界条件异常
class BoundaryException : public VehicleException

}
```

---

## 使用示例

### 1. 参数验证

```cpp
#include "vehicle_exceptions.h"

void validateMass(double mass) {
    if (mass <= 0) {
        throw ValidationException("mass", mass, "must be positive");
    }
    if (mass < 500 || mass > 5000) {
        throw BoundaryException("mass", mass, 500, 5000);
    }
}

// 使用
try {
    validateMass(1500.0);  // OK
    validateMass(-100);    // 抛出 ValidationException
} catch (const ValidationException& e) {
    std::cerr << e.what() << std::endl;
}
```

### 2. 配置检查

```cpp
void loadConfig(const std::string& filename) {
    if (!std::ifstream(filename).good()) {
        throw ConfigurationException("config_file", "file not found");
    }
    // 加载配置...
}
```

### 3. 数值计算

```cpp
double divide(double a, double b) {
    if (std::abs(b) < 1e-10) {
        throw NumericalException("division", "division by zero");
    }
    return a / b;
}
```

### 4. 实时调度

```cpp
void setupRealtimeScheduler() {
    int result = pthread_setschedparam(/*...*/);
    if (result != 0) {
        throw RealtimeException("scheduler", result);
    }
}
```

---

## 全局异常处理

### 顶层异常捕获

```cpp
#include "exception_handler.h"

int main() {
    // 设置日志文件
    ev_simulation::ExceptionHandler::getInstance()
        .setLogFile("logs/error.log");
    
    // 顶层异常捕获
    try {
        // 主程序逻辑
        runSimulation();
        
    } catch (const ev_simulation::VehicleException& e) {
        ev_simulation::ExceptionHandler::getInstance()
            .handleException(e, "main");
        return 1;
        
    } catch (const std::exception& e) {
        ev_simulation::ExceptionHandler::getInstance()
            .handleException(e, "main");
        return 1;
        
    } catch (...) {
        ev_simulation::ExceptionHandler::getInstance().log(
            ev_simulation::LogLevel::CRITICAL,
            "Unknown exception in main"
        );
        return 1;
    }
    
    return 0;
}
```

### 使用宏简化

```cpp
TRY_CATCH_ALL(
    runSimulation();
, "main_simulation")
```

---

## 异常处理策略

### 可恢复异常
- **ConfigurationException** - 使用默认配置
- **ValidationException** - 记录日志，跳过该操作
- **BoundaryException** - 使用边界值

### 不可恢复异常
- **NumericalException** - 停止仿真
- **RealtimeException** - 停止实时调度

### 异常传播
```cpp
void lowLevelFunction() {
    // 低层函数抛出异常
    throw ValidationException("param", -1, "must be positive");
}

void midLevelFunction() {
    try {
        lowLevelFunction();
    } catch (const ValidationException& e) {
        // 添加上下文信息
        throw InitializationException("midLevelFunction", 
            "Failed due to: " + std::string(e.what()));
    }
}

void highLevelFunction() {
    try {
        midLevelFunction();
    } catch (const InitializationException& e) {
        // 记录并处理
        ExceptionHandler::getInstance().logException(e, "highLevelFunction");
    }
}
```

---

## 日志记录

### 日志级别

```cpp
// INFO - 一般信息
ExceptionHandler::getInstance().log(
    LogLevel::INFO, "Simulation started"
);

// WARNING - 警告
ExceptionHandler::getInstance().log(
    LogLevel::WARNING, "Parameter at boundary"
);

// ERROR - 错误
ExceptionHandler::getInstance().log(
    LogLevel::ERROR, "Simulation step failed"
);

// CRITICAL - 严重错误
ExceptionHandler::getInstance().log(
    LogLevel::CRITICAL, "System crash"
);
```

### 日志格式

```
2026-03-06 22:30:00 [ERROR] [simulation] Validation failed for 'mass' with value -100: must be positive
```

---

## 最佳实践

### 1. 早抛出，晚捕获
```cpp
// ✅ 好的做法
void validate(double value) {
    if (value <= 0) {
        throw ValidationException("value", value, "must be positive");
    }
}

// ❌ 不好的做法
void validate(double value) {
    try {
        if (value <= 0) {
            throw ValidationException("value", value, "must be positive");
        }
    } catch (...) {
        // 立即捕获
    }
}
```

### 2. 使用RAII守卫
```cpp
void criticalOperation() {
    ExceptionGuard guard("criticalOperation");
    
    // 如果发生异常，守卫会自动记录
    performOperation();
}
```

### 3. 异常安全保证
```cpp
class SafeContainer {
public:
    void addItem(const Item& item) {
        // 强异常安全保证
        std::vector<Item> temp = items_;
        temp.push_back(item);
        items_.swap(temp);  // 不会抛出异常
    }
    
private:
    std::vector<Item> items_;
};
```

### 4. 避免异常规范
```cpp
// ❌ C++17已弃用
void func() throw(std::exception);

// ✅ 使用noexcept
void func() noexcept;

// ✅ 或不指定
void func();
```

---

## 性能考虑

### 异常开销
- **正常路径：** 几乎无开销
- **异常路径：** 较大开销（栈展开）

### 何时使用异常
- **使用异常：**
  - 构造函数失败
  - 资源获取失败
  - 配置错误
  - 不可恢复错误

- **不使用异常：**
  - 频繁的操作
  - 性能关键路径
  - 预期的错误（如文件不存在）

---

## 测试异常处理

```cpp
#include <gtest/gtest.h>
#include "vehicle_exceptions.h"

TEST(ExceptionTest, ValidationException) {
    EXPECT_THROW(
        validateMass(-100),
        ValidationException
    );
    
    try {
        validateMass(-100);
    } catch (const ValidationException& e) {
        EXPECT_EQ(e.getParamName(), "mass");
        EXPECT_EQ(e.getValue(), -100);
        EXPECT_EQ(e.getConstraint(), "must be positive");
    }
}

TEST(ExceptionTest, BoundaryException) {
    EXPECT_THROW(
        validateMass(100),  // < 500
        BoundaryException
    );
    
    try {
        validateMass(100);
    } catch (const BoundaryException& e) {
        EXPECT_EQ(e.getMinValue(), 500);
        EXPECT_EQ(e.getMaxValue(), 5000);
    }
}
```

---

## 总结

### 已实现
- ✅ 完整的异常层次结构
- ✅ 全局异常处理器
- ✅ 日志记录系统
- ✅ RAII守卫
- ✅ 详细错误信息

### 使用原则
1. 使用合适的异常类型
2. 提供清晰的错误信息
3. 在顶层捕获异常
4. 记录所有异常
5. 测试异常路径

---

**异常处理机制已完善！** ✅
