project('pcre2-library-options-consumer', 'c', meson_version: '>=1.6.0')

default_deps = [dependency('libpcre2-8', fallback: 'pcre2'), dependency('libpcre2-posix', fallback: 'pcre2')]
default_consumer = executable(
  'default-consumer',
  'consumer.c',
  dependencies: default_deps,
  c_args: get_option('expect_default_static') ? '-DEXPECT_STATIC' : '-DEXPECT_SHARED',
)
test('default-consumer', default_consumer)

if get_option('request_static')
  static_deps = [
    dependency('libpcre2-8', static: true, fallback: 'pcre2'),
    dependency('libpcre2-posix', static: true, fallback: 'pcre2'),
  ]
  static_consumer = executable('static-consumer', 'consumer.c', dependencies: static_deps, c_args: '-DEXPECT_STATIC')
  test('static-consumer', static_consumer)
endif

if get_option('request_shared')
  shared_deps = [
    dependency('libpcre2-8', static: false, fallback: 'pcre2'),
    dependency('libpcre2-posix', static: false, fallback: 'pcre2'),
  ]
  shared_consumer = executable('shared-consumer', 'consumer.c', dependencies: shared_deps, c_args: '-DEXPECT_SHARED')
  test('shared-consumer', shared_consumer)
endif
