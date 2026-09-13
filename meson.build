project(
  'PCRE2',
  'c',
  version: '10.49',
  license: 'BSD-3-Clause WITH PCRE2-exception',
  meson_version: '>=1.3.0',
  default_options: ['c_std=c99', 'c_args=-fvisibility=hidden'],
)

fs = import('fs')
cc = meson.get_compiler('c')

# Parse version numbers and date out of configure.ac, as CMake does.
configure_values = {}
searched_variables = [
  'pcre2_major',
  'pcre2_minor',
  'pcre2_prerelease',
  'pcre2_date',
  'libpcre2_posix_version',
  'libpcre2_8_version',
  'libpcre2_16_version',
  'libpcre2_32_version',
]
foreach line : fs.read('configure.ac').split('\n')
  foreach variable : searched_variables
    marker = 'm4_define(' + variable + ','
    if line.strip().startswith(marker)
      configure_values += {variable: line.split('[')[1].split(']')[0]}
    endif
  endforeach
endforeach
foreach variable : searched_variables
  if not configure_values.has_key(variable)
    error('Could not parse @0@ from configure.ac'.format(variable))
  endif
endforeach

pcre2_major = configure_values['pcre2_major']
pcre2_minor = configure_values['pcre2_minor']
pcre2_prerelease = configure_values['pcre2_prerelease']
pcre2_date = configure_values['pcre2_date']
if meson.project_version() != pcre2_major + '.' + pcre2_minor
  error('project version and configure.ac version do not match')
endif

lib_versions = {}
lib_soversions = {}
foreach library : ['posix', '8', '16', '32']
  parts = configure_values['libpcre2_' + library + '_version'].split(':')
  current = parts[0].to_int()
  revision = parts[1]
  age = parts[2]
  soversion = current - age.to_int()
  lib_soversions += {library: soversion.to_string()}
  lib_versions += {library: '@0@.@1@.@2@'.format(soversion, age, revision)}
endforeach

# Configuration checks.
config = configuration_data()
foreach header : ['assert.h', 'dirent.h', 'sys/stat.h', 'sys/types.h', 'unistd.h', 'windows.h']
  config.set('HAVE_' + header.underscorify().to_upper(), cc.has_header(header))
endforeach

system_extension_args = []
warning_as_error = cc.get_supported_arguments('-Werror')
getrlimit_test = '''
  #include <sys/time.h>
  #include <sys/resource.h>
  int main(void) {
    struct rlimit rlim;
    getrlimit(RLIMIT_STACK, &rlim);
    return 0;
  }
'''
if not cc.compiles(getrlimit_test, args: warning_as_error, name: 'getrlimit without feature-test macros')
  if cc.compiles(getrlimit_test, args: warning_as_error + ['-D_ALL_SOURCE'], name: 'getrlimit with _ALL_SOURCE')
    system_extension_args += '-D_ALL_SOURCE'
  endif
endif
mkostemp_test = '''
  #include <stdlib.h>
  int main(void) {
    char name[] = "pcre2-XXXXXX";
    return mkostemp(name, 0);
  }
'''
if not cc.compiles(mkostemp_test, args: warning_as_error, name: 'mkostemp without feature-test macros')
  if cc.compiles(mkostemp_test, args: warning_as_error + ['-D_GNU_SOURCE'], name: 'mkostemp with _GNU_SOURCE')
    system_extension_args += '-D_GNU_SOURCE'
  endif
endif
add_project_arguments(system_extension_args, language: 'c')

config.set('HAVE_MEMFD_CREATE', cc.has_function(
  'memfd_create',
  prefix: '#include <sys/mman.h>',
  args: system_extension_args,
))
config.set('HAVE_SECURE_GETENV', cc.has_function(
  'secure_getenv',
  prefix: '#include <stdlib.h>',
  args: system_extension_args,
))
config.set('HAVE_SETRLIMIT', cc.has_function(
  'setrlimit',
  prefix: '#include <sys/resource.h>',
  args: system_extension_args,
))
cc.compiles(
  '''
  #include <stdlib.h>
  #include <limits.h>
  int main(int c, char *v[]) {
    char buf[PATH_MAX];
    realpath(v[c], buf);
    return 0;
  }
  ''',
  args: system_extension_args,
  name: 'realpath',
)

config.set('HAVE_ATTRIBUTE_UNINITIALIZED', cc.compiles(
  'int main(void) { char buf[128] __attribute__((uninitialized)); (void)buf; return 0; }',
  args: warning_as_error,
  name: '__attribute__((uninitialized))',
))
have_visibility = cc.compiles(
  '''
  extern __attribute__ ((visibility ("default"))) int f(void);
  int main(void) { return f(); }
  int f(void) { return 42; }
  ''',
  args: warning_as_error,
  name: '__attribute__((visibility("default")))',
)
config.set('PCRE2_EXPORT', have_visibility ? '__attribute__ ((visibility ("default")))' : '')
config.set('HAVE_BUILTIN_ASSUME', cc.compiles(
  'int main(void) { __assume(1); return 0; }',
  name: '__assume',
))
config.set('HAVE_BUILTIN_MUL_OVERFLOW', cc.compiles(
  '''
  #include <stddef.h>
  int main(void) { int a, b; size_t m; __builtin_mul_overflow(a, b, &m); return 0; }
  ''',
  name: '__builtin_mul_overflow',
))
config.set('HAVE_BUILTIN_UNREACHABLE', cc.compiles(
  'int main(int c, char *v[]) { if (c) __builtin_unreachable(); return (int)(*v[0]); }',
  name: '__builtin_unreachable',
))

