
project(package)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)

file(
	GLOB
	HEADERS
	base/*
)

add_definitions(
	-DSTB_IMAGE_IMPLEMENTATION
	-DSTB_IMAGE_WRITE_IMPLEMENTATION
	-DSTB_IMAGE_RESIZE_IMPLEMENTATION
)

list(
	APPEND
	CMAKE_MODULE_PATH
	${CMAKE_CURRENT_SOURCE_DIR}/marco
)

include(
	marco
)

add_subdirectory(3rdparty)
add_subdirectory(reflection)
add_subdirectory(time_clock)
add_subdirectory(image_tool)
add_subdirectory(path_walker)
add_subdirectory(thread_pool)
add_subdirectory(singleton_pattern)

set_property(TARGET image PROPERTY FOLDER "3rdparty")
set_property(TARGET time_clock PROPERTY FOLDER "module")
set_property(TARGET image_tool PROPERTY FOLDER "module")
set_property(TARGET reflection PROPERTY FOLDER "module")
set_property(TARGET path_walker PROPERTY FOLDER "module")
set_property(TARGET thread_pool PROPERTY FOLDER "module")
set_property(TARGET singleton_pattern PROPERTY FOLDER "module")

add_library(
	${PROJECT_NAME}
	${HEADERS}
	$<TARGET_OBJECTS:image>
	$<TARGET_OBJECTS:reflection>
	$<TARGET_OBJECTS:time_clock>
	$<TARGET_OBJECTS:image_tool>
	$<TARGET_OBJECTS:path_walker>
	$<TARGET_OBJECTS:thread_pool>
	$<TARGET_OBJECTS:singleton_pattern>
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
)
