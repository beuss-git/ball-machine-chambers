cmake_minimum_required(VERSION 3.28)
project(ball-machine-chambers)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} 
  src/main.cpp
  )

target_compile_options(${PROJECT_NAME} PRIVATE 
  -Wall 
  -Wextra 
  -Wshadow 
  )

set (CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS} -z stack-size=32768")
set (CMAKE_CPP_FLAGS                  "${CMAKE_CPP_FLAGS} -z stack-size=32768")
set(CMAKE_EXECUTABLE_SUFFIX ".wasm")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s WASM=1 -s STANDALONE_WASM=1 --no-entry -s EXPORTED_FUNCTIONS='[_init, _saveMemory, _ballsMemory, _canvasMemory, _save, _load, _step, _render, _saveSize]'")


#target_link_libraries(${PROJECT_NAME} PRIVATE argparse SDL2::SDL2 SDL2_ttf unofficial::skia::skia SmoothScroll)


add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

