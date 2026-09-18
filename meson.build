# -----------------------------------------------------------------------------
# Project setup
# -----------------------------------------------------------------------------

project(
  'PCRE2',
  'c',
  version: '10.49',
  license: 'BSD-3-Clause WITH PCRE2-exception',
  meson_version: '>=1.3.0',
  default_options: ['c_std=c11,c99'],
)

cc = meson.get_compiler('c')
host_is_windows = host_machine.system() == 'windows'
msvc_like = cc.get_id() in ['msvc', 'clang-cl', 'intel-cl']
mingw = host_is_windows and not msvc_like
windows_debug_postfix = host_is_windows and get_option('buildtype') == 'debug' ? 'd' : ''

if msvc_like
  add_project_arguments('-D_CRT_SECURE_NO_DEPRECATE', '-D_CRT_SECURE_NO_WARNINGS', language: 'c')
endif

# -----------------------------------------------------------------------------
# Release and library version information
# -----------------------------------------------------------------------------

project_version_parts = meson.project_version().split('.')
pcre2_major = project_version_parts[0]
pcre2_minor = project_version_parts[1]
pcre2_prerelease = '-DEV'
pcre2_date = '2026-11-31'
libtool_versions = {
  'posix': '3:8:0',
  '8': '16:0:16',
  '16': '16:0:16',
  '32': '16:0:16',
}

lib_versions = {}
lib_soversions = {}
lib_darwin_versions = {}
foreach library : ['posix', '8', '16', '32']
  parts = libtool_versions[library].split(':')
  current = parts[0].to_int()
  revision = parts[1]
  age = parts[2]
  soversion = current - age.to_int()
  lib_soversions += {library: soversion.to_string()}
  lib_versions += {library: '@0@.@1@.@2@'.format(soversion, age, revision)}
  darwin_compatibility = current + 1
  lib_darwin_versions += {
    library: [darwin_compatibility.to_string(), '@0@.@1@'.format(darwin_compatibility, revision)],
  }
endforeach

# -----------------------------------------------------------------------------
# Configuration checks
# -----------------------------------------------------------------------------
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
config.set('HAVE_STDATOMIC_H', cc.compiles(
  '''
  #ifdef __STDC_NO_ATOMICS__
  #error "C compiler does not support standard C atomics"
  #endif
  #include <stdatomic.h>
  int main(void) {
    atomic_int value;
    atomic_init(&value, 0);
    return atomic_load(&value);
  }
  ''',
  name: 'standard C atomics',
))
config.set('HAVE_REALPATH', cc.compiles(
  '''
  #include <stdlib.h>
  #include <limits.h>
  int main(int c, char *v[]) {
    char buf[PATH_MAX];
    return realpath(v[c], buf) == NULL;
  }
  ''',
  args: system_extension_args,
  name: 'realpath',
))

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
config.set('HAVE_BUILTIN_ASSUME', cc.links(
  'int main(void) { __assume(1); return 0; }',
  args: warning_as_error,
  name: '__assume',
))
config.set('HAVE_BUILTIN_MUL_OVERFLOW', cc.links(
  '''
  #include <stddef.h>
  int main(void) { int a, b; size_t m; __builtin_mul_overflow(a, b, &m); return 0; }
  ''',
  args: warning_as_error,
  name: '__builtin_mul_overflow',
))
config.set('HAVE_BUILTIN_UNREACHABLE', cc.links(
  'int main(int c, char *v[]) { if (c) __builtin_unreachable(); return (int)(*v[0]); }',
  args: warning_as_error,
  name: '__builtin_unreachable',
))

if cc.compiles(
  '#ifndef __CET__\n#error CET is not enabled\n#endif\nint main(void) { return 0; }',
  name: 'Intel CET',
)
  add_project_arguments(cc.get_supported_arguments('-mshstk'), language: 'c')
endif

# -----------------------------------------------------------------------------
# User-configurable options
# -----------------------------------------------------------------------------
build_shared = get_option('build_shared_libs')
build_static = get_option('build_static_libs')
build_8 = get_option('pcre2_build_pcre2_8')
build_16 = get_option('pcre2_build_pcre2_16')
build_32 = get_option('pcre2_build_pcre2_32')
if not build_shared and not build_static
  error('At least one of build_shared_libs or build_static_libs must be enabled.')
endif
if not build_8 and not build_16 and not build_32
  error('At least one PCRE2 code unit width must be enabled.')
endif

