/**
 * @file ipc_manager.h
 * @brief IPC管理器实现 - 支持共享内存、消息队列和性能监控
 * @author EV Dynamics Simulation Team
 * @date 2026-03-08
 * @version 1.0
 * 
 * @details 本模块实现了高性能的进程间通信机制，包括：
 * - POSIX共享内存
 * - POSIX消息队列
 * - System V共享内存（可选）
 * - 性能监控与统计
 * - 线程安全的数据访问
 */

#ifndef IPC_MANAGER_H
#define IPC_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <cstring>
#include <stdexcept>
#include <chrono>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <mqueue.h>
#include <semaphore.h>
#endif

namespace ev_dynamics {
namespace scheduler {

/**
 * @brief IPC错误代码
 */
enum class IPCErrorCode {
    SUCCESS = 0,                ///< 成功
    INVALID_PARAMETER,          ///< 无效参数
    ALREADY_EXISTS,             ///< 已存在
    NOT_FOUND,                  ///< 未找到
    PERMISSION_DENIED,          ///< 权限拒绝
    OUT_OF_MEMORY,              ///< 内存不足
    TIMEOUT,                    ///< 超时
    INTERNAL_ERROR              ///< 内部错误
};

/**
 * @brief IPC结果类型
 */
template<typename T>
struct IPCResult {
    IPCErrorCode error_code;    ///< 错误代码
    std::string error_message;  ///< 错误消息
    T value;                    ///< 返回值
    
    /**
     * @brief 构造函数
     * @param code 错误代码
     * @param msg 错误消息
     * @param val 返回值
     */
    IPCResult(IPCErrorCode code = IPCErrorCode::SUCCESS, 
              const std::string& msg = "", 
              const T& val = T())
        : error_code(code), error_message(msg), value(val) {}
    
    /**
     * @brief 检查是否成功
     * @return true 如果成功
     */
    bool isSuccess() const {
        return error_code == IPCErrorCode::SUCCESS;
    }
    
    /**
     * @brief 创建成功结果
     * @param val 返回值
     * @return 成功结果
     */
    static IPCResult<T> success(const T& val = T()) {
        return IPCResult<T>(IPCErrorCode::SUCCESS, "", val);
    }
    
    /**
     * @brief 创建错误结果
     * @param code 错误代码
     * @param msg 错误消息
     * @return 错误结果
     */
    static IPCResult<T> error(IPCErrorCode code, const std::string& msg) {
        return IPCResult<T>(code, msg);
    }
};

/**
 * @brief 共享内存统计信息
 */
struct SharedMemoryStatistics {
    size_t total_size;              ///< 总大小（字节）
    size_t used_size;               ///< 已使用大小（字节）
    uint64_t read_count;            ///< 读次数
    uint64_t write_count;           ///< 写次数
    uint64_t read_bytes;            ///< 读取字节数
    uint64_t write_bytes;           ///< 写入字节数
    uint64_t access_errors;         ///< 访问错误次数
    std::chrono::steady_clock::time_point creation_time; ///< 创建时间
    
    /**
     * @brief 构造函数
     */
    SharedMemoryStatistics()
        : total_size(0)
        , used_size(0)
        , read_count(0)
        , write_count(0)
        , read_bytes(0)
        , write_bytes(0)
        , access_errors(0)
        , creation_time(std::chrono::steady_clock::now()) {}
    
    /**
     * @brief 重置统计
     */
    void reset() {
        *this = SharedMemoryStatistics();
    }
    
