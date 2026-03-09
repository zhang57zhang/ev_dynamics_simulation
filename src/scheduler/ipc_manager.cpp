/**
 * @file ipc_manager.cpp
 * @brief IPC管理器实现
 * @author EV Dynamics Simulation Team
 * @date 2026-03-08
 * @version 1.0
 */

#include "ipc_manager.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace ev_dynamics {
namespace scheduler {

// ============================================================================
// SharedMemory 实现
// ============================================================================

SharedMemory::SharedMemory()
    : size_(0)
    , memory_ptr_(nullptr)
    , is_open_(false)
    , is_creator_(false)
#ifdef _WIN32
    , handle_(nullptr)
#else
    , fd_(-1)
#endif
{
}

SharedMemory::~SharedMemory() {
    close();
}

IPCResult<bool> SharedMemory::create(const SharedMemoryConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (is_open_) {
        return IPCResult<bool>::error(IPCErrorCode::ALREADY_EXISTS, 
                                       "Shared memory already open");
    }
    
    if (config.name.empty() || config.size == 0) {
        return IPCResult<bool>::error(IPCErrorCode::INVALID_PARAMETER,
                                       "Invalid name or size");
    }
    
#ifdef _WIN32
    // Windows实现
    handle_ = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        nullptr,
        config.read_only ? PAGE_READONLY : PAGE_READWRITE,
        0,
        static_cast<DWORD>(config.size),
        config.name.c_str()
    );
    
    if (handle_ == nullptr) {
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to create file mapping");
    }
    
    memory_ptr_ = MapViewOfFile(
        handle_,
        config.read_only ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS,
        0, 0, config.size
    );
    
    if (memory_ptr_ == nullptr) {
        CloseHandle(handle_);
        handle_ = nullptr;
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to map view of file");
    }
    
#else
    // POSIX实现
    std::string shm_name = "/" + config.name;
    
    // 创建共享内存
    fd_ = shm_open(shm_name.c_str(), 
                   O_CREAT | O_RDWR | (config.create_new ? O_EXCL : 0),
                   S_IRUSR | S_IWUSR);
    
    if (fd_ == -1) {
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to create shared memory");
    }
    
    // 设置大小
    if (ftruncate(fd_, config.size) == -1) {
        ::close(fd_);
        shm_unlink(shm_name.c_str());
        fd_ = -1;
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to set size");
    }
    
    // 映射内存
    int prot = config.read_only ? PROT_READ : (PROT_READ | PROT_WRITE);
    memory_ptr_ = mmap(nullptr, config.size, prot, MAP_SHARED, fd_, 0);
    
    if (memory_ptr_ == MAP_FAILED) {
        ::close(fd_);
        shm_unlink(shm_name.c_str());
        fd_ = -1;
        memory_ptr_ = nullptr;
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to map memory");
    }
#endif
    
    name_ = config.name;
    size_ = config.size;
    is_open_ = true;
    is_creator_ = true;
    
    statistics_.total_size = config.size;
    statistics_.used_size = 0;
    statistics_.creation_time = std::chrono::steady_clock::now();
    
    // 清零内存
    if (!config.read_only) {
        memset(memory_ptr_, 0, size_);
    }
    
    return IPCResult<bool>::success(true);
}

IPCResult<bool> SharedMemory::open(const std::string& name, bool read_only) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (is_open_) {
        return IPCResult<bool>::error(IPCErrorCode::ALREADY_EXISTS,
                                       "Shared memory already open");
    }
    
    if (name.empty()) {
        return IPCResult<bool>::error(IPCErrorCode::INVALID_PARAMETER,
                                       "Invalid name");
    }
    
