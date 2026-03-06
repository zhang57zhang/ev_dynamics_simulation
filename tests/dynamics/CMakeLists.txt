# tests/dynamics/CMakeLists.txt
# 车辆动力学测试

message(STATUS "Configuring dynamics tests...")

# 查找依赖
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(GTest REQUIRED)

# 集成测试
add_executable(test_dynamics_integration
    test_integration.cpp
)

target_include_directories(test_dynamics_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/dynamics
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_dynamics_integration PRIVATE
    Eigen3::Eigen
    GTest::gtest
    GTest::gtest_main
)

# 设置C++标准
target_compile_features(test_dynamics_integration PRIVATE cxx_std_17)

# 添加测试
include(GoogleTest)
gtest_discover_tests(test_dynamics_integration
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTIES
        LABELS "dynamics"
)

message(STATUS "Dynamics tests configured")