    /**
     * @brief 计算使用率
     * @return 使用率（0.0-1.0）
     */
    double utilizationRate() const {
        return (total_size > 0) ? static_cast<double>(used_size) / total_size : 0.0;
    }
};

/**
 * @brief 消息队列统计信息
 */
struct MessageQueueStatistics {
    uint64_t messages_sent;         ///< 已发送消息数
    uint64_t messages_received;     ///< 已接收消息数
    uint64_t bytes_sent;            ///< 已发送字节数
    uint64_t bytes_received;        ///< 已接收字节数
    uint64_t send_errors;           ///< 发送错误次数
    uint64_t receive_errors;        ///< 接收错误次数
    uint64_t queue_full_count;      ///< 队列满次数
    uint64_t queue_empty_count;     ///< 队列空次数
    size_t current_queue_size;      ///< 当前队列大小
    size_t max_queue_size;          ///< 最大队列大小
    std::chrono::steady_clock::time_point creation_time; ///< 创建时间
    
    /**
     * @brief 构造函数
     */
    MessageQueueStatistics()
        : messages_sent(0)
        , messages_received(0)
        , bytes_sent(0)
        , bytes_received(0)
        , send_errors(0)
        , receive_errors(0)
        , queue_full_count(0)
        , queue_empty_count(0)
        , current_queue_size(0)
        , max_queue_size(0)
        , creation_time(std::chrono::steady_clock::now()) {}
    
    /**
     * @brief 重置统计
     */
    void reset() {
        *this = MessageQueueStatistics();
    }
    
    /**
     * @brief 计算平均消息大小
     * @return 平均消息大小（字节）
     */
    double averageMessageSize() const {
        if (messages_sent == 0) return 0.0;
        return static_cast<double>(bytes_sent) / messages_sent;
    }
};

/**
 * @brief 消息结构
 */
struct Message {
    uint32_t message_id;            ///< 消息ID
    uint32_t message_type;          ///< 消息类型
    uint32_t sender_id;             ///< 发送者ID
    uint32_t receiver_id;           ///< 接收者ID
    uint64_t timestamp;             ///< 时间戳（微秒）
    uint32_t data_size;             ///< 数据大小
    uint8_t priority;               ///< 优先级
    uint8_t reserved[3];            ///< 保留字段
    
    static constexpr size_t MAX_DATA_SIZE = 4096; ///< 最大数据大小（字节）
    uint8_t data[MAX_DATA_SIZE];    ///< 消息数据
    
    /**
     * @brief 默认构造函数
     */
    Message()
        : message_id(0)
        , message_type(0)
        , sender_id(0)
        , receiver_id(0)
        , timestamp(0)
        , data_size(0)
        , priority(0) {
        memset(data, 0, MAX_DATA_SIZE);
    }
    
    /**
     * @brief 构造函数
     * @param type 消息类型
     * @param sender 发送者ID
     * @param receiver 接收者ID
     */
    Message(uint32_t type, uint32_t sender, uint32_t receiver)
        : message_id(0)
        , message_type(type)
        , sender_id(sender)
        , receiver_id(receiver)
        , timestamp(0)
        , data_size(0)
        , priority(0) {
        memset(data, 0, MAX_DATA_SIZE);
    }
    
    /**
     * @brief 设置数据
     * @param data_ptr 数据指针
     * @param size 数据大小
     * @return true 如果设置成功
     */
    bool setData(const void* data_ptr, size_t size) {
        if (size > MAX_DATA_SIZE) {
            return false;
        }
        memcpy(data, data_ptr, size);
        data_size = static_cast<uint32_t>(size);
        return true;
    }
    
    /**
     * @brief 获取数据
     * @param buffer 数据缓冲区
     * @param buffer_size 缓冲区大小
     * @return 实际读取的大小
     */
    size_t getData(void* buffer, size_t buffer_size) const {
        size_t copy_size = (buffer_size < data_size) ? buffer_size : data_size;
        memcpy(buffer, data, copy_size);
        return copy_size;
    }
    
    /**
     * @brief 计算消息总大小
     * @return 总大小（字节）
     */
    size_t totalSize() const {
        return sizeof(Message) - MAX_DATA_SIZE + data_size;
    }
};

/**
 * @brief 共享内存配置
 */
struct SharedMemoryConfig {
    std::string name;               ///< 共享内存名称
    size_t size;                    ///< 大小（字节）
    bool read_only;                 ///< 是否只读
    bool create_new;                ///< 是否创建新的
    bool auto_cleanup;              ///< 是否自动清理
    