#ifdef _WIN32
    // Windows实现
    handle_ = OpenFileMappingA(
        read_only ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS,
        FALSE,
        name.c_str()
    );
    
    if (handle_ == nullptr) {
        return IPCResult<bool>::error(IPCErrorCode::NOT_FOUND,
                                       "Shared memory not found");
    }
    
    // 获取大小
    MEMORY_BASIC_INFORMATION info;
    if (VirtualQueryEx(GetCurrentProcess(), memory_ptr_, &info, sizeof(info)) == 0) {
        CloseHandle(handle_);
        handle_ = nullptr;
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to query memory info");
    }
    
    size_ = info.RegionSize;
    
    memory_ptr_ = MapViewOfFile(
        handle_,
        read_only ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS,
        0, 0, 0
    );
    
    if (memory_ptr_ == nullptr) {
        CloseHandle(handle_);
        handle_ = nullptr;
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to map view of file");
    }
    
#else
    // POSIX实现
    std::string shm_name = "/" + name;
    
    fd_ = shm_open(shm_name.c_str(), read_only ? O_RDONLY : O_RDWR, 0);
    
    if (fd_ == -1) {
        return IPCResult<bool>::error(IPCErrorCode::NOT_FOUND,
                                       "Shared memory not found");
    }
    
    // 获取大小
    struct stat sb;
    if (fstat(fd_, &sb) == -1) {
        ::close(fd_);
        fd_ = -1;
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to get size");
    }
    
    size_ = sb.st_size;
    
    // 映射内存
    int prot = read_only ? PROT_READ : (PROT_READ | PROT_WRITE);
    memory_ptr_ = mmap(nullptr, size_, prot, MAP_SHARED, fd_, 0);
    
    if (memory_ptr_ == MAP_FAILED) {
        ::close(fd_);
        fd_ = -1;
        memory_ptr_ = nullptr;
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to map memory");
    }
#endif
    
    name_ = name;
    is_open_ = true;
    is_creator_ = false;
    
    statistics_.total_size = size_;
    statistics_.creation_time = std::chrono::steady_clock::now();
    
    return IPCResult<bool>::success(true);
}

void SharedMemory::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_open_) {
        return;
    }
    
#ifdef _WIN32
    if (memory_ptr_) {
        UnmapViewOfFile(memory_ptr_);
        memory_ptr_ = nullptr;
    }
    
    if (handle_) {
        CloseHandle(handle_);
        handle_ = nullptr;
    }
#else
    if (memory_ptr_ && memory_ptr_ != MAP_FAILED) {
        munmap(memory_ptr_, size_);
        memory_ptr_ = nullptr;
    }
    
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
#endif
    
    is_open_ = false;
    is_creator_ = false;
    size_ = 0;
    name_.clear();
}

IPCResult<size_t> SharedMemory::write(size_t offset, const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_open_) {
        return IPCResult<size_t>::error(IPCErrorCode::NOT_FOUND, "Not open");
    }
    
    if (!memory_ptr_) {
        return IPCResult<size_t>::error(IPCErrorCode::INTERNAL_ERROR, "Invalid pointer");
    }
    
    if (offset >= size_) {
        return IPCResult<size_t>::error(IPCErrorCode::INVALID_PARAMETER, "Invalid offset");
    }
    
    size_t write_size = std::min(size, size_ - offset);
    
    memcpy(static_cast<uint8_t*>(memory_ptr_) + offset, data, write_size);
    
    // 更新统计
    statistics_.write_count++;
    statistics_.write_bytes += write_size;
    statistics_.used_size = std::max(statistics_.used_size, offset + write_size);
    
    return IPCResult<size_t>::success(write_size);
}

IPCResult<size_t> SharedMemory::read(size_t offset, void* buffer, size_t buffer_size) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_open_) {
        return IPCResult<size_t>::error(IPCErrorCode::NOT_FOUND, "Not open");
    }
    
    if (!memory_ptr_) {
        return IPCResult<size_t>::error(IPCErrorCode::INTERNAL_ERROR, "Invalid pointer");
    }
    
    if (offset >= size_) {
        return IPCResult<size_t>::error(IPCErrorCode::INVALID_PARAMETER, "Invalid offset");
    }
    
    size_t read_size = std::min(buffer_size, size_ - offset);
    
    memcpy(buffer, static_cast<const uint8_t*>(memory_ptr_) + offset, read_size);
    
    // 更新统计
    statistics_.read_count++;
    statistics_.read_bytes += read_size;
    
    return IPCResult<size_t>::success(read_size);
}

