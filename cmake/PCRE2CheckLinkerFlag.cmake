# This file can be removed once the minimum CMake version is increased to 3.18
# or higher. Calls to pcre2_check_linker_flag can be changed to the built in
# check_linker_flag.

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
  include(CheckLinkerFlag)

  function(pcre2_check_linker_flag lang flag out_var)
    check_linker_flag("${lang}" "${flag}" "${out_var}")
  endfunction()
else()
  # Fallback for versions of CMake older than 3.18.

  include(CheckCCompilerFlag)
  include(CMakePushCheckState)

  function(pcre2_check_linker_flag lang flag out_var)
    cmake_policy(PUSH)
    cmake_policy(SET CMP0056 NEW)
    cmake_push_check_state(RESET)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}")
    check_c_compiler_flag("" ${out_var})
    cmake_pop_check_state()
    cmake_policy(POP)
  endfunction()
endif()
