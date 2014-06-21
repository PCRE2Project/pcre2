## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4


## Specify the documentation files that are distributed.

# FIXME
#dist_doc_DATA = \
#  doc/pcre.txt \
#  doc/pcre-config.txt \
#  doc/pcregrep.txt \
#  doc/pcretest.txt \
#  AUTHORS \
#  COPYING \
#  ChangeLog \
#  LICENCE \
#  NEWS \
#  README

# FIXME
#dist_html_DATA = \
#  doc/html/NON-AUTOTOOLS-BUILD.txt \
#  doc/html/README.txt \
#  doc/html/index.html \
#  doc/html/pcre-config.html \
#  doc/html/pcre.html \
#  doc/html/pcre16.html \
#  doc/html/pcre32.html \
#  doc/html/pcre_assign_jit_stack.html \
#  doc/html/pcre_compile.html \
#  doc/html/pcre_compile2.html \
#  doc/html/pcre_config.html \
#  doc/html/pcre_copy_named_substring.html \
#  doc/html/pcre_copy_substring.html \
#  doc/html/pcre_dfa_match.html \
#  doc/html/pcre_match.html \
#  doc/html/pcre_free_study.html \
#  doc/html/pcre_free_substring.html \
#  doc/html/pcre_free_substring_list.html \
#  doc/html/pcre_fullinfo.html \
#  doc/html/pcre_get_named_substring.html \
#  doc/html/pcre_get_stringnumber.html \
#  doc/html/pcre_get_stringtable_entries.html \
#  doc/html/pcre_get_substring.html \
#  doc/html/pcre_get_substring_list.html \
#  doc/html/pcre_jit_match.html \
#  doc/html/pcre_jit_stack_alloc.html \
#  doc/html/pcre_jit_stack_free.html \
#  doc/html/pcre_maketables.html \
#  doc/html/pcre_pattern_to_host_byte_order.html \
#  doc/html/pcre_refcount.html \
#  doc/html/pcre_study.html \
#  doc/html/pcre_utf16_to_host_byte_order.html \
#  doc/html/pcre_utf32_to_host_byte_order.html \
#  doc/html/pcre_version.html \
#  doc/html/pcreapi.html \
#  doc/html/pcrebuild.html \
#  doc/html/pcrecallout.html \
#  doc/html/pcrecompat.html \
#  doc/html/pcredemo.html \
#  doc/html/pcregrep.html \
#  doc/html/pcrejit.html \
#  doc/html/pcrelimits.html \
#  doc/html/pcrematching.html \
#  doc/html/pcrepartial.html \
#  doc/html/pcrepattern.html \
#  doc/html/pcreperform.html \
#  doc/html/pcreposix.html \
#  doc/html/pcreprecompile.html \
#  doc/html/pcresample.html \
#  doc/html/pcrestack.html \
#  doc/html/pcresyntax.html \
#  doc/html/pcretest.html \
#  doc/html/pcreunicode.html

