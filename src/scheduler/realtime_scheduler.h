/**
 * @file realtime_scheduler.h
 * @brief POSIX实时调度器实现 - 支持实时任务调度、优先级管理和周期性任务
 * @author EV Dynamics Simulation Team
 * @date 2026-03-08
 * @version 1.0
 * 
 * @details 本模块实现了符合POSIX标准的实时调度器，提供：
 * - 多种调度策略（FIFO、RR、SPORADIC）
 * - 动态优先级管理
 * - 周期性任务支持
 * - 任务同步与互斥
 * - 性能监控与统计
 */

#ifndef REALTIME_SCHEDULER_H
#define REALTIME_SCHEDULER_H

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <functional>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <signal.h>
#include <sys/mman.h>
#endif

namespace ev_dynamics {
namespace scheduler {

/**
 * @brief 调度策略枚举
 */
enum class SchedulingPolicy {
    FIFO,           ///< 先进先出调度（SCHED_FIFO）
    ROUND_ROBIN,    ///< 轮转调度（SCHED_RR）
    SPORADIC,       ///< 随机调度（SCHED_SPORADIC）
    OTHER           ///< 普通调度（SCHED_OTHER）
};

/**
 * @brief 任务状态枚举
 */
enum class TaskState {
    CREATED,        ///< 任务已创建
    READY,          ///< 任务就绪
    RUNNING,        ///< 任务运行中
    SUSPENDED,      ///< 任务挂起
    BLOCKED,        ///< 任务阻塞
    TERMINATED      ///< 任务终止
};

/**
 * @brief 任务优先级结构
 */
struct TaskPriority {
    int base_priority;          ///< 基础优先级（0-99，数值越大优先级越高）
    int current_priority;       ///< 当前优先级（可能因优先级继承而改变）
    bool is_inherited;          ///< 是否为继承的优先级
    
    /**
     * @brief 构造函数
     * @param base 基础优先级
     */
    explicit TaskPriority(int base = 50) 
        : base_priority(base), current_priority(base), is_inherited(false) {}
    
    /**
     * @brief 设置基础优先级
     * @param priority 新的优先级值
     */
    void setBasePriority(int priority) {
        if (priority < 0 || priority > 99) {
            throw std::invalid_argument("Priority must be between 0 and 99");
        }
        base_priority = priority;
        if (!is_inherited) {
            current_priority = priority;
        }
    }
    
    /**
     * @brief 继承优先级（用于优先级继承协议）
     * @param inherited_priority 继承的优先级
     */
    void inheritPriority(int inherited_priority) {
        if (inherited_priority > current_priority) {
            current_priority = inherited_priority;
            is_inherited = true;
        }
    }
    
    /**
     * @brief 恢复基础优先级
     */
    void restoreBasePriority() {
        current_priority = base_priority;
        is_inherited = false;
    }
};

/**
 * @brief 任务统计信息
 */
struct TaskStatistics {
    uint64_t total_executions;      ///< 总执行次数
    uint64_t total_execution_time_us; ///< 总执行时间（微秒）
    uint64_t max_execution_time_us;  ///< 最大执行时间（微秒）
    uint64_t min_execution_time_us;  ///< 最小执行时间（微秒）
    uint64_t deadline_misses;        ///< 截止时间错过次数
    uint64_t preemptions;            ///< 抢占次数
    double avg_execution_time_us;    ///< 平均执行时间（微秒）
    
    /**
     * @brief 构造函数 - 初始化统计信息
     */
    TaskStatistics() 
        : total_executions(0)
        , total_execution_time_us(0)
        , max_execution_time_us(0)
        , min_execution_time_us(UINT64_MAX)
        , deadline_misses(0)
        , preemptions(0)
        , avg_execution_time_us(0.0) {}
    
