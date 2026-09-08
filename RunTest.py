#!/usr/bin/env python3

###############################################################################
# Run the PCRE2 tests using the pcre2test program. The appropriate tests are
# selected, depending on which build-time options were used.
#
# When JIT support is available, all appropriate tests are run with and without
# JIT, unless "-nojit" is given on the command line. There are also two tests
# for JIT-specific features, one to be run when JIT support is available
# (unless "-nojit" is specified), and one when it is not.
#
# Whichever of the 8-, 16- and 32-bit libraries exist are tested. It is also
# possible to select which to test by giving "-8", "-16" or "-32" on the
# command line.
#
# As well as "-nojit", "-8", "-16", and "-32", arguments for this script are
# individual test numbers, ranges of tests such as 3-6 or 3- (meaning 3 to the
# end), or a number preceded by ~ to exclude a test. For example, "3-15 ~10"
# runs tests 3 to 15, excluding test 10, and just "~10" runs all the tests
# except test 10. Whatever order the arguments are in, these tests are always
# run in numerical order.
#
# If no specific tests are selected (which is the case when this script is run
# via 'make check') the default is to run all the numbered tests.
#
# There may also be named (as well as numbered) tests for special purposes. At
# present there is just one, called "heap". This test's output contains the
# sizes of heap frames and frame vectors, which depend on the environment. It
# is therefore not run unless explicitly requested.
#
# Inappropriate tests are automatically skipped (with a comment to say so). For
# example, if JIT support is not compiled, test 16 is skipped, whereas if JIT
# support is compiled, test 15 is skipped.
#
# Other arguments can be one of the words "-valgrind", "-valgrind-log", or
# "-sim" followed by an argument to run cross-compiled executables under a
# simulator, for example:
#
# RunTest 3 -sim "qemu-arm -s 8388608"
#
# For backwards compatibility, -nojit, -valgrind, -valgrind-log, and -sim may
# be given without the leading "-" character.
#
# When PCRE2 is compiled by clang with -fsanitize arguments, some tests need
# very much more stack than normal. In environments where the stack can be
# set at runtime, -bigstack sets a gigantic stack.
#
# Special cases where only one argument is allowed:
#   - If the script is invoked as "RunTest list", a list of available tests is
#     output, but none of them are run.
###############################################################################

import os
import difflib
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

# Define test titles in variables so that they can be output as a list. Some
# of them are modified (e.g. with -8 or -16) when used in the actual tests.

title0="Test 0: Unchecked pcre2test argument tests (to improve coverage)"
title1="Test 1: Main non-UTF, non-UCP functionality (compatible with Perl >= 5.10)"
title2="Test 2: API, errors, internals and non-Perl stuff"
title3="Test 3: Locale-specific features"
title4A="Test 4: UTF"
title4B=" and Unicode property support (compatible with Perl >= 5.10)"
title5A="Test 5: API, internals, and non-Perl stuff for UTF"
title5B=" and UCP support"
title6="Test 6: DFA matching main non-UTF, non-UCP functionality"
title7A="Test 7: DFA matching with UTF"
title7B=" and Unicode property support"
title8="Test 8: Internal offsets and code size tests"
title9="Test 9: Specials for the basic 8-bit library"
title10="Test 10: Specials for the 8-bit library with UTF-8 and UCP support"
title11="Test 11: Specials for the basic 16-bit and 32-bit libraries"
title12="Test 12: Specials for the 16-bit and 32-bit libraries UTF and UCP support"
title13="Test 13: DFA specials for the basic 16-bit and 32-bit libraries"
title14="Test 14: DFA specials for UTF and UCP support"
title15="Test 15: Non-JIT limits and other non-JIT tests"
title16="Test 16: JIT-specific features when JIT is not available"
title17="Test 17: JIT-specific features when JIT is available"
title18="Test 18: Tests of the POSIX interface, excluding UTF/UCP"
title19="Test 19: Tests of the POSIX interface with UTF/UCP"
title20="Test 20: Serialization and code copy tests"
title21=r"Test 21: \C tests without UTF (supported for DFA matching)"
title22=r"Test 22: \C tests with UTF (not supported for DFA matching)"
title23=r"Test 23: \C disabled test"
title24="Test 24: Non-UTF pattern conversion tests"
title25="Test 25: UTF pattern conversion tests"
title26="Test 26: Unicode property tests (compatible with Perl >= 5.38)"
title27="Test 27: Auto-generated unicode property tests"
title28="Test 28: EBCDIC-specific tests"
title29="Test 29: EBCDIC-specific tests (for NL=0x25)"
maxtest=29
titleheap="Test 'heap': Environment-specific heap tests"