void* SharedMemory::getPointer() const {
    return memory_ptr_;
}

size_t SharedMemory::getSize() const {
    return size_;
}

std::string SharedMemory::getName() const {
    return name_;
}

bool SharedMemory::isOpen() const {
    return is_open_;
}

SharedMemoryStatistics SharedMemory::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return statistics_;
}

void SharedMemory::resetStatistics() {
    std::lock_guard<std::mutex> lock(mutex_);
    statistics_.reset();
}

void SharedMemory::zeroMemory() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (is_open_ && memory_ptr_) {
        memset(memory_ptr_, 0, size_);
        statistics_.used_size = 0;
    }
}

IPCResult<bool> SharedMemory::unlink(const std::string& name) {
    if (name.empty()) {
        return IPCResult<bool>::error(IPCErrorCode::INVALID_PARAMETER, "Invalid name");
    }
    
#ifdef _WIN32
    // Windows共享内存会在最后一个句柄关闭时自动清理
    return IPCResult<bool>::success(true);
#else
    std::string shm_name = "/" + name;
    
    if (shm_unlink(shm_name.c_str()) == -1) {
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to unlink");
    }
    
    return IPCResult<bool>::success(true);
#endif
}

// ============================================================================
// MessageQueue 实现
// ============================================================================

MessageQueue::MessageQueue()
    : max_messages_(0)
    , max_message_size_(0)
    , is_open_(false)
    , is_creator_(false)
#ifdef _WIN32
    , handle_(nullptr)
#else
    , descriptor_((mqd_t)-1)
#endif
{
}

MessageQueue::~MessageQueue() {
    close();
}

IPCResult<bool> MessageQueue::create(const MessageQueueConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (is_open_) {
        return IPCResult<bool>::error(IPCErrorCode::ALREADY_EXISTS,
                                       "Queue already open");
    }
    
    if (config.name.empty()) {
        return IPCResult<bool>::error(IPCErrorCode::INVALID_PARAMETER,
                                       "Invalid name");
    }
    
#ifdef _WIN32
    // Windows实现 - 使用邮槽（Mailslot）
    std::string mailslot_name = "\\\\.\\mailslot\\" + config.name;
    
    handle_ = CreateMailslotA(
        mailslot_name.c_str(),
        static_cast<DWORD>(config.max_message_size),
        MAILSLOT_WAIT_FOREVER,
        nullptr
    );
    
    if (handle_ == INVALID_HANDLE_VALUE) {
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to create mailslot");
    }
#else
    // POSIX实现
    std::string mq_name = "/" + config.name;
    
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = config.max_messages;
    attr.mq_msgsize = config.max_message_size;
    attr.mq_curmsgs = 0;
    
    descriptor_ = mq_open(mq_name.c_str(), 
                          O_CREAT | O_RDWR | (config.create_new ? O_EXCL : 0),
                          S_IRUSR | S_IWUSR,
                          &attr);
    
    if (descriptor_ == (mqd_t)-1) {
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to create message queue");
    }
#endif
    
    name_ = config.name;
    max_messages_ = config.max_messages;
    max_message_size_ = config.max_message_size;
    is_open_ = true;
    is_creator_ = true;
    
    statistics_.max_queue_size = config.max_messages;
    statistics_.creation_time = std::chrono::steady_clock::now();
    
    return IPCResult<bool>::success(true);
}

