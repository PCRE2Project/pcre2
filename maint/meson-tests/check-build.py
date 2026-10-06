#!/usr/bin/env python3

import bz2
import gzip
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SOURCE = Path(__file__).resolve().parents[2]


class BuildTests(unittest.TestCase):

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='pcre2-meson-build-')
        self.addCleanup(self.temporary.cleanup)
        self.work = Path(self.temporary.name)

    def run_command(self, *command, cwd=None, env=None, succeeds=True):
        result = subprocess.run(
            [str(arg) for arg in command],
            cwd=cwd,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        if succeeds:
            self.assertEqual(result.returncode, 0, result.stdout)
        else:
            self.assertNotEqual(result.returncode, 0, result.stdout)
        return result.stdout

    def setup_meson(self, build, *options, source=SOURCE, env=None):
        return self.run_command(
            'meson',
            'setup',
            build,
            source,
            '-Dpcre2_support_libbz2=disabled',
            '-Dpcre2_support_libz=disabled',
            '-Dpcre2_support_libreadline=disabled',
            *options,
            env=env,
        )

    def check_pkgconfig_version(self, directory, version, prerelease):
        env = os.environ.copy()
        env.update(PKG_CONFIG_PATH='', PKG_CONFIG_LIBDIR=str(directory))
        env.pop('PKG_CONFIG_DISABLE_UNINSTALLED', None)
        releases = ('-DEV', '-RC1', '-RC2', '')
        expected = version + prerelease.replace('-', '~')
        for interface in ('8', '16', '32', 'posix'):
            package = 'libpcre2-' + interface
            self.assertEqual(self.run_command('pkg-config', '--modversion', package, env=env).strip(), expected)
            self.run_command('pkg-config', '--exact-version=' + expected, package, env=env)
            self.run_command('pkg-config', '--max-version=' + version, package, env=env)
            for other in releases:
                self.run_command(
                    'pkg-config',
                    '--atleast-version=' + version + other.replace('-', '~'),
                    package,
                    env=env,
                    succeeds=releases.index(prerelease) >= releases.index(other),
                )

    @unittest.skipUnless(
        sys.platform.startswith('linux') and all(shutil.which(tool) for tool in ('gcc', 'ar', 'pkg-config')),
        'Dependency fixtures use GCC and static archives on Linux')
    def test_optional_dependency_discovery(self):
        source = self.work / 'source'
        source.mkdir()
        for name in ('FindBZip2', 'FindReadline', 'FindEditline'):
            shutil.copytree(SOURCE / 'meson' / name, source / 'meson' / name)
        shutil.copy2(SOURCE / 'meson.options', source / 'meson.options')
        (source / 'meson.build').write_text('''
project('optional-dependency-fixtures', 'c', meson_version: '>=1.6.0')
cc = meson.get_compiler('c')
subdir('meson/FindBZip2')
zlib_dep = dependency('zlib', required: get_option('pcre2_support_libz'))
subdir('meson/FindReadline')
subdir('meson/FindEditline')
foreach name, dep : {'bzip2': bzip2_dep, 'zlib': zlib_dep, 'readline': readline_dep, 'libedit': editline_dep}
  if dep.found()
    consumer = executable(name, name + '.c', dependencies: dep,
      c_args: name == 'libedit' ? ['-DEDITLINE_HEADER="' + editline_header + '"'] : [])
    test(name, consumer)
  endif
endforeach
message('Editline header: ' + editline_header)
''')
        headers = {
            'readline/readline.h': 'char *readline(const char *);\n',
            'readline.h': 'char *readline(const char *);\n',
            'editline/readline.h': 'char *readline(const char *);\n',
            'edit/readline/readline.h': 'char *readline(const char *);\n',
            'bzlib.h': 'void *BZ2_bzopen(const char *, const char *);\n',
            'zlib.h': '#define ZLIB_VERSION "1.3"\nconst char *zlibVersion(void);\n',
        }
        prefix = self.work / 'dependencies'
        include = prefix / 'include'
        libraries = prefix / 'lib'
        libraries.mkdir(parents=True)
        for name, content in headers.items():
            path = include / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content)
        terminal_names = ('tinfo', 'curses', 'ncurses', 'ncursesw', 'termcap')
        implementations = {
            'readline': 'extern int fixture_terminal(void);\n'
            'char *readline(const char *s) { return fixture_terminal() == 42 ? "readline" : 0; }\n',
            'edit': 'char *readline(const char *s) { return "editline"; }\n',
            'bz2': 'void *BZ2_bzopen(const char *s, const char *m) { return (void *)s; }\n',
            'z': 'const char *zlibVersion(void) { return "1.3"; }\n',
        }
        implementations.update({name: 'int fixture_terminal(void) { return 42; }\n' for name in terminal_names})
        for name, content in implementations.items():
            code = self.work / (name + '.c')
            code.write_text(content)
            obj = self.work / (name + '.o')
            self.run_command('gcc', '-c', code, '-o', obj)
            self.run_command('ar', 'rcs', libraries / ('lib' + name + '.a'), obj)
        consumers = {
            'readline': '#include <readline/readline.h>\nint main(void) { return !readline(""); }\n',
            'libedit': '#include EDITLINE_HEADER\nint main(void) { return !readline(""); }\n',
            'bzip2': '#include <bzlib.h>\nint main(void) { return !BZ2_bzopen("fixture", "r"); }\n',
            'zlib': '#include <zlib.h>\nint main(void) { return zlibVersion()[0] != \'1\'; }\n',
        }
        for name, content in consumers.items():
            (source / (name + '.c')).write_text(content)
        pkgconfig = self.work / 'pkgconfig'
        pkgconfig.mkdir()
        for name, library in (('readline', 'readline'), ('libedit', 'edit'), ('bzip2', 'bz2'), ('zlib', 'z')):
            (pkgconfig / (name + '.pc')).write_text(f'Name: {name}\nDescription: Dependency fixture\nVersion: 1.3\n'
                                                    f'Libs: -L{libraries} -l{library}\n'
                                                    f'Cflags: -I{include}\n' +
                                                    ('Libs.private: -ltinfo\n' if name == 'readline' else ''))
        wrapper = self.work / 'compiler.py'
        wrapper.write_text('''
import json, os, subprocess, sys
from pathlib import Path
args = sys.argv[1:]
with open(os.environ['PCRE2_DEPENDENCY_LOG'], 'a') as log:
    log.write(json.dumps(args) + '\\n')
for name in json.loads(os.environ.get('PCRE2_BLOCK_LIBRARIES', '[]')):
    if any(arg == '-l' + name or Path(arg).name.startswith('lib' + name + '.') for arg in args):
        sys.exit(1)
for arg in args:
    if arg.endswith('.c') and Path(arg).is_file():
        code = Path(arg).read_text()
        for header in json.loads(os.environ.get('PCRE2_BLOCK_HEADERS', '[]')):
            if '#include <' + header + '>' in code:
                sys.exit(1)
sys.exit(subprocess.run([os.environ['PCRE2_REAL_CC'], *args]).returncode)
''')
        feature_names = {'bzip2': 'libbz2', 'zlib': 'libz', 'readline': 'libreadline', 'libedit': 'libedit'}
        all_blocked = [
            'readline', 'edit', 'z', 'bz2', 'bzip2', 'libbz2', 'libbzip2', 'bz2d', 'bzip2d', 'libbz2d', 'libbzip2d'
        ]
        cases = [
            ('native', dict.fromkeys(feature_names, 'enabled'), [], [], set(feature_names), False),
            ('pkgconfig-static', dict.fromkeys(feature_names, 'enabled'), [], [], set(feature_names), True),
            ('disabled', dict.fromkeys(feature_names, 'disabled'), [], [], set(), False),
            ('absent-auto', dict.fromkeys(feature_names, 'auto'), all_blocked, list(headers), set(), False),
        ]
        for index, terminal in enumerate(terminal_names):
            cases.append(('terminal-' + terminal, {
                'readline': 'enabled'
            }, list(terminal_names[:index]), [], {'readline'}, False))
        for header in ('readline.h', 'editline/readline.h', 'edit/readline/readline.h'):
            blocked_headers = [
                name for name in headers
                if name in ('readline.h', 'editline/readline.h', 'edit/readline/readline.h') and name != header
            ]
            cases.append(('header-' + header.replace('/', '-'), {
                'libedit': 'enabled'
            }, [], blocked_headers, {'libedit'}, False))
        for name in feature_names:
            cases.append(('required-' + name, {name: 'enabled'}, all_blocked, list(headers), None, False))
        cases.extend([
            ('unusable-readline-auto', {
                'readline': 'auto'
            }, list(terminal_names), [], set(), False),
            ('unusable-readline-enabled', {
                'readline': 'enabled'
            }, list(terminal_names), [], None, False),
            ('missing-bzip2-header', {
                'bzip2': 'enabled'
            }, [], ['bzlib.h'], None, False),
        ])
        for label, features, blocked_libraries, blocked_headers, expected, metadata in cases:
            with self.subTest(case=label):
                log = self.work / (label + '.log')
                native = self.work / (label + '.ini')
                native.write_text(
                    '[binaries]\n'
                    f'c = [{sys.executable!r}, {str(wrapper)!r}]\n'
                    f'pkg-config = {shutil.which("pkg-config") if metadata else shutil.which("false")!r}\n'
                    f'cmake = {shutil.which("false")!r}\n'
                    '[built-in options]\n'
                    f'c_args = [{("-I" + str(include))!r}]\n'
                    f'c_link_args = [{("-L" + str(libraries))!r}]\n'
                    f'prefer_static = {str(metadata).lower()}\n')
                env = os.environ.copy()
                env.update(
                    PKG_CONFIG_PATH='',
                    PKG_CONFIG_LIBDIR=str(pkgconfig),
                    PCRE2_REAL_CC=shutil.which('gcc'),
                    PCRE2_DEPENDENCY_LOG=str(log),
                    PCRE2_BLOCK_LIBRARIES=json.dumps(blocked_libraries),
                    PCRE2_BLOCK_HEADERS=json.dumps(blocked_headers),
                )
                options = [
                    '-Dpcre2_support_' + option + '=' + features.get(name, 'disabled')
                    for name, option in feature_names.items()
                ]
                build = self.work / label
                output = self.run_command(
                    'meson',
                    'setup',
                    build,
                    source,
                    '--native-file',
                    native,
                    *options,
                    env=env,
                    succeeds=expected is not None,
                )
                if expected is None:
                    self.assertIn('ERROR:', output)
                    continue
                targets = json.loads(self.run_command('meson', 'introspect', '--targets', build))
                self.assertEqual({target['name'] for target in targets}, expected)
                self.run_command('meson', 'test', '-C', build, '--print-errorlogs', env=env)
                if label.startswith('header-'):
                    expected_header = next(name
                                           for name in ('readline.h', 'editline/readline.h', 'edit/readline/readline.h')
                                           if name not in blocked_headers)
                    self.assertIn('Editline header: ' + expected_header, output)
                if label == 'disabled':
                    commands = [json.loads(line) for line in log.read_text().splitlines()]
                    self.assertFalse(any(arg.startswith('-l') for command in commands for arg in command))

    @unittest.skipUnless(sys.platform.startswith('linux'), 'Optional dependency integration uses Linux executables')
    def test_optional_dependency_programs(self):
        native = self.work / 'native.ini'
        native.write_text(f'[binaries]\npkg-config = {shutil.which("false")!r}\ncmake = {shutil.which("false")!r}\n')
        for terminal in ('libreadline', 'libedit'):
            with self.subTest(terminal=terminal):
                build = self.work / terminal
                self.setup_meson(
                    build,
                    '--native-file',
                    native,
                    '-Dpcre2_support_libbz2=auto',
                    '-Dpcre2_support_libz=auto',
                    '-Dpcre2_support_libreadline=disabled',
                    '-Dpcre2_support_libedit=disabled',
                    '-Dpcre2_support_' + terminal + '=auto',
                )
                config = (build / 'config.h').read_text()
                for name in ('SUPPORT_LIBBZ2', 'SUPPORT_LIBZ', 'SUPPORT_' + terminal.upper()):
                    if not re.search(r'^#define ' + name + r'\b', config, re.M):
                        self.skipTest('Integration requires development files for zlib, bzip2, readline, and libedit')
                self.run_command('meson', 'test', '-C', build, '--print-errorlogs')
                payload = b'needle\nother\n'
                for suffix, contents in (('gz', gzip.compress(payload)), ('bz2', bz2.compress(payload))):
                    compressed = self.work / ('input.' + suffix)
                    compressed.write_bytes(contents)
                    self.assertEqual(self.run_command(build / 'pcre2grep', 'needle', compressed), 'needle\n')

    def test_release_versions(self):
        source = self.work / 'source'
        (source / 'maint').mkdir(parents=True)
        for name in ('UpdateCommon.py', 'UpdateRelease.py'):
            shutil.copy2(SOURCE / 'maint' / name, source / 'maint' / name)
        for name in ('configure.ac', 'CMakeLists.txt', 'meson.build', 'MODULE.bazel', 'build.zig.zon'):
            shutil.copy2(SOURCE / name, source / name)
        versions = {
            'pcre2_major': '11',
            'pcre2_minor': '01',
            'pcre2_prerelease': '-RC1',
            'pcre2_date': '2027-01-02',
            'libpcre2_posix_version': '4:2:1',
            'libpcre2_8_version': '17:3:16',
            'libpcre2_16_version': '18:4:15',
            'libpcre2_32_version': '19:5:14',
        }
        configure = (source / 'configure.ac').read_text()
        for name, value in versions.items():
            configure, count = re.subn(
                rf'(m4_define\({name},\s*\[)[^]]*(\]\))',
                lambda match: match[1] + value + match[2],
                configure,
            )
            self.assertEqual(count, 1)
        (source / 'configure.ac').write_text(configure)
        original = (source / 'meson.build').read_text()
        layouts = (
            "{'posix': '0:0:0', '8': '0:0:0', '16': '0:0:0', '32': '0:0:0'}",
            "{\n  'posix' : '0:0:0',\n  '8': '0:0:0',\n  '16': '0:0:0',\n  '32': '0:0:0',\n}",
        )
        for layout in layouts:
            with self.subTest(layout=layout):
                (source / 'meson.build').write_text(
                    re.sub(
                        r'(?m)^libtool_versions\s*=\s*\{[^}]*\}',
                        'libtool_versions = ' + layout,
                        original,
                    ))
                for formatted in (False, True):
                    if formatted:
                        self.run_command(
                            'meson',
                            'format',
                            '-i',
                            '-c',
                            SOURCE / 'maint/formatting/meson.format',
                            source / 'meson.build',
                        )
                    self.run_command(sys.executable, 'maint/UpdateRelease.py', cwd=source)
                    updated = (source / 'meson.build').read_text()
                    self.assertIn("version: '11.01-RC1'", updated)
                    self.assertIn("pcre2_date = '2027-01-02'", updated)
                    for library in ('posix', '8', '16', '32'):
                        self.assertRegex(
                            updated,
                            rf"'{library}'\s*:\s*'{versions['libpcre2_' + library + '_version']}'",
                        )
                    self.run_command(sys.executable, 'maint/UpdateRelease.py', cwd=source)
                    self.assertEqual(updated, (source / 'meson.build').read_text())
        (source / 'meson.build').write_text(original.replace("'posix':", "'missing':"))
        output = self.run_command(sys.executable, 'maint/UpdateRelease.py', cwd=source, succeeds=False)
        self.assertIn('Expected exactly one posix entry', output)

    def test_stale_headers(self):
        source = self.work / 'source'
        source.mkdir()
        for entry in SOURCE.iterdir():
            if entry.is_file() and not entry.name.startswith('.'):
                shutil.copy2(entry, source / entry.name)
        for directory in ('src', 'meson', 'cmake', 'doc'):
            shutil.copytree(SOURCE / directory, source / directory)
        for header in ('config.h', 'pcre2.h'):
            shutil.copy2(source / 'src' / (header + '.generic'), source / 'src' / header)
        marker = source / 'src' / 'keep-me'
        marker.write_text('unrelated file\n')
        build = self.work / 'build'
        self.setup_meson(build, '-Dpcre2_support_unicode=false', source=source)
        for header in ('config.h', 'pcre2.h'):
            self.assertFalse((source / 'src' / header).exists())
        self.assertEqual(marker.read_text(), 'unrelated file\n')
        self.run_command('meson', 'compile', '-C', build, '-j', '2', 'pcre2test')
        executable = build / ('pcre2test.exe' if os.name == 'nt' else 'pcre2test')
        self.assertEqual(self.run_command(executable, '-C', 'unicode').strip(), '0')

    def test_clean_test_and_installed_contract(self):
        build = self.work / 'build'
        prefix = self.work / 'install'
        self.setup_meson(
            build,
            '--buildtype=release',
            f'--prefix={prefix}',
            '--libdir=lib',
            '-Ddefault_library=both',
            '-Ddefault_both_libraries=static',
            '-Dpcre2_build_pcre2_16=true',
            '-Dpcre2_build_pcre2_32=true',
            '-Dpcre2_support_jit=enabled',
        )
        tests = json.loads(self.run_command('meson', 'introspect', '--tests', build))
        self.assertIn('pcre2_grep_test', {test['name'] for test in tests})
        self.run_command('meson', 'test', '-C', build, '--print-errorlogs')
        self.run_command('meson', 'install', '-C', build)
        self.assertEqual(list(prefix.rglob('*.cmake')), [])
        if sys.platform.startswith('linux'):
            self.run_command(
                SOURCE / 'maint/RunManifestTest',
                prefix,
                SOURCE / 'maint/manifest-install-linux',
                'release',
                'meson',
                cwd=self.work,
            )
        relocated = self.work / 'relocated'
        prefix.rename(relocated)
        suffix = '.exe' if os.name == 'nt' else ''
        for name, args in (('pcre2grep', ['--version']), ('pcre2test', ['-C', 'version'])):
            program = relocated / 'bin' / (name + suffix)
            self.run_command(program, *args)
        absolute_prefix = self.work / 'absolute-prefix'
        libraries = self.work / 'libraries'
        headers = self.work / 'headers'
        self.run_command(
            'meson',
            'configure',
            build,
            f'--prefix={absolute_prefix}',
            f'--libdir={libraries}',
            f'--includedir={headers}',
        )
        self.run_command('meson', 'install', '-C', build)
        self.assertEqual(list(absolute_prefix.rglob('*.cmake')), [])
        self.assertEqual(list(libraries.rglob('*.cmake')), [])
        self.assertTrue((headers / 'pcre2.h').is_file())
        self.assertTrue((headers / 'pcre2posix.h').is_file())
        self.assertEqual(
            {path.name
             for path in (libraries / 'pkgconfig').glob('*.pc')},
            {'libpcre2-' + interface + '.pc'
             for interface in ('8', '16', '32', 'posix')},
        )
        if os.name != 'nt':
            flags = self.run_command(absolute_prefix / 'bin/pcre2-config', '--cflags', '--libs8')
            self.assertEqual(flags, f'-I{headers}\n-L{libraries} -lpcre2-8\n')

    def test_cmake_private_programs(self):
        build = self.work / 'build'
        prefix = self.work / 'install'
        self.run_command(
            'cmake',
            '-S',
            SOURCE,
            '-B',
            build,
            '-G',
            'Ninja',
            '-DCMAKE_BUILD_TYPE=Release',
            f'-DCMAKE_INSTALL_PREFIX={prefix}',
            '-DCMAKE_INSTALL_LIBDIR=lib',
            '-DBUILD_SHARED_LIBS=ON',
            '-DBUILD_STATIC_LIBS=ON',
            '-DPCRE2_BUILD_PCRE2_16=ON',
            '-DPCRE2_BUILD_PCRE2_32=ON',
            '-DPCRE2_SUPPORT_JIT=ON',
            '-DPCRE2_SUPPORT_LIBREADLINE=OFF',
            '-DPCRE2_SUPPORT_LIBBZ2=OFF',
            '-DPCRE2_SUPPORT_LIBZ=OFF',
        )
        self.run_command('cmake', '--build', build, '--parallel', '2')
        self.run_command('cmake', '--install', build)
        if sys.platform.startswith('linux'):
            self.run_command(
                SOURCE / 'maint/RunManifestTest',
                prefix,
                SOURCE / 'maint/manifest-install-linux',
                cwd=self.work,
            )
        suffix = '.exe' if os.name == 'nt' else ''
        for name, args in (('pcre2grep', ['--version']), ('pcre2test', ['-C', 'version'])):
            program = prefix / 'bin' / (name + suffix)
            self.run_command(program, *args)
            program.unlink()
        self.run_command(
            sys.executable,
            SOURCE / 'maint/cmake-tests/check-interface.py',
            'install',
            prefix,
            '--generator',
            'Ninja',
        )
        self.run_command(
            'cmake',
            '-S',
            SOURCE,
            '-B',
            build,
            '-DPCRE2_BUILD_PCRE2GREP=OFF',
            '-DPCRE2_BUILD_TESTS=OFF',
        )

    @unittest.skipUnless(sys.platform.startswith('linux') or os.name == 'nt', 'Fixtures use Linux or Windows modes')
    def test_install_manifests(self):
        windows = os.name == 'nt'
        if windows and not shutil.which('pwsh'):
            self.skipTest('PowerShell is required')
        exports = {
            'lib/cmake/pcre2/' + name
            for name in ('pcre2-config.cmake', 'pcre2-config-version.cmake', 'pcre2-targets.cmake',
                         'pcre2-targets-release.cmake')
        }
        archives = {'lib/libpcre2-' + width + '.la' for width in ('8', '16', '32', 'posix')}
        files = {'include/pcre2.h': 0o644}
        files.update({name: 0o644 for name in exports})
        files.update({name: 0o755 for name in archives})
        links = {}
        if not windows:
            files['lib/libpcre2-8.so.0.16.0'] = 0o644
            links = {
                'lib/libpcre2-8.so': 'libpcre2-8.so.0.16.0',
                'lib/libpcre2-8.so.0': 'libpcre2-8.so.0.16.0',
            }
        directories = ('', 'include', 'lib', 'lib/cmake', 'lib/cmake/pcre2')
        entries = {}
        for name in directories:
            if name or not windows:
                entries[name] = 'd----' if windows else 'drwxr-xr-x'
        for name, mode in files.items():
            entries[name] = '-a---' if windows else ('-rwxr-xr-x' if mode == 0o755 else '-rw-r--r--')
        entries.update({name: 'lrwxrwxrwx' for name in links})
        lines = []
        for name, mode in entries.items():
            path = 'install-dir' + ('/' + name if name else '')
            if windows:
                path = '.\\' + path.replace('/', '\\')
            lines.append(mode + ' ' + path + (' -> ' + links[name] if name in links else ''))
        lines.sort(key=lambda line: line.split(' ', 1)[1])
        manifests = self.work / 'manifests'
        manifests.mkdir()
        manifest = manifests / 'manifest-install'
        manifest.write_bytes(('\n'.join(lines) + '\n').encode())
        cases = [('autoconf', 'release'), ('cmake', 'release'), ('meson', 'release')]
        if not windows:
            cases.append(('cmake', 'relwithdebinfo'))
        for producer, buildtype in cases:
            with self.subTest(producer=producer, buildtype=buildtype):
                case = self.work / (producer + '-' + buildtype)
                prefix = case / ('staging/usr/local' if producer == 'autoconf' and not windows else 'install-dir')
                prefix.mkdir(parents=True)
                prefix.chmod(0o755)
                for directory in ('include', 'lib'):
                    (prefix / directory).mkdir(mode=0o755)
                for name, mode in files.items():
                    if (name in exports and producer != 'cmake') or (name in archives and producer != 'autoconf'):
                        continue
                    path = prefix / name.replace('targets-release', 'targets-' + buildtype)
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_text('fixture\n')
                    if name.endswith('.so.0.16.0') and producer != 'cmake':
                        mode = 0o755
                    path.chmod(mode)
                for name, target in links.items():
                    if name.endswith('.so') and producer != 'autoconf':
                        target = 'libpcre2-8.so.0'
                    (prefix / name).symlink_to(target)

                def check(succeeds=True):
                    if windows:
                        command = [
                            'pwsh', '-NoProfile', '-File', SOURCE / 'maint/RunManifestTest.ps1', 'install-dir',
                            manifest, producer
                        ]
                    else:
                        command = [SOURCE / 'maint/RunManifestTest', prefix, manifest, buildtype, producer]
                    return self.run_command(*command, cwd=case, succeeds=succeeds)

                check()
                header = prefix / 'include/pcre2.h'
                header.unlink()
                self.assertIn('Installed files differ', check(succeeds=False))
                header.write_text('fixture\n')
                header.chmod(0o644)
                extra = prefix / 'lib/unexpected.cmake'
                extra.write_text('fixture\n')
                self.assertIn('Installed files differ', check(succeeds=False))
                extra.unlink()
                if producer != 'autoconf':
                    extra = prefix / 'lib/libpcre2-8.la'
                    extra.write_text('fixture\n')
                    self.assertIn('Installed files differ', check(succeeds=False))
                    extra.unlink()
                if producer == 'cmake':
                    missing = prefix / 'lib/cmake/pcre2/pcre2-config.cmake'
                    missing.unlink()
                    self.assertIn('Installed files differ', check(succeeds=False))
                    missing.write_text('fixture\n')
                    missing.chmod(0o644)
                else:
                    extra = prefix / 'lib/cmake/pcre2/pcre2-config.cmake'
                    extra.parent.mkdir(parents=True)
                    extra.write_text('fixture\n')
                    self.assertIn('Installed files differ', check(succeeds=False))
                    extra.unlink()
                    extra.parent.rmdir()
                    extra.parent.parent.rmdir()
                check()
                if not windows:
                    link = prefix / 'lib/libpcre2-8.so'
                    link.unlink()
                    link.symlink_to('libpcre2-8.so.999')
                    self.assertIn('Installed files differ', check(succeeds=False))
                else:
                    self.run_command(
                        'pwsh',
                        '-NoProfile',
                        '-File',
                        SOURCE / 'maint/RunManifestTest.ps1',
                        'install-dir',
                        manifest,
                        'unknown',
                        cwd=case,
                        succeeds=False,
                    )
        if not windows:
            self.run_command(
                SOURCE / 'maint/RunManifestTest',
                prefix,
                manifest,
                'release',
                'unknown',
                cwd=self.work,
                succeeds=False,
            )
            tarball = self.work / 'tarball-dir'
            tarball.mkdir()
            tarball.chmod(0o755)
            (tarball / 'source').write_text('fixture\n')
            (tarball / 'source').chmod(0o644)
            manifest.write_text('drwxr-xr-x tarball-dir\n-rw-r--r-- tarball-dir/source\n')
            self.run_command(SOURCE / 'maint/RunManifestTest', tarball, manifest, cwd=self.work)

    @unittest.skipUnless(
        sys.platform.startswith('linux')
        and all(shutil.which(tool) for tool in ('autoconf', 'autoheader', 'automake', 'aclocal', 'libtoolize', 'make')),
        'Autoconf installation check requires the Linux Autotools toolchain')
    def test_autoconf_install_manifest(self):
        source = self.work / 'source'
        source.mkdir()
        for entry in SOURCE.iterdir():
            if entry.is_file() and not entry.name.startswith('.'):
                shutil.copy2(entry, source / entry.name)
        for directory in ('src', 'm4', 'doc', 'deps'):
            shutil.copytree(SOURCE / directory, source / directory)
        self.run_command('sh', 'autogen.sh', cwd=source)
        build = self.work / 'build'
        build.mkdir()
        self.run_command(
            source / 'configure',
            '--prefix=/usr/local',
            '--enable-pcre2-16',
            '--enable-pcre2-32',
            '--enable-jit',
            '--disable-pcre2grep-libz',
            '--disable-pcre2grep-libbz2',
            cwd=build,
        )
        self.run_command('make', '-j2', cwd=build)
        staging = self.work / 'staging'
        self.run_command('make', 'install', f'DESTDIR={staging}', cwd=build)
        self.run_command(
            SOURCE / 'maint/RunManifestTest',
            staging / 'usr/local',
            SOURCE / 'maint/manifest-install-linux',
            'release',
            'autoconf',
            cwd=self.work,
        )

    @unittest.skipUnless(sys.platform.startswith('linux') and shutil.which('gcc'), 'Probe fixture uses GCC on Linux')
    def test_version_script_decisions(self):
        source = self.work / 'source'
        source.mkdir()
        shutil.copytree(SOURCE / 'meson/PCRE2CheckVscript', source / 'PCRE2CheckVscript')
        wrapper = self.work / 'compiler.py'
        wrapper.write_text('''
import os
import subprocess
import sys
from pathlib import Path

mode = sys.argv[1]
args = sys.argv[2:]
for index, arg in enumerate(args):
    if not arg.startswith(('-Wl,--version-script,', '-Wl,-M,')):
        continue
    _, flag, path = arg.split(',', 2)
    if mode == 'unsupported' or (mode == 'sun' and flag == '--version-script'):
        sys.exit(1)
    if mode == 'broken' and Path(path).stem.endswith('-broken'):
        del args[index]
        break
    if mode == 'wildcard' and Path(path).stem.endswith('-no-star'):
        sys.exit(1)
    args[index] = '-Wl,--version-script,' + path
sys.exit(subprocess.run([os.environ['PCRE2_REAL_CC'], *args]).returncode)
''')
        cases = (
            ('gnu', True, '--version-script', True),
            ('sun', True, '-M', True),
            ('broken', False, '', False),
            ('wildcard', True, '--version-script', False),
            ('unsupported', False, '', False),
        )
        for mode, supported, flag, no_star in cases:
            with self.subTest(mode=mode):
                (source / 'meson.build').write_text(f'''
project('version-script-probes', 'c')
cc = meson.get_compiler('c')
msvc_like = false
subdir('PCRE2CheckVscript')
assert(have_vscript == {str(supported).lower()}, 'version script support')
assert(vscript_flag == '{flag}', 'version script flag')
assert(have_vscript_no_star == {str(no_star).lower()}, 'version script wildcard')
''')
                env = os.environ.copy()
                env['CC'] = ' '.join(shlex.quote(arg) for arg in (sys.executable, str(wrapper), mode))
                env['PCRE2_REAL_CC'] = shutil.which('gcc')
                self.run_command('meson', 'setup', self.work / mode, source, env=env)

    @unittest.skipUnless(
        os.name != 'nt' and all(
            shutil.which(tool)
            for tool in ('autoconf', 'autoheader', 'automake', 'aclocal', 'libtoolize', 'pkg-config')),
        'Autoconf version checks require the Autotools toolchain and pkg-config')
    def test_autoconf_pkgconfig_versions(self):
        source = self.work / 'source'
        source.mkdir()
        for entry in SOURCE.iterdir():
            if entry.is_file() and not entry.name.startswith('.'):
                shutil.copy2(entry, source / entry.name)
        for directory in ('src', 'm4'):
            shutil.copytree(SOURCE / directory, source / directory)
        original = (source / 'configure.ac').read_text()
        major = re.search(r'm4_define\(pcre2_major,\s*\[([0-9]+)\]\)', original)[1]
        minor = re.search(r'm4_define\(pcre2_minor,\s*\[([0-9]+)\]\)', original)[1]
        version = major + '.' + minor
        for prerelease in ('-DEV', '-RC1', '-RC2', ''):
            with self.subTest(prerelease=prerelease):
                (source / 'configure.ac').write_text(
                    re.sub(
                        r'(m4_define\(pcre2_prerelease,\s*\[)[^]]*',
                        lambda match: match[1] + prerelease,
                        original,
                    ))
                self.run_command('sh', 'autogen.sh', cwd=source)
                build = self.work / (prerelease or 'final')
                build.mkdir()
                self.run_command(source / 'configure', '--disable-pcre2grep', '--disable-pcre2test', cwd=build)
                self.check_pkgconfig_version(build, version, prerelease)
                self.assertEqual(self.run_command(build / 'pcre2-config', '--version').strip(), version + prerelease)

    @unittest.skipUnless(
        sys.platform.startswith('linux') and shutil.which('gcc'),
        'Compiler probe comparisons use GNU-compatible compilers on Linux')
    def test_configuration_checks(self):
        source = self.work / 'source'
        source.mkdir()
        for directory in ('cmake', 'meson'):
            shutil.copytree(SOURCE / directory, source / directory)
        shutil.copy2(SOURCE / 'meson.options', source / 'meson.options')
        marker = '# User-configurable options'
        meson_checks = (SOURCE / 'meson.build').read_text().split(marker)[0]
        cmake_checks = (SOURCE / 'CMakeLists.txt').read_text().split(marker)[0]
        names = re.findall(r'#cmakedefine (HAVE_\w+)', (SOURCE / 'src/config-cmake.h.in').read_text())
        names.append('HAVE_VISIBILITY')
        for system in ('meson', 'cmake'):
            template = ''.join('#mesondefine ' + name + '\n' if system == 'meson' else '#cmakedefine ' + name + ' 1\n'
                               for name in names)
            (source / (system + '.h.in')).write_text(template + '#define PCRE2_EXPORT @PCRE2_EXPORT@\n')
        for compiler in ('gcc', 'clang'):
            if not shutil.which(compiler):
                continue
            for mode in ('default', 'strict-c11', 'msvc-like'):
                with self.subTest(compiler=compiler, mode=mode):
                    meson_text = meson_checks
                    cmake_text = cmake_checks
                    if mode == 'msvc-like':
                        meson_text = meson_text.replace('# Configuration checks\n',
                                                        '# Configuration checks\nmsvc_like = true\n')
                        cmake_text = cmake_text.replace('# Configuration checks\n',
                                                        '# Configuration checks\nset(MSVC TRUE)\n')
                    (source / 'meson.build'
                     ).write_text(meson_text + "\nconfig.set('HAVE_VISIBILITY', have_visibility)\n"
                                  "configure_file(input: 'meson.h.in', output: 'checks.h', configuration: config)\n")
                    (source / 'CMakeLists.txt').write_text(cmake_text + '\nconfigure_file(cmake.h.in checks.h @ONLY)\n')
                    meson_build = self.work / ('meson-' + compiler + '-' + mode)
                    cmake_build = self.work / ('cmake-' + compiler + '-' + mode)
                    env = os.environ.copy()
                    env.update(CC=compiler, CFLAGS='-Wall -Wextra')
                    self.setup_meson(meson_build,
                                     *(['-Dc_std=c11'] if mode == 'strict-c11' else []),
                                     source=source,
                                     env=env)
                    self.run_command('cmake',
                                     '-S',
                                     source,
                                     '-B',
                                     cmake_build,
                                     '-G',
                                     'Ninja',
                                     '-DCMAKE_C_COMPILER=' + compiler,
                                     '-DCMAKE_C_EXTENSIONS=' + ('OFF' if mode == 'strict-c11' else 'ON'),
                                     env=env)
                    definitions = []
                    for build in (meson_build, cmake_build):
                        # Meson's feature defines are valueless; CMake's have value 1.
                        definitions.append({
                            name: value if name == 'PCRE2_EXPORT' else '1'
                            for name, value in re.findall(r'^#define[ \t]+(\w+)[ \t]*(.*)$', (
                                build / 'checks.h').read_text(), re.MULTILINE)
                        })
                    self.assertEqual(*definitions)
                    self.assertEqual(definitions[0]['HAVE_BUILTIN_MUL_OVERFLOW'], '1')
                    self.assertEqual(definitions[0]['HAVE_BUILTIN_UNREACHABLE'], '1')
                    self.assertEqual('HAVE_VISIBILITY' in definitions[0], mode != 'msvc-like')
                    self.assertNotIn('HAVE_MKOSTEMP:', (cmake_build / 'CMakeCache.txt').read_text())

    @unittest.skipUnless(sys.platform.startswith('linux'), 'Configuration comparisons use Linux builds')
    def test_build_configuration(self):
        cases = [
            ({
                'pcre2_build_pcre2_8': False,
                'pcre2_ebcdic': True
            }, 'At least one'),
            ({
                'pcre2_build_pcre2_8': False,
                'pcre2_build_pcre2_16': True,
                'pcre2_fuzz_support': True,
                'pcre2_ebcdic': True
            }, 'Fuzzer support requires'),
            ({
                'pcre2_diff_fuzz_support': True,
                'pcre2_ebcdic': True
            }, 'PCRE2_FUZZ_SUPPORT'),
            ({
                'pcre2_fuzz_support': True,
                'pcre2_diff_fuzz_support': True
            }, 'PCRE2_SUPPORT_JIT'),
            ({
                'pcre2_ebcdic': True
            }, 'Support for EBCDIC and Unicode'),
            ({
                'pcre2_ebcdic': True,
                'pcre2_support_unicode': False,
                'pcre2_build_pcre2_16': True
            }, 'EBCDIC support is available only'),
            ({
                'pcre2_build_pcre2_8': False,
                'pcre2_build_pcre2_16': True
            }, None),
        ]
        for index, (options, error) in enumerate(cases):
            with self.subTest(options=options):
                for system in ('meson', 'cmake'):
                    build = self.work / (system + '-' + str(index))
                    if system == 'meson':
                        command = [
                            'meson', 'setup', build, SOURCE, '-Dpcre2_support_libreadline=disabled',
                            '-Dpcre2_support_libedit=disabled', '-Dpcre2_support_libz=disabled',
                            '-Dpcre2_support_libbz2=disabled'
                        ]
                        command += ['-D' + name + '=' + str(value).lower() for name, value in options.items()]
                    else:
                        command = [
                            'cmake', '-S', SOURCE, '-B', build, '-G', 'Ninja', '-DPCRE2_SUPPORT_LIBREADLINE=OFF',
                            '-DPCRE2_SUPPORT_LIBEDIT=OFF', '-DPCRE2_SUPPORT_LIBZ=OFF', '-DPCRE2_SUPPORT_LIBBZ2=OFF'
                        ]
                        command += [
                            '-D' + name.upper() + '=' + ('ON' if value else 'OFF') for name, value in options.items()
                        ]
                    output = self.run_command(*command, succeeds=error is None)
                    if error is not None:
                        self.assertIn(error, output)
                    elif system == 'meson':
                        targets = json.loads(self.run_command('meson', 'introspect', '--targets', build))
                        self.assertNotIn('pcre2grep', {target['name'] for target in targets})
                        self.assertIn('disabling pcre2grep', output)
                    else:
                        self.assertIn('must be enabled for the pcre2grep program', output)

    @unittest.skipUnless(
        sys.platform.startswith('linux') and shutil.which('gcc') and shutil.which('nm'),
        'Visibility checks use GNU-compatible compilers and nm on Linux')
    def test_symbol_visibility(self):
        for compiler in ('gcc', 'clang'):
            if not shutil.which(compiler):
                continue
            with self.subTest(compiler=compiler):
                build = self.work / compiler
                env = os.environ.copy()
                env['CC'] = compiler
                self.setup_meson(build,
                                 '-Ddefault_library=both',
                                 '-Dpcre2_symvers=false',
                                 '-Dpcre2_build_tests=false',
                                 '-Dpcre2_build_pcre2grep=false',
                                 env=env)
                commands = [
                    entry for entry in json.loads((build / 'compile_commands.json').read_text())
                    if entry['file'].endswith('/pcre2_compile.c')
                ]
                self.assertEqual(len(commands), 2)
                for entry in commands:
                    self.assertIn('-fvisibility=hidden', shlex.split(entry['command']))
                self.run_command('meson', 'compile', '-C', build, '-j', '2')
                symbols = self.run_command('nm', '-D', '--defined-only', build / 'libpcre2-8.so')
                self.assertRegex(symbols, r'\bpcre2_compile_8\b')
                self.assertNotIn('_pcre2_', symbols)
                self.assertIn('_pcre2_', self.run_command('nm', '--defined-only', build / 'libpcre2-8.a'))
                posix_symbols = self.run_command('nm', '-D', '--defined-only', build / 'libpcre2-posix.so')
                self.assertRegex(posix_symbols, r'\bpcre2_regcomp\b')

    @unittest.skipUnless(os.name != 'nt' and shutil.which('pkg-config'), 'Version checks use pkg-config and a shell')
    def test_cmake_meson_pkgconfig_versions(self):
        source = self.work / 'source'
        source.mkdir()
        for entry in SOURCE.iterdir():
            if entry.is_file() and not entry.name.startswith('.'):
                shutil.copy2(entry, source / entry.name)
        for directory in ('src', 'cmake', 'meson', 'doc'):
            shutil.copytree(SOURCE / directory, source / directory)
        originals = {name: (source / name).read_text() for name in ('CMakeLists.txt', 'meson.build')}
        version = re.search(r'set\(PCRE2_MAJOR "(\d+)"\)', originals['CMakeLists.txt'])[1] + '.' + re.search(
            r'set\(PCRE2_MINOR "(\d+)"\)', originals['CMakeLists.txt'])[1]
        for prerelease in ('-DEV', '-RC1', '-RC2', ''):
            with self.subTest(prerelease=prerelease):
                (source / 'CMakeLists.txt').write_text(
                    re.sub(
                        r'(set\(PCRE2_PRERELEASE ")[^"]*',
                        lambda match: match[1] + prerelease,
                        originals['CMakeLists.txt'],
                    ))
                (source / 'meson.build').write_text(
                    re.sub(
                        r"(\bversion: ')[^']*",
                        lambda match: match[1] + version + prerelease,
                        originals['meson.build'],
                    ))
                meson_build = self.work / ('meson' + (prerelease or '-final'))
                cmake_build = self.work / ('cmake' + (prerelease or '-final'))
                self.setup_meson(
                    meson_build,
                    '-Dpcre2_build_pcre2_16=true',
                    '-Dpcre2_build_pcre2_32=true',
                    '-Dpcre2_build_pcre2grep=false',
                    '-Dpcre2_build_tests=false',
                    source=source,
                )
                project_version = json.loads(self.run_command('meson', 'introspect', '--projectinfo',
                                                              meson_build))['version']
                self.assertEqual(project_version, version + prerelease)
                header = (meson_build / 'pcre2.h').read_text()
                self.assertRegex(header, r'(?m)^#define PCRE2_MINOR[ \t]+' + version.split('.')[1] + '$')
                self.assertRegex(header, r'(?m)^#define PCRE2_PRERELEASE[ \t]*' + re.escape(prerelease) + '$')
                self.run_command(
                    'cmake',
                    '-S',
                    source,
                    '-B',
                    cmake_build,
                    '-G',
                    'Ninja',
                    '-DCMAKE_BUILD_TYPE=Release',
                    '-DPCRE2_BUILD_PCRE2_16=ON',
                    '-DPCRE2_BUILD_PCRE2_32=ON',
                    '-DPCRE2_BUILD_PCRE2GREP=OFF',
                    '-DPCRE2_BUILD_TESTS=OFF',
                    '-DPCRE2_SUPPORT_LIBREADLINE=OFF',
                    '-DPCRE2_SUPPORT_LIBBZ2=OFF',
                    '-DPCRE2_SUPPORT_LIBZ=OFF',
                )
                self.check_pkgconfig_version(cmake_build, version, prerelease)
                self.check_pkgconfig_version(meson_build / 'meson-private', version, prerelease)
                self.check_pkgconfig_version(meson_build / 'meson-uninstalled', version, prerelease)
                for build in (meson_build, cmake_build):
                    self.assertEqual(self.run_command('sh', build / 'pcre2-config', '--version').strip(), version)

    @unittest.skipUnless(
        sys.platform.startswith('linux') and shutil.which('pkg-config'),
        'Pkg-config consumer fixtures use Linux linker flags')
    def test_pkgconfig_consumers(self):
        packages = ['libpcre2-' + interface for interface in ('8', '16', '32', 'posix')]
        for library_type in ('shared', 'static', 'both'):
            for jit in (False, True):
                with self.subTest(library_type=library_type, jit=jit):
                    directory = self.work / (library_type + ('-jit' if jit else '-nojit'))
                    build = directory / 'build'
                    prefix = directory / 'install with spaces'
                    libdir = prefix / 'lib'
                    includedir = prefix / 'include'
                    absolute_dirs = library_type == 'both' and not jit
                    if absolute_dirs:
                        libdir = directory / 'absolute libraries'
                        includedir = directory / 'absolute headers'
                    self.setup_meson(
                        build,
                        '--buildtype=debug',
                        f'--prefix={prefix}',
                        f'--libdir={libdir if absolute_dirs else "lib"}',
                        f'--includedir={includedir if absolute_dirs else "include"}',
                        f'-Ddefault_library={library_type}',
                        '-Dpcre2_build_pcre2_16=true',
                        '-Dpcre2_build_pcre2_32=true',
                        '-Dpcre2_build_pcre2grep=false',
                        '-Dpcre2_build_tests=false',
                        f'-Dpcre2_support_jit={"enabled" if jit else "disabled"}',
                    )
                    self.run_command('meson', 'compile', '-C', build, '-j', '2')
                    self.run_command('meson', 'install', '-C', build)
                    version = json.loads(self.run_command('meson', 'introspect', '--projectinfo', build))['version']
                    self.assertRegex(version, r'^\d+\.\d+(?:-(?:DEV|RC\d+))?$')
                    version = version.replace('-', '~')
                    dependencies = json.loads(self.run_command('meson', 'introspect', '--dependencies', build))
                    thread_flags = next(dep['link_args'] for dep in dependencies if dep['name'] == 'threads')
                    compiler = json.loads(self.run_command('meson', 'introspect', '--compilers',
                                                           build))['host']['c']['exelist']
                    for uninstalled in (False, True):
                        pcdir = build / 'meson-uninstalled' if uninstalled else libdir / 'pkgconfig'
                        suffix = '-uninstalled.pc' if uninstalled else '.pc'
                        self.assertEqual(sorted(path.name for path in pcdir.glob('*.pc')),
                                         sorted(package + suffix for package in packages))
                        env = os.environ.copy()
                        env.update(PKG_CONFIG_PATH='', PKG_CONFIG_LIBDIR=str(pcdir))
                        env.pop('PKG_CONFIG_DISABLE_UNINSTALLED', None)
                        env.pop('PKG_CONFIG_SYSROOT_DIR', None)
                        for package in packages:
                            self.assertEqual(
                                self.run_command('pkg-config', '--modversion', package, env=env).strip(), version)
                            text = (pcdir / (package + suffix)).read_text()
                            field = 'Requires' if library_type == 'static' else 'Requires.private'
                            if package == 'libpcre2-posix':
                                self.assertIn(field + ': libpcre2-8\n', text)
                            static_libs = shlex.split(
                                self.run_command('pkg-config', '--static', '--libs', package, env=env))
                            for flag in thread_flags:
                                self.assertEqual(flag in static_libs, jit, static_libs)
                            for static in ((False, True) if library_type == 'both' else (library_type == 'static', )):
                                with self.subTest(package=package, static=static, uninstalled=uninstalled):
                                    options = ['--static'] if static else []
                                    cflags = shlex.split(
                                        self.run_command('pkg-config', *options, '--cflags', package, env=env))
                                    libs = shlex.split(
                                        self.run_command('pkg-config', *options, '--libs', package, env=env))
                                    for flag in thread_flags:
                                        self.assertEqual(flag in libs, jit and static, libs)
                                    self.assertEqual('-DPCRE2_STATIC' in cflags, library_type == 'static', cflags)
                                    self.assertEqual('-DPCRE2POSIX_SHARED' in cflags, package == 'libpcre2-posix'
                                                     and library_type != 'static', cflags)
                                    if library_type == 'static':
                                        ordinary = shlex.split(
                                            self.run_command('pkg-config', '--libs', package, env=env))
                                        self.assertIn(
                                            '-lpcre2-8' if package == 'libpcre2-posix' else '-l' + package[3:],
                                            ordinary)
                                        for flag in thread_flags:
                                            self.assertEqual(flag in ordinary, jit, ordinary)
                                    consumer = directory / (package + '.c')
                                    if package == 'libpcre2-posix':
                                        consumer.write_text('''
#include <pcre2posix.h>
int main(void) {
  regex_t regex;
  if (regcomp(&regex, "a", 0)) return 1;
  int result = regexec(&regex, "a", 0, 0, 0);
  regfree(&regex);
  return result;
}
''')
                                    else:
                                        consumer.write_text(
                                            f'#define PCRE2_CODE_UNIT_WIDTH {package.rsplit("-", 1)[1]}\n' + '''
#include <pcre2.h>
int main(void) {
  int error, jit;
  PCRE2_SIZE offset;
  PCRE2_UCHAR pattern[] = { 'a', 0 };
  pcre2_code *code = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0, &error, &offset, 0);
  if (!code) return 1;
  pcre2_config(PCRE2_CONFIG_JIT, &jit);
  int result = jit ? pcre2_jit_compile(code, PCRE2_JIT_COMPLETE) : 0;
  pcre2_code_free(code);
  return result;
}
''')
                                    executable = directory / 'consumer'
                                    link_flags = ['-Wl,-Bstatic', *libs, '-Wl,-Bdynamic'] if static else libs
                                    runtime = env.copy()
                                    runtime['LD_LIBRARY_PATH'] = str(build if uninstalled else libdir)
                                    self.run_command(*compiler,
                                                     *cflags,
                                                     consumer,
                                                     '-o',
                                                     executable,
                                                     *link_flags,
                                                     env=runtime)
                                    self.run_command(executable, env=runtime)
                                    linked = self.run_command('ldd', executable, env=runtime)
                                    self.assertEqual('libpcre2-' in linked, not static, linked)

        subset = self.work / 'subset'
        self.setup_meson(
            subset,
            '-Dpcre2_build_pcre2_8=false',
            '-Dpcre2_build_pcre2_16=true',
            '-Dpcre2_build_pcre2grep=false',
            '-Dpcre2_build_tests=false',
        )
        installed = json.loads(self.run_command('meson', 'introspect', '--installed', subset))
        self.assertEqual([Path(path).name for path in installed if path.endswith('.pc')], ['libpcre2-16.pc'])
        self.assertEqual([path.name for path in (subset / 'meson-uninstalled').glob('*.pc')],
                         ['libpcre2-16-uninstalled.pc'])

    @unittest.skipIf(os.name == 'nt', 'The shell harness requires a Unix shell')
    def test_harness_executable_paths(self):
        build = self.work / 'build with spaces'
        self.setup_meson(build)
        self.run_command('meson', 'compile', '-C', build, '-j', '2', 'pcre2test', 'pcre2grep')
        path_directory = self.work / 'path'
        path_directory.mkdir()
        marker = self.work / 'path-searched'
        for name in ('pcre2test', 'pcre2grep'):
            program = path_directory / name
            program.write_text('#!/bin/sh\nprintf "%s\\n" "$0" >> "$PCRE2_TEST_PATH_MARKER"\nexit 97\n')
            program.chmod(0o755)
        base_env = os.environ.copy()
        base_env['PATH'] = str(path_directory) + os.pathsep + base_env.get('PATH', '')
        base_env['PCRE2_TEST_PATH_MARKER'] = str(marker)
        base_env.pop('MESON_EXE_WRAPPER', None)
        for name in ('pcre2test', 'pcre2grep'):
            base_env.pop(name, None)
        for script in ('RunTest', 'RunTest.py', 'RunGrepTest', 'RunGrepTest.py'):
            runner = [sys.executable if script.endswith('.py') else 'sh', SOURCE / script]
            names = ('pcre2test', 'pcre2grep') if script.startswith('RunGrep') else ('pcre2test', )
            for layout in ('bare', 'relative', 'absolute'):
                paths = {
                    name: name if layout == 'bare' else './' + name if layout == 'relative' else str(build / name)
                    for name in names
                }
                for argument_source in ('command-line', 'environment'):
                    with self.subTest(script=script, layout=layout, argument_source=argument_source):
                        env = base_env.copy()
                        args = ['--srcdir', SOURCE]
                        if argument_source == 'environment':
                            env.update(paths)
                        else:
                            for name, path in paths.items():
                                args += ['--' + name, path]
                        if not script.startswith('RunGrep'):
                            args += ['1']
                        self.run_command(*runner, *args, cwd=build, env=env)
                        self.assertFalse(marker.exists(), 'A harness searched PATH for a tested executable')

    @unittest.skipIf(os.name == 'nt', 'The shell harness requires a Unix shell')
    def test_shell_test_dependencies(self):
        native = self.work / 'native.ini'
        native.write_text("[binaries]\npython3 = 'nonexistent-pcre2-test-python'\n")
        build = self.work / 'build'
        self.setup_meson(build, '--native-file', native)
        tests = json.loads(self.run_command('meson', 'introspect', '--tests', build))
        self.assertIn('pcre2_grep_test', {test['name'] for test in tests})
        harness = next(test for test in tests if test['name'] == 'pcre2_test')
        self.assertTrue(any(argument.endswith('/RunTest') for argument in harness['cmd']), harness)
        self.run_command('meson', 'test', '-C', build, '--print-errorlogs')

    @unittest.skipUnless(sys.platform.startswith('linux') and shutil.which('gcc'), 'Cross fixtures use Linux/GCC')
    def test_cross_test_wrappers(self):
        wrapper = self.work / 'wrapper.py'
        wrapper.write_text('''
import json
import os
import sys

assert sys.argv[1] == '--marker'
with open(sys.argv[2], 'a') as log:
    log.write(json.dumps(sys.argv[3:]) + '\\n')
os.execv(sys.argv[3], sys.argv[3:])
''')
        for harness in ('python', 'shell'):
            for mode in ('wrapped', 'unavailable', 'direct'):
                with self.subTest(harness=harness, mode=mode):
                    directory = self.work / (harness + '-' + mode)
                    directory.mkdir()
                    log = directory / 'wrapper.log'
                    wrapper_command = [sys.executable, str(wrapper), '--marker', str(log)]
                    cross = directory / 'cross.ini'
                    # The native compiler simulates a cross build so this test needs no emulator.
                    cross.write_text("[binaries]\n"
                                     f"c = {shutil.which('gcc')!r}\n"
                                     "python3 = 'nonexistent-host-python'\n"
                                     "sh = 'nonexistent-host-shell'\n" +
                                     (f"exe_wrapper = {wrapper_command!r}\n" if mode != 'unavailable' else '') +
                                     "[properties]\n"
                                     f"needs_exe_wrapper = {str(mode != 'direct').lower()}\n"
                                     "[host_machine]\n"
                                     "system = 'linux'\n"
                                     "cpu_family = 'x86_64'\n"
                                     "cpu = 'x86_64'\n"
                                     "endian = 'little'\n")
                    native = directory / 'native.ini'
                    python = sys.executable if harness == 'python' else 'nonexistent-pcre2-test-python'
                    native.write_text(f"[binaries]\npython3 = {python!r}\nsh = {shutil.which('sh')!r}\n")
                    build = directory / 'build'
                    output = self.setup_meson(
                        build,
                        '--cross-file',
                        cross,
                        '--native-file',
                        native,
                        '-Dpcre2_support_jit=enabled',
                    )
                    self.assertIn('Skipping pcre2_grep_test', output)
                    tests = json.loads(self.run_command('meson', 'introspect', '--tests', build))
                    names = {test['name'] for test in tests}
                    self.assertNotIn('pcre2_grep_test', names)
                    self.assertEqual('pcre2_test' in names, mode != 'unavailable')
                    if mode != 'unavailable':
                        main_test = next(test for test in tests if test['name'] == 'pcre2_test')
                        script = '/RunTest.py' if harness == 'python' else '/RunTest'
                        self.assertTrue(any(arg.endswith(script) for arg in main_test['cmd']), main_test)
                    else:
                        self.assertIn('Skipping pcre2_test', output)
                    if log.exists():
                        log.unlink()
                    self.run_command('meson', 'test', '-C', build, '--print-errorlogs')
                    results = [
                        json.loads(line) for line in (build / 'meson-logs/testlog.json').read_text().splitlines()
                    ]
                    self.assertEqual(len(results), 2 if mode == 'unavailable' else 3)
                    self.assertEqual(
                        {result['result']
                         for result in results},
                        {'SKIP'} if mode == 'unavailable' else {'OK'},
                        results,
                    )
                    if mode == 'wrapped':
                        commands = [json.loads(line) for line in log.read_text().splitlines()]
                        self.assertEqual(
                            {Path(command[0]).name
                             for command in commands},
                            {'pcre2test', 'pcre2posix_test', 'pcre2_jit_test'},
                        )
                        self.assertTrue(any(command[1:3] == ['-C', 'pcre2-8'] for command in commands))
                        env = os.environ.copy()
                        env['MESON_EXE_WRAPPER'] = 'nonexistent-default-wrapper'
                        runner = [sys.executable, SOURCE /
                                  'RunTest.py'] if harness == 'python' else ['sh', SOURCE / 'RunTest']
                        self.run_command(
                            *runner,
                            '--sim',
                            ' '.join(wrapper_command),
                            '--srcdir',
                            SOURCE,
                            '--pcre2test',
                            'pcre2test',
                            '1',
                            cwd=build,
                            env=env,
                        )
                    else:
                        self.assertFalse(log.exists(), 'An unnecessary wrapper must not be used')


if __name__ == '__main__':
    unittest.main()
