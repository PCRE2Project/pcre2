#!/usr/bin/env python3

# Run pcre2grep tests. The assumption is that the PCRE2 tests check the library
# itself. What we are checking here is the file handling and options that are
# supported by pcre2grep. This script must be run in the build directory.

# CODING CONVENTIONS:
# * Put printf arguments in single, not double quotes to avoid unwanted
#     escaping.
# * Use \0 for binary zero in printf, not \x0, for the benefit of older
#     versions (and use octal for other special values).

import difflib
import os
import shutil
import subprocess
import sys
from pathlib import Path

sys.stdout.reconfigure(line_buffering=True)

# Set the C locale, so that sort(1) behaves predictably.

os.environ["LC_ALL"]="C"

# Remove any non-default colouring and aliases that the caller may have set.

for name in ("PCRE2GREP_COLOUR", "PCRE2GREP_COLOR", "PCREGREP_COLOUR", "PCREGREP_COLOR", "GREP_COLOR", "GREP_COLORS"):
  os.environ.pop(name, None)

# Remember the current (build) directory, set the program to be tested, and
# valgrind settings when requested.

builddir=Path.cwd()
pcre2grep=os.environ.get("pcre2grep", str(builddir / "pcre2grep"))
pcre2test=os.environ.get("pcre2test", str(builddir / "pcre2test"))
if os.name == "nt":
  if pcre2grep == str(builddir / "pcre2grep") and Path(f"{pcre2grep}.exe").is_file(): pcre2grep=f"{pcre2grep}.exe"
  if pcre2test == str(builddir / "pcre2test") and Path(f"{pcre2test}.exe").is_file(): pcre2test=f"{pcre2test}.exe"
pcre2grep_is_exe=pcre2grep.lower().endswith(".exe")

if not Path(pcre2grep).is_file() or not os.access(pcre2grep, os.X_OK): print(f"** {pcre2grep} does not exist or is not executable."); sys.exit(1)
if not Path(pcre2test).is_file() or not os.access(pcre2test, os.X_OK): print(f"** {pcre2test} does not exist or is not executable."); sys.exit(1)

valgrind=[]
for argument in sys.argv[1:]:
  if argument in ("valgrind", "-valgrind"): valgrind=["valgrind", "-q", "--leak-check=no", "--smc-check=all-non-file", "--error-exitcode=70"]
  else: print(f"RunGrepTest: Unknown argument {argument}"); sys.exit(1)

def invoke(*args, cwd=None, stdin=None, stdout=None, stderr=None, valgrind_args=True, vjs_args=False):
  command=[str(argument) for argument in args]
  if valgrind and valgrind_args: command=valgrind + (vjs if vjs_args else []) + command
  return subprocess.run(command, cwd=cwd, stdin=stdin, stdout=stdout, stderr=stderr, check=False).returncode

def output(arguments, cwd=builddir, stdin=None, stderr=None, transform=None, write_returncode=True, vjs_args=True):
  with open("testtrygrep", "ab") as stream:
    process=subprocess.run(valgrind + (vjs if vjs_args else []) + [str(argument) for argument in arguments], cwd=cwd, input=stdin if isinstance(stdin, bytes) else None, stdin=stdin if hasattr(stdin, "read") else None, stdout=subprocess.PIPE, stderr=stderr, check=False)
    data=process.stdout if transform is None else transform(process.stdout)
    stream.write(data)
    if write_returncode: stream.write(f"RC={process.returncode}\n".encode("latin-1"))
  return process.returncode

def output_both(arguments, **kwargs):
  return output(arguments, stderr=subprocess.STDOUT, **kwargs)

def compare(expected, actual):
  try:
    if Path(expected).read_bytes() == Path(actual).read_bytes(): return True
    print("".join(difflib.unified_diff(Path(expected).read_bytes().decode("latin-1").splitlines(keepends=True), Path(actual).read_bytes().decode("latin-1").splitlines(keepends=True), fromfile=str(expected), tofile=str(actual))), end="")
  except OSError: pass
  return False

pcre2grep_version=subprocess.run([pcre2grep, "-V"], stdout=subprocess.PIPE, check=False).stdout.decode("latin-1").strip()
print(f"Testing {pcre2grep_version}" + (" using valgrind" if valgrind else ""))

# Set up a suitable "diff" command for comparison. Some systems have a diff
# that lacks a -u option. Try to deal with this; better do the test for the -b
# option as well.

# Add a -a (always treat as text) if available. This was added in an attempt
# to get more detail from an Alpine Linux test failure on GitHub.

# Some tests involve NUL characters. It seems impossible to handle them easily
# in many operating systems. An earlier version of this script used sed to
# translate NUL into the string ZERO, but this didn't work on Solaris (aka
# SunOS), where the version of sed explicitly doesn't like them, and also MacOS
# (Darwin), OpenBSD, FreeBSD, NetBSD, and some Linux distributions like Alpine,
# even when using GNU sed. A user suggested using tr instead, which
# necessitates translating to a single character. However, on (some versions
# of?) Solaris, the normal "tr" cannot handle binary zeros, but if
# /usr/xpg4/bin/tr is available, it can do so, so test for that.

# If this test is being run from "make check", $srcdir will be set. If not, set
# it to the current or parent directory, whichever one contains the test data.
# Subsequently, we run most of the pcre2grep tests in the source directory so
# that the file names in the output are always the same.

srcdir=Path(os.environ.get("srcdir", ""))
if not (srcdir / "testdata").is_dir():
  if Path("testdata").is_dir(): srcdir=Path(".")
  elif Path("../testdata").is_dir(): srcdir=Path("..")
  else: print("Cannot find the testdata directory"); sys.exit(1)

# Set up the path to the valgrind JIT suppressions

vjs=[]
if valgrind:
  with open(os.devnull, "wb") as null: jit=invoke(pcre2test, "-C", "jit", stdout=null, valgrind_args=False)
  if jit != 0: vjs=[f"--suppressions={(srcdir.resolve() / 'testdata' / 'valgrind-jit.supp')}" ]

# Check for the availability of UTF-8 support

with open(os.devnull, "wb") as null: utf8=invoke(pcre2test, "-C", "unicode", stdout=null, valgrind_args=False)

# Check default newline convention. If it does not include LF, force LF.

nl=subprocess.run([pcre2test, "-C", "newline"], stdout=subprocess.PIPE, check=False).stdout.decode("latin-1").strip()
if nl not in ("LF", "ANY", "ANYCRLF"):
  pcre2grep_args=[pcre2grep, "-N", "LF"]
  print("Default newline setting forced to LF")
else: pcre2grep_args=[pcre2grep]
if pcre2grep_is_exe: pcre2grep_args.append("--lf")

def supports(text):
  help_process=subprocess.run(valgrind + vjs + [*pcre2grep_args, "--help"], stdout=subprocess.PIPE, check=False)
  return subprocess.run(valgrind + vjs + [*pcre2grep_args, "-q", text], input=help_process.stdout, stdout=subprocess.DEVNULL, check=False).returncode == 0

# ------ Function to run and check a special pcre2grep arguments test -------

def checkspecial(arguments, expected):
  with open("testtrygrep", "ab") as stream: returncode=invoke(*pcre2grep_args, *arguments, stdout=stream, stderr=subprocess.STDOUT)
  if returncode != expected: print(f"** pcre2grep {' '.join(arguments)} failed - check testtrygrep"); sys.exit(1)

# ------ Normal tests ------

print("Testing pcre2grep main features")

def header(number):
  with open("testtrygrep", "wb" if number == 1 else "ab") as stream: stream.write(f"---------------------------- Test {number} {'-' * (33 - len(str(number)) if number in (100, 101, 108, 120) else 32 - len(str(number)) if number >= 100 or 38 <= number <= 54 or 64 <= number <= 68 else 31 - len(str(number)))}\n".encode("latin-1"))