    /**
     * @brief 更新执行时间统计
     * @param execution_time_us 本次执行时间（微秒）
     */
    void updateExecutionTime(uint64_t execution_time_us) {
        total_executions++;
        total_execution_time_us += execution_time_us;
        
        if (execution_time_us > max_execution_time_us) {
            max_execution_time_us = execution_time_us;
        }
        if (execution_time_us < min_execution_time_us) {
            min_execution_time_us = execution_time_us;
        }
        
        avg_execution_time_us = static_cast<double>(total_execution_time_us) / total_executions;
    }
    
    /**
     * @brief 记录截止时间错过
     */
    void recordDeadlineMiss() {
        deadline_misses++;
    }
    
    /**
     * @brief 记录抢占
     */
    void recordPreemption() {
        preemptions++;
    }
    
    /**
     * @brief 重置统计信息
     */
    void reset() {
        *this = TaskStatistics();
    }
};

/**
 * @brief 任务控制块（TCB）
 */
struct TaskControlBlock {
    std::string task_id;                ///< 任务唯一标识符
    std::string task_name;              ///< 任务名称
    TaskPriority priority;              ///< 任务优先级
    SchedulingPolicy policy;            ///< 调度策略
    TaskState state;                    ///< 任务状态
    
    std::function<void()> task_function; ///< 任务函数
    
    // 周期性任务相关
    bool is_periodic;                   ///< 是否为周期性任务
    uint64_t period_us;                 ///< 周期（微秒）
    uint64_t deadline_us;               ///< 相对截止时间（微秒）
    uint64_t next_release_time_us;      ///< 下次释放时间（微秒）
    uint64_t wcet_us;                   ///< 最坏情况执行时间（微秒）
    
    // 统计信息
    TaskStatistics statistics;          ///< 任务统计
    
    // 同步原语
    std::mutex task_mutex;              ///< 任务互斥锁
    std::condition_variable task_cv;    ///< 任务条件变量
    
    // 线程句柄
#ifdef _WIN32
    HANDLE thread_handle;               ///< Windows线程句柄
#else
    pthread_t thread_handle;            ///< POSIX线程句柄
#endif
    
    std::atomic<bool> should_terminate; ///< 终止标志
    
    /**
     * @brief 构造函数
     * @param id 任务ID
     * @param name 任务名称
     * @param func 任务函数
     * @param prio 优先级
     * @param pol 调度策略
     */
    TaskControlBlock(const std::string& id, 
                     const std::string& name,
                     std::function<void()> func,
                     int prio = 50,
                     SchedulingPolicy pol = SchedulingPolicy::FIFO)
        : task_id(id)
        , task_name(name)
        , priority(prio)
        , policy(pol)
        , state(TaskState::CREATED)
        , task_function(func)
        , is_periodic(false)
        , period_us(0)
        , deadline_us(0)
        , next_release_time_us(0)
        , wcet_us(0)
        , should_terminate(false) {
    }
    
    /**
     * @brief 设置为周期性任务
     * @param period 周期（微秒）
     * @param deadline 相对截止时间（微秒）
     * @param wcet 最坏情况执行时间（微秒）
     */
    void setPeriodic(uint64_t period, uint64_t deadline = 0, uint64_t wcet = 0) {
        is_periodic = true;
        period_us = period;
        deadline_us = (deadline == 0) ? period : deadline;
        wcet_us = wcet;
    }
    
    /**
     * @brief 更新状态
     * @param new_state 新状态
     */
    void updateState(TaskState new_state) {
        std::lock_guard<std::mutex> lock(task_mutex);
        state = new_state;
    }
    
    /**
     * @brief 检查是否应该终止
     * @return true 如果应该终止
     */
    bool shouldTerminate() const {
        return should_terminate.load();
    }
    
    /**
     * @brief 请求终止
     */
    void requestTermination() {
        should_terminate.store(true);
        task_cv.notify_all();
    }
};

/**
 * @brief 调度器配置
 */
struct SchedulerConfig {
    SchedulingPolicy default_policy;    ///< 默认调度策略
    int default_priority;               ///< 默认优先级
    bool enable_priority_inheritance;   ///< 启用优先级继承
    bool enable_statistics;             ///< 启用统计
    uint64_t tick_interval_us;          ///< 时钟滴答间隔（微秒）
    size_t max_tasks;                   ///< 最大任务数
    
