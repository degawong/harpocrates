
project(package)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)

file(
	GLOB
	HEADERS
	base/*
)

add_definitions(
	-Dwong=harpocrates
	-DSTB_IMAGE_IMPLEMENTATION
	-DSTB_IMAGE_WRITE_IMPLEMENTATION
)

add_subdirectory(reflection)
add_subdirectory(time_clock)
add_subdirectory(path_walker)
add_subdirectory(thread_pool)
add_subdirectory(singleton_pattern)

add_library(
	${PROJECT_NAME}
	${HEADERS}
	$<TARGET_OBJECTS:reflection>
	$<TARGET_OBJECTS:time_clock>
	$<TARGET_OBJECTS:path_walker>
	$<TARGET_OBJECTS:thread_pool>
	$<TARGET_OBJECTS:singleton_pattern>
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
