cmake_minimum_required(VERSION 3.16)
project(ev_dynamics_simulation VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 包含依赖配置
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 子目录
add_subdirectory(src/powertrain)
add_subdirectory(src/chassis)
add_subdirectory(src/dynamics)
add_subdirectory(src/tire)
add_subdirectory(src/scheduler)

# 测试
enable_testing()
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# 安装
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY config/ DESTINATION config)

# 打印配置信息
message(STATUS "=== EV Dynamics Simulation Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
