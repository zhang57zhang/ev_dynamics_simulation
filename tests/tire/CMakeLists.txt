# tests/tire/CMakeLists.txt
# 轮胎模型测试

message(STATUS "Configuring tire tests...")

# 收集测试源文件
set(TIRE_TEST_SOURCES
    test_tire.cpp
    test_tire_extended.cpp
)

# 创建测试可执行文件
add_executable(test_tire ${TIRE_TEST_SOURCES})

# 链接库
target_link_libraries(test_tire
    PRIVATE
    tire
    gtest
    gtest_main
    Eigen3::Eigen
)

# 包含目录
target_include_directories(test_tire PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# C++17标准
target_compile_features(test_tire PUBLIC cxx_std_17)

# 添加测试
add_test(NAME TireModelTests COMMAND test_tire)

# 设置测试属性
set_tests_properties(TireModelTests PROPERTIES
    TIMEOUT 60
    LABELS "tire;unit"
)

message(STATUS "Tire tests configured")