    /**
     * @brief 构造函数
     * @param shm_name 共享内存名称
     * @param shm_size 大小
     */
    SharedMemoryConfig(const std::string& shm_name = "", size_t shm_size = 4096)
        : name(shm_name)
        , size(shm_size)
        , read_only(false)
        , create_new(true)
        , auto_cleanup(true) {}
};

/**
 * @brief 消息队列配置
 */
struct MessageQueueConfig {
    std::string name;               ///< 队列名称
    size_t max_messages;            ///< 最大消息数
    size_t max_message_size;        ///< 最大消息大小
    bool create_new;                ///< 是否创建新的
    bool auto_cleanup;              ///< 是否自动清理
    
    /**
     * @brief 构造函数
     * @param queue_name 队列名称
     */
    MessageQueueConfig(const std::string& queue_name = "")
        : name(queue_name)
        , max_messages(100)
        , max_message_size(sizeof(Message))
        , create_new(true)
        , auto_cleanup(true) {}
};

/**
 * @brief 共享内存类
 * 
 * 提供POSIX共享内存的封装，支持跨进程的内存共享。
 */
class SharedMemory {
public:
    /**
     * @brief 构造函数
     */
    SharedMemory();
    
    /**
     * @brief 析构函数
     */
    ~SharedMemory();
    
    /**
     * @brief 创建共享内存
     * @param config 配置
     * @return 操作结果
     */
    IPCResult<bool> create(const SharedMemoryConfig& config);
    
    /**
     * @brief 打开已存在的共享内存
     * @param name 名称
     * @param read_only 是否只读
     * @return 操作结果
     */
    IPCResult<bool> open(const std::string& name, bool read_only = false);
    
    /**
     * @brief 关闭共享内存
     */
    void close();
    
    /**
     * @brief 写入数据
     * @param offset 偏移量
     * @param data 数据指针
     * @param size 数据大小
     * @return 实际写入的大小
     */
    IPCResult<size_t> write(size_t offset, const void* data, size_t size);
    
    /**
     * @brief 读取数据
     * @param offset 偏移量
     * @param buffer 缓冲区
     * @param buffer_size 缓冲区大小
     * @return 实际读取的大小
     */
    IPCResult<size_t> read(size_t offset, void* buffer, size_t buffer_size) const;
    
    /**
     * @brief 获取指针
     * @return 内存指针
     */
    void* getPointer() const;
    
    /**
     * @brief 获取大小
     * @return 大小（字节）
     */
    size_t getSize() const;
    
    /**
     * @brief 获取名称
     * @return 名称
     */
    std::string getName() const;
    
    /**
     * @brief 检查是否已打开
     * @return true 如果已打开
     */
    bool isOpen() const;
    
    /**
     * @brief 获取统计信息
     * @return 统计信息
     */
    SharedMemoryStatistics getStatistics() const;
    
    /**
     * @brief 重置统计信息
     */
    void resetStatistics();
    
    /**
     * @brief 清零内存
     */
    void zeroMemory();
    
    /**
     * @brief 删除共享内存
     * @param name 名称
     * @return 操作结果
     */
    static IPCResult<bool> unlink(const std::string& name);

private:
    std::string name_;              ///< 名称
    size_t size_;                   ///< 大小
    void* memory_ptr_;              ///< 内存指针
    bool is_open_;                  ///< 是否打开
    bool is_creator_;               ///< 是否创建者
    
#ifdef _WIN32
    HANDLE handle_;                 ///< Windows句柄
#else
    int fd_;                        ///< 文件描述符
#endif
    
    mutable SharedMemoryStatistics statistics_; ///< 统计信息
    mutable std::mutex mutex_;      ///< 互斥锁
};

/**
 * @brief 消息队列类
 * 
 * 提供POSIX消息队列的封装，支持进程间消息传递。
 */
class MessageQueue {
public:
    /**
     * @brief 构造函数
     */
    MessageQueue();
    
