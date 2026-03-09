/**
 * @file vehicle_exceptions.h
 * @brief 车辆动力学仿真自定义异常类
 * @author CodeCraft
 * @date 2026-03-06
 * 
 * 提供完整的异常层次结构，用于不同的错误场景
 */

#ifndef VEHICLE_EXCEPTIONS_H
#define VEHICLE_EXCEPTIONS_H

#include <stdexcept>
#include <string>
#include <sstream>

namespace ev_simulation {

/**
 * @brief 基础异常类
 */
class VehicleException : public std::runtime_error {
public:
    explicit VehicleException(const std::string& message)
        : std::runtime_error(message) {}
    
    virtual ~VehicleException() = default;
    
    virtual const char* what() const noexcept override {
        return std::runtime_error::what();
    }
};

/**
 * @brief 配置错误异常
 */
class ConfigurationException : public VehicleException {
public:
    explicit ConfigurationException(const std::string& param_name, 
                                   const std::string& reason)
        : VehicleException(formatMessage(param_name, reason)),
          param_name_(param_name),
          reason_(reason) {}
    
    const std::string& getParamName() const { return param_name_; }
    const std::string& getReason() const { return reason_; }

private:
    static std::string formatMessage(const std::string& param_name,
                                    const std::string& reason) {
        std::ostringstream oss;
        oss << "Configuration error for parameter '" << param_name 
            << "': " << reason;
        return oss.str();
    }
    
    std::string param_name_;
    std::string reason_;
};

/**
 * @brief 参数验证异常
 */
class ValidationException : public VehicleException {
public:
    explicit ValidationException(const std::string& param_name,
                                double value,
                                const std::string& constraint)
        : VehicleException(formatMessage(param_name, value, constraint)),
          param_name_(param_name),
          value_(value),
          constraint_(constraint) {}
    
    const std::string& getParamName() const { return param_name_; }
    double getValue() const { return value_; }
    const std::string& getConstraint() const { return constraint_; }

private:
    static std::string formatMessage(const std::string& param_name,
                                    double value,
                                    const std::string& constraint) {
        std::ostringstream oss;
        oss << "Validation failed for '" << param_name 
            << "' with value " << value 
            << ": " << constraint;
        return oss.str();
    }
    
    std::string param_name_;
    double value_;
    std::string constraint_;
};

/**
 * @brief 数值计算异常
 */
class NumericalException : public VehicleException {
public:
    explicit NumericalException(const std::string& operation,
                               const std::string& reason)
        : VehicleException(formatMessage(operation, reason)),
          operation_(operation),
          reason_(reason) {}
    
    const std::string& getOperation() const { return operation_; }
    const std::string& getReason() const { return reason_; }

private:
    static std::string formatMessage(const std::string& operation,
                                    const std::string& reason) {
        std::ostringstream oss;
        oss << "Numerical error during '" << operation 
            << "': " << reason;
        return oss.str();
    }
    
    std::string operation_;
    std::string reason_;
};

/**
 * @brief 实时调度异常
 */
class RealtimeException : public VehicleException {
public:
    explicit RealtimeException(const std::string& component,
                              int error_code)
        : VehicleException(formatMessage(component, error_code)),
          component_(component),
          error_code_(error_code) {}
    
    const std::string& getComponent() const { return component_; }
    int getErrorCode() const { return error_code_; }

private:
    static std::string formatMessage(const std::string& component,
                                    int error_code) {
        std::ostringstream oss;
        oss << "Realtime error in '" << component 
            << "' with error code " << error_code;
        return oss.str();
    }
    
    std::string component_;
    int error_code_;
};

/**
 * @brief 初始化异常
 */
class InitializationException : public VehicleException {
public:
    explicit InitializationException(const std::string& component,
                                    const std::string& reason)
        : VehicleException(formatMessage(component, reason)),
          component_(component),
          reason_(reason) {}
    
    const std::string& getComponent() const { return component_; }
    const std::string& getReason() const { return reason_; }

private:
    static std::string formatMessage(const std::string& component,
                                    const std::string& reason) {
        std::ostringstream oss;
        oss << "Initialization failed for '" << component 
            << "': " << reason;
        return oss.str();
    }
    
    std::string component_;
    std::string reason_;
};

/**
 * @brief 边界条件异常
 */
class BoundaryException : public VehicleException {
public:
    explicit BoundaryException(const std::string& param_name,
                              double value,
                              double min_val,
                              double max_val)
        : VehicleException(formatMessage(param_name, value, min_val, max_val)),
          param_name_(param_name),
          value_(value),
          min_val_(min_val),
          max_val_(max_val) {}
    
    const std::string& getParamName() const { return param_name_; }
    double getValue() const { return value_; }
    double getMinValue() const { return min_val_; }
    double getMaxValue() const { return max_val_; }

private:
    static std::string formatMessage(const std::string& param_name,
                                    double value,
                                    double min_val,
                                    double max_val) {
        std::ostringstream oss;
        oss << "Boundary violation for '" << param_name 
            << "': value=" << value 
            << ", valid range=[" << min_val << ", " << max_val << "]";
        return oss.str();
    }
    
    std::string param_name_;
    double value_;
    double min_val_;
    double max_val_;
};

} // namespace ev_simulation

#endif // VEHICLE_EXCEPTIONS_H