build_grep = get_option('pcre2_build_pcre2grep')
if build_grep and not build_8
  message('PCRE2_BUILD_PCRE2_8 must be enabled for pcre2grep; disabling pcre2grep')
  build_grep = false
endif

jit_option = get_option('pcre2_support_jit')
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

ebcdic = get_option('pcre2_ebcdic') or get_option('pcre2_ebcdic_nl25') or get_option('pcre2_ebcdic_ignoring_compiler')
rebuild_chartables = get_option('pcre2_rebuild_chartables')
if ebcdic
  if not get_option('pcre2_ebcdic_ignoring_compiler')
    rebuild_chartables = true
  endif
  if get_option('pcre2_support_unicode')
    error('Support for EBCDIC and Unicode cannot be enabled at the same time.')
  endif
  if build_16 or build_32
    error('EBCDIC support is available only for the 8-bit library.')
  endif
endif

if get_option('pcre2_fuzz_support') and not build_8
  error('Fuzzer support requires PCRE2_BUILD_PCRE2_8.')
endif
if get_option('pcre2_diff_fuzz_support')
  if not get_option('pcre2_fuzz_support')
    error('Differential fuzzing requires PCRE2_FUZZ_SUPPORT.')
  endif
  if not jit_enabled
    error('Differential fuzzing requires PCRE2_SUPPORT_JIT.')
  endif
endif

# -----------------------------------------------------------------------------
# External packages
# -----------------------------------------------------------------------------

thread_dep = dependency('threads', required: jit_enabled)
bzip2_option = get_option('pcre2_support_libbz2')
bzip2_dep = dependency('', required: false)
if not bzip2_option.disabled()
  bzip2_dep = dependency('bzip2', required: false)
  if not bzip2_dep.found()
    bzip2_dep = dependency('BZip2', method: 'cmake', required: bzip2_option)
  endif
endif
zlib_dep = dependency('zlib', required: get_option('pcre2_support_libz'))
editline_dep = dependency('libedit', required: get_option('pcre2_support_libedit'))
readline_dep = dependency('readline', required: get_option('pcre2_support_libreadline'))
if editline_dep.found() and readline_dep.found()
  error('Only one of PCRE2_SUPPORT_LIBREADLINE and PCRE2_SUPPORT_LIBEDIT can be enabled.')
endif

# -----------------------------------------------------------------------------
# Build configuration
# -----------------------------------------------------------------------------

newline_values = {'CR': 1, 'LF': 2, 'CRLF': 3, 'ANY': 4, 'ANYCRLF': 5, 'NUL': 6}
config.set('NEWLINE_DEFAULT', newline_values[get_option('pcre2_newline')])
config.set('SUPPORT_PCRE2_8', build_8)
config.set('SUPPORT_PCRE2_16', build_16)
config.set('SUPPORT_PCRE2_32', build_32)
config.set('DISABLE_PERCENT_ZT', get_option('pcre2_disable_percent_zt'))
config.set('SUPPORT_LIBBZ2', bzip2_dep.found())
config.set('SUPPORT_LIBEDIT', editline_dep.found())
config.set('SUPPORT_LIBREADLINE', readline_dep.found())
config.set('SUPPORT_LIBZ', zlib_dep.found())
config.set('SUPPORT_JIT', jit_enabled)
config.set('SLJIT_PROT_EXECUTABLE_ALLOCATOR', get_option('pcre2_support_jit_sealloc'))
config.set('SUPPORT_PCRE2GREP_JIT', get_option('pcre2grep_support_jit'))
config.set('SUPPORT_PCRE2GREP_CALLOUT', get_option('pcre2grep_support_callout'))
config.set('SUPPORT_PCRE2GREP_CALLOUT_FORK',
  get_option('pcre2grep_support_callout') and get_option('pcre2grep_support_callout_fork'))
