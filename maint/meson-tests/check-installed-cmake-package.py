#!/usr/bin/env python3

import argparse
import os
import subprocess
import tempfile
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(
        description='Test an installed PCRE2 CMake package with Meson.'
    )
    parser.add_argument('install_prefix', type=Path)
    args = parser.parse_args()

    install_prefix = args.install_prefix.resolve()
    source_dir = Path(__file__).resolve().parent / 'cmake-installed-interface'
    test_env = os.environ.copy()
    test_env['PATH'] = str(install_prefix / 'bin') + os.pathsep + test_env['PATH']

    with tempfile.TemporaryDirectory(prefix='pcre2-meson-cmake-installed-') as temp:
        for linkage in ('default', 'static', 'shared'):
            build_dir = Path(temp) / linkage
            subprocess.run([
                'meson', 'setup', str(build_dir), str(source_dir),
                f'-Dcmake_prefix_path={install_prefix}',
                f'-Dlinkage={linkage}',
            ], check=True)
            subprocess.run(['meson', 'compile', '-C', str(build_dir)], check=True)
            subprocess.run([
                'meson', 'test', '-C', str(build_dir), '--print-errorlogs',
            ], check=True, env=test_env)

    print('Meson consumers of the installed CMake package passed')


if __name__ == '__main__':
    main()
