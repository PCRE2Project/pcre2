#!/usr/bin/env python3

import json
import shutil
import subprocess
import tempfile
from pathlib import Path


CASES = (
    ('static', 'static', 'shared', True),
    ('shared', 'shared', 'shared', False),
    ('both-shared', 'both', 'shared', False),
    ('both-static', 'both', 'static', True),
    ('both-auto', 'both', 'auto', False),
)


def run_case(source_dir, work_dir, case):
    name, library_type, both_type, default_static = case
    build_dir = work_dir / name
    request_static = library_type in ('static', 'both')
    request_shared = library_type in ('shared', 'both')
    setup_command = [
        'meson', 'setup', str(build_dir), str(source_dir),
        '--wrap-mode=forcefallback',
        f'-Dexpect_default_static={str(default_static).lower()}',
        f'-Drequest_static={str(request_static).lower()}',
        f'-Drequest_shared={str(request_shared).lower()}',
        f'-Dpcre2:default_library={library_type}',
        f'-Ddefault_both_libraries={both_type}',
        '-Dpcre2:pcre2_build_tests=false',
        '-Dpcre2:pcre2_build_pcre2grep=false',
        '-Dpcre2:pcre2_support_libbz2=disabled',
        '-Dpcre2:pcre2_support_libz=disabled',
        '-Dpcre2:pcre2_support_libreadline=disabled',
    ]
    subprocess.run(setup_command, check=True)
    subprocess.run(['meson', 'compile', '-C', str(build_dir)], check=True)
    subprocess.run(
        ['meson', 'test', '-C', str(build_dir), '--print-errorlogs'],
        check=True,
    )

    targets = json.loads(subprocess.check_output(
        ['meson', 'introspect', '--targets', str(build_dir)], text=True
    ))
    expected_types = set()
    if request_static:
        expected_types.add('static library')
    if request_shared:
        expected_types.add('shared library')
    for target_name in ('pcre2-8', 'pcre2-posix'):
        actual_types = {
            target['type'] for target in targets
            if target['subproject'] == 'pcre2' and target['name'] == target_name
        }
        if actual_types != expected_types:
            raise RuntimeError(
                f'{name}: {target_name} produced {actual_types}, '
                f'expected {expected_types}'
            )


def main():
    script_dir = Path(__file__).resolve().parent
    source_root = script_dir.parents[1]
    fixture_dir = script_dir / 'library-options'
    with tempfile.TemporaryDirectory(prefix='pcre2-meson-library-options-') as temp:
        work_dir = Path(temp)
        source_dir = work_dir / 'consumer'
        shutil.copytree(fixture_dir, source_dir)
        subprojects_dir = source_dir / 'subprojects'
        subprojects_dir.mkdir()
        (subprojects_dir / 'pcre2').symlink_to(source_root, target_is_directory=True)
        for case in CASES:
            run_case(source_dir, work_dir, case)

    print('Meson library option and fallback dependency checks passed')


if __name__ == '__main__':
    main()