    /**
     * @brief 析构函数
     */
    ~MessageQueue();
    
    /**
     * @brief 创建消息队列
     * @param config 配置
     * @return 操作结果
     */
    IPCResult<bool> create(const MessageQueueConfig& config);
    
    /**
     * @brief 打开已存在的消息队列
     * @param name 名称
     * @return 操作结果
     */
    IPCResult<bool> open(const std::string& name);
    
    /**
     * @brief 关闭消息队列
     */
    void close();
    
    /**
     * @brief 发送消息
     * @param message 消息
     * @param timeout_ms 超时时间（毫秒），0表示无限等待
     * @return 操作结果
     */
    IPCResult<bool> send(const Message& message, uint32_t timeout_ms = 0);
    
    /**
     * @brief 接收消息
     * @param message 输出消息
     * @param timeout_ms 超时时间（毫秒），0表示无限等待
     * @return 操作结果
     */
    IPCResult<bool> receive(Message& message, uint32_t timeout_ms = 0);
    
    /**
     * @brief 发送高优先级消息
     * @param message 消息
     * @return 操作结果
     */
    IPCResult<bool> sendPriority(const Message& message);
    
    /**
     * @brief 检查队列是否为空
     * @return true 如果为空
     */
    bool isEmpty() const;
    
    /**
     * @brief 检查队列是否已满
     * @return true 如果已满
     */
    bool isFull() const;
    
    /**
     * @brief 获取当前队列大小
     * @return 队列大小
     */
    size_t getCurrentSize() const;
    
    /**
     * @brief 获取最大队列大小
     * @return 最大队列大小
     */
    size_t getMaxSize() const;
    
    /**
     * @brief 获取名称
     * @return 名称
     */
    std::string getName() const;
    
    /**
     * @brief 检查是否已打开
     * @return true 如果已打开
     */
    bool isOpen() const;
    
    /**
     * @brief 获取统计信息
     * @return 统计信息
     */
    MessageQueueStatistics getStatistics() const;
    
    /**
     * @brief 重置统计信息
     */
    void resetStatistics();
    
    /**
     * @brief 清空队列
     */
    void clear();
    
    /**
     * @brief 删除消息队列
     * @param name 名称
     * @return 操作结果
     */
    static IPCResult<bool> unlink(const std::string& name);

private:
    std::string name_;              ///< 名称
    size_t max_messages_;           ///< 最大消息数
    size_t max_message_size_;       ///< 最大消息大小
    bool is_open_;                  ///< 是否打开
    bool is_creator_;               ///< 是否创建者
    
#ifdef _WIN32
    HANDLE handle_;                 ///< Windows句柄
#else
    mqd_t descriptor_;              ///< POSIX消息队列描述符
#endif
    
    mutable MessageQueueStatistics statistics_; ///< 统计信息
    mutable std::mutex mutex_;      ///< 互斥锁
};

/**
 * @brief IPC管理器类
 * 
 * 统一管理共享内存和消息队列，提供便捷的IPC接口。
 */
class IPCManager {
public:
    /**
     * @brief 获取单例实例
     * @return 管理器实例引用
     */
    static IPCManager& getInstance();
    
    /**
     * @brief 初始化管理器
     * @return true 如果成功
     */
    bool initialize();
    
    /**
     * @brief 关闭管理器
     */
    void shutdown();
    
    /**
     * @brief 创建共享内存
     * @param name 名称
     * @param size 大小
     * @return 共享内存指针
     */
    std::shared_ptr<SharedMemory> createSharedMemory(
        const std::string& name, size_t size);
    
    /**
     * @brief 获取共享内存
     * @param name 名称
     * @return 共享内存指针
     */
    std::shared_ptr<SharedMemory> getSharedMemory(const std::string& name) const;
    