# FIXME
#dist_man_MANS = \
#  doc/pcre2-config.1 \
#  doc/pcre2.3 \
#  doc/pcre2-16.3 \
#  doc/pcre2-32.3 \
#  doc/pcre2_assign_jit_stack.3 \
#  doc/pcre2_compile.3 \
#  doc/pcre2_compile2.3 \
#  doc/pcre2_config.3 \
#  doc/pcre2_copy_named_substring.3 \
#  doc/pcre2_copy_substring.3 \
#  doc/pcre2_dfa_match.3 \
#  doc/pcre2_match.3 \
#  doc/pcre2_free_study.3 \
#  doc/pcre2_free_substring.3 \
#  doc/pcre2_free_substring_list.3 \
#  doc/pcre2_fullinfo.3 \
#  doc/pcre2_get_named_substring.3 \
#  doc/pcre2_get_stringnumber.3 \
#  doc/pcre2_get_stringtable_entries.3 \
#  doc/pcre2_get_substring.3 \
#  doc/pcre2_get_substring_list.3 \
#  doc/pcre2_jit_match.3 \
#  doc/pcre2_jit_stack_alloc.3 \
#  doc/pcre2_jit_stack_free.3 \
#  doc/pcre2_maketables.3 \
#  doc/pcre2_pattern_to_host_byte_order.3 \
#  doc/pcre2_refcount.3 \
#  doc/pcre2_study.3 \
#  doc/pcre2_utf16_to_host_byte_order.3 \
#  doc/pcre2_utf32_to_host_byte_order.3 \
#  doc/pcre2_version.3 \
#  doc/pcre2api.3 \
#  doc/pcre2build.3 \
#  doc/pcre2callout.3 \
#  doc/pcre2compat.3 \
#  doc/pcre2demo.3 \
#  doc/pcre2grep.1 \
#  doc/pcre2jit.3 \
#  doc/pcre2limits.3 \
#  doc/pcre2matching.3 \
#  doc/pcre2partial.3 \
#  doc/pcre2pattern.3 \
#  doc/pcre2perform.3 \
#  doc/pcre2posix.3 \
#  doc/pcre2precompile.3 \
#  doc/pcre2sample.3 \
#  doc/pcre2stack.3 \
#  doc/pcre2syntax.3 \
#  doc/pcre2test.1 \
#  doc/pcre2unicode.3

# The Libtool libraries to install.  We'll add to this later.

lib_LTLIBRARIES =

# Unit tests you want to run when people type 'make check'.
# TESTS is for binary unit tests, check_SCRIPTS for script-based tests

TESTS =
check_SCRIPTS =
dist_noinst_SCRIPTS =

# Some of the binaries we make are to be installed, and others are
# (non-user-visible) helper programs needed to build the libraries.

bin_PROGRAMS =
noinst_PROGRAMS =

# Additional files to delete on 'make clean' and 'make maintainer-clean'.

CLEANFILES =
MAINTAINERCLEANFILES =

# Additional files to bundle with the distribution, over and above what
# the Autotools include by default.

EXTRA_DIST =

# These files contain additional m4 macros that are used by autoconf.

EXTRA_DIST += \
  m4/ax_pthread.m4 m4/pcre_visibility.m4

# These files contain maintenance information

EXTRA_DIST += \
  doc/perltest.txt \
  NON-UNIX-USE \
  NON-AUTOTOOLS-BUILD \
  HACKING

# These files are used in the preparation of a release

EXTRA_DIST += \
  PrepareRelease \
  CheckMan \
  CleanTxt \
  Detrail \
  132html \
  doc/index.html.src

# These files are usable versions of pcre2.h and config.h that are distributed
# for the benefit of people who are building PCRE manually, without the
# Autotools support.

EXTRA_DIST += \
  src/pcre2.h.generic \
  src/config.h.generic

# The only difference between pcre2.h.in and pcre2.h is the setting of the PCRE
# version number. Therefore, we can create the generic version just by copying.

src/pcre2.h.generic: src/pcre2.h.in configure.ac
	rm -f $@
	cp -p src/pcre2.h $@

# It is more complicated for config.h.generic. We need the version that results
# from a default configuration so as to get all the default values for PCRE
# configuration macros such as MATCH_LIMIT and NEWLINE. We can get this by
# doing a configure in a temporary directory. However, some trickery is needed,
# because the source directory may already be configured. If you just try
# running configure in a new directory, it complains. For this reason, we move
# config.status out of the way while doing the default configuration. The
# resulting config.h is munged by perl to put #ifdefs round any #defines for
# macros with values, and to #undef all boolean macros such as HAVE_xxx and
# SUPPORT_xxx. We also get rid of any gcc-specific visibility settings. Make
# sure that PCRE2_EXP_DEFN is unset (in case it has visibility settings).

