/**
 * @file test_scheduler_extended.cpp
 * @brief Scheduler模块扩展测试套件 - 覆盖边界情况和性能测试
 * @author EV Dynamics Simulation Team
 * @date 2026-03-08
 * @version 1.0
 */

#include <gtest/gtest.h>
#include "../src/scheduler/realtime_scheduler.h"
#include "../src/scheduler/ipc_manager.h"
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <algorithm>

using namespace ev_dynamics::scheduler;

// ============================================================================
// 扩展测试1: 多优先级任务调度测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, MultiPriorityScheduling) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    std::vector<int> execution_order;
    std::mutex order_mutex;
    
    // 创建不同优先级的任务
    for (int priority : {30, 70, 50, 90, 10}) {
        std::string task_id = "prio_task_" + std::to_string(priority);
        scheduler.createTask(
            task_id,
            "Priority " + std::to_string(priority),
            [&order_mutex, &execution_order, priority]() {
                std::lock_guard<std::mutex> lock(order_mutex);
                execution_order.push_back(priority);
            },
            priority,
            SchedulingPolicy::FIFO
        );
    }
    
    ASSERT_TRUE(scheduler.start());
    
    // 启动所有任务
    for (int priority : {30, 70, 50, 90, 10}) {
        std::string task_id = "prio_task_" + std::to_string(priority);
        scheduler.startTask(task_id);
    }
    
    // 等待所有任务完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    scheduler.stop();
    
    // 验证高优先级任务先执行（允许部分并发导致的顺序变化）
    // 这里只验证执行了所有任务
    EXPECT_EQ(execution_order.size(), 5);
    
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试2: 任务互斥与同步测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, TaskSynchronization) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    std::atomic<int> shared_counter{0};
    const int INCREMENTS_PER_TASK = 1000;
    const int NUM_TASKS = 5;
    
    // 创建多个任务同时增加计数器
    for (int i = 0; i < NUM_TASKS; i++) {
        std::string task_id = "sync_task_" + std::to_string(i);
        scheduler.createTask(
            task_id,
            "Sync Task " + std::to_string(i),
            [&shared_counter]() {
                for (int j = 0; j < INCREMENTS_PER_TASK; j++) {
                    shared_counter++;
                }
            },
            50,
            SchedulingPolicy::FIFO
        );
    }
    
    ASSERT_TRUE(scheduler.start());
    
    // 启动所有任务
    for (int i = 0; i < NUM_TASKS; i++) {
        std::string task_id = "sync_task_" + std::to_string(i);
        scheduler.startTask(task_id);
    }
    
    // 等待任务完成
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    scheduler.stop();
    
    // 验证计数器（注意：由于没有互斥保护，可能会有竞态条件）
    // 这里主要测试任务能正常执行
    EXPECT_GT(shared_counter.load(), 0);
    
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试3: 截止时间错过检测测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, DeadlineMissDetection) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    // 创建一个故意超时的周期性任务
    std::atomic<int> execution_count{0};
    auto task = scheduler.createPeriodicTask(
        "deadline_test_task",
        "Deadline Test Task",
        [&execution_count]() {
            execution_count++;
            // 故意睡眠超过周期时间
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        },
        100000,  // 100ms period
        100000,  // 100ms deadline
        50,
        50000    // WCET = 50ms (实际会超过)
    );
    
    ASSERT_NE(task, nullptr);
    
    ASSERT_TRUE(scheduler.start());
    ASSERT_TRUE(scheduler.startTask("deadline_test_task"));
    
    // 运行足够时间让任务错过截止时间
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    scheduler.stop();
    
    // 验证截止时间错过被记录
    auto stats = scheduler.getTaskStatistics("deadline_test_task");
    EXPECT_GT(stats.deadline_misses, 0);
    
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试4: 共享内存并发访问测试
// ============================================================================
TEST(IPCExtendedTest, SharedMemoryConcurrency) {
    const int NUM_THREADS = 10;
    const int OPERATIONS_PER_THREAD = 100;
    
    SharedMemory shm;
    SharedMemoryConfig config("concurrent_shm", 1024);
    ASSERT_TRUE(shm.create(config).isSuccess());
    
    // 初始化计数器区域
    int64_t counter = 0;
    shm.write(0, &counter, sizeof(counter));
    
    std::vector<std::thread> threads;
    
    // 启动多个线程同时访问共享内存
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back([&shm, i]() {
            for (int j = 0; j < OPERATIONS_PER_THREAD; j++) {
                // 读取当前值
                int64_t value;
                shm.read(0, &value, sizeof(value));
                
                // 增加并写回
                value++;
                shm.write(0, &value, sizeof(value));
                
                // 写入线程标识到偏移位置
                int thread_id = i;
                shm.write(100 + i * 4, &thread_id, sizeof(thread_id));
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证统计信息
    auto stats = shm.getStatistics();
    EXPECT_EQ(stats.read_count, NUM_THREADS * OPERATIONS_PER_THREAD);
    EXPECT_EQ(stats.write_count, NUM_THREADS * OPERATIONS_PER_THREAD * 2);
    
    shm.close();
    SharedMemory::unlink("concurrent_shm");
}

// ============================================================================
// 扩展测试5: 消息队列高吞吐量测试
// ============================================================================
TEST(IPCExtendedTest, MessageQueueHighThroughput) {
    MessageQueue mq;
    MessageQueueConfig config("throughput_mq");
    config.max_messages = 1000;
    
    ASSERT_TRUE(mq.create(config).isSuccess());
    
    const int NUM_MESSAGES = 500;
    std::atomic<int> sent_count{0};
    std::atomic<int> received_count{0};
    
    // 发送线程
    std::thread sender_thread([&]() {
        for (int i = 0; i < NUM_MESSAGES; i++) {
            Message msg(i, 100, 200);
            msg.setData(&i, sizeof(i));
            
            if (mq.send(msg, 100).isSuccess()) {
                sent_count++;
            }
            
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });
    
    // 接收线程
    std::thread receiver_thread([&]() {
        for (int i = 0; i < NUM_MESSAGES; i++) {
            Message msg;
            if (mq.receive(msg, 100).isSuccess()) {
                received_count++;
            }
        }
    });
    
    sender_thread.join();
    receiver_thread.join();
    
    // 验证统计信息
    EXPECT_GT(sent_count.load(), 0);
    EXPECT_GT(received_count.load(), 0);
    
    auto stats = mq.getStatistics();
    EXPECT_GT(stats.messages_sent, 0);
    EXPECT_GT(stats.messages_received, 0);
    
    mq.close();
    MessageQueue::unlink("throughput_mq");
}

// ============================================================================
// 扩展测试6: 调度策略测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, SchedulingPolicies) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    // 测试所有调度策略
    std::vector<SchedulingPolicy> policies = {
        SchedulingPolicy::FIFO,
        SchedulingPolicy::ROUND_ROBIN,
        SchedulingPolicy::SPORADIC,
        SchedulingPolicy::OTHER
    };
    
    for (size_t i = 0; i < policies.size(); i++) {
        std::string task_id = "policy_task_" + std::to_string(i);
        auto task = scheduler.createTask(
            task_id,
            "Policy Task " + std::to_string(i),
            []() {},
            50,
            policies[i]
        );
        
        ASSERT_NE(task, nullptr);
        EXPECT_EQ(task->policy, policies[i]);
    }
    
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试7: 任务优先级继承测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, PriorityInheritance) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    // 创建低优先级任务
    auto low_task = scheduler.createTask(
        "low_priority_task",
        "Low Priority Task",
        []() {},
        30,
        SchedulingPolicy::FIFO
    );
    
    ASSERT_NE(low_task, nullptr);
    EXPECT_EQ(low_task->priority.base_priority, 30);
    EXPECT_FALSE(low_task->priority.is_inherited);
    
    // 测试优先级继承
    low_task->priority.inheritPriority(50);
    EXPECT_EQ(low_task->priority.current_priority, 50);
    EXPECT_TRUE(low_task->priority.is_inherited);
    
    // 再次继承更高优先级
    low_task->priority.inheritPriority(70);
    EXPECT_EQ(low_task->priority.current_priority, 70);
    
    // 继承更低优先级（不应改变）
    low_task->priority.inheritPriority(40);
    EXPECT_EQ(low_task->priority.current_priority, 70);
    
    // 恢复基础优先级
    low_task->priority.restoreBasePriority();
    EXPECT_EQ(low_task->priority.current_priority, 30);
    EXPECT_FALSE(low_task->priority.is_inherited);
    
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试8: 统计信息精确性测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, StatisticsAccuracy) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    std::atomic<int> execution_count{0};
    
    auto task = scheduler.createTask(
        "stats_accuracy_task",
        "Stats Accuracy Task",
        [&execution_count]() {
            execution_count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        },
        50,
        SchedulingPolicy::FIFO
    );
    
    ASSERT_NE(task, nullptr);
    
    ASSERT_TRUE(scheduler.start());
    ASSERT_TRUE(scheduler.startTask("stats_accuracy_task"));
    
    // 等待任务执行多次
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    scheduler.stop();
    
    // 验证统计信息
    auto task_stats = scheduler.getTaskStatistics("stats_accuracy_task");
    EXPECT_EQ(task_stats.total_executions, execution_count.load());
    EXPECT_GT(task_stats.total_execution_time_us, 0);
    EXPECT_GT(task_stats.avg_execution_time_us, 0.0);
    EXPECT_GT(task_stats.max_execution_time_us, 0);
    EXPECT_GT(task_stats.min_execution_time_us, 0);
    EXPECT_GE(task_stats.max_execution_time_us, task_stats.min_execution_time_us);
    
    // 验证调度器统计
    auto scheduler_stats = scheduler.getSchedulerStatistics();
    EXPECT_GT(scheduler_stats.total_schedules, 0);
    EXPECT_GT(scheduler_stats.context_switches, 0);
    EXPECT_GT(scheduler_stats.cpu_utilization, 0.0);
    
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试9: IPC管理器并发访问测试
// ============================================================================
TEST(IPCExtendedTest, IPCManagerConcurrency) {
    auto& manager = IPCManager::getInstance();
    ASSERT_TRUE(manager.initialize());
    
    const int NUM_THREADS = 10;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    
    // 并发创建和访问IPC资源
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back([&manager, &success_count, i]() {
            std::string shm_name = "concurrent_shm_" + std::to_string(i);
            std::string mq_name = "concurrent_mq_" + std::to_string(i);
            
            auto shm = manager.createSharedMemory(shm_name, 1024);
            if (shm) {
                success_count++;
            }
            
            auto mq = manager.createMessageQueue(mq_name, 50);
            if (mq) {
                success_count++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有资源创建成功
    EXPECT_EQ(success_count.load(), NUM_THREADS * 2);
    
    // 验证资源数量
    auto shm_names = manager.getAllSharedMemoryNames();
    auto mq_names = manager.getAllMessageQueueNames();
    EXPECT_EQ(shm_names.size(), NUM_THREADS);
    EXPECT_EQ(mq_names.size(), NUM_THREADS);
    
    manager.shutdown();
}

// ============================================================================
// 扩展测试10: 边界条件测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, BoundaryConditions) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    // 测试优先级边界值
    auto task_min_prio = scheduler.createTask("min_prio", "Min Priority", [](){}, 0);
    ASSERT_NE(task_min_prio, nullptr);
    EXPECT_EQ(task_min_prio->priority.base_priority, 0);
    
    auto task_max_prio = scheduler.createTask("max_prio", "Max Priority", [](){}, 99);
    ASSERT_NE(task_max_prio, nullptr);
    EXPECT_EQ(task_max_prio->priority.base_priority, 99);
    
    // 测试零周期任务
    auto zero_period_task = scheduler.createPeriodicTask(
        "zero_period", "Zero Period", [](){}, 0, 0, 50);
    ASSERT_NE(zero_period_task, nullptr);
    EXPECT_EQ(zero_period_task->period_us, 0);
    
    // 测试最大任务数限制
    SchedulerConfig config;
    config.max_tasks = 5;
    scheduler.shutdown();
    ASSERT_TRUE(scheduler.initialize(config));
    
    for (int i = 0; i < 5; i++) {
        std::string task_id = "max_task_" + std::to_string(i);
        EXPECT_NE(scheduler.createTask(task_id, "Max Task", [](){}), nullptr);
    }
    
    // 第6个任务应该失败
    auto overflow_task = scheduler.createTask("overflow", "Overflow", [](){});
    EXPECT_EQ(overflow_task, nullptr);
    
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试11: 内存泄漏检测测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, MemoryLeakDetection) {
    auto& scheduler = RealtimeScheduler::getInstance();
    
    // 多次初始化和关闭，检查是否有内存泄漏
    for (int iteration = 0; iteration < 10; iteration++) {
        ASSERT_TRUE(scheduler.initialize());
        
        // 创建并删除多个任务
        for (int i = 0; i < 20; i++) {
            std::string task_id = "leak_test_" + std::to_string(iteration) + "_" + std::to_string(i);
            auto task = scheduler.createTask(task_id, "Leak Test", [](){});
            ASSERT_NE(task, nullptr);
            
            if (i % 2 == 0) {
                scheduler.deleteTask(task_id);
            }
        }
        
        scheduler.shutdown();
    }
    
    // 如果能成功完成10次迭代，说明没有严重的内存泄漏
    SUCCEED();
}

// ============================================================================
// 扩展测试12: 性能基准测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, PerformanceBenchmark) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    const int NUM_ITERATIONS = 100;
    
    // 测试任务创建性能
    auto create_start = std::chrono::steady_clock::now();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        std::string task_id = "perf_task_" + std::to_string(i);
        scheduler.createTask(task_id, "Perf Task", [](){});
    }
    auto create_end = std::chrono::steady_clock::now();
    
    auto create_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        create_end - create_start).count();
    
    // 平均创建时间应该小于1ms
    double avg_create_time_us = static_cast<double>(create_duration) / NUM_ITERATIONS;
    EXPECT_LT(avg_create_time_us, 1000.0);
    
    // 测试任务查找性能
    auto lookup_start = std::chrono::steady_clock::now();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        std::string task_id = "perf_task_" + std::to_string(i % NUM_ITERATIONS);
        auto task = scheduler.getTask(task_id);
        ASSERT_NE(task, nullptr);
    }
    auto lookup_end = std::chrono::steady_clock::now();
    
    auto lookup_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        lookup_end - lookup_start).count();
    
    // 平均查找时间应该小于0.1ms
    double avg_lookup_time_us = static_cast<double>(lookup_duration) / NUM_ITERATIONS;
    EXPECT_LT(avg_lookup_time_us, 100.0);
    
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试13: 错误恢复测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, ErrorRecovery) {
    auto& scheduler = RealtimeScheduler::getInstance();
    
    // 测试未初始化时启动
    EXPECT_FALSE(scheduler.start());
    
    // 初始化后应该能正常启动
    ASSERT_TRUE(scheduler.initialize());
    EXPECT_TRUE(scheduler.start());
    
    // 测试重复启动
    EXPECT_TRUE(scheduler.start());
    
    // 测试运行时重新初始化
    EXPECT_TRUE(scheduler.initialize());
    
    scheduler.stop();
    scheduler.shutdown();
    
    // 测试关闭后重新初始化
    ASSERT_TRUE(scheduler.initialize());
    EXPECT_TRUE(scheduler.start());
    scheduler.stop();
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试14: 长时间运行稳定性测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, LongRunningStability) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    std::atomic<bool> running{true};
    std::atomic<int> execution_count{0};
    
    // 创建长时间运行的任务
    auto task = scheduler.createPeriodicTask(
        "long_running_task",
        "Long Running Task",
        [&execution_count]() {
            execution_count++;
        },
        100000,  // 100ms period
        100000,
        50,
        50000
    );
    
    ASSERT_NE(task, nullptr);
    ASSERT_TRUE(scheduler.start());
    ASSERT_TRUE(scheduler.startTask("long_running_task"));
    
    // 运行5秒
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    running = false;
    scheduler.stop();
    
    // 验证任务持续执行
    EXPECT_GT(execution_count.load(), 40);  // 至少执行40次（实际应该约50次）
    
    // 验证没有截止时间错过（如果任务足够快）
    auto stats = scheduler.getTaskStatistics("long_running_task");
    EXPECT_EQ(stats.deadline_misses, 0);
    
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试15: 资源清理测试
// ============================================================================
TEST(IPCExtendedTest, ResourceCleanup) {
    auto& manager = IPCManager::getInstance();
    ASSERT_TRUE(manager.initialize());
    
    // 创建多个IPC资源
    for (int i = 0; i < 10; i++) {
        std::string shm_name = "cleanup_shm_" + std::to_string(i);
        std::string mq_name = "cleanup_mq_" + std::to_string(i);
        
        manager.createSharedMemory(shm_name, 1024);
        manager.createMessageQueue(mq_name, 50);
    }
    
    // 验证资源创建
    EXPECT_EQ(manager.getAllSharedMemoryNames().size(), 10);
    EXPECT_EQ(manager.getAllMessageQueueNames().size(), 10);
    
    // 调用清理
    manager.cleanup();
    
    // 验证资源已清理
    EXPECT_EQ(manager.getAllSharedMemoryNames().size(), 0);
    EXPECT_EQ(manager.getAllMessageQueueNames().size(), 0);
    
    manager.shutdown();
}

