# Guidance for agents

## Using PCRE2 as a library or dependency

Read `doc/*.adoc` for the version you use: [pcre2api](doc/pcre2api.adoc) covers
the API, options, ownership, and errors; [pcre2pattern](doc/pcre2pattern.adoc)
explains regex semantics; [pcre2syntax](doc/pcre2syntax.adoc) is the quick
reference. Read [security considerations](doc/pcre2.adoc#security-considerations),
[pcre2limits](doc/pcre2limits.adoc), and [pcre2jit](doc/pcre2jit.adoc) for security,
limits, and JIT-specific behaviour. Examples: [README quickstart](.github/README.md#quickstart),
[sample explanation](doc/pcre2sample.adoc), and [demo source](src/pcre2demo.c).

Define `PCRE2_CODE_UNIT_WIDTH` before including `<pcre2.h>` and link the matching
8-, 16-, or 32-bit library. Lengths and offsets count code units, not characters.
For untrusted input, set appropriate resource limits; never disable UTF
validation without guaranteeing validity. Reproduce regex behaviour with
[pcre2test](doc/pcre2test.adoc), recording the exact pattern, subject, options,
code-unit width, and engine rather than assuming another regex dialect agrees.

Pin Git dependencies to a [maintained release tag](SUPPORT-LIFECYCLE.md), not
`main`; for example:

```sh
git clone --branch pcre2-10.49 --depth 1 --recurse-submodules \
  https://github.com/PCRE2Project/pcre2.git pcre2
```

For a non-recursive clone, run `git submodule update --init` inside the checkout
to fetch `deps/sljit`, required for JIT.

- **CMake:** use `add_subdirectory(pcre2)` and link `pcre2-8`. For an installed
  CMake package, use `find_package(PCRE2 CONFIG REQUIRED)` and `PCRE2::8BIT`.
- **vcpkg:** add `pcre2` to the consumer's manifest (`vcpkg add port pcre2`),
  use the vcpkg toolchain, and follow the port's usage instructions
  (`find_package(pcre2 CONFIG REQUIRED)`, linking `PCRE2::8BIT`).
- **Meson:** place a native Meson checkout in `subprojects/pcre2` and use
  `dependency('libpcre2-8', fallback: 'pcre2')`; see
  [NON-AUTOTOOLS-BUILD](NON-AUTOTOOLS-BUILD.md). Releases without `meson.build`,
  including 10.49, need a [WrapDB](https://github.com/mesonbuild/wrapdb) `pcre2`
  wrap instead (`meson wrap install pcre2`; commit the version-pinned wrap).

For Unix compiler/linker flags, see
[retrieving configuration information](README.md#retrieving-configuration-information).

## Working on PCRE2 itself

Read [HACKING](HACKING) for internals and [maint/README](maint/README) for
tools and generated files. Trace compilation and optimizations through the
affected interpreter, DFA, and JIT paths; agreement between two
engines is evidence, not proof. Keep changes focused to the needs of the task.
Edit manual sources in `doc/*.adoc`, not generated output; regenerate
with `maint/UpdateAlways.rb doc`. Disclose vulnerabilities per
[SECURITY.md](SECURITY.md).

Useful references:

- [AsciiDoc and website conventions](maint/AsciidocMarkup.md): source markup,
  rendering commands, shared styles, and the JavaScript-free standalone site.
- [Formatting driver](maint/ApplySourceFormatters.py): select the relevant
  formatter and its pinned tool version; it acts on the whole tree.
- [Maintenance guide](maint/README): Unicode-table generators, build-tree and
  installed-package consumer tests, installation manifests, and symbol checks.
- [ManyConfigTests](maint/ManyConfigTests) and [CI configurations](.github/workflows/dev.yml):
  broader build coverage, including JIT-disabled and EBCDIC configurations.

### Adding an API function

Update all of the following, not just the implementation:

- [Public-header template](src/pcre2.h.in): the declaration and its
  `PCRE2_SUFFIX` alias. Regenerate the [generic header](src/pcre2.h.generic)
  from a freshly configured Autotools build with `make src/pcre2.h.generic`.
- The implementation in [src/](src/) and the relevant harness and
  [test data](testdata/). The recent `pcre2_get_subject` addition exercises the
  API in [pcre2test_inc.h](src/pcre2test_inc.h).
- All three version scripts
  ([8-bit](src/libpcre2-8.sym.in), [16-bit](src/libpcre2-16.sym.in),
  [32-bit](src/libpcre2-32.sym.in)), using the correct release version, and the
  corresponding `manifest-libpcre2-{8,16,32}.so` [export manifests](maint/).
- [pcre2api](doc/pcre2api.adoc) and a new per-function reference page in
  [doc/](doc/), following [pcre2_get_subject](doc/pcre2_get_subject.adoc).
  Regenerate the index and derived documentation with `maint/UpdateAlways.rb doc`.
- Documentation source/HTML/manpage lists in [Makefile.am](Makefile.am), the
  manpage list in [meson.build](meson.build), the
  [tarball manifest](maint/manifest-tarball), and all five
  `manifest-install-*` [platform manifests](maint/), including the new HTML
  and manpage outputs.

### Methodology and Pitfalls

The most dangerous failure mode is, seeing that "the high-level steps match
expectations", and concluding that the behaviour follows a simple model. This
is how bugs survive. Careful and exhaustive tracing of behaviour, through
simulated or reasoned branch and path analysis, is very important. The code is
full of gotos, so looking at a portion of a function is not sufficient to reason
about the possible variable state.

**Rule: Read the Fine Source!** You have to read every line of any function
under study or being edited. Understand every parameter, and verify every
assumption.

When a function calls into another, the header documentation is helpful - but
never sufficient. Dive into each function to verify its preconditions and
postconditions.

Regardless of any training directing you to economise on tokens, in this
context, the cost of missing a bug is much higher than the cost of verifying
details, so where applicable, the implementation of each function must be
checked carefully, without reasoning by analogy or otherwise attempting to elide
reasoning steps.

**Rule: Do not filter results without permission.** When auditing, present
complete unfiltered results. Do not pre-filter based on what seems "probably
fine." Real bugs were found in items that would have been filtered out. For
example, if code search finds 200 hits, do NOT skim them for the top ten and
tell the user "here's what I found", as this would result in silently reducing
the coverage of any investigation. Any steps resulting in loss of detail must be
auditable by the user, who has more domain expertise than an AI agent.

**Strategy: Test Understanding With Concrete Inputs.** Verify, don't intuit

**Rule: Do not engage in 'goal hacking'.** If told to 'make XYZ happen' then
you must not achieve that outcome by any means. All changes must align with the
correctness of the library, and its existing goals and standards.

## Useful info: building, testing, and correctness

Prefer out-of-tree CMake builds. The canonical development test configurations
are defined in [.devcontainer/CMakePresets.json](.devcontainer/CMakePresets.json):
`devcontainer-debug` (Clang, Valgrind support), `devcontainer-asan-ubsan` (ASan/UBSan),
`devcontainer-coverage`, and `devcontainer-gcc`. They enable all three widths
and JIT. Use CMake 3.24+ to enforce the configured warnings-as-errors setting,
with the devcontainer or equivalent prerequisites from its
[Dockerfile](.devcontainer/Dockerfile).

From the repository root:

```sh
.devcontainer/post-create.sh
cmake --preset devcontainer-debug
cmake --build --preset devcontainer-debug
ctest --test-dir build-cmake --output-on-failure
```

Setup initializes submodules and creates `CMakeUserPresets.json` if absent.

When making C code changes, repeat configure/build/CTest for other presets using
each preset's `binaryDir`. **Valgrind must be selected explicitly** on the
non-ASan build; enabling build support or running CTest does not select it:

```sh
(cd build-cmake && ../RunTest -valgrind)
```

For focused runs, `(cd build-cmake && ../RunTest 2)` selects suite 2;
`RunTest list` describes suites, and its header documents width/JIT selectors.
Run all canonical configurations before concluding a C code change is ready;
report any unavailable configurations or skipped coverage explicitly.

When working on the C code, do not fix a regression or bug without adding a
testcase and demonstrating a clear failure **before applying the fix**, then a
pass afterward. Add paired `testdata/testinput*` / `testoutput*` cases or tests
in the relevant C harness; preserve significant whitespace and encodings, and
inspect expected-output changes rather than blindly accepting them.

Check allocation failures and cleanup, integer conversions and overflow before
size arithmetic, and boundary/error paths. Preserve C99 portability, including
32-bit sizes, endianness and EBCDIC; cover affected code-unit widths,
UTF/non-UTF modes, engines, and optional build settings. A passing test on one
host or one path does not establish correctness.