    /**
     * @brief 构造函数 - 默认配置
     */
    SchedulerConfig()
        : default_policy(SchedulingPolicy::FIFO)
        , default_priority(50)
        , enable_priority_inheritance(true)
        , enable_statistics(true)
        , tick_interval_us(1000)  // 1ms
        , max_tasks(100) {}
};

/**
 * @brief 调度器统计信息
 */
struct SchedulerStatistics {
    uint64_t total_schedules;           ///< 总调度次数
    uint64_t context_switches;          ///< 上下文切换次数
    uint64_t deadline_misses;           ///< 总截止时间错过次数
    uint64_t task_creations;            ///< 任务创建次数
    uint64_t task_terminations;         ///< 任务终止次数
    double cpu_utilization;             ///< CPU利用率
    std::chrono::steady_clock::time_point start_time; ///< 启动时间
    
    /**
     * @brief 构造函数
     */
    SchedulerStatistics()
        : total_schedules(0)
        , context_switches(0)
        , deadline_misses(0)
        , task_creations(0)
        , task_terminations(0)
        , cpu_utilization(0.0)
        , start_time(std::chrono::steady_clock::now()) {}
    
    /**
     * @brief 重置统计
     */
    void reset() {
        *this = SchedulerStatistics();
    }
};

/**
 * @brief POSIX实时调度器类
 * 
 * 实现了符合POSIX标准的实时调度器，支持多种调度策略、
 * 优先级管理、周期性任务调度和性能监控。
 */
class RealtimeScheduler {
public:
    /**
     * @brief 获取调度器单例
     * @return 调度器实例引用
     */
    static RealtimeScheduler& getInstance();
    
    /**
     * @brief 初始化调度器
     * @param config 调度器配置
     * @return true 如果初始化成功
     */
    bool initialize(const SchedulerConfig& config = SchedulerConfig());
    
    /**
     * @brief 关闭调度器
     */
    void shutdown();
    
    /**
     * @brief 启动调度器
     * @return true 如果启动成功
     */
    bool start();
    
    /**
     * @brief 停止调度器
     */
    void stop();
    
    /**
     * @brief 创建任务
     * @param task_id 任务ID
     * @param task_name 任务名称
     * @param task_func 任务函数
     * @param priority 优先级
     * @param policy 调度策略
     * @return 任务控制块指针
     */
    std::shared_ptr<TaskControlBlock> createTask(
        const std::string& task_id,
        const std::string& task_name,
        std::function<void()> task_func,
        int priority = 50,
        SchedulingPolicy policy = SchedulingPolicy::FIFO
    );
    
    /**
     * @brief 创建周期性任务
     * @param task_id 任务ID
     * @param task_name 任务名称
     * @param task_func 任务函数
     * @param period_us 周期（微秒）
     * @param deadline_us 截止时间（微秒）
     * @param priority 优先级
     * @param wcet_us 最坏情况执行时间（微秒）
     * @return 任务控制块指针
     */
    std::shared_ptr<TaskControlBlock> createPeriodicTask(
        const std::string& task_id,
        const std::string& task_name,
        std::function<void()> task_func,
        uint64_t period_us,
        uint64_t deadline_us = 0,
        int priority = 50,
        uint64_t wcet_us = 0
    );
    
    /**
     * @brief 删除任务
     * @param task_id 任务ID
     * @return true 如果删除成功
     */
    bool deleteTask(const std::string& task_id);
    
    /**
     * @brief 启动任务
     * @param task_id 任务ID
     * @return true 如果启动成功
     */
    bool startTask(const std::string& task_id);
    
    /**
     * @brief 挂起任务
     * @param task_id 任务ID
     * @return true 如果挂起成功
     */
    bool suspendTask(const std::string& task_id);
    
