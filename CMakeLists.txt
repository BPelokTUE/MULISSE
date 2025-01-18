cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0167 NEW)
project(ulisse_mts)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CLI11_PRECOMPILED ON)

# Use debug build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Add debug symbols for debug build
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Add optimizations for release build
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/lib)

# Add CLI11 subdirectory
add_subdirectory(extern/CLI11)

# Add Boost
find_package(Boost 1.87 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Add main executable
file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE CLI11::CLI11)

# Add doctest submodule
set(DOCTEST_LIB extern/doctest/doctest)
set(doctest "${DOCTEST_LIB}/doctest.h")
add_library(doctest INTERFACE)
include_directories(${DOCTEST_LIB})

# Gather test files
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp src/**/*.cpp)

# Add test executable
set(TEST_NAME ${PROJECT_NAME}_tests)
add_executable(${TEST_NAME} ${TEST_SOURCES})
target_link_libraries(${TEST_NAME} PRIVATE CLI11::CLI11)
target_link_libraries(${TEST_NAME} PRIVATE doctest)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