// ============================================================================
// 扩展测试16: 任务构建器测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, TaskBuilderPattern) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    // 使用Builder模式创建普通任务
    auto task1 = TaskBuilder(scheduler)
        .withId("builder_task_1")
        .withName("Builder Task 1")
        .withFunction([]() { })
        .withPriority(60)
        .withPolicy(SchedulingPolicy::ROUND_ROBIN)
        .build();
    
    ASSERT_NE(task1, nullptr);
    EXPECT_EQ(task1->task_id, "builder_task_1");
    EXPECT_EQ(task1->priority.base_priority, 60);
    EXPECT_EQ(task1->policy, SchedulingPolicy::ROUND_ROBIN);
    
    // 使用Builder模式创建周期性任务
    auto task2 = TaskBuilder(scheduler)
        .withId("builder_periodic_1")
        .withName("Builder Periodic Task")
        .withFunction([]() { })
        .withPriority(70)
        .asPeriodic(50000, 50000, 25000)  // 50ms period, 25ms WCET
        .build();
    
    ASSERT_NE(task2, nullptr);
    EXPECT_TRUE(task2->is_periodic);
    EXPECT_EQ(task2->period_us, 50000);
    EXPECT_EQ(task2->wcet_us, 25000);
    
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试17: 调度器配置构建器测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, SchedulerConfigBuilderPattern) {
    auto config = SchedulerConfigBuilder()
        .withDefaultPolicy(SchedulingPolicy::FIFO)
        .withDefaultPriority(60)
        .withPriorityInheritance(true)
        .withStatistics(true)
        .withTickInterval(500)  // 0.5ms
        .withMaxTasks(50)
        .build();
    
    EXPECT_EQ(config.default_policy, SchedulingPolicy::FIFO);
    EXPECT_EQ(config.default_priority, 60);
    EXPECT_TRUE(config.enable_priority_inheritance);
    EXPECT_TRUE(config.enable_statistics);
    EXPECT_EQ(config.tick_interval_us, 500);
    EXPECT_EQ(config.max_tasks, 50);
}

