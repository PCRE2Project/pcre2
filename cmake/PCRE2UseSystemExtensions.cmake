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
#
# Autoconf enables this unconditionally. However, our CMake script potentially
# supports *more* platforms than Autoconf, so we use a feature check.

function(pcre2_use_system_extensions)
  if (WIN32)
    return()
  endif()

  include(CheckCSourceCompiles)

  set(_pcre2_test_src "
#include <sys/time.h>
#include <sys/resource.h>

int main(void) {
    struct rlimit rlim;
    getrlimit(RLIMIT_STACK, &rlim);
    return 0;
}
")

  check_c_source_compiles("${_pcre2_test_src}" HAVE_GETRLIMIT)

  if (NOT HAVE_GETRLIMIT)
    # Try again with _ALL_SOURCE
    set(CMAKE_REQUIRED_DEFINITIONS "-D_ALL_SOURCE")
    check_c_source_compiles("${_pcre2_test_src}" HAVE_GETRLIMIT_ALLSOURCE)
    unset(CMAKE_REQUIRED_DEFINITIONS)

    if (HAVE_GETRLIMIT_ALLSOURCE)
      add_compile_definitions(_ALL_SOURCE)
    endif()
  endif()
endfunction()