IPCResult<bool> MessageQueue::open(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (is_open_) {
        return IPCResult<bool>::error(IPCErrorCode::ALREADY_EXISTS,
                                       "Queue already open");
    }
    
    if (name.empty()) {
        return IPCResult<bool>::error(IPCErrorCode::INVALID_PARAMETER,
                                       "Invalid name");
    }
    
#ifdef _WIN32
    // Windows实现
    std::string mailslot_name = "\\\\.\\mailslot\\" + name;
    
    handle_ = CreateFileA(
        mailslot_name.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (handle_ == INVALID_HANDLE_VALUE) {
        return IPCResult<bool>::error(IPCErrorCode::NOT_FOUND,
                                       "Message queue not found");
    }
#else
    // POSIX实现
    std::string mq_name = "/" + name;
    
    descriptor_ = mq_open(mq_name.c_str(), O_RDWR);
    
    if (descriptor_ == (mqd_t)-1) {
        return IPCResult<bool>::error(IPCErrorCode::NOT_FOUND,
                                       "Message queue not found");
    }
    
    // 获取属性
    struct mq_attr attr;
    if (mq_getattr(descriptor_, &attr) == -1) {
        mq_close(descriptor_);
        descriptor_ = (mqd_t)-1;
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to get attributes");
    }
    
    max_messages_ = attr.mq_maxmsg;
    max_message_size_ = attr.mq_msgsize;
#endif
    
    name_ = name;
    is_open_ = true;
    is_creator_ = false;
    
    statistics_.max_queue_size = max_messages_;
    statistics_.creation_time = std::chrono::steady_clock::now();
    
    return IPCResult<bool>::success(true);
}

void MessageQueue::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_open_) {
        return;
    }
    
#ifdef _WIN32
    if (handle_ && handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(handle_);
        handle_ = nullptr;
    }
#else
    if (descriptor_ != (mqd_t)-1) {
        mq_close(descriptor_);
        descriptor_ = (mqd_t)-1;
    }
#endif
    
    is_open_ = false;
    is_creator_ = false;
    name_.clear();
}

IPCResult<bool> MessageQueue::send(const Message& message, uint32_t timeout_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_open_) {
        return IPCResult<bool>::error(IPCErrorCode::NOT_FOUND, "Not open");
    }
    
    size_t msg_size = message.totalSize();
    
#ifdef _WIN32
    // Windows实现
    DWORD bytes_written = 0;
    BOOL result = WriteFile(
        handle_,
        &message,
        static_cast<DWORD>(msg_size),
        &bytes_written,
        nullptr
    );
    
    if (!result) {
        statistics_.send_errors++;
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to send message");
    }
#else
    // POSIX实现
    unsigned int priority = message.priority;
    
    if (timeout_ms > 0) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;
        ts.tv_sec += timeout_ms / 1000 + ts.tv_nsec / 1000000000;
        ts.tv_nsec %= 1000000000;
        
        if (mq_timedsend(descriptor_, 
                         reinterpret_cast<const char*>(&message),
                         msg_size, priority, &ts) == -1) {
            statistics_.send_errors++;
            if (errno == ETIMEDOUT) {
                return IPCResult<bool>::error(IPCErrorCode::TIMEOUT, "Send timeout");
            }
            return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                           "Failed to send message");
        }
    } else {
        if (mq_send(descriptor_, 
                    reinterpret_cast<const char*>(&message),
                    msg_size, priority) == -1) {
            statistics_.send_errors++;
            return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                           "Failed to send message");
        }
    }
#endif
    
    // 更新统计
    statistics_.messages_sent++;
    statistics_.bytes_sent += msg_size;
    statistics_.current_queue_size++;
    
    if (statistics_.current_queue_size >= max_messages_) {
        statistics_.queue_full_count++;
    }
    
    return IPCResult<bool>::success(true);
}

IPCResult<bool> MessageQueue::receive(Message& message, uint32_t timeout_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_open_) {
        return IPCResult<bool>::error(IPCErrorCode::NOT_FOUND, "Not open");
    }
    