if cc.compiles(
  '#ifndef __CET__\n#error CET is not enabled\n#endif\nint main(void) { return 0; }',
  name: 'Intel CET',
)
  add_project_arguments(cc.get_supported_arguments('-mshstk'), language: 'c')
endif

# User-configurable options and their constraints.
build_shared = get_option('BUILD_SHARED_LIBS')
build_static = get_option('BUILD_STATIC_LIBS')
build_8 = get_option('PCRE2_BUILD_PCRE2_8')
build_16 = get_option('PCRE2_BUILD_PCRE2_16')
build_32 = get_option('PCRE2_BUILD_PCRE2_32')
if not build_shared and not build_static
  error('At least one of BUILD_SHARED_LIBS or BUILD_STATIC_LIBS must be enabled.')
endif
if not build_8 and not build_16 and not build_32
  error('At least one PCRE2 code unit width must be enabled.')
endif

build_grep = get_option('PCRE2_BUILD_PCRE2GREP')
if build_grep and not build_8
  message('PCRE2_BUILD_PCRE2_8 must be enabled for pcre2grep; disabling pcre2grep')
  build_grep = false
endif

jit_option = get_option('PCRE2_SUPPORT_JIT')
jit_enabled = jit_option == 'ON'
if jit_option == 'AUTO'
  jit_enabled = cc.compiles(
    '''
    #define SLJIT_CONFIG_AUTO 1
    #include "sljit_src/sljitConfigCPU.h"
    #if defined SLJIT_CONFIG_UNSUPPORTED && SLJIT_CONFIG_UNSUPPORTED
    #error unsupported
    #endif
    int main(void) { return 0; }
    ''',
    include_directories: include_directories('deps/sljit'),
    name: 'JIT support on this CPU',
  )
endif
thread_dep = dependency('threads', required: jit_enabled)

bzip2_dep = dependency('bzip2', required: get_option('PCRE2_SUPPORT_LIBBZ2'))
zlib_dep = dependency('zlib', required: get_option('PCRE2_SUPPORT_LIBZ'))
editline_dep = dependency('libedit', required: get_option('PCRE2_SUPPORT_LIBEDIT'))
readline_dep = dependency('readline', required: get_option('PCRE2_SUPPORT_LIBREADLINE'))
if editline_dep.found() and readline_dep.found()
  error('Only one of PCRE2_SUPPORT_LIBREADLINE and PCRE2_SUPPORT_LIBEDIT can be enabled.')
endif

ebcdic = get_option('PCRE2_EBCDIC') or get_option('PCRE2_EBCDIC_NL25') or get_option('PCRE2_EBCDIC_IGNORING_COMPILER')
rebuild_chartables = get_option('PCRE2_REBUILD_CHARTABLES')
if ebcdic
  if not get_option('PCRE2_EBCDIC_IGNORING_COMPILER')
    rebuild_chartables = true
  endif
  if get_option('PCRE2_SUPPORT_UNICODE')
    error('Support for EBCDIC and Unicode cannot be enabled at the same time.')
  endif
  if build_16 or build_32
    error('EBCDIC support is available only for the 8-bit library.')
  endif
endif

if get_option('PCRE2_FUZZ_SUPPORT') and not build_8
  error('Fuzzer support requires PCRE2_BUILD_PCRE2_8.')
endif
if get_option('PCRE2_DIFF_FUZZ_SUPPORT')
  if not get_option('PCRE2_FUZZ_SUPPORT')
    error('Differential fuzzing requires PCRE2_FUZZ_SUPPORT.')
  endif
  if not jit_enabled
    error('Differential fuzzing requires PCRE2_SUPPORT_JIT.')
  endif
endif

