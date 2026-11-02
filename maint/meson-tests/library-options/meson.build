project('pcre2-subproject-consumer', 'c', meson_version: '>=1.6.0')

deps = [dependency('libpcre2-8', fallback: 'pcre2'), dependency('libpcre2-posix', fallback: 'pcre2')]
foreach kind : ['static', 'shared']
  build_target(
    'naming-reference-' + kind,
    'consumer.c',
    target_type: kind + '_library',
    dependencies: deps,
    build_by_default: false,
    install: true,
  )
endforeach
consumer = executable('consumer', 'consumer.c', dependencies: deps)
test('consumer', consumer)
