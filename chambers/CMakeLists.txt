set(CMAKE_EXECUTABLE_SUFFIX ".wasm")

add_compile_options(-fvisibility=hidden)

# Global link options for WebAssembly
add_link_options(
  "SHELL:-s WASM=1"
  "SHELL:-s STANDALONE_WASM=1"
  "SHELL:--strip-all"
  "SHELL:--export-dynamic"
  "SHELL:--import-memory"
  "SHELL:--no-entry"
  "SHELL:-s EXPORTED_FUNCTIONS='[_init, _saveMemory, _ballsMemory, _canvasMemory, _save, _load, _step, _render, _saveSize]'"
  "SHELL:-s EXPORTED_RUNTIME_METHODS='[logWasm]'"
  "SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=0"
  )

add_subdirectory(portals)
add_subdirectory(simple_example)
