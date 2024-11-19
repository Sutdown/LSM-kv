cmake_minimum_required(VERSION 3.16)
project(lsmkv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


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
        src/cache/*.cc
        src/cache/*.h
        src/compress/*.cc
        src/compress/*.h
        src/config/*.cc
        src/config/*.h
        src/db/*.cc
        src/db/*.h
        src/file/*.cc
        src/file/*.h
        src/filter/*.cc
        src/filter/*.h
        src/log/*.cc
        src/log/*.h
        src/memory/*.cc
        src/memory/*.h
        src/table/*.cc
        src/table/*.h
        src/utils/*.cc
        src/utils/*.h
        src/main.cc)
add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE pthread nlohmann_json::nlohmann_json)

if (LSMKV-UNITTEST)
    enable_testing()
    file(GLOB_RECURSE SRC_WITHOUT_MAIN
    src/cache/*.cc
    src/cache/*.h
    src/compress/*.cc
    src/compress/*.h
    src/config/*.cc
    src/config/*.h
    src/db/*.cc
    src/db/*.h
    src/file/*.cc
    src/file/*.h
    src/filter/*.cc
    src/filter/*.h
    src/log/*.cc
    src/log/*.h
    src/memory/*.cc
    src/memory/*.h
    src/table/*.cc
    src/table/*.h
    src/utils/*.cc
    src/utils/*.h)
    file(GLOB_RECURSE SRC_TEST 
        tests/*.cpp)
    add_executable(lsmkv-unittest ${SRC_WITHOUT_MAIN} ${SRC_TEST})
    target_link_libraries(lsmkv-unittest PRIVATE gtest pthread nlohmann_json::nlohmann_json)
endif (LSMKV-UNITTEST)