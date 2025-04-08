cmake_minimum_required(VERSION 3.15)
project(TestInstallInterface C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# To test the static interface, uncomment the following line:
# set(PCRE2_USE_STATIC_LIBS ON)
find_package(PCRE2 REQUIRED CONFIG)

add_executable(test_executable main.c)
target_link_libraries(test_executable PRIVATE PCRE2::8BIT)

if(WIN32 AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.21 AND NOT PCRE2_USE_STATIC_LIBS)
  # Ensure that the DLLs are available for the executable to run. Only needed
  # on Windows.
  add_custom_command(TARGET test_executable POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:test_executable> $<TARGET_FILE_DIR:test_executable>
    COMMAND_EXPAND_LISTS
    )
endif()
