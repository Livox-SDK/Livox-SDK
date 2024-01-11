cmake_minimum_required(VERSION 3.0)

set(DEMO_NAME lidar_sample_cc)
add_executable(${DEMO_NAME} main.cpp lds_lidar.cpp)

target_link_libraries(${DEMO_NAME}
        PRIVATE
        ${PROJECT_NAME}_static
        )
target_include_directories(${DEMO_NAME}
        PRIVATE
        "${CMAKE_SOURCE_DIR}/sdk_core/include/third_party/cmdline"
        )

message("----- CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message("----- CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message("----- CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
message("----- CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")
message("----- PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")


get_target_property(incl_dirs ${DEMO_NAME} INCLUDE_DIRECTORIES)
message("----- ${DEMO_NAME} incl dirs = ${incl_dirs}")
