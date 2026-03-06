# powertrain/CMakeLists.txt
# 动力系统模块

message(STATUS "Configuring powertrain module...")

# 子模块目录
set(POWERTRAIN_SUBMODULES
    motor
    battery
    transmission
)

# 收集所有源文件
set(POWERTRAIN_SOURCES
    powertrain_binding.cpp
)

# 创建Python绑定模块
add_python_module(powertrain ${POWERTRAIN_SOURCES})

# 设置模块包含目录
target_include_directories(powertrain PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/motor
    ${CMAKE_CURRENT_SOURCE_DIR}/battery
    ${CMAKE_CURRENT_SOURCE_DIR}/transmission
)

# 子模块的源文件（未来实现）
# foreach(submodule ${POWERTRAIN_SUBMODULES})
#     add_subdirectory(${submodule})
# endforeach()

message(STATUS "Powertrain module configured")
