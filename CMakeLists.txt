cmake_minimum_required(VERSION 3.10)
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
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Add tests
enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB SOURCES
    src/*.cpp
    src/Summarization/*.cpp
    src/Search/*.cpp
    src/Util/*.cpp
)

# Add CLI11 subdirectory
add_subdirectory(extern/CLI11)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE CLI11::CLI11)
