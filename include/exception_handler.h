/**
 * @file exception_handler.h
 * @brief 全局异常处理器
 * @author CodeCraft
 * @date 2026-03-06
 * 
 * 提供顶层异常捕获和日志记录功能
 */

#ifndef EXCEPTION_HANDLER_H
#define EXCEPTION_HANDLER_H

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <memory>
#include "vehicle_exceptions.h"

namespace ev_simulation {

/**
 * @brief 日志级别
 */
enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/**
 * @brief 异常处理器
 */
class ExceptionHandler {
public:
    static ExceptionHandler& getInstance() {
        static ExceptionHandler instance;
        return instance;
    }
    
    /**
     * @brief 处理异常
     */
    void handleException(const std::exception& e, 
                        const std::string& context = "") {
        logException(e, context);
        
        // 根据异常类型决定是否重新抛出
        if (shouldRethrow(e)) {
            throw;
        }
    }
    
    /**
     * @brief 设置日志文件
     */
    void setLogFile(const std::string& filename) {
        log_file_ = filename;
        log_stream_.open(filename, std::ios::app);
    }
    
    /**
     * @brief 记录异常日志
     */
    void logException(const std::exception& e, 
                     const std::string& context = "") {
        std::ostringstream oss;
        
        // 时间戳
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        
        // 日志级别
        oss << " [ERROR] ";
        
        // 上下文
        if (!context.empty()) {
            oss << "[" << context << "] ";
        }
        
        // 异常信息
        oss << e.what() << std::endl;
        
        // 输出到控制台
        std::cerr << oss.str();
        
        // 输出到文件
        if (log_stream_.is_open()) {
            log_stream_ << oss.str();
            log_stream_.flush();
        }
    }
    
    /**
     * @brief 通用日志记录
     */
    void log(LogLevel level, const std::string& message,
            const std::string& context = "") {
        std::ostringstream oss;
        
        // 时间戳
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        
        // 日志级别
        oss << " [" << levelToString(level) << "] ";
        
        // 上下文
        if (!context.empty()) {
            oss << "[" << context << "] ";
        }
        
        // 消息
        oss << message << std::endl;
        
        // 输出
        if (level >= LogLevel::ERROR) {
            std::cerr << oss.str();
        } else {
            std::cout << oss.str();
        }
        
        if (log_stream_.is_open()) {
            log_stream_ << oss.str();
            log_stream_.flush();
        }
    }

private:
    ExceptionHandler() = default;
    ~ExceptionHandler() {
        if (log_stream_.is_open()) {
            log_stream_.close();
        }
    }
    
    ExceptionHandler(const ExceptionHandler&) = delete;
    ExceptionHandler& operator=(const ExceptionHandler&) = delete;
    
    bool shouldRethrow(const std::exception& e) {
        // 数值异常和实时异常应该重新抛出
        // 配置异常和验证异常可以处理
        return dynamic_cast<const NumericalException*>(&e) != nullptr ||
               dynamic_cast<const RealtimeException*>(&e) != nullptr;
    }
    
    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
    std::string log_file_;
    std::ofstream log_stream_;
};

/**
 * @brief RAII异常守卫
 */
class ExceptionGuard {
public:
    explicit ExceptionGuard(const std::string& context)
        : context_(context) {}
    
    ~ExceptionGuard() {
        if (std::uncaught_exception()) {
            ExceptionHandler::getInstance().log(
                LogLevel::ERROR,
                "Exception during context: " + context_
            );
        }
    }
    
private:
    std::string context_;
};

/**
 * @brief 顶层异常处理宏
 */
#define TRY_CATCH_ALL(block, context) \
    try { \
        ExceptionGuard guard(context); \
        block \
    } catch (const ev_simulation::VehicleException& e) { \
        ev_simulation::ExceptionHandler::getInstance().handleException(e, context); \
    } catch (const std::exception& e) { \
        ev_simulation::ExceptionHandler::getInstance().handleException(e, context); \
    } catch (...) { \
        ev_simulation::ExceptionHandler::getInstance().log( \
            ev_simulation::LogLevel::CRITICAL, \
            "Unknown exception occurred", \
            context \
        ); \
    }

} // namespace ev_simulation

#endif // EXCEPTION_HANDLER_H
