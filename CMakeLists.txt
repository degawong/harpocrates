
project(harpocrates)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)

set(
	CMAKE_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} -g"
)

add_definitions(
	-DNEON_OPTIMIZE_LEVEL
	-DSTB_IMAGE_IMPLEMENTATION
	-DSTB_IMAGE_WRITE_IMPLEMENTATION
	-DSTB_IMAGE_RESIZE_IMPLEMENTATION
)

add_subdirectory(module)

set_property(
	GLOBAL	PROPERTY
	USE_FOLDERS ON
)

file(
	GLOB
	SRCS
	*.cpp
)

filter(
	SRCS
)

add_executable(
	${PROJECT_NAME}
	${SRCS}
	module/harpocrates.h
)

target_link_libraries(
	${PROJECT_NAME}
    package
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

set_directory_properties(
	PROPERTIES
	VS_STARTUP_PROJECT
	${PROJECT_NAME}
)

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY
	"${CMAKE_CURRENT_SOURCE_DIR}/workspace/"
)

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS
	"f:/image/relighting_test_image/image_yuv"
)
