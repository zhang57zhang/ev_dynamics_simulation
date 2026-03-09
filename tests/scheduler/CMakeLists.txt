# tests/scheduler/CMakeLists.txt
# Scheduler模块测试

message(STATUS "Configuring scheduler tests...")

# 查找GTest
find_package(GTest REQUIRED)

# ============================================================================
# 基础测试
# ============================================================================

add_executable(test_scheduler
    test_scheduler.cpp
)

target_link_libraries(test_scheduler
    PRIVATE
        ev_scheduler
        GTest::gtest
        GTest::gtest_main
)

if(NOT MSVC)
    find_package(Threads REQUIRED)
    target_link_libraries(test_scheduler PRIVATE Threads::Threads)
endif()

# 添加测试
add_test(NAME SchedulerBasicTests COMMAND test_scheduler)
set_tests_properties(SchedulerBasicTests PROPERTIES
    LABELS "scheduler;unit"
    TIMEOUT 60
)

# ============================================================================
# 扩展测试
# ============================================================================

add_executable(test_scheduler_extended
    test_scheduler_extended.cpp
)

target_link_libraries(test_scheduler_extended
    PRIVATE
        ev_scheduler
        GTest::gtest
        GTest::gtest_main
)

if(NOT MSVC)
    target_link_libraries(test_scheduler_extended PRIVATE Threads::Threads)
endif()

# 添加测试
add_test(NAME SchedulerExtendedTests COMMAND test_scheduler_extended)
set_tests_properties(SchedulerExtendedTests PROPERTIES
    LABELS "scheduler;extended;performance"
    TIMEOUT 120
)

# ============================================================================
# 代码覆盖率配置（可选）
# ============================================================================

option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE AND NOT MSVC)
    message(STATUS "Enabling code coverage for scheduler tests")
    
    # 添加覆盖率标志
    target_compile_options(test_scheduler PRIVATE
        --coverage -fprofile-arcs -ftest-coverage
    )
    target_compile_options(test_scheduler_extended PRIVATE
        --coverage -fprofile-arcs -ftest-coverage
    )
    
    target_link_libraries(test_scheduler PRIVATE --coverage)
    target_link_libraries(test_scheduler_extended PRIVATE --coverage)
    
    # 添加覆盖率目标
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage_scheduler
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' --output-file coverage.info.cleaned
            COMMAND ${GENHTML_PATH} -o coverage_report coverage.info.cleaned
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in coverage_report/"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS test_scheduler test_scheduler_extended
            COMMENT "Generating code coverage report for scheduler module"
        )
    endif()
endif()

message(STATUS "Scheduler tests configured")