newline_values = {'CR': 1, 'LF': 2, 'CRLF': 3, 'ANY': 4, 'ANYCRLF': 5, 'NUL': 6}
config.set('NEWLINE_DEFAULT', newline_values[get_option('PCRE2_NEWLINE')])
config.set('SUPPORT_PCRE2_8', build_8)
config.set('SUPPORT_PCRE2_16', build_16)
config.set('SUPPORT_PCRE2_32', build_32)
config.set('DISABLE_PERCENT_ZT', get_option('PCRE2_DISABLE_PERCENT_ZT'))
config.set('SUPPORT_LIBBZ2', bzip2_dep.found())
config.set('SUPPORT_LIBEDIT', editline_dep.found())
config.set('SUPPORT_LIBREADLINE', readline_dep.found())
config.set('SUPPORT_LIBZ', zlib_dep.found())
config.set('SUPPORT_JIT', jit_enabled)
config.set('SLJIT_PROT_EXECUTABLE_ALLOCATOR', get_option('PCRE2_SUPPORT_JIT_SEALLOC'))
config.set('SUPPORT_PCRE2GREP_JIT', get_option('PCRE2GREP_SUPPORT_JIT'))
config.set('SUPPORT_PCRE2GREP_CALLOUT', get_option('PCRE2GREP_SUPPORT_CALLOUT'))
config.set('SUPPORT_PCRE2GREP_CALLOUT_FORK',
  get_option('PCRE2GREP_SUPPORT_CALLOUT') and get_option('PCRE2GREP_SUPPORT_CALLOUT_FORK'))
config.set('SUPPORT_UNICODE', get_option('PCRE2_SUPPORT_UNICODE'))
config.set('SUPPORT_VALGRIND', get_option('PCRE2_SUPPORT_VALGRIND'))
config.set('SUPPORT_DIFF_FUZZ', get_option('PCRE2_DIFF_FUZZ_SUPPORT'))
config.set('BSR_ANYCRLF', get_option('PCRE2_SUPPORT_BSR_ANYCRLF'))
config.set('EBCDIC', ebcdic)
config.set('EBCDIC_NL25', get_option('PCRE2_EBCDIC_NL25'))
config.set('EBCDIC_IGNORING_COMPILER', get_option('PCRE2_EBCDIC_IGNORING_COMPILER'))
config.set('NEVER_BACKSLASH_C', get_option('PCRE2_NEVER_BACKSLASH_C'))
foreach option : [
  'PCRE2_LINK_SIZE',
  'PCRE2_HEAP_LIMIT',
  'PCRE2_MATCH_LIMIT',
  'PCRE2_MATCH_LIMIT_DEPTH',
  'PCRE2_MAX_VARLOOKBEHIND',
  'PCRE2_PARENS_NEST_LIMIT',
  'PCRE2GREP_BUFSIZE',
  'PCRE2GREP_MAX_BUFSIZE',
]
  config.set(option, get_option(option))
endforeach

config_h = configure_file(
  input: 'src/config-meson.h.in',
  output: 'config.h',
  configuration: config,
)
pcre2_header_config = {
  'PCRE2_MAJOR': pcre2_major,
  'PCRE2_MINOR': pcre2_minor,
  'PCRE2_PRERELEASE': pcre2_prerelease,
  'PCRE2_DATE': pcre2_date,
}
pcre2_h = configure_file(
  input: 'src/pcre2.h.in',
  output: 'pcre2.h',
  configuration: pcre2_header_config,
)
pcre2posix_h = configure_file(
  input: 'src/pcre2posix.h',
  output: 'pcre2posix.h',
  copy: true,
)

if rebuild_chartables
  pcre2_dftables = executable(
    'pcre2_dftables',
    'src/pcre2_dftables.c',
    config_h,
    include_directories: include_directories('.', 'src'),
    c_args: '-DHAVE_CONFIG_H',
    native: true,
  )
  chartables = custom_target(
    'pcre2_chartables',
    output: 'pcre2_chartables.c',
    command: [pcre2_dftables, '@OUTPUT@'],
  )
elif not ebcdic
  chartables = configure_file(
    input: 'src/pcre2_chartables.c.dist',
    output: 'pcre2_chartables.c',
    copy: true,
  )
elif get_option('PCRE2_EBCDIC_NL25')
  chartables = configure_file(
    input: 'src/pcre2_chartables.c.ebcdic-1047-nl25',
    output: 'pcre2_chartables.c',
    copy: true,
  )
else
  chartables = configure_file(
    input: 'src/pcre2_chartables.c.ebcdic-1047-nl15',
    output: 'pcre2_chartables.c',
    copy: true,
  )
endif

pcre2_sources = files(
  'src/pcre2_auto_possess.c',
  'src/pcre2_chkdint.c',
  'src/pcre2_compile.c',
  'src/pcre2_compile_cgroup.c',
  'src/pcre2_compile_class.c',
  'src/pcre2_config.c',
  'src/pcre2_context.c',
  'src/pcre2_convert.c',
  'src/pcre2_dfa_match.c',
  'src/pcre2_error.c',
  'src/pcre2_extuni.c',
  'src/pcre2_find_bracket.c',
  'src/pcre2_jit_compile.c',
  'src/pcre2_maketables.c',
  'src/pcre2_match.c',
  'src/pcre2_match_data.c',
  'src/pcre2_match_next.c',
  'src/pcre2_newline.c',
  'src/pcre2_ord2utf.c',
  'src/pcre2_pattern_info.c',
  'src/pcre2_script_run.c',
  'src/pcre2_serialize.c',
  'src/pcre2_string_utils.c',
  'src/pcre2_study.c',
  'src/pcre2_substitute.c',
  'src/pcre2_substring.c',
  'src/pcre2_tables.c',
  'src/pcre2_ucd.c',
  'src/pcre2_valid_utf.c',
  'src/pcre2_xclass.c',
)
pcre2_sources += [chartables, config_h, pcre2_h]
pcre2_includes = include_directories('.', 'src')
private_deps = jit_enabled ? [thread_dep] : []
common_c_args = ['-DHAVE_CONFIG_H']
debug_option = get_option('PCRE2_DEBUG')
if debug_option == 'ON' or (debug_option == 'IfDebugBuild' and get_option('buildtype') == 'debug')
  common_c_args += '-DPCRE2_DEBUG'
