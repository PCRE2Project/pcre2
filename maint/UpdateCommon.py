# Common helpers for UpdateRelease.py and UpdateDates.py.

import re
import os

script_dir = os.path.dirname(os.path.abspath(__file__))


def get_version_info():
    with open(f"{script_dir}/../configure.ac", 'r') as file:
        content = file.read()

    pattern = r"m4_define\((pcre2_(?:major|minor|prerelease|date)|libpcre2_(?:posix|8|16|32)_version), *\[(.*?)\]\)"
    return dict(re.findall(pattern, content))


VERSION_INFO = get_version_info()
CURRENT_RELEASE = '%s.%s%s' % (VERSION_INFO['pcre2_major'], VERSION_INFO['pcre2_minor'],
                               VERSION_INFO['pcre2_prerelease'])


# Update a file, using a pattern. Verify that it matches the file, and perform
# the replacement.
def update_file(filename, pattern, replacement):
    with open(filename, 'r') as file:
        content = file.read()

    if not re.search(pattern, content):
        raise Exception('Pattern not found in %s' % filename)

    content = re.sub(pattern, replacement, content)

    with open(filename, 'w') as file:
        file.write(content)
