/**
 * @file realtime_scheduler.cpp
 * @brief POSIX实时调度器实现
 * @author EV Dynamics Simulation Team
 * @date 2026-03-08
 * @version 1.0
 */

#include "realtime_scheduler.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace ev_dynamics {
namespace scheduler {

// ============================================================================
// 单例模式实现
// ============================================================================

RealtimeScheduler& RealtimeScheduler::getInstance() {
    static RealtimeScheduler instance;
    return instance;
}

RealtimeScheduler::RealtimeScheduler()
    : is_running_(false)
    , is_initialized_(false)
    , current_task_(nullptr) {
}

RealtimeScheduler::~RealtimeScheduler() {
    shutdown();
}

// ============================================================================
// 初始化与关闭
// ============================================================================

bool RealtimeScheduler::initialize(const SchedulerConfig& config) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    if (is_initialized_.load()) {
        return true;  // 已经初始化
    }
    
    config_ = config;
    
#ifndef _WIN32
    // 在Linux系统上锁定内存以防止页面错误（可选）
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        std::cerr << "Warning: Failed to lock memory: " << strerror(errno) << std::endl;
        // 不是致命错误，继续
    }
#endif
    
    is_initialized_.store(true);
    return true;
}

void RealtimeScheduler::shutdown() {
    stop();
    
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    // 终止所有任务
    for (auto& pair : tasks_) {
        if (pair.second) {
            pair.second->requestTermination();
        }
    }
    
    // 等待所有任务线程结束
    for (auto& pair : tasks_) {
        if (pair.second) {
#ifdef _WIN32
            if (pair.second->thread_handle != INVALID_HANDLE_VALUE) {
                WaitForSingleObject(pair.second->thread_handle, 1000);
                CloseHandle(pair.second->thread_handle);
            }
#else
            if (pair.second->thread_handle != 0) {
                pthread_join(pair.second->thread_handle, nullptr);
            }
#endif
        }
    }
    
    tasks_.clear();
    is_initialized_.store(false);
    
#ifndef _WIN32
    munlockall();
#endif
}

bool RealtimeScheduler::start() {
    if (!is_initialized_.load()) {
        std::cerr << "Error: Scheduler not initialized" << std::endl;
        return false;
    }
    
    if (is_running_.load()) {
        return true;  // 已经在运行
    }
    
    is_running_.store(true);
    
    // 启动调度器线程
    scheduler_thread_ = std::thread(&RealtimeScheduler::schedulerLoop, this);
    
    // 设置调度器线程为高优先级
#ifdef _WIN32
    HANDLE thread_handle = scheduler_thread_.native_handle();
    SetThreadPriority(thread_handle, THREAD_PRIORITY_TIME_CRITICAL);
#else
    sched_param param;
    param.sched_priority = 99;  // 最高优先级
    pthread_setschedparam(scheduler_thread_.native_handle(), SCHED_FIFO, &param);
#endif
    
    return true;
}

void RealtimeScheduler::stop() {
    if (!is_running_.load()) {
        return;
    }
    
    is_running_.store(false);
    scheduler_cv_.notify_all();
    
    if (scheduler_thread_.joinable()) {
        scheduler_thread_.join();
    }
}

// ============================================================================
// 任务管理
// ============================================================================

std::shared_ptr<TaskControlBlock> RealtimeScheduler::createTask(
    const std::string& task_id,
    const std::string& task_name,
    std::function<void()> task_func,
    int priority,
    SchedulingPolicy policy) {
    
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    // 检查任务是否已存在
    if (tasks_.find(task_id) != tasks_.end()) {
        std::cerr << "Error: Task " << task_id << " already exists" << std::endl;
        return nullptr;
    }
    
    // 检查最大任务数限制
    if (tasks_.size() >= config_.max_tasks) {
        std::cerr << "Error: Maximum number of tasks reached" << std::endl;
        return nullptr;
    }
    
    // 创建任务控制块
    auto tcb = std::make_shared<TaskControlBlock>(task_id, task_name, task_func, priority, policy);
    
    // 设置默认调度策略
    if (policy == SchedulingPolicy::OTHER) {
        tcb->policy = config_.default_policy;
    }
    
    tasks_[task_id] = tcb;
    
    // 更新统计
    {
        std::lock_guard<std::mutex> stat_lock(statistics_mutex_);
        statistics_.task_creations++;
    }
    
    return tcb;
}

