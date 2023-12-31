cmake_minimum_required(VERSION 3.16.3)
project(Perun LANGUAGES CXX)
message("-- Cmake config in: ${CMAKE_HOME_DIRECTORY}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -pedantic")


# tmp main target
add_executable(main src/main.cpp)