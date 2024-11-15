cmake_minimum_required(VERSION 3.16)
project(lsmkv)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-march=native -ffast-math)
set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_CXX_FLAGS "-Werror -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

option(LSMKV-UNITTEST "build unit-test" ON)

# gtest
find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

# spdlog
find_package(spdlog REQUIRED)
include_directories(${spdlog_INCLUDE_DIRS})

# nlohmann_json
find_package(nlohmann_json 3.2.0 REQUIRED)

include_directories(src)

file(GLOB_RECURSE SRC
        src/cache/*.cpp
        src/cache/*.h
        src/db/*.h
        src/file/*.cpp
        src/file/*.h
        src/config/*.cpp
        src/config/*.h
        src/filter/*.cpp
        src/filter/*.h
        src/log/*.cpp
        src/log/*.h
        src/memory/*.cpp
        src/memory/*.h
        src/utils/*.cpp
        src/utils/*.h
        src/main.cpp)
add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE pthread nlohmann_json::nlohmann_json)

if (LSMKV-UNITTEST)
    enable_testing()
    file(GLOB_RECURSE SRC_WITHOUT_MAIN
        src/cache/*.cpp
        src/cache/*.h
        src/db/*.h
        src/file/*.cpp
        src/file/*.h
        src/config/*.cpp
        src/config/*.h
        src/filter/*.cpp
        src/filter/*.h
        src/log/*.cpp
        src/log/*.h
        src/memory/*.cpp
        src/memory/*.h
        src/utils/*.cpp
        src/utils/*.h)
    file(GLOB_RECURSE SRC_TEST 
        tests/*.cpp)
    add_executable(lsmkv-unittest ${SRC_WITHOUT_MAIN} ${SRC_TEST})
    target_link_libraries(lsmkv-unittest PRIVATE gtest pthread nlohmann_json::nlohmann_json)
endif (LSMKV-UNITTEST)