std::shared_ptr<TaskControlBlock> RealtimeScheduler::createPeriodicTask(
    const std::string& task_id,
    const std::string& task_name,
    std::function<void()> task_func,
    uint64_t period_us,
    uint64_t deadline_us,
    int priority,
    uint64_t wcet_us) {
    
    auto tcb = createTask(task_id, task_name, task_func, priority, config_.default_policy);
    
    if (tcb) {
        tcb->setPeriodic(period_us, deadline_us, wcet_us);
        tcb->next_release_time_us = getCurrentTimeUs() + period_us;
    }
    
    return tcb;
}

bool RealtimeScheduler::deleteTask(const std::string& task_id) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return false;
    }
    
    auto& tcb = it->second;
    
    // 请求任务终止
    tcb->requestTermination();
    
    // 等待任务结束
#ifdef _WIN32
    if (tcb->thread_handle != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(tcb->thread_handle, 1000);
        CloseHandle(tcb->thread_handle);
    }
#else
    if (tcb->thread_handle != 0) {
        pthread_join(tcb->thread_handle, nullptr);
    }
#endif
    
    tasks_.erase(it);
    
    // 更新统计
    {
        std::lock_guard<std::mutex> stat_lock(statistics_mutex_);
        statistics_.task_terminations++;
    }
    
    return true;
}

bool RealtimeScheduler::startTask(const std::string& task_id) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return false;
    }
    
    auto& tcb = it->second;
    
    if (tcb->state != TaskState::CREATED && tcb->state != TaskState::SUSPENDED) {
        return false;
    }
    
    tcb->updateState(TaskState::READY);
    
    // 创建任务线程
#ifdef _WIN32
    tcb->thread_handle = CreateThread(
        nullptr, 0,
        [](LPVOID param) -> DWORD {
            auto* tcb_ptr = static_cast<TaskControlBlock*>(param);
            RealtimeScheduler::getInstance().taskWrapper(
                std::shared_ptr<TaskControlBlock>(tcb_ptr, [](TaskControlBlock*){}));
            return 0;
        },
        tcb.get(), 0, nullptr);
    
    if (tcb->thread_handle == NULL) {
        tcb->updateState(TaskState::TERMINATED);
        return false;
    }
#else
    if (pthread_create(&tcb->thread_handle, nullptr,
        [](void* param) -> void* {
            auto* tcb_ptr = static_cast<TaskControlBlock*>(param);
            RealtimeScheduler::getInstance().taskWrapper(
                std::shared_ptr<TaskControlBlock>(tcb_ptr, [](TaskControlBlock*){}));
            return nullptr;
        }, tcb.get()) != 0) {
        tcb->updateState(TaskState::TERMINATED);
        return false;
    }
#endif
    
    // 设置线程调度参数
    setThreadSchedulingParams(tcb);
    
    return true;
}

bool RealtimeScheduler::suspendTask(const std::string& task_id) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return false;
    }
    
    auto& tcb = it->second;
    
    if (tcb->state != TaskState::RUNNING && tcb->state != TaskState::READY) {
        return false;
    }
    
    tcb->updateState(TaskState::SUSPENDED);
    tcb->task_cv.notify_all();
    
    return true;
}

bool RealtimeScheduler::resumeTask(const std::string& task_id) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return false;
    }
    
    auto& tcb = it->second;
    
    if (tcb->state != TaskState::SUSPENDED) {
        return false;
    }
    
    tcb->updateState(TaskState::READY);
    tcb->task_cv.notify_all();
    
    return true;
}

bool RealtimeScheduler::terminateTask(const std::string& task_id) {
    return deleteTask(task_id);
}

bool RealtimeScheduler::setTaskPriority(const std::string& task_id, int priority) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return false;
    }
    
    try {
        it->second->priority.setBasePriority(priority);
        
        // 更新线程调度参数
        setThreadSchedulingParams(it->second);
        
        return true;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int RealtimeScheduler::getTaskPriority(const std::string& task_id) const {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return -1;
    }
    
    return it->second->priority.base_priority;
}

std::shared_ptr<TaskControlBlock> RealtimeScheduler::getTask(const std::string& task_id) const {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return nullptr;
    }
    
    return it->second;
}

std::vector<std::string> RealtimeScheduler::getAllTaskIds() const {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    std::vector<std::string> ids;
    ids.reserve(tasks_.size());
    
    for (const auto& pair : tasks_) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

TaskStatistics RealtimeScheduler::getTaskStatistics(const std::string& task_id) const {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return TaskStatistics();
    }
    
    return it->second->statistics;
}