src/config.h.generic: configure.ac
	rm -rf $@ _generic
	mkdir _generic
	cs=$(srcdir)/config.status; test ! -f $$cs || mv -f $$cs $$cs.aside
	cd _generic && $(abs_top_srcdir)/configure || :
	cs=$(srcdir)/config.status; test ! -f $$cs.aside || mv -f $$cs.aside $$cs
	test -f _generic/src/config.h
	perl -n \
	  -e 'BEGIN{$$blank=0;}' \
	  -e 'if(/PCRE2_EXP_DEFN/){print"/* #undef PCRE2_EXP_DEFN */\n";$$blank=0;next;}' \
	  -e 'if(/to make a symbol visible/){next;}' \
	  -e 'if(/__attribute__ \(\(visibility/){next;}' \
	  -e 'if(/LT_OBJDIR/){print"/* This is ignored unless you are using libtool. */\n";}' \
	  -e 'if(/^#define\s((?:HAVE|SUPPORT|STDC)_\w+)/){print"/* #undef $$1 */\n";$$blank=0;next;}' \
	  -e 'if(/^#define\s(?!PACKAGE|VERSION)(\w+)/){print"#ifndef $$1\n$$_#endif\n";$$blank=0;next;}' \
	  -e 'if(/^\s*$$/){print unless $$blank; $$blank=1;} else{print;$$blank=0;}' \
	  _generic/src/config.h >$@
	rm -rf _generic

MAINTAINERCLEANFILES += src/pcre2.h.generic src/config.h.generic

# These are the header files we'll install. We do not distribute pcre2.h
# because it is generated from pcre2.h.in.

nodist_include_HEADERS = src/pcre2.h
include_HEADERS = src/pcre2posix.h

# This is the "config" script.

bin_SCRIPTS = pcre2-config

## ---------------------------------------------------------------
## The dftables program is used to rebuild character tables before compiling
## PCRE, if --enable-rebuild-chartables is specified. It is not a user-visible
## program. The default (when --enable-rebuild-chartables is not specified) is
## to copy a distributed set of tables that are defined for ASCII code. In this
## case, dftables is not needed.

if WITH_REBUILD_CHARTABLES
noinst_PROGRAMS += dftables
dftables_SOURCES = dftables.c
src/pcre2_chartables.c: dftables$(EXEEXT)
	./dftables$(EXEEXT) $@
else
src/pcre2_chartables.c: $(srcdir)/src/pcre2_chartables.c.dist
	rm -f $@
	$(LN_S) $(srcdir)/pcre2_chartables.c.dist $@
endif # WITH_REBUILD_CHARTABLES

BUILT_SOURCES = src/pcre2_chartables.c
NODIST_SOURCES = src/pcre2_chartables.c

## Define the list of common sources, then arrange to build whichever of the
## 8-, 16-, or 32-bit libraries are configured.

COMMON_SOURCES = \
  src/pcre2_auto_possess.c \
  src/pcre2_byte_order.c \
  src/pcre2_compile.c \
  src/pcre2_config.c \
  src/pcre2_context.c \
  src/pcre2_dfa_match.c \
  src/pcre2_error.c \
  src/pcre2_match.c \
  src/pcre2_internal.h \
  src/pcre2_intmodedep.h \
  src/pcre2_jit_compile.c \
  src/pcre2_jit_match.c \
  src/pcre2_jit_misc.c \
  src/pcre2_maketables.c \
  src/pcre2_match_data.c \
  src/pcre2_newline.c \
  src/pcre2_ord2utf.c \
  src/pcre2_pattern_info.c \
  src/pcre2_string_utils.c \
  src/pcre2_study.c \
  src/pcre2_substring.c \
  src/pcre2_tables.c \
  src/pcre2_ucd.c \
  src/pcre2_ucp.h \
  src/pcre2_valid_utf.c \
  src/pcre2_version.c \
  src/pcre2_xclass.c

if WITH_PCRE8
lib_LTLIBRARIES += libpcre2-8.la
libpcre2_8_la_SOURCES = \
  $(COMMON_SOURCES)
nodist_libpcre2_8_la_SOURCES = \
  $(NODIST_SOURCES)
