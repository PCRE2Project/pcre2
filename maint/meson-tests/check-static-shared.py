#!/usr/bin/env python3

import argparse
import json
import re
import shlex
import subprocess
import tempfile
from pathlib import Path


def introspect(build_dir, query):
    output = subprocess.check_output(
        ['meson', 'introspect', query, str(build_dir)], text=True
    )
    return json.loads(output)


def has_define(arguments, name):
    return any(argument in (f'-D{name}', f'/D{name}') for argument in arguments)


def check_compile_variants(build_dir):
    commands = json.loads((build_dir / 'compile_commands.json').read_text())
    core_commands = [
        command for command in commands
        if Path(command['file']).name == 'pcre2_compile.c'
    ]

    for width in ('8', '16', '32'):
        width_commands = [
            command for command in core_commands
            if f'PCRE2_CODE_UNIT_WIDTH={width}' in command['command']
        ]
        if not width_commands:
            continue
        if len(width_commands) != 2:
            raise RuntimeError(
                f'expected two pcre2_compile.c commands for width {width}, '
                f'found {len(width_commands)}'
            )

        static_commands = [
            command for command in width_commands
            if has_define(shlex.split(command['command']), 'PCRE2_STATIC')
        ]
        if len(static_commands) != 1:
            raise RuntimeError(
                f'expected PCRE2_STATIC on exactly one width-{width} command'
            )
        if len({command['output'] for command in width_commands}) != 2:
            raise RuntimeError(f'width-{width} static and shared objects are reused')

    posix_commands = [
        command for command in commands
        if Path(command['file']).name == 'pcre2posix.c'
    ]
    if len(posix_commands) != 2:
        raise RuntimeError(
            f'expected two pcre2posix.c commands, found {len(posix_commands)}'
        )
    posix_arguments = [shlex.split(command['command']) for command in posix_commands]
    if sum(has_define(arguments, 'PCRE2_STATIC') for arguments in posix_arguments) != 1:
        raise RuntimeError('expected PCRE2_STATIC on exactly one POSIX command')
    if sum(has_define(arguments, 'PCRE2POSIX_SHARED') for arguments in posix_arguments) != 1:
        raise RuntimeError('expected PCRE2POSIX_SHARED on exactly one POSIX command')
    if len({command['output'] for command in posix_commands}) != 2:
        raise RuntimeError('POSIX static and shared objects are reused')


def check_embedded_archive(build_dir, fixture_source):
    targets = introspect(build_dir, '--targets')
    archives = [
        Path(filename)
        for target in targets
        if target['name'] == 'pcre2-8' and target['type'] == 'static library'
        for filename in target['filename']
    ]
    if len(archives) != 1:
        raise RuntimeError(f'expected one 8-bit static archive, found {len(archives)}')

    compiler = introspect(build_dir, '--compilers')['host']['c']['linker_exelist']
    with tempfile.TemporaryDirectory(dir=build_dir) as temp_dir:
        fixture_library = Path(temp_dir) / 'libpcre2-static-fixture.so'
        command = compiler + [
            '-shared', '-o', str(fixture_library), str(fixture_source),
            '-Wl,--whole-archive', str(archives[0]), '-Wl,--no-whole-archive',
        ]
        try:
            subprocess.run(command, check=True)
        except subprocess.CalledProcessError as error:
            raise RuntimeError(
                'could not embed the static archive; configure with '
                '-Db_staticpic=true'
            ) from error

        symbols = subprocess.check_output(
            ['nm', '-D', '--defined-only', str(fixture_library)], text=True
        )
        if not re.search(r'\bfixture_export$', symbols, re.MULTILINE):
            raise RuntimeError('fixture export is missing from the shared library')
        exported_pcre2 = [
            line for line in symbols.splitlines()
            if re.search(r'\bpcre2_.*_8(?:@@?\S+)?$', line)
        ]
        if exported_pcre2:
            raise RuntimeError(
                'embedded static archive exports PCRE2 APIs:\n'
                + '\n'.join(exported_pcre2)
            )


def main():
    parser = argparse.ArgumentParser(
        description='Check independent Meson variants and hidden static symbols.'
    )
    parser.add_argument('build_dir', type=Path)
    args = parser.parse_args()
    build_dir = args.build_dir.resolve()
    fixture_source = Path(__file__).with_name('static-shared-fixture.c')

    check_compile_variants(build_dir)
    check_embedded_archive(build_dir, fixture_source)
    print('Meson static/shared object and visibility checks passed')


if __name__ == '__main__':
    main()