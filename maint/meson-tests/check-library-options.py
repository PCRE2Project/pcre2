#!/usr/bin/env python3

import argparse
import json
import os
import shutil
import subprocess
import tempfile
from pathlib import Path


def introspect(build_dir, kind):
    return json.loads(subprocess.check_output(['meson', 'introspect', f'--{kind}', str(build_dir)], text=True))


def check_library_names(build_dir, library_type, build_type, postfix):
    system = introspect(build_dir, 'machines')['host']['system']
    linker = introspect(build_dir, 'compilers')['host']['c']['linker_id']
    msvc_import = system == 'windows' and linker in ('link', 'lld-link', 'xilink', 'optlink')
    has_import = system in ('windows', 'cygwin')
    if postfix == 'auto':
        postfix = '-static'
    if not has_import:
        postfix = ''
    debug_postfix = 'd' if system == 'windows' and build_type == 'debug' else ''
    installed = {Path(filename).name for filename in introspect(build_dir, 'installed')}
    targets = introspect(build_dir, 'targets')
    static_reference = next(
        Path(target['filename'][0]).name for target in targets if target['name'] == 'naming-reference-static')
    if has_import:
        import_reference = [
            filename for filename in installed
            if 'naming-reference-shared' in filename and filename.endswith(('.a', '.lib'))
        ]
        assert len(import_reference) == 1, import_reference
    script = (build_dir / 'subprojects/pcre2/pcre2-config').read_text()
    for interface in ('8', '16', '32', 'posix'):
        name = f'pcre2-{interface}'
        if library_type != 'shared':
            static_name = f'{name}{postfix}{debug_postfix}'
            expected = (f'{static_name}.lib' if msvc_import else static_reference.replace(
                'naming-reference-static', static_name))
            assert expected in installed, (expected, installed)
        if has_import and library_type != 'static':
            shared_name = f'{name}{debug_postfix}'
            expected = (f'{shared_name}.lib' if msvc_import else import_reference[0].replace(
                'naming-reference-shared', shared_name))
            assert expected in installed, (expected, installed)
        advertised_postfix = postfix if library_type == 'static' else ''
        flag = f'-l{name}{advertised_postfix}{debug_postfix}'
        assert flag in script.split(), (flag, script)
        metadata = (build_dir / f'meson-private/lib{name}.pc').read_text()
        assert flag in metadata.split(), (flag, metadata)
    return msvc_import


def main():
    parser = argparse.ArgumentParser(description='Check Meson consumers and library filenames.')
    parser.add_argument('--cross-file', type=Path)
    parser.add_argument('--configure-only',
                        action='store_true',
                        help='Check filenames without building or running consumers')
    args = parser.parse_args()
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
        cases = [
            ('static', 'static', 'release', 'auto', []),
            ('shared', 'shared', 'release', 'auto', []),
            ('both', 'both', 'release', 'auto', []),
            ('static-custom', 'static', 'debug', '-custom', []),
            ('both-custom', 'both', 'debug', '-custom', []),
            ('static-empty', 'static', 'debug', '', []),
            ('both-empty', 'both', 'release', '', []),
            ('both-empty-debug', 'both', 'debug', '', []),
            ('both-dll', 'both', 'release', '.dll', []),
        ]
        meson_version = subprocess.check_output(['meson', '--version'], text=True).strip()
        if tuple(int(part) for part in meson_version.split('.')[:2]) >= (1, 10):
            cases.extend([
                ('static-platform', 'static', 'release', 'auto', ['-Dnamingscheme=platform']),
                ('both-platform', 'both', 'release', 'auto', ['-Dnamingscheme=platform']),
                ('both-empty-platform', 'both', 'release', '', ['-Dnamingscheme=platform']),
            ])
        msvc_import = False
        for case, library_type, build_type, postfix, options in cases:
            build_dir = work_dir / case
            command = [
                'meson',
                'setup',
                str(build_dir),
                str(source_dir),
                *(['--vsenv'] if os.name == 'nt' and args.cross_file is None else []),
                *(['--cross-file', str(args.cross_file.resolve())] if args.cross_file else []),
                f'--buildtype={build_type}',
                '--wrap-mode=forcefallback',
                f'-Dpcre2:default_library={library_type}',
                f'-Dpcre2:pcre2_static_library_postfix={postfix}',
                '-Dpcre2:pcre2_build_pcre2_16=true',
                '-Dpcre2:pcre2_build_pcre2_32=true',
                '-Dpcre2:pcre2_build_tests=false',
                '-Dpcre2:pcre2_build_pcre2grep=false',
                '-Dpcre2:pcre2_support_libbz2=disabled',
                '-Dpcre2:pcre2_support_libz=disabled',
                '-Dpcre2:pcre2_support_libreadline=disabled',
                *options,
            ]
            collision = postfix == '' and library_type == 'both' and msvc_import
            if collision:
                result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                assert result.returncode != 0, result.stdout
                assert 'Static and shared libraries are being built with colliding suffix "lib"' in result.stdout, result.stdout
                assert 'pcre2_static_library_postfix' in result.stdout, result.stdout
                continue
            subprocess.run(command, check=True)
            msvc_import = check_library_names(build_dir, library_type, build_type, postfix)
            if not args.configure_only:
                subprocess.run(['meson', 'compile', '-C', str(build_dir), 'consumer'], check=True)
                subprocess.run([
                    'meson',
                    'test',
                    '-C',
                    str(build_dir),
                    '--no-rebuild',
                    '--print-errorlogs',
                ],
                               check=True)

    print('Meson library filename and subproject checks passed')


if __name__ == '__main__':
    main()