header(1)
output([*pcre2grep_args, "PATTERN", "./testdata/grepinput"], cwd=srcdir)
header(2)
output([*pcre2grep_args, "^PATTERN", "./testdata/grepinput"], cwd=srcdir)
header(3)
output([*pcre2grep_args, "-in", "PATTERN", "./testdata/grepinput"], cwd=srcdir)
header(4)
output([*pcre2grep_args, "-ic", "PATTERN", "./testdata/grepinput"], cwd=srcdir)
header(5)
output([*pcre2grep_args, "-in", "PATTERN", "./testdata/grepinput", "./testdata/grepinputx"], cwd=srcdir)
header(6)
output([*pcre2grep_args, "-inh", "PATTERN", "./testdata/grepinput", "./testdata/grepinputx"], cwd=srcdir)
header(7)
output([*pcre2grep_args, "-il", "PATTERN", "./testdata/grepinput", "./testdata/grepinputx"], cwd=srcdir)
header(8)
output([*pcre2grep_args, "-l", "PATTERN", "./testdata/grepinput", "./testdata/grepinputx"], cwd=srcdir)
header(9)
output([*pcre2grep_args, "-q", "PATTERN", "./testdata/grepinput", "./testdata/grepinputx"], cwd=srcdir)
header(10)
output([*pcre2grep_args, "-q", "NEVER-PATTERN", "./testdata/grepinput", "./testdata/grepinputx"], cwd=srcdir)
header(11)
output([*pcre2grep_args, "-vn", "pattern", "./testdata/grepinputx"], cwd=srcdir)
header(12)
output([*pcre2grep_args, "-ix", "pattern", "./testdata/grepinputx"], cwd=srcdir)
header(13)
Path("testtemp1grep").write_bytes(b"seventeen\n")
output([*pcre2grep_args, "-f./testdata/greplist", "-f", builddir / "testtemp1grep", "./testdata/grepinputx"], cwd=srcdir)
header(14)
output([*pcre2grep_args, "-w", "pat", "./testdata/grepinput", "./testdata/grepinputx"], cwd=srcdir)
header(15)
output_both([*pcre2grep_args, "abc^*", "./testdata/grepinput"], cwd=srcdir)
header(16)
output_both([*pcre2grep_args, "abc", "./testdata/grepinput", "./testdata/nonexistfile"], cwd=srcdir)
header(17)
output([*pcre2grep_args, "-M", r"the\noutput", "./testdata/grepinput"], cwd=srcdir)
header(18)
output([*pcre2grep_args, "-Mn", r"(the\noutput|dog\.\n--)", "./testdata/grepinput"], cwd=srcdir)
header(19)
output([*pcre2grep_args, "-Mix", "Pattern", "./testdata/grepinputx"], cwd=srcdir)
header(20)
output([*pcre2grep_args, "-Mixn", r"complete pair\nof lines", "./testdata/grepinputx"], cwd=srcdir)
header(21)
output([*pcre2grep_args, "-nA3", "four", "./testdata/grepinputx"], cwd=srcdir)
header(22)
output([*pcre2grep_args, "-nB3", "four", "./testdata/grepinputx"], cwd=srcdir)
header(23)
output([*pcre2grep_args, "-C3", "four", "./testdata/grepinputx"], cwd=srcdir)
header(24)
output([*pcre2grep_args, "-A9", "four", "./testdata/grepinputx"], cwd=srcdir)
header(25)
output([*pcre2grep_args, "-nB9", "four", "./testdata/grepinputx"], cwd=srcdir)
header(26)
output([*pcre2grep_args, "-A9", "-B9", "four", "./testdata/grepinputx"], cwd=srcdir)
header(27)
output([*pcre2grep_args, "-A10", "four", "./testdata/grepinputx"], cwd=srcdir)
header(28)
output([*pcre2grep_args, "-nB10", "four", "./testdata/grepinputx"], cwd=srcdir)
header(29)
output([*pcre2grep_args, "-C12", "-B10", "four", "./testdata/grepinputx"], cwd=srcdir)
header(30)
output([*pcre2grep_args, "-inB3", "pattern", "./testdata/grepinput", "./testdata/grepinputx"], cwd=srcdir)
header(31)
output([*pcre2grep_args, "-inA3", "pattern", "./testdata/grepinput", "./testdata/grepinputx"], cwd=srcdir)
header(32)
output([*pcre2grep_args, "-L", "fox", "./testdata/grepinput", "./testdata/grepinputx"], cwd=srcdir)
header(33)
output_both([*pcre2grep_args, "fox", "./testdata/grepnonexist"], cwd=srcdir)
header(34)
output_both([*pcre2grep_args, "-s", "fox", "./testdata/grepnonexist"], cwd=srcdir)
header(35)
output([*pcre2grep_args, "-L", "-r", "--include=grepinputx", "--include", "grepinput8", "--exclude-dir=^\\.", "fox", "./testdata"], cwd=srcdir, transform=lambda data: b"".join(sorted(data.splitlines(keepends=True))))
header(36)
output([*pcre2grep_args, "-L", "-r", "--include=grepinput[^C]", "--exclude", "grepinput$", "--exclude=grepinput(Bad)?8", "--exclude=grepinputM", "--exclude=grepinputUN", "--exclude-dir=^\\.", "fox", "./testdata"], cwd=srcdir, transform=lambda data: b"".join(sorted(data.splitlines(keepends=True))))
header(37)
with open("teststderrgrep", "wb") as stream:
  output([*pcre2grep_args, r"^(a+)*\d", "./testdata/grepinput"], cwd=srcdir, stderr=stream)
with open("testtrygrep", "ab") as stream:
  stream.write(b"======== STDERR ========\n")
  stream.write(Path("teststderrgrep").read_bytes())
