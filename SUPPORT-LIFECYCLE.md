# PCRE2 Support Lifecycle

This document outlines the support lifecycle policy for PCRE2 releases. It is
intended to provide clear guidance to distributors on the supported versions of
PCRE2, and the actions required to distribute older versions of PCRE2 securely.

## Release policy

PCRE2 releases follow a rolling version policy. There is one linear history of
releases, with each new release providing a mixture of enhancements and fixes to
the previous release.

PCRE2 is designed to have a high level of backwards-compatibility and stability.
Where possible, we recommend that consumers of the library update to the latest
version.

However, we recognise that Linux distributions in particular bundle older
versions for many years. To enable this practice, this document provides advice
on how to package and distribute older releases of PCRE2.

This document will be updated with each new PCRE2 release to reflect the current
support status and any new lifecycle recommendations.

## Actions for PCRE2 distributors and packagers

* Check this document on each release of PCRE2.
* Each version of PCRE2 that you are supporting should be listed here.
* Check for any changes in the "patches to apply" section, and consider
  backporting these.

## Support Policy

1. Each PCRE2 version will be supported for at least 5 years from its release
   date.
2. Selected releases older than 5 years may continue to receive support if they
   are actively distributed.
3. For supported versions, security and high-severity bug fixes will be
   backported and listed in this document.

The following distributions have been considered to determine support
requirements:

- RHEL/CentOS/Fedora: Long lifecycle. Although it's listed at 10+ years, there
  is effectively a freeze after five years of "Full Support", with presumably only
  CVE fixes being backported after that during "Maintenance Support".

  Packages: https://src.fedoraproject.org/rpms/pcre2

  Packages: https://gitlab.com/redhat/centos-stream/rpms/pcre2

- Debian: Five year lifecycle. There is a three-year period of full support. In
  practice, backports to "oldoldstable" releases are made only if absolutely
  required, such as a CVE that requires fixing for compliance reasons. The
  "stable" and "oldstable" releases actively accumulate backported fixes.

  Packages: https://salsa.debian.org/debian/pcre2

- Ubuntu: Five year LTS. I believe most customers update fairly promptly every
  two years, and few if any non-security backports are made to LTS releases
  older than the current one.

  Packages: https://code.launchpad.net/ubuntu/+source/pcre2

- Alpine: Two year lifecycle.

  Packages: https://gitlab.alpinelinux.org/alpine/aports/-/tree/master/main/pcre2?ref_type=heads

- SUSE: Extremely long lifecycle; PCRE2 will stop backporting fixes long before
  SUSE drops support.

  Packages: https://build.opensuse.org/package/show/openSUSE:Factory/pcre2

- Arch: Rolling release. No need for backports. However, it is a major upstream
  packaging source for the Arch-based ecosystem.

  Packages: https://gitlab.archlinux.org/archlinux/packaging/packages/pcre2

- Other notable primary packaging sources include:
  * Gentoo

    https://gitweb.gentoo.org/repo/gentoo.git/tree/dev-libs/libpcre2

  * NixOS

    https://github.com/NixOS/nixpkgs/blob/master/pkgs/development/libraries/pcre2/default.nix

Please contact us if you would like the PCRE2 maintainers to be aware of your
packages, particularly if you are a distributor of PCRE2, and provide a support
lifecycle for old (not-latest) PCRE2 releases.

## Supported Versions

Below is the list of supported PCRE2 versions. For each version, specific
recommendations and backported fixes (if any) are provided.

### PCRE2 10.32 10-September-2018

* Shipped in RHEL/CentOS 8 (EOL 31 May 2024).

This version is older than 5 years, but remains listed in SUPPORT-LIFECYCLE.md
potentially until May 2029 (end of "Maintenance Support" for RHEL 8), in
recognition that customer workloads may be using this version until then.

I am not providing a recommendation of what patches to apply. Distributors still
shipping this release have likely frozen their codebase at this point.

For the record, RHEL 8 used the following sets of patches:
https://src.fedoraproject.org/rpms/pcre2/tree/f28,
https://gitlab.com/redhat/centos-stream/rpms/pcre2/-/tree/c8s?ref_type=heads

The most serious of these was the backported mitigation for CVE-2019-20454 from
the fix in 10.34.

### PCRE2 10.36 04-December-2020

* Shipped in Debian 11 "Bullseye" (EOL 31 Aug 2026).

I am not providing a recommendation of what patches to apply. Distributors still
shipping this release have likely frozen their codebase at this point, since the
release of Debian 12.

### PCRE2 10.37 26-May-2021

I am not providing a recommendation of what patches to apply. I am not aware of
any Linux distributions providing extended support for 10.37. I have not tested
this release with any backported fixes applied.

Patches introduced:
* `patches/pcre2-10.37-Remove-real-POSIX.patch`: To be backported to older
  versions as required. This changes a behaviour which has been present since
  the beginning of PCRE2's libpcre2posix.so, although the behaviour was updated
  in 10.33. Versions prior to 10.33 cannot benefit from the backport; versions
  10.33-10.36 may use the backport at the distributor's discretion (that is, if
  users have requested it; but it is not needed otherwise).

### PCRE2 10.38 01-October-2021

Do not use (update to 10.39).

