project(Examples VERSION 1.0.0 LANGUAGES CXX)

# Basic example
set(BASIC_EXAMPLE_SOURCES example_basic.cpp)

add_executable(basic_example
  ${BASIC_EXAMPLE_SOURCES}
)

target_link_libraries(basic_example
  PRIVATE Logger
)

# Advanced example
set(ADVANCED_EXAMPLE_SOURCES example_advanced.cpp)

add_executable(advanced_example
  ${ADVANCED_EXAMPLE_SOURCES}
)

target_link_libraries(advanced_example
  PRIVATE Logger
)

message(STATUS "${PROJECT_NAME} built for Logger library")