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

# Include header files
include_directories(${CMAKE_SOURCE_DIR}/lib)
# Include external header files
# - CLI11: 2.4.2:    https://github.com/CLIUtils/CLI11/releases
# - doctest: 2.4.11: https://github.com/doctest/doctest/tree/master/doctest/doctest.h
# - Fake-it 2.4.1:   https://github.com/eranpeer/FakeIt/tree/master/single_header/doctest/fakeit.h 
# - cereal: 1.3.2    https://github.com/USCiLab/cereal
include_directories(${CMAKE_SOURCE_DIR}/extern/doctest) # Required for fake-it
include_directories(${CMAKE_SOURCE_DIR}/extern)

# Add Boost
find_package(Boost 1.87 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Add main executable
file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

# Gather test files
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp src/**/*.cpp)

# Add test executable
set(TEST_NAME ${PROJECT_NAME}_tests)
add_executable(${TEST_NAME} ${TEST_SOURCES})
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