SchedulerStatistics RealtimeScheduler::getSchedulerStatistics() const {
    std::lock_guard<std::mutex> lock(statistics_mutex_);
    
    SchedulerStatistics stats = statistics_;
    stats.cpu_utilization = calculateCpuUtilization();
    
    return stats;
}

void RealtimeScheduler::resetStatistics() {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    // 重置调度器统计
    {
        std::lock_guard<std::mutex> stat_lock(statistics_mutex_);
        statistics_.reset();
    }
    
    // 重置所有任务统计
    for (auto& pair : tasks_) {
        pair.second->statistics.reset();
    }
}

bool RealtimeScheduler::isRunning() const {
    return is_running_.load();
}

// ============================================================================
// 时间相关函数
// ============================================================================

uint64_t RealtimeScheduler::getCurrentTimeUs() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

void RealtimeScheduler::sleepFor(uint64_t duration_us) {
#ifdef _WIN32
    // Windows: 使用高精度定时器
    HANDLE timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
    if (timer) {
        LARGE_INTEGER li;
        li.QuadPart = -static_cast<LONGLONG>(duration_us * 10);  // 转换为100纳秒单位
        SetWaitableTimer(timer, &li, 0, nullptr, nullptr, FALSE);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    } else {
        // 回退到标准睡眠
        std::this_thread::sleep_for(std::chrono::microseconds(duration_us));
    }
#else
    // POSIX: 使用nanosleep
    struct timespec ts;
    ts.tv_sec = duration_us / 1000000;
    ts.tv_nsec = (duration_us % 1000000) * 1000;
    nanosleep(&ts, nullptr);
#endif
}

void RealtimeScheduler::sleepUntil(uint64_t wakeup_time_us) {
    uint64_t current_time = getCurrentTimeUs();
    
    if (wakeup_time_us > current_time) {
        sleepFor(wakeup_time_us - current_time);
    }
}

// ============================================================================
// 私有成员函数实现
// ============================================================================

void RealtimeScheduler::schedulerLoop() {
    while (is_running_.load()) {
        // 检查周期性任务释放
        checkPeriodicReleases();
        
        // 更新就绪队列
        updateReadyQueue();
        
        // 选择下一个任务
        auto next_task = selectNextTask();
        
        if (next_task) {
            // 上下文切换
            std::shared_ptr<TaskControlBlock> current = nullptr;
            {
                std::lock_guard<std::mutex> lock(current_task_mutex_);
                current = current_task_;
            }
            
            if (current != next_task) {
                contextSwitch(current, next_task);
            }
        }
        
        // 睡眠一个时钟滴答
        {
            std::unique_lock<std::mutex> lock(scheduler_mutex_);
            scheduler_cv_.wait_for(lock, 
                std::chrono::microseconds(config_.tick_interval_us),
                [this] { return !is_running_.load(); });
        }
    }
}

void RealtimeScheduler::taskWrapper(std::shared_ptr<TaskControlBlock> tcb) {
    if (!tcb) {
        return;
    }
    
    // 如果是周期性任务
    if (tcb->is_periodic) {
        periodicTaskExecution(tcb);
    } else {
        // 非周期性任务，执行一次
        while (!tcb->shouldTerminate()) {
            // 等待就绪状态
            {
                std::unique_lock<std::mutex> lock(tcb->task_mutex);
                tcb->task_cv.wait(lock, [&] {
                    return tcb->state == TaskState::READY || 
                           tcb->state == TaskState::RUNNING ||
                           tcb->shouldTerminate();
                });
            }
            
            if (tcb->shouldTerminate()) {
                break;
            }
            
            // 执行任务
            tcb->updateState(TaskState::RUNNING);
            
            uint64_t start_time = getCurrentTimeUs();
            
            try {
                if (tcb->task_function) {
                    tcb->task_function();
                }
            } catch (const std::exception& e) {
                std::cerr << "Exception in task " << tcb->task_id << ": " << e.what() << std::endl;
            }
            
            uint64_t end_time = getCurrentTimeUs();
            uint64_t execution_time = end_time - start_time;
            
            // 更新统计
            if (config_.enable_statistics) {
                tcb->statistics.updateExecutionTime(execution_time);
            }
            
            tcb->updateState(TaskState::READY);
            
            // 等待下次调度或终止
            {
                std::unique_lock<std::mutex> lock(tcb->task_mutex);
                tcb->task_cv.wait(lock, [&] {
                    return tcb->shouldTerminate();
                });
            }
        }
    }
    
    tcb->updateState(TaskState::TERMINATED);
    
    // 更新统计
    {
        std::lock_guard<std::mutex> lock(statistics_mutex_);
        statistics_.task_terminations++;
    }
}

