cmake_minimum_required(VERSION 3.28)
project(chamber)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${PROJECT_NAME} 
  src/libchamber/Chamber.cpp
  )

target_compile_options(${PROJECT_NAME} PRIVATE 
  -Wall 
  -Wextra 
  -Wshadow 
  )

set (CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS}")
set (CMAKE_CPP_FLAGS                  "${CMAKE_CPP_FLAGS} -z stack-size=32768")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s WASM=1 -s STANDALONE_WASM=1 --strip-all --export-dynamic --import-memory -fvisibility=hidden --no-entry -s EXPORTED_FUNCTIONS='[_init, _saveMemory, _ballsMemory, _canvasMemory, _save, _load, _step, _render, _saveSize]' -s EXPORTED_RUNTIME_METHODS='[logWasm]' -s ERROR_ON_UNDEFINED_SYMBOLS=0")

include(cmake/LibPhysics.cmake)

target_link_libraries(${PROJECT_NAME} PUBLIC physics)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_library(project_warnings INTERFACE)

include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

