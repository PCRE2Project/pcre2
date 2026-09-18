#! /usr/bin/env python3

# Script to update all the hardcoded release numbers in the source tree.
#  - CMake and Meson build metadata.
#  - AsciiDoc documentation sources.
#  - Bazel MODULE file.
#  - Zig package manifest.

# This script should be run in the main PCRE2 directory.

import glob
import re

from UpdateCommon import update_file, CURRENT_RELEASE, VERSION_INFO


def update_adoc_version(filename):
    print('  Updating %s' % filename)
    update_file(filename, r'(?m)^:mansource: PCRE2 .*$', ':mansource: PCRE2 %s' % CURRENT_RELEASE)


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

print('Updating meson.build')
update_file('meson.build', r"(?m)^  version: '.*',$",
            "  version: '%s.%s'," % (VERSION_INFO['pcre2_major'], VERSION_INFO['pcre2_minor']))
update_file('meson.build', r"(?m)^pcre2_prerelease = '.*'$",
            "pcre2_prerelease = '%s'" % VERSION_INFO['pcre2_prerelease'])
update_file('meson.build', r"(?m)^pcre2_date = '.*'$",
            "pcre2_date = '%s'" % VERSION_INFO['pcre2_date'])
for library in ('posix', '8', '16', '32'):
    configure_name = 'libpcre2_%s_version' % library
    update_file('meson.build', r"(?m)^  '%s': '.*',$" % library,
                "  '%s': '%s'," % (library, VERSION_INFO[configure_name]))

print('Updating AsciiDoc sources')
for filename in glob.glob('doc/*.adoc'):
    update_adoc_version(filename)

# MODULE.bazel
print('Updating MODULE.bazel')
update_file('MODULE.bazel', r'(?m)^    version = ".*?"', '    version = "%s"' % CURRENT_RELEASE)

# build.zig.zon
# Zig requires a semver MAJOR.MINOR.PATCH[-prerelease], so insert .0 as patch.
print('Updating build.zig.zon')
ZIG_VERSION = re.sub(r'^(\d+\.\d+)(.*)$', r'\1.0\2', CURRENT_RELEASE)
update_file('build.zig.zon', r'(?m)^    \.version = ".*?"', '    .version = "%s"' % ZIG_VERSION)