config.set('SUPPORT_UNICODE', get_option('pcre2_support_unicode'))
config.set('SUPPORT_VALGRIND', get_option('pcre2_support_valgrind'))
config.set('SUPPORT_DIFF_FUZZ', get_option('pcre2_diff_fuzz_support'))
config.set('BSR_ANYCRLF', get_option('pcre2_support_bsr_anycrlf'))
config.set('EBCDIC', ebcdic)
config.set('EBCDIC_NL25', get_option('pcre2_ebcdic_nl25'))
config.set('EBCDIC_IGNORING_COMPILER', get_option('pcre2_ebcdic_ignoring_compiler'))
config.set('NEVER_BACKSLASH_C', get_option('pcre2_never_backslash_c'))
config_options = {
  'PCRE2_LINK_SIZE': 'pcre2_link_size',
  'PCRE2_HEAP_LIMIT': 'pcre2_heap_limit',
  'PCRE2_MATCH_LIMIT': 'pcre2_match_limit',
  'PCRE2_MATCH_LIMIT_DEPTH': 'pcre2_match_limit_depth',
  'PCRE2_MAX_VARLOOKBEHIND': 'pcre2_max_varlookbehind',
  'PCRE2_PARENS_NEST_LIMIT': 'pcre2_parens_nest_limit',
  'PCRE2GREP_BUFSIZE': 'pcre2grep_bufsize',
  'PCRE2GREP_MAX_BUFSIZE': 'pcre2grep_max_bufsize',
}
foreach config_name, option_name : config_options
  config.set(config_name, get_option(option_name))
endforeach

# -----------------------------------------------------------------------------
# Generated files
# -----------------------------------------------------------------------------

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

# -----------------------------------------------------------------------------
# Character table generation
# -----------------------------------------------------------------------------

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
elif get_option('pcre2_ebcdic_nl25')
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

# -----------------------------------------------------------------------------
# Source code
# -----------------------------------------------------------------------------

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
fs = import('fs')
pcre2_resource = []
pcre2_posix_resource = []
if host_is_windows and build_shared
  windows_module = import('windows')
  if fs.exists('pcre2.rc')
    pcre2_resource = windows_module.compile_resources('pcre2.rc')
  endif
  if fs.exists('pcre2posix.rc')
    pcre2_posix_resource = windows_module.compile_resources('pcre2posix.rc')
  endif
endif
pcre2_includes = include_directories('.', 'src')
private_deps = jit_enabled ? [thread_dep] : []
common_c_args = ['-DHAVE_CONFIG_H']
debug_option = get_option('pcre2_debug')
if debug_option == 'ON' or (debug_option == 'IfDebugBuild' and get_option('buildtype') == 'debug')
  common_c_args += '-DPCRE2_DEBUG'
endif

# Configure GNU version scripts or Sun linker map files.
symbol_files = {}
foreach library : ['pcre2-8', 'pcre2-16', 'pcre2-32', 'pcre2-posix']
  symbol_files += {
    library: configure_file(
      input: 'src/lib@0@.sym.in'.format(library),
      output: 'lib@0@.sym'.format(library),
      configuration: {'PCRE2_EXTRA_LOCAL_SYMS': '  local: *;'},
    ),
  }
endforeach
vscript_flag = ''
if not msvc_like and get_option('pcre2_symvers')
  vscript_path = meson.current_build_dir() / 'libpcre2-8.sym'
  if cc.has_link_argument('-Wl,--version-script=' + vscript_path)
    vscript_flag = '--version-script'
  elif cc.has_link_argument('-Wl,-M,' + vscript_path)
    vscript_flag = '-M'
  endif
endif
use_vscript = vscript_flag != ''

# -----------------------------------------------------------------------------
# Libraries
# -----------------------------------------------------------------------------

