#! /usr/bin/env python3

# Script to update all the hardcoded release numbers in the source tree.
#  - CMake build metadata.
#  - AsciiDoc documentation sources.
#  - Bazel MODULE file.
#  - Zig package manifest.
#  - Installed library filenames in platform manifests.

# This script should be run in the main PCRE2 directory.

import glob
import re

from UpdateCommon import update_file, CURRENT_RELEASE, VERSION_INFO


def update_adoc_version(filename):
    print('  Updating %s' % filename)
    update_file(filename, r'(?m)^:mansource: PCRE2 .*$', ':mansource: PCRE2 %s' % CURRENT_RELEASE)


def libtool_filename_version(library):
    version = VERSION_INFO['libpcre2_%s_version' % library]

    try:
        current, revision, age = (int(value) for value in version.split(':'))
    except ValueError:
        raise Exception('Invalid libtool version for libpcre2-%s: %s' % (library, version))

    if current < age:
        raise Exception('Invalid libtool version for libpcre2-%s: current is less than age' % library)

    return '%d.%d.%d' % (current - age, age, revision)


print('Updating CMakeLists.txt')
cmake_versions = {
    'PCRE2_MAJOR': 'pcre2_major',
    'PCRE2_MINOR': 'pcre2_minor',
    'PCRE2_PRERELEASE': 'pcre2_prerelease',
    'PCRE2_DATE': 'pcre2_date',
    'LIBPCRE2_POSIX_VERSION': 'libpcre2_posix_version',
    'LIBPCRE2_8_VERSION': 'libpcre2_8_version',
    'LIBPCRE2_16_VERSION': 'libpcre2_16_version',
    'LIBPCRE2_32_VERSION': 'libpcre2_32_version',
}
for variable, configure_name in cmake_versions.items():
    update_file('CMakeLists.txt', r'(?m)^set\(%s ".*"\)$' % variable,
                'set(%s "%s")' % (variable, VERSION_INFO[configure_name]))

print('Updating AsciiDoc sources')
for filename in glob.glob('doc/*.adoc'):
    if filename == 'doc/index.adoc':
        continue
    update_adoc_version(filename)

# MODULE.bazel
print('Updating MODULE.bazel')
update_file('MODULE.bazel', r'(?m)^    version = ".*?"', '    version = "%s"' % CURRENT_RELEASE)

# build.zig.zon
# Zig requires a semver MAJOR.MINOR.PATCH[-prerelease], so insert .0 as patch.
print('Updating build.zig.zon')
ZIG_VERSION = re.sub(r'^(\d+\.\d+)(.*)$', r'\1.0\2', CURRENT_RELEASE)
update_file('build.zig.zon', r'(?m)^    \.version = ".*?"', '    .version = "%s"' % ZIG_VERSION)

# Install manifests
print('Updating install manifests')
for filename in sorted(glob.glob('maint/manifest-*install-*')):
    if filename.endswith('-windows'):
        continue

    print('  Updating %s' % filename)
    is_macos = filename.endswith('-macos')

    for library in ('8', '16', '32', 'posix'):
        version = libtool_filename_version(library)

        if is_macos:
            pattern = r'libpcre2-%s\.\d+\.\d+\.\d+\.dylib' % library
            replacement = 'libpcre2-%s.%s.dylib' % (library, version)
        else:
            pattern = r'libpcre2-%s\.so\.\d+\.\d+\.\d+' % library
            replacement = 'libpcre2-%s.so.%s' % (library, version)

        update_file(filename, pattern, replacement)
