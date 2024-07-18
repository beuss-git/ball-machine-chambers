cmake_minimum_required(VERSION 3.28)
project(ball-machine-chambers)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(libchamber)
add_subdirectory(chambers)
