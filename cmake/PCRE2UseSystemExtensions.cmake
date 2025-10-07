# This CMake module is supposed to give similar results to the
# AC_USE_SYSTEM_EXTENSIONS Autoconf macro, which turns on a load of
# system feature-check macros, including _ALL_SOURCE, _GNU_SOURCE,
# _NETBSD_SOURCE, and many more.
#
# Because PCRE2 uses so few OS features, we don't seem to actually need to
# enable many of these. Modern platforms with CMake users generally enable
# all the basic POSIX features by default.
#
# So far, we know that we require:
#   - _ALL_SOURCE on IBM systems (z/OS, probably AIX) in order to call
#     getrlimit() in pcre2test.
#   - _GNU_SOURCE on Linux in order to call mkostemp() in some (non-default)
#     configurations of the JIT.
#
# Autoconf enables this unconditionally. However, our CMake script potentially
# supports *more* platforms than Autoconf, so we use a feature check.

function(pcre2_use_system_extensions)
  if(WIN32)
    return()
  endif()

  include(CheckSymbolExists)
  include(CheckCSourceCompiles)
  include(CMakePushCheckState)

  cmake_push_check_state(RESET)
  set(
    _pcre2_test_src
    "
#include <sys/time.h>
#include <sys/resource.h>

int main(void) {
    struct rlimit rlim;
    getrlimit(RLIMIT_STACK, &rlim);
    return 0;
}
"
  )
  set(CMAKE_REQUIRED_QUIET TRUE)
  check_c_source_compiles("${_pcre2_test_src}" _HAVE_GETRLIMIT_NAKED)

  if(NOT _HAVE_GETRLIMIT_NAKED)
    # Try again with _ALL_SOURCE
    set(CMAKE_REQUIRED_DEFINITIONS "-D_ALL_SOURCE")
    check_c_source_compiles("${_pcre2_test_src}" _HAVE_GETRLIMIT_ALLSOURCE)
    unset(CMAKE_REQUIRED_DEFINITIONS)

    if(_HAVE_GETRLIMIT_ALLSOURCE)
      message(STATUS "Detected platform feature gate _ALL_SOURCE")
      add_compile_definitions(_ALL_SOURCE)
    endif()
  endif()

  check_symbol_exists(mkostemp stdlib.h _HAVE_MKOSTEMP_NAKED)

  if(NOT _HAVE_MKOSTEMP_NAKED)
    # Try again with _GNU_SOURCE
    set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE")
    check_symbol_exists(mkostemp stdlib.h _HAVE_MKOSTEMP_GNUSOURCE)
    unset(CMAKE_REQUIRED_DEFINITIONS)

    if(_HAVE_MKOSTEMP_GNUSOURCE)
      message(STATUS "Detected platform feature gate _GNU_SOURCE")
      add_compile_definitions(_GNU_SOURCE)
    endif()
  endif()

  cmake_pop_check_state()
endfunction()
