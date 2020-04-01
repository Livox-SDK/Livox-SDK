cmake_minimum_required(VERSION 3.0)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif ()

set(DEMO_NAME lidar_utc_sync)
add_executable(${DEMO_NAME} main.cpp lds_lidar.cpp synchro.cpp)

if (WIN32)
add_custom_command(TARGET ${DEMO_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/third_party/apr/bin/libapr-1.dll"
        $<TARGET_FILE_DIR:${DEMO_NAME}>)
endif()

target_link_libraries(${DEMO_NAME}
        PRIVATE
        ${PROJECT_NAME}_static
        )