libpcre2_8_la_CFLAGS = \
  -DPCRE2_CODE_UNIT_WIDTH=8 \
  $(VISIBILITY_CFLAGS) \
  $(AM_CFLAGS)
libpcre2_8_la_LIBADD =
endif # WITH_PCRE8

if WITH_PCRE16
lib_LTLIBRARIES += libpcre2-16.la
libpcre2_16_la_SOURCES = \
  $(COMMON_SOURCES)
nodist_libpcre2_16_la_SOURCES = \
  $(NODIST_SOURCES)
libpcre2_16_la_CFLAGS = \
  -DPCRE2_CODE_UNIT_WIDTH=16 \
  $(VISIBILITY_CFLAGS) \
  $(AM_CFLAGS)
libpcre2_16_la_LIBADD =
endif # WITH_PCRE16

if WITH_PCRE32
lib_LTLIBRARIES += libpcre2-32.la
libpcre2_32_la_SOURCES = \
  $(COMMON_SOURCES)
nodist_libpcre2_32_la_SOURCES = \
  $(NODIST_SOURCES)
libpcre2_32_la_CFLAGS = \
  -DPCRE2_CODE_UNIT_WIDTH=32 \
  $(VISIBILITY_CFLAGS) \
  $(AM_CFLAGS)
libpcre2_32_la_LIBADD =
endif # WITH_PCRE32

# The pcre2_chartables.c.dist file is the default version of
# pcre2_chartables.c, used unless --enable-rebuild-chartables is specified.

EXTRA_DIST += src/pcre2_chartables.c.dist
CLEANFILES += src/pcre2_chartables.c

# The JIT compiler lives in a separate directory, but its files are #included
# when pcre2_jit_compile.c is processed, so they must be distributed.

EXTRA_DIST += \
  sljit/sljitConfig.h \
  sljit/sljitConfigInternal.h \
  sljit/sljitExecAllocator.c \
  sljit/sljitLir.c \
  sljit/sljitLir.h \
  sljit/sljitNativeARM_64.c \
  sljit/sljitNativeARM_Thumb2.c \
  sljit/sljitNativeARM_v5.c \
  sljit/sljitNativeMIPS_32.c \
  sljit/sljitNativeMIPS_common.c \
  sljit/sljitNativePPC_32.c \
  sljit/sljitNativePPC_64.c \
  sljit/sljitNativePPC_common.c \
  sljit/sljitNativeSPARC_32.c \
  sljit/sljitNativeSPARC_common.c \
  sljit/sljitNativeTILEGX.c \
  sljit/sljitNativeTILEGX-encoder.c \
  sljit/sljitNativeX86_32.c \
  sljit/sljitNativeX86_64.c \
  sljit/sljitNativeX86_common.c \
  sljit/sljitUtils.c

if WITH_PCRE8
libpcre2_8_la_LDFLAGS = $(EXTRA_LIBPCRE2_8_LDFLAGS)
endif # WITH_PCRE8
if WITH_PCRE16
libpcre2_16_la_LDFLAGS = $(EXTRA_LIBPCRE2_16_LDFLAGS)
endif # WITH_PCRE16
if WITH_PCRE32
libpcre2_32_la_LDFLAGS = $(EXTRA_LIBPCRE2_32_LDFLAGS)
endif # WITH_PCRE32

if WITH_VALGRIND
if WITH_PCRE8
libpcre2_8_la_CFLAGS += $(VALGRIND_CFLAGS)
endif # WITH_PCRE8
if WITH_PCRE16
libpcre2_16_la_CFLAGS += $(VALGRIND_CFLAGS)
endif # WITH_PCRE16
if WITH_PCRE32
libpcre2_32_la_CFLAGS += $(VALGRIND_CFLAGS)
endif # WITH_PCRE32
endif # WITH_VALGRIND

if WITH_GCOV
if WITH_PCRE8
libpcre2_8_la_CFLAGS += $(GCOV_CFLAGS)
endif # WITH_PCRE8
if WITH_PCRE16
libpcre2_16_la_CFLAGS += $(GCOV_CFLAGS)
endif # WITH_PCRE16
if WITH_PCRE32
libpcre2_32_la_CFLAGS += $(GCOV_CFLAGS)
endif # WITH_PCRE32
endif # WITH_GCOV

