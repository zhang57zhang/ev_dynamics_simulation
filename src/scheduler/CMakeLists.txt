# scheduler/CMakeLists.txt
# 实时调度模块

message(STATUS "Configuring scheduler module...")

# ============================================================================
# C++库配置
# ============================================================================

# 收集C++源文件
set(SCHEDULER_CPP_SOURCES
    realtime_scheduler.cpp
    ipc_manager.cpp
)

# 创建静态库
add_library(ev_scheduler STATIC ${SCHEDULER_CPP_SOURCES})

# 设置包含目录
target_include_directories(ev_scheduler
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# 设置C++标准
target_compile_features(ev_scheduler PUBLIC cxx_std_17)

# 编译选项
if(MSVC)
    target_compile_options(ev_scheduler PRIVATE /W4 /WX)
else()
    target_compile_options(ev_scheduler PRIVATE -Wall -Wextra -Werror -pedantic)
    # POSIX线程库
    find_package(Threads REQUIRED)
    target_link_libraries(ev_scheduler PUBLIC Threads::Threads)
    # RT库（用于实时功能）
    target_link_libraries(ev_scheduler PUBLIC rt)
endif()

# ============================================================================
# Python绑定（可选）
# ============================================================================

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

# 创建Python绑定模块（如果Python可用）
find_package(Python3 QUIET COMPONENTS Interpreter Development)
if(Python3_FOUND)
    message(STATUS "Python3 found, building scheduler Python bindings")
    add_python_module(scheduler ${SCHEDULER_SOURCES})
    
    target_include_directories(scheduler PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/realtime_scheduler
        ${CMAKE_CURRENT_SOURCE_DIR}/task_manager
        ${CMAKE_CURRENT_SOURCE_DIR}/monitor
    )
else()
    message(STATUS "Python3 not found, skipping scheduler Python bindings")
endif()

message(STATUS "Scheduler module configured")
