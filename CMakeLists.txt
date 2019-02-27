cmake_minimum_required(VERSION 3.0)

project(livox_sdk)

set(CMAKE_CXX_STANDARD 98)

message(STATUS "main project dir: " ${PROJECT_SOURCE_DIR})

add_subdirectory(sdk_core sdk_core)
add_subdirectory(sample/hub)
add_subdirectory(sample/lidar)

