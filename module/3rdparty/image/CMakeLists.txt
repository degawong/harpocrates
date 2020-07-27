
project(image)

cmake_minimum_required(
	VERSION
	3.0
)

set(
	CMAKE_CXX_STANDARD
	20
)

add_definitions(
	-DSTB_IMAGE_IMPLEMENTATION
	-DSTB_IMAGE_WRITE_IMPLEMENTATION
	-DSTB_IMAGE_RESIZE_IMPLEMENTATION
)

file(
	GLOB_RECURSE SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/*
)

# filter(
	# SRCS
# )

add_library(
	${PROJECT_NAME}
	OBJECT
	${SRCS}
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
