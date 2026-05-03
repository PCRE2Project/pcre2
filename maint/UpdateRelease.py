#! /usr/bin/env python3

# Script to update all the hardcoded release numbers in the source tree.
#  - Documentation manpages.
#  - Bazel MODULE file.
#  - Zig package manifest.

# This script should be run in the main PCRE2 directory.

import glob
import re

from UpdateCommon import update_file, CURRENT_RELEASE

def update_man_version(filename):
    print('  Updating %s' % filename)
    update_file(filename, r'(.TH.*? )"PCRE2 .*?"', '\\1"PCRE2 %s"' % CURRENT_RELEASE)

print('Updating man pages')

# doc/*.1
for filename in glob.glob('doc/*.1'):
    update_man_version(filename)

# doc/*.3
for filename in glob.glob('doc/*.3'):
    update_man_version(filename)

# MODULE.bazel
print('Updating MODULE.bazel')
update_file('MODULE.bazel', r'(?m)^    version = ".*?"', '    version = "%s"' % CURRENT_RELEASE)

# build.zig.zon
# Zig requires a semver MAJOR.MINOR.PATCH[-prerelease], so insert .0 as patch.
print('Updating build.zig.zon')
ZIG_VERSION = re.sub(r'^(\d+\.\d+)(.*)$', r'\1.0\2', CURRENT_RELEASE)
update_file('build.zig.zon', r'(?m)^    \.version = ".*?"', '    .version = "%s"' % ZIG_VERSION)
