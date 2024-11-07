cmake_minimum_required(VERSION 3.16)
project(lsmkv LANGUAGES CXX)

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

file(GLOB_RECURSE SRC src/*.cc)
add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE pthread nlohmann_json::nlohmann_json)

if (LSMKV-UNITTEST)
    enable_testing()
    file(GLOB_RECURSE SRC_WITHOUT_MAIN
            src/filter/*.cpp
            src/log/*.cpp
            src/memory/*.cpp
            src/utils/*.cpp)
    file(GLOB_RECURSE SRC_TEST tests/*.cpp)
    add_executable(lsmkv-unitest ${SRC_WITHOUT_MAIN} ${SRC_TEST})
    target_link_libraries(lsmkv-unitest PRIVATE gtest pthread nlohmann_json::nlohmann_json)
endif (LSMKV-UNITTEST)