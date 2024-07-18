project(portals-chamber)

add_executable(${PROJECT_NAME} 
  src/portals.cpp
  src/portal.cpp
  src/canvas_ity.cpp
  src/utils/image.cpp
  )

target_link_libraries(${PROJECT_NAME} PRIVATE chamber)
target_include_directories(${PROJECT_NAME} PRIVATE include)

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s WASM=1 -s STANDALONE_WASM=1 --strip-all --export-dynamic --import-memory -fvisibility=hidden --no-entry -s EXPORTED_FUNCTIONS='[_init, _saveMemory, _ballsMemory, _canvasMemory, _save, _load, _step, _render, _saveSize]' -s EXPORTED_RUNTIME_METHODS='[logWasm]' -s ERROR_ON_UNDEFINED_SYMBOLS=0")