if len(sys.argv) == 2 and sys.argv[1] == "list":
  for title in (title0, title1, title2 + " (not UTF or UCP)", title3, title4A + title4B, title5A + title5B, title6, title7A + title7B, title8, title9, title10, title11, title12, title13, title14, title15, title16, title17, title18, title19, title20, title21, title22, title23, title24, title25, title26, title27, title28, title29): print(title)
  print(); print(titleheap); print(); print("Numbered tests are automatically run if nothing selected."); print("Named tests must be explicitly selected.")
  sys.exit(0)

# Find the test data

srcdir=os.environ.get("srcdir", "")
if srcdir and Path(srcdir).is_dir(): testdata=Path(srcdir) / "testdata"
elif Path("testdata").is_dir(): testdata=Path("testdata")
elif Path("../testdata").is_dir(): testdata=Path("../testdata")
else: print("Cannot find the testdata directory"); sys.exit(1)

yield_=0

# ------ Function to check results of a test -------

# This function is called with four parameters:
#
#  returncode  the return code after a call to pcre2test
#  suffix      the suffix of the output file to compare with
#  opts        the pcre2test option arguments (empty, -jit, or -dfa)
#  bits        the library code unit width

def invoke(*args, stdin=None, stdout=None, stderr=None, use_valgrind=True, use_vjs=False):
  command=[str(arg) for arg in args]
  if valgrind and use_valgrind:
    executable=command.index(str(pcre2test))
    command[executable:executable]=valgrind + (vjs if use_vjs else [])
  return subprocess.run(command, stdin=stdin, stdout=stdout, stderr=stderr, check=False).returncode

def compare(first, second):
  try:
    with open(first, "rb") as first_file, open(second, "rb") as second_file:
      while True:
        first_bytes=first_file.read(65536); second_bytes=second_file.read(65536)
        if first_bytes != second_bytes: return False
        if not first_bytes: return True
  except OSError: return False

def checkresult(returncode, suffix, opts, bits):
  global yield_
  opt="".join(opts)
  output=Path(f"testoutput{bits}{opt}") / f"testoutput{suffix}"
  if returncode != 0: print(f"** pcre2test failed - check {output}"); yield_=1; return
  with_=" with JIT" if opt == "-jit" else " with DFA" if opt == "-dfa" else ""
  expected=testdata / f"testoutput{suffix}"
  if ebcdic == 1 or utf == 0:
    # We currently only use the #if ... #endif support in pcre2test for tests
    # which are excluded in certain builds:
    #   - EBCDIC
    #   - No-UTF
    # Run in "preprocess-only" mode (-E) on the testoutput file to trim
    # the output lines matching the input lines which are discarded.
    expected_output=Path(f"testoutput{bits}{opt}") / f"testoutput{suffix}-expected"
    with open(expected_output, "wb") as stream: invoke(*sim, pcre2test, "-q", "-E", expected, stdout=stream, use_valgrind=False)
    expected=expected_output
  if not compare(expected, output):
    try:
      print("".join(difflib.unified_diff(expected.read_bytes().decode("latin-1").splitlines(keepends=True), output.read_bytes().decode("latin-1").splitlines(keepends=True), fromfile=str(expected), tofile=str(output))), end="")
    except OSError: pass
    print(); print(f"** Test {suffix} failed{with_}"); yield_=1; return
  print(f"  OK{with_}")

# ------ Test setup ------

arg8=arg16=arg32=nojit=bigstack=malloc=False
sim=[]; skip=[]; valgrind=[]; vjs=[]; globalopts=["-q"]
pcre2test=os.environ.get("pcre2test", "./pcre2test")
if os.name == "nt" and pcre2test == "./pcre2test" and Path("./pcre2test.exe").is_file(): pcre2test="./pcre2test.exe"
if not Path(pcre2test).is_file() or not os.access(pcre2test, os.X_OK): print(f"** {pcre2test} does not exist or is not executable."); sys.exit(1)