# Separate targets preserve CMake's independent static/shared options.
static_output_names = {}
shared_output_names = {}
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
    static_name = name + windows_debug_postfix
    if msvc_like
      static_name = name + '-static' + windows_debug_postfix
    endif
    static_lib = static_library(
      static_name,
      pcre2_sources,
      include_directories: pcre2_includes,
      dependencies: private_deps,
      c_args: width_args + ['-DPCRE2_STATIC'],
      pic: get_option('pcre2_static_pic'),
      name_suffix: msvc_like ? 'lib' : [],
      gnu_symbol_visibility: 'hidden',
      install: true,
    )
    static_output_names += {width: fs.name(static_lib.full_path())}
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
    shared_name = name + windows_debug_postfix
    shared_name_prefix = mingw and get_option('non_standard_lib_prefix') ? '' : []
    link_args = use_vscript ? ['-Wl,' + vscript_flag + ',' + meson.current_build_dir() / ('lib' + name + '.sym')] : []
    shared_kwargs = {
      'include_directories': pcre2_includes,
      'dependencies': private_deps,
      'c_args': width_args,
      'link_args': link_args,
      'link_depends': use_vscript ? symbol_files[name] : [],
      'name_prefix': shared_name_prefix,
      'gnu_symbol_visibility': 'hidden',
      'install': true,
    }
    if mingw and get_option('non_standard_lib_suffix')
      shared_kwargs += {'soversion': '0'}
    endif
    if not host_is_windows
      shared_kwargs += {
        'version': lib_versions[width],
        'soversion': lib_soversions[width],
      }
    endif
    if host_machine.system() == 'darwin'
      shared_kwargs += {'darwin_versions': lib_darwin_versions[width]}
    endif
    shared_lib = shared_library(
      shared_name,
      pcre2_sources + pcre2_resource,
      kwargs: shared_kwargs,
    )
    shared_output_names += {width: fs.name(shared_lib.full_path())}
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
    posix_static_name = 'pcre2-posix' + windows_debug_postfix
    if msvc_like
      posix_static_name = 'pcre2-posix-static' + windows_debug_postfix
    endif
    pcre2_posix_static = static_library(
      posix_static_name,
      'src/pcre2posix.c',
      config_h,
      pcre2_h,
      pcre2posix_h,
      include_directories: pcre2_includes,
      dependencies: pcre2_8_static_dep,
      c_args: common_c_args + ['-DPCRE2_CODE_UNIT_WIDTH=8', '-DPCRE2_STATIC'],
      pic: get_option('pcre2_static_pic'),
      name_suffix: msvc_like ? 'lib' : [],
      gnu_symbol_visibility: 'hidden',
      install: true,
    )
    static_output_names += {'posix': fs.name(pcre2_posix_static.full_path())}
    pcre2_posix_static_dep = declare_dependency(
      link_with: pcre2_posix_static,
      include_directories: pcre2_includes,
      compile_args: '-DPCRE2_STATIC',
      dependencies: pcre2_8_static_dep,
      version: meson.project_version(),
    )
  endif
  if build_shared
    posix_shared_name = 'pcre2-posix' + windows_debug_postfix
    posix_shared_name_prefix = mingw and get_option('non_standard_lib_prefix') ? '' : []
    posix_link_args = use_vscript ? ['-Wl,' + vscript_flag + ',' + meson.current_build_dir() / 'libpcre2-posix.sym'] : []
    posix_shared_kwargs = {
      'include_directories': pcre2_includes,
      'dependencies': pcre2_8_shared_dep,
      'c_args': common_c_args + ['-DPCRE2_CODE_UNIT_WIDTH=8', '-DPCRE2POSIX_SHARED'],
      'link_args': posix_link_args,
      'link_depends': use_vscript ? symbol_files['pcre2-posix'] : [],
      'name_prefix': posix_shared_name_prefix,
      'gnu_symbol_visibility': 'hidden',
      'install': true,
    }
    if mingw and get_option('non_standard_lib_suffix')
      posix_shared_kwargs += {'soversion': '0'}
    endif
    if not host_is_windows
      posix_shared_kwargs += {
        'version': lib_versions['posix'],
        'soversion': lib_soversions['posix'],
      }
    endif
    if host_machine.system() == 'darwin'
      posix_shared_kwargs += {'darwin_versions': lib_darwin_versions['posix']}
    endif
    pcre2_posix_shared = shared_library(
      posix_shared_name,
      'src/pcre2posix.c',
      pcre2_posix_resource,
      config_h,
      pcre2_h,
      pcre2posix_h,
      kwargs: posix_shared_kwargs,
    )
    shared_output_names += {'posix': fs.name(pcre2_posix_shared.full_path())}
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

# -----------------------------------------------------------------------------
# Package metadata
# -----------------------------------------------------------------------------

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

# CMake package files. Meson has no target-export API, so define imported
# targets using the filenames generated for this host platform.
cmake_install_dir = get_option('pcre2_install_cmakedir')
if cmake_install_dir == ''
  cmake_install_dir = get_option('libdir') / 'cmake/pcre2'
endif
cmake_target_definitions = ''
cmake_release_properties = ''
cmake_thread_interface = jit_enabled ? '  INTERFACE_LINK_LIBRARIES "Threads::Threads"\n' : ''
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
  INTERFACE_COMPILE_DEFINITIONS "PCRE2_STATIC"
  INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/@1@"
@2@)
'''.format(width, get_option('includedir'), cmake_thread_interface)
    cmake_release_properties += '''
