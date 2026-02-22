cmake_minimum_required(VERSION 3.15)
project(TestBuildInterface C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# We had a regression where a parent project compile definition containing
# generator expressions was incorrectly propagated to CMAKE_REQUIRED_DEFINITIONS
# (which doesn't support generator expressions). This dummy macro here prevents
# that happening again.
add_compile_definitions($<$<CONFIG:Debug>:PARENT_PROJECT_MACRO>)

# To test the static vs dynamic interface, uncomment one of the following lines:
# set(BUILD_STATIC_LIBS OFF)
# set(BUILD_SHARED_LIBS ON)
add_subdirectory(pcre2)

add_executable(test_executable main.c)
target_link_libraries(test_executable PRIVATE pcre2-8)