    /**
     * @brief 删除共享内存
     * @param name 名称
     * @return true 如果成功
     */
    bool deleteSharedMemory(const std::string& name);
    
    /**
     * @brief 创建消息队列
     * @param name 名称
     * @param max_messages 最大消息数
     * @return 消息队列指针
     */
    std::shared_ptr<MessageQueue> createMessageQueue(
        const std::string& name, size_t max_messages = 100);
    
    /**
     * @brief 获取消息队列
     * @param name 名称
     * @return 消息队列指针
     */
    std::shared_ptr<MessageQueue> getMessageQueue(const std::string& name) const;
    
    /**
     * @brief 删除消息队列
     * @param name 名称
     * @return true 如果成功
     */
    bool deleteMessageQueue(const std::string& name);
    
    /**
     * @brief 获取所有共享内存名称
     * @return 名称列表
     */
    std::vector<std::string> getAllSharedMemoryNames() const;
    
    /**
     * @brief 获取所有消息队列名称
     * @return 名称列表
     */
    std::vector<std::string> getAllMessageQueueNames() const;
    
    /**
     * @brief 清理所有IPC资源
     */
    void cleanup();
    
    /**
     * @brief 获取性能统计
     * @return 统计信息JSON字符串
     */
    std::string getPerformanceStats() const;
    
private:
    // 私有构造函数（单例模式）
    IPCManager();
    ~IPCManager();
    
    // 禁用拷贝和赋值
    IPCManager(const IPCManager&) = delete;
    IPCManager& operator=(const IPCManager&) = delete;
    
    std::map<std::string, std::shared_ptr<SharedMemory>> shared_memories_; ///< 共享内存映射
    std::map<std::string, std::shared_ptr<MessageQueue>> message_queues_;  ///< 消息队列映射
    
    mutable std::mutex mutex_;      ///< 互斥锁
    bool is_initialized_;           ///< 初始化标志
};

/**
 * @brief IPC资源构建器（Builder模式）
 * 
 * 提供流畅的API来创建IPC资源。
 */
class IPCBuilder {
public:
    /**
     * @brief 构造函数
     * @param manager IPC管理器引用
     */
    explicit IPCBuilder(IPCManager& manager)
        : manager_(manager)
        , resource_type_(ResourceType::SHARED_MEMORY)
        , size_(4096)
        , max_messages_(100) {}
    
    /**
     * @brief 设置资源类型为共享内存
     * @return 构建器引用
     */
    IPCBuilder& asSharedMemory() {
        resource_type_ = ResourceType::SHARED_MEMORY;
        return *this;
    }
    
    /**
     * @brief 设置资源类型为消息队列
     * @return 构建器引用
     */
    IPCBuilder& asMessageQueue() {
        resource_type_ = ResourceType::MESSAGE_QUEUE;
        return *this;
    }
    
    /**
     * @brief 设置名称
     * @param name 资源名称
     * @return 构建器引用
     */
    IPCBuilder& withName(const std::string& name) {
        name_ = name;
        return *this;
    }
    
    /**
     * @brief 设置大小（用于共享内存）
     * @param size 大小（字节）
     * @return 构建器引用
     */
    IPCBuilder& withSize(size_t size) {
        size_ = size;
        return *this;
    }
    
    /**
     * @brief 设置最大消息数（用于消息队列）
     * @param max_messages 最大消息数
     * @return 构建器引用
     */
    IPCBuilder& withMaxMessages(size_t max_messages) {
        max_messages_ = max_messages;
        return *this;
    }
    
    /**
     * @brief 构建共享内存
     * @return 共享内存指针
     */
    std::shared_ptr<SharedMemory> buildSharedMemory() {
        if (name_.empty()) {
            return nullptr;
        }
        return manager_.createSharedMemory(name_, size_);
    }
    
