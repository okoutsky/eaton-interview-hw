cmake_minimum_required(VERSION 3.16.3)

# Project name
project(eaton-interview-hw)

set(PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Add CMake search directory
list(APPEND CMAKE_MODULE_PATH ${PROJECT_PATH}/cmake)

# set paths
set(INCLUDE_PATH      "${PROJECT_PATH}/include")
set(MODULES_PATH      "${PROJECT_PATH}/modules")
set(SRC_PATH          "${PROJECT_PATH}/src")

option(BUILD_TESTS "Build tests" ON)

# Verify that all project dependencies are met
include(Dependencies)

# Compilation options
include(CompileOptions)

# Add all source directories
add_subdirectory(src)
add_subdirectory(tools)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
