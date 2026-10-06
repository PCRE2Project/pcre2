# -----------------------------------------------------------------------------
# Project setup
# -----------------------------------------------------------------------------

project(
  'PCRE2',
  'c',
  # This line is updated by UpdateRelease.py
  version: '10.49-DEV',
  license: 'BSD-3-Clause WITH PCRE2-exception',
  meson_version: '>=1.6.0',
  default_options: ['c_std=gnu11,c11,gnu99,c99', 'default_library=static'],
)

cc = meson.get_compiler('c')
fs = import('fs')
host_is_windows = host_machine.system() == 'windows'
msvc_like = cc.get_id() in ['msvc', 'clang-cl', 'intel-cl']

if msvc_like
  add_project_arguments('-D_CRT_SECURE_NO_DEPRECATE', '-D_CRT_SECURE_NO_WARNINGS', language: 'c')
endif

# -----------------------------------------------------------------------------
# External packages
# -----------------------------------------------------------------------------

subdir('meson/FindBZip2')
zlib_dep = dependency('zlib', required: get_option('pcre2_support_libz'))
subdir('meson/FindReadline')
subdir('meson/FindEditline')

# -----------------------------------------------------------------------------
# Configuration checks
# -----------------------------------------------------------------------------

config = configuration_data()
foreach header : ['assert.h', 'dirent.h', 'sys/stat.h', 'sys/types.h', 'unistd.h', 'windows.h']
  config.set('HAVE_' + header.underscorify().to_upper(), cc.check_header(header))
endforeach

# Check whether any system-wide extensions need to be enabled, in order for
# OS functionality to be exposed.
subdir('meson/PCRE2UseSystemExtensions')

config.set(
  'HAVE_MEMFD_CREATE',
  cc.has_function('memfd_create', prefix: '#include <sys/mman.h>', args: system_extension_args),
)
config.set(
  'HAVE_SECURE_GETENV',
  cc.has_function('secure_getenv', prefix: '#include <stdlib.h>', args: system_extension_args),
)
config.set(
  'HAVE_SETRLIMIT',
  cc.has_function('setrlimit', prefix: '#include <sys/resource.h>', args: system_extension_args),
)

config.set(
  'HAVE_STDATOMIC_H',
  cc.links(
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
  ),
)

config.set(
  'HAVE_REALPATH',
  cc.links(
    '''
  #include <stdlib.h>
  #include <limits.h>
  int main(int c, char *v[]) {
    char buf[PATH_MAX];
    return realpath(v[c], buf) == NULL;
  }
  ''',
    name: 'realpath',
  ),
)

config.set(
  'HAVE_ATTRIBUTE_UNINITIALIZED',
  cc.links(
    'int main(void) { char buf[128] __attribute__((uninitialized)); (void)buf; return 0; }',
    werror: true,
    name: '__attribute__((uninitialized))',
  ),
)

have_visibility = false
if not msvc_like
  have_visibility = cc.links(
    '''
  extern __attribute__ ((visibility ("default"))) int f(void);
  int main(void) { return f(); }
  int f(void) { return 42; }
  ''',
    werror: true,
    name: '__attribute__((visibility("default")))',
  )
endif
# Library targets use gnu_symbol_visibility: 'hidden'. Meson does not expose its
# compiler-specific visibility flags for a CMake-style consistency check.
config.set('PCRE2_EXPORT', have_visibility ? '__attribute__ ((visibility ("default")))' : '')

config.set('HAVE_BUILTIN_ASSUME', cc.links('int main(void) { __assume(1); return 0; }', name: '__assume'))

config.set(
  'HAVE_BUILTIN_MUL_OVERFLOW',
  cc.links(
    '''
  #include <stddef.h>
  int main(void) { int a, b; size_t m; __builtin_mul_overflow(a, b, &m); return 0; }
  ''',
    name: '__builtin_mul_overflow',
  ),
)

config.set(
  'HAVE_BUILTIN_UNREACHABLE',
  cc.links(
    'int main(int c, char *v[]) { if (c) __builtin_unreachable(); return (int)(*v[0]); }',
    name: '__builtin_unreachable',
  ),
)