    /**
     * @brief 构建消息队列
     * @return 消息队列指针
     */
    std::shared_ptr<MessageQueue> buildMessageQueue() {
        if (name_.empty()) {
            return nullptr;
        }
        return manager_.createMessageQueue(name_, max_messages_);
    }
    
private:
    enum class ResourceType {
        SHARED_MEMORY,
        MESSAGE_QUEUE
    };
    
    IPCManager& manager_;
    ResourceType resource_type_;
    std::string name_;
    size_t size_;
    size_t max_messages_;
};

/**
 * @brief IPC监控器
 * 
 * 提供IPC资源监控和诊断功能。
 */
class IPCMonitor {
public:
    /**
     * @brief 构造函数
     * @param manager IPC管理器引用
     */
    explicit IPCMonitor(IPCManager& manager)
        : manager_(manager) {}
    
    /**
     * @brief 生成性能报告
     * @return 报告字符串
     */
    std::string generateReport() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        
        oss << "========== IPC Performance Report ==========\n\n";
        
        // 共享内存统计
        auto shm_names = manager_.getAllSharedMemoryNames();
        oss << "[Shared Memory] Count: " << shm_names.size() << "\n";
        for (const auto& name : shm_names) {
            auto shm = manager_.getSharedMemory(name);
            if (shm) {
                auto stats = shm->getStatistics();
                oss << "  " << name << ":\n";
                oss << "    Size: " << stats.total_size << " bytes\n";
                oss << "    Read Count: " << stats.read_count << "\n";
                oss << "    Write Count: " << stats.write_count << "\n";
                oss << "    Utilization: " << (stats.utilizationRate() * 100.0) << "%\n\n";
            }
        }
        
        // 消息队列统计
        auto mq_names = manager_.getAllMessageQueueNames();
        oss << "[Message Queues] Count: " << mq_names.size() << "\n";
        for (const auto& name : mq_names) {
            auto mq = manager_.getMessageQueue(name);
            if (mq) {
                auto stats = mq->getStatistics();
                oss << "  " << name << ":\n";
                oss << "    Messages Sent: " << stats.messages_sent << "\n";
                oss << "    Messages Received: " << stats.messages_received << "\n";
                oss << "    Send Errors: " << stats.send_errors << "\n";
                oss << "    Receive Errors: " << stats.receive_errors << "\n";
                oss << "    Queue Full Count: " << stats.queue_full_count << "\n\n";
            }
        }
        
        oss << "=============================================\n";
        
        return oss.str();
    }
    
    /**
     * @brief 检查系统健康状态
     * @return 健康状态（0-100）
     */
    int checkHealth() const {
        int health_score = 100;
        
        // 检查共享内存错误率
        auto shm_names = manager_.getAllSharedMemoryNames();
        for (const auto& name : shm_names) {
            auto shm = manager_.getSharedMemory(name);
            if (shm) {
                auto stats = shm->getStatistics();
                if (stats.access_errors > 0) {
                    health_score -= 10;
                }
            }
        }
        
        // 检查消息队列错误率
        auto mq_names = manager_.getAllMessageQueueNames();
        for (const auto& name : mq_names) {
            auto mq = manager_.getMessageQueue(name);
            if (mq) {
                auto stats = mq->getStatistics();
                uint64_t total_ops = stats.messages_sent + stats.messages_received;
                if (total_ops > 0) {
                    uint64_t errors = stats.send_errors + stats.receive_errors;
                    double error_rate = static_cast<double>(errors) / total_ops;
                    if (error_rate > 0.05) {  // 超过5%错误率
                        health_score -= 15;
                    }
                }
            }
        }
        
        return std::max(0, std::min(100, health_score));
    }
    
    /**
     * @brief 导出统计数据为JSON
     * @return JSON字符串
     */
    std::string exportToJson() const {
        return manager_.getPerformanceStats();
    }
    
