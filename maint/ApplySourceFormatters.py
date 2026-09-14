#! /usr/bin/env python3

# Apply formatters to source files in the PCRE2 source tree.
#
# This script should be run in the main PCRE2 directory. Most files use the
# standard formatting style. pcre2demo.c uses a narrower style, while generated
# tables and templates are checked with the standard style after formatting has
# been made stable by their generators.

import argparse
import glob
from pathlib import Path
import shutil
import subprocess
import sys

CLANG_FORMAT_COMMAND = 'clang-format-20'
BUILDIFIER_VERSION = '8.5.1'
BUILDIFIER_DOWNLOAD_URL = ('https://github.com/bazel-contrib/buildtools/'
                           'releases/download/v8.5.1/buildifier-linux-amd64')
ZIG_VERSION = '0.16.0'
YAPF_VERSION = '0.43.0'
GERSEMI_VERSION = '0.29.0'


def require_command(command, version_args, expected_version, download_url=None):
    if shutil.which(command) is None:
        print('** %s is required but is not on PATH' % command, file=sys.stderr)
        if download_url is not None:
            print('   Download it from %s' % download_url, file=sys.stderr)
        sys.exit(1)

    version = subprocess.check_output([command, *version_args], text=True).strip()
    if expected_version not in version:
        print('** %s version %s is required; found: %s' % (command, expected_version, version), file=sys.stderr)
        if download_url is not None:
            print('   Download it from %s' % download_url, file=sys.stderr)
        sys.exit(1)


def venv_tool(name, package, version):
    venv_directory = Path('maint/formatting/venv-' + name)
    tool = venv_directory / 'bin' / name

    if not venv_directory.exists():
        print('Creating %s' % venv_directory)
        subprocess.run([sys.executable, '-m', 'venv', str(venv_directory)], check=True)
        subprocess.run([
            str(venv_directory / 'bin' / 'python'), '-m', 'pip', 'install', '--disable-pip-version-check',
            package + '==' + version
        ],
                       check=True)
    elif not tool.exists():
        print('** %s exists but does not contain %s' % (venv_directory, name), file=sys.stderr)
        sys.exit(1)

    require_command(str(tool), ['--version'], version)
    return str(tool)


#######################
# Run clang-format
#######################
def run_clang_format():
    require_command(CLANG_FORMAT_COMMAND, ['--version'], '20')

    EXCLUDED_FILES = {
        'maint/pcre2_chartables.c.non-standard',
        'src/config.h.generic',
        'src/pcre2.h.in',
        'src/pcre2.h.generic',
        'src/pcre2posix.h',
    }

    NARROW_FILES = {
        'src/pcre2demo.c',
    }

    CHECK_FILES = {
        'src/config-cmake.h.in',
        'src/pcre2_chartables.c.dist',
        'src/pcre2_chartables.c.ebcdic-1047-nl15',
        'src/pcre2_chartables.c.ebcdic-1047-nl25',
        'src/pcre2_ucd.c',
        'src/pcre2_ucp.h',
        'src/pcre2_ucptables_inc.h',
    }

    source_files = set()
    for pattern in ('src/**/*.[ch]', 'src/**/*.[ch].*', 'maint/**/*.[ch]', 'maint/**/*.[ch].*'):
        source_files.update(glob.glob(pattern, recursive=True))

    apply_files = sorted(source_files - EXCLUDED_FILES - NARROW_FILES - CHECK_FILES)
    narrow_files = sorted(source_files & NARROW_FILES)
    check_files = sorted(source_files & CHECK_FILES)
    assert len(narrow_files) == len(NARROW_FILES), "Mismatch to expected narrow files"
    assert len(check_files) == len(CHECK_FILES), "Mismatch to expected check files"

    def call_format(style, files, check_only=False):
        args = ['-i'] if not check_only else ['--dry-run', '--Werror']
        subprocess.run([CLANG_FORMAT_COMMAND, *args, '--style=file:' + style, *files], check=True)

    call_format('maint/formatting/.clang-format', apply_files)
    call_format('maint/formatting/clang-format-narrow/.clang-format', narrow_files)
    call_format('maint/formatting/.clang-format', check_files, check_only=True)


#######################
# Run yapf
#######################
def run_yapf():
    python_files = sorted({
        filename
        for pattern in ('*.py', 'maint/**/*.py')
        for filename in glob.glob(pattern, recursive=True) if '/venv-' not in filename
    })
    yapf = venv_tool('yapf', 'yapf', YAPF_VERSION)
    subprocess.run([yapf, '-i', '--style=maint/formatting/style.yapf', *python_files], check=True)


#######################
# Run gersemi
#######################
def run_gersemi():
    definition_files = sorted({
        filename
        for pattern in ('CMakeLists.txt', 'cmake/**/*.cmake', 'cmake/**/*.cmake.in')
        for filename in glob.glob(pattern, recursive=True)
    })
    cmake_files = sorted({
        filename
        for pattern in ('CMakeLists.txt', 'cmake/**/*.cmake', 'cmake/**/*.cmake.in',
                        'maint/cmake-tests/**/CMakeLists.txt')
        for filename in glob.glob(pattern, recursive=True)
    })
    gersemi = venv_tool('gersemi', 'gersemi', GERSEMI_VERSION)
    # Format a little wider than then 100 columns we use for C. CMake is edited
    # less often, and is uglier when it wraps.
    subprocess.run([
        gersemi, '-i', '--indent', '2', '--line-length', '120', '--definitions', *definition_files, '--', *cmake_files
    ],
                   check=True)


#######################
# Run buildifier
#######################
def run_buildifier():
    require_command('buildifier', ['--version'], BUILDIFIER_VERSION, BUILDIFIER_DOWNLOAD_URL)
    subprocess.run(['buildifier', '--lint=warn', 'BUILD.bazel', 'MODULE.bazel'], check=True)


#######################
# Run zig fmt
#######################
def run_zig_format():
    require_command('zig', ['version'], ZIG_VERSION)
    subprocess.run(['zig', 'fmt', 'build.zig', 'build.zig.zon'], check=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Apply source formatters to the PCRE2 tree.')
    parser.add_argument('formatter',
                        nargs='?',
                        default='all',
                        choices=('all', 'clang-format', 'yapf', 'gersemi', 'buildifier', 'zig'),
                        help='formatter to run (default: all)')
    arguments = parser.parse_args()

    formatters = {
        'clang-format': run_clang_format,
        'yapf': run_yapf,
        'gersemi': run_gersemi,
        'buildifier': run_buildifier,
        'zig': run_zig_format,
    }
    formatter_names = formatters.keys() if arguments.formatter == 'all' else [arguments.formatter]
    for formatter_name in formatter_names:
        formatters[formatter_name]()