# Detect support for linker scripts.
subdir('meson/PCRE2CheckVscript')

if cc.links('#ifndef __CET__\n#error CET is not enabled\n#endif\nint main(void) { return 0; }', name: 'Intel CET')
  add_project_arguments(cc.get_supported_arguments('-mshstk'), language: 'c')
endif

# -----------------------------------------------------------------------------
# User-configurable options
# -----------------------------------------------------------------------------

# Declarations and value constraints are in meson.options.

library_type = get_option('default_library')
build_shared = library_type in ['shared', 'both']
build_static = library_type in ['static', 'both']
# With both variants, Meson's auto selection falls back to shared.
default_to_static = library_type == 'static' or \
  (library_type == 'both' and get_option('default_both_libraries') == 'static')
build_8 = get_option('pcre2_build_pcre2_8')
build_16 = get_option('pcre2_build_pcre2_16')
build_32 = get_option('pcre2_build_pcre2_32')

config_options = {
  'PCRE2_LINK_SIZE': 'pcre2_link_size',
  'PCRE2_PARENS_NEST_LIMIT': 'pcre2_parens_nest_limit',
  'PCRE2_HEAP_LIMIT': 'pcre2_heap_limit',
  'PCRE2_MAX_VARLOOKBEHIND': 'pcre2_max_varlookbehind',
  'PCRE2_MATCH_LIMIT': 'pcre2_match_limit',
  'PCRE2_MATCH_LIMIT_DEPTH': 'pcre2_match_limit_depth',
  'PCRE2GREP_BUFSIZE': 'pcre2grep_bufsize',
  'PCRE2GREP_MAX_BUFSIZE': 'pcre2grep_max_bufsize',
}
foreach config_name, option_name : config_options
  config.set(config_name, get_option(option_name))
endforeach

