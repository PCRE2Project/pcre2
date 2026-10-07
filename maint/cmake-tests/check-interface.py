#!/usr/bin/env python3

import argparse
import os
import platform
import re
import shlex
import shutil
import subprocess
import tempfile
from pathlib import Path


def cmake_flags():
    return shlex.split(os.environ.get('CMAKE_FLAGS', ''), posix=os.name != 'nt')


def find_executable(build_dir, build_type):
    suffix = '.exe' if os.name == 'nt' else ''
    candidates = list(build_dir.rglob('test_executable' + suffix))
    if build_type:
        configured = [path for path in candidates if path.parent.name == build_type]
        if configured:
            candidates = configured
    if len(candidates) != 1:
        raise RuntimeError(f'expected one test executable, found {candidates}')
    return candidates[0]


def find_dumpbin():
    dumpbin = shutil.which('dumpbin')
    if dumpbin:
        return [dumpbin, '/dependents']

    vswhere = Path(os.environ.get(
        'ProgramFiles(x86)', r'C:\Program Files (x86)'
    )) / 'Microsoft Visual Studio/Installer/vswhere.exe'
    output = subprocess.check_output([
        str(vswhere), '-latest',
        '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
        '-find', r'VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe',
    ], text=True)
    return [output.splitlines()[0], '/dependents']


def dependencies(executable):
    system = platform.system()
    if system == 'Windows':
        command = find_dumpbin()
    elif system == 'Darwin':
        command = ['otool', '-L']
    else:
        command = ['ldd']
    output = subprocess.check_output(command + [str(executable)], text=True)
    print(output, end='')
    return output.lower()


def check_linkage(output, shared, build_type, expected_library=None):
    pcre2_library = re.compile(
        r'\b(?:lib)?pcre2-(?:8|16|32|posix)d?(?:\.\d+)*'
        r'\.(?:dll|dylib|so(?:\.\d+)*)\b'
    )
    if not shared:
        if pcre2_library.search(output):
            raise RuntimeError('PCRE2 found in static consumer dependencies')
        return

    system = platform.system()
    if system == 'Windows':
        debug_suffix = 'd' if build_type.lower() == 'debug' else ''
        expected = f'pcre2-8{debug_suffix}.dll'
    elif system == 'Darwin':
        expected = '@rpath/libpcre2-8.0.dylib'
    else:
        expected = 'libpcre2-8.so.0'
    if expected not in output:
        raise RuntimeError(f'{expected} not found in shared consumer dependencies')
    if expected_library and str(expected_library).lower() not in output:
        raise RuntimeError(
            f'{expected_library} not found in shared consumer dependencies'
        )


def run_case(args, source_dir, build_dir, shared):
    configure = ['cmake'] + cmake_flags()
    if args.generator:
        configure += ['-G', args.generator]
    if args.architecture:
        configure += ['-A', args.architecture]
    configure += [
        '-S', str(source_dir), '-B', str(build_dir),
        f'-DCMAKE_BUILD_TYPE={args.build_type}',
    ]
    if args.mode == 'install':
        configure += [
            f'-DCMAKE_PREFIX_PATH={args.install_prefix.resolve()}',
            f'-DPCRE2_USE_STATIC_LIBS={"OFF" if shared else "ON"}',
        ]
    else:
        configure += [
            f'-DPCRE2_SOURCE_DIR={args.source_dir.resolve()}',
            f'-DBUILD_STATIC_LIBS={"OFF" if shared else "ON"}',
            f'-DBUILD_SHARED_LIBS={"ON" if shared else "OFF"}',
        ]

    linkage = 'shared' if shared else 'static'
    print(f'== Testing CMake {args.mode} interface with {linkage} linkage ==')
    subprocess.run(configure, check=True)
    subprocess.run([
        'cmake', '--build', str(build_dir), '--config', args.build_type,
    ], check=True)
    executable = find_executable(build_dir, args.build_type)
    subprocess.run([str(executable)], check=True)
    expected_library = None
    if shared and platform.system() == 'Linux':
        if args.mode == 'install':
            expected_library = args.install_prefix.resolve() / 'lib/libpcre2-8.so.0'
        else:
            expected_library = build_dir.resolve() / 'pcre2/libpcre2-8.so.0'
    check_linkage(
        dependencies(executable), shared, args.build_type, expected_library
    )


def main():
    parser = argparse.ArgumentParser(
        description='Test CMake build-tree or installed-package consumers.'
    )
    subparsers = parser.add_subparsers(dest='mode', required=True)
    for mode in ('install', 'build-tree'):
        subparser = subparsers.add_parser(mode)
        subparser.add_argument('--build-type', default='Release')
        subparser.add_argument('--generator')
        subparser.add_argument('--architecture')
        if mode == 'install':
            subparser.add_argument('install_prefix', type=Path)
        else:
            subparser.add_argument('source_dir', type=Path)
    args = parser.parse_args()

    fixture_name = 'install-interface' if args.mode == 'install' else 'build-interface'
    fixture_dir = Path(__file__).resolve().parent / fixture_name
    with tempfile.TemporaryDirectory(prefix=f'pcre2-cmake-{args.mode}-') as temp:
        for shared in (False, True):
            run_case(args, fixture_dir, Path(temp) / ('shared' if shared else 'static'), shared)

    print(f'CMake {args.mode} interface checks passed')


if __name__ == '__main__':
    main()
