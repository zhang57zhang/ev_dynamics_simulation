# src/CMakeLists.txt
# 源代码顶层CMake配置

message(STATUS "Configuring source directories...")

# 各个子模块由顶层CMakeLists.txt直接添加
# 这里仅用于通用配置

# 设置源代码包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