jit_option = get_option('pcre2_support_jit')
jit_enabled = jit_option.enabled()
if jit_option.auto()
  jit_enabled = cc.links(
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

build_grep = get_option('pcre2_build_pcre2grep')

# -----------------------------------------------------------------------------
# Build configuration
# -----------------------------------------------------------------------------

pcre2_includes = include_directories('.', 'src')

if not build_8 and not build_16 and not build_32
  error('At least one PCRE2 code unit width must be enabled.')
endif

config.set('SUPPORT_PCRE2_8', build_8)
config.set('SUPPORT_PCRE2_16', build_16)
config.set('SUPPORT_PCRE2_32', build_32)

if build_grep and not build_8
  error('PCRE2_BUILD_PCRE2_8 must be enabled for pcre2grep')
endif

# The dependency finders have already checked explicitly requested libraries.
if editline_dep.found() and readline_dep.found()
  error('Only one of PCRE2_SUPPORT_LIBREADLINE and PCRE2_SUPPORT_LIBEDIT can be enabled.')
endif

config.set('BSR_ANYCRLF', get_option('pcre2_support_bsr_anycrlf'))
config.set('NEVER_BACKSLASH_C', get_option('pcre2_never_backslash_c'))
config.set('SUPPORT_UNICODE', get_option('pcre2_support_unicode'))

config.set('SUPPORT_JIT', jit_enabled)
thread_dep = dependency('threads', required: jit_enabled)
config.set('SLJIT_PROT_EXECUTABLE_ALLOCATOR', get_option('pcre2_support_jit_sealloc'))
config.set('SUPPORT_PCRE2GREP_JIT', get_option('pcre2grep_support_jit'))
config.set('SUPPORT_PCRE2GREP_CALLOUT', get_option('pcre2grep_support_callout'))
config.set(
  'SUPPORT_PCRE2GREP_CALLOUT_FORK',
  get_option('pcre2grep_support_callout') and get_option('pcre2grep_support_callout_fork'),
)
config.set('SUPPORT_VALGRIND', get_option('pcre2_support_valgrind'))

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
config.set('SUPPORT_DIFF_FUZZ', get_option('pcre2_diff_fuzz_support'))

config.set('DISABLE_PERCENT_ZT', get_option('pcre2_disable_percent_zt'))

pcre2test_deps = []
pcre2grep_deps = []
config.set('SUPPORT_LIBREADLINE', readline_dep.found())
if readline_dep.found()
  pcre2test_deps += readline_dep
endif
config.set('SUPPORT_LIBEDIT', editline_dep.found())
config.set('HAVE_EDITLINE_READLINE_H', editline_header == 'editline/readline.h')
config.set('HAVE_EDIT_READLINE_READLINE_H', editline_header == 'edit/readline/readline.h')
if editline_dep.found()
  pcre2test_deps += editline_dep
endif
config.set('SUPPORT_LIBZ', zlib_dep.found())
if zlib_dep.found()
  pcre2grep_deps += zlib_dep
endif
config.set('SUPPORT_LIBBZ2', bzip2_dep.found())
if bzip2_dep.found()
  pcre2grep_deps += bzip2_dep
endif

newline_values = {'CR': 1, 'LF': 2, 'CRLF': 3, 'ANY': 4, 'ANYCRLF': 5, 'NUL': 6}
config.set('NEWLINE_DEFAULT', newline_values[get_option('pcre2_newline')])

rebuild_chartables = get_option('pcre2_rebuild_chartables')
ebcdic = get_option('pcre2_ebcdic') or get_option('pcre2_ebcdic_nl25') or get_option('pcre2_ebcdic_ignoring_compiler')
config.set('EBCDIC', ebcdic)
config.set('EBCDIC_NL25', get_option('pcre2_ebcdic_nl25'))
config.set('EBCDIC_IGNORING_COMPILER', get_option('pcre2_ebcdic_ignoring_compiler'))

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

# Match CMake's cleanup before generating headers: quoted includes would otherwise
# pick up these source-tree headers from an earlier Autotools build.
build_python = import('python').find_installation()
run_command(
  build_python,
  '-c',
  '''
import os, sys
for name in ('config.h', 'pcre2.h'):
    path = os.path.join(sys.argv[1], name)
    if os.path.lexists(path):
        os.remove(path)
''',
  meson.project_source_root() / 'src',
  check: true,
)

# -----------------------------------------------------------------------------
# Release and library version information
# -----------------------------------------------------------------------------

project_version_parts = meson.project_version().split('-')
pcre2_version = project_version_parts[0]
pcre2_major = pcre2_version.split('.')[0]
pcre2_minor = pcre2_version.split('.')[1]
pcre2_prerelease = project_version_parts.length() > 1 ? '-' + project_version_parts[1] : ''

# These lines are updated by UpdateRelease.py
pcre2_date = '2026-11-31'
libtool_versions = {'posix': '3:8:0', '8': '16:0:16', '16': '16:0:16', '32': '16:0:16'}

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
  lib_darwin_versions += {library: [darwin_compatibility.to_string(), '@0@.@1@'.format(darwin_compatibility, revision)]}
endforeach

# -----------------------------------------------------------------------------
# Generated files
# -----------------------------------------------------------------------------

config_h = configure_file(input: 'src/config-meson.h.in', output: 'config.h', configuration: config)

pcre2_header_config = {
  'PCRE2_MAJOR': pcre2_major,
  'PCRE2_MINOR': pcre2_minor,
  'PCRE2_PRERELEASE': pcre2_prerelease,
  'PCRE2_DATE': pcre2_date,
}
pcre2_h = configure_file(input: 'src/pcre2.h.in', output: 'pcre2.h', configuration: pcre2_header_config)
pcre2posix_h = configure_file(input: 'src/pcre2posix.h', output: 'pcre2posix.h', copy: true)

symbol_files = {}
use_vscript = have_vscript and get_option('pcre2_symvers')
if use_vscript
  foreach library : ['pcre2-8', 'pcre2-16', 'pcre2-32', 'pcre2-posix']
    symbol_files += {
      library: configure_file(
        input: 'src/lib@0@.sym.in'.format(library),
        output: 'lib@0@.sym'.format(library),
        configuration: {'PCRE2_EXTRA_LOCAL_SYMS': have_vscript_no_star ? '' : '  local: *;'},
      ),
    }
  endforeach
endif

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
  chartables = custom_target('pcre2_chartables', output: 'pcre2_chartables.c', command: [pcre2_dftables, '@OUTPUT@'])
elif not ebcdic
  chartables = configure_file(input: 'src/pcre2_chartables.c.dist', output: 'pcre2_chartables.c', copy: true)
elif get_option('pcre2_ebcdic_nl25')
  chartables = configure_file(input: 'src/pcre2_chartables.c.ebcdic-1047-nl25', output: 'pcre2_chartables.c', copy: true)
else
  chartables = configure_file(input: 'src/pcre2_chartables.c.ebcdic-1047-nl15', output: 'pcre2_chartables.c', copy: true)
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

# -----------------------------------------------------------------------------
# Libraries
# -----------------------------------------------------------------------------

common_c_args = ['-DHAVE_CONFIG_H']
debug_option = get_option('pcre2_debug')
if debug_option == 'enabled' or (debug_option == 'auto' and get_option('buildtype') == 'debug')
  common_c_args += '-DPCRE2_DEBUG'
endif
windows_debug_postfix = host_is_windows and get_option('buildtype') == 'debug' ? 'd' : ''
private_deps = jit_enabled ? [thread_dep] : []

# Separate targets keep each variant's arguments and platform names explicit.
foreach width : ['8', '16', '32']
  enabled = get_variable('build_' + width)
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
      name_suffix: msvc_like ? 'lib' : [],
      gnu_symbol_visibility: 'hidden',
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
    shared_name = name + windows_debug_postfix
    link_args = use_vscript ? ['-Wl,' + vscript_flag + ',' + meson.current_build_dir() / ('lib' + name + '.sym')] : []
    shared_kwargs = {
      'include_directories': pcre2_includes,
      'dependencies': private_deps,
      'c_args': width_args,
      'link_args': link_args,
      'link_depends': use_vscript ? symbol_files[name] : [],
      'gnu_symbol_visibility': 'hidden',
      'install': true,
    }
    if not host_is_windows
      shared_kwargs += {'version': lib_versions[width], 'soversion': lib_soversions[width]}
    endif
    if host_machine.system() == 'darwin'
      shared_kwargs += {'darwin_versions': lib_darwin_versions[width]}
    endif
    shared_lib = shared_library(shared_name, pcre2_sources, kwargs: shared_kwargs)
    shared_dep = declare_dependency(
      link_with: shared_lib,
      include_directories: pcre2_includes,
      dependencies: private_deps,
      version: meson.project_version(),
    )
    set_variable('pcre2_' + width + '_shared_lib', shared_lib)
    set_variable('pcre2_' + width + '_shared_dep', shared_dep)
  endif

  # The first variant override also supplies unqualified dependency() calls, so the
  # order of the dependency overrides matters.
  if default_to_static
    static_dep = get_variable('pcre2_' + width + '_static_dep')
    set_variable('libpcre2_' + width, static_dep)
    meson.override_dependency('libpcre2-' + width, static_dep, static: true)
    if build_shared
      shared_dep = get_variable('pcre2_' + width + '_shared_dep')
      meson.override_dependency('libpcre2-' + width, shared_dep, static: false)
    endif
  else
    shared_dep = get_variable('pcre2_' + width + '_shared_dep')
    set_variable('libpcre2_' + width, shared_dep)
    meson.override_dependency('libpcre2-' + width, shared_dep, static: false)
    if build_static
      static_dep = get_variable('pcre2_' + width + '_static_dep')
      meson.override_dependency('libpcre2-' + width, static_dep, static: true)
    endif
  endif
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
      name_suffix: msvc_like ? 'lib' : [],
      gnu_symbol_visibility: 'hidden',
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
    posix_shared_name = 'pcre2-posix' + windows_debug_postfix
    posix_link_args = use_vscript ? ['-Wl,' + vscript_flag + ',' + meson.current_build_dir() / 'libpcre2-posix.sym'] : []
    posix_shared_kwargs = {
      'include_directories': pcre2_includes,
      'dependencies': pcre2_8_shared_dep,
      'c_args': common_c_args + ['-DPCRE2_CODE_UNIT_WIDTH=8', '-DPCRE2POSIX_SHARED'],
      'link_args': posix_link_args,
      'link_depends': use_vscript ? symbol_files['pcre2-posix'] : [],
      'gnu_symbol_visibility': 'hidden',
      'install': true,
    }
    if not host_is_windows
      posix_shared_kwargs += {'version': lib_versions['posix'], 'soversion': lib_soversions['posix']}
    endif
    if host_machine.system() == 'darwin'
      posix_shared_kwargs += {'darwin_versions': lib_darwin_versions['posix']}
    endif
    pcre2_posix_shared = shared_library(
      posix_shared_name,
      'src/pcre2posix.c',
      config_h,
      pcre2_h,
      pcre2posix_h,
      kwargs: posix_shared_kwargs,
    )
    pcre2_posix_shared_dep = declare_dependency(
      link_with: pcre2_posix_shared,
      include_directories: pcre2_includes,
      compile_args: '-DPCRE2POSIX_SHARED',
      dependencies: pcre2_8_shared_dep,
      version: meson.project_version(),
    )
  endif

  if default_to_static
    libpcre2_posix = pcre2_posix_static_dep
    meson.override_dependency('libpcre2-posix', pcre2_posix_static_dep, static: true)
    if build_shared
      meson.override_dependency('libpcre2-posix', pcre2_posix_shared_dep, static: false)
    endif
  else
    libpcre2_posix = pcre2_posix_shared_dep
    meson.override_dependency('libpcre2-posix', pcre2_posix_shared_dep, static: false)
    if build_static
      meson.override_dependency('libpcre2-posix', pcre2_posix_static_dep, static: true)
    endif
  endif
endif

# -----------------------------------------------------------------------------
# Package metadata
# -----------------------------------------------------------------------------

pkg = import('pkgconfig')
# In pkg-config versions, a tilde sorts prereleases before the final release.
pkg_version = meson.project_version().replace('-', '~')
foreach width : ['8', '16', '32']
  enabled = get_variable('build_' + width)
  if not enabled
    continue
  endif

  pkg_lib = build_shared ? get_variable('pcre2_' + width + '_shared_lib') : get_variable(
    'pcre2_' + width + '_static_lib',
  )
  pkg.generate(
    pkg_lib,
    filebase: 'libpcre2-' + width,
    name: 'libpcre2-' + width,
    description: 'PCRE2 - Perl compatible regular expressions C library (2nd API) with @0@ bit character support'.format(
      width,
    ),
    extra_cflags: build_shared ? [] : ['-DPCRE2_STATIC'],
    # Shared targets do not contribute their dependencies automatically.
    libraries_private: private_deps,
    version: pkg_version,
  )
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
    version: pkg_version,
  )