## A version of the 8-bit library that has a POSIX API.

if WITH_PCRE8
lib_LTLIBRARIES += libpcre2-posix.la
libpcre2_posix_la_SOURCES = src/pcre2posix.c
libpcre2_posix_la_CFLAGS = \
  -DPCRE2_CODE_UNIT_WIDTH=8 \
  $(VISIBILITY_CFLAGS) $(AM_CFLAGS)
libpcre2_posix_la_LDFLAGS = $(EXTRA_LIBPCRE2_POSIX_LDFLAGS)
libpcre2_posix_la_LIBADD = libpcre2-8.la
if WITH_GCOV
libpcre2_posix_la_CFLAGS += $(GCOV_CFLAGS)
endif # WITH_GCOV
endif # WITH_PCRE8

## Build pcre2grep if the 8-bit library is enabled

if WITH_PCRE8
bin_PROGRAMS += pcre2grep
pcre2grep_SOURCES = src/pcre2grep.c
pcre2grep_CFLAGS = $(AM_CFLAGS)
pcre2grep_LDADD = $(LIBZ) $(LIBBZ2)
pcre2grep_LDADD += libpcre2-8.la
if WITH_GCOV
pcre2grep_CFLAGS += $(GCOV_CFLAGS)
pcre2grep_LDADD += $(GCOV_LIBS)
endif # WITH_GCOV
endif # WITH_PCRE8

## -------- Testing ----------

## If JIT support is enabled, arrange for the JIT test program to run.

if WITH_JIT
TESTS += pcre2_jit_test
noinst_PROGRAMS += pcre2_jit_test
pcre2_jit_test_SOURCES = pcre2_jit_test.c
pcre2_jit_test_CFLAGS = $(AM_CFLAGS)
pcre2_jit_test_LDADD =
if WITH_PCRE8
pcre2_jit_test_LDADD += libpcre2-8.la
endif # WITH_PCRE8
if WITH_PCRE16
pcre2_jit_test_LDADD += libpcre2-16.la
endif # WITH_PCRE16
if WITH_PCRE32
pcre2_jit_test_LDADD += libpcre2-32.la
endif # WITH_PCRE32
if WITH_GCOV
pcre2_jit_test_CFLAGS += $(GCOV_CFLAGS)
pcre2_jit_test_LDADD += $(GCOV_LIBS)
endif # WITH_GCOV
endif # WITH_JIT

# Build the general pcre2test program. The file src/pcre2_printint.c is
# #included by pcre2test as many times as needed, at different code unit
# widths.

bin_PROGRAMS += pcre2test
EXTRA_DIST += src/pcre2_printint.c
pcre2test_SOURCES = src/pcre2test.c
pcre2test_CFLAGS = $(AM_CFLAGS)
pcre2test_LDADD = $(LIBREADLINE)

if WITH_PCRE8
pcre2test_LDADD += libpcre2-8.la libpcre2-posix.la
endif # WITH_PCRE8

if WITH_PCRE16
pcre2test_LDADD += libpcre2-16.la
endif # WITH_PCRE16

if WITH_PCRE32
pcre2test_LDADD += libpcre2-32.la
endif # WITH_PCRE32

if WITH_VALGRIND
pcre2test_CFLAGS += $(VALGRIND_CFLAGS)
endif # WITH_VALGRIND

if WITH_GCOV
pcre2test_CFLAGS += $(GCOV_CFLAGS)
pcre2test_LDADD += $(GCOV_LIBS)
endif # WITH_GCOV

## The main library tests. Each test is a binary plus a script that runs that
## binary in various ways. We install these test binaries in case folks find it
## helpful.

# FIXME
#TESTS += RunTest
#dist_noinst_SCRIPTS += RunTest
#EXTRA_DIST += RunTest.bat