    /**
     * @brief 恢复任务
     * @param task_id 任务ID
     * @return true 如果恢复成功
     */
    bool resumeTask(const std::string& task_id);
    
    /**
     * @brief 终止任务
     * @param task_id 任务ID
     * @return true 如果终止成功
     */
    bool terminateTask(const std::string& task_id);
    
    /**
     * @brief 设置任务优先级
     * @param task_id 任务ID
     * @param priority 新优先级
     * @return true 如果设置成功
     */
    bool setTaskPriority(const std::string& task_id, int priority);
    
    /**
     * @brief 获取任务优先级
     * @param task_id 任务ID
     * @return 优先级值，-1表示失败
     */
    int getTaskPriority(const std::string& task_id) const;
    
    /**
     * @brief 获取任务控制块
     * @param task_id 任务ID
     * @return 任务控制块指针，nullptr表示未找到
     */
    std::shared_ptr<TaskControlBlock> getTask(const std::string& task_id) const;
    
    /**
     * @brief 获取所有任务ID列表
     * @return 任务ID列表
     */
    std::vector<std::string> getAllTaskIds() const;
    
    /**
     * @brief 获取任务统计信息
     * @param task_id 任务ID
     * @return 任务统计信息
     */
    TaskStatistics getTaskStatistics(const std::string& task_id) const;
    
    /**
     * @brief 获取调度器统计信息
     * @return 调度器统计信息
     */
    SchedulerStatistics getSchedulerStatistics() const;
    
    /**
     * @brief 重置所有统计信息
     */
    void resetStatistics();
    
    /**
     * @brief 检查调度器是否正在运行
     * @return true 如果正在运行
     */
    bool isRunning() const;
    
    /**
     * @brief 获取当前时间戳（微秒）
     * @return 当前时间戳
     */
    static uint64_t getCurrentTimeUs();
    
    /**
     * @brief 睡眠指定时间（微秒）
     * @param duration_us 睡眠时间（微秒）
     */
    static void sleepFor(uint64_t duration_us);
    
    /**
     * @brief 睡眠到指定时间点
     * @param wakeup_time_us 唤醒时间（微秒）
     */
    static void sleepUntil(uint64_t wakeup_time_us);
    
private:
    // 私有构造函数（单例模式）
    RealtimeScheduler();
    ~RealtimeScheduler();
    
    // 禁用拷贝和赋值
    RealtimeScheduler(const RealtimeScheduler&) = delete;
    RealtimeScheduler& operator=(const RealtimeScheduler&) = delete;
    
    /**
     * @brief 调度器主循环
     */
    void schedulerLoop();
    
    /**
     * @brief 任务包装函数
     * @param tcb 任务控制块
     */
    void taskWrapper(std::shared_ptr<TaskControlBlock> tcb);
    
    /**
     * @brief 周期性任务执行函数
     * @param tcb 任务控制块
     */
    void periodicTaskExecution(std::shared_ptr<TaskControlBlock> tcb);
    
    /**
     * @brief 选择下一个要执行的任务
     * @return 任务控制块指针，nullptr表示没有就绪任务
     */
    std::shared_ptr<TaskControlBlock> selectNextTask();
    
    /**
     * @brief 更新就绪队列
     */
    void updateReadyQueue();
    
    /**
     * @brief 检查周期性任务释放
     */
    void checkPeriodicReleases();
    
    /**
     * @brief 执行上下文切换
     * @param from 当前任务
     * @param to 目标任务
     */
    void contextSwitch(std::shared_ptr<TaskControlBlock> from, 
                       std::shared_ptr<TaskControlBlock> to);
    
    /**
     * @brief 设置线程调度参数（POSIX）
     * @param tcb 任务控制块
     * @return true 如果设置成功
     */
    bool setThreadSchedulingParams(std::shared_ptr<TaskControlBlock> tcb);
    