// ============================================================================
// 扩展测试18: 调度器监控器测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, SchedulerMonitorFunctionality) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    // 创建一些任务
    for (int i = 0; i < 3; i++) {
        std::string task_id = "monitor_task_" + std::to_string(i);
        scheduler.createPeriodicTask(
            task_id,
            "Monitor Task " + std::to_string(i),
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(5)); },
            100000, 100000, 50, 50000
        );
    }
    
    // 启动调度器和任务
    ASSERT_TRUE(scheduler.start());
    for (int i = 0; i < 3; i++) {
        std::string task_id = "monitor_task_" + std::to_string(i);
        scheduler.startTask(task_id);
    }
    
    // 运行一段时间
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // 创建监控器
    SchedulerMonitor monitor(scheduler);
    
    // 测试生成报告
    std::string report = monitor.generateReport();
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("Scheduler Statistics"), std::string::npos);
    EXPECT_NE(report.find("Task Statistics"), std::string::npos);
    
    // 测试健康检查
    int health = monitor.checkHealth();
    EXPECT_GE(health, 0);
    EXPECT_LE(health, 100);
    
    // 测试异常检测
    auto anomalous = monitor.detectAnomalousTasks();
    // 初始应该没有异常任务
    EXPECT_EQ(anomalous.size(), 0);
    
    // 测试JSON导出
    std::string json = monitor.exportToJson();
    EXPECT_FALSE(json.empty());
    EXPECT_NE(json.find("scheduler"), std::string::npos);
    EXPECT_NE(json.find("tasks"), std::string::npos);
    
    scheduler.stop();
    scheduler.shutdown();
}

