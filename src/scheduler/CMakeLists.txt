# scheduler/CMakeLists.txt
# 实时调度模块

message(STATUS "Configuring scheduler module...")

# 子模块目录
set(SCHEDULER_SUBMODULES
    realtime_scheduler
    task_manager
    monitor
)

# 收集所有源文件
set(SCHEDULER_SOURCES
    scheduler_binding.cpp
)

# 创建Python绑定模块
add_python_module(scheduler ${SCHEDULER_SOURCES})

# 设置模块包含目录
target_include_directories(scheduler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/realtime_scheduler
    ${CMAKE_CURRENT_SOURCE_DIR}/task_manager
    ${CMAKE_CURRENT_SOURCE_DIR}/monitor
)

message(STATUS "Scheduler module configured")
