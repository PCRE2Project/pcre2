# Meson build

The native Meson build is intended to be a peer of the CMake build. It uses the
same defaults and generated configuration, builds static and shared libraries
independently, and installs equivalent headers, tools, documentation,
pkg-config files, and CMake package metadata.

Configure, build, test, and install with:

```sh
meson setup build-meson
meson compile -C build-meson
meson test -C build-meson
meson install -C build-meson
```

Project options use lower-case snake_case names, following Meson conventions:

```sh
meson setup build-meson \
   -Dbuild_shared_libs=true \
   -Dbuild_static_libs=false \
   -Dpcre2_build_pcre2_16=true \
   -Dpcre2_support_jit=AUTO
```

`meson configure build-meson` lists every option and its description.

## Integration

When PCRE2 is a Meson subproject, use `dependency('libpcre2-8')`,
`dependency('libpcre2-16')`, `dependency('libpcre2-32')`, or
`dependency('libpcre2-posix')`. Only enabled code-unit widths are exported.

Installation includes the enabled libraries, public headers, `pcre2grep`,
`pcre2-config`, pkg-config metadata, CMake package metadata, manpages, and HTML
and text documentation.

## Implemented platform behavior

- C11 is preferred with a C99 fallback, and standard atomics are checked.
- Static and shared variants have separate object compilation. Both use hidden
  visibility; static producer declarations do not expose PCRE2 symbols when an
  archive is embedded in another shared library.
- GNU version scripts and Sun linker map flags are detected.
- Windows resources, MSVC-compatible CRT definitions, test stack size, library
  naming, debug postfixes, and MinGW prefix/suffix compatibility options are
  supported. Meson installs target debug files using its native target support.
- Darwin compatibility and current versions are derived from the same libtool
  ABI tuples used by CMake.
- Installed CMake metadata preserves the `PCRE2::8BIT`, `PCRE2::16BIT`,
  `PCRE2::32BIT`, and `PCRE2::POSIX` interfaces for shared and static selection.
  Imported files use Meson's actual target basenames, full versioned shared
  locations, ELF SONAMEs, and Windows DLL/import-library pairs.
- Release and ABI values are stamped in both build systems by
  `maint/UpdateRelease.py` from the release script's `VERSION_INFO` record.

## Verified behavior

Linux native builds have been checked with all code-unit widths and both
library variants enabled. The checks include the full build, distinct static
and shared compile commands, hidden symbols from PIC static archives embedded
in a shared object, GNU-versioned shared exports, installation, and standalone
CMake consumers using both shared and static package aliases.

## Decisions and remaining work

1. Explicit `static_library()` and `shared_library()` targets are retained
   instead of `both_libraries()`. This makes independent producer definitions
   and platform output names unambiguous.
2. The minimum remains Meson 1.3 because the implementation does not require a
   newer API. Development validation currently uses Meson 1.7.
3. Optional zlib, bzip2, readline, and editline switches are Meson `feature`
   options, so `auto` retains CMake's enable-when-found behavior.
4. Meson has no equivalent of CMake's `install(EXPORT)`. The build generates
   target declarations and release artifact properties explicitly, while
   taking static and shared basenames from the actual Meson targets to avoid
   duplicating naming rules.
5. Symbol maps conservatively retain `local: *;`. Reliably proving that a
   linker accepts an omitted wildcard requires a shared-library link probe,
   which Meson's compiler checks do not directly provide.
6. Windows/MSVC, MinGW, Darwin, and Sun-linker paths are implemented but have
   not been executed on those hosts in this effort. Native artifact, consumer,
   symbol, and runtime checks remain TODOs for the future CI integration.
7. `src/config-meson.h.in` mirrors `src/config-cmake.h.in` because their
   substitution directives differ. Changes to configuration macros must be
   applied to both; no template-sync tooling is planned.