## When the 8-bit library is configured, pcre2grep will have been built.

#if WIDH_PCRE8
#TESTS += RunGrepTest
#dist_noinst_SCRIPTS += RunGrepTest
#endif # WITH_PCRE8

## Distribute all the test data files

EXTRA_DIST += \
  testdata/grepbinary \
  testdata/grepfilelist \
  testdata/grepinput \
  testdata/grepinput3 \
  testdata/grepinput8 \
  testdata/grepinputv \
  testdata/grepinputx \
  testdata/greplist \
  testdata/grepoutput \
  testdata/grepoutput8 \
  testdata/grepoutputN \
  testdata/greppatN4 \
  testdata/saved16 \
  testdata/saved16BE-1 \
  testdata/saved16BE-2 \
  testdata/saved16LE-1 \
  testdata/saved16LE-2 \
  testdata/saved32 \
  testdata/saved32BE-1 \
  testdata/saved32BE-2 \
  testdata/saved32LE-1 \
  testdata/saved32LE-2 \
  testdata/saved8 \
  testdata/testinput1 \
  testdata/testinput2 \
  testdata/testinput3 \
  testdata/testinput4 \
  testdata/testinput5 \
  testdata/testinput6 \
  testdata/testinput7 \
  testdata/testinput8 \
  testdata/testinput9 \
  testdata/testinput10 \
  testdata/testinput11 \
  testdata/testinput12 \
  testdata/testinput13 \
  testdata/testinput14 \
  testdata/testinput15 \
  testdata/testinput16 \
  testdata/testinput17 \
  testdata/testinput18 \
  testdata/testinput19 \
  testdata/testinput20 \
  testdata/testinput21 \
  testdata/testinput22 \
  testdata/testinput23 \
  testdata/testinput24 \
  testdata/testinput25 \
  testdata/testinput26 \
  testdata/testinputEBC \
  testdata/testoutput1 \
  testdata/testoutput2 \
  testdata/testoutput3 \
  testdata/testoutput3A \
  testdata/testoutput3B \
  testdata/testoutput4 \
  testdata/testoutput5 \
  testdata/testoutput6 \
  testdata/testoutput7 \
  testdata/testoutput8 \
  testdata/testoutput9 \
  testdata/testoutput10 \
  testdata/testoutput11-8 \
  testdata/testoutput11-16 \
  testdata/testoutput11-32 \
  testdata/testoutput12 \
  testdata/testoutput13 \
  testdata/testoutput14 \
  testdata/testoutput15 \
  testdata/testoutput16 \
  testdata/testoutput17 \
  testdata/testoutput18-16 \
  testdata/testoutput18-32 \
  testdata/testoutput19 \
  testdata/testoutput20 \
  testdata/testoutput21-16 \
  testdata/testoutput21-32 \
  testdata/testoutput22-16 \
  testdata/testoutput22-32 \
  testdata/testoutput23 \
  testdata/testoutput24 \
  testdata/testoutput25 \
  testdata/testoutput26 \
  testdata/testoutputEBC \
  testdata/wintestinput3 \
  testdata/wintestoutput3 \
  perltest.pl

CLEANFILES += \
	testsavedregex \
	teststderr \
        testtemp* \
	testtry \
        testNinput

## ------------ End of testing -------------


# PCRE demonstration program. Not built automatcally. The point is that the
# users should build it themselves. So just distribute the source.

EXTRA_DIST += src/pcre2demo.c


# We have .pc files for pkg-config users.

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =

if WITH_PCRE8
pkgconfig_DATA += libpcre2-8.pc libpcre2-posix.pc
endif

if WITH_PCRE16
pkgconfig_DATA += libpcre2-16.pc
endif

if WITH_PCRE32
pkgconfig_DATA += libpcre2-32.pc
endif


