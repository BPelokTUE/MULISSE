cmake_minimum_required(VERSION 3.10)
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
project(mulisse)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ccache)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()
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
# - CLI11: 2.4.2:       https://github.com/CLIUtils/CLI11/releases
# - doctest: 2.4.11:    https://github.com/doctest/doctest/tree/master/doctest/doctest.h
# - Fake-it 2.4.1:      https://github.com/eranpeer/FakeIt/tree/master/single_header/doctest/fakeit.h 
# - cereal: 1.3.2       https://github.com/USCiLab/cereal
# - magic_enum: 0.9.7   https://github.com/Neargye/magic_enum/releases/tag/v0.9.7
include_directories(${CMAKE_SOURCE_DIR}/extern/doctest) # Required for fake-it
include_directories(${CMAKE_SOURCE_DIR}/extern SYSTEM GLAD)

# Add Boost
find_package(Boost 1.74 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Add FFTW3
include_directories(/usr/local/fftw3/include)
link_directories(/usr/local/fftw3/lib)

# Add OpenMP
# Apple settings from: https://stackoverflow.com/a/60198415/27492177
if(APPLE)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)

    if(CMAKE_C_COMPILER_ID MATCHES "Clang\$")
        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
        set(OpenMP_C_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY omp)
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang\$")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY omp)
    endif()
endif()

find_package(OpenMP REQUIRED)

# Add main executable
file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

# Gather test files
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp src/**/*.cpp)

# Add test executable
set(TEST_NAME ${PROJECT_NAME}_tests)
add_executable(${TEST_NAME} ${TEST_SOURCES})
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

# Link FFTW3 and math library
target_link_libraries(${PROJECT_NAME} fftw3 m)
target_link_libraries(${TEST_NAME} fftw3 m)

# Link OpenMP
if (OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
    target_link_libraries(${TEST_NAME} OpenMP::OpenMP_CXX)
endif()

# Ensure flags are passed to ccache
if (CCACHE_PROGRAM)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()