#ifdef _WIN32
    // Windows实现
    DWORD bytes_read = 0;
    DWORD timeout = (timeout_ms == 0) ? INFINITE : timeout_ms;
    
    BOOL result = ReadFile(
        handle_,
        &message,
        sizeof(Message),
        &bytes_read,
        nullptr
    );
    
    if (!result) {
        statistics_.receive_errors++;
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to receive message");
    }
#else
    // POSIX实现
    unsigned int priority;
    ssize_t bytes_read;
    
    if (timeout_ms > 0) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;
        ts.tv_sec += timeout_ms / 1000 + ts.tv_nsec / 1000000000;
        ts.tv_nsec %= 1000000000;
        
        bytes_read = mq_timedreceive(descriptor_,
                                     reinterpret_cast<char*>(&message),
                                     sizeof(Message), &priority, &ts);
        
        if (bytes_read == -1) {
            statistics_.receive_errors++;
            if (errno == ETIMEDOUT) {
                return IPCResult<bool>::error(IPCErrorCode::TIMEOUT, "Receive timeout");
            }
            return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                           "Failed to receive message");
        }
    } else {
        bytes_read = mq_receive(descriptor_,
                                reinterpret_cast<char*>(&message),
                                sizeof(Message), &priority);
        
        if (bytes_read == -1) {
            statistics_.receive_errors++;
            return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                           "Failed to receive message");
        }
    }
    
    message.priority = static_cast<uint8_t>(priority);
#endif
    
    // 更新统计
    statistics_.messages_received++;
    statistics_.bytes_received += bytes_read;
    statistics_.current_queue_size--;
    
    if (statistics_.current_queue_size == 0) {
        statistics_.queue_empty_count++;
    }
    
    return IPCResult<bool>::success(true);
}

IPCResult<bool> MessageQueue::sendPriority(const Message& message) {
    Message priority_message = message;
    priority_message.priority = 255;  // 最高优先级
    return send(priority_message, 0);
}

bool MessageQueue::isEmpty() const {
    return getCurrentSize() == 0;
}

bool MessageQueue::isFull() const {
    return getCurrentSize() >= max_messages_;
}

size_t MessageQueue::getCurrentSize() const {
#ifdef _WIN32
    // Windows实现 - 无法直接获取大小
    return statistics_.current_queue_size;
#else
    if (!is_open_ || descriptor_ == (mqd_t)-1) {
        return 0;
    }
    
    struct mq_attr attr;
    if (mq_getattr(descriptor_, &attr) == -1) {
        return statistics_.current_queue_size;
    }
    
    return attr.mq_curmsgs;
#endif
}

size_t MessageQueue::getMaxSize() const {
    return max_messages_;
}

std::string MessageQueue::getName() const {
    return name_;
}

bool MessageQueue::isOpen() const {
    return is_open_;
}

MessageQueueStatistics MessageQueue::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return statistics_;
}

void MessageQueue::resetStatistics() {
    std::lock_guard<std::mutex> lock(mutex_);
    statistics_.reset();
}

void MessageQueue::clear() {
#ifdef _WIN32
    // Windows实现 - 读取并丢弃所有消息
    Message msg;
    while (receive(msg, 1).isSuccess()) {
        // 丢弃消息
    }
#else
    // POSIX实现
    if (is_open_ && descriptor_ != (mqd_t)-1) {
        struct mq_attr attr;
        mq_getattr(descriptor_, &attr);
        
        Message msg;
        while (attr.mq_curmsgs > 0) {
            if (mq_receive(descriptor_, reinterpret_cast<char*>(&msg),
                          sizeof(Message), nullptr) == -1) {
                break;
            }
            mq_getattr(descriptor_, &attr);
        }
    }
#endif
    
    statistics_.current_queue_size = 0;
}

IPCResult<bool> MessageQueue::unlink(const std::string& name) {
    if (name.empty()) {
        return IPCResult<bool>::error(IPCErrorCode::INVALID_PARAMETER, "Invalid name");
    }
    
#ifdef _WIN32
    // Windows邮槽会在最后一个句柄关闭时自动清理
    return IPCResult<bool>::success(true);
#else
    std::string mq_name = "/" + name;
    
    if (mq_unlink(mq_name.c_str()) == -1) {
        return IPCResult<bool>::error(IPCErrorCode::INTERNAL_ERROR,
                                       "Failed to unlink");
    }
    
    return IPCResult<bool>::success(true);
#endif
}