### PCRE2 10.39 29-October-2021

* Shipped in Ubuntu 22.04 "Jammy Jellyfish" (EOL 01 Apr 2027).

I am not providing a recommendation of what patches to apply. Distributors still
shipping this release have likely frozen their codebase at this point, since the
release of Ubuntu 24.04.

Patches introduced:
* `patches/pcre2-10.39-Fix-incorrect-detection.patch`: Significant bugfix for
  JIT matching

### PCRE2 10.40 15-April-2022

* Shipped in RHEL/CentOS 9 (EOL 31 May 2027).

I am not providing a recommendation of what patches to apply. Distributors still
shipping this release have likely frozen their codebase at this point, since the
release of RHEL 10.

For the record, RHEL/CentOS 9 ships rather more backported patches than other
distributions:
https://gitlab.com/redhat/centos-stream/rpms/pcre2/-/tree/c9s?ref_type=heads

Some of these patches may be relevant to other distributions, but I have not
tested them myself for compatibility with 10.40 or other earlier releases.

Patches introduced:
* `patches/pcre2-10.40-A-Fixed-a-unicode.patch`: Fixed CVE-2022-1586. This
  affects all recent previous versions of PCRE2.
* `patches/pcre2-10.40-B-Fixed-an-issue-affecting.patch`: Fixed CVE-2022-1587.
  This affects all recent previous versions of PCRE2.

### PCRE2 10.41 06-December-2022

Do not use (update to 10.42).

Introduced the fix for CVE-2022-41409. This only affects the pcre2test runner
for the test suite, so this fix is not backported.

### PCRE2 10.42 11-December-2022

* Shipped in Debian 12 "Bookworm" (EOL 30 Jun 2028).
* Shipped in Ubuntu 24.04 "Noble Numbat" (EOL 25 Apr 2029).

Users on RISC-V are advised to update to 10.43, or not use the JIT unless using
a backport for https://github.com/zherczeg/sljit/pull/223. Given the small
RISC-V userbase (especially on older releases), disabling the JIT for RISC-V
builds is likely acceptable.

Patches to apply:
* `patches/pcre2-10.43-Avoid-LIMIT_HEAP-integer.patch`
* `patches/pcre2-10.43-Fix-heapframe-overflow.patch`

### PCRE2 10.43 16-February-2024

* Shipped in Alpine 3.20 (EOL 01 Apr 2026)
* Shipped in Alpine 3.21 (EOL 01 Nov 2026)

Patches to apply:
* `patches/pcre2-10.44-Fix-locking-region.patch`
* `patches/pcre2-10.44-Fix-incorrect-compiling.patch`

Patches introduced:
* `patches/pcre2-10.43-Avoid-LIMIT_HEAP-integer.patch`: Fix integer overflow in
  handling of LIMIT_HEAP. This is not a critical fix for backporting, and in
  10.41, the code for handling the heapframes changed to always using a
  heap-allocated frames vector. This patch is therefore recommended for
  backporting to 10.42 only.
* `patches/pcre2-10.43-Fix-heapframe-overflow.patch`: Fix buffer overrun in
  handling of LIMIT_HEAP. These patches can be backported to all recent previous
  versions (since LIMIT_HEAP was introduced in 10.30).

### PCRE2 10.44 07-June-2024

* Shipped in RHEL/CentOS 10 (EOL 31 May 2030).

Patches to apply:
* `patches/pcre2-10.45-Memory-reports-only-compiled.patch`

Patches introduced:
* `patches/pcre2-10.44-Fix-locking-region.patch`: To be backported to 10.43 only
  (bug introduced in 10.43). Crashes in multithreaded applications using the PCRE2
  JIT were likely in 10.43 without this patch.
* `patches/pcre2-10.44-Fix-incorrect-compiling.patch`: Fixes a bug causing
  incorrect compilation of certain patterns which could crashes, for backporting
  to 10.43. Because 10.42 and earlier required fixed-length lookbehinds, the
  patch does not require backporting to earlier versions.

### PCRE2 10.45 05-February-2025

Do not use (update to 10.46, which is is a drop-in compatible release with a
security fix).

Patches introduced:
* `patches/pcre2-10.45-Memory-reports-only-compiled.patch`: To be backported to
  10.44 only. This is a test-only fix, so is not important to backport unless
  test suite failures are observed.

### PCRE2 10.46 27-August-2025

* Shipped in Debian 13 "Trixie" (EOL 30 Jun 2030).
* Shipped in Alpine 3.22 (EOL 01 May 2027)

Introduced the fix for CVE-2025-58050. This only affects 10.45. Do not backport
the patch (just update to 10.46).

Patches to apply:
* `patches/pcre2-10.47-Fix-for-callback.patch`

### PCRE2 10.47 21-October-2025

Patches introduced:
* `patches/pcre2-10.47-Fix-for-callback.patch`: A fix for a significant memory
  read overrun in a function rarely called by applications, to be backported to
  10.45 and 10.46 only (bug introduced in 10.45).
* There is a new and rewritten JIT implementation for the AArch64 SIMD code,
  fixing crashes and out-of-bounds memory reads in the previous, legacy AArch64
  JIT. This is probably not possible to backport.
