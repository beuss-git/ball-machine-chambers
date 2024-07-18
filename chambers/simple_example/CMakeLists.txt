project(simple-chamber)

add_executable(${PROJECT_NAME} 
  src/simple_chamber.cpp
  )

target_link_libraries(${PROJECT_NAME} PRIVATE chamber)