void RealtimeScheduler::periodicTaskExecution(std::shared_ptr<TaskControlBlock> tcb) {
    if (!tcb) {
        return;
    }
    
    while (!tcb->shouldTerminate()) {
        // 等待下一次释放时间
        sleepUntil(tcb->next_release_time_us);
        
        if (tcb->shouldTerminate()) {
            break;
        }
        
        // 检查是否错过截止时间
        uint64_t current_time = getCurrentTimeUs();
        if (current_time > tcb->next_release_time_us + tcb->deadline_us) {
            if (config_.enable_statistics) {
                tcb->statistics.recordDeadlineMiss();
                std::lock_guard<std::mutex> lock(statistics_mutex_);
                statistics_.deadline_misses++;
            }
            std::cerr << "Warning: Task " << tcb->task_id << " missed deadline" << std::endl;
        }
        
        // 执行任务
        tcb->updateState(TaskState::RUNNING);
        
        uint64_t start_time = getCurrentTimeUs();
        
        try {
            if (tcb->task_function) {
                tcb->task_function();
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in periodic task " << tcb->task_id << ": " << e.what() << std::endl;
        }
        
        uint64_t end_time = getCurrentTimeUs();
        uint64_t execution_time = end_time - start_time;
        
        // 更新统计
        if (config_.enable_statistics) {
            tcb->statistics.updateExecutionTime(execution_time);
            
            // 检查是否超过WCET
            if (tcb->wcet_us > 0 && execution_time > tcb->wcet_us) {
                std::cerr << "Warning: Task " << tcb->task_id 
                          << " exceeded WCET (" << execution_time << " > " << tcb->wcet_us << ")" 
                          << std::endl;
            }
        }
        
        tcb->updateState(TaskState::READY);
        
        // 更新下次释放时间
        tcb->next_release_time_us += tcb->period_us;
    }
    
    tcb->updateState(TaskState::TERMINATED);
}

std::shared_ptr<TaskControlBlock> RealtimeScheduler::selectNextTask() {
    std::lock_guard<std::mutex> lock(ready_queue_mutex_);
    
    // 就绪队列已经按优先级排序，取出最高优先级任务
    while (!ready_queue_.empty()) {
        auto task = ready_queue_.top();
        ready_queue_.pop();
        
        if (task && task->state == TaskState::READY) {
            // 更新统计
            if (config_.enable_statistics) {
                std::lock_guard<std::mutex> stat_lock(statistics_mutex_);
                statistics_.total_schedules++;
            }
            return task;
        }
    }
    
    return nullptr;
}

void RealtimeScheduler::updateReadyQueue() {
    std::lock_guard<std::mutex> lock(ready_queue_mutex_);
    
    // 清空就绪队列
    while (!ready_queue_.empty()) {
        ready_queue_.pop();
    }
    
    // 将所有就绪任务加入队列
    std::lock_guard<std::mutex> tasks_lock(tasks_mutex_);
    for (const auto& pair : tasks_) {
        if (pair.second && pair.second->state == TaskState::READY) {
            ready_queue_.push(pair.second);
        }
    }
}

void RealtimeScheduler::checkPeriodicReleases() {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    uint64_t current_time = getCurrentTimeUs();
    
    for (auto& pair : tasks_) {
        auto& tcb = pair.second;
        
        if (tcb && tcb->is_periodic && tcb->state == TaskState::SUSPENDED) {
            // 检查是否到达释放时间
            if (current_time >= tcb->next_release_time_us) {
                tcb->updateState(TaskState::READY);
                tcb->task_cv.notify_all();
            }
        }
    }
}

void RealtimeScheduler::contextSwitch(
    std::shared_ptr<TaskControlBlock> from,
    std::shared_ptr<TaskControlBlock> to) {
    
    // 更新当前任务
    {
        std::lock_guard<std::mutex> lock(current_task_mutex_);
        current_task_ = to;
    }
    
    // 记录上下文切换
    if (config_.enable_statistics) {
        if (from) {
            from->statistics.recordPreemption();
        }
        
        std::lock_guard<std::mutex> lock(statistics_mutex_);
        statistics_.context_switches++;
    }
    
    // 唤醒目标任务
    if (to) {
        to->task_cv.notify_all();
    }
}

bool RealtimeScheduler::setThreadSchedulingParams(std::shared_ptr<TaskControlBlock> tcb) {
    if (!tcb) {
        return false;
    }
    
#ifdef _WIN32
    // Windows线程优先级设置
    HANDLE thread_handle = tcb->thread_handle;
    if (thread_handle == INVALID_HANDLE_VALUE || thread_handle == NULL) {
        return false;
    }
    
    int windows_priority;
    switch (tcb->policy) {
        case SchedulingPolicy::FIFO:
        case SchedulingPolicy::ROUND_ROBIN:
            // 实时优先级（Windows中THREAD_PRIORITY_TIME_CRITICAL等价于实时）
            windows_priority = THREAD_PRIORITY_TIME_CRITICAL;
            break;
        default:
            // 普通优先级
            windows_priority = THREAD_PRIORITY_NORMAL;
            break;
    }
    
    return SetThreadPriority(thread_handle, windows_priority) != 0;
    
#else
    // POSIX线程调度参数设置
    int policy;
    struct sched_param param;
    
    switch (tcb->policy) {
        case SchedulingPolicy::FIFO:
            policy = SCHED_FIFO;
            break;
        case SchedulingPolicy::ROUND_ROBIN:
            policy = SCHED_RR;
            break;
        case SchedulingPolicy::SPORADIC:
#ifdef SCHED_SPORADIC
            policy = SCHED_SPORADIC;
#else
            policy = SCHED_FIFO;  // 回退到FIFO
#endif
            break;
        default:
            policy = SCHED_OTHER;
            break;
    }
    
    param.sched_priority = tcb->priority.current_priority;
    
    return pthread_setschedparam(tcb->thread_handle, policy, &param) == 0;
#endif
}

double RealtimeScheduler::calculateCpuUtilization() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        now - statistics_.start_time).count();
    
    if (elapsed == 0) {
        return 0.0;
    }
    
    // 计算所有任务的总执行时间
    uint64_t total_execution_time = 0;
    {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        for (const auto& pair : tasks_) {
            total_execution_time += pair.second->statistics.total_execution_time_us;
        }
    }
    
    return static_cast<double>(total_execution_time) / elapsed;
}

