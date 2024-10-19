project(guard)

add_executable(${PROJECT_NAME}
  src/guard.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE chamber)