do=[False] * 30; doheap=False; arguments=iter(sys.argv[1:])
for argument in arguments:
  if argument.isdecimal() and int(argument) <= maxtest: do[int(argument)]=True
  elif argument == "heap": doheap=True
  elif argument == "-8": arg8=True
  elif argument == "-16": arg16=True
  elif argument == "-32": arg32=True
  elif argument in ("bigstack", "-bigstack"): bigstack=True
  elif argument in ("malloc", "-malloc"): malloc=True
  elif argument in ("nojit", "-nojit"): nojit=True
  elif argument in ("sim", "-sim"):
    try: sim=shlex.split(next(arguments))
    except StopIteration: print(f"Missing argument after '{argument}'"); sys.exit(1)
  elif argument in ("valgrind", "-valgrind"): valgrind=["valgrind", "--tool=memcheck", "-q", "--leak-check=yes", "--errors-for-leak-kinds=all", "--smc-check=all-non-file", "--error-exitcode=70"]
  elif argument in ("valgrind-log", "-valgrind-log"): valgrind=["valgrind", "--tool=memcheck", "--num-callers=30", "--leak-check=yes", "--errors-for-leak-kinds=all", "--error-limit=no", "--smc-check=all-non-file", "--log-file=report.%p"]
  elif re.fullmatch(r"~[0-9]+", argument): skip.append(int(argument[1:]))
  elif match := re.fullmatch(r"([0-9]+)-([0-9]*)", argument):
    first=int(match.group(1)); last=int(match.group(2) or maxtest)
    if first > maxtest or last > maxtest: print(f"Invalid test range '{argument}'"); sys.exit(1)
    for number in range(first, last + 1): do[number]=True
  else: print(f"Unknown option or test selector '{argument}'"); sys.exit(1)

def capability(name):
  with open(os.devnull, "wb") as null: return invoke(*sim, pcre2test, "-C", name, stdout=null, stderr=null, use_valgrind=False)

with open(os.devnull, "wb") as null: support_setstack=invoke(*sim, pcre2test, "-S", "32", os.devnull, os.devnull, stdout=null, stderr=null, use_valgrind=False)
support8=capability("pcre2-8"); support16=capability("pcre2-16"); support32=capability("pcre2-32")
supportBSC=capability("backslash-C")
ebcdic=capability("ebcdic"); ebcdic_io=capability("ebcdic-io"); ebcdic_nl25=capability("ebcdic-nl25")
utf=capability("unicode"); jit=capability("jit")
if ebcdic == 1:
  if ebcdic_io == 0: print("Running tests in EBCDIC mode, and expecting ASCII test data")
  else:
    print("Running tests in EBCDIC mode, and expecting EBCDIC test data")
    print("If you are on an EBCDIC machine, you will need to convert the PCRE2")
    print("testdata/ directory from ISO8859-1, so the data match the EBCDIC")
    print("codepage that your C compiler is using for C character literals.")
    print("For example:")
    print("  iconv -f ISO8859-1 -t IBM-1047 ...")
if support_setstack == 0 and bigstack: globalopts.extend(("-S", "32"))
if malloc: globalopts.append("-malloc")
if arg8 and support8 == 0: print("Cannot run 8-bit library tests: 8-bit library not compiled"); sys.exit(1)
if arg16 and support16 == 0: print("Cannot run 16-bit library tests: 16-bit library not compiled"); sys.exit(1)
if arg32 and support32 == 0: print("Cannot run 32-bit library tests: 32-bit library not compiled"); sys.exit(1)
test8="-8" if support8 != 0 and (not (arg8 or arg16 or arg32) or arg8) else "skip"; test16="-16" if support16 != 0 and (not (arg8 or arg16 or arg32) or arg16) else "skip"; test32="-32" if support32 != 0 and (not (arg8 or arg16 or arg32) or arg32) else "skip"
jitopts=[]
if jit != 0 and not nojit: jitopts=[["-jit"]]
if jitopts and valgrind: vjs=[f"--suppressions={testdata / 'valgrind-jit.supp'}"]
if not any(do) and not doheap: do=[True] * 30
for number in skip:
  if number <= maxtest: do[number]=False

