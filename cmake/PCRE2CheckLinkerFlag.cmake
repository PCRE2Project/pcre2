include(CheckLinkerFlag OPTIONAL)

# This file can be removed once the minimum CMake version is increased to 3.18
# or higher. Calls to pcre2_check_linker_flag can be changed to the built in
# check_linker_flag.

if(COMMAND check_linker_flag)
  macro(pcre2_check_linker_flag)
    check_linker_flag(${ARGN})
  endmacro()
else()
  # Fallback for versions of CMake older than 3.18.

  include(CheckCCompilerFlag)

  function(pcre2_check_linker_flag lang flag out_var)
    cmake_policy(PUSH)
    cmake_policy(SET CMP0056 NEW)
    set(_CMAKE_EXE_LINKER_FLAGS_SAVE ${CMAKE_EXE_LINKER_FLAGS})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}")
    check_c_compiler_flag("" ${out_var})
    set(CMAKE_EXE_LINKER_FLAGS ${_CMAKE_EXE_LINKER_FLAGS_SAVE})
    cmake_policy(POP)
  endfunction()
endif()
