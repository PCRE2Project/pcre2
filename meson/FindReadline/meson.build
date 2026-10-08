readline_option = get_option('pcre2_support_libreadline')
readline_dep = dependency('', required: false)
if not readline_option.disabled()
  readline_dep = dependency('readline', required: false)
  if not readline_dep.found()
    readline_dep = cc.find_library('readline', has_headers: ['stdio.h', 'readline/readline.h'], required: false)
  endif
  if readline_dep.found()
    readline_prefix = '#include <stdio.h>\n#include <readline/readline.h>'
    readline_links = cc.has_function('readline', prefix: readline_prefix, dependencies: readline_dep)
    if not readline_links
      # Keep this search order aligned with cmake/FindReadline.cmake.
      foreach name : ['tinfo', 'curses', 'ncurses', 'ncursesw', 'termcap']
        terminal_dep = cc.find_library(name, required: false)
        if terminal_dep.found() and cc.has_function(
          'readline',
          prefix: readline_prefix,
          dependencies: [readline_dep, terminal_dep],
        )
          readline_dep = declare_dependency(dependencies: [readline_dep, terminal_dep])
          readline_links = true
          break
        endif
      endforeach
    endif
    if not readline_links
      readline_dep = dependency('', required: false)
    endif
  endif
  if not readline_dep.found()
    if readline_option.enabled()
      error(
        'libreadline was requested but its headers and library could not be linked, including terminal dependencies.',
      )
    endif
    message('libreadline support disabled: no usable headers and library found.')
  endif
endif
