editline_option = get_option('pcre2_support_libedit')
editline_dep = dependency('', required: false)
editline_header = ''
if not editline_option.disabled()
  editline_dep = dependency('libedit', required: false)
  if not editline_dep.found()
    editline_dep = cc.find_library('edit', required: false)
  endif
  if editline_dep.found()
    # pkg-config may supply the leaf include directory. Otherwise use the
    # header layouts searched by cmake/FindEditline.cmake.
    foreach header : ['readline.h', 'editline/readline.h', 'edit/readline/readline.h']
      if cc.has_function('readline', prefix: '#include <stdio.h>\n#include <' + header + '>', dependencies: editline_dep)
        editline_header = header
        break
      endif
    endforeach
    if editline_header == ''
      editline_dep = dependency('', required: false)
    endif
  endif
  if not editline_dep.found()
    if editline_option.enabled()
      error('libedit was requested but its headers and library could not be linked.')
    endif
    message('libedit support disabled: no usable headers and library found.')
  endif
endif
