cmake_minimum_required(VERSION 3.0)

set(DEMO_NAME hub_sample)
add_executable(${DEMO_NAME} main.c)

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
