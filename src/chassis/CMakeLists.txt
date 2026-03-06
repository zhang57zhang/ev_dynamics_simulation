# chassis/CMakeLists.txt
# 底盘系统模块

message(STATUS "Configuring chassis module...")

# 子模块目录
set(CHASSIS_SUBMODULES
    suspension
    steering
    braking
)

# 收集所有源文件
set(CHASSIS_SOURCES
    chassis_binding.cpp
)

# 创建Python绑定模块
add_python_module(chassis ${CHASSIS_SOURCES})

# 设置模块包含目录
target_include_directories(chassis PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/suspension
    ${CMAKE_CURRENT_SOURCE_DIR}/steering
    ${CMAKE_CURRENT_SOURCE_DIR}/braking
)

message(STATUS "Chassis module configured")
