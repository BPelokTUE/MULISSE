cmake_minimum_required(VERSION 3.10)
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(mulisse_scripts)

# Set the same C++ standard as the main project
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the main project's headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
# Include the main project's external headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../extern SYSTEM GLAD)

# Find required packages (same as main project)
find_package(Boost 1.74 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Add FFTW3
include_directories(/usr/local/fftw3/include)
link_directories(/usr/local/fftw3/lib)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Collect all cpp files in the directory
file(GLOB_RECURSE MAIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../src/**/*.cpp)
file(GLOB SCRIPT_SOURCES src/*.cpp)

# Create an executable for each script
foreach(SCRIPT_SOURCE ${SCRIPT_SOURCES})
    get_filename_component(SCRIPT_NAME ${SCRIPT_SOURCE} NAME_WE)
    add_executable(${SCRIPT_NAME} ${SCRIPT_SOURCE} ${MAIN_SOURCES})
    
    # Link with required libraries
    target_link_libraries(${SCRIPT_NAME} 
        fftw3 
        m
        OpenMP::OpenMP_CXX
    )
endforeach()