// ============================================================================
// 额外的实用函数实现
// ============================================================================

/**
 * @brief 将任务控制块转换为字符串表示
 * @param tcb 任务控制块
 * @return 字符串表示
 */
std::string taskToString(const TaskControlBlock& tcb) {
    std::ostringstream oss;
    oss << "Task {\n";
    oss << "  ID: " << tcb.task_id << "\n";
    oss << "  Name: " << tcb.task_name << "\n";
    oss << "  Priority: " << tcb.priority.base_priority << "\n";
    oss << "  Policy: " << static_cast<int>(tcb.policy) << "\n";
    oss << "  State: " << static_cast<int>(tcb.state) << "\n";
    oss << "  Periodic: " << (tcb.is_periodic ? "Yes" : "No") << "\n";
    if (tcb.is_periodic) {
        oss << "  Period: " << tcb.period_us << " us\n";
        oss << "  Deadline: " << tcb.deadline_us << " us\n";
        oss << "  WCET: " << tcb.wcet_us << " us\n";
    }
    oss << "}";
    return oss.str();
}

/**
 * @brief 验证任务参数的有效性
 * @param task_id 任务ID
 * @param task_func 任务函数
 * @param priority 优先级
 * @return true 如果参数有效
 */
bool validateTaskParameters(const std::string& task_id,
                           const std::function<void()>& task_func,
                           int priority) {
    if (task_id.empty()) {
        return false;
    }
    
    if (!task_func) {
        return false;
    }
    
    if (priority < 0 || priority > 99) {
        return false;
    }
    
    return true;
}

/**
 * @brief 计算任务的响应时间（简化版）
 * @param wcet 最坏情况执行时间
 * @param period 周期
 * @param higher_priority_utilizations 更高优先级任务的利用率
 * @return 响应时间
 */