// ============================================================================
// IPCManager 实现
// ============================================================================

IPCManager& IPCManager::getInstance() {
    static IPCManager instance;
    return instance;
}

IPCManager::IPCManager()
    : is_initialized_(false) {
}

IPCManager::~IPCManager() {
    shutdown();
}

bool IPCManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (is_initialized_) {
        return true;
    }
    
    is_initialized_ = true;
    return true;
}

void IPCManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_initialized_) {
        return;
    }
    
    cleanup();
    is_initialized_ = false;
}

std::shared_ptr<SharedMemory> IPCManager::createSharedMemory(
    const std::string& name, size_t size) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (shared_memories_.find(name) != shared_memories_.end()) {
        return shared_memories_[name];
    }
    
    auto shm = std::make_shared<SharedMemory>();
    SharedMemoryConfig config(name, size);
    
    auto result = shm->create(config);
    if (result.isSuccess()) {
        shared_memories_[name] = shm;
        return shm;
    }
    
    return nullptr;
}

std::shared_ptr<SharedMemory> IPCManager::getSharedMemory(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = shared_memories_.find(name);
    if (it != shared_memories_.end()) {
        return it->second;
    }
    
    return nullptr;
}

bool IPCManager::deleteSharedMemory(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = shared_memories_.find(name);
    if (it == shared_memories_.end()) {
        return false;
    }
    
    it->second->close();
    SharedMemory::unlink(name);
    shared_memories_.erase(it);
    
    return true;
}

std::shared_ptr<MessageQueue> IPCManager::createMessageQueue(
    const std::string& name, size_t max_messages) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (message_queues_.find(name) != message_queues_.end()) {
        return message_queues_[name];
    }
    
    auto mq = std::make_shared<MessageQueue>();
    MessageQueueConfig config(name);
    config.max_messages = max_messages;
    
    auto result = mq->create(config);
    if (result.isSuccess()) {
        message_queues_[name] = mq;
        return mq;
    }
    
    return nullptr;
}

std::shared_ptr<MessageQueue> IPCManager::getMessageQueue(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = message_queues_.find(name);
    if (it != message_queues_.end()) {
        return it->second;
    }
    
    return nullptr;
}

bool IPCManager::deleteMessageQueue(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = message_queues_.find(name);
    if (it == message_queues_.end()) {
        return false;
    }
    
    it->second->close();
    MessageQueue::unlink(name);
    message_queues_.erase(it);
    
    return true;
}

std::vector<std::string> IPCManager::getAllSharedMemoryNames() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> names;
    names.reserve(shared_memories_.size());
    
    for (const auto& pair : shared_memories_) {
        names.push_back(pair.first);
    }
    
    return names;
}

std::vector<std::string> IPCManager::getAllMessageQueueNames() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> names;
    names.reserve(message_queues_.size());
    
    for (const auto& pair : message_queues_) {
        names.push_back(pair.first);
    }
    
    return names;
}

void IPCManager::cleanup() {
    // 清理所有共享内存
    for (auto& pair : shared_memories_) {
        pair.second->close();
    }
    shared_memories_.clear();
    
    // 清理所有消息队列
    for (auto& pair : message_queues_) {
        pair.second->close();
    }
    message_queues_.clear();
}

