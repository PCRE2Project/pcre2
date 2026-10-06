#!/usr/bin/env python3

import os
import shutil
import subprocess
import tempfile
from pathlib import Path


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
        pcre2_source = subprojects_dir / 'pcre2'
        pcre2_source.mkdir()
        for entry in source_root.iterdir():
            if entry.is_file() and not entry.name.startswith('.'):
                shutil.copy2(entry, pcre2_source / entry.name)
        for directory in ('src', 'meson', 'doc'):
            shutil.copytree(source_root / directory, pcre2_source / directory)
        for library_type in ('static', 'shared'):
            build_dir = work_dir / library_type
            subprocess.run([
                'meson', 'setup', str(build_dir), str(source_dir),
                *(['--vsenv'] if os.name == 'nt' else []),
                '--buildtype=release',
                '--wrap-mode=forcefallback',
                f'-Dpcre2:default_library={library_type}',
                '-Dpcre2:pcre2_build_tests=false',
                '-Dpcre2:pcre2_build_pcre2grep=false',
                '-Dpcre2:pcre2_support_libbz2=disabled',
                '-Dpcre2:pcre2_support_libz=disabled',
                '-Dpcre2:pcre2_support_libreadline=disabled',
            ], check=True)
            subprocess.run(['meson', 'compile', '-C', str(build_dir)], check=True)
            subprocess.run([
                'meson', 'test', '-C', str(build_dir), '--print-errorlogs',
            ], check=True)

    print('Meson static/shared subproject smoke tests passed')


if __name__ == '__main__':
    main()
