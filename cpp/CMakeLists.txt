cmake_minimum_required(VERSION 3.20)
project(semester2_lab3 LANGUAGES CXX)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(structures
    src/array.cpp
    src/single_list.cpp
    src/double_list.cpp
    src/queue.cpp
    src/stack.cpp
    src/avl_tree.cpp
    src/hash_table.cpp
    src/persistence.cpp)
target_include_directories(structures PUBLIC src)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(structures PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(LAB3_COVERAGE "Instrument the structures library for GCC coverage" OFF)
if(LAB3_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "LAB3_COVERAGE requires GCC")
    endif()
    target_compile_options(structures PRIVATE -O0 -g --coverage)
    target_link_options(structures INTERFACE --coverage)
endif()

include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        URL https://codeload.github.com/google/googletest/zip/refs/tags/v1.17.0)
    FetchContent_MakeAvailable(googletest)
    add_executable(structures_tests tests/structures_test.cpp tests/persistence_test.cpp)
    target_link_libraries(structures_tests PRIVATE structures GTest::gtest_main)
    add_test(NAME structures_tests COMMAND structures_tests)
endif()

add_executable(structures_bench tests/benchmark.cpp)
target_link_libraries(structures_bench PRIVATE structures)
