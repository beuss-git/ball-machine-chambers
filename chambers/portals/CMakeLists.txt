project(portals-chamber)


add_executable(${PROJECT_NAME} 
  src/portals_chamber.cpp
  src/portal.cpp
  src/canvas_ity.cpp
  src/utils/image.cpp
  )

#target_compile_definitions(${PROJECT_NAME} PRIVATE -DRENDER_LIVE)

target_link_libraries(${PROJECT_NAME} PRIVATE chamber)
target_include_directories(${PROJECT_NAME} PRIVATE include)

