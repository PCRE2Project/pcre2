project('pcre2-cmake-installed-consumer', 'c', meson_version: '>=1.6.0')

linkage = get_option('linkage')
cmake_target = linkage == 'default' ? 'PCRE2::POSIX' : 'pcre2::pcre2-posix-' + linkage
pcre2_dep = dependency('PCRE2', method: 'cmake', modules: [cmake_target])

consumer = executable(
  'cmake-installed-consumer',
  'consumer.c',
  dependencies: pcre2_dep,
  c_args: linkage == 'static' ? '-DEXPECT_STATIC' : '-DEXPECT_SHARED',
)
test('cmake-installed-consumer', consumer)
