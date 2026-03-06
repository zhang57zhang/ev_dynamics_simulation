# dynamics/CMakeLists.txt
# 车辆动力学模块

message(STATUS "Configuring dynamics module...")

# 子模块目录
set(DYNAMICS_SUBMODULES
    vehicle_dynamics
    rk4_integrator
    state_space
)

# 收集所有源文件
set(DYNAMICS_SOURCES
    dynamics_binding.cpp
    vehicle_dynamics.cpp
    rk4_integrator.cpp
    state_space.cpp
)

# 创建Python绑定模块
add_python_module(dynamics ${DYNAMICS_SOURCES})

# 设置模块包含目录
target_include_directories(dynamics PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vehicle_dynamics
    ${CMAKE_CURRENT_SOURCE_DIR}/rk4_integrator
    ${CMAKE_CURRENT_SOURCE_DIR}/state_space
)

# 链接Eigen3（如果需要）
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
target_link_libraries(dynamics PRIVATE Eigen3::Eigen)

message(STATUS "Dynamics module configured")
