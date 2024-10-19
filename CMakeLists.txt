cmake_minimum_required(VERSION 3.25)
project(chamber)

option(CHAMBER_BUILD_EXAMPLES "Build examples" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -z stack-size=16384")

add_library(${PROJECT_NAME}
  src/libchamber/chamber.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall
  -Wextra
  -Wshadow
)

include(cmake/LibPhysics.cmake)

target_link_libraries(${PROJECT_NAME} PUBLIC physics)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_library(project_warnings INTERFACE)

include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

if(CHAMBER_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
