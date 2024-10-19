include(FetchContent)

set(LIBPHYSICS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/libphysics.a")

if(NOT EXISTS ${LIBPHYSICS_PATH})
  FetchContent_Declare(
    libphysics
    URL https://sphaerophoria.dev/libphysics.a
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs
    DOWNLOAD_NO_EXTRACT TRUE
  )

  FetchContent_MakeAvailable(libphysics)
endif()

add_library(physics STATIC IMPORTED GLOBAL)
set_target_properties(physics PROPERTIES
  IMPORTED_LOCATION ${LIBPHYSICS_PATH}
)
