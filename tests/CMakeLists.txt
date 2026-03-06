# tests/CMakeLists.txt
# EV动力学仿真系统测试

message(STATUS "Configuring tests...")

# 查找GTest
find_package(GTest QUIET)

if(GTest_FOUND)
    message(STATUS "GTest found, enabling C++ tests")
    
    # 添加dynamics测试子目录
    add_subdirectory(dynamics)
else()
    message(STATUS "GTest not found, skipping C++ tests")
    message(STATUS "Install GTest to enable C++ unit tests")
endif()

# Python测试（始终可用）
find_package(Python3 QUIET COMPONENTS Interpreter)

if(Python3_FOUND)
    message(STATUS "Python3 found, Python tests available")
    
    # 添加Python测试（使用add_test）
    add_test(NAME python_tests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_all_tests.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(python_tests PROPERTIES
        LABELS "python"
    )
endif()

message(STATUS "Tests configured")