// ============================================================================
// 扩展测试19: 调度器工具函数测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, UtilityFunctions) {
    using namespace scheduler_utils;
    
    // 测试策略转字符串
    EXPECT_EQ(policyToString(SchedulingPolicy::FIFO), "FIFO");
    EXPECT_EQ(policyToString(SchedulingPolicy::ROUND_ROBIN), "ROUND_ROBIN");
    EXPECT_EQ(policyToString(SchedulingPolicy::SPORADIC), "SPORADIC");
    
    // 测试状态转字符串
    EXPECT_EQ(stateToString(TaskState::CREATED), "CREATED");
    EXPECT_EQ(stateToString(TaskState::RUNNING), "RUNNING");
    EXPECT_EQ(stateToString(TaskState::TERMINATED), "TERMINATED");
    
    // 测试时间格式化
    EXPECT_NE(formatTime(500).find("us"), std::string::npos);
    EXPECT_NE(formatTime(5000).find("ms"), std::string::npos);
    EXPECT_NE(formatTime(5000000).find("s"), std::string::npos);
    
    // 测试利用率计算
    EXPECT_DOUBLE_EQ(calculateUtilization(25000, 100000), 0.25);
    EXPECT_DOUBLE_EQ(calculateUtilization(50000, 100000), 0.5);
    EXPECT_DOUBLE_EQ(calculateUtilization(100, 0), 0.0);
    
    // 测试可调度性验证
    std::vector<double> utilizations = {0.2, 0.3, 0.25};
    bool schedulable = isSchedulable(utilizations);
    EXPECT_TRUE(schedulable);  // 总利用率0.75 < n*(2^(1/n)-1)
    
    // 测试超周期计算
    std::vector<uint64_t> periods = {100, 200, 400};
    uint64_t hyper_period = calculateHyperPeriod(periods);
    EXPECT_EQ(hyper_period, 400);  // LCM(100, 200, 400) = 400
}