    /**
     * @brief 计算CPU利用率
     * @return CPU利用率（0.0-1.0）
     */
    double calculateCpuUtilization() const;
    
    // 成员变量
    SchedulerConfig config_;                           ///< 调度器配置
    std::map<std::string, std::shared_ptr<TaskControlBlock>> tasks_; ///< 任务映射表
    mutable std::mutex tasks_mutex_;                   ///< 任务表互斥锁
    
    std::priority_queue<std::shared_ptr<TaskControlBlock>> ready_queue_; ///< 就绪队列
    std::mutex ready_queue_mutex_;                     ///< 就绪队列互斥锁
    
    std::thread scheduler_thread_;                     ///< 调度器线程
    std::atomic<bool> is_running_;                     ///< 运行标志
    std::atomic<bool> is_initialized_;                 ///< 初始化标志
    
    SchedulerStatistics statistics_;                   ///< 调度器统计
    mutable std::mutex statistics_mutex_;              ///< 统计互斥锁
    
    std::condition_variable scheduler_cv_;             ///< 调度器条件变量
    std::mutex scheduler_mutex_;                       ///< 调度器互斥锁
    
    std::shared_ptr<TaskControlBlock> current_task_;   ///< 当前运行任务
    mutable std::mutex current_task_mutex_;            ///< 当前任务互斥锁
};

/**
 * @brief 任务构建器（Builder模式）
 * 
 * 提供流畅的API来创建和配置任务。
 */
class TaskBuilder {
public:
    /**
     * @brief 构造函数
     * @param scheduler 调度器引用
     */
    explicit TaskBuilder(RealtimeScheduler& scheduler)
        : scheduler_(scheduler)
        , priority_(50)
        , policy_(SchedulingPolicy::FIFO)
        , is_periodic_(false)
        , period_us_(0)
        , deadline_us_(0)
        , wcet_us_(0) {}
    
    /**
     * @brief 设置任务ID
     * @param id 任务ID
     * @return 构建器引用
     */
    TaskBuilder& withId(const std::string& id) {
        task_id_ = id;
        return *this;
    }
    
    /**
     * @brief 设置任务名称
     * @param name 任务名称
     * @return 构建器引用
     */
    TaskBuilder& withName(const std::string& name) {
        task_name_ = name;
        return *this;
    }
    
    /**
     * @brief 设置任务函数
     * @param func 任务函数
     * @return 构建器引用
     */
    TaskBuilder& withFunction(std::function<void()> func) {
        task_function_ = func;
        return *this;
    }
    
    /**
     * @brief 设置优先级
     * @param priority 优先级（0-99）
     * @return 构建器引用
     */
    TaskBuilder& withPriority(int priority) {
        priority_ = priority;
        return *this;
    }
    
    /**
     * @brief 设置调度策略
     * @param policy 调度策略
     * @return 构建器引用
     */
    TaskBuilder& withPolicy(SchedulingPolicy policy) {
        policy_ = policy;
        return *this;
    }
    
    /**
     * @brief 设置为周期性任务
     * @param period_us 周期（微秒）
     * @param deadline_us 截止时间（微秒，默认等于周期）
     * @param wcet_us 最坏情况执行时间（微秒）
     * @return 构建器引用
     */
    TaskBuilder& asPeriodic(uint64_t period_us, uint64_t deadline_us = 0, uint64_t wcet_us = 0) {
        is_periodic_ = true;
        period_us_ = period_us;
        deadline_us_ = (deadline_us == 0) ? period_us : deadline_us;
        wcet_us_ = wcet_us;
        return *this;
    }
    
