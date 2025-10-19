<picture>
  <source media="(prefers-color-scheme: dark)" width="100%" height="100px" srcset="https://raw.githubusercontent.com/PCRE2Project/pcre2/refs/heads/pages/pages/static/images/pcre2-readme-dark.svg">
  <img alt="PCRE2: Perl-Compatible Regular Expressions" width="100%" height="100px" src="https://raw.githubusercontent.com/PCRE2Project/pcre2/refs/heads/pages/pages/static/images/pcre2-readme-light.svg">
</picture>

## Overview

The PCRE2 library is a set of C functions that implement **regular expression
pattern matching**.

It is **self-contained and portable**, and designed to be **easy to embed** into existing
projects and build systems, on almost **any platform** or build target.

The PCRE2 library is **free and open-source** (BSD licence), and permitted in proprietary software.

It supports Unicode matching and a very wide range of regular expression features. It accepts input in various character encodings, and optionally includes a highly **performant JIT matching engine**.

PCRE2 is **mature and highly-trusted**: bundled in dozens or hundreds of open-source and commercial products, such as Excel, Safari, Apache, and Git, and used as the basis for regular expressions in several programming languages including PHP and R.

<table border="0">
<tbody>
<tr>
<th align="left">Website</th>
<td>

https://pcre2project.github.io/pcre2/

</td>
</tr>
<tr>
<th align="left">Distribution</th>
<td>

