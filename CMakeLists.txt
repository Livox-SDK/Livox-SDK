cmake_minimum_required(VERSION 3.0)

project(livox_sdk)

message(STATUS "main project dir: " ${PROJECT_SOURCE_DIR})

# This variable is set by project() in CMake 3.21+
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}" PROJECT_IS_TOP_LEVEL)

if(PROJECT_IS_TOP_LEVEL)
	option(shared_DEVELOPER_MODE "Enable developer mode" OFF)
endif()

set(CMAKE_CXX_STANDARD 11)

if (CMAKE_CROSSCOMPILING)
	set(THREADS_PTHREAD_ARG
		"PLEASE_FILL_OUT-FAILED_TO_RUN"
		CACHE STRING "Result from TRY_RUN" FORCE)
endif()

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(UNIX)

if(WIN32)
	add_compile_definitions("WIN32")
endif(WIN32)

# Always build the core SDK library
add_subdirectory(sdk_core sdk_core)
# Only build the samples if this is the top-level project
if(PROJECT_IS_TOP_LEVEL)
	add_subdirectory(sample/hub)
	add_subdirectory(sample/lidar)
	add_subdirectory(sample/hub_lvx_file)
	add_subdirectory(sample/lidar_lvx_file)
	add_subdirectory(sample_cc/hub)
	add_subdirectory(sample_cc/lidar)
	add_subdirectory(sample_cc/trouble_shooting)
	add_subdirectory(sample_cc/lidar_utc_sync)
endif()