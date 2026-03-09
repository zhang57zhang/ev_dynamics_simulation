# 依赖配置
# EV Dynamics Simulation Project

include(FetchContent)

# ============================================================================
# Eigen3 - 线性代数库
# ============================================================================
# 首先尝试在本地third_party目录查找
set(EIGEN3_LOCAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/eigen3")

if(EXISTS "${EIGEN3_LOCAL_PATH}/Eigen")
    message(STATUS "Using local Eigen3 from third_party directory")
    set(EIGEN3_INCLUDE_DIR "${EIGEN3_LOCAL_PATH}")
    add_library(Eigen3::Eigen INTERFACE IMPORTED)
    target_include_directories(Eigen3::Eigen INTERFACE "${EIGEN3_INCLUDE_DIR}")
else()
    # 尝试系统安装
    find_package(Eigen3 3.3 QUIET)
    
    if(NOT Eigen3_FOUND)
        message(STATUS "Eigen3 not found, fetching from GitHub...")
        FetchContent_Declare(
            Eigen3
            GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
            GIT_TAG 3.4.0
        )
        FetchContent_MakeAvailable(Eigen3)
        message(STATUS "Eigen3 fetched and configured")
    else()
        message(STATUS "Eigen3 found: ${Eigen3_VERSION}")
        message(STATUS "Eigen3 include dir: ${EIGEN3_INCLUDE_DIR}")
    endif()
endif()

# ============================================================================
# pybind11 - Python绑定（可选）
# ============================================================================
option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)

if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 QUIET)

    if(NOT pybind11_FOUND)
        message(STATUS "pybind11 not found, Python bindings disabled")
        message(STATUS "Install pybind11 to enable Python bindings: pip install pybind11")
        set(BUILD_PYTHON_BINDINGS OFF)
    else()
        message(STATUS "pybind11 found: ${pybind11_VERSION}")
        message(STATUS "Python include dirs: ${PYTHON_INCLUDE_DIRS}")
        message(STATUS "Python libraries: ${PYTHON_LIBRARIES}")
    endif()
endif()

# ============================================================================
# Google Test - 单元测试（可选）
# ============================================================================
option(BUILD_TESTING "Build tests" OFF)

if(BUILD_TESTING)
    find_package(GTest QUIET)

    if(NOT GTest_FOUND)
        message(STATUS "GTest not found, tests disabled")
        message(STATUS "Install GTest to enable tests")
        set(BUILD_TESTING OFF)
    else()
        message(STATUS "GTest found: ${GTest_VERSION}")
        message(STATUS "GTest include dirs: ${GTEST_INCLUDE_DIRS}")
        message(STATUS "GTest libraries: ${GTEST_LIBRARIES}")
        enable_testing()
    endif()
endif()

# ============================================================================
# 包含目录
# ============================================================================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${EIGEN3_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
)

if(GTest_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
endif()

# ============================================================================
# 链接目录
# ============================================================================
link_directories(
    ${CMAKE_BINARY_DIR}/lib
)

# ============================================================================
# 辅助函数：创建Python模块
# ============================================================================
function(add_python_module MODULE_NAME SOURCES)
    if(NOT BUILD_PYTHON_BINDINGS)
        message(STATUS "Python module ${MODULE_NAME} skipped (Python bindings disabled)")
        return()
    endif()
    
    if(NOT pybind11_FOUND)
        message(WARNING "Cannot create Python module ${MODULE_NAME}: pybind11 not found")
        return()
    endif()
    
    # 创建模块
    pybind11_add_module(${MODULE_NAME} MODULE ${SOURCES})
    
    # 设置输出目录
    set_target_properties(${MODULE_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python_modules
    )
    
    # 链接依赖
    target_link_libraries(${MODULE_NAME} PRIVATE
        Eigen3::Eigen
    )
    
    # 编译选项
    if(MSVC)
        target_compile_options(${MODULE_NAME} PRIVATE /bigobj)
    endif()
    
    message(STATUS "Python module configured: ${MODULE_NAME}")
endfunction()

# ============================================================================
# 辅助函数：创建库
# ============================================================================
function(add_ev_library LIB_NAME SOURCES)
    # 创建静态库
    add_library(${LIB_NAME} STATIC ${SOURCES})
    
    # 设置包含目录
    target_include_directories(${LIB_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${EIGEN3_INCLUDE_DIR}
    )
    
    # 编译选项
    if(MSVC)
        target_compile_options(${LIB_NAME} PRIVATE /W4)
    else()
        target_compile_options(${LIB_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    
    message(STATUS "Library configured: ${LIB_NAME}")
endfunction()

# ============================================================================
# 辅助函数：创建测试
# ============================================================================
function(add_ev_test TEST_NAME SOURCES)
    if(NOT BUILD_TESTING)
        return()
    endif()
    
    # 创建可执行文件
    add_executable(${TEST_NAME} ${SOURCES})
    
    # 链接GTest
    target_link_libraries(${TEST_NAME} PRIVATE
        GTest::GTest
        GTest::Main
    )
    
    # 添加测试
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
    message(STATUS "Test configured: ${TEST_NAME}")
endfunction()

# ============================================================================
# 版本信息
# ============================================================================
message(STATUS "")
message(STATUS "=== Dependencies Summary ===")
message(STATUS "Eigen3: ${Eigen3_VERSION}")
message(STATUS "pybind11: ${pybind11_VERSION}")
if(GTest_FOUND)
    message(STATUS "GTest: ${GTest_VERSION}")
else()
    message(STATUS "GTest: NOT FOUND (tests disabled)")
endif()
message(STATUS "Python: ${PYTHON_VERSION_STRING}")
message(STATUS "============================")
message(STATUS "")
