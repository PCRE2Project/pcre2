cmake_minimum_required(VERSION 3.15)
project(TestBuildInterface C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# We had a regression where a parent project compile definition containing
# generator expressions was incorrectly propagated to CMAKE_REQUIRED_DEFINITIONS
# (which doesn't support generator expressions). This dummy macro here prevents
# that happening again.
add_compile_definitions($<$<CONFIG:Debug>:PARENT_PROJECT_MACRO>)

if(NOT PCRE2_SOURCE_DIR)
	message(FATAL_ERROR "PCRE2_SOURCE_DIR must name the PCRE2 source directory")
endif()
add_subdirectory("${PCRE2_SOURCE_DIR}" pcre2)

add_executable(test_executable main.c)
target_link_libraries(test_executable PRIVATE pcre2-8)

if(WIN32 AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.21 AND BUILD_SHARED_LIBS)
  # Ensure that the DLLs are available for the executable to run. Only needed
  # on Windows.
  add_custom_command(
    TARGET test_executable
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:test_executable> $<TARGET_FILE_DIR:test_executable>
    COMMAND_EXPAND_LISTS
  )
endif()