    /**
     * @brief 构建并创建任务
     * @return 任务控制块指针
     */
    std::shared_ptr<TaskControlBlock> build() {
        if (task_id_.empty() || !task_function_) {
            return nullptr;
        }
        
        if (is_periodic_) {
            return scheduler_.createPeriodicTask(
                task_id_, task_name_, task_function_,
                period_us_, deadline_us_, priority_, wcet_us_
            );
        } else {
            return scheduler_.createTask(
                task_id_, task_name_, task_function_,
                priority_, policy_
            );
        }
    }
    
private:
    RealtimeScheduler& scheduler_;
    std::string task_id_;
    std::string task_name_;
    std::function<void()> task_function_;
    int priority_;
    SchedulingPolicy policy_;
    bool is_periodic_;
    uint64_t period_us_;
    uint64_t deadline_us_;
    uint64_t wcet_us_;
};

/**
 * @brief 调度器配置构建器
 * 
 * 提供流畅的API来配置调度器。
 */
class SchedulerConfigBuilder {
public:
    /**
     * @brief 构造函数
     */
    SchedulerConfigBuilder() {
        config_.default_policy = SchedulingPolicy::FIFO;
        config_.default_priority = 50;
        config_.enable_priority_inheritance = true;
        config_.enable_statistics = true;
        config_.tick_interval_us = 1000;
        config_.max_tasks = 100;
    }
    
    /**
     * @brief 设置默认调度策略
     * @param policy 调度策略
     * @return 构建器引用
     */
    SchedulerConfigBuilder& withDefaultPolicy(SchedulingPolicy policy) {
        config_.default_policy = policy;
        return *this;
    }
    
    /**
     * @brief 设置默认优先级
     * @param priority 优先级
     * @return 构建器引用
     */
    SchedulerConfigBuilder& withDefaultPriority(int priority) {
        config_.default_priority = priority;
        return *this;
    }
    
    /**
     * @brief 启用/禁用优先级继承
     * @param enable 是否启用
     * @return 构建器引用
     */
    SchedulerConfigBuilder& withPriorityInheritance(bool enable) {
        config_.enable_priority_inheritance = enable;
        return *this;
    }
    
    /**
     * @brief 启用/禁用统计
     * @param enable 是否启用
     * @return 构建器引用
     */
    SchedulerConfigBuilder& withStatistics(bool enable) {
        config_.enable_statistics = enable;
        return *this;
    }
    
    /**
     * @brief 设置时钟滴答间隔
     * @param interval_us 间隔（微秒）
     * @return 构建器引用
     */
    SchedulerConfigBuilder& withTickInterval(uint64_t interval_us) {
        config_.tick_interval_us = interval_us;
        return *this;
    }
    
    /**
     * @brief 设置最大任务数
     * @param max_tasks 最大任务数
     * @return 构建器引用
     */
    SchedulerConfigBuilder& withMaxTasks(size_t max_tasks) {
        config_.max_tasks = max_tasks;
        return *this;
    }
    
    /**
     * @brief 构建配置
     * @return 调度器配置
     */
    SchedulerConfig build() {
        return config_;
    }
    
private:
    SchedulerConfig config_;
};

/**
 * @brief 调度器监控器
 * 
 * 提供调度器性能监控和诊断功能。
 */
class SchedulerMonitor {
public:
    /**
     * @brief 构造函数
     * @param scheduler 调度器引用
     */
    explicit SchedulerMonitor(RealtimeScheduler& scheduler)
        : scheduler_(scheduler) {}
    
    /**
     * @brief 生成性能报告
     * @return 报告字符串
     */
    std::string generateReport() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        
        oss << "========== Scheduler Performance Report ==========\n\n";
        
        // 调度器统计
        auto stats = scheduler_.getSchedulerStatistics();
        oss << "[Scheduler Statistics]\n";
        oss << "  Total Schedules: " << stats.total_schedules << "\n";
        oss << "  Context Switches: " << stats.context_switches << "\n";
        oss << "  Deadline Misses: " << stats.deadline_misses << "\n";
        oss << "  Task Creations: " << stats.task_creations << "\n";
        oss << "  Task Terminations: " << stats.task_terminations << "\n";
        oss << "  CPU Utilization: " << (stats.cpu_utilization * 100.0) << "%\n\n";
        