endif

# Configure Linux/GNU symbol version scripts.
symbol_files = {}
foreach library : ['pcre2-8', 'pcre2-16', 'pcre2-32', 'pcre2-posix']
  symbol_files += {
    library: configure_file(
      input: 'src/lib@0@.sym.in'.format(library),
      output: 'lib@0@.sym'.format(library),
      configuration: {'PCRE2_EXTRA_LOCAL_SYMS': ''},
    ),
  }
endforeach
have_vscript = cc.has_link_argument(
  '-Wl,--version-script=' + meson.current_build_dir() / 'libpcre2-8.sym',
)
use_vscript = have_vscript and get_option('PCRE2_SYMVERS')

# Libraries. Separate targets preserve CMake's independent static/shared options.
foreach width : ['8', '16', '32']
  enabled = width == '8' ? build_8 : build_16
  if width == '32'
    enabled = build_32
  endif
  if not enabled
    continue
  endif
  name = 'pcre2-' + width
  width_args = common_c_args + ['-DPCRE2_CODE_UNIT_WIDTH=' + width]
  if build_static
    static_lib = static_library(
      name,
      pcre2_sources,
      include_directories: pcre2_includes,
      dependencies: private_deps,
      c_args: width_args,
      pic: get_option('PCRE2_STATIC_PIC'),
      install: true,
    )
    static_dep = declare_dependency(
      link_with: static_lib,
      include_directories: pcre2_includes,
      compile_args: '-DPCRE2_STATIC',
      dependencies: private_deps,
      version: meson.project_version(),
    )
    set_variable('pcre2_' + width + '_static_lib', static_lib)
    set_variable('pcre2_' + width + '_static_dep', static_dep)
  endif
  if build_shared
    link_args = use_vscript ? ['-Wl,--version-script=' + meson.current_build_dir() / ('lib' + name + '.sym')] : []
    shared_lib = shared_library(
      name,
      pcre2_sources,
      include_directories: pcre2_includes,
      dependencies: private_deps,
      c_args: width_args,
      link_args: link_args,
      link_depends: use_vscript ? symbol_files[name] : [],
      version: lib_versions[width],
      soversion: lib_soversions[width],
      gnu_symbol_visibility: 'hidden',
      install: true,
    )
    shared_dep = declare_dependency(
      link_with: shared_lib,
      include_directories: pcre2_includes,
      dependencies: private_deps,
      version: meson.project_version(),
    )
    set_variable('pcre2_' + width + '_shared_lib', shared_lib)
    set_variable('pcre2_' + width + '_shared_dep', shared_dep)
  endif
  selected_dep = build_static ? get_variable('pcre2_' + width + '_static_dep') : get_variable('pcre2_' + width + '_shared_dep')
  set_variable('libpcre2_' + width, selected_dep)
  meson.override_dependency('libpcre2-' + width, selected_dep)
endforeach

if build_8
  if build_static
    pcre2_posix_static = static_library(
      'pcre2-posix',
      'src/pcre2posix.c',
      config_h,
      pcre2_h,
      pcre2posix_h,
      include_directories: pcre2_includes,
      dependencies: pcre2_8_static_dep,
      c_args: common_c_args + ['-DPCRE2_CODE_UNIT_WIDTH=8'],
      pic: get_option('PCRE2_STATIC_PIC'),
      install: true,
    )
    pcre2_posix_static_dep = declare_dependency(
      link_with: pcre2_posix_static,
      include_directories: pcre2_includes,
      compile_args: '-DPCRE2_STATIC',
      dependencies: pcre2_8_static_dep,
      version: meson.project_version(),
    )
  endif
  if build_shared
    posix_link_args = use_vscript ? ['-Wl,--version-script=' + meson.current_build_dir() / 'libpcre2-posix.sym'] : []
    pcre2_posix_shared = shared_library(
      'pcre2-posix',
      'src/pcre2posix.c',
      config_h,
      pcre2_h,
      pcre2posix_h,
      include_directories: pcre2_includes,
      dependencies: pcre2_8_shared_dep,
      c_args: common_c_args + ['-DPCRE2_CODE_UNIT_WIDTH=8', '-DPCRE2POSIX_SHARED'],
      link_args: posix_link_args,
      link_depends: use_vscript ? symbol_files['pcre2-posix'] : [],
      version: lib_versions['posix'],
      soversion: lib_soversions['posix'],
      gnu_symbol_visibility: 'hidden',
      install: true,
    )
    pcre2_posix_shared_dep = declare_dependency(
      link_with: pcre2_posix_shared,
      include_directories: pcre2_includes,
      compile_args: '-DPCRE2POSIX_SHARED',
      dependencies: pcre2_8_shared_dep,
      version: meson.project_version(),
    )
  endif
  libpcre2_posix = build_static ? pcre2_posix_static_dep : pcre2_posix_shared_dep
  meson.override_dependency('libpcre2-posix', libpcre2_posix)
