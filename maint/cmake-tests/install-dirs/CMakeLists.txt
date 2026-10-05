cmake_minimum_required(VERSION 3.15)
project(TestInstallDirs NONE)

get_filename_component(PCRE2_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

foreach(layout default relative absolute)
  set(build_dir "${CMAKE_CURRENT_BINARY_DIR}/${layout}")
  set(prefix "${build_dir}/prefix")
  if(layout STREQUAL "default")
    set(include_dir "include")
    set(lib_dir "lib")
  elseif(layout STREQUAL "relative")
    set(include_dir "custom/include")
    set(lib_dir "custom/lib")
  else()
    set(include_dir "${CMAKE_CURRENT_BINARY_DIR}/external/include")
    set(lib_dir "${CMAKE_CURRENT_BINARY_DIR}/external/lib")
  endif()

  execute_process(
    COMMAND
      "${CMAKE_COMMAND}" -S "${PCRE2_SOURCE_DIR}" -B "${build_dir}" "-DCMAKE_INSTALL_PREFIX=${prefix}"
      "-DCMAKE_INSTALL_INCLUDEDIR=${include_dir}" "-DCMAKE_INSTALL_LIBDIR=${lib_dir}" -DPCRE2_BUILD_PCRE2_16=ON
      -DPCRE2_BUILD_PCRE2_32=ON -DPCRE2_BUILD_TESTS=OFF -DPCRE2_BUILD_PCRE2GREP=OFF
    RESULT_VARIABLE result
  )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "Could not configure the ${layout} installation layout")
  endif()

  if(layout STREQUAL "absolute")
    set(expected_include "${include_dir}")
    set(expected_lib "${lib_dir}")
  else()
    set(expected_include "\${prefix}/${include_dir}")
    set(expected_lib "\${exec_prefix}/${lib_dir}")
  endif()

  foreach(width 8 16 32 posix)
    file(READ "${build_dir}/libpcre2-${width}.pc" contents)
    foreach(field "includedir=${expected_include}" "libdir=${expected_lib}")
      string(FIND "${contents}" "\n${field}\n" position)
      if(position EQUAL -1)
        message(FATAL_ERROR "${layout}: libpcre2-${width}.pc is missing ${field}")
      endif()
    endforeach()
  endforeach()

  file(READ "${build_dir}/pcre2-config" contents)
  foreach(flag "-I${expected_include}" "-L${expected_lib}")
    string(FIND "${contents}" "${flag}" position)
    if(position EQUAL -1)
      message(FATAL_ERROR "${layout}: pcre2-config is missing ${flag}")
    endif()
  endforeach()
endforeach()