        // 任务统计
        oss << "[Task Statistics]\n";
        auto task_ids = scheduler_.getAllTaskIds();
        for (const auto& task_id : task_ids) {
            auto task_stats = scheduler_.getTaskStatistics(task_id);
            oss << "  Task: " << task_id << "\n";
            oss << "    Executions: " << task_stats.total_executions << "\n";
            oss << "    Avg Time: " << task_stats.avg_execution_time_us << " us\n";
            oss << "    Max Time: " << task_stats.max_execution_time_us << " us\n";
            oss << "    Min Time: " << task_stats.min_execution_time_us << " us\n";
            oss << "    Deadline Misses: " << task_stats.deadline_misses << "\n";
            oss << "    Preemptions: " << task_stats.preemptions << "\n\n";
        }
        
        oss << "==================================================\n";
        
        return oss.str();
    }
    
    /**
     * @brief 检查系统健康状态
     * @return 健康状态（0-100，100表示完美）
     */
    int checkHealth() const {
        int health_score = 100;
        
        auto stats = scheduler_.getSchedulerStatistics();
        
        // 检查截止时间错过率
        if (stats.total_schedules > 0) {
            double miss_rate = static_cast<double>(stats.deadline_misses) / stats.total_schedules;
            health_score -= static_cast<int>(miss_rate * 50);  // 最多扣50分
        }
        
        // 检查CPU利用率
        if (stats.cpu_utilization > 0.9) {
            health_score -= 20;  // CPU过载扣20分
        } else if (stats.cpu_utilization > 0.8) {
            health_score -= 10;  // CPU高负载扣10分
        }
        
        // 确保分数在0-100范围内
        return std::max(0, std::min(100, health_score));
    }
    
    /**
     * @brief 检测异常任务
     * @return 异常任务ID列表
     */
    std::vector<std::string> detectAnomalousTasks() const {
        std::vector<std::string> anomalous_tasks;
        
        auto task_ids = scheduler_.getAllTaskIds();
        for (const auto& task_id : task_ids) {
            auto stats = scheduler_.getTaskStatistics(task_id);
            
            // 检查高截止时间错过率
            if (stats.total_executions > 10) {
                double miss_rate = static_cast<double>(stats.deadline_misses) / stats.total_executions;
                if (miss_rate > 0.1) {  // 超过10%错过率
                    anomalous_tasks.push_back(task_id);
                }
            }
            
            // 检查高抢占率
            if (stats.preemptions > stats.total_executions * 2) {
                anomalous_tasks.push_back(task_id);
            }
        }
        
        return anomalous_tasks;
    }
    
    /**
     * @brief 导出统计数据为JSON
     * @return JSON字符串
     */
    std::string exportToJson() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        
        oss << "{\n";
        
        // 调度器统计
        auto stats = scheduler_.getSchedulerStatistics();
        oss << "  \"scheduler\": {\n";
        oss << "    \"total_schedules\": " << stats.total_schedules << ",\n";
        oss << "    \"context_switches\": " << stats.context_switches << ",\n";
        oss << "    \"deadline_misses\": " << stats.deadline_misses << ",\n";
        oss << "    \"cpu_utilization\": " << stats.cpu_utilization << "\n";
        oss << "  },\n";
        
        // 任务统计
        oss << "  \"tasks\": {\n";
        auto task_ids = scheduler_.getAllTaskIds();
        for (size_t i = 0; i < task_ids.size(); i++) {
            auto task_stats = scheduler_.getTaskStatistics(task_ids[i]);
            oss << "    \"" << task_ids[i] << "\": {\n";
            oss << "      \"executions\": " << task_stats.total_executions << ",\n";
            oss << "      \"avg_time_us\": " << task_stats.avg_execution_time_us << ",\n";
            oss << "      \"deadline_misses\": " << task_stats.deadline_misses << "\n";
            oss << "    }";
            if (i < task_ids.size() - 1) {
                oss << ",";
            }
            oss << "\n";
        }
        oss << "  }\n";
        oss << "}\n";
        
