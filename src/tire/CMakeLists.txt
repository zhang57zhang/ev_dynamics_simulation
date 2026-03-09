# tire/CMakeLists.txt
# 轮胎模型模块

message(STATUS "Configuring tire module...")

# 收集所有源文件
set(TIRE_SOURCES
    pacejka_model.cpp
    tire_dynamics.cpp
    tire_binding.cpp
)

# 创建静态库
add_library(tire STATIC ${TIRE_SOURCES})

# 设置C++17标准
target_compile_features(tire PUBLIC cxx_std_17)

# 包含目录
target_include_directories(tire PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接Eigen3
target_link_libraries(tire PUBLIC Eigen3::Eigen)

# 编译选项
if(MSVC)
    target_compile_options(tire PRIVATE /W4 /WX)
else()
    target_compile_options(tire PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

# 创建Python绑定模块（可选）
if(PYTHON_BINDINGS)
    add_python_module(tire_binding tire_binding.cpp)
    target_link_libraries(tire_binding PRIVATE tire)
    target_include_directories(tire_binding PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

message(STATUS "Tire module configured")
