cmake_minimum_required(VERSION 3.16.3)
project(Perun LANGUAGES CXX)
message("-- Cmake config in: ${CMAKE_HOME_DIRECTORY}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -pedantic -Wimplicit-fallthrough=0")

# Set the build type to Debug
set(CMAKE_BUILD_TYPE Debug)

# Or set it to Release
# set(CMAKE_BUILD_TYPE Release)

# compile debug definition
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
else()
    add_compile_definitions(DEBUG=0)
endif()


# add all files
# Set the source directory
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Find .h files
file(GLOB_RECURSE HEADER_FILES ${SOURCE_DIR}/*.h)

# Find .cpp files
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.cpp)

# config include list
set(HEADER_DIRS_LIST "")
foreach(HEADER_FILE ${HEADER_FILES})
    get_filename_component(HEADER_DIR ${HEADER_FILE} DIRECTORY)
    list(APPEND HEADER_DIRS_LIST ${HEADER_DIR})
endforeach()
list(REMOVE_DUPLICATES HEADER_DIRS_LIST)
message(" + ${HEADER_DIRS_LIST}")

# specify main target
add_executable(main ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(main PRIVATE HEADER_DIRS_LIST)