uint64_t calculateResponseTime(uint64_t wcet,
                               uint64_t period,
                               const std::vector<double>& higher_priority_utilizations) {
    // 简化的响应时间计算（迭代法）
    double R = static_cast<double>(wcet);
    double prev_R = 0;
    const int MAX_ITERATIONS = 100;
    const double EPSILON = 1.0;
    
    for (int i = 0; i < MAX_ITERATIONS; i++) {
        prev_R = R;
        
        double interference = 0.0;
        for (double util : higher_priority_utilizations) {
            interference += util * R;
        }
        
        R = static_cast<double>(wcet) + interference;
        
        if (std::abs(R - prev_R) < EPSILON) {
            break;
        }
        
        // 如果响应时间超过周期，认为不可调度
        if (R > static_cast<double>(period)) {
            return static_cast<uint64_t>(period + 1);
        }
    }
    
    return static_cast<uint64_t>(R);
}

/**
 * @brief 检查任务集是否满足速率单调调度（RM）的充分条件
 * @param utilizations 任务利用率列表
 * @return true 如果满足充分条件
 */
bool checkRMSufficientCondition(const std::vector<double>& utilizations) {
    double total_utilization = 0.0;
    for (double u : utilizations) {
        total_utilization += u;
    }
    
    // Liu & Layland界限
    size_t n = utilizations.size();
    if (n == 0) {
        return true;
    }
    
    double bound = n * (std::pow(2.0, 1.0/n) - 1.0);
    
    return total_utilization <= bound;
}

/**
 * @brief 计算任务的松弛时间
 * @param deadline 截止时间
 * @param remaining_exec_time 剩余执行时间
 * @param current_time 当前时间
 * @return 松弛时间
 */
int64_t calculateSlackTime(uint64_t deadline,
                          uint64_t remaining_exec_time,
                          uint64_t current_time) {
    int64_t slack = static_cast<int64_t>(deadline - current_time - remaining_exec_time);
    return slack;
}

/**
 * @brief 生成任务调度时间线（用于调试和可视化）
 * @param tasks 任务列表
 * @param duration_us 持续时间（微秒）
 * @param time_granularity_us 时间粒度（微秒）
 * @return 时间线表示（简化版）
 */
std::string generateTimeline(const std::vector<std::shared_ptr<TaskControlBlock>>& tasks,
                            uint64_t duration_us,
                            uint64_t time_granularity_us) {
    std::ostringstream oss;
    oss << "Timeline (duration=" << duration_us << " us, granularity=" << time_granularity_us << " us):\n";
    
    for (uint64_t t = 0; t < duration_us; t += time_granularity_us) {
        oss << "[" << t << "] ";
        
        // 检查每个任务在这个时间点的状态
        for (const auto& task : tasks) {
            if (task->is_periodic) {
                uint64_t release_time = task->next_release_time_us;
                if (t >= release_time && t < release_time + task->deadline_us) {
                    oss << task->task_id << " ";
                }
            }
        }
        
        oss << "\n";
    }
    
    return oss.str();
}

/**
 * @brief 计算EDF（最早截止时间优先）调度的最优性
 * @param tasks 任务列表
 * @return true 如果EDF可以调度该任务集
 */
bool isEDFSchedulable(const std::vector<std::shared_ptr<TaskControlBlock>>& tasks) {
    double total_utilization = 0.0;
    
    for (const auto& task : tasks) {
        if (task->is_periodic && task->period_us > 0) {
            double utilization = static_cast<double>(task->wcet_us) / task->period_us;
            total_utilization += utilization;
        }
    }
    
    // EDF的充要条件：总利用率 <= 1
    return total_utilization <= 1.0;
}

/**
 * @brief 优先级比较函数（用于排序）
 * @param a 任务A
 * @param b 任务B
 * @return true 如果A优先级更高
 */
bool compareTaskPriority(const std::shared_ptr<TaskControlBlock>& a,
                        const std::shared_ptr<TaskControlBlock>& b) {
    return a->priority.current_priority > b->priority.current_priority;
}

/**
 * @brief 截止时间比较函数（用于EDF调度）
 * @param a 任务A
 * @param b 任务B
 * @param current_time 当前时间
 * @return true 如果A的截止时间更早
 */
bool compareTaskDeadline(const std::shared_ptr<TaskControlBlock>& a,
                        const std::shared_ptr<TaskControlBlock>& b,
                        uint64_t current_time) {
    uint64_t deadline_a = a->next_release_time_us + a->deadline_us;
    uint64_t deadline_b = b->next_release_time_us + b->deadline_us;
    return deadline_a < deadline_b;
}

} // namespace scheduler
} // namespace ev_dynamics
