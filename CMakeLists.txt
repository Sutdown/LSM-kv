cmake_minimum_required(VERSION 3.16)
project(lsmkv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

option(LSMKV-UNITTEST "build unit-test" ON)

# 查找源文件
file(GLOB_RECURSE SRC_FILES src/filter/*.cpp src/log/*.cpp src/utils/*.cpp)

# 添加可执行文件（主程序）
add_executable(${PROJECT_NAME} ${SRC_FILES})

# gtest
find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

# spdlog
find_package(spdlog REQUIRED)
include_directories(${spdlog_INCLUDE_DIRS})

# 查找并链接 pthread 库
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

if (LSMKV-UNITTEST)
    enable_testing()
    file(GLOB_RECURSE SRC_TEST tests/*.cpp)
    find_package(GTest REQUIRED)
    add_executable(lsmkv-unitest ${SRC_WITHOUT_MAIN} ${SRC_TEST})
    target_link_libraries(smallkv-unitest libgtest.so)
endif (LSMKV-UNITTEST)