    /**
     * @brief 检测资源泄漏
     * @return 可能泄漏的资源列表
     */
    std::vector<std::string> detectLeaks() const {
        std::vector<std::string> leaked_resources;
        
        // 检查长时间未使用的共享内存
        auto shm_names = manager_.getAllSharedMemoryNames();
        for (const auto& name : shm_names) {
            auto shm = manager_.getSharedMemory(name);
            if (shm) {
                auto stats = shm->getStatistics();
                // 如果创建了但从未读写，可能是泄漏
                if (stats.read_count == 0 && stats.write_count == 0) {
                    auto now = std::chrono::steady_clock::now();
                    auto age = std::chrono::duration_cast<std::chrono::seconds>(
                        now - stats.creation_time).count();
                    if (age > 300) {  // 超过5分钟未使用
                        leaked_resources.push_back("SHM:" + name);
                    }
                }
            }
        }
        
        return leaked_resources;
    }
    
private:
    IPCManager& manager_;
};

/**
 * @brief IPC实用工具函数
 */
namespace ipc_utils {
    /**
     * @brief 生成唯一资源名称
     * @param prefix 前缀
     * @return 唯一名称
     */
    inline std::string generateUniqueName(const std::string& prefix) {
        static std::atomic<uint64_t> counter{0};
        auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        std::ostringstream oss;
        oss << prefix << "_" << timestamp << "_" << counter.fetch_add(1);
        return oss.str();
    }
    
    /**
     * @brief 计算数据校验和
     * @param data 数据指针
     * @param size 数据大小
     * @return 校验和
     */
    inline uint32_t calculateChecksum(const void* data, size_t size) {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        uint32_t checksum = 0;
        
        for (size_t i = 0; i < size; i++) {
            checksum = ((checksum << 5) + checksum) + bytes[i];
        }
        
        return checksum;
    }
    
    /**
     * @brief 格式化字节大小
     * @param bytes 字节数
     * @return 格式化的字符串
     */
    inline std::string formatBytes(size_t bytes) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        
        if (bytes < 1024) {
            oss << bytes << " B";
        } else if (bytes < 1024 * 1024) {
            oss << (bytes / 1024.0) << " KB";
        } else if (bytes < 1024 * 1024 * 1024) {
            oss << (bytes / (1024.0 * 1024.0)) << " MB";
        } else {
            oss << (bytes / (1024.0 * 1024.0 * 1024.0)) << " GB";
        }
        
        return oss.str();
    }
    
    /**
     * @brief 验证消息完整性
     * @param message 消息
     * @return true 如果消息完整
     */
    inline bool validateMessage(const Message& message) {
        // 检查数据大小
        if (message.data_size > Message::MAX_DATA_SIZE) {
            return false;
        }
        
        // 检查时间戳
        if (message.timestamp == 0) {
            return false;
        }
        
        return true;
    }
    
    /**
     * @brief 序列化消息到字节流
     * @param message 消息
     * @return 字节流
     */
    inline std::vector<uint8_t> serializeMessage(const Message& message) {
        std::vector<uint8_t> buffer;
        buffer.resize(sizeof(Message));
        std::memcpy(buffer.data(), &message, sizeof(Message));
        return buffer;
    }
    
    /**
     * @brief 从字节流反序列化消息
     * @param buffer 字节流
     * @return 消息
     */
    inline Message deserializeMessage(const std::vector<uint8_t>& buffer) {
        Message message;
        if (buffer.size() >= sizeof(Message)) {
            std::memcpy(&message, buffer.data(), sizeof(Message));
        }
        return message;
    }
    
    /**
     * @brief 计算传输速率
     * @param bytes 字节数
     * @param duration_ms 持续时间（毫秒）
     * @return 速率（字节/秒）
     */
    inline double calculateTransferRate(uint64_t bytes, uint64_t duration_ms) {
        if (duration_ms == 0) {
            return 0.0;
        }
        return (bytes * 1000.0) / duration_ms;
    }
} // namespace ipc_utils

} // namespace scheduler
} // namespace ev_dynamics

#endif // IPC_MANAGER_H
