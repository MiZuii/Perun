cmake_minimum_required(VERSION 3.16.3)
project(Perun LANGUAGES CXX)
message("-- Cmake config in: ${CMAKE_HOME_DIRECTORY}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=x86-64-v3 -Wall -Wextra -pedantic")

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

# specify main target
add_executable(main ${SOURCE_FILES} ${HEADER_FILES})

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    add_compile_definitions(x86_64)
endif()
