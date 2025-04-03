cmake_minimum_required(VERSION 3.31)

project(LoggerTests LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB TEST_SOURCES "*.cpp")

add_executable(logger_test 
  ${TEST_SOURCES}
)

target_link_libraries(logger_test 
  PRIVATE 
    gtest 
    gtest_main
    Logger
)

include(GoogleTest)
gtest_discover_tests(logger_test)