endif

# Generate pkg-config files.
pkg = import('pkgconfig')
foreach width : ['8', '16', '32']
  enabled = width == '8' ? build_8 : build_16
  if width == '32'
    enabled = build_32
  endif
  if enabled
    pkg_lib = build_shared ? get_variable('pcre2_' + width + '_shared_lib') : get_variable('pcre2_' + width + '_static_lib')
    pkg.generate(
      pkg_lib,
      filebase: 'libpcre2-' + width,
      name: 'libpcre2-' + width,
      description: 'PCRE2 - Perl compatible regular expressions C library (2nd API) with @0@ bit character support'.format(width),
      extra_cflags: build_shared ? [] : ['-DPCRE2_STATIC'],
      version: meson.project_version(),
    )
  endif
endforeach
if build_8
  pkg_posix_lib = build_shared ? pcre2_posix_shared : pcre2_posix_static
  pkg.generate(
    pkg_posix_lib,
    filebase: 'libpcre2-posix',
    name: 'libpcre2-posix',
    description: 'Posix compatible interface to libpcre2-8',
    extra_cflags: build_shared ? ['-DPCRE2POSIX_SHARED'] : ['-DPCRE2_STATIC'],
    requires_private: 'libpcre2-8',
    version: meson.project_version(),
  )
endif

pcre2_config_data = {
  'prefix': get_option('prefix'),
  'exec_prefix': '${prefix}',
  'libdir': '${exec_prefix}/' + get_option('libdir'),
  'includedir': '${prefix}/' + get_option('includedir'),
  'PACKAGE_VERSION': meson.project_version(),
  'enable_pcre2_8': build_8 ? 'yes' : 'no',
  'enable_pcre2_16': build_16 ? 'yes' : 'no',
  'enable_pcre2_32': build_32 ? 'yes' : 'no',
  'PCRE2_STATIC_CFLAG': build_shared ? '' : '-DPCRE2_STATIC',
  'PCRE2POSIX_CFLAG': build_shared ? '-DPCRE2POSIX_SHARED' : '',
  'LIB_POSTFIX': '',
}
pcre2_config = configure_file(
  input: 'pcre2-config.in',
  output: 'pcre2-config',
  configuration: pcre2_config_data,
  install: true,
  install_dir: get_option('bindir'),
  install_mode: 'rwxr-xr-x',
)

# CMake package files. Meson has no target-export API, so define the installed
# Linux targets explicitly while retaining the public CMake package interface.
cmake_install_dir = get_option('PCRE2_INSTALL_CMAKEDIR')
if cmake_install_dir == ''
  cmake_install_dir = get_option('libdir') / 'cmake/pcre2'
endif
cmake_target_definitions = ''
cmake_thread_interface = jit_enabled ? ' INTERFACE_LINK_LIBRARIES "Threads::Threads"' : ''
foreach width : ['8', '16', '32']
  enabled = width == '8' ? build_8 : build_16
  if width == '32'
    enabled = build_32
  endif
  if not enabled
    continue
  endif
  if build_static
    cmake_target_definitions += '''
add_library(pcre2::pcre2-@0@-static STATIC IMPORTED)
set_target_properties(pcre2::pcre2-@0@-static PROPERTIES
  IMPORTED_LOCATION "${PACKAGE_PREFIX_DIR}/@1@/libpcre2-@0@.a"
  INTERFACE_COMPILE_DEFINITIONS "PCRE2_STATIC"
  INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/@2@"@3@)
'''.format(width, get_option('libdir'), get_option('includedir'), cmake_thread_interface)
  endif
  if build_shared
    cmake_target_definitions += '''
add_library(pcre2::pcre2-@0@-shared SHARED IMPORTED)
set_target_properties(pcre2::pcre2-@0@-shared PROPERTIES
  IMPORTED_LOCATION "${PACKAGE_PREFIX_DIR}/@1@/libpcre2-@0@.so"
  INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/@2@"@3@)
'''.format(width, get_option('libdir'), get_option('includedir'), cmake_thread_interface)
  endif