set_property(TARGET pcre2::pcre2-@0@-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(pcre2::pcre2-@0@-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${PACKAGE_PREFIX_DIR}/@1@/@2@")
'''.format(width, get_option('libdir'), static_output_names[width])
  endif
  if build_shared
    cmake_target_definitions += '''
add_library(pcre2::pcre2-@0@-shared SHARED IMPORTED)
set_target_properties(pcre2::pcre2-@0@-shared PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/@1@"
@2@)
'''.format(width, get_option('includedir'), cmake_thread_interface)
    shared_install_name = shared_output_names[width]
    shared_soname_property = ''
    shared_import_property = ''
    shared_install_dir = get_option('libdir')
    if host_is_windows
      shared_install_dir = get_option('bindir')
      import_filename = shared_output_names[width].replace('.dll', '')
      if mingw and get_option('non_standard_lib_suffix')
        import_filename = import_filename.replace('-0', '')
      endif
      import_filename += msvc_like ? '.lib' : '.dll.a'
      shared_import_property = '  IMPORTED_IMPLIB_RELEASE "${PACKAGE_PREFIX_DIR}/@0@/@1@"\n'.format(
        get_option('libdir'), import_filename)
    elif host_machine.system() == 'darwin'
      shared_soname_property = '  IMPORTED_SONAME_RELEASE "@rpath/@0@"\n'.format(shared_install_name)
    elif host_machine.system() != 'cygwin'
      unversioned_name = shared_output_names[width].replace('.' + lib_versions[width], '')
      shared_soname_property = '  IMPORTED_SONAME_RELEASE "@0@.@1@"\n'.format(
        unversioned_name, lib_soversions[width])
    endif
    cmake_release_properties += '''
set_property(TARGET pcre2::pcre2-@0@-shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(pcre2::pcre2-@0@-shared PROPERTIES
  IMPORTED_LOCATION_RELEASE "${PACKAGE_PREFIX_DIR}/@1@/@2@"
@3@@4@)
'''.format(width, shared_install_dir, shared_install_name, shared_import_property, shared_soname_property)
  endif
endforeach
if build_8
  if build_static
    cmake_target_definitions += '''
add_library(pcre2::pcre2-posix-static STATIC IMPORTED)
set_target_properties(pcre2::pcre2-posix-static PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "PCRE2_STATIC"
  INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/@0@"
  INTERFACE_LINK_LIBRARIES "pcre2::pcre2-8-static")
'''.format(get_option('includedir'))
    cmake_release_properties += '''
set_property(TARGET pcre2::pcre2-posix-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(pcre2::pcre2-posix-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${PACKAGE_PREFIX_DIR}/@0@/@1@")
'''.format(get_option('libdir'), static_output_names['posix'])
  endif
  if build_shared
    cmake_target_definitions += '''
add_library(pcre2::pcre2-posix-shared SHARED IMPORTED)
set_target_properties(pcre2::pcre2-posix-shared PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "PCRE2POSIX_SHARED"
  INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/@0@"
  INTERFACE_LINK_LIBRARIES "pcre2::pcre2-8-shared")
'''.format(get_option('includedir'))
    shared_install_name = shared_output_names['posix']
    shared_soname_property = ''
    shared_import_property = ''
    shared_install_dir = get_option('libdir')
    if host_is_windows
      shared_install_dir = get_option('bindir')
      import_filename = shared_output_names['posix'].replace('.dll', '')
      if mingw and get_option('non_standard_lib_suffix')
        import_filename = import_filename.replace('-0', '')
      endif
      import_filename += msvc_like ? '.lib' : '.dll.a'
      shared_import_property = '  IMPORTED_IMPLIB_RELEASE "${PACKAGE_PREFIX_DIR}/@0@/@1@"\n'.format(
        get_option('libdir'), import_filename)
    elif host_machine.system() == 'darwin'
      shared_soname_property = '  IMPORTED_SONAME_RELEASE "@rpath/@0@"\n'.format(shared_install_name)
    elif host_machine.system() != 'cygwin'
      unversioned_name = shared_output_names['posix'].replace('.' + lib_versions['posix'], '')
      shared_soname_property = '  IMPORTED_SONAME_RELEASE "@0@.@1@"\n'.format(
        unversioned_name, lib_soversions['posix'])
    endif
    cmake_release_properties += '''
set_property(TARGET pcre2::pcre2-posix-shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(pcre2::pcre2-posix-shared PROPERTIES
  IMPORTED_LOCATION_RELEASE "${PACKAGE_PREFIX_DIR}/@0@/@1@"
@2@@3@)
'''.format(shared_install_dir, shared_install_name, shared_import_property, shared_soname_property)
  endif
endif
configure_file(
  input: 'meson/pcre2-targets.cmake.in',
  output: 'pcre2-targets.cmake',
  configuration: {'TARGET_DEFINITIONS': cmake_target_definitions},
  install: true,
  install_dir: cmake_install_dir,
)
configure_file(
  input: 'meson/pcre2-targets-release.cmake.in',
  output: 'pcre2-targets-release.cmake',
  configuration: {'RELEASE_PROPERTIES': cmake_release_properties},
  install: true,
  install_dir: cmake_install_dir,
)
cmake = import('cmake')
cmake.configure_package_config_file(
  name: 'PCRE2',
  input: 'cmake/pcre2-config.cmake.in',
  install_dir: cmake_install_dir,
  configuration: {
    'REQUIRE_PTHREAD': jit_enabled ? '1' : '0',
    'PCRE2_MAJOR': pcre2_major,
    'PCRE2_MINOR': pcre2_minor,
  },
)
cmake.write_basic_package_version_file(
  name: 'PCRE2',
  version: meson.project_version() + '.0',
  compatibility: 'SameMajorVersion',
  install_dir: cmake_install_dir,
)

# -----------------------------------------------------------------------------
# Executables
# -----------------------------------------------------------------------------

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

# -----------------------------------------------------------------------------
# Testing
# -----------------------------------------------------------------------------

if get_option('pcre2_build_tests')
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
    link_args: msvc_like ? ['/STACK:2500000'] : [],
    install: true,
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
      config_h,
      pcre2_h,
      dependencies: test_deps,
      include_directories: pcre2_includes,
      c_args: common_c_args,
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

# -----------------------------------------------------------------------------
# Fuzzer support
# -----------------------------------------------------------------------------

if get_option('pcre2_fuzz_support')
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

# -----------------------------------------------------------------------------
# Installation
# -----------------------------------------------------------------------------

install_headers(pcre2_h, pcre2posix_h)
man_pages = [
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
]
foreach man_page : man_pages
  install_man('doc/man' / man_page.split('/')[1])
endforeach
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

# -----------------------------------------------------------------------------
# Configuration summary
# -----------------------------------------------------------------------------

if get_option('pcre2_heap_match_recurse')
  warning('PCRE2_HEAP_MATCH_RECURSE is obsolete and does nothing.')
endif
if get_option('pcre2_show_report')
  summary(
    {
      'Install prefix': get_option('prefix'),
      'C compiler': cc.get_id(),
      'Build 8 bit pcre2 library': build_8,
      'Build 16 bit pcre2 library': build_16,
      'Build 32 bit pcre2 library': build_32,
      'Include debugging code': debug_option,
      'Enable JIT compiling support': jit_enabled,
      'Use SELinux allocator in JIT': get_option('pcre2_support_jit_sealloc'),
      'Enable Unicode support': get_option('pcre2_support_unicode'),
      'Newline char/sequence': get_option('pcre2_newline'),
      '\\R matches only ANYCRLF': get_option('pcre2_support_bsr_anycrlf'),
      '\\C is disabled': get_option('pcre2_never_backslash_c'),
      'EBCDIC coding': ebcdic,
      'Rebuild char tables': rebuild_chartables,
      'Internal link size': get_option('pcre2_link_size'),
      'Maximum variable lookbehind': get_option('pcre2_max_varlookbehind'),
      'Nested parentheses limit': get_option('pcre2_parens_nest_limit'),
      'Heap limit': get_option('pcre2_heap_limit'),
      'Match limit': get_option('pcre2_match_limit'),
      'Match depth limit': get_option('pcre2_match_limit_depth'),
      'Build shared libs': build_shared,
      'with symbol versioning': use_vscript,
      'Build static libs': build_static,
      'with PIC enabled': get_option('pcre2_static_pic'),
      'Build pcre2grep': build_grep,
      'Build tests': get_option('pcre2_build_tests'),
      'Link pcre2grep with libz': zlib_dep.found(),
      'Link pcre2grep with libbz2': bzip2_dep.found(),
      'Link pcre2test with libeditline': editline_dep.found(),
      'Link pcre2test with libreadline': readline_dep.found(),
      'Enable Valgrind support': get_option('pcre2_support_valgrind'),
      'Fuzzer support': get_option('pcre2_fuzz_support'),
      'Differential fuzzer support': get_option('pcre2_diff_fuzz_support'),
    },
    section: 'PCRE2-' + meson.project_version() + ' configuration',
  )
endif