std::string IPCManager::getPerformanceStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << "{\n";
    oss << "  \"shared_memories\": {\n";
    oss << "    \"count\": " << shared_memories_.size() << ",\n";
    
    uint64_t total_shm_read = 0, total_shm_write = 0;
    for (const auto& pair : shared_memories_) {
        auto stats = pair.second->getStatistics();
        total_shm_read += stats.read_bytes;
        total_shm_write += stats.write_bytes;
    }
    
    oss << "    \"total_read_bytes\": " << total_shm_read << ",\n";
    oss << "    \"total_write_bytes\": " << total_shm_write << "\n";
    oss << "  },\n";
    
    oss << "  \"message_queues\": {\n";
    oss << "    \"count\": " << message_queues_.size() << ",\n";
    
    uint64_t total_mq_sent = 0, total_mq_received = 0;
    for (const auto& pair : message_queues_) {
        auto stats = pair.second->getStatistics();
        total_mq_sent += stats.messages_sent;
        total_mq_received += stats.messages_received;
    }
    
    oss << "    \"total_messages_sent\": " << total_mq_sent << ",\n";
    oss << "    \"total_messages_received\": " << total_mq_received << "\n";
    oss << "  }\n";
    oss << "}";
    
    return oss.str();
}

// ============================================================================
// 额外的IPC实用函数实现
// ============================================================================

/**
 * @brief 创建消息的工厂函数
 * @param type 消息类型
 * @param sender 发送者ID
 * @param receiver 接收者ID
 * @param data 数据指针
 * @param data_size 数据大小
 * @return 创建的消息
 */
Message createMessage(uint32_t type, uint32_t sender, uint32_t receiver,
                     const void* data, size_t data_size) {
    Message msg(type, sender, receiver);
    msg.timestamp = RealtimeScheduler::getCurrentTimeUs();
    
    if (data && data_size > 0) {
        msg.setData(data, data_size);
    }
    
    return msg;
}

/**
 * @brief 批量发送消息
 * @param mq 消息队列
 * @param messages 消息列表
 * @param successful_count 成功发送的数量（输出参数）
 * @return true 如果所有消息都发送成功
 */
bool sendBatchMessages(std::shared_ptr<MessageQueue> mq,
                      const std::vector<Message>& messages,
                      size_t& successful_count) {
    if (!mq || !mq->isOpen()) {
        successful_count = 0;
        return false;
    }
    
    successful_count = 0;
    bool all_success = true;
    
    for (const auto& msg : messages) {
        auto result = mq->send(msg);
        if (result.isSuccess()) {
            successful_count++;
        } else {
            all_success = false;
        }
    }
    
    return all_success;
}

/**
 * @brief 批量接收消息
 * @param mq 消息队列
 * @param max_count 最大接收数量
 * @param messages 接收到的消息列表（输出参数）
 * @return 实际接收的数量
 */
size_t receiveBatchMessages(std::shared_ptr<MessageQueue> mq,
                           size_t max_count,
                           std::vector<Message>& messages) {
    if (!mq || !mq->isOpen()) {
        return 0;
    }
    
    messages.clear();
    messages.reserve(max_count);
    
    for (size_t i = 0; i < max_count; i++) {
        Message msg;
        auto result = mq->receive(msg, 10);  // 10ms timeout
        if (result.isSuccess()) {
            messages.push_back(msg);
        } else {
            break;  // 队列为空或超时
        }
    }
    
    return messages.size();
}

/**
 * @brief 复制共享内存数据
 * @param src 源共享内存
 * @param dst 目标共享内存
 * @param size 要复制的字节数
 * @return true 如果复制成功
 */
bool copySharedMemory(std::shared_ptr<SharedMemory> src,
                     std::shared_ptr<SharedMemory> dst,
                     size_t size) {
    if (!src || !dst || !src->isOpen() || !dst->isOpen()) {
        return false;
    }
    
    if (src->getSize() < size || dst->getSize() < size) {
        return false;
    }
    
    std::vector<uint8_t> buffer(size);
    auto read_result = src->read(0, buffer.data(), size);
    if (!read_result.isSuccess()) {
        return false;
    }
    
    auto write_result = dst->write(0, buffer.data(), size);
    return write_result.isSuccess();
}

/**
 * @brief 计算共享内存使用率
 * @param shm 共享内存
 * @return 使用率（0.0-1.0）
 */