endif

pcre2_config_data = {
  'prefix': get_option('prefix'),
  'exec_prefix': '${prefix}',
  'libdir': fs.is_absolute(get_option('libdir')) ? get_option('libdir') : '${exec_prefix}/' + get_option('libdir'),
  'includedir': fs.is_absolute(get_option('includedir')) ? get_option('includedir') : '${prefix}/' + get_option(
    'includedir',
  ),
  'PACKAGE_VERSION': pcre2_version,
  'enable_pcre2_8': build_8 ? 'yes' : 'no',
  'enable_pcre2_16': build_16 ? 'yes' : 'no',
  'enable_pcre2_32': build_32 ? 'yes' : 'no',
  'PCRE2_STATIC_CFLAG': build_shared ? '' : '-DPCRE2_STATIC',
  'PCRE2POSIX_CFLAG': build_shared ? '-DPCRE2POSIX_SHARED' : '',
  'LIB_POSTFIX': windows_debug_postfix,
}
pcre2_config = configure_file(
  input: 'pcre2-config.in',
  output: 'pcre2-config',
  configuration: pcre2_config_data,
  install: true,
  install_dir: get_option('bindir'),
  install_mode: 'rwxr-xr-x',
)

# -----------------------------------------------------------------------------
# Executables
# -----------------------------------------------------------------------------

