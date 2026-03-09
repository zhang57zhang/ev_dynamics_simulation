/**
 * @file test_scheduler.cpp
 * @brief Scheduler模块基础测试套件
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

using namespace ev_dynamics::scheduler;

// ============================================================================
// 测试1: 实时调度器初始化与关闭测试
// ============================================================================
TEST(RealtimeSchedulerTest, InitializeAndShutdown) {
    auto& scheduler = RealtimeScheduler::getInstance();
    
    // 测试初始化
    EXPECT_TRUE(scheduler.initialize());
    
    // 测试重复初始化
    EXPECT_TRUE(scheduler.initialize());
    
    // 测试运行状态
    EXPECT_FALSE(scheduler.isRunning());
    
    // 测试关闭
    scheduler.shutdown();
}

// ============================================================================
// 测试2: 任务创建与删除测试
// ============================================================================
TEST(RealtimeSchedulerTest, CreateAndDeleteTask) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    // 创建简单任务
    std::atomic<int> counter{0};
    auto task = scheduler.createTask(
        "test_task_1",
        "Test Task 1",
        [&counter]() {
            counter++;
        },
        50,
        SchedulingPolicy::FIFO
    );
    
    ASSERT_NE(task, nullptr);
    EXPECT_EQ(task->task_id, "test_task_1");
    EXPECT_EQ(task->task_name, "Test Task 1");
    EXPECT_EQ(task->priority.base_priority, 50);
    EXPECT_EQ(task->policy, SchedulingPolicy::FIFO);
    EXPECT_EQ(task->state, TaskState::CREATED);
    
    // 测试获取任务
    auto retrieved_task = scheduler.getTask("test_task_1");
    EXPECT_EQ(retrieved_task, task);
    
    // 测试删除任务
    EXPECT_TRUE(scheduler.deleteTask("test_task_1"));
    EXPECT_EQ(scheduler.getTask("test_task_1"), nullptr);
    
    // 测试删除不存在的任务
    EXPECT_FALSE(scheduler.deleteTask("non_existent_task"));
    
    scheduler.shutdown();
}

// ============================================================================
// 测试3: 周期性任务测试
// ============================================================================
TEST(RealtimeSchedulerTest, PeriodicTaskExecution) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    // 创建周期性任务（100ms周期）
    std::atomic<int> execution_count{0};
    auto task = scheduler.createPeriodicTask(
        "periodic_task_1",
        "Periodic Task 1",
        [&execution_count]() {
            execution_count++;
        },
        100000,  // 100ms in microseconds
        100000,   // deadline = period
        50,       // priority
        50000     // WCET = 50ms
    );
    
    ASSERT_NE(task, nullptr);
    EXPECT_TRUE(task->is_periodic);
    EXPECT_EQ(task->period_us, 100000);
    EXPECT_EQ(task->deadline_us, 100000);
    EXPECT_EQ(task->wcet_us, 50000);
    
    // 启动调度器和任务
    ASSERT_TRUE(scheduler.start());
    EXPECT_TRUE(scheduler.startTask("periodic_task_1"));
    
    // 等待足够时间让任务执行多次（500ms）
    std::this_thread::sleep_for(std::chrono::milliseconds(550));
    
    // 停止调度器
    scheduler.stop();
    
    // 验证任务执行了约5次（允许±1次误差）
    EXPECT_GE(execution_count.load(), 4);
    EXPECT_LE(execution_count.load(), 6);
    
    // 验证统计信息
    auto stats = scheduler.getTaskStatistics("periodic_task_1");
    EXPECT_GE(stats.total_executions, 4);
    EXPECT_LE(stats.total_executions, 6);
    EXPECT_GT(stats.total_execution_time_us, 0);
    EXPECT_GT(stats.avg_execution_time_us, 0.0);
    
    scheduler.shutdown();
}

// ============================================================================
// 测试4: 优先级管理测试
// ============================================================================
TEST(RealtimeSchedulerTest, PriorityManagement) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    // 创建任务
    auto task = scheduler.createTask(
        "priority_test_task",
        "Priority Test Task",
        []() {},
        50,
        SchedulingPolicy::FIFO
    );
    
    ASSERT_NE(task, nullptr);
    EXPECT_EQ(task->priority.base_priority, 50);
    EXPECT_EQ(task->priority.current_priority, 50);
    
    // 测试设置优先级
    EXPECT_TRUE(scheduler.setTaskPriority("priority_test_task", 75));
    EXPECT_EQ(task->priority.base_priority, 75);
    EXPECT_EQ(task->priority.current_priority, 75);
    EXPECT_EQ(scheduler.getTaskPriority("priority_test_task"), 75);
    
    // 测试无效优先级（应该失败）
    EXPECT_FALSE(scheduler.setTaskPriority("priority_test_task", -1));
    EXPECT_FALSE(scheduler.setTaskPriority("priority_test_task", 100));
    
    // 测试优先级继承
    task->priority.inheritPriority(80);
    EXPECT_EQ(task->priority.current_priority, 80);
    EXPECT_TRUE(task->priority.is_inherited);
    
    // 恢复基础优先级
    task->priority.restoreBasePriority();
    EXPECT_EQ(task->priority.current_priority, 75);
    EXPECT_FALSE(task->priority.is_inherited);
    
    scheduler.shutdown();
}

// ============================================================================
// 测试5: 共享内存基础测试
// ============================================================================
TEST(IPCTest, SharedMemoryBasicOperations) {
    SharedMemoryConfig config("test_shared_memory", 4096);
    
    SharedMemory shm;
    
    // 测试创建
    auto result = shm.create(config);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(shm.isOpen());
    EXPECT_EQ(shm.getSize(), 4096);
    EXPECT_EQ(shm.getName(), "test_shared_memory");
    
    // 测试写入
    const char* test_data = "Hello, Shared Memory!";
    size_t data_size = strlen(test_data) + 1;
    auto write_result = shm.write(0, test_data, data_size);
    EXPECT_TRUE(write_result.isSuccess());
    EXPECT_EQ(write_result.value, data_size);
    
    // 测试读取
    char buffer[256];
    auto read_result = shm.read(0, buffer, sizeof(buffer));
    EXPECT_TRUE(read_result.isSuccess());
    EXPECT_EQ(read_result.value, data_size);
    EXPECT_STREQ(buffer, test_data);
    
    // 测试统计信息
    auto stats = shm.getStatistics();
    EXPECT_EQ(stats.total_size, 4096);
    EXPECT_EQ(stats.write_count, 1);
    EXPECT_EQ(stats.read_count, 1);
    EXPECT_EQ(stats.write_bytes, data_size);
    EXPECT_EQ(stats.read_bytes, data_size);
    
    // 测试关闭
    shm.close();
    EXPECT_FALSE(shm.isOpen());
    
    // 清理
    SharedMemory::unlink("test_shared_memory");
}

// ============================================================================
// 测试6: 消息队列基础测试
// ============================================================================
TEST(IPCTest, MessageQueueBasicOperations) {
    MessageQueueConfig config("test_message_queue");
    config.max_messages = 10;
    config.max_message_size = sizeof(Message);
    
    MessageQueue mq;
    
    // 测试创建
    auto result = mq.create(config);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(mq.isOpen());
    EXPECT_EQ(mq.getName(), "test_message_queue");
    EXPECT_EQ(mq.getMaxSize(), 10);
    EXPECT_TRUE(mq.isEmpty());
    EXPECT_FALSE(mq.isFull());
    
    // 创建消息
    Message send_msg(1, 100, 200);  // type=1, sender=100, receiver=200
    const char* msg_data = "Test Message Data";
    send_msg.setData(msg_data, strlen(msg_data) + 1);
    send_msg.timestamp = RealtimeScheduler::getCurrentTimeUs();
    
    // 测试发送
    auto send_result = mq.send(send_msg);
    EXPECT_TRUE(send_result.isSuccess());
    EXPECT_FALSE(mq.isEmpty());
    EXPECT_EQ(mq.getCurrentSize(), 1);
    
    // 测试接收
    Message recv_msg;
    auto recv_result = mq.receive(recv_msg);
    EXPECT_TRUE(recv_result.isSuccess());
    EXPECT_EQ(recv_msg.message_type, 1);
    EXPECT_EQ(recv_msg.sender_id, 100);
    EXPECT_EQ(recv_msg.receiver_id, 200);
    
    char recv_data[256];
    recv_msg.getData(recv_data, sizeof(recv_data));
    EXPECT_STREQ(recv_data, msg_data);
    
    // 测试统计信息
    auto stats = mq.getStatistics();
    EXPECT_EQ(stats.messages_sent, 1);
    EXPECT_EQ(stats.messages_received, 1);
    EXPECT_TRUE(mq.isEmpty());
    
    // 测试关闭
    mq.close();
    EXPECT_FALSE(mq.isOpen());
    
    // 清理
    MessageQueue::unlink("test_message_queue");
}

// ============================================================================
// 测试7: IPC管理器测试
// ============================================================================
TEST(IPCTest, IPCManagerOperations) {
    auto& manager = IPCManager::getInstance();
    
    // 初始化
    EXPECT_TRUE(manager.initialize());
    
    // 创建共享内存
    auto shm = manager.createSharedMemory("manager_test_shm", 1024);
    EXPECT_NE(shm, nullptr);
    EXPECT_NE(manager.getSharedMemory("manager_test_shm"), nullptr);
    
    // 创建消息队列
    auto mq = manager.createMessageQueue("manager_test_mq", 20);
    EXPECT_NE(mq, nullptr);
    EXPECT_NE(manager.getMessageQueue("manager_test_mq"), nullptr);
    
    // 测试获取所有名称
    auto shm_names = manager.getAllSharedMemoryNames();
    EXPECT_EQ(shm_names.size(), 1);
    EXPECT_EQ(shm_names[0], "manager_test_shm");
    
    auto mq_names = manager.getAllMessageQueueNames();
    EXPECT_EQ(mq_names.size(), 1);
    EXPECT_EQ(mq_names[0], "manager_test_mq");
    
    // 测试删除
    EXPECT_TRUE(manager.deleteSharedMemory("manager_test_shm"));
    EXPECT_EQ(manager.getSharedMemory("manager_test_shm"), nullptr);
    
    EXPECT_TRUE(manager.deleteMessageQueue("manager_test_mq"));
    EXPECT_EQ(manager.getMessageQueue("manager_test_mq"), nullptr);
    
    // 测试性能统计
    std::string stats = manager.getPerformanceStats();
    EXPECT_FALSE(stats.empty());
    EXPECT_NE(stats.find("shared_memories"), std::string::npos);
    EXPECT_NE(stats.find("message_queues"), std::string::npos);
    
    manager.shutdown();
}

// ============================================================================
// 测试8: 任务状态转换测试
// ============================================================================
TEST(RealtimeSchedulerTest, TaskStateTransitions) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    std::atomic<bool> task_started{false};
    std::atomic<bool> task_completed{false};
    
    auto task = scheduler.createTask(
        "state_test_task",
        "State Test Task",
        [&task_started, &task_completed]() {
            task_started = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            task_completed = true;
        },
        50,
        SchedulingPolicy::FIFO
    );
    
    ASSERT_NE(task, nullptr);
    EXPECT_EQ(task->state, TaskState::CREATED);
    
    // 启动任务
    ASSERT_TRUE(scheduler.start());
    EXPECT_TRUE(scheduler.startTask("state_test_task"));
    
    // 等待任务开始
    while (!task_started.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // 挂起任务
    EXPECT_TRUE(scheduler.suspendTask("state_test_task"));
    EXPECT_EQ(task->state, TaskState::SUSPENDED);
    
    // 恢复任务
    EXPECT_TRUE(scheduler.resumeTask("state_test_task"));
    EXPECT_EQ(task->state, TaskState::READY);
    
    // 等待任务完成
    while (!task_completed.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    scheduler.stop();
    scheduler.shutdown();
}

// ============================================================================
// 测试9: 调度器统计信息测试
// ============================================================================
TEST(RealtimeSchedulerTest, SchedulerStatistics) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    // 创建多个任务
    for (int i = 0; i < 5; i++) {
        std::string task_id = "stats_task_" + std::to_string(i);
        scheduler.createTask(
            task_id,
            "Stats Task " + std::to_string(i),
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(10)); },
            50 + i * 5,
            SchedulingPolicy::FIFO
        );
    }
    
    // 获取调度器统计
    auto stats = scheduler.getSchedulerStatistics();
    EXPECT_EQ(stats.task_creations, 5);
    EXPECT_EQ(stats.start_time.time_since_epoch().count(), 0);
    
    // 获取所有任务ID
    auto task_ids = scheduler.getAllTaskIds();
    EXPECT_EQ(task_ids.size(), 5);
    
    // 重置统计
    scheduler.resetStatistics();
    stats = scheduler.getSchedulerStatistics();
    EXPECT_EQ(stats.task_creations, 0);
    
    scheduler.shutdown();
}

// ============================================================================
// 测试10: 错误处理测试
// ============================================================================
TEST(RealtimeSchedulerTest, ErrorHandling) {
    auto& scheduler = RealtimeScheduler::getInstance();
    
    // 测试未初始化时的操作
    EXPECT_FALSE(scheduler.start());
    
    ASSERT_TRUE(scheduler.initialize());
    
    // 测试创建重复任务
    scheduler.createTask("duplicate_task", "Duplicate Task", [](){}, 50);
    auto duplicate = scheduler.createTask("duplicate_task", "Duplicate Task 2", [](){}, 50);
    EXPECT_EQ(duplicate, nullptr);
    
    // 测试操作不存在的任务
    EXPECT_FALSE(scheduler.startTask("non_existent"));
    EXPECT_FALSE(scheduler.suspendTask("non_existent"));
    EXPECT_EQ(scheduler.getTaskPriority("non_existent"), -1);
    
    // 测试无效参数
    TaskPriority priority(50);
    EXPECT_THROW(priority.setBasePriority(-1), std::invalid_argument);
    EXPECT_THROW(priority.setBasePriority(100), std::invalid_argument);
    
    scheduler.shutdown();
}

// ============================================================================
// 测试11: 时间相关功能测试
// ============================================================================
TEST(RealtimeSchedulerTest, TimeFunctions) {
    // 测试获取当前时间
    uint64_t time1 = RealtimeScheduler::getCurrentTimeUs();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    uint64_t time2 = RealtimeScheduler::getCurrentTimeUs();
    
    // 验证时间差约为100ms（允许±20ms误差）
    uint64_t diff = time2 - time1;
    EXPECT_GE(diff, 80000);   // 80ms
    EXPECT_LE(diff, 120000);  // 120ms
    
    // 测试睡眠函数
    uint64_t start = RealtimeScheduler::getCurrentTimeUs();
    RealtimeScheduler::sleepFor(50000);  // 50ms
    uint64_t end = RealtimeScheduler::getCurrentTimeUs();
    
    diff = end - start;
    EXPECT_GE(diff, 45000);   // 45ms
    EXPECT_LE(diff, 60000);   // 60ms
}

// ============================================================================
// 测试12: 高负载场景测试
// ============================================================================
TEST(RealtimeSchedulerTest, HighLoadScenario) {
    auto& scheduler = RealtimeScheduler::getInstance();
    ASSERT_TRUE(scheduler.initialize());
    
    const int NUM_TASKS = 20;
    std::atomic<int> total_executions{0};
    
    // 创建大量周期性任务
    for (int i = 0; i < NUM_TASKS; i++) {
        std::string task_id = "load_task_" + std::to_string(i);
        scheduler.createPeriodicTask(
            task_id,
            "Load Task " + std::to_string(i),
            [&total_executions]() {
                total_executions++;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            },
            50000 + i * 1000,  // 50ms + i*1ms period
            50000 + i * 1000,  // deadline = period
            50,                // priority
            25000              // WCET = 25ms
        );
    }
    
    // 启动调度器
    ASSERT_TRUE(scheduler.start());
    
    // 启动所有任务
    for (int i = 0; i < NUM_TASKS; i++) {
        std::string task_id = "load_task_" + std::to_string(i);
        EXPECT_TRUE(scheduler.startTask(task_id));
    }
    
    // 运行1秒
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 停止调度器
    scheduler.stop();
    
    // 验证总执行次数（每个任务应该执行约20次）
    EXPECT_GT(total_executions.load(), NUM_TASKS * 15);
    
    // 验证统计信息
    auto stats = scheduler.getSchedulerStatistics();
    EXPECT_GT(stats.total_schedules, 0);
    EXPECT_GT(stats.context_switches, 0);
    
    scheduler.shutdown();
}

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