# gcov/lcov code coverage reporting
#
# Coverage reporting targets:
#
# coverage: Create a coverage report from 'make check'
# coverage-baseline: Capture baseline coverage information
# coverage-reset: This zeros the coverage counters only
# coverage-report: This creates the coverage report only
# coverage-clean-report: This removes the generated coverage report
#   without cleaning the coverage data itself
# coverage-clean-data: This removes the captured coverage data without
#   removing the coverage files created at compile time (*.gcno)
# coverage-clean: This cleans all coverage data including the generated
#   coverage report.

if WITH_GCOV
COVERAGE_TEST_NAME = $(PACKAGE)
COVERAGE_NAME = $(PACKAGE)-$(VERSION)
COVERAGE_OUTPUT_FILE = $(COVERAGE_NAME)-coverage.info
COVERAGE_OUTPUT_DIR = $(COVERAGE_NAME)-coverage
COVERAGE_LCOV_EXTRA_FLAGS =
COVERAGE_GENHTML_EXTRA_FLAGS =

coverage_quiet = $(coverage_quiet_$(V))
coverage_quiet_ = $(coverage_quiet_$(AM_DEFAULT_VERBOSITY))
coverage_quiet_0 = --quiet

coverage-check: all
	-$(MAKE) $(AM_MAKEFLAGS) -k check

coverage-baseline:
	$(LCOV) $(coverage_quiet) \
		--directory $(top_builddir) \
		--output-file "$(COVERAGE_OUTPUT_FILE)" \
		--capture \
		--initial

coverage-report:
	$(LCOV) $(coverage_quiet) \
		--directory $(top_builddir) \
		--capture \
		--output-file "$(COVERAGE_OUTPUT_FILE).tmp" \
		--test-name "$(COVERAGE_TEST_NAME)" \
		--no-checksum \
		--compat-libtool \
		$(COVERAGE_LCOV_EXTRA_FLAGS)
	$(LCOV) $(coverage_quiet) \
		--directory $(top_builddir) \
		--output-file "$(COVERAGE_OUTPUT_FILE)" \
		--remove "$(COVERAGE_OUTPUT_FILE).tmp" \
		"/tmp/*" \
		"/usr/include/*" \
		"$(includedir)/*"
	-@rm -f "$(COVERAGE_OUTPUT_FILE).tmp"
	LANG=C $(GENHTML) $(coverage_quiet) \
		--prefix $(top_builddir) \
		--output-directory "$(COVERAGE_OUTPUT_DIR)" \
		--title "$(PACKAGE) $(VERSION) Code Coverage Report" \
		--show-details "$(COVERAGE_OUTPUT_FILE)" \
		--legend \
		$(COVERAGE_GENHTML_EXTRA_FLAGS)
	@echo "Code coverage report written to file://$(abs_builddir)/$(COVERAGE_OUTPUT_DIR)/index.html"

coverage-reset:
	-$(LCOV) $(coverage_quiet) --zerocounters --directory $(top_builddir)

coverage-clean-report:
	-rm -f "$(COVERAGE_OUTPUT_FILE)" "$(COVERAGE_OUTPUT_FILE).tmp"
	-rm -rf "$(COVERAGE_OUTPUT_DIR)"

coverage-clean-data:
	-find $(top_builddir) -name "*.gcda" -delete

coverage-clean: coverage-reset coverage-clean-report coverage-clean-data
	-find $(top_builddir) -name "*.gcno" -delete

coverage-distclean: coverage-clean

coverage: coverage-reset coverage-baseline coverage-check coverage-report
clean-local: coverage-clean
distclean-local: coverage-distclean

.PHONY: coverage coverage-baseline coverage-check coverage-report coverage-reset coverage-clean-report coverage-clean-data coverage-clean coverage-distclean

else

coverage:
	@echo "Configuring with --enable-coverage is required to generate code coverage report."
endif # WITH_GCOV

## CMake support

EXTRA_DIST += \
  cmake/COPYING-CMAKE-SCRIPTS \
  cmake/FindPackageHandleStandardArgs.cmake \
  cmake/FindReadline.cmake \
  cmake/FindEditline.cmake \
  CMakeLists.txt \
  config-cmake.h.in

## end Makefile.am
