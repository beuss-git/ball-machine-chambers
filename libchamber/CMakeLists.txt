cmake_minimum_required(VERSION 3.25)
project(chamber)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${PROJECT_NAME} 
  src/libchamber/chamber.cpp
  )

target_compile_options(${PROJECT_NAME} PRIVATE 
  -Wall 
  -Wextra 
  -Wshadow 
  )

#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s WASM=1 -s PURE_WASI=1 -s INITIAL_HEAP=65536 -s INITIAL_MEMORY=65536  -s MAXIMUM_MEMORY=65536 -s STANDALONE_WASM=1 --strip-all -fvisibility=hidden --no-entry -s EXPORTED_FUNCTIONS='[_init, _saveMemory, _ballsMemory, _canvasMemory, _save, _load, _step, _render, _saveSize]' -s EXPORTED_RUNTIME_METHODS='[logWasm]' -s ERROR_ON_UNDEFINED_SYMBOLS=0")

include(cmake/LibPhysics.cmake)

target_link_libraries(${PROJECT_NAME} PUBLIC physics)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_library(project_warnings INTERFACE)

include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