double calculateSharedMemoryUtilization(std::shared_ptr<SharedMemory> shm) {
    if (!shm || !shm->isOpen()) {
        return 0.0;
    }
    
    auto stats = shm->getStatistics();
    return stats.utilizationRate();
}

/**
 * @brief 验证消息完整性（增强版）
 * @param message 消息
 * @param expected_checksum 预期校验和
 * @return true 如果消息完整且校验和匹配
 */
bool validateMessageIntegrity(const Message& message, uint32_t expected_checksum) {
    if (!ipc_utils::validateMessage(message)) {
        return false;
    }
    
    uint32_t actual_checksum = ipc_utils::calculateChecksum(message.data, message.data_size);
    return actual_checksum == expected_checksum;
}

/**
 * @brief 创建消息确认机制
 * @param mq 消息队列
 * @param original_msg 原始消息
 * @param success 操作是否成功
 * @return 确认消息
 */
Message createAckMessage(std::shared_ptr<MessageQueue> mq,
                        const Message& original_msg,
                        bool success) {
    Message ack(0xFF, original_msg.receiver_id, original_msg.sender_id);
    ack.timestamp = RealtimeScheduler::getCurrentTimeUs();
    
    uint8_t status = success ? 1 : 0;
    ack.setData(&status, sizeof(status));
    
    return ack;
}

/**
 * @brief 计算消息队列的平均延迟
 * @param mq 消息队列
 * @return 平均延迟（微秒），0表示无法计算
 */
uint64_t calculateAverageQueueDelay(std::shared_ptr<MessageQueue> mq) {
    if (!mq || !mq->isOpen()) {
        return 0;
    }
    
    auto stats = mq->getStatistics();
    
    if (stats.messages_received == 0) {
        return 0;
    }
    
    // 这是一个简化的估算，实际需要跟踪每条消息的时间戳
    // 这里返回0表示需要更复杂的实现
    return 0;
}

/**
 * @brief 清理空闲的IPC资源
 * @param manager IPC管理器
 * @param idle_threshold_seconds 空闲阈值（秒）
 * @return 清理的资源数量
 */
size_t cleanupIdleIPCResources(IPCManager& manager, uint64_t idle_threshold_seconds) {
    size_t cleaned = 0;
    
    auto now = std::chrono::steady_clock::now();
    
    // 检查共享内存
    auto shm_names = manager.getAllSharedMemoryNames();
    for (const auto& name : shm_names) {
        auto shm = manager.getSharedMemory(name);
        if (shm) {
            auto stats = shm->getStatistics();
            auto age = std::chrono::duration_cast<std::chrono::seconds>(
                now - stats.creation_time).count();
            
            // 如果长时间未使用且空闲
            if (age > idle_threshold_seconds && 
                stats.read_count == 0 && 
                stats.write_count == 0) {
                manager.deleteSharedMemory(name);
                cleaned++;
            }
        }
    }
    
    // 检查消息队列
    auto mq_names = manager.getAllMessageQueueNames();
    for (const auto& name : mq_names) {
        auto mq = manager.getMessageQueue(name);
        if (mq) {
            auto stats = mq->getStatistics();
            auto age = std::chrono::duration_cast<std::chrono::seconds>(
                now - stats.creation_time).count();
            
            // 如果长时间未使用且为空
            if (age > idle_threshold_seconds && 
                stats.messages_sent == 0 && 
                stats.messages_received == 0) {
                manager.deleteMessageQueue(name);
                cleaned++;
            }
        }
    }
    
    return cleaned;
}

/**
 * @brief 导出IPC资源状态到文件
 * @param manager IPC管理器
 * @param file_path 文件路径
 * @return true 如果导出成功
 */
bool exportIPCState(const IPCManager& manager, const std::string& file_path) {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    file << manager.getPerformanceStats();
    file.close();
    
    return true;
}

} // namespace scheduler
} // namespace ev_dynamics
