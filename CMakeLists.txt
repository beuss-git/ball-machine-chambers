cmake_minimum_required(VERSION 3.28)
project(ball-machine-chambers)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS}")
set (CMAKE_CPP_FLAGS                  "${CMAKE_CPP_FLAGS} -z stack-size=16384")

add_subdirectory(libchamber)
add_subdirectory(chambers)
