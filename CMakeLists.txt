
project(harpocrates)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)

set(
	CMAKE_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} -g"
)

add_subdirectory(module)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET reflection PROPERTY FOLDER "module")
set_property(TARGET path_walker PROPERTY FOLDER "module")
set_property(TARGET time_clock PROPERTY FOLDER "module")
set_property(TARGET thread_pool PROPERTY FOLDER "module")
set_property(TARGET singleton_pattern PROPERTY FOLDER "module")

list(
	APPEND
	CMAKE_MODULE_PATH
	${CMAKE_CURRENT_SOURCE_DIR}/module/marco
)

include(
	marco
)

file(
	GLOB
	SRCS
	*.cpp
)

add_executable(
	${PROJECT_NAME}
	${SRCS}
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
