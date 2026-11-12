bzip2_option = get_option('pcre2_support_libbz2')
bzip2_dep = dependency('', required: false)
if not bzip2_option.disabled()
  bzip2_dep = dependency('bzip2', required: false)
  if not bzip2_dep.found()
    bzip2_dep = dependency('BZip2', method: 'cmake', required: false)
  endif
  if not bzip2_dep.found()
    # Match the release/debug library names in CMake's FindBZip2.
    bzip2_names = ['bz2', 'bzip2', 'libbz2', 'libbzip2']
    bzip2_debug_names = ['bz2d', 'bzip2d', 'libbz2d', 'libbzip2d']
    bzip2_names = get_option('buildtype') == 'debug' ? bzip2_debug_names + bzip2_names : bzip2_names + bzip2_debug_names
    foreach name : bzip2_names
      bzip2_dep = cc.find_library(name, has_headers: 'bzlib.h', required: false)
      if bzip2_dep.found()
        break
      endif
    endforeach
  endif
  if bzip2_dep.found() and not cc.has_function('BZ2_bzopen', prefix: '#include <bzlib.h>', dependencies: bzip2_dep)
    bzip2_dep = dependency('', required: false)
  endif
  if not bzip2_dep.found()
    if bzip2_option.enabled()
      error('libbz2 was requested but its headers and library could not be linked.')
    endif
    message('libbz2 support disabled: no usable headers and library found.')
  endif
endif
