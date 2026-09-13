# Meson build

The native Meson build currently targets Linux. It follows the CMake build's
ordering, option names, defaults, generated headers, targets, tests, and
installation layout.

Configure, build, test, and install with:

```sh
meson setup build-meson
meson compile -C build-meson
meson test -C build-meson
meson install -C build-meson
```

Pass PCRE2 options during setup using their CMake spelling, for example:

```sh
meson setup build-meson \
  -DBUILD_SHARED_LIBS=true \
  -DBUILD_STATIC_LIBS=false \
  -DPCRE2_BUILD_PCRE2_16=true \
  -DPCRE2_SUPPORT_JIT=AUTO
```

`meson configure build-meson` lists every option and its description.

## Integration

When PCRE2 is a Meson subproject, use `dependency('libpcre2-8')`,
`dependency('libpcre2-16')`, `dependency('libpcre2-32')`, or
`dependency('libpcre2-posix')`. Only enabled code-unit widths are exported.

Installation includes the enabled libraries, public headers, `pcre2grep`,
`pcre2-config`, pkg-config metadata, CMake package metadata, manpages, and HTML
and text documentation.

## Choices and questions for review

1. **Platform scope:** this first implementation supports Linux. GNU-style
   linker version scripts are detected; the CMake build's Sun linker fallback,
   Windows resources/DLL naming/PDB handling, and Mach-O compatibility versions
   remain future platform work.
2. **Option compatibility:** project options retain the CMake uppercase names.
   Optional zlib, bzip2, readline, and editline switches are Meson `feature`
   options so `auto` reproduces CMake's "enable when found" behavior.
3. **Library selection:** independent `BUILD_STATIC_LIBS` and
   `BUILD_SHARED_LIBS` options are retained instead of replacing them with
   Meson's `default_library`, including support for building both variants.
   Unsuffixed in-tree dependencies prefer static libraries, matching CMake's
   aliases; installed pkg-config files prefer shared libraries when available.
4. **CMake consumers:** Meson has no equivalent of CMake's `install(EXPORT)`.
   The build therefore generates Linux imported targets explicitly while
   preserving the existing `PCRE2::8BIT`, `PCRE2::16BIT`, `PCRE2::32BIT`, and
   `PCRE2::POSIX` package interface.
5. **Generated configuration:** `src/config-meson.h.in` mirrors
   `src/config-cmake.h.in` because Meson and CMake use different conditional
   substitution directives. Keeping both templates makes either build
   self-contained but means changes must be applied to both.
6. **Version declaration:** Meson requires the project version in `project()`.
   Configuration also parses `configure.ac` and fails if the two versions
   diverge, making the unavoidable duplicate value visible during setup.
