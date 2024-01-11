# This variable is set by project() in CMake 3.21+
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}" PROJECT_IS_TOP_LEVEL)

if(PROJECT_IS_TOP_LEVEL)
	option(shared_DEVELOPER_MODE "Enable developer mode" OFF)
endif()

option(BUILD_SAMPLES "Build the samples" OFF)
