# Chassis测试CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# 基础测试
add_executable(test_chassis
    test_chassis.cpp
)

target_link_libraries(test_chassis
    PRIVATE
        chassis
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(test_chassis
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/chassis
)

# 扩展测试
add_executable(test_chassis_extended
    test_chassis_extended.cpp
)

target_link_libraries(test_chassis_extended
    PRIVATE
        chassis
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(test_chassis_extended
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/chassis
)

# 添加测试
add_test(NAME ChassisBasicTests COMMAND test_chassis)
add_test(NAME ChassisExtendedTests COMMAND test_chassis_extended)

# 设置测试标签
set_tests_properties(ChassisBasicTests PROPERTIES
    LABELS "chassis;unit"
)

set_tests_properties(ChassisExtendedTests PROPERTIES
    LABELS "chassis;unit;extended"
)

# 内存检查（如果可用）
if(MEMORYCHECK_COMMAND)
    set_tests_properties(ChassisBasicTests ChassisExtendedTests
        PROPERTIES
        MEMORYCHECK_TYPE "AddressSanitizer"
    )
endif()