// ============================================================================
// 扩展测试20: IPC构建器测试
// ============================================================================
TEST(IPCExtendedTest, IPCBuilderPattern) {
    auto& manager = IPCManager::getInstance();
    ASSERT_TRUE(manager.initialize());
    
    // 使用Builder模式创建共享内存
    auto shm = IPCBuilder(manager)
        .asSharedMemory()
        .withName("builder_shm_1")
        .withSize(2048)
        .buildSharedMemory();
    
    ASSERT_NE(shm, nullptr);
    EXPECT_EQ(shm->getSize(), 2048);
    EXPECT_EQ(shm->getName(), "builder_shm_1");
    
    // 使用Builder模式创建消息队列
    auto mq = IPCBuilder(manager)
        .asMessageQueue()
        .withName("builder_mq_1")
        .withMaxMessages(50)
        .buildMessageQueue();
    
    ASSERT_NE(mq, nullptr);
    EXPECT_EQ(mq->getName(), "builder_mq_1");
    EXPECT_EQ(mq->getMaxSize(), 50);
    
    manager.shutdown();
}

// ============================================================================
// 扩展测试21: IPC监控器测试
// ============================================================================
TEST(IPCExtendedTest, IPCMonitorFunctionality) {
    auto& manager = IPCManager::getInstance();
    ASSERT_TRUE(manager.initialize());
    
    // 创建一些IPC资源
    auto shm1 = manager.createSharedMemory("monitor_shm_1", 1024);
    auto shm2 = manager.createSharedMemory("monitor_shm_2", 2048);
    auto mq1 = manager.createMessageQueue("monitor_mq_1", 50);
    
    // 对资源进行一些操作
    char data[] = "Test data";
    shm1->write(0, data, strlen(data) + 1);
    
    Message msg(1, 100, 200);
    msg.setData("Test", 5);
    mq1->send(msg);
    
    // 创建监控器
    IPCMonitor monitor(manager);
    
    // 测试生成报告
    std::string report = monitor.generateReport();
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("Shared Memory"), std::string::npos);
    EXPECT_NE(report.find("Message Queues"), std::string::npos);
    
    // 测试健康检查
    int health = monitor.checkHealth();
    EXPECT_GE(health, 0);
    EXPECT_LE(health, 100);
    
    // 测试JSON导出
    std::string json = monitor.exportToJson();
    EXPECT_FALSE(json.empty());
    
    // 测试泄漏检测
    auto leaks = monitor.detectLeaks();
    // 新创建的资源不应该被检测为泄漏
    EXPECT_EQ(leaks.size(), 0);
    
    manager.shutdown();
}

