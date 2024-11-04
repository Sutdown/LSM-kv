cmake_minimum_required(VERSION 3.16)
project(SingletonLoggerTest)

# 设置 C++ 版本
set(CMAKE_CXX_STANDARD 17)

# 查找 spdlog 包
find_package(spdlog REQUIRED)

# 添加源文件
add_executable(SingletonLoggerTest main.cpp logger.cpp)

# 链接 spdlog
target_link_libraries(SingletonLoggerTest spdlog::spdlog)
