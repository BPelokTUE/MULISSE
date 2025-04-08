cmake_minimum_required(VERSION 3.10)
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
project(mulisse)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion")

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
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# Add FFTW3
set(LOCAL_SETTINGS_FILE "${CMAKE_SOURCE_DIR}/local_settings.json")
file(READ ${LOCAL_SETTINGS_FILE} LOCAL_SETTINGS)
string(REGEX MATCH "\"FFTW_PATH\"[ \t]*:[ \t]*\"([^\"]*)\"" _ ${LOCAL_SETTINGS})
set(FFTW_PATH "${CMAKE_MATCH_1}")

include_directories(${FFTW_PATH}include)
link_directories(SYSTEM ${FFTW_PATH}lib)

# Add OpenMP
find_package(OpenMP REQUIRED)

# Ensure flags are passed to ccache
if (CCACHE_PROGRAM)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()

# Function for setting up target (adding compile options, preprocessor definitions, and linking libraries)
function(set_up_target target)
    # Set compiler-specific flags to ignore warnings for the extern directory
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_include_directories(${target} SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/extern)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${target} PRIVATE /external:I ${CMAKE_SOURCE_DIR}/extern)
    endif()

    # Add preprocessor definitions
    if(DISABLE_LOGGING)
        target_compile_definitions(${target} PRIVATE DISABLE_LOGGING)
    endif()

    if(DISABLE_PARALLELISM)
        target_compile_definitions(${target} PRIVATE DISABLE_PARALLELISM)
    endif()

    if (USE_DOUBLE)
        target_compile_definitions(${target} PRIVATE USE_DOUBLE)
        target_link_libraries(${target} fftw3l m)
    else()
        target_link_libraries(${target} fftw3f m)
    endif()

    # Link OpenMP
    if (OpenMP_CXX_FOUND)
        target_link_libraries(${target} OpenMP::OpenMP_CXX)
    endif()
endfunction()

# Add main executable
file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
set_up_target(${PROJECT_NAME})


# Option to ignore tests
option(IGNORE_TESTS "Ignore tests" OFF)

if (NOT IGNORE_TESTS)
    # Gather test files
    file(GLOB_RECURSE TEST_SOURCES tests/*.cpp src/**/*.cpp)

    # Add test executable
    set(TEST_NAME ${PROJECT_NAME}_tests)
    add_executable(${TEST_NAME} ${TEST_SOURCES})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    target_compile_definitions(${TEST_NAME} PRIVATE ENABLE_TEST_CODE)
    set_up_target(${TEST_NAME})
endif()