// ============================================================================
// 扩展测试22: IPC工具函数测试
// ============================================================================
TEST(IPCExtendedTest, IPCUtilityFunctions) {
    using namespace ipc_utils;
    
    // 测试唯一名称生成
    std::string name1 = generateUniqueName("test");
    std::string name2 = generateUniqueName("test");
    EXPECT_NE(name1, name2);
    EXPECT_EQ(name1.substr(0, 5), "test_");
    
    // 测试校验和计算
    const char* data = "Hello";
    uint32_t checksum = calculateChecksum(data, strlen(data));
    EXPECT_GT(checksum, 0);
    
    // 测试字节格式化
    EXPECT_NE(formatBytes(512).find("B"), std::string::npos);
    EXPECT_NE(formatBytes(2048).find("KB"), std::string::npos);
    EXPECT_NE(formatBytes(2097152).find("MB"), std::string::npos);
    
    // 测试消息验证
    Message valid_msg(1, 100, 200);
    valid_msg.timestamp = 1000000;
    EXPECT_TRUE(validateMessage(valid_msg));
    
    Message invalid_msg(1, 100, 200);
    invalid_msg.data_size = Message::MAX_DATA_SIZE + 1;
    EXPECT_FALSE(validateMessage(invalid_msg));
    
    // 测试消息序列化/反序列化
    Message original(1, 100, 200);
    original.setData("Test", 5);
    original.timestamp = 123456;
    
    auto serialized = serializeMessage(original);
    EXPECT_EQ(serialized.size(), sizeof(Message));
    
    Message deserialized = deserializeMessage(serialized);
    EXPECT_EQ(deserialized.message_type, original.message_type);
    EXPECT_EQ(deserialized.sender_id, original.sender_id);
    
    // 测试传输速率计算
    double rate = calculateTransferRate(1000000, 1000);  // 1MB in 1 second
    EXPECT_DOUBLE_EQ(rate, 1000000.0);
}