endforeach
if build_8
  if build_static
    cmake_target_definitions += '''
add_library(pcre2::pcre2-posix-static STATIC IMPORTED)
set_target_properties(pcre2::pcre2-posix-static PROPERTIES
  IMPORTED_LOCATION "${PACKAGE_PREFIX_DIR}/@0@/libpcre2-posix.a"
  INTERFACE_COMPILE_DEFINITIONS "PCRE2_STATIC"
  INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/@1@"
  INTERFACE_LINK_LIBRARIES "pcre2::pcre2-8-static")
'''.format(get_option('libdir'), get_option('includedir'))
  endif
  if build_shared
    cmake_target_definitions += '''
add_library(pcre2::pcre2-posix-shared SHARED IMPORTED)
set_target_properties(pcre2::pcre2-posix-shared PROPERTIES
  IMPORTED_LOCATION "${PACKAGE_PREFIX_DIR}/@0@/libpcre2-posix.so"
  INTERFACE_COMPILE_DEFINITIONS "PCRE2POSIX_SHARED"
  INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/@1@"
  INTERFACE_LINK_LIBRARIES "pcre2::pcre2-8-shared")
'''.format(get_option('libdir'), get_option('includedir'))
  endif
endif
configure_file(
  input: 'meson/pcre2-targets.cmake.in',
  output: 'pcre2-targets.cmake',
  configuration: {'TARGET_DEFINITIONS': cmake_target_definitions},
  install: true,
  install_dir: cmake_install_dir,
)
cmake = import('cmake')
cmake.configure_package_config_file(
  name: 'pcre2',
  input: 'cmake/pcre2-config.cmake.in',
  install_dir: cmake_install_dir,
  configuration: {
    'REQUIRE_PTHREAD': jit_enabled ? '1' : '0',
    'PCRE2_MAJOR': pcre2_major,
    'PCRE2_MINOR': pcre2_minor,
  },
)
cmake.write_basic_package_version_file(
  name: 'pcre2',
  version: meson.project_version() + '.0',
  compatibility: 'SameMajorVersion',
  install_dir: cmake_install_dir,
)

# Executables.
if build_grep
  grep_deps = [libpcre2_posix]
  if zlib_dep.found()
    grep_deps += zlib_dep
  endif
  if bzip2_dep.found()
    grep_deps += bzip2_dep
  endif
  pcre2grep = executable(
    'pcre2grep',
    'src/pcre2grep.c',
    config_h,
    pcre2_h,
    include_directories: pcre2_includes,
    dependencies: grep_deps,
    c_args: common_c_args + ['-DPCRE2_CODE_UNIT_WIDTH=8'],
    install: true,
  )
endif

if get_option('PCRE2_BUILD_TESTS')
  test_deps = []
  foreach width : ['8', '16', '32']
    enabled = width == '8' ? build_8 : build_16
    if width == '32'
      enabled = build_32
    endif
    if enabled
      test_deps += get_variable('libpcre2_' + width)
    endif
  endforeach
  if build_8
    test_deps += libpcre2_posix
  endif
  if readline_dep.found()
    test_deps += readline_dep
  elif editline_dep.found()
    test_deps += editline_dep
  endif
  pcre2test = executable(
    'pcre2test',
    'src/pcre2test.c',
    config_h,
    pcre2_h,
    include_directories: pcre2_includes,
    dependencies: test_deps,
    c_args: common_c_args,
  )
  if build_8
    pcre2posix_test = executable(
      'pcre2posix_test',
      'src/pcre2posix_test.c',
      dependencies: [libpcre2_posix, libpcre2_8],
      include_directories: pcre2_includes,
    )
    test('pcre2posix_test', pcre2posix_test)
  endif
  if jit_enabled
    pcre2_jit_test = executable(
      'pcre2_jit_test',
      'src/pcre2_jit_test.c',
      dependencies: test_deps,
      include_directories: pcre2_includes,
    )
    test('pcre2_jit_test', pcre2_jit_test)
  endif

  python = find_program('python3', required: false)
  if python.found()
    test(
      'pcre2_test',
      python,
      args: [files('RunTest.py'), '--srcdir', meson.project_source_root(), '--pcre2test', pcre2test.full_path()],
      workdir: meson.project_build_root(),
      timeout: 300,
    )
    if build_grep
      test(
        'pcre2_grep_test',
        python,
        args: [
          files('RunGrepTest.py'),
          '--srcdir', meson.project_source_root(),
          '--pcre2test', pcre2test.full_path(),
          '--pcre2grep', pcre2grep.full_path(),
        ],
        workdir: meson.project_build_root(),
        should_fail: ebcdic,
        timeout: 300,
      )
    endif
  else
    shell = find_program('sh')
    test(
      'pcre2_test',
      shell,
      args: [files('RunTest'), '--srcdir', meson.project_source_root(), '--pcre2test', pcre2test.full_path()],
      workdir: meson.project_build_root(),
      timeout: 300,
    )
    if build_grep
      test(
        'pcre2_grep_test',
        shell,
        args: [
          files('RunGrepTest'),
          '--srcdir', meson.project_source_root(),
          '--pcre2test', pcre2test.full_path(),
          '--pcre2grep', pcre2grep.full_path(),
        ],
        workdir: meson.project_build_root(),
        should_fail: ebcdic,
        timeout: 300,
      )
    endif
  endif