print(); print(f"PCRE2 C library tests using test data from {testdata}", flush=True); invoke(*sim, pcre2test, os.devnull, use_valgrind=False); print()

# ------ Normal Tests ------

for bmode in (test8, test16, test32):
  if bmode == "skip": continue
  bits=bmode[1:]
  if bmode == "-16" and test8 != "skip" or bmode == "-32" and (test8 != "skip" or test16 != "skip"): print()
  if bmode in ("-16", "-32"): print(f"---- Testing {bits}-bit library ----"); print()
  if bmode == "-8": print("---- Testing 8-bit library ----"); print()
  for opts in [[], *jitopts, ["-dfa"]]: Path(f"testoutput{bits}{''.join(opts)}").mkdir(exist_ok=True)

  # Test 0 is a special test. Its output is not checked, because it will
  # be different on different hardware and with different configurations.
  # Running this test just exercises the code.

  if do[0]:
    print(title0)
    Path("testSinput").write_bytes(b"/abc/jit,memory,framesize\n   abc\n")
    Path("testSoutput").write_bytes(b"\n")
    saverc=0
    def checkspecial(arguments, expect=0, stderr=subprocess.STDOUT, use_vjs=False):
      global yield_
      with open("testSoutput", "ab") as stream: returncode=invoke(*sim, pcre2test, *arguments, stdout=stream, stderr=stderr, use_vjs=use_vjs)
      if returncode != expect: print(f"** pcre2test {' '.join(arguments)} failed - check testSoutput"); yield_=1; return 1
      return 0
    saverc=checkspecial((bmode, "-C")) or saverc
    saverc=checkspecial(("--help",)) or saverc
    saverc=checkspecial((bmode, "testSinput"), use_vjs=True) or saverc
    saverc=checkspecial((bmode, str(testdata / "testinputheap"))) or saverc
    if support_setstack == 0: saverc=checkspecial((bmode, "-S", "1", "-t", "10", "testSinput"), use_vjs=True) or saverc
    saverc=checkspecial((bmode, "reallydoesnotexist"), 1) or saverc
    saverc=checkspecial((bmode, "testSinput", "reallydoesnotexist/outfile"), 1, use_vjs=True) or saverc
    saverc=checkspecial((bmode, "-pattern", "debug", "testSinput"), use_vjs=True) or saverc
    saverc=checkspecial((bmode, "-pattern", "INVALID", "testSinput"), 1, subprocess.DEVNULL) or saverc
    saverc=checkspecial((bmode, "-subject", "notempty", "testSinput"), use_vjs=True) or saverc
    saverc=checkspecial((bmode, "-subject", "INVALID", "testSinput"), 1, subprocess.DEVNULL) or saverc
    saverc=checkspecial(("-LM",)) or saverc
    saverc=checkspecial(("-LP",)) or saverc
    saverc=checkspecial(("-LS",)) or saverc
    saverc=checkspecial((bmode, "-unittest"), use_vjs=True) or saverc
    if saverc == 0: print("  OK")

  # Primary non-UTF test, compatible with JIT and all versions of Perl >= 5.8

  if do[1]:
    print(title1)
    for opts in [[], *jitopts]:
      output=Path(f"testoutput{bits}{''.join(opts)}") / "testoutput1"
      checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput1", output, use_vjs=opts == ["-jit"]), "1", opts, bits)

  # PCRE2 tests that are not Perl-compatible: API, errors, internals. We copy
  # the testbtables file to the current directory for use by this test.

  if do[2]:
    print(f"{title2} (excluding UTF-{bits})")
    shutil.copyfile(testdata / "testbtables", "testbtables")
    for opts in [[], *jitopts]:
      output=Path(f"testoutput{bits}{''.join(opts)}") / "testoutput2"
      saverc=invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput2", output, use_vjs=opts == ["-jit"])
      if saverc == 0:
        with open(output, "ab") as stream: saverc=invoke(*sim, pcre2test, *globalopts, bmode, *opts, "-error", "-80,-62,-2,-1,0,100,101,191,300", stdout=stream, use_vjs=opts == ["-jit"])
        checkresult(saverc, "2", opts, bits)
      else: checkresult(saverc, "2", opts, bits)

  # Locale-specific tests. Unfortunately, different versions of the French
  # locale give different outputs for some items. This test passes if the
  # output matches any one of the alternative output files.

  if do[3]:
    locale=""
    locales=("french", "fr_FR", "fr", "fr_CA")
    if shutil.which("locale") is None: available_locales=[]
    else: available_locales=subprocess.run(["locale", "-a"], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, check=False).stdout.decode("latin-1").splitlines()
    for loc in locales:
      if loc not in available_locales: continue
      probe=subprocess.run([*sim, *valgrind, pcre2test, "-q", bmode], input=f"/a/locale={loc}\n".encode("latin-1"), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, check=False)
      if b"Failed to set locale" in probe.stdout: continue
      locale=loc
      Path("test3input").write_bytes((testdata / "testinput3").read_bytes().replace(b"fr_FR", loc.encode("latin-1")))
      Path("test3output").write_bytes((testdata / "testoutput3").read_bytes().replace(b"fr_FR", loc.encode("latin-1")))
      Path("test3outputA").write_bytes((testdata / "testoutput3A").read_bytes().replace(b"fr_FR", loc.encode("latin-1")))
      Path("test3outputB").write_bytes((testdata / "testoutput3B").read_bytes().replace(b"fr_FR", loc.encode("latin-1")))
      Path("test3outputC").write_bytes((testdata / "testoutput3C").read_bytes().replace(b"fr_FR", loc.encode("latin-1")))
      Path("test3outputD").write_bytes((testdata / "testoutput3D").read_bytes().replace(b"fr_FR", loc.encode("latin-1")))
      break
    if locale:
      print(f"{title3} (using '{locale}' locale)")
      for opts in [[], *jitopts]:
        output=Path(f"testoutput{bits}") / "testoutput3"
        saverc=invoke(*sim, pcre2test, *globalopts, bmode, *opts, "test3input", output, use_vjs=opts == ["-jit"])
        with_=" with JIT" if opts == ["-jit"] else ""
        if saverc == 0 and any(compare(expected, output) for expected in (Path("test3output"), Path("test3outputA"), Path("test3outputB"), Path("test3outputC"), Path("test3outputD"))): print(f"  OK{with_}")
        elif saverc != 0: print(f"** pcre2test failed - check {output}"); yield_=1
        else: print(); print(f"** Locale test did not run successfully{with_}. The output did not match"); print("   test3output, test3outputA, test3outputB, test3outputC, or test3outputD."); print("   This may mean that there is a problem with the locale settings rather"); print("   than a bug in PCRE2."); yield_=1
    else:
      print("Cannot test locale-specific features - none of the French locales")
      print(f"({' '.join(locales)}) could be set.")
      print(" ")

  # Tests for UTF and Unicode property support

  if do[4]:
    print(f"{title4A}-{bits}{title4B}")
    if utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      for opts in [[], *jitopts]:
        output=Path(f"testoutput{bits}{''.join(opts)}") / "testoutput4"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput4", output, use_vjs=opts == ["-jit"]), "4", opts, bits)

  if do[5]:
    print(f"{title5A}-{bits}{title5B}")
    if utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      for opts in [[], *jitopts]:
        output=Path(f"testoutput{bits}{''.join(opts)}") / "testoutput5"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput5", output, use_vjs=opts == ["-jit"]), "5", opts, bits)

  # Tests for DFA matching support

  if do[6]:
    print(title6)
    output=Path(f"testoutput{bits}") / "testoutput6"
    checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput6", output), "6", [], bits)

  if do[7]:
    print(f"{title7A}-{bits}{title7B}")
    if utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      output=Path(f"testoutput{bits}") / "testoutput7"
      checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput7", output), "7", [], bits)

  # Test of internal offsets and code sizes. This test is run only when there
  # is UTF/UCP support. The actual tests are mostly the same as in some of the
  # above, but in this test we inspect some offsets and sizes. This is a
  # doublecheck for the maintainer, just in case something changes unexpectedly.

  if do[8]:
    print(title8)
    with open(os.devnull, "wb") as null: bits_link_size=invoke(*sim, pcre2test, f"-{bits}", "-C", "linksize", stdout=null, stderr=null)
    if utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      suffix=f"8-{bits}-{bits_link_size}"
      output=Path(f"testoutput{bits}") / f"testoutput{suffix}"
      checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput8", output), suffix, [], bits)

  # Tests for 8-bit-specific features

  if do[9]:
    print(title9)
    if bits in ("16", "32"): print("  Skipped when running 16/32-bit tests")
    else:
      for opts in [[], *jitopts]:
        output=Path(f"testoutput{bits}{''.join(opts)}") / "testoutput9"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput9", output, use_vjs=opts == ["-jit"]), "9", opts, bits)

  # Tests for UTF-8 and UCP 8-bit-specific features

  if do[10]:
    print(title10)
    if bits in ("16", "32"): print("  Skipped when running 16/32-bit tests")
    elif utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      for opts in [[], *jitopts]:
        output=Path(f"testoutput{bits}{''.join(opts)}") / "testoutput10"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput10", output, use_vjs=opts == ["-jit"]), "10", opts, bits)

  # Tests for 16-bit and 32-bit features. Output is different for the two widths.

  if do[11]:
    print(title11)
    if bits == "8": print("  Skipped when running 8-bit tests")
    else:
      for opts in [[], *jitopts]:
        suffix=f"11-{bits}"
        output=Path(f"testoutput{bits}{''.join(opts)}") / f"testoutput{suffix}"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput11", output, use_vjs=opts == ["-jit"]), suffix, opts, bits)

  # Tests for 16-bit and 32-bit features with UTF-16/32 and UCP support. Output
  # is different for the two widths.

  if do[12]:
    print(title12)
    if bits == "8": print("  Skipped when running 8-bit tests")
    elif utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      for opts in [[], *jitopts]:
        suffix=f"12-{bits}"
        output=Path(f"testoutput{bits}{''.join(opts)}") / f"testoutput{suffix}"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput12", output, use_vjs=opts == ["-jit"]), suffix, opts, bits)

  # Tests for 16/32-bit-specific features in DFA non-UTF modes

  if do[13]:
    print(title13)
    if bits == "8": print("  Skipped when running 8-bit tests")
    else:
      output=Path(f"testoutput{bits}") / "testoutput13"
      checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput13", output), "13", [], bits)

  # Tests for DFA UTF and UCP features. Output is different for the different widths.

  if do[14]:
    print(title14)
    if utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      suffix=f"14-{bits}"
      output=Path(f"testoutput{bits}") / f"testoutput{suffix}"
      checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput14", output), suffix, [], bits)

  # Test non-JIT match and recursion limits

  if do[15]:
    print(title15)
    output=Path(f"testoutput{bits}") / "testoutput15"
    checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput15", output), "15", [], bits)

  # Test JIT-specific features when JIT is not available

  if do[16]:
    print(title16)
    if jit != 0: print("  Skipped because JIT is available")
    else:
      output=Path(f"testoutput{bits}") / "testoutput16"
      checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput16", output), "16", [], bits)

  # Test JIT-specific features when JIT is available

  if do[17]:
    print(title17)
    if jit == 0 or nojit: print("  Skipped because JIT is not available or nojit was specified")
    else:
      output=Path(f"testoutput{bits}") / "testoutput17"
      checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput17", output, use_vjs=True), "17", [], bits)

  # Tests for the POSIX interface without UTF/UCP (8-bit only)

  if do[18]:
    print(title18)
    if bits in ("16", "32"): print("  Skipped when running 16/32-bit tests")
    else:
      output=Path(f"testoutput{bits}") / "testoutput18"
      checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput18", output), "18", [], bits)

  # Tests for the POSIX interface with UTF/UCP (8-bit only)

  if do[19]:
    print(title19)
    if bits in ("16", "32"): print("  Skipped when running 16/32-bit tests")
    elif utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      output=Path(f"testoutput{bits}") / "testoutput19"
      checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput19", output), "19", [], bits)

  # Serialization tests

  if do[20]:
    print(title20)
    output=Path(f"testoutput{bits}") / "testoutput20"
    checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput20", output), "20", [], bits)

  # \C tests without UTF - DFA matching is supported

  if do[21]:
    print(title21)
    if supportBSC == 0: print(r"  Skipped because \C is disabled")
    else:
      for opts in [[], *jitopts, ["-dfa"]]:
        output=Path(f"testoutput{bits}{''.join(opts)}") / "testoutput21"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput21", output, use_vjs=opts == ["-jit"]), "21", opts, bits)

  # \C tests with UTF - DFA matching is not supported for \C in UTF mode

  if do[22]:
    print(title22)
    if supportBSC == 0: print(r"  Skipped because \C is disabled")
    elif utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      for opts in [[], *jitopts]:
        suffix=f"22-{bits}"
        output=Path(f"testoutput{bits}{''.join(opts)}") / f"testoutput{suffix}"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput22", output, use_vjs=opts == ["-jit"]), suffix, opts, bits)

  # Test when \C is disabled

  if do[23]:
    print(title23)
    if supportBSC != 0: print(r"  Skipped because \C is not disabled")
    else:
      output=Path(f"testoutput{bits}") / "testoutput23"
      checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput23", output), "23", [], bits)

  # Non-UTF pattern conversion tests

  if do[24]:
    print(title24)
    output=Path(f"testoutput{bits}") / "testoutput24"
    checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput24", output), "24", [], bits)

  # UTF pattern conversion tests

  if do[25]:
    print(title25)
    if utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      output=Path(f"testoutput{bits}") / "testoutput25"
      checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinput25", output), "25", [], bits)

  # Unicode property tests

  if do[26]:
    print(title26)
    if utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      for opts in [[], *jitopts]:
        output=Path(f"testoutput{bits}{''.join(opts)}") / "testoutput26"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput26", output, use_vjs=opts == ["-jit"]), "26", opts, bits)

  # Auto-generated Unicode property tests

  if do[27]:
    print(title27)
    if utf == 0: print(f"  Skipped because UTF-{bits} support is not available")
    else:
      for opts in [[], *jitopts]:
        output=Path(f"testoutput{bits}{''.join(opts)}") / "testoutput27"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput27", output, use_vjs=opts == ["-jit"]), "27", opts, bits)

  # EBCDIC tests

  if do[28]:
    print(title28)
    if ebcdic == 0: print("  Skipped when not targetting EBCDIC")
    else:
      for opts in [[], *jitopts, ["-dfa"]]:
        output=Path(f"testoutput{bits}{''.join(opts)}") / "testoutput28"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput28", output, use_vjs=opts == ["-jit"]), "28", opts, bits)

  # EBCDIC tests (for NL=0x25)

  if do[29]:
    print(title29)
    if ebcdic == 0: print("  Skipped when not targetting EBCDIC")
    elif ebcdic_nl25 == 0: print("  Skipped because EBCDIC newline is not 0x25")
    else:
      for opts in [[], *jitopts, ["-dfa"]]:
        output=Path(f"testoutput{bits}{''.join(opts)}") / "testoutput29"
        checkresult(invoke(*sim, pcre2test, *globalopts, bmode, *opts, testdata / "testinput29", output, use_vjs=opts == ["-jit"]), "29", opts, bits)

  # Manually selected heap tests - output may vary in different environments,
  # which is why that are not automatically run.

  if doheap:
    print(titleheap)
    suffix=f"heap-{bits}"
    output=Path(f"testoutput{bits}") / f"testoutput{suffix}"
    checkresult(invoke(*sim, pcre2test, *globalopts, bmode, testdata / "testinputheap", output), suffix, [], bits)

if yield_ == 0:
  print(); print("All tests passed.")
  for name in ("testbtables", "testSinput", "testSoutput", "test3input", "test3output", "test3outputA", "test3outputB", "test3outputC", "test3outputD", "testsaved1", "testsaved2", "teststdout", "teststderr"):
    Path(name).unlink(missing_ok=True)
  for cleanup_bits in ("8", "16", "32"):
    shutil.rmtree(f"testoutput{cleanup_bits}", ignore_errors=True)
    shutil.rmtree(f"testoutput{cleanup_bits}-jit", ignore_errors=True)
    shutil.rmtree(f"testoutput{cleanup_bits}-dfa", ignore_errors=True)
else: print(); print("** Tests failed. See output above for details.")
sys.exit(yield_)