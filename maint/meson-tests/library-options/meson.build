project('pcre2-subproject-consumer', 'c', meson_version: '>=1.6.0')

deps = [dependency('libpcre2-8', fallback: 'pcre2'), dependency('libpcre2-posix', fallback: 'pcre2')]
consumer = executable('consumer', 'consumer.c', dependencies: deps)
test('consumer', consumer)
