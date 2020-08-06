
project(memory_tool)

cmake_minimum_required(
	VERSION
	3.0
)

set(
	CMAKE_CXX_STANDARD
	20
)

file(
	GLOB_RECURSE SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/*
)

add_library(
	${PROJECT_NAME}
	OBJECT	
	${SRCS}
)

target_link_libraries(
	${PROJECT_NAME}
	singleton_pattern
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../
)