header(38)
output([*pcre2grep_args, r">\x00<", "./testdata/grepinput"], cwd=srcdir)
header(39)
output([*pcre2grep_args, "-A1", "before the binary zero", "./testdata/grepinput"], cwd=srcdir)
header(40)
output([*pcre2grep_args, "-B1", "after the binary zero", "./testdata/grepinput"], cwd=srcdir)
header(41)
output([*pcre2grep_args, "-B1", "-o", r"\w+ the binary zero", "./testdata/grepinput"], cwd=srcdir)
header(42)
output([*pcre2grep_args, "-B1", "-onH", r"\w+ the binary zero", "./testdata/grepinput"], cwd=srcdir)
header(43)
output([*pcre2grep_args, "-on", "before|zero|after", "./testdata/grepinput"], cwd=srcdir)
header(44)
output([*pcre2grep_args, "-on", "-e", "before", "-ezero", "-e", "after", "./testdata/grepinput"], cwd=srcdir)
header(45)
output([*pcre2grep_args, "-on", "-f", "./testdata/greplist", "-e", "binary", "./testdata/grepinput"], cwd=srcdir)
header(46)
output_both([*pcre2grep_args, "-e", "unopened)", "-e", "abc", "./testdata/grepinput"], cwd=srcdir)
output_both([*pcre2grep_args, "-eabc", "-e", "(unclosed", "./testdata/grepinput"], cwd=srcdir)
output_both([*pcre2grep_args, "-eabc", "-e", "xyz", "-e", "[unclosed", "./testdata/grepinput"], cwd=srcdir)
output_both([*pcre2grep_args, "--regex=123", "-eabc", "-e", "xyz", "-e", "[unclosed", "./testdata/grepinput"], cwd=srcdir)
header(47)
output([*pcre2grep_args, "-Fx", "AB.VE\nelephant", "./testdata/grepinput"], cwd=srcdir)
header(48)
output([*pcre2grep_args, "-F", "AB.VE\nelephant", "./testdata/grepinput"], cwd=srcdir)
header(49)
output([*pcre2grep_args, "-F", "-e", "DATA", "-e", "AB.VE\nelephant", "./testdata/grepinput"], cwd=srcdir)
header(50)
output([*pcre2grep_args, "^(abc|def|ghi|jkl)", "./testdata/grepinputx"], cwd=srcdir)
header(51)
output([*pcre2grep_args, "-Mv", r"brown\sfox", "./testdata/grepinputv"], cwd=srcdir)
header(52)
output([*pcre2grep_args, "--colour=always", "jumps", "./testdata/grepinputv"], cwd=srcdir)
header(53)
output([*pcre2grep_args, "--file-offsets", "before|zero|after", "./testdata/grepinput"], cwd=srcdir)
header(54)
output([*pcre2grep_args, "--line-offsets", "before|zero|after", "./testdata/grepinput"], cwd=srcdir)
header(55)
output([*pcre2grep_args, "-f./testdata/greplist", "--color=always", "./testdata/grepinputx"], cwd=srcdir)
header(56)
output([*pcre2grep_args, "-c", "--exclude=grepinputC", "lazy", *[f"./testdata/{path.name}" for path in sorted((srcdir / "testdata").glob("grepinput*"))]], cwd=srcdir)
header(57)
output([*pcre2grep_args, "-c", "-l", "--exclude=grepinputC", "lazy", *[f"./testdata/{path.name}" for path in sorted((srcdir / "testdata").glob("grepinput*"))]], cwd=srcdir)
header(58)
output([*pcre2grep_args, "--regex=PATTERN", "./testdata/grepinput"], cwd=srcdir)
header(59)
output([*pcre2grep_args, "--regexp=PATTERN", "./testdata/grepinput"], cwd=srcdir)
header(60)
output([*pcre2grep_args, "--regex", "PATTERN", "./testdata/grepinput"], cwd=srcdir)
header(61)
output([*pcre2grep_args, "--regexp", "PATTERN", "./testdata/grepinput"], cwd=srcdir)
header(62)
output_both([*pcre2grep_args, "--match-limit=1000", "--no-jit", "-M", r"This is a file(.|\R)*file.", "./testdata/grepinput"], cwd=srcdir, vjs_args=False)
header(63)
output_both([*pcre2grep_args, "--recursion-limit=1K", "--no-jit", "-M", r"This is a file(.|\R)*file.", "./testdata/grepinput"], cwd=srcdir, vjs_args=False)
header(64)
output([*pcre2grep_args, "-o1", "(?<=PAT)TERN (ap(pear)s)", "./testdata/grepinput"], cwd=srcdir)
header(65)
output([*pcre2grep_args, "-o2", "(?<=PAT)TERN (ap(pear)s)", "./testdata/grepinput"], cwd=srcdir)
header(66)
output([*pcre2grep_args, "-o3", "(?<=PAT)TERN (ap(pear)s)", "./testdata/grepinput"], cwd=srcdir)
header(67)
output([*pcre2grep_args, "-o12", "(?<=PAT)TERN (ap(pear)s)", "./testdata/grepinput"], cwd=srcdir)
header(68)
output([*pcre2grep_args, "--only-matching=2", "(?<=PAT)TERN (ap(pear)s)", "./testdata/grepinput"], cwd=srcdir)
header(69)
output([*pcre2grep_args, "-vn", "--colour=always", "pattern", "./testdata/grepinputx"], cwd=srcdir)
header(70)
output([*pcre2grep_args, "--color=always", "-M", r"triple:\t.*\n\n", "./testdata/grepinput3"], cwd=srcdir)
output([*pcre2grep_args, "--color=always", "-M", "-n", r"triple:\t.*\n\n", "./testdata/grepinput3"], cwd=srcdir)
output([*pcre2grep_args, "-M", r"triple:\t.*\n\n", "./testdata/grepinput3"], cwd=srcdir)
output([*pcre2grep_args, "-M", "-n", r"triple:\t.*\n\n", "./testdata/grepinput3"], cwd=srcdir)
header(71)
output([*pcre2grep_args, "-o", "^01|^02|^03", "./testdata/grepinput"], cwd=srcdir)
header(72)
output([*pcre2grep_args, "--color=always", "^01|^02|^03", "./testdata/grepinput"], cwd=srcdir)
header(73)
output([*pcre2grep_args, "-o", "--colour=always", "^01|^02|^03", "./testdata/grepinput"], cwd=srcdir)
header(74)
output([*pcre2grep_args, "-o", "^01|02|^03", "./testdata/grepinput"], cwd=srcdir)
header(75)
output([*pcre2grep_args, "--color=always", "^01|02|^03", "./testdata/grepinput"], cwd=srcdir)
header(76)
output([*pcre2grep_args, "-o", "--colour=always", "^01|02|^03", "./testdata/grepinput"], cwd=srcdir)
header(77)
output([*pcre2grep_args, "-o", "^01|^02|03", "./testdata/grepinput"], cwd=srcdir)
header(78)
output([*pcre2grep_args, "--color=always", "^01|^02|03", "./testdata/grepinput"], cwd=srcdir)
header(79)
output([*pcre2grep_args, "-o", "--colour=always", "^01|^02|03", "./testdata/grepinput"], cwd=srcdir)
header(80)
output([*pcre2grep_args, "-o", r"\b01|\b02", "./testdata/grepinput"], cwd=srcdir)
header(81)
output([*pcre2grep_args, "--color=always", r"\b01|\b02", "./testdata/grepinput"], cwd=srcdir)
header(82)
output([*pcre2grep_args, "-o", "--colour=always", r"\b01|\b02", "./testdata/grepinput"], cwd=srcdir)
header(83)
output_both([*pcre2grep_args, "--buffer-size=10", "--max-buffer-size=100", "^a", "./testdata/grepinput3"], cwd=srcdir)
header(84)
Path("testtemp1grep").write_bytes(b"testdata/grepinput3\n")
output_both([*pcre2grep_args, "--file-list", "./testdata/grepfilelist", "--file-list", builddir / "testtemp1grep", "fox|complete|t7"], cwd=srcdir)
header(85)
output_both([*pcre2grep_args, "--file-list=./testdata/grepfilelist", "dolor", "./testdata/grepinput3"], cwd=srcdir)
header(86)
output_both([*pcre2grep_args, "dog", "./testdata/grepbinary"], cwd=srcdir)
header(87)
output_both([*pcre2grep_args, "cat", "./testdata/grepbinary"], cwd=srcdir)
header(88)
output_both([*pcre2grep_args, "-v", "cat", "./testdata/grepbinary"], cwd=srcdir)
header(89)
output_both([*pcre2grep_args, "-I", "dog", "./testdata/grepbinary"], cwd=srcdir)
header(90)
output_both([*pcre2grep_args, "--binary-files=without-match", "dog", "./testdata/grepbinary"], cwd=srcdir)
header(91)
output_both([*pcre2grep_args, "-a", "dog", "./testdata/grepbinary"], cwd=srcdir)
header(92)
output_both([*pcre2grep_args, "--binary-files=text", "dog", "./testdata/grepbinary"], cwd=srcdir)
header(93)
output_both([*pcre2grep_args, "--text", "dog", "./testdata/grepbinary"], cwd=srcdir)
header(94)
output([*pcre2grep_args, "-L", "-r", "--include=grepinputx", "--include", "grepinput8", "fox", *[f"./testdata/{path.name}" for path in sorted((srcdir / "testdata").glob("grepinput*"))]], cwd=srcdir, transform=lambda data: b"".join(sorted(data.splitlines(keepends=True))))
header(95)
output_both([*pcre2grep_args, "--file-list", "./testdata/grepfilelist", "--exclude", "grepinputv", "fox|complete"], cwd=srcdir)
header(96)
output([*pcre2grep_args, "-L", "-r", "--include-dir=testdata", "--exclude", "^(?!grepinput)", "--exclude=grepinput[MCU]", "fox", *[f"./{path.name}" for path in sorted(srcdir.glob("test*"))]], cwd=srcdir, transform=lambda data: b"".join(sorted(data.splitlines(keepends=True))))
header(97)
Path("testtemp1grep").write_bytes(b"grepinput$\ngrepinput8\ngrepinputBad8\n")
output([*pcre2grep_args, "-L", "-r", "--include=grepinput", "--exclude=grepinput[MCU]", "--exclude-from", builddir / "testtemp1grep", "--exclude-dir=^\\.", "fox", "./testdata"], cwd=srcdir, transform=lambda data: b"".join(sorted(data.splitlines(keepends=True))))
header(98)
Path("testtemp1grep").write_bytes(b"grepinput$\ngrepinput8\ngrepinputBad8\n")
output([*pcre2grep_args, "-L", "-r", "--exclude=grepinput3", "--exclude=grepinput[MCU]", "--include=grepinput", "--exclude-from", builddir / "testtemp1grep", "--exclude-dir=^\\.", "fox", "./testdata"], cwd=srcdir, transform=lambda data: b"".join(sorted(data.splitlines(keepends=True))))
header(99)
Path("testtemp1grep").write_bytes(b"grepinput$\ngrepinputBad8\n")
Path("testtemp2grep").write_bytes(b"grepinput8\n")
output([*pcre2grep_args, "-L", "-r", "--include", "grepinput", "--exclude=grepinput[MCU]", "--exclude-from", builddir / "testtemp1grep", f"--exclude-from={builddir / 'testtemp2grep'}", "--exclude-dir=^\\.", "fox", "./testdata"], cwd=srcdir, transform=lambda data: b"".join(sorted(data.splitlines(keepends=True))))
header(100)
output([*pcre2grep_args, "-Ho2", "--only-matching=1", "-o3", r"(\w+) binary (\w+)(\.)?", "./testdata/grepinput"], cwd=srcdir)
header(101)
output([*pcre2grep_args, "-o3", "-Ho2", "-o12", "--only-matching=1", "-o3", "--colour=always", "--om-separator=|", r"(\w+) binary (\w+)(\.)?", "./testdata/grepinput"], cwd=srcdir)
header(102)
output_both([*pcre2grep_args, "-n", "^$", "./testdata/grepinput3"], cwd=srcdir)
header(103)
output_both([*pcre2grep_args, "--only-matching", "^$", "./testdata/grepinput3"], cwd=srcdir)
header(104)
output_both([*pcre2grep_args, "-n", "--only-matching", "^$", "./testdata/grepinput3"], cwd=srcdir)
header(105)
output_both([*pcre2grep_args, "--colour=always", "ipsum|", "./testdata/grepinput3"], cwd=srcdir)
header(106)
output_both([*pcre2grep_args, "-M", "|a"], cwd=srcdir, stdin=b"a\n")
header(107)
Path("testtemp1grep").write_bytes(b"a\naaaaa\n")
output_both([*pcre2grep_args, "--line-offsets", "--allow-lookaround-bsk", r"(?<=\Ka)", builddir / "testtemp1grep"], cwd=srcdir)
header(108)
output([*pcre2grep_args, "-lq", "PATTERN", "./testdata/grepinput", "./testdata/grepinputx"], cwd=srcdir)
header(109)
output([*pcre2grep_args, "-cq", "--exclude=grepinputC", "lazy", *[f"./testdata/{path.name}" for path in sorted((srcdir / "testdata").glob("grepinput*"))]], cwd=srcdir)
header(110)
output([*pcre2grep_args, "--om-separator", "/", "-Mo0", "-o1", "-o2", r"match (\d+):\n (.)\n", "testdata/grepinput"], cwd=srcdir)
header(111)
output([*pcre2grep_args, "--line-offsets", "-M", r"match (\d+):\n (.)\n", "testdata/grepinput"], cwd=srcdir)
header(112)
output([*pcre2grep_args, "--file-offsets", "-M", r"match (\d+):\n (.)\n", "testdata/grepinput"], cwd=srcdir)
header(113)
output([*pcre2grep_args, "--total-count", "--exclude=grepinputC", "the", *[f"testdata/{path.name}" for path in sorted((srcdir / "testdata").glob("grepinput*"))]], cwd=srcdir)
header(114)
output([*pcre2grep_args, "-tc", "--exclude=grepinputC", "the", *[f"testdata/{path.name}" for path in sorted((srcdir / "testdata").glob("grepinput*"))]], cwd=srcdir)
header(115)
output([*pcre2grep_args, "-tlc", "--exclude=grepinputC", "the", *[f"testdata/{path.name}" for path in sorted((srcdir / "testdata").glob("grepinput*"))]], cwd=srcdir)
header(116)
output([*pcre2grep_args, "--exclude=grepinput[MCU]", "-th", "the", *[f"testdata/{path.name}" for path in sorted((srcdir / "testdata").glob("grepinput*"))]], cwd=srcdir)
header(117)
output([*pcre2grep_args, "-tch", "--exclude=grepinputC", "the", *[f"testdata/{path.name}" for path in sorted((srcdir / "testdata").glob("grepinput*"))]], cwd=srcdir)
header(118)
output([*pcre2grep_args, "-tL", "--exclude=grepinputC", "the", *[f"testdata/{path.name}" for path in sorted((srcdir / "testdata").glob("grepinput*"))]], cwd=srcdir)
header(119)
Path("testNinputgrep").write_bytes(b"123\n456\n789\n---abc\ndef\nxyz\n---\n")
output([*pcre2grep_args, "-Mo", r"(\n|[^-])*---", "testNinputgrep"])
header(120)
output([*pcre2grep_args, "-HO", "$0:$2$1$3", r"(\w+) binary (\w+)(\.)?", "./testdata/grepinput"], cwd=srcdir)
output([*pcre2grep_args, "-HO", "$&:$2$1$3", r"(\w+) binary (\w+)(\.)?", "./testdata/grepinput"], cwd=srcdir)
output([*pcre2grep_args, "-m", "1", "-O", "$0:$a$b$e$f$r$t$v", r"(\w+) binary (\w+)(\.)?", "./testdata/grepinput"], cwd=srcdir)
output_both([*pcre2grep_args, "-HO", "${X}", r"(\w+) binary (\w+)(\.)?", "./testdata/grepinput"], cwd=srcdir)
output_both([*pcre2grep_args, "-HO", "XX$", r"(\w+) binary (\w+)(\.)?", "./testdata/grepinput"], cwd=srcdir)
output_both([*pcre2grep_args, "-O", "$x{12345678}", r"(\w+) binary (\w+)(\.)?", "./testdata/grepinput"], cwd=srcdir)
output_both([*pcre2grep_args, "-O", "$x{123Z", r"(\w+) binary (\w+)(\.)?", "./testdata/grepinput"], cwd=srcdir)
output_both([*pcre2grep_args, "--output", "$x{1234}", r"(\w+) binary (\w+)(\.)?", "./testdata/grepinput"], cwd=srcdir)
header(121)
output([*pcre2grep_args, "-F", r"\E and (regex)", "testdata/grepinputv"], cwd=srcdir)
header(122)
output([*pcre2grep_args, "-w", "cat|dog", "testdata/grepinputv"], cwd=srcdir)
header(123)
output([*pcre2grep_args, "-w", "dog|cat", "testdata/grepinputv"], cwd=srcdir)
header(124)
output([*pcre2grep_args, "-Mn", "--colour=always", r"start[\s]+end", "testdata/grepinputM"], cwd=srcdir)
output([*pcre2grep_args, "-Mn", "--colour=always", "-A2", r"start[\s]+end", "testdata/grepinputM"], cwd=srcdir)
output([*pcre2grep_args, "-Mn", r"start[\s]+end", "testdata/grepinputM"], cwd=srcdir)
output([*pcre2grep_args, "-Mn", "-A2", r"start[\s]+end", "testdata/grepinputM"], cwd=srcdir)
header(125)
Path("testNinputgrep").write_bytes(b"abcd\n")
output([*pcre2grep_args, "--colour=always", "--allow-lookaround-bsk", r"(?<=\K.)", "testNinputgrep"])
output([*pcre2grep_args, "--colour=always", "--allow-lookaround-bsk", r"(?=.\K)", "testNinputgrep"])
output([*pcre2grep_args, "--colour=always", "--allow-lookaround-bsk", r"(?<=\K[ac])", "testNinputgrep"])
output([*pcre2grep_args, "--colour=always", "--allow-lookaround-bsk", r"(?=[ac]\K)", "testNinputgrep"])
environment=os.environ.copy()
environment["GREP_COLORS"]="ms=1;20"
with open("testtrygrep", "ab") as output_file:
  process=subprocess.run(valgrind + vjs + [*pcre2grep_args, "--colour=always", "--allow-lookaround-bsk", r"(?=[ac]\K)", "testNinputgrep"], stdout=output_file, stderr=subprocess.STDOUT, env=environment, check=False)
  output_file.write(f"RC={process.returncode}\n".encode("latin-1"))
