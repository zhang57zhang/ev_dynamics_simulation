# Chassis模块CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# 查找Eigen3
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# 源文件
set(CHASSIS_SOURCES
    suspension.cpp
    steering.cpp
    braking.cpp
)

# 头文件
set(CHASSIS_HEADERS
    suspension.h
    steering.h
    braking.h
)

# 创建静态库
add_library(chassis STATIC ${CHASSIS_SOURCES} ${CHASSIS_HEADERS})

# 设置包含目录
target_include_directories(chassis
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${EIGEN3_INCLUDE_DIR}
)

# 链接依赖
target_link_libraries(chassis
    PUBLIC
        Eigen3::Eigen
)

# 编译选项
if(MSVC)
    target_compile_options(chassis PRIVATE /W4)
else()
    target_compile_options(chassis PRIVATE -Wall -Wextra -Wpedantic)
endif()

# C++17标准
target_compile_features(chassis PUBLIC cxx_std_17)

# Python绑定（可选）
if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 REQUIRED)
    
    pybind11_add_module(chassis_binding chassis_binding.cpp)
    
    target_link_libraries(chassis_binding
        PRIVATE
            chassis
    )
    
    target_include_directories(chassis_binding
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# 安装
install(TARGETS chassis
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${CHASSIS_HEADERS}
    DESTINATION include/chassis
)