        return oss.str();
    }
    
private:
    RealtimeScheduler& scheduler_;
};

/**
 * @brief 调度器实用工具函数
 */
namespace scheduler_utils {
    /**
     * @brief 将调度策略转换为字符串
     * @param policy 调度策略
     * @return 策略名称
     */
    inline std::string policyToString(SchedulingPolicy policy) {
        switch (policy) {
            case SchedulingPolicy::FIFO: return "FIFO";
            case SchedulingPolicy::ROUND_ROBIN: return "ROUND_ROBIN";
            case SchedulingPolicy::SPORADIC: return "SPORADIC";
            case SchedulingPolicy::OTHER: return "OTHER";
            default: return "UNKNOWN";
        }
    }
    
    /**
     * @brief 将任务状态转换为字符串
     * @param state 任务状态
     * @return 状态名称
     */
    inline std::string stateToString(TaskState state) {
        switch (state) {
            case TaskState::CREATED: return "CREATED";
            case TaskState::READY: return "READY";
            case TaskState::RUNNING: return "RUNNING";
            case TaskState::SUSPENDED: return "SUSPENDED";
            case TaskState::BLOCKED: return "BLOCKED";
            case TaskState::TERMINATED: return "TERMINATED";
            default: return "UNKNOWN";
        }
    }
    
    /**
     * @brief 格式化时间（微秒）为可读字符串
     * @param time_us 时间（微秒）
     * @return 格式化的时间字符串
     */
    inline std::string formatTime(uint64_t time_us) {
        std::ostringstream oss;
        
        if (time_us < 1000) {
            oss << time_us << " us";
        } else if (time_us < 1000000) {
            oss << std::fixed << std::setprecision(2) << (time_us / 1000.0) << " ms";
        } else {
            oss << std::fixed << std::setprecision(2) << (time_us / 1000000.0) << " s";
        }
        
        return oss.str();
    }
    
    /**
     * @brief 计算任务利用率
     * @param wcet_us 最坏情况执行时间
     * @param period_us 周期
     * @return 利用率（0.0-1.0）
     */
    inline double calculateUtilization(uint64_t wcet_us, uint64_t period_us) {
        if (period_us == 0) {
            return 0.0;
        }
        return static_cast<double>(wcet_us) / period_us;
    }
    
    /**
     * @brief 验证任务集可调度性（利用率测试）
     * @param tasks_utilizations 任务利用率列表
     * @return true 如果可能可调度
     */
    inline bool isSchedulable(const std::vector<double>& tasks_utilizations) {
        double total_utilization = 0.0;
        for (double u : tasks_utilizations) {
            total_utilization += u;
        }
        
        // 简单的充分条件（对于RM调度）
        size_t n = tasks_utilizations.size();
        double bound = n * (std::pow(2.0, 1.0/n) - 1.0);
        
        return total_utilization <= bound;
    }
    
    /**
     * @brief 计算超周期（所有周期的最小公倍数）
     * @param periods 周期列表（微秒）
     * @return 超周期（微秒）
     */
    inline uint64_t calculateHyperPeriod(const std::vector<uint64_t>& periods) {
        if (periods.empty()) {
            return 0;
        }
        
        auto gcd = [](uint64_t a, uint64_t b) -> uint64_t {
            while (b) {
                a %= b;
                std::swap(a, b);
            }
            return a;
        };
        
        auto lcm = [gcd](uint64_t a, uint64_t b) -> uint64_t {
            if (a == 0 || b == 0) return 0;
            return (a / gcd(a, b)) * b;
        };
        
        uint64_t result = periods[0];
        for (size_t i = 1; i < periods.size(); i++) {
            result = lcm(result, periods[i]);
        }
        
        return result;
    }
} // namespace scheduler_utils

} // namespace scheduler
} // namespace ev_dynamics

#endif // REALTIME_SCHEDULER_H