header(126)
Path("testtemp1grep").write_bytes(b"Next line pattern has binary zero\nABC\0XYZ\n")
Path("testtemp2grep").write_bytes(b"ABC\0XYZ\nABCDEF\nDEFABC\n")
output([*pcre2grep_args, "-a", "-f", "testtemp1grep", "testtemp2grep"])
Path("testtemp1grep").write_bytes(b"Next line pattern is erroneous.\n^abc)(xy")
output_both([*pcre2grep_args, "-a", "-f", "testtemp1grep", "testtemp2grep"])
header(127)
output([*pcre2grep_args, "-o", "--om-capture=0", "pattern()()()()", "testdata/grepinput"], cwd=srcdir)
header(128)
output_both([*pcre2grep_args, "-m1M", "-o1", "--om-capture=0", "pattern()()()()", "testdata/grepinput"], cwd=srcdir)
header(129)
output_both([*pcre2grep_args, "-m", "2", "fox", "testdata/grepinput"], cwd=srcdir)
header(130)
output_both([*pcre2grep_args, "-o", "-m2", "fox", "testdata/grepinput"], cwd=srcdir)
header(131)
output_both([*pcre2grep_args, "-oc", "-m2", "fox", "testdata/grepinput"], cwd=srcdir)
header(132)
with open(srcdir / "testdata" / "grepinput", "rb") as stream:
  output_both([*pcre2grep_args, "-m1", "-A3", "^match"], cwd=srcdir, stdin=stream, write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---\n")
  returncode=output_both([*pcre2grep_args, "-m1", ".*"], cwd=srcdir, stdin=stream, write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"RC={returncode}\n".encode("latin-1"))
header(133)
with open(srcdir / "testdata" / "grepinput", "rb") as stream:
  output_both([*pcre2grep_args, "-m1", "-A3", "^match"], cwd=srcdir, stdin=stream, write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---\n")
  returncode=output_both([*pcre2grep_args, "-m1", "-A3", "^match"], cwd=srcdir, stdin=stream, write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"RC={returncode}\n".encode("latin-1"))
header(134)
output_both([*pcre2grep_args, "--max-count=1", "-nH", "-O", "=$x{41}$x423$o{103}$o1045=", "fox", "-"], cwd=srcdir, stdin=(srcdir / "testdata" / "grepinputv").read_bytes())
header(135)
output([*pcre2grep_args, "-HZ", "word", "./testdata/grepinputv"], cwd=srcdir, transform=lambda data: data.replace(b"\0", b"@"))
output([*pcre2grep_args, "-lZ", "word", "./testdata/grepinputv", "./testdata/grepinputv"], cwd=srcdir, transform=lambda data: data.replace(b"\0", b"@"))
output([*pcre2grep_args, "-A", "1", "-B", "1", "-HZ", "word", "./testdata/grepinputv"], cwd=srcdir, transform=lambda data: data.replace(b"\0", b"@"))
output([*pcre2grep_args, "-MHZn", r"start[\s]+end", "testdata/grepinputM"], cwd=srcdir)
header(136)
output_both([*pcre2grep_args, "-m1MK", "-o1", "--om-capture=0", "pattern()()()()", "testdata/grepinput"], cwd=srcdir)
output_both([*pcre2grep_args, "--max-count=1MK", "-o1", "--om-capture=0", "pattern()()()()", "testdata/grepinput"], cwd=srcdir)
header(137)
Path("testtemp1grep").write_bytes(b"Last line\nhas no newline")
output([*pcre2grep_args, "-A1", "Last", "testtemp1grep"])
header(138)
Path("testtemp1grep").write_bytes(b"AbC\n" * 24)
output_both([*pcre2grep_args, "--no-jit", "--heap-limit=0", "b", "testtemp1grep"])
header(139)
output([*pcre2grep_args, "--line-buffered", "fox", "testdata/grepinputv"], cwd=srcdir)
header(140)
output([*pcre2grep_args, "--buffer-size=10", "-A1", "brown", "testdata/grepinputv"], cwd=srcdir)
header(141)
Path("testtemp1grep").write_bytes(f"{srcdir}/testdata/grepinputv\n-\n".encode("latin-1"))
Path("testtemp2grep").write_bytes(b"This is a line from stdin.")
output_both([*pcre2grep_args, "--file-list", "testtemp1grep", "line from stdin"], stdin=Path("testtemp2grep").read_bytes())
header(142)
Path("testtemp1grep").write_bytes(b"/does/not/exist\n")
output_both([*pcre2grep_args, "--file-list", "testtemp1grep", "line from stdin"])
header(143)
Path("testtemp1grep").write_bytes(b"fox|cat")
output_both([*pcre2grep_args, "-f", "-", srcdir / "testdata" / "grepinputv"], stdin=Path("testtemp1grep").read_bytes())
header(144)
output_both([*pcre2grep_args, "-f", "/non/exist", srcdir / "testdata" / "grepinputv"])
header(145)
Path("testtemp1grep").write_bytes(b"*meta*\rdog.")
output_both([*pcre2grep_args, "-Ncr", "-F", "-f", "testtemp1grep", srcdir / "testdata" / "grepinputv"])
header(146)
Path("testtemp1grep").write_bytes(b"A123B")
output_both([*pcre2grep_args, "-H", "-e", "123|fox", "-"], stdin=Path("testtemp1grep").read_bytes())
output_both([*pcre2grep_args, "-h", "-e", "123|fox", "-", srcdir / "testdata" / "grepinputv"], stdin=Path("testtemp1grep").read_bytes())
output_both([*pcre2grep_args, "-", srcdir / "testdata" / "grepinputv"], stdin=Path("testtemp1grep").read_bytes())
header(147)
output_both([*pcre2grep_args, "-e", "123|fox", "--", "-nonfile"])
Path("testtemp1grep").write_bytes(b"A123B")
Path("-testtemp1grep").write_bytes(b"C123D")
output_both([*pcre2grep_args, "123", "--", "-testtemp1grep"])
output_both([*pcre2grep_args, "123", "testtemp1grep", "--", "-testtemp1grep"])
output_both([*pcre2grep_args, "123", "--"], stdin=Path("testtemp1grep").read_bytes())
Path("--").write_bytes(b"E123F")
output_both([*pcre2grep_args, "--", "123", "--"])
header(148)
output_both([*pcre2grep_args, "--nonexist"])
output_both([*pcre2grep_args, "-n-n-bad"])
output_both([*pcre2grep_args, "--context"])
output_both([*pcre2grep_args, "--only-matching", "--output=xx"])
output_both([*pcre2grep_args, "--colour=badvalue"])
output_both([*pcre2grep_args, "--newline=badvalue"])
output_both([*pcre2grep_args, "-d", "badvalue"])
output_both([*pcre2grep_args, "-D", "badvalue"])
output_both([*pcre2grep_args, "--buffer-size=0"])
output_both([*pcre2grep_args, "--exclude", "(badpat", "abc", os.devnull])
output_both([*pcre2grep_args, "--exclude-from", "/non/exist", "abc", os.devnull])
output_both([*pcre2grep_args, "--include-from", "/non/exist", "abc", os.devnull])
output_both([*pcre2grep_args, "--file-list=/non/exist", "abc", os.devnull])
header(149)
output_both([*pcre2grep_args, "--binary-files=binary", "dog", "./testdata/grepbinary"], cwd=srcdir)
output_both([*pcre2grep_args, "--binary-files=wrong", "dog", "./testdata/grepbinary"], cwd=srcdir)

# This test runs the code that tests locale support. However, on some systems
# (e.g. Alpine Linux) there is no locale support and running this test just
# generates a "no match" result. Therefore, we test for locale support, and if
# it is found missing, we pretend that the test has run as expected so that the
# output matches.

header(150)
if shutil.which("locale") is None:
  with open("testtrygrep", "ab") as stream: stream.write(b"pcre2grep: Failed to set locale locale.bad (obtained from LC_CTYPE)\nRC=2\n")
else:
  environment=os.environ.copy()
  environment.pop("LC_ALL", None)
  environment["LC_CTYPE"]="locale.bad"
  with open("testtrygrep", "ab") as stream:
    process=subprocess.run(valgrind + vjs + [*pcre2grep_args, "abc", os.devnull], cwd=srcdir, stdout=stream, stderr=subprocess.STDOUT, env=environment, check=False)
    stream.write(f"RC={process.returncode}\n".encode("latin-1"))
header(151)
output([*pcre2grep_args, "--colour=always", "-e", "this", "-e", "The", "-e", "The wo", "testdata/grepinputv"], cwd=srcdir)
header(152)
output([*pcre2grep_args, "-nA3", "--group-separator=++", "four", "./testdata/grepinputx"], cwd=srcdir)
header(153)
output([*pcre2grep_args, "-nA3", "--no-group-separator", "four", "./testdata/grepinputx"], cwd=srcdir)
header(154)
Path("testtemp1grep").write_bytes(b"")
output([*pcre2grep_args, "-f", builddir / "testtemp1grep", "./testdata/grepinputv"], cwd=srcdir)
header(155)
Path("testtemp1grep").write_bytes(b"\n")
output([*pcre2grep_args, "-f", builddir / "testtemp1grep", "./testdata/grepinputv"], cwd=srcdir)
header(156)
Path("testtemp1grep").write_bytes(b"\n")
output([*pcre2grep_args, "--posix-pattern-file", "--file", builddir / "testtemp1grep", "./testdata/grepinputv"], cwd=srcdir)
header(157)
Path("testtemp1grep").write_bytes(b"spaces \n")
with open(builddir / "testtemp2grep", "wb") as stream: process=subprocess.run(valgrind + vjs + [*pcre2grep_args, "-o", "--posix-pattern-file", f"--file={builddir / 'testtemp1grep'}", "./testdata/grepinputv"], cwd=srcdir, stdout=stream, check=False)
if process.returncode == 0: output([*pcre2grep_args, "-q", "s ", builddir / "testtemp2grep"], cwd=srcdir)
else:
  with open("testtrygrep", "ab") as output_file: output_file.write(f"RC={process.returncode}\n".encode("latin-1"))
header(158)
Path("testtemp1grep").write_bytes(b"spaces.\n")
output([*pcre2grep_args, "-f", builddir / "testtemp1grep", "./testdata/grepinputv"], cwd=srcdir)
header(159)
Path("testtemp1grep").write_bytes(b"spaces.\r\n")
output([*pcre2grep_args, "--posix-pattern-file", f"-f{builddir / 'testtemp1grep'}", "./testdata/grepinputv"], cwd=srcdir)
header(160)
output([*pcre2grep_args, "-nC3", "^(ert|jkl)", "./testdata/grepinput"], cwd=srcdir)
output([*pcre2grep_args, "-n", "-B4", "-A2", "^(ert|dfg)", "./testdata/grepinput"], cwd=srcdir)
header(161)
Path("testNinputgrep").write_bytes(b"XfooY\n")
output([*pcre2grep_args, "--allow-lookaround-bsk", "-o", r"(?=foo\K)", "testNinputgrep"])
output([*pcre2grep_args, "--allow-lookaround-bsk", "--output", "$0", r"(?=foo\K)", "testNinputgrep"])
output([*pcre2grep_args, "--allow-lookaround-bsk", "--line-offsets", r"(?=foo\K)", "testNinputgrep"])
output([*pcre2grep_args, "--allow-lookaround-bsk", "--file-offsets", r"(?=foo\K)", "testNinputgrep"])

# Now compare the results.

if not compare(srcdir / "testdata" / "grepoutput", "testtrygrep"): sys.exit(1)

# These tests require UTF-8 support

if utf8 != 0:
  print("Testing pcre2grep UTF-8 features")
  with open("testtrygrep", "wb") as output_file: output_file.write(b"---------------------------- Test U1 ------------------------------\n")
  output([*pcre2grep_args, "-n", "-u", "--newline=any", "^X", "./testdata/grepinput8"], cwd=srcdir)
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U2 ------------------------------\n")
  output([*pcre2grep_args, "-n", "-u", "-C", "3", "--newline=any", "Match", "./testdata/grepinput8"], cwd=srcdir)
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U3 ------------------------------\n")
  output([*pcre2grep_args, "--line-offsets", "-u", "--newline=any", "--allow-lookaround-bsk", r"(?<=\K\x{17f})", "./testdata/grepinput8"], cwd=srcdir)
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U4 ------------------------------\n")
  output_both([*pcre2grep_args, "-u", "-o", "....", "./testdata/grepinputBad8"], cwd=srcdir)
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U5 ------------------------------\n")
  output([*pcre2grep_args, "-U", "-o", "....", "./testdata/grepinputBad8"], cwd=srcdir)
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U6 -----------------------------\n")
  output_both([*pcre2grep_args, "-u", "-m1", "-O", "=$x{1d3}$o{744}=", "fox"], cwd=srcdir, stdin=(srcdir / "testdata" / "grepinputv").read_bytes())
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U7 ------------------------------\n")
  output([*pcre2grep_args, "-ui", "--colour=always", r"k+|\babc\b", "./testdata/grepinput8"], cwd=srcdir)
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U8 ------------------------------\n")
  output([*pcre2grep_args, "-UiEP", "--colour=always", r"k+|\babc\b", "./testdata/grepinput8"], cwd=srcdir)
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U9 ------------------------------\n")
  output([*pcre2grep_args, "-u", "--colour=always", r"A\d", "./testdata/grepinput8"], cwd=srcdir)
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U10 ------------------------------\n")
  output([*pcre2grep_args, "-u", "--posix-digit", "--colour=always", r"A\d", "./testdata/grepinput8"], cwd=srcdir)
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U11 ------------------------------\n")
  Path("testtemp1grep").write_bytes(b"\200" * 48 + b"x\n")
  returncode=output_both([*pcre2grep_args, "--no-jit", "--buffer-size=16", "-U", "--allow-lookaround-bsk", "-o", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"only-matching RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "--no-jit", "--buffer-size=16", "-U", "--allow-lookaround-bsk", "-M", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "--no-jit", "--buffer-size=16", "-U", "--allow-lookaround-bsk", "--colour=always", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour RC={returncode}\n".encode("latin-1"))
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U12 ------------------------------\n")
  # Colouring only, so the subject is a single line. An empty match at the end
  # of the subject, then one that matches before retrying at the end.
  Path("testtemp1grep").write_bytes(b"abc\n")
  returncode=output_both([*pcre2grep_args, "-n", "--colour=always", "-u", "$", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour-empty-at-end RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "--colour=always", "-u", ".?", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour-nonempty-then-empty RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "--colour=always", "-u", "^", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour-empty-at-start RC={returncode}\n".encode("latin-1"))

  # An empty line, so the first match is empty with nothing following it, and a
  # final line with no newline, so a restart can reach the end of the line.
  Path("testtemp1grep").write_bytes(b"\nabc\n")
  returncode=output_both([*pcre2grep_args, "-n", "--colour=always", "-u", ".?", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour-empty-line RC={returncode}\n".encode("latin-1"))
  Path("testtemp1grep").write_bytes(b"abc")
  returncode=output_both([*pcre2grep_args, "-n", "--colour=always", "-u", ".?", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour-no-final-newline RC={returncode}\n".encode("latin-1"))

  # Multiline, where a restart may have to move on to a following line.
  Path("testtemp1grep").write_bytes(b"a\nb\n")
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-u", "$", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-empty-at-line-end RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-u", "^", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-empty-at-line-start RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-u", ".?", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-nonempty-then-empty RC={returncode}\n".encode("latin-1"))

  # A match that spans several lines, so that the restart has to skip over more
  # than one line before looking for the next match.
  Path("testtemp1grep").write_bytes(b"a\nb\nc\nd\n")
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-u", "(?s)a.*c", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-spanning RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-u", "(?s)a.*c|$", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-spanning-then-empty RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-M", "--colour=always", "-u", "(?s)a.*c|$", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-colour-spanning RC={returncode}\n".encode("latin-1"))

  # Invalid UTF-8 after an empty match, which is when the scan over
  # continuation bytes that follow the restart point can run. Here the scan
  # stops at the start of the next character.
  Path("testtemp1grep").write_bytes(b"x\200\200y\n")
  returncode=output_both([*pcre2grep_args, "-n", "--colour=always", "-U", "^", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour-utf-scan-to-char RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-U", "^", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-utf-scan-to-char RC={returncode}\n".encode("latin-1"))

  # The continuation bytes run up to the newline, and then up to the end of the
  # subject, when there is no newline to stop the scan.
  Path("testtemp1grep").write_bytes(b"x\200\200\n")
  returncode=output_both([*pcre2grep_args, "-n", "--colour=always", "-U", "^", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour-utf-scan-to-newline RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-U", "^", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-utf-scan-to-newline RC={returncode}\n".encode("latin-1"))
  Path("testtemp1grep").write_bytes(b"x\200\200")
  returncode=output_both([*pcre2grep_args, "-n", "--colour=always", "-U", "^", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour-utf-scan-to-end RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-U", "^", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-utf-scan-to-end RC={returncode}\n".encode("latin-1"))

  # A subject that is nothing but continuation bytes, so the very first match
  # is empty and the scan immediately reaches the end.
  Path("testtemp1grep").write_bytes(b"\200\200")
  returncode=output_both([*pcre2grep_args, "-n", "--colour=always", "-U", ".?", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour-utf-only-continuations RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-U", ".?", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-utf-only-continuations RC={returncode}\n".encode("latin-1"))

  # Continuation bytes at the end of a multi-line subject, reached after an
  # empty match on an earlier line, and after a match that spans lines.
  Path("testtemp1grep").write_bytes(b"a\nb\n\n\200\200")
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-U", ".?", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-utf-trailing RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-U", "$", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-utf-trailing-empty RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-M", "--colour=always", "-U", "(?s)a.*b|$", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-colour-utf-trailing-spanning RC={returncode}\n".encode("latin-1"))
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U13 ------------------------------\n")
  # The same restart, but reached by \K rather than by an empty match.
  Path("testtemp1grep").write_bytes(b"abc\n")
  returncode=output_both([*pcre2grep_args, "-n", "--colour=always", "-u", "--allow-lookaround-bsk", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour-bsk RC={returncode}\n".encode("latin-1"))
  Path("testtemp1grep").write_bytes(b"a\nb\n")
  returncode=output_both([*pcre2grep_args, "-n", "-M", "-u", "--allow-lookaround-bsk", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-bsk RC={returncode}\n".encode("latin-1"))

  # The same, but with the restart landing on invalid UTF-8.
  Path("testtemp1grep").write_bytes(b"x\200\200y\n")
  returncode=output_both([*pcre2grep_args, "--no-jit", "-n", "--colour=always", "-U", "--allow-lookaround-bsk", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"colour-bsk-utf-scan-to-char RC={returncode}\n".encode("latin-1"))
  Path("testtemp1grep").write_bytes(b"x\200\200")
  returncode=output_both([*pcre2grep_args, "--no-jit", "-n", "-M", "-U", "--allow-lookaround-bsk", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-bsk-utf-scan-to-end RC={returncode}\n".encode("latin-1"))
  Path("testtemp1grep").write_bytes(b"a\nx\200\200")
  returncode=output_both([*pcre2grep_args, "--no-jit", "-n", "-M", "--colour=always", "-U", "--allow-lookaround-bsk", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-colour-bsk-utf-trailing RC={returncode}\n".encode("latin-1"))
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test U14 ------------------------------\n")

  # A pattern that keeps making progress, so the restart is the ordinary one
  # and the line is never advanced. This is the path the other U14 tests build on.
  Path("testtemp1grep").write_bytes(b"abcabc\n")
  returncode=output_both([*pcre2grep_args, "-n", "-o", "-u", "a.", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"control-progress RC={returncode}\n".encode("latin-1"))

  # An empty match, which is the case the restart exists for. "$" matches only
  # at the end of the subject, so the restart is immediately at the end; "^"
  # matches at the start, so it has to step forward by one character first.
  returncode=output_both([*pcre2grep_args, "-n", "-o", "-u", "$", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"empty-at-end RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-o", "-u", "^", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"empty-at-start RC={returncode}\n".encode("latin-1"))

  # \K in a lookbehind reports a non-empty match that ends no later than the
  # offset it was started from. PCRE2_NOTEMPTY is set once a match has been
  # found, so this, unlike an empty match, can stop the loop making progress on
  # every iteration and not just the first.
  returncode=output_both([*pcre2grep_args, "-n", "-o", "-u", "--allow-lookaround-bsk", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"bsk-restart RC={returncode}\n".encode("latin-1"))

  # The step forward lands on invalid UTF-8, so the scan over continuation
  # bytes runs. Here it stops at the start of the next character, and then at
  # the newline, both of which are within the subject.
  Path("testtemp1grep").write_bytes(b"x\200\200y\n")
  returncode=output_both([*pcre2grep_args, "--no-jit", "-n", "-o", "-U", "--allow-lookaround-bsk", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"bsk-utf-scan-to-char RC={returncode}\n".encode("latin-1"))
  Path("testtemp1grep").write_bytes(b"x\200\200\n")
  returncode=output_both([*pcre2grep_args, "--no-jit", "-n", "-o", "-U", "--allow-lookaround-bsk", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"bsk-utf-scan-to-newline RC={returncode}\n".encode("latin-1"))

  # The same, but with no newline to stop the scan, so it runs to the end of
  # the subject and then looks at the byte after it.
  Path("testtemp1grep").write_bytes(b"x\200\200")
  returncode=output_both([*pcre2grep_args, "--no-jit", "-n", "-o", "-U", "--allow-lookaround-bsk", r"(?<=\K.)", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"bsk-utf-scan-to-end RC={returncode}\n".encode("latin-1"))

  # Moving on to another line, which only happens in multiline mode. The first
  # match ends exactly at the start of the next line, so the line is advanced
  # and the line number with it; the second ends two lines further on.
  Path("testtemp1grep").write_bytes(b"ab\ncd\n")
  returncode=output_both([*pcre2grep_args, "-n", "-o", "-M", "-u", r"ab\n", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-line-boundary RC={returncode}\n".encode("latin-1"))
  Path("testtemp1grep").write_bytes(b"ab\ncd\nef\n")
  returncode=output_both([*pcre2grep_args, "-n", "-o", "-M", "-u", "(?s)ab.*ef", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-spanning RC={returncode}\n".encode("latin-1"))

  # An empty match in multiline mode, where the step forward is what takes the
  # restart past the end of the line.
  Path("testtemp1grep").write_bytes(b"a\nb\n")
  returncode=output_both([*pcre2grep_args, "-n", "-o", "-M", "-u", "$", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"multiline-empty RC={returncode}\n".encode("latin-1"))

  # A match that ends between the CR and the LF of a CRLF line ending, so the
  # restart is past the text of the line but short of the start of the next
  # one. Without -M the line is never advanced, so that is the control.
  Path("testtemp1grep").write_bytes(b"ab\r\ncd\r\n")
  returncode=output_both([*pcre2grep_args, "-n", "-o", "-N", "CRLF", r"\r", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"crlf-not-multiline RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-o", "-M", "-N", "CRLF", r"\r", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"crlf-mid-terminator RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-n", "-o", "-M", "-N", "ANY", r"\r", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"any-mid-terminator RC={returncode}\n".encode("latin-1"))
  returncode=output_both([*pcre2grep_args, "-M", "-N", "CRLF", "--line-offsets", r"\r", "testtemp1grep"], write_returncode=False)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"crlf-mid-terminator-line-offsets RC={returncode}\n".encode("latin-1"))

  if not compare(srcdir / "testdata" / "grepoutput8", "testtrygrep"): sys.exit(1)
else:
  print("Skipping pcre2grep UTF-8 tests: no UTF-8 support in PCRE2 library")

# We go to some contortions to try to ensure that the tests for the various
# newline settings will work in environments where the normal newline sequence
# is not \n. Do not use exported files, whose line endings might be changed.
# Instead, create an input file using printf so that its contents are exactly
# what we want. Note the messy fudge to get printf to write a string that
# starts with a hyphen. These tests are run in the build directory.

print("Testing pcre2grep newline settings")
Path("testNinputgrep").write_bytes(b"abc\rdef\r\nghi\njkl")
with open("testtrygrep", "wb") as output_file: output_file.write(b"---------------------------- Test N1 ------------------------------\r\n")
output([*pcre2grep_args, "-n", "-N", "CR", "^(abc|def|ghi|jkl)", "testNinputgrep"])
output([*pcre2grep_args, "-B1", "-n", "-N", "CR", "^def", "testNinputgrep"])
with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test N2 ------------------------------\r\n")
output([*pcre2grep_args, "-n", "--newline=crlf", "^(abc|def|ghi|jkl)", "testNinputgrep"])
output([*pcre2grep_args, "-B1", "-n", "-N", "CRLF", "^ghi", "testNinputgrep"])
with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test N3 ------------------------------\r\n")
output([*pcre2grep_args, "-n", "--newline=cr", "-F", "def\rjkl", "testNinputgrep"])
with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test N4 ------------------------------\r\n")
output([*pcre2grep_args, "-n", "--newline=crlf", "-F", "-f", srcdir / "testdata" / "greppatN4", "testNinputgrep"])
with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test N5 ------------------------------\r\n")
output([*pcre2grep_args, "-n", "--newline=any", "^(abc|def|ghi|jkl)", "testNinputgrep"])
output([*pcre2grep_args, "-B1", "-n", "--newline=any", "^def", "testNinputgrep"])
with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test N6 ------------------------------\r\n")
output([*pcre2grep_args, "-n", "--newline=anycrlf", "^(abc|def|ghi|jkl)", "testNinputgrep"])
output([*pcre2grep_args, "-B1", "-n", "--newline=anycrlf", "^jkl", "testNinputgrep"])
with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test N7 ------------------------------\r\n")
Path("testNinputgrep").write_bytes(b"xyz\0abc\0def")
output([*pcre2grep_args, "-na", "--newline=nul", "^(abc|def)", "testNinputgrep"], transform=lambda data: data.replace(b"\0", b"@"))
output([*pcre2grep_args, "-B1", "-na", "--newline=nul", "^(abc|def)", "testNinputgrep"], transform=lambda data: data.replace(b"\0", b"@"))
with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test N8 ------------------------------\r\n")
output([*pcre2grep_args, "-na", "--newline=anycrlf", "^a", srcdir / "testdata" / "grepinputBad8_Trail"])
with open("testtrygrep", "ab") as output_file: output_file.write(b"\n")
if not compare(srcdir / "testdata" / "grepoutputN", "testtrygrep"): sys.exit(1)

# These newline tests need UTF support.

if utf8 != 0:
  print("Testing pcre2grep newline settings with UTF-8 features")
  with open("testtrygrep", "wb") as output_file: output_file.write(b"---------------------------- Test UN1 ------------------------------\r\n")
  output([*pcre2grep_args, "-nau", "--newline=anycrlf", "^(abc|def)", srcdir / "testdata" / "grepinputUN"])
  with open("testtrygrep", "ab") as output_file: output_file.write(b"---------------------------- Test UN2 ------------------------------\r\n")
  output([*pcre2grep_args, "-nauU", "--newline=anycrlf", "^a", srcdir / "testdata" / "grepinputBad8_Trail"])
  with open("testtrygrep", "ab") as output_file: output_file.write(b"\n")
  if not compare(srcdir / "testdata" / "grepoutputUN", "testtrygrep"): sys.exit(1)
else:
  print("Skipping pcre2grep newline UTF-8 tests: no UTF-8 support in PCRE2 library")

# If pcre2grep supports script callouts, run some tests on them. It is possible
# to restrict these callouts to the non-fork case, either for security, or for
# environments that do not support fork(). This is handled by comparing to a
# different output.

if supports("callout scripts in patterns are supported"):
  print("Testing pcre2grep script callouts")
  callout_echo="/bin/echo" if os.name != "nt" else f"{pcre2test}|-echo"
  with open("testtrygrep", "wb") as output_file: output_file.write(b"--- Test 1 ---\n")
  output([*pcre2grep_args, f'(T)(..(.))(?C"{callout_echo}|Arg1: [$1] [$2] [$3]|Arg2: $|${{1}}$| ($4) ($14) ($0)")()', srcdir / "testdata" / "grepinputv"])
  with open("testtrygrep", "ab") as output_file: output_file.write(b"--- Test 2 ---\n")
  output([*pcre2grep_args, f'(T)(..(.))()()()()()()()(..)(?C"{callout_echo}|Arg1: [$11] [${{11}}]")', srcdir / "testdata" / "grepinputv"])
  with open("testtrygrep", "ab") as output_file: output_file.write(b"--- Test 3 ---\n")
  output([*pcre2grep_args, '(T)(?C"|$0:$1$n")', srcdir / "testdata" / "grepinputv"])
  with open("testtrygrep", "ab") as output_file: output_file.write(b"--- Test 4 ---\n")
  output([*pcre2grep_args, f'(T)(?C"{callout_echo}|$0:$1$n")', srcdir / "testdata" / "grepinputv"])
  with open("testtrygrep", "ab") as output_file: output_file.write(b"--- Test 5 ---\n")
  output([*pcre2grep_args, '(T)(?C"|$1$n")(*F)', srcdir / "testdata" / "grepinputv"])
  with open("testtrygrep", "ab") as output_file: output_file.write(b"--- Test 6 ---\n")
  output([*pcre2grep_args, "-m1", '(T)(?C"|$0:$1:$x{41}$o{101}$n")', srcdir / "testdata" / "grepinputv"])
  nonfork=supports("Non-fork callout scripts in patterns are supported")
  if not compare(srcdir / "testdata" / ("grepoutputCN" if nonfork else "grepoutputC"), "testtrygrep"): sys.exit(1)
  if utf8 != 0:
    print("Testing pcre2grep script callout with UTF-8 features")
    with open("testtrygrep", "wb") as output_file: output_file.write(b"--- Test 1 ---\n")
    output([*pcre2grep_args, "-u", '(T)(?C"|$0:$x{a6}$n")', srcdir / "testdata" / "grepinputv"])
    with open("testtrygrep", "ab") as output_file: output_file.write(b"--- Test 2 ---\n")
    output([*pcre2grep_args, "-u", f'(T)(?C"{callout_echo}|$0:$x{{a6}}$n")', srcdir / "testdata" / "grepinputv"])
    if not compare(srcdir / "testdata" / ("grepoutputCNU" if nonfork else "grepoutputCU"), "testtrygrep"): sys.exit(1)
  else:
    print("Skipping pcre2grep script callout UTF-8 tests: no UTF-8 support in PCRE2 library")
else:
  print("Script callouts are not supported")

# Test reading .gz and .bz2 files when supported.

if supports(r"\.gz are read using zlib"):
  print("Testing reading .gz file")
  with open("testtrygrep", "wb") as output_file: returncode=invoke(*pcre2grep_args, "one|two", srcdir / "testdata" / "grepinputC.gz", stdout=output_file, vjs_args=True)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"RC={returncode}\n".encode("latin-1"))
  if not compare(srcdir / "testdata" / "grepoutputCgz", "testtrygrep"): sys.exit(1)

if supports(r"\.bz2 are read using bzlib2"):
  print("Testing reading .bz2 file")
  with open("testtrygrep", "wb") as output_file: returncode=invoke(*pcre2grep_args, "one|two", srcdir / "testdata" / "grepinputC.bz2", stdout=output_file, vjs_args=True)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"RC={returncode}\n".encode("latin-1"))
  with open("testtrygrep", "ab") as output_file: returncode=invoke(*pcre2grep_args, "one|two", srcdir / "testdata" / "grepnot.bz2", stdout=output_file, vjs_args=True)
  with open("testtrygrep", "ab") as output_file: output_file.write(f"RC={returncode}\n".encode("latin-1"))
  if not compare(srcdir / "testdata" / "grepoutputCbz2", "testtrygrep"): sys.exit(1)

# Finally, some tests to exercise code that is not tested above, just to be
# sure that it runs OK. Doing this improves the coverage statistics. The output
# is not checked.

print("Testing miscellaneous pcre2grep arguments (unchecked)")
Path("testtrygrep").write_bytes(b"\n")
checkspecial(("-xxxxx",), 2)
checkspecial(("--help",), 0)
checkspecial(("--line-buffered", "--colour=auto", "abc", os.devnull), 1)
checkspecial(("--line-buffered", "--color", "abc", os.devnull), 1)
checkspecial(("-dskip", "abc", "."), 1)
checkspecial(("-Dread", "-Dskip", "abc", os.devnull), 1)
checkspecial(("-f", srcdir / "testdata" / "greplistBad", os.devnull), 2)
checkspecial(("(unpaired", os.devnull), 2)
checkspecial(("-e", "(unpaired1", "-e", "(unpaired2", os.devnull), 2)

# Clean up local working files
for filename in ("testNinputgrep", "teststderrgrep", "testtrygrep", "testtemp1grep", "testtemp2grep", "-testtemp1grep", "--"):
  Path(filename).unlink(missing_ok=True)

sys.exit(0)

# End