endif

# Fuzzer support.
if get_option('PCRE2_FUZZ_SUPPORT')
  foreach width : ['8', '16', '32']
    enabled = width == '8' ? build_8 : build_16
    if width == '32'
      enabled = build_32
    endif
    if not enabled
      continue
    endif
    suffix = width == '8' ? '' : '-' + width
    static_library(
      'pcre2-fuzzsupport' + suffix,
      'src/pcre2_fuzzsupport.c',
      config_h,
      pcre2_h,
      include_directories: pcre2_includes,
      c_args: common_c_args + ['-DPCRE2_CODE_UNIT_WIDTH=' + width],
    )
    executable(
      'pcre2fuzzcheck-' + width,
      'src/pcre2_fuzzsupport.c',
      config_h,
      pcre2_h,
      include_directories: pcre2_includes,
      dependencies: get_variable('libpcre2_' + width),
      c_args: common_c_args + ['-DSTANDALONE', '-DPCRE2_CODE_UNIT_WIDTH=' + width],
    )
  endforeach
endif

# Installation.
install_headers(pcre2_h, pcre2posix_h)
install_man(
  'doc/pcre2-config.1',
  'doc/pcre2grep.1',
  'doc/pcre2test.1',
  'doc/pcre2.3',
  'doc/pcre2_callout_enumerate.3',
  'doc/pcre2_code_copy.3',
  'doc/pcre2_code_copy_with_tables.3',
  'doc/pcre2_code_free.3',
  'doc/pcre2_compile.3',
  'doc/pcre2_compile_context_copy.3',
  'doc/pcre2_compile_context_create.3',
  'doc/pcre2_compile_context_free.3',
  'doc/pcre2_config.3',
  'doc/pcre2_convert_context_copy.3',
  'doc/pcre2_convert_context_create.3',
  'doc/pcre2_convert_context_free.3',
  'doc/pcre2_converted_pattern_free.3',
  'doc/pcre2_dfa_match.3',
  'doc/pcre2_general_context_copy.3',
  'doc/pcre2_general_context_create.3',
  'doc/pcre2_general_context_free.3',
  'doc/pcre2_get_bsr.3',
  'doc/pcre2_get_depth_limit.3',
  'doc/pcre2_get_error_message.3',
  'doc/pcre2_get_heap_limit.3',
  'doc/pcre2_get_mark.3',
  'doc/pcre2_get_match_data_heapframes_size.3',
  'doc/pcre2_get_match_data_size.3',
  'doc/pcre2_get_match_limit.3',
  'doc/pcre2_get_max_pattern_length.3',
  'doc/pcre2_get_newline.3',
  'doc/pcre2_get_offset_limit.3',
  'doc/pcre2_get_ovector_count.3',
  'doc/pcre2_get_ovector_pointer.3',
  'doc/pcre2_get_parens_nest_limit.3',
  'doc/pcre2_get_startchar.3',
  'doc/pcre2_jit_compile.3',
  'doc/pcre2_jit_free_unused_memory.3',
  'doc/pcre2_jit_match.3',
  'doc/pcre2_jit_stack_assign.3',
  'doc/pcre2_jit_stack_create.3',
  'doc/pcre2_jit_stack_free.3',
  'doc/pcre2_maketables.3',
  'doc/pcre2_maketables_free.3',
  'doc/pcre2_match.3',
  'doc/pcre2_match_context_copy.3',
  'doc/pcre2_match_context_create.3',
  'doc/pcre2_match_context_free.3',
  'doc/pcre2_match_data_create.3',
  'doc/pcre2_match_data_create_from_pattern.3',
  'doc/pcre2_match_data_free.3',
  'doc/pcre2_next_match.3',
  'doc/pcre2_pattern_convert.3',
  'doc/pcre2_pattern_info.3',
  'doc/pcre2_serialize_decode.3',
  'doc/pcre2_serialize_encode.3',
  'doc/pcre2_serialize_free.3',
  'doc/pcre2_serialize_get_number_of_codes.3',
  'doc/pcre2_set_bsr.3',
  'doc/pcre2_set_callout.3',
  'doc/pcre2_set_character_tables.3',
  'doc/pcre2_set_compile_extra_options.3',
  'doc/pcre2_set_compile_recursion_guard.3',
  'doc/pcre2_set_depth_limit.3',
  'doc/pcre2_set_glob_escape.3',
  'doc/pcre2_set_glob_separator.3',
  'doc/pcre2_set_heap_limit.3',
  'doc/pcre2_set_match_limit.3',
  'doc/pcre2_set_max_pattern_compiled_length.3',
  'doc/pcre2_set_max_pattern_length.3',
  'doc/pcre2_set_max_varlookbehind.3',
  'doc/pcre2_set_newline.3',
  'doc/pcre2_set_offset_limit.3',
  'doc/pcre2_set_optimize.3',
  'doc/pcre2_set_parens_nest_limit.3',
  'doc/pcre2_set_recursion_limit.3',
  'doc/pcre2_set_recursion_memory_management.3',
  'doc/pcre2_set_substitute_callout.3',
  'doc/pcre2_set_substitute_case_callout.3',
  'doc/pcre2_substitute.3',
  'doc/pcre2_substring_copy_byname.3',
  'doc/pcre2_substring_copy_bynumber.3',
  'doc/pcre2_substring_free.3',
  'doc/pcre2_substring_get_byname.3',
  'doc/pcre2_substring_get_bynumber.3',
  'doc/pcre2_substring_length_byname.3',
  'doc/pcre2_substring_length_bynumber.3',
  'doc/pcre2_substring_list_free.3',
  'doc/pcre2_substring_list_get.3',
  'doc/pcre2_substring_nametable_scan.3',
  'doc/pcre2_substring_number_from_name.3',
  'doc/pcre2api.3',
  'doc/pcre2build.3',
  'doc/pcre2callout.3',
  'doc/pcre2compat.3',
  'doc/pcre2convert.3',
  'doc/pcre2demo.3',
  'doc/pcre2jit.3',
  'doc/pcre2limits.3',
  'doc/pcre2matching.3',
  'doc/pcre2partial.3',
  'doc/pcre2pattern.3',
  'doc/pcre2perform.3',
  'doc/pcre2posix.3',
  'doc/pcre2sample.3',
  'doc/pcre2serialize.3',
  'doc/pcre2syntax.3',
  'doc/pcre2unicode.3',
)
install_data(
  'AUTHORS.md',
  'COPYING',
  'ChangeLog',
  'LICENCE.md',
  'NEWS',
  'README',
  'SECURITY.md',
  'SUPPORT-LIFECYCLE.md',
  'doc/pcre2.txt',
  'doc/pcre2-config.txt',
  'doc/pcre2grep.txt',
  'doc/pcre2test.txt',
  install_dir: get_option('datadir') / 'doc/pcre2',
)
install_subdir(
  'doc/html',
  install_dir: get_option('datadir') / 'doc/pcre2',
)

