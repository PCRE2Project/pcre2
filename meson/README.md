# Meson build

The native Meson build is intended to be a peer of the CMake build. It uses the
same generated configuration, builds static and shared libraries independently,
and installs equivalent headers, tools, documentation, pkg-config files, and
CMake package metadata.

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
   -Ddefault_library=shared \
   -Dpcre2_build_pcre2_16=true \
   -Dpcre2_support_jit=auto
```

`meson configure build-meson` lists every option and its description.

PCRE2 uses Meson's built-in library options. `default_library` accepts
`static`, `shared`, or `both`; PCRE2 defaults it to `static`. When both variants
are built, `default_both_libraries` selects the variant used by PCRE2's tools
and by an unqualified subproject dependency. Its values are `shared` (the
Meson default), `static`, and `auto` (`auto` selects shared when
`default_library` is `both`):

```sh
meson setup build-meson-both \
   -Ddefault_library=both \
   -Ddefault_both_libraries=shared
```

In Meson 1.6 and 1.7, `default_both_libraries` is global when PCRE2 is a
subproject. Set it without a subproject prefix. Meson 1.8 and later also allow
the per-subproject form `-Dpcre2:default_both_libraries=static`.

## Integration

When PCRE2 is a Meson subproject, use `dependency('libpcre2-8')`,
`dependency('libpcre2-16')`, `dependency('libpcre2-32')`, or
`dependency('libpcre2-posix')`. Only enabled code-unit widths are exported.
An unqualified dependency uses the selected default variant. Consumers can
request a specific available variant with `static: true` or `static: false`.

Installation includes the enabled libraries, public headers, `pcre2grep`,
`pcre2-config`, `pcre2test` when tests are enabled, pkg-config metadata, CMake
package metadata, manpages, and HTML and text documentation.

## Implemented platform behavior

- C11 is preferred with a C99 fallback, and standard atomics are checked.
- Static and shared variants have separate object compilation. Both use hidden
  visibility; static producer declarations do not expose PCRE2 symbols when an
  archive is embedded in another shared library.
- GNU version scripts and Sun linker map flags are detected.
- Windows resources, MSVC-compatible CRT definitions, test stack size, library
   naming, and debug postfixes are supported. Meson installs target debug files
   using its native target support.
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
CMake consumers using both shared and static package aliases. Static-only,
shared-only, and 16-bit-only installations were also checked for exact target
availability and valid legacy CMake library variables.

The full native feature matrix has also been checked with JIT, zlib, bzip2,
and readline enabled. The Meson test suite, the complete core test suite under
Valgrind, the static/shared producer regression check, and the canonical shared
symbol manifests all pass. All four installed pkg-config modules resolve, and
shared and fully static `pcre2demo` consumers compile and run. A separate
editline-enabled, readline-disabled build also passes. After normalizing
Meson's Debian multiarch library directory and standard `PCRE2Config*.cmake`
filename spelling, the 273 installed paths match the Linux CMake install
manifest.

A Windows GNU cross-build using Zig has also been checked with all code-unit
widths and both library variants. Release and debug-postfixed DLLs, static
archives, import libraries, install layout, generated CMake artifact paths, and
relocated shared and static CMake consumer links were verified. The resulting
consumers are x86-64 PE executables; runtime execution was not available in
this environment.

The static/shared regression check can be repeated with:

```sh
meson setup build-meson-variants \
   -Ddefault_library=both \
   -Dpcre2_build_pcre2_16=true \
   -Dpcre2_build_pcre2_32=true \
   -Db_staticpic=true
meson compile -C build-meson-variants
python3 maint/meson-tests/check-static-shared.py build-meson-variants
```

The complete producer and fallback dependency selection matrix can be checked
with:

```sh
python3 maint/meson-tests/check-library-options.py
```

## Decisions and remaining work

1. Explicit `static_library()` and `shared_library()` targets are retained
   instead of `both_libraries()`. This keeps each variant's arguments and
   platform output names unambiguous while the standard `default_library` and
   `default_both_libraries` options control production and selection.
2. The minimum is Meson 1.6, which provides `default_both_libraries` and
   variant-aware internal dependency behavior. Development validation currently
   uses Meson 1.7.
3. Optional JIT, zlib, bzip2, readline, and editline switches are Meson
   `feature` options. Static-library PIC uses Meson's built-in `b_staticpic`
   option, including its default of `true`. Bzip2 lookup falls back from
   pkg-config to CMake's `BZip2` package because some systems, including the
   validated Ubuntu environment, do not provide a bzip2 `.pc` file. An
   explicitly disabled option performs neither lookup.
4. Meson has no equivalent of CMake's `install(EXPORT)`. The build generates
   target declarations and release artifact properties explicitly, while
   taking static and shared basenames from the actual Meson targets to avoid
   duplicating naming rules.
5. Symbol maps conservatively retain `local: *;`. Reliably proving that a
   linker accepts an omitted wildcard requires a shared-library link probe,
   which Meson's compiler checks do not directly provide.
6. Windows GNU paths have been cross-built, but native Windows execution, MSVC
   artifacts and PDBs, and resource compilation remain unchecked. The resource
   inputs are not present in this checkout. Darwin and Sun-linker paths also
   remain unexecuted. These native checks remain TODOs for future CI
   integration.
7. `src/config-meson.h.in` mirrors `src/config-cmake.h.in` because their
   substitution directives differ. Changes to configuration macros must be
   applied to both; no template-sync tooling is planned.