if build_grep
  pcre2grep = executable(
    'pcre2grep',
    'src/pcre2grep.c',
    config_h,
    pcre2_h,
    include_directories: pcre2_includes,
    dependencies: [libpcre2_posix] + pcre2grep_deps,
    c_args: common_c_args + ['-DPCRE2_CODE_UNIT_WIDTH=8'],
    install: true,
  )
endif

# -----------------------------------------------------------------------------
# Testing
# -----------------------------------------------------------------------------

if get_option('pcre2_build_tests')
  if build_8
    pcre2test_deps += libpcre2_posix
  endif
  pcre2_jit_test_deps = []
  foreach width : ['8', '16', '32']
    enabled = get_variable('build_' + width)
    if enabled
      pcre2test_deps += get_variable('libpcre2_' + width)
      pcre2_jit_test_deps += get_variable('libpcre2_' + width)
    endif
  endforeach

  pcre2test = executable(
    'pcre2test',
    'src/pcre2test.c',
    config_h,
    pcre2_h,
    include_directories: pcre2_includes,
    dependencies: pcre2test_deps,
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
  endif
  if jit_enabled
    pcre2_jit_test = executable(
      'pcre2_jit_test',
      'src/pcre2_jit_test.c',
      config_h,
      pcre2_h,
      dependencies: pcre2_jit_test_deps,
      include_directories: pcre2_includes,
      c_args: common_c_args,
    )
  endif

  if meson.can_run_host_binaries()
    python = find_program('python3', native: true, required: false)
    if python.found()
      test_runner = python
      test_script = files('RunTest.py')
      grep_test_script = files('RunGrepTest.py')
    else
      test_runner = find_program('sh', native: true)
      test_script = files('RunTest')
      grep_test_script = files('RunGrepTest')
    endif
    # Passing the target lets Meson provide MESON_EXE_WRAPPER to the harness.
    test(
      'pcre2_test',
      test_runner,
      args: [test_script, '--srcdir', meson.project_source_root(), '--pcre2test', pcre2test],
      workdir: meson.project_build_root(),
      timeout: -1,
    )
    if build_grep and not meson.is_cross_build()
      test(
        'pcre2_grep_test',
        test_runner,
        args: [
          grep_test_script,
          '--srcdir',
          meson.project_source_root(),
          '--pcre2test',
          pcre2test.full_path(),
          '--pcre2grep',
          pcre2grep.full_path(),
        ],
        depends: [pcre2test, pcre2grep],
        workdir: meson.project_build_root(),
        should_fail: ebcdic,
        timeout: -1,
      )
    endif
  else
    message('Skipping pcre2_test: host binaries cannot run without an exe_wrapper.')
  endif
  if build_grep and meson.is_cross_build()
    message('Skipping pcre2_grep_test: the grep harness does not support cross builds.')
  endif
  if jit_enabled
    test('pcre2_jit_test', pcre2_jit_test, timeout: -1)
  endif
  if build_8
    test('pcre2posix_test', pcre2posix_test, timeout: -1)
  endif
endif

# -----------------------------------------------------------------------------
# Fuzzer support
# -----------------------------------------------------------------------------

if get_option('pcre2_fuzz_support')
  foreach width : ['8', '16', '32']
    enabled = get_variable('build_' + width)
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

# Alas, Meson has no support for globbing, nor does install_man apparently accept a directory
man_pages = [
  'doc/man/pcre2-config.1',
  'doc/man/pcre2grep.1',
  'doc/man/pcre2test.1',
  'doc/man/pcre2.3',
  'doc/man/pcre2_callout_enumerate.3',
  'doc/man/pcre2_code_copy.3',
  'doc/man/pcre2_code_copy_with_tables.3',
  'doc/man/pcre2_code_free.3',
  'doc/man/pcre2_compile.3',
  'doc/man/pcre2_compile_context_copy.3',
  'doc/man/pcre2_compile_context_create.3',
  'doc/man/pcre2_compile_context_free.3',
  'doc/man/pcre2_config.3',
  'doc/man/pcre2_convert_context_copy.3',
  'doc/man/pcre2_convert_context_create.3',
  'doc/man/pcre2_convert_context_free.3',
  'doc/man/pcre2_converted_pattern_free.3',
  'doc/man/pcre2_dfa_match.3',
  'doc/man/pcre2_general_context_copy.3',
  'doc/man/pcre2_general_context_create.3',
  'doc/man/pcre2_general_context_free.3',
  'doc/man/pcre2_get_bsr.3',
  'doc/man/pcre2_get_depth_limit.3',
  'doc/man/pcre2_get_error_message.3',
  'doc/man/pcre2_get_heap_limit.3',
  'doc/man/pcre2_get_mark.3',
  'doc/man/pcre2_get_match_data_heapframes_size.3',
  'doc/man/pcre2_get_match_data_size.3',
  'doc/man/pcre2_get_match_limit.3',
  'doc/man/pcre2_get_max_pattern_length.3',
  'doc/man/pcre2_get_newline.3',
  'doc/man/pcre2_get_offset_limit.3',
  'doc/man/pcre2_get_ovector_count.3',
  'doc/man/pcre2_get_ovector_pointer.3',
  'doc/man/pcre2_get_parens_nest_limit.3',
  'doc/man/pcre2_get_startchar.3',
  'doc/man/pcre2_jit_compile.3',
  'doc/man/pcre2_jit_free_unused_memory.3',
  'doc/man/pcre2_jit_match.3',
  'doc/man/pcre2_jit_stack_assign.3',
  'doc/man/pcre2_jit_stack_create.3',
  'doc/man/pcre2_jit_stack_free.3',
  'doc/man/pcre2_maketables.3',
  'doc/man/pcre2_maketables_free.3',
  'doc/man/pcre2_match.3',
  'doc/man/pcre2_match_context_copy.3',
  'doc/man/pcre2_match_context_create.3',
  'doc/man/pcre2_match_context_free.3',
  'doc/man/pcre2_match_data_create.3',
  'doc/man/pcre2_match_data_create_from_pattern.3',
  'doc/man/pcre2_match_data_free.3',
  'doc/man/pcre2_next_match.3',
  'doc/man/pcre2_pattern_convert.3',
  'doc/man/pcre2_pattern_info.3',
  'doc/man/pcre2_serialize_decode.3',
  'doc/man/pcre2_serialize_encode.3',
  'doc/man/pcre2_serialize_free.3',
  'doc/man/pcre2_serialize_get_number_of_codes.3',
  'doc/man/pcre2_set_bsr.3',
  'doc/man/pcre2_set_callout.3',
  'doc/man/pcre2_set_character_tables.3',
  'doc/man/pcre2_set_compile_extra_options.3',
  'doc/man/pcre2_set_compile_recursion_guard.3',
  'doc/man/pcre2_set_depth_limit.3',
  'doc/man/pcre2_set_glob_escape.3',
  'doc/man/pcre2_set_glob_separator.3',
  'doc/man/pcre2_set_heap_limit.3',
  'doc/man/pcre2_set_match_limit.3',
  'doc/man/pcre2_set_max_pattern_compiled_length.3',
  'doc/man/pcre2_set_max_pattern_length.3',
  'doc/man/pcre2_set_max_varlookbehind.3',
  'doc/man/pcre2_set_newline.3',
  'doc/man/pcre2_set_offset_limit.3',
  'doc/man/pcre2_set_optimize.3',
  'doc/man/pcre2_set_parens_nest_limit.3',
  'doc/man/pcre2_set_recursion_limit.3',
  'doc/man/pcre2_set_recursion_memory_management.3',
  'doc/man/pcre2_set_substitute_callout.3',
  'doc/man/pcre2_set_substitute_case_callout.3',
  'doc/man/pcre2_substitute.3',
  'doc/man/pcre2_substring_copy_byname.3',
  'doc/man/pcre2_substring_copy_bynumber.3',
  'doc/man/pcre2_substring_free.3',
  'doc/man/pcre2_substring_get_byname.3',
  'doc/man/pcre2_substring_get_bynumber.3',
  'doc/man/pcre2_substring_length_byname.3',
  'doc/man/pcre2_substring_length_bynumber.3',
  'doc/man/pcre2_substring_list_free.3',
  'doc/man/pcre2_substring_list_get.3',
  'doc/man/pcre2_substring_nametable_scan.3',
  'doc/man/pcre2_substring_number_from_name.3',
  'doc/man/pcre2api.3',
  'doc/man/pcre2build.3',
  'doc/man/pcre2callout.3',
  'doc/man/pcre2compat.3',
  'doc/man/pcre2convert.3',
  'doc/man/pcre2demo.3',
  'doc/man/pcre2jit.3',
  'doc/man/pcre2limits.3',
  'doc/man/pcre2matching.3',
  'doc/man/pcre2partial.3',
  'doc/man/pcre2pattern.3',
  'doc/man/pcre2perform.3',
  'doc/man/pcre2posix.3',
  'doc/man/pcre2sample.3',
  'doc/man/pcre2serialize.3',
  'doc/man/pcre2syntax.3',
  'doc/man/pcre2unicode.3',
]
install_man(man_pages)

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
install_subdir('doc/html', install_dir: get_option('datadir') / 'doc/pcre2')

# -----------------------------------------------------------------------------
# Configuration summary
# -----------------------------------------------------------------------------

if get_option('pcre2_show_report')
  ebcdic_nl_code = get_option('pcre2_ebcdic_nl25') ? '0x25' : '0x15'
  summary(
    {
      'Install prefix': get_option('prefix'),
      'C compiler': cc.get_id(),
      'Build type': get_option('buildtype'),
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
      'EBCDIC code for NL': ebcdic ? ebcdic_nl_code : 'n/a',
      'EBCDIC coding ignoring compiler': get_option('pcre2_ebcdic_ignoring_compiler'),
      'Rebuild char tables': rebuild_chartables,
      'Internal link size': get_option('pcre2_link_size'),
      'Maximum variable lookbehind': get_option('pcre2_max_varlookbehind'),
      'Nested parentheses limit': get_option('pcre2_parens_nest_limit'),
      'Heap limit': get_option('pcre2_heap_limit'),
      'Match limit': get_option('pcre2_match_limit'),
      'Match depth limit': get_option('pcre2_match_limit_depth'),
      'Library variants': library_type,
      'Default both-library linkage': get_option('default_both_libraries'),
      'Build shared libs': build_shared,
      'with symbol versioning': use_vscript,
      'Build static libs': build_static,
      'with PIC enabled': get_option('b_staticpic'),
      'Build pcre2grep': build_grep,
      'Enable JIT in pcre2grep': get_option('pcre2grep_support_jit'),
      'Enable callouts in pcre2grep': get_option('pcre2grep_support_callout'),
      'Enable callout fork in pcre2grep': get_option('pcre2grep_support_callout_fork'),
      'Initial buffer size for pcre2grep': get_option('pcre2grep_bufsize'),
      'Maximum buffer size for pcre2grep': get_option('pcre2grep_max_bufsize'),
      'Build tests': get_option('pcre2_build_tests'),
      'Link pcre2grep with libz': zlib_dep.found(),
      'Link pcre2grep with libbz2': bzip2_dep.found(),
      'Link pcre2test with libeditline': editline_dep.found(),
      'Link pcre2test with libreadline': readline_dep.found(),
      'Enable Valgrind support': get_option('pcre2_support_valgrind'),
      'Fuzzer support': get_option('pcre2_fuzz_support'),
      'Differential fuzzer support': get_option('pcre2_diff_fuzz_support'),
      'Use %zu and %td': get_option('pcre2_disable_percent_zt') ? 'OFF' : 'AUTO',
    },
    section: 'PCRE2-' + meson.project_version() + ' configuration',
  )
endif
