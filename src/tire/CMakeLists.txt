# tire/CMakeLists.txt
# 轮胎模型模块

message(STATUS "Configuring tire module...")

# 子模块目录
set(TIRE_SUBMODULES
    pacejka
    slip_calculator
)

# 收集所有源文件
set(TIRE_SOURCES
    tire_binding.cpp
)

# 创建Python绑定模块
add_python_module(tire ${TIRE_SOURCES})

# 设置模块包含目录
target_include_directories(tire PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/pacejka
    ${CMAKE_CURRENT_SOURCE_DIR}/slip_calculator
)

message(STATUS "Tire module configured")
