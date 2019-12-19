cmake_minimum_required(VERSION 3.0)

project(livox_sdk)

set(CMAKE_CXX_STANDARD 11)

message(STATUS "main project dir: " ${PROJECT_SOURCE_DIR})

add_subdirectory(sdk_core sdk_core)
add_subdirectory(sample/hub)
add_subdirectory(sample/lidar)
add_subdirectory(sample/hub_lvx_file)
add_subdirectory(sample/lidar_lvx_file)
add_subdirectory(sample_cc/hub)
add_subdirectory(sample_cc/lidar)
add_subdirectory(sample_cc/trouble_shooting)
add_subdirectory(sample_cc/lidar_utc_sync)