if get_option('PCRE2_HEAP_MATCH_RECURSE')
  warning('PCRE2_HEAP_MATCH_RECURSE is obsolete and does nothing.')
endif
if get_option('PCRE2_SHOW_REPORT')
  summary(
    {
      'Install prefix': get_option('prefix'),
      'C compiler': cc.get_id(),
      'Build 8 bit pcre2 library': build_8,
      'Build 16 bit pcre2 library': build_16,
      'Build 32 bit pcre2 library': build_32,
      'Include debugging code': debug_option,
      'Enable JIT compiling support': jit_enabled,
      'Use SELinux allocator in JIT': get_option('PCRE2_SUPPORT_JIT_SEALLOC'),
      'Enable Unicode support': get_option('PCRE2_SUPPORT_UNICODE'),
      'Newline char/sequence': get_option('PCRE2_NEWLINE'),
      '\\R matches only ANYCRLF': get_option('PCRE2_SUPPORT_BSR_ANYCRLF'),
      '\\C is disabled': get_option('PCRE2_NEVER_BACKSLASH_C'),
      'EBCDIC coding': ebcdic,
      'Rebuild char tables': rebuild_chartables,
      'Internal link size': get_option('PCRE2_LINK_SIZE'),
      'Maximum variable lookbehind': get_option('PCRE2_MAX_VARLOOKBEHIND'),
      'Nested parentheses limit': get_option('PCRE2_PARENS_NEST_LIMIT'),
      'Heap limit': get_option('PCRE2_HEAP_LIMIT'),
      'Match limit': get_option('PCRE2_MATCH_LIMIT'),
      'Match depth limit': get_option('PCRE2_MATCH_LIMIT_DEPTH'),
      'Build shared libs': build_shared,
      'with symbol versioning': use_vscript,
      'Build static libs': build_static,
      'with PIC enabled': get_option('PCRE2_STATIC_PIC'),
      'Build pcre2grep': build_grep,
      'Build tests': get_option('PCRE2_BUILD_TESTS'),
      'Link pcre2grep with libz': zlib_dep.found(),
      'Link pcre2grep with libbz2': bzip2_dep.found(),
      'Link pcre2test with libeditline': editline_dep.found(),
      'Link pcre2test with libreadline': readline_dep.found(),
      'Enable Valgrind support': get_option('PCRE2_SUPPORT_VALGRIND'),
      'Fuzzer support': get_option('PCRE2_FUZZ_SUPPORT'),
      'Differential fuzzer support': get_option('PCRE2_DIFF_FUZZ_SUPPORT'),
    },
    section: 'PCRE2-' + meson.project_version() + ' configuration',
  )
endif