[![GitHub Release](https://img.shields.io/github/v/release/PCRE2Project/pcre2?display_name=release&style=flat-square&label=Latest%20release&color=006094)](https://github.com/PCRE2Project/pcre2/releases)&nbsp;
[![BSD licence](https://img.shields.io/badge/Licence-BSD%203--clause-006094?style=flat-square)](https://github.com/PCRE2Project/pcre2/blob/master/LICENCE.md)

</tr>
</tr>
<tr>
<th align="left">Testing</th>
<td>

[![Codecov](https://img.shields.io/codecov/c/github/PCRE2Project/pcre2?component=library&style=flat-square&logo=codecov&label=Coverage&color=009400)](https://app.codecov.io/gh/PCRE2Project/pcre2/components)&nbsp;
[![Clang Sanitizers](https://img.shields.io/badge/Clang-Sanitizers-262D3A?style=flat-square&logo=llvm&color=006094)](https://github.com/PCRE2Project/pcre2/actions/workflows/dev.yml)&nbsp;
[![Clang Static Analyzer](https://img.shields.io/badge/Clang-Static%20Analyzer-262D3A?style=flat-square&logo=llvm&color=006094)](https://github.com/PCRE2Project/pcre2/actions/workflows/clang-analyzer.yml)&nbsp;
[![Valgrind](https://img.shields.io/badge/Valgrind-006094?style=flat-square)](https://github.com/PCRE2Project/pcre2/actions/workflows/dev.yml)&nbsp;
[![Coverity Scan](https://img.shields.io/coverity/scan/pcre2?style=flat-square&label=Coverity&color=009400)](https://scan.coverity.com/projects/pcre2?tab=overview)&nbsp;
[![CodeQL](https://img.shields.io/badge/GitHub-CodeQL-006094?style=flat-square)](https://github.com/PCRE2Project/pcre2/actions/workflows/codeql.yml)&nbsp;
[![OSS-Fuzz](https://img.shields.io/badge/Google-OSS--Fuzz-006094?style=flat-square)](https://google.github.io/oss-fuzz/)&nbsp;
[![OSSF-Scorecard Score](https://img.shields.io/ossf-scorecard/github.com/PCRE2Project/pcre2?style=flat-square&label=OSSF-Scorecard&color=009400)](https://scorecard.dev/viewer/?uri=github.com%2FPCRE2Project%2Fpcre2)&nbsp;

</td>
</tr>
<tr>
<th align="left">Platforms</th>
<td>Tested continuously on Linux, Windows, macOS, FreeBSD, OpenBSD, Solaris, z/OS;<br />
x86, ARM, RISC-V, POWER, S390X; others known to work
</td>
</tr>
</tbody>
</table>

## Quickstart

<picture>
  <source media="(prefers-color-scheme: dark)" width="787px" srcset="https://github.com/user-attachments/assets/f38bd06c-abda-44bf-a3b1-3f9b59d3a287">
  <img alt="Recording of a terminal session showing the PCRE2 quickstart; reproduced in text form below" width="787px" src="https://github.com/user-attachments/assets/9d3e9d99-96e3-430a-83c1-e08024d83d27">
</picture>

<details>
<summary>Show script</summary>

```bash session
# Fetch PCRE2 with 'git clone', or use curl/wget to download a release.
# Here, let's use git to check out a release tag:
git clone https://github.com/PCRE2Project/pcre2.git ./pcre2 \
    --branch pcre2-$PCRE2_VERSION \
    -c advice.detachedHead=false --depth 1

# If using the JIT, remember to fetch the Git submodule:
(cd ./pcre2; git submodule update --init)

# Now let's build PCRE2:
(cd ./pcre2; \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug \
        -DPCRE2_SUPPORT_JIT=ON -B build; \
    cmake --build build/)

# Great, PCRE2 is built.

# Here's a quick little demo to show how we can make use of PCRE2.
# For a fuller example, see './pcre2/src/pcre2demo.c'.
# See below for the demo code.

# Compile the demo:
gcc -g -I./pcre2/build -L./pcre2/build demo.c -o demo -lpcre2-8

# Finally, run our demo:
./demo 'c.t' 'dogs and cats'

# We fetched, built, and called PCRE2 successfully! :)
```

File `demo.c`:

```c
/* Set PCRE2_CODE_UNIT_WIDTH to indicate we will use 8-bit input. */
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include <string.h> /* for strlen */
#include <stdio.h>  /* for printf */

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pattern> <subject>\n", argv[0]);
        return 1;
    }

    const char *pattern = argv[1];
    const char *subject = argv[2];

    /* Compile the pattern. */
    int error_number;
    PCRE2_SIZE error_offset;
    pcre2_code *re = pcre2_compile(
        pattern,               /* the pattern */
        PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
        0,                     /* default options */
        &error_number,         /* for error number */
        &error_offset,         /* for error offset */
        NULL);                 /* use default compile context */
    if (re == NULL) {
        fprintf(stderr, "Invalid pattern: %s\n", pattern);
        return 1;
    }

    /* Match the pattern against the subject text. */
    pcre2_match_data *match_data =
        pcre2_match_data_create_from_pattern(re, NULL);
    int rc = pcre2_match(
        re,                   /* the compiled pattern */
        subject,              /* the subject text */
        strlen(subject),      /* the length of the subject */
        0,                    /* start at offset 0 in the subject */
        0,                    /* default options */
        match_data,           /* block for storing the result */
        NULL);                /* use default match context */

    /* Print the match result. */
    if (rc == PCRE2_ERROR_NOMATCH) {
        printf("No match\n");
    } else if (rc < 0) {
        fprintf(stderr, "Matching error\n");
    } else {
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
        printf("Found match: '%.*s'\n", (int)(ovector[1] - ovector[0]),
               subject + ovector[0]);
    }

    pcre2_match_data_free(match_data);   /* Free resources */
    pcre2_code_free(re);
    return 0;
}
```

</details>

---

The main ways of obtaining PCRE2 are:

1. Via Git clone:

    ```
    git clone https://github.com/PCRE2Project/pcre2.git
    ```

    Please use a release tag in production, not the development branch!

    Because PCRE2's JIT uses code from a Git submodule, you must check this out after a fresh clone:

    ```
    git submodule update --init
    ```

3. Via download of the [release tarball](https://github.com/PCRE2Project/pcre2/releases/latest).

4. Finally, PCRE2 is also bundled by various downstream package managers (such as Linux distributions, or [vcpkg](https://vcpkg.io/)). These are provided by third parties, not the PCRE2 project.

The main ways of building PCRE2 are:

1. Via CMake (Linux/Windows/macOS, and others)

    ```
    cd pcre2/
    cmake -B build .
    cmake --build build/
    ```

2. Via Autoconf (Linux/Unix)

    ```
    cd pcre2/
    ./configure
    make
    ```

See ["Platforms"](#platforms) below for links to more detailed build documentation.

## API Overview

The PCRE2 API supports strings in 8-bit, 16-bit, and 32-bit encodings, with or without UTF encoding. There is also EBCDIC support.

The default regular expression dialect closely matches the syntax and behaviour of Perl 5, with PCRE2-specific extensions. A wide variety of granular flags can be passed to the PCRE2 API to customise this to more closely follow other dialects such as JavaScript or Python.

The default matching engine uses a depth-first tree search with backtracking, which is highly feature-rich but has worst-case exponential time (PCRE2 allows aborting the match if a time limit is exceeded, expressed as a maximum number of steps in the tree search). The second matching engine uses a JIT for greatly improved performance, compiling the regular expression to a block of equivalent native machine code.

PCRE2 has a third matching engine, using a DFA engine which is generally slower, but has worst-case polynomial matching time and is able to find the POSIX-style "leftmost-longest" match.

There are accompanying utility functions for converting glob patterns and POSIX BRE/ERE patterns to PCRE2 regular expressions; and also for performing high-level regular expression operations such as search-and-replace with a powerful replacement string syntax.

As well as the PCRE2 API, the library also offers a POSIX-compatible `<regex.h>` header and `regexec()` function. However, this does not provide the ability to pass PCRE2 flags, so we recommend users consume the PCRE2 API if possible.

See the [full library and API documentation](https://pcre2project.github.io/pcre2/doc/) for further details.

For third-party documentation, see further:

- A curated summary of changes for each PCRE release, and some excellent tutorials on PCRE2 on the
  [RexEgg website](http://www.rexegg.com/pcre-documentation.html).
- Jan Goyvaerts' popular Regular-Expressions.info site includes [information about PCRE2](https://www.regular-expressions.info/pcre.html) as well as tutorials and highly detailed comparisons of PCRE2 to other regular expression dialects.
- Jeffrey Friedl's book [_Mastering Regular Expressions_](https://regex.info/book.html) includes chapters on Perl and PCRE, and is available in print and online via O'Reilly Media.

## Platforms

PCRE2 is portable C code, and is likely to work on any system with a C99 compiler.

<dl>
<dt>Operating systems</dt>
<dd>
Our continuous integration tests on <strong>Linux</strong> (GCC and Clang, glibc and musl), <strong>Windows</strong> (MSVC and MinGW-x64), and <strong>macOS</strong> (Clang), as well as <strong>FreeBSD</strong>, <strong>OpenBSD</strong>, <strong>Solaris</strong> (Oracle Studio <code>cc</code>), and <strong>z/OS</strong> (<code>xlc</code> and <code>ibm-clang</code>).
</dd>
<dt>Processors</dt>
<dd>
PCRE2 is tested continuously on x86 (i686 and amd64), ARM 32- and 64-bit (armv7 and aarch64), RISC-V (riscv64), POWER (ppc64le), and the big-endian S390x.
</dd>
</dl>

Other systems are likely to work (including mobile, embedded platforms, and commercial UNIX systems), but these are not tested continuously by the PCRE2 maintainers. Users are encouraged to run the full PCRE2 test suite when compiling for any new platform. We are aware of working ports to VMS and z/OS (PCRE2 supports EBCDIC).

PCRE2 releases support CMake for building, and for UNIX platforms include a `./configure` script built by Autoconf. Build files for the Bazel build system and `zig build` are also included. Integrating PCRE2 with other systems can be done by including the `.c` files in an existing project.

Please see the files [README](./README) and [NON-AUTOTOOLS-BUILD](./NON-AUTOTOOLS-BUILD) for full build documentation, as well as the man pages, including [`man pcre2/doc/pcre2build.3`](https://pcre2project.github.io/pcre2/doc/pcre2build/).

## Licence

PCRE2 is released under the **BSD 3-clause licence** with a PCRE2 Exception. It is open-source and also corporate-friendly.

- See [LICENCE](./LICENCE.md) for legal text.
- See [AUTHORS](./AUTHORS.md) for details of the current maintainers of PCRE2 and acknowledgements of its contributors, including Philip Hazel, the original author.

## Contributing & support

Join the community by reporting issues or asking questions via [GitHub issues](https://github.com/PCRE2Project/pcre2/issues). We welcome feedback and proposals.

Contributions ranging from bug fixes to feature requests are welcome, and can be made via GitHub pull requests.

Please review our [SECURITY](./SECURITY.md) policy for information on reporting security issues.

Release announcements will be made via the [pcre2-dev@googlegroups.com](https://groups.google.com/g/pcre2-dev) mailing list, where you can also start discussions about PCRE2 issues and development. You can browse the [list archives](https://groups.google.com/g/pcre2-dev).
