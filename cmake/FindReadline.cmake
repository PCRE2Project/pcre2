# from http://websvn.kde.org/trunk/KDE/kdeedu/cmake/modules/FindReadline.cmake
# http://websvn.kde.org/trunk/KDE/kdeedu/cmake/modules/COPYING-CMAKE-SCRIPTS
# --> BSD licensed
#
# GNU Readline library finder

find_path(READLINE_INCLUDE_DIR readline/readline.h PATH_SUFFIXES include)
mark_as_advanced(READLINE_INCLUDE_DIR)

find_library(READLINE_LIBRARY NAMES readline)
mark_as_advanced(READLINE_LIBRARY)

set(_pcre2_readline_ncurses_library "")
if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
  # Check if we need to link to ncurses as well

  include(CheckSymbolExists)
  include(CMakePushCheckState)

  set(first_run FALSE)
  if(NOT DEFINED HAVE_READLINE_FUNC)
    set(first_run TRUE)
  endif()

  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_LIBRARIES "${READLINE_LIBRARY}")
  set(CMAKE_REQUIRED_INCLUDES "${READLINE_INCLUDE_DIR}")
  check_symbol_exists("readline" "stdio.h;readline/readline.h" HAVE_READLINE_FUNC)

  if(NOT HAVE_READLINE_FUNC)
    foreach(
      lib
      IN
      ITEMS tinfo curses ncurses ncursesw termcap
    )
      find_library(NCURSES_LIBRARY_${lib} NAMES ${lib})
      mark_as_advanced(NCURSES_LIBRARY_${lib})
      if(NCURSES_LIBRARY_${lib})
        cmake_reset_check_state()
        set(CMAKE_REQUIRED_LIBRARIES "${READLINE_LIBRARY}" "${NCURSES_LIBRARY_${lib}}")
        set(CMAKE_REQUIRED_INCLUDES "${READLINE_INCLUDE_DIR}")
        check_symbol_exists("readline" "stdio.h;readline/readline.h" HAVE_READLINE_FUNC_${lib})

        if(HAVE_READLINE_FUNC_${lib})
          if(first_run)
            message(STATUS "Looking for readline - readline needs ${lib}")
          endif()
          set(_pcre2_readline_ncurses_library "${NCURSES_LIBRARY_${lib}}")
          break()
        endif()
      endif()
    endforeach()
  endif()

  cmake_pop_check_state()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Readline DEFAULT_MSG READLINE_LIBRARY READLINE_INCLUDE_DIR)

if(Readline_FOUND)
  set(READLINE_LIBRARIES "${READLINE_LIBRARY}")
  if(_pcre2_readline_ncurses_library)
    list(APPEND READLINE_LIBRARIES "${_pcre2_readline_ncurses_library}")
  endif()
  set(READLINE_INCLUDE_DIRS "${READLINE_INCLUDE_DIR}")
endif()