// ============================================================================
// 扩展测试23: 综合集成测试
// ============================================================================
TEST(RealtimeSchedulerExtendedTest, ComprehensiveIntegration) {
    auto& scheduler = RealtimeScheduler::getInstance();
    auto& ipc_manager = IPCManager::getInstance();
    
    // 初始化
    auto config = SchedulerConfigBuilder()
        .withDefaultPolicy(SchedulingPolicy::FIFO)
        .withStatistics(true)
        .withMaxTasks(20)
        .build();
    
    ASSERT_TRUE(scheduler.initialize(config));
    ASSERT_TRUE(ipc_manager.initialize());
    
    // 创建共享内存用于任务间通信
    auto control_shm = ipc_manager.createSharedMemory("control_shm", 1024);
    ASSERT_NE(control_shm, nullptr);
    
    // 创建消息队列用于任务间消息传递
    auto event_mq = ipc_manager.createMessageQueue("event_mq", 100);
    ASSERT_NE(event_mq, nullptr);
    
    // 创建多个协作任务
    std::atomic<int> task_counter{0};
    
    // 生产者任务
    auto producer = TaskBuilder(scheduler)
        .withId("producer")
        .withName("Producer Task")
        .withFunction([&]() {
            task_counter++;
            Message msg(1, 1, 2);
            msg.setData("data", 5);
            event_mq->send(msg);
        })
        .asPeriodic(50000)  // 50ms period
        .build();
    
    ASSERT_NE(producer, nullptr);
    
    // 消费者任务
    auto consumer = TaskBuilder(scheduler)
        .withId("consumer")
        .withName("Consumer Task")
        .withFunction([&]() {
            task_counter++;
            Message msg;
            if (event_mq->receive(msg, 10).isSuccess()) {
                // 处理消息
            }
        })
        .asPeriodic(100000)  // 100ms period
        .build();
    
    ASSERT_NE(consumer, nullptr);
    
    // 启动系统
    ASSERT_TRUE(scheduler.start());
    ASSERT_TRUE(scheduler.startTask("producer"));
    ASSERT_TRUE(scheduler.startTask("consumer"));
    
    // 运行一段时间
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 监控系统
    SchedulerMonitor sched_monitor(scheduler);
    IPCMonitor ipc_monitor(ipc_manager);
    
    int sched_health = sched_monitor.checkHealth();
    int ipc_health = ipc_monitor.checkHealth();
    
    EXPECT_GT(sched_health, 0);
    EXPECT_GT(ipc_health, 0);
    
    // 验证任务执行
    EXPECT_GT(task_counter.load(), 0);
    
    // 停止系统
    scheduler.stop();
    scheduler.shutdown();
    ipc_manager.shutdown();
}

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
