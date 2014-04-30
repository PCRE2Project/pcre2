/*************************************************
*             PCRE2 testing program              *
*************************************************/

/* PCRE2 is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language. In 2014
the API was completely revised and '2' was added to the name, because the old
API, which had lasted for 16 years, could not accommodate new requirements. At
the same time, this testing program was re-designed because its original
hacked-up (non-) design had also run out of steam.

                       Written by Philip Hazel
     Original code Copyright (c) 1997-2012 University of Cambridge
         Rewritten code Copyright (c) 2014 University of Cambridge

-----------------------------------------------------------------------------
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of the University of Cambridge nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------
*/

/* This program supports the testing of all of the 8-bit, 16-bit, and 32-bit
PCRE2 libraries in a single program. This is different from modules such as
pcre2_compile.c in the library itself, which are compiled separately for each
mode. If two modes are enabled, for example, pcre2_compile.c is compiled twice.
By contrast, pcre2test.c is compiled only once. Therefore, it must not make use
of any of the macros from pcre2.h or pcre2_internal.h that depend on
PCRE2_CODE_UNIT_WIDTH. It does, however, make use of SUPPORT_PCRE8,
SUPPORT_PCRE16, and SUPPORT_PCRE32 to ensure that it calls only supported
library functions. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <errno.h>

/* Both libreadline and libedit are optionally supported. The user-supplied
original patch uses readline/readline.h for libedit, but in at least one system
it is installed as editline/readline.h, so the configuration code now looks for
that first, falling back to readline/readline.h. */

#if defined(SUPPORT_LIBREADLINE) || defined(SUPPORT_LIBEDIT)
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#if defined(SUPPORT_LIBREADLINE)
#include <readline/readline.h>
#include <readline/history.h>
#else
#if defined(HAVE_EDITLINE_READLINE_H)
#include <editline/readline.h>
#else
#include <readline/readline.h>
#endif
#endif
#endif


/* ---------------------- System-specific definitions ---------------------- */

/* A number of things vary for Windows builds. Originally, pcretest opened its
input and output without "b"; then I was told that "b" was needed in some
environments, so it was added for release 5.0 to both the input and output. (It
makes no difference on Unix-like systems.) Later I was told that it is wrong
for the input on Windows. I've now abstracted the modes into two macros that
are set here, to make it easier to fiddle with them, and removed "b" from the
input mode under Windows. */

#if defined(_WIN32) || defined(WIN32)
#include <io.h>                /* For _setmode() */
#include <fcntl.h>             /* For _O_BINARY */
#define INPUT_MODE   "r"
#define OUTPUT_MODE  "wb"

#ifndef isatty
#define isatty _isatty         /* This is what Windows calls them, I'm told, */
#endif                         /* though in some environments they seem to   */
                               /* be already defined, hence the #ifndefs.    */
#ifndef fileno
#define fileno _fileno
#endif

/* A user sent this fix for Borland Builder 5 under Windows. */

#ifdef __BORLANDC__
#define _setmode(handle, mode) setmode(handle, mode)
#endif

/* Not Windows */

#else
#include <sys/time.h>          /* These two includes are needed */
#include <sys/resource.h>      /* for setrlimit(). */
#if defined NATIVE_ZOS         /* z/OS uses non-binary I/O */
#define INPUT_MODE   "r"
#define OUTPUT_MODE  "w"
#else
#define INPUT_MODE   "rb"
#define OUTPUT_MODE  "wb"
#endif
#endif

#ifdef __VMS
#include <ssdef.h>
void vms_setsymbol( char *, char *, int );
#endif

/* ------------------End of system-specific definitions -------------------- */

/* Glueing macros that are used in several places below. */

#define glue(a,b) a##b
#define G(a,b) glue(a,b)

/* Other parameters */

#ifndef CLOCKS_PER_SEC
#ifdef CLK_TCK
#define CLOCKS_PER_SEC CLK_TCK
#else
#define CLOCKS_PER_SEC 100
#endif
#endif

#define DEFAULT_OVECCOUNT 15  /* Default ovector count */
#define LOOPREPEAT 500000     /* Default loop count for timing. */
#define VERSION_SIZE 64       /* Size of buffer for the version string. */

/* Execution modes */

#define PCRE8_MODE   8
#define PCRE16_MODE 16
#define PCRE32_MODE 32

/* Processing returns */

enum { PR_OK, PR_SKIP, PR_ABEND };

/* This is defined before including pcre2_internal.h so that it does not get
defined therein. */

#define PRIV(name) name

/* We have to include pcre2_internal.h and pcre2_intstructs.h because we need
to know about the internal macros, structures, and other internal data values;
pcre2test has "inside information" compared to a program that strictly follows
the PCRE2 API. We need the structures at all supported code widths;
pcre2_internal.h does not include the structures if PCRE2_CODE_UNIT_WIDTH is
unset. This enables us to #include pcre2_intstructs.h as many times as
necessary.

Although pcre2_internal.h does itself include pcre2.h, we explicitly include it
before pcre2_internal.h so that the PCRE2_EXP_xxx macros get set
appropriately for an application, not for building PCRE2. */

#include "pcre2.h"
#include "pcre2posix.h"
#include "pcre2_internal.h"

/* Now include the internal mode-dependent structures at all supporte widths.
Because PCRE2_CODE_UNIT_WIDTH was not defined before including pcre2.h, it will
have left PCRE2_SUFFIX defined as a no-op. We must re-define it appropriately
while including the internal structures, and then restore it to a no-op. */

#ifdef SUPPORT_PCRE8
#undef PCRE2_SUFFIX
#define PCRE2_SUFFIX(a) G(a,8)
#include "pcre2_intstructs.h"
#endif

#ifdef SUPPORT_PCRE16
#undef PCRE2_SUFFIX
#define PCRE2_SUFFIX(a) G(a,16)
#include "pcre2_intstructs.h"
#endif

#ifdef SUPPORT_PCRE32
#undef PCRE2_SUFFIX
#define PCRE2_SUFFIX(a) G(a,32)
#include "pcre2_intstructs.h"
#endif

#undef PCRE2_SUFFIX
#define PCRE2_SUFFIX(a) a

/* We need access to some of the data tables that PCRE uses. So as not to have
to keep two copies, we include the source files here, having previously defined
PRIV to change the names of the external symbols to prevent clashes. Defining
PCRE2_INCLUDED makes some minor chantes in the files. */

#define PCRE2_INCLUDED
#include "pcre2_tables.c"

#ifdef FIXME
#include "pcre2_ucd.c"
#endif

/* The definition of the macro PRINTABLE, which determines whether to print an
output character as-is or as a hex value when showing compiled patterns, is
the same as in the printint.c file. We uses it here in cases when the locale
has not been explicitly changed, so as to get consistent output from systems
that differ in their output from isprint() even in the "C" locale. */

#ifdef EBCDIC
#define PRINTABLE(c) ((c) >= 64 && (c) < 255)
#else
#define PRINTABLE(c) ((c) >= 32 && (c) < 127)
#endif

#define PRINTOK(c) (locale_set? isprint(c) : PRINTABLE(c))

/* If we have 8-bit support, default to it; if there is also 16-or 32-bit
support, it can be changed by an option. If there is no 8-bit support, there
must be 16- or 32-bit support, so default to one of them. The config function,
JIT stack, contexts, and version string are the same in all modes, so use the
form of the first that is available. */

#if defined SUPPORT_PCRE8
#define DEFAULT_TEST_MODE PCRE8_MODE
#define PCRE2_CONFIG pcre2_config_8
#define PCRE2_JIT_STACK pcre2_jit_stack_8
#define PCRE2_REAL_GENERAL_CONTEXT pcre2_real_general_context_8
#define PCRE2_REAL_COMPILE_CONTEXT pcre2_real_compile_context_8
#define PCRE2_REAL_MATCH_CONTEXT pcre2_real_match_context_8
#define PCRE2_VERSION pcre2_version_8
#define VERSION_TYPE PCRE2_UCHAR8

#elif defined SUPPORT_PCRE16
#define DEFAULT_TEST_MODE PCRE16_MODE
#define PCRE2_CONFIG pcre2_config_16
#define PCRE2_JIT_STACK pcre2_jit_stack_16
#define PCRE2_REAL_GENERAL_CONTEXT pcre2_real_general_context_16
#define PCRE2_REAL_COMPILE_CONTEXT pcre2_real_compile_context_16
#define PCRE2_REAL_MATCH_CONTEXT pcre2_real_match_context_16
#define PCRE2_VERSION pcre2_version_16
#define VERSION_TYPE PCRE2_UCHAR16

#elif defined SUPPORT_PCRE32
#define DEFAULT_TEST_MODE PCRE32_MODE
#define PCRE2_CONFIG pcre2_config_32
#define PCRE2_JIT_STACK pcre2_jit_stack_32
#define PCRE2_REAL_GENERAL_CONTEXT pcre2_real_general_context_32
#define PCRE2_REAL_COMPILE_CONTEXT pcre2_real_compile_context_32
#define PCRE2_REAL_MATCH_CONTEXT pcre2_real_match_context_32
#define PCRE2_VERSION pcre2_version_32
#define VERSION_TYPE PCRE2_UCHAR32
#endif


/* Table of names for newline types. Must be kept in step with the definitions
of PCRE2_NEWLINE_xx in pcre2.h. */

static const char *newlines[] = {
  "DEFAULT", "CR", "LF", "CRLF", "ANY", "ANYCRLF" };


/* Modifier types and applicability */

enum { MOD_CTC,    /* Applies to a compile context */
       MOD_CTM,    /* Applies to a match context */
       MOD_PAT,    /* Applies to a pattern */
       MOD_DAT,    /* Applies to a data line */
       MOD_PD,     /* Applies to a pattern or a data line */
       MOD_CTL,    /* Is a control bit */
       MOD_BSR,    /* Is a BSR value */
       MOD_IN2,    /* Is one or two integer values */
       MOD_INT,    /* Is an integer value */
       MOD_NL,     /* Is a newline value */
       MOD_NN,     /* Is a number or a name; more than one may occur */
       MOD_OPT,    /* Is an option bit */
       MOD_STR };  /* Is a string */

/* Control bits */

#define CTL_AFTERTEXT        0x00000001
#define CTL_ALLAFTERTEXT     0x00000002
#define CTL_ALLCAPTURES      0x00000004
#define CTL_ALTGLOBMATCH     0x00000008
#define CTL_BYTECODE         0x00000010
#define CTL_CALLOUT_CAPTURE  0x00000020
#define CTL_CALLOUT_NONE     0x00000040
#define CTL_DFA              0x00000080
#define CTL_DFA_RESTART      0x00000100
#define CTL_DFA_SHORTEST     0x00000200
#define CTL_FLIPBYTES        0x00000400
#define CTL_FULLBYTECODE     0x00000800
#define CTL_GETLIST          0x00001000
#define CTL_GLOBMATCH        0x00002000
#define CTL_INFO             0x00004000
#define CTL_JITVERIFY        0x00008000
#define CTL_LIMITS           0x00010000
#define CTL_MARKS            0x00020000
#define CTL_MEMORY           0x00040000
#define CTL_PERLCOMPAT       0x00080000
#define CTL_POSIX            0x00100000

#define CTL_DEBUG            (CTL_FULLBYTECODE|CTL_INFO)  /* For setting */
#define CTL_ANYINFO          (CTL_DEBUG|CTL_BYTECODE)     /* For testing */

/* Structures and tables for handling modifiers. */

typedef struct patctl {    /* Structure for pattern control settings. */
  uint32_t  options;
  uint32_t  control;
  uint32_t  jit;
  uint32_t  stackguard_test;
  uint32_t  tables_id;
  char      locale[32];
  char      save[64];
} patctl;

#define MAXCPYGET 10
#define LENCPYGET 64

typedef struct datctl {    /* Structure for data line control settings. */
  uint32_t  options;
  uint32_t  control;
  uint32_t  cfail[2];
  uint32_t  copy_numbers[MAXCPYGET];
  uint32_t  get_numbers[MAXCPYGET];
  uint32_t  jitstack;
  uint32_t  oveccount;
  uint32_t  offset;
  char      copy_names[LENCPYGET];
  char      get_names[LENCPYGET];
} datctl;

/* Ids for which context to modify. */

enum { CTX_PAT, CTX_DEFPAT, CTX_DAT, CTX_DEFDAT, CTX_DEFANY };

/* Macros to simplify the big table below. */

#define CO(name) offsetof(PCRE2_REAL_COMPILE_CONTEXT, name)
#define MO(name) offsetof(PCRE2_REAL_MATCH_CONTEXT, name)
#define PO(name) offsetof(patctl, name)
#define PD(name) PO(name)
#define DO(name) offsetof(datctl, name)

/* Table of all long-form modifiers. Must be in collating sequence because it 
is searched by binary chop. */

typedef struct modstruct {
  const char *name;
  uint16_t    which;
  uint16_t    type;
  uint32_t    value;
  size_t      offset;
} modstruct;

static modstruct modlist[] = {
  { "aftertext",           MOD_PAT, MOD_CTL, CTL_AFTERTEXT,             PO(control) },
  { "allaftertext",        MOD_PAT, MOD_CTL, CTL_ALLAFTERTEXT,          PO(control) },
  { "allcaptures",         MOD_PAT, MOD_CTL, CTL_ALLCAPTURES,           PO(control) },
  { "allow_empty_class",   MOD_PAT, MOD_OPT, PCRE2_ALLOW_EMPTY_CLASS,   PO(options) },
  { "alt_bsux",            MOD_PAT, MOD_OPT, PCRE2_ALT_BSUX,            PO(options) },
  { "altglobmatch",        MOD_PAT, MOD_CTL, CTL_ALTGLOBMATCH,          PO(control) },
  { "anchored",            MOD_PD,  MOD_OPT, PCRE2_ANCHORED,            PD(options) },
  { "auto_callout",        MOD_PAT, MOD_OPT, PCRE2_AUTO_CALLOUT,        PO(options) },
  { "bsr",                 MOD_CTC, MOD_BSR, 0,                         CO(bsr_convention) },
  { "bytecode",            MOD_PAT, MOD_CTL, CTL_BYTECODE,              PO(control) },
  { "callout_capture",     MOD_DAT, MOD_CTL, CTL_CALLOUT_CAPTURE,       DO(control) },
  { "callout_fail",        MOD_DAT, MOD_IN2, 0,                         DO(cfail) },
  { "callout_none",        MOD_DAT, MOD_CTL, CTL_CALLOUT_NONE,          DO(control) },
  { "caseless",            MOD_PAT, MOD_OPT, PCRE2_CASELESS,            PO(options) },
  { "copy",                MOD_DAT, MOD_NN,  DO(copy_numbers),          DO(copy_names) },
  { "debug",               MOD_PAT, MOD_CTL, CTL_DEBUG,                 PO(control) },
  { "dfa",                 MOD_DAT, MOD_CTL, CTL_DFA,                   DO(control) },
  { "dfa_restart",         MOD_DAT, MOD_OPT, CTL_DFA_RESTART,           DO(options) },
  { "dfa_shortest",        MOD_DAT, MOD_OPT, CTL_DFA_SHORTEST,          DO(options) },
  { "dollar_endonly",      MOD_PAT, MOD_OPT, PCRE2_DOLLAR_ENDONLY,      PO(options) },
  { "dotall",              MOD_PAT, MOD_OPT, PCRE2_DOTALL,              PO(options) },
  { "dupnames",            MOD_PAT, MOD_OPT, PCRE2_DUPNAMES,            PO(options) },
  { "extended",            MOD_PAT, MOD_OPT, PCRE2_EXTENDED,            PO(options) },
  { "firstline",           MOD_PAT, MOD_OPT, PCRE2_FIRSTLINE,           PO(options) },
  { "flipbytes",           MOD_PAT, MOD_CTL, CTL_FLIPBYTES,             PO(control) },
  { "fullbytecode",        MOD_PAT, MOD_CTL, CTL_FULLBYTECODE,          PO(control) },
  { "get",                 MOD_DAT, MOD_NN,  DO(get_numbers),           DO(get_names) },
  { "getlist",             MOD_DAT, MOD_CTL, CTL_GETLIST,               DO(control) },
  { "globmatch",           MOD_PAT, MOD_CTL, CTL_GLOBMATCH,             PO(control) },
  { "info",                MOD_PAT, MOD_CTL, CTL_INFO,                  PO(control) },
  { "jit",                 MOD_PAT, MOD_INT, 1,                         PO(jit) },
  { "jitstack",            MOD_DAT, MOD_INT, 0,                         DO(jitstack) },
  { "jitverify",           MOD_PAT, MOD_CTL, CTL_JITVERIFY,             PO(control) },
  { "limits",              MOD_DAT, MOD_CTL, CTL_LIMITS,                DO(control) },
  { "locale",              MOD_PAT, MOD_STR, 0,                         PO(locale) },
  { "marks",               MOD_PAT, MOD_CTL, CTL_MARKS,                 PO(control) },
  { "match_limit",         MOD_CTM, MOD_INT, 0,                         MO(match_limit) },
  { "match_unset_backref", MOD_PAT, MOD_OPT, PCRE2_MATCH_UNSET_BACKREF, PO(options) },
  { "memory",              MOD_PD,  MOD_CTL, CTL_MEMORY,                PD(control) },
  { "multiline",           MOD_PAT, MOD_OPT, PCRE2_MULTILINE,           PO(options) },
  { "never_ucp",           MOD_PAT, MOD_OPT, PCRE2_NEVER_UCP,           PO(options) },
  { "never_utf",           MOD_PAT, MOD_OPT, PCRE2_NEVER_UTF,           PO(options) },
  { "newline",             MOD_CTC, MOD_NL,  0,                         CO(newline_convention) },
  { "no_auto_capture",     MOD_PAT, MOD_OPT, PCRE2_NO_AUTO_CAPTURE,     PO(options) },
  { "no_auto_possess",     MOD_PAT, MOD_OPT, PCRE2_NO_AUTO_POSSESS,     PO(options) },
  { "no_start_optimize",   MOD_PD,  MOD_OPT, PCRE2_NO_START_OPTIMIZE,   PD(options) },
  { "no_utf_check",        MOD_PD,  MOD_OPT, PCRE2_NO_UTF_CHECK,        PD(options) },
  { "notbol",              MOD_DAT, MOD_OPT, PCRE2_NOTBOL,              DO(options) },
  { "notempty",            MOD_DAT, MOD_OPT, PCRE2_NOTEMPTY,            DO(options) },
  { "notempty_atstart",    MOD_DAT, MOD_OPT, PCRE2_NOTEMPTY_ATSTART,    DO(options) },
  { "noteol",              MOD_DAT, MOD_OPT, PCRE2_NOTEOL,              DO(options) },
  { "offset",              MOD_DAT, MOD_INT, 0,                         DO(offset) },
  { "ovector",             MOD_DAT, MOD_INT, 0,                         DO(oveccount) },
  { "parens_nest_limit",   MOD_CTC, MOD_INT, 0,                         CO(parens_nest_limit) },
  { "partial_hard",        MOD_DAT, MOD_OPT, PCRE2_PARTIAL_HARD,        DO(options) },
  { "partial_soft",        MOD_DAT, MOD_OPT, PCRE2_PARTIAL_SOFT,        DO(options) },
  { "perlcompat",          MOD_PAT, MOD_CTL, CTL_PERLCOMPAT,            PO(control) },
  { "posix",               MOD_PAT, MOD_CTL, CTL_POSIX,                 PO(control) },
  { "recursion_limit",     MOD_CTM, MOD_INT, 0,                         MO(recursion_limit) },
  { "save",                MOD_PAT, MOD_STR, 0,                         PO(save) },
  { "stackguard",          MOD_PAT, MOD_INT, 0,                         PO(stackguard_test) },
  { "tables",              MOD_PAT, MOD_INT, 0,                         PO(tables_id) },
  { "ucp",                 MOD_PAT, MOD_OPT, PCRE2_UCP,                 PO(options) },
  { "ungreedy",            MOD_PAT, MOD_OPT, PCRE2_UNGREEDY,            PO(options) },
  { "utf",                 MOD_PAT, MOD_OPT, PCRE2_UTF,                 PO(options) }
};

#define MODLISTCOUNT sizeof(modlist)/sizeof(modstruct)

/* Controls and options that are supported for use with the POSIX interface. */

#define POSIX_SUPPORTED_COMPILE_OPTIONS ( \
  PCRE2_CASELESS|PCRE2_DOTALL|PCRE2_MULTILINE|PCRE2_NO_AUTO_CAPTURE| \
  PCRE2_UCP|PCRE2_UTF|PCRE2_UNGREEDY)
  
#define POSIX_SUPPORTED_COMPILE_CONTROLS ( \
  CTL_AFTERTEXT|CTL_ALLAFTERTEXT|CTL_POSIX)
  
#define POSIX_SUPPORTED_MATCH_OPTIONS ( \
  PCRE2_NOTBOL|PCRE2_NOTEMPTY|PCRE2_NOTEOL)
  
#define POSIX_SUPPORTED_MATCH_CONTROLS ( 0 )    

/* Table of single-character and doubled-character abbreviated modifiers. The
index field is initialized to -1, but the first time the modifier is
encountered, it is filled in with the index of the full entry in modlist, to
save repeated searching when processing multiple test items. This short list is 
searched serially, so its order does not matter. */

typedef struct c1modstruct {
  const char *fullname;
  uint32_t    onechar;
  int         index;
} c1modstruct;

static c1modstruct c1modlist[] = {
  { "bytecode",     'B',           -1 },
  { "fullbytecode", ('B'<<8)|'B',  -1 },
  { "debug",        'D',           -1 },
  { "info",         'I',           -1 },
  { "partial_soft", 'P',           -1 },
  { "partial_hard", ('P'<<8)|'P',  -1 },
  { "globmatch",    'g',           -1 },
  { "altglobmatch", ('g'<<8)|'g',  -1 },
  { "caseless",     'i',           -1 },
  { "multiline",    'm',           -1 },
  { "dotall",       's',           -1 },
  { "extended",     'x',           -1 }
};

#define C1MODLISTCOUNT sizeof(c1modlist)/sizeof(c1modstruct)

/* Table of arguments for the -C command line option. Use macros to make the
table easier to read. */

#if defined SUPPORT_PCRE8
#define SUPPORT_8 1
#endif
#if defined SUPPORT_PCRE16
#define SUPPORT_16 1
#endif
#if defined SUPPORT_PCRE32
#define SUPPORT_32 1
#endif

#ifndef SUPPORT_8
#define SUPPORT_8 0
#endif
#ifndef SUPPORT_16
#define SUPPORT_16 0
#endif
#ifndef SUPPORT_32
#define SUPPORT_32 0
#endif

#ifdef EBCDIC
#define SUPPORT_EBCDIC 1
#define EBCDIC_NL CHAR_LF
#else
#define SUPPORT_EBCDIC 0
#define EBCDIC_NL 0
#endif

typedef struct coptstruct {
  const char *name;
  uint32_t    type;
  uint32_t    value;
} coptstruct;

enum { CONF_BSR,
       CONF_FIX,
       CONF_FIZ,
       CONF_INT,
       CONF_NL
};

static coptstruct coptlist[] = {
  { "bsr",       CONF_BSR, PCRE2_CONFIG_BSR },
  { "ebcdic",    CONF_FIX, SUPPORT_EBCDIC },
  { "ebcdic-nl", CONF_FIZ, EBCDIC_NL },
  { "jit",       CONF_INT, PCRE2_CONFIG_JIT },
  { "linksize",  CONF_INT, PCRE2_CONFIG_LINK_SIZE },
  { "newline",   CONF_NL,  PCRE2_CONFIG_NEWLINE },
  { "pcre16",    CONF_FIX, SUPPORT_16 },
  { "pcre32",    CONF_FIX, SUPPORT_32 },
  { "pcre8",     CONF_FIX, SUPPORT_8 },
  { "utf",       CONF_INT, PCRE2_CONFIG_UTF }
};

#define COPTLISTCOUNT sizeof(coptlist)/sizeof(coptstruct)

#undef SUPPORT_8
#undef SUPPORT_16
#undef SUPPORT_32
#undef SUPPORT_EBCDIC


/* Static variables */

static FILE *infile;
static FILE *outfile;

/* FIXME */
static BOOL locale_set = FALSE;

static int buffer_size = 50000;
static size_t dbuffer_size = 1u << 14;        /* Bytes */
static int code_unit_size;                    /* Bytes */
static int test_mode = DEFAULT_TEST_MODE;
static int timeit = 0;
static int timeitm = 0;

clock_t total_compile_time = 0;
clock_t total_match_time = 0;

static uint8_t *buffer = NULL;
static uint8_t *pbuffer = NULL;
static uint8_t *dbuffer = NULL;

static VERSION_TYPE version[64];

static patctl def_patctl;
static patctl pat_patctl;
static datctl def_datctl;
static datctl dat_datctl;

static regex_t preg = { NULL, NULL, 0, 0 };


/* We need buffers for building 16/32-bit strings; 8-bit strings don't need
rebuilding, but set up an appropriate name (pbuffer8) for use in macros. It
will be pointed to the same memory as pbuffer. We also need the tables of
operator lengths that are used for 16/32-bit compiling, in order to swap bytes
in a pattern for saving/reloading testing. Luckily, the data for these tables
is defined as a macro. However, we must ensure that LINK_SIZE and IMM2_SIZE
(which are used in the tables) are adjusted appropriately for the 16/32-bit
world. LINK_SIZE is also used later in this program. */

static uint8_t *pbuffer8 = NULL;

#ifdef SUPPORT_PCRE16
#undef IMM2_SIZE
#define IMM2_SIZE 1

#if LINK_SIZE == 2
#undef LINK_SIZE
#define LINK_SIZE 1
#elif LINK_SIZE == 3 || LINK_SIZE == 4
#undef LINK_SIZE
#define LINK_SIZE 2
#else
#error LINK_SIZE must be either 2, 3, or 4
#endif  /* LINK_SIZE */

static int pbuffer16_size = 0;
static uint16_t *pbuffer16 = NULL;
#ifdef FIXME
static const uint16_t OP_lengths16[] = { OP_LENGTHS };
#endif  /* FIXME */
#endif  /* SUPPORT_PCRE16 */


#ifdef SUPPORT_PCRE32
#undef IMM2_SIZE
#define IMM2_SIZE 1
#undef LINK_SIZE
#define LINK_SIZE 1

static int pbuffer32_size = 0;
static uint32_t *pbuffer32 = NULL;
#ifdef FIXME
static const uint32_t OP_lengths32[] = { OP_LENGTHS };
#endif  /* FIXME */
#endif  /* SUPPORT_PCRE32 */


/* ---------------- Mode-dependent variables -------------------*/

#ifdef SUPPORT_PCRE8
pcre2_code_8             *compiled_code8;
pcre2_compile_context_8  *pat_context8, *default_pat_context8;
pcre2_match_context_8    *dat_context8, *default_dat_context8;
#endif

#ifdef SUPPORT_PCRE16
pcre2_code_16            *compiled_code16;
pcre2_compile_context_16 *pat_context16, *default_pat_context16;
pcre2_match_context_16   *dat_context16, *default_dat_context16;
#endif

#ifdef SUPPORT_PCRE32
pcre2_code_32            *compiled_code32;
pcre2_compile_context_32 *pat_context32, *default_pat_context32;
pcre2_match_context_32   *dat_context32, *default_dat_context32;
#endif


/* ---------------- Macros that work in all modes ----------------- */

#define CAST8VAR(x) CASTVAR(uint8_t *, x)
#define SET(x,y) SETOP(x,y,=)
#define SETPLUS(x,y) SETOP(x,y,+=)


/* ---------------- Mode-dependent, runtime-testing macros ------------------*/

/* Define macros for variables and functions that must be selected dynamically
depending on the mode setting (8, 16, 32). These are dependent on which modes
are supported. */

#if (defined (SUPPORT_PCRE8) + defined (SUPPORT_PCRE16) + \
     defined (SUPPORT_PCRE32)) >= 2

/* ----- All three modes supported ----- */

#if defined(SUPPORT_PCRE8) && defined(SUPPORT_PCRE16) && defined(SUPPORT_PCRE32)

#define CASTVAR(t,x) ( \
  (test_mode == PCRE8_MODE)? (t)G(x,8) : \
  (test_mode == PCRE16_MODE)? (t)G(x,16) : (t)G(x,32))

#define DATCTXCPY(a,b) \
  if (test_mode == PCRE8_MODE) \
    memcpy(G(a,8),G(b,8),sizeof(pcre2_match_context_8)); \
  else if (test_mode == PCRE16_MODE) \
    memcpy(G(a,16),G(b,16),sizeof(pcre2_match_context_16)); \
  else memcpy(G(a,32),G(b,32),sizeof(pcre2_match_context_32))

#define FLD(a,b) ((test_mode == PCRE8_MODE)? G(a,8)->b : \
  (test_mode == PCRE16_MODE)? G(a,16)->b : G(a,32)->b)

#define PATCTXCPY(a,b) \
  if (test_mode == PCRE8_MODE) \
    memcpy(G(a,8),G(b,8),sizeof(pcre2_compile_context_8)); \
  else if (test_mode == PCRE16_MODE) \
    memcpy(G(a,16),G(b,16),sizeof(pcre2_compile_context_16)); \
  else memcpy(G(a,32),G(b,32),sizeof(pcre2_compile_context_32))

#define PCHARS(lv, p, offset, len, utf, f) \
  if (test_mode == PCRE32_MODE) \
    lv = pchars32(p, offset, len, utf, f); \
  else if (test_mode == PCRE16_MODE) \
    lv = pchars16(p, offset, len, utf, f); \
  else \
    lv = pchars8(p, offset, len, utf, f)

#define PCHARSV(p, offset, len, utf, f) \
  if (test_mode == PCRE32_MODE) \
    (void)pchars32((PCRE2_SPTR32)p+offset, len, utf, f); \
  else if (test_mode == PCRE16_MODE) \
    (void)pchars16((PCRE2_SPTR16)p+offset, len, utf, f); \
  else \
    (void)pchars8((PCRE2_SPTR8)p+offset, len, utf, f)

#define PCRE2_COMPILE(a,b,c,d,e,f,g) \
  if (test_mode == PCRE8_MODE) \
    G(a,8) = G(pcre2_compile,8)(G(b,8),c,d,e,f,G(g,8)); \
  else if (test_mode == PCRE16_MODE) \
    G(a,16) = G(pcre2_compile,16)(G(b,16),c,d,e,f,G(g,16)); \
  else \
    G(a,32) = G(pcre2_compile,32)(G(b,32),c,d,e,f,G(g,32))

#define PCRE2_JIT_COMPILE(a,b) \
  if (test_mode == PCRE8_MODE) \
    G(pcre2_jit_compile,8)(G(a,8),b); \
  else if (test_mode == PCRE16_MODE) \
    G(pcre2_jit_compile,16)(G(a,16),b); \
  else \
    G(pcre2_jit_compile,32)(G(a,32),b)

#define PCRE2_PATTERN_INFO(a,b,c,d) \
  if (test_mode == PCRE8_MODE) \
    a = G(pcre2_pattern_info,8)(G(b,8),c,d); \
  else if (test_mode == PCRE16_MODE) \
    a = G(pcre2_pattern_info,16)(G(b,16),c,d); \
  else \
    a = G(pcre2_pattern_info,32)(G(b,32),c,d)

#define PTR(x) ( \
  (test_mode == PCRE8_MODE)? (void *)G(x,8) : \
  (test_mode == PCRE16_MODE)? (void *)G(x,16) : \
  (void *)G(x,32))

#define SETOP(x,y,z) \
  if (test_mode == PCRE8_MODE) G(x,8) z y; \
  else if (test_mode == PCRE16_MODE) G(x,16) z y; \
  else G(x,32) z y

#define SETCASTPTR(x,y) \
  if (test_mode == PCRE8_MODE) \
    G(x,8) = (uint8_t *)y; \
  else if (test_mode == PCRE16_MODE) \
    G(x,16) = (uint16_t *)y; \
  else \
    G(x,32) = (uint32_t *)y

#define STRLEN(p) ((test_mode == PCRE8_MODE)? ((int)strlen((char *)p)) : \
  (test_mode == PCRE16_MODE)? ((int)strlen16((PCRE2_SPTR16)p)) : \
  ((int)strlen32((PCRE2_SPTR32)p)))

#define SUB1(a,b) \
  if (test_mode == PCRE8_MODE) G(a,8)(G(b,8)); \
  else if (test_mode == PCRE16_MODE) G(a,16)(G(b,16)); \
  else G(a,32)(G(b,32))

#define SUB2(a,b,c) \
  if (test_mode == PCRE8_MODE) G(a,8)(G(b,8),G(c,8)); \
  else if (test_mode == PCRE16_MODE) G(a,16)(G(b,16),G(c,16)); \
  else G(a,32)(G(b,32),G(c,32))

#define TEST(x,r,y) ( \
  (test_mode == PCRE8_MODE && G(x,8) r (y)) || \
  (test_mode == PCRE16_MODE && G(x,16) r (y)) || \
  (test_mode == PCRE32_MODE && G(x,32) r (y)))

#define VAR(x) ( \
  (test_mode == PCRE8_MODE)? G(x,8) : \
  (test_mode == PCRE16_MODE)? G(x,16) : G(x,32))


/* ----- Two out of three modes are supported ----- */

#else

/* We can use some macro trickery to make a single set of definitions work in
the three different cases. */

/* ----- 32-bit and 16-bit but not 8-bit supported ----- */

#if defined(SUPPORT_PCRE32) && defined(SUPPORT_PCRE16)
#define BITONE 32
#define BITTWO 16

/* ----- 32-bit and 8-bit but not 16-bit supported ----- */

#elif defined(SUPPORT_PCRE32) && defined(SUPPORT_PCRE8)
#define BITONE 32
#define BITTWO 8

/* ----- 16-bit and 8-bit but not 32-bit supported ----- */

#else
#define BITONE 16
#define BITTWO 8
#endif


/* ----- Common macros for two-mode cases ----- */

#define CASTVAR(t,x) ( \
  (test_mode == G(G(PCRE,BITONE(,_MODE))? \
    (t)G(x,BITONE) : (t)G(x,BITTWO))

#define DATCTXCPY(a,b) \
  if (test_mode == G(G(PCRE,BITONE),_MODE)) \
    memcpy(G(a,BITONE),G(b,BITONE),sizeof(G(pcre2_match_context_,BITONE))); \
  else \
    memcpy(G(a,BITTWO),G(b,BITTWO),sizeof(G(pcre2_match_context_,BITTWO)))

#define FLD(a,b) \
  ((test_mode == G(G(PCRE,BITONE),_MODE))? G(a,BITONE)->b : G(a,BITTWO)->b)

#define PATCTXCPY(a,b) \
  if (test_mode == G(G(PCRE,BITONE),_MODE)) \
    memcpy(G(a,BITONE),G(b,BITONE),sizeof(G(pcre2_compile_context_,BITONE))); \
  else \
    memcpy(G(a,BITTWO),G(b,BITTWO),sizeof(G(pcre2_compile_context_,BITTWO)))

#define PCHARS(lv, p, offset, len, utf, f) \
  if (test_mode == G(G(PCRE,BITONE),_MODE)) \
    lv = G(pchars,BITONE)(p, offset, len, utf, f); \
  else \
    lv = G(PCHARS,BITTWO)(p, offset, len, utf, f)

#define PCHARSV(p, offset, len, utf, f) \
  if (test_mode == G(G(PCRE,BITONE),_MODE)) \
    (void)G(pchars,BITONE)((G(PCRE2_SPTR,BITONE))p+offset, len, utf, f); \
  else \
    (void)G(PCHARS,BITTWO)((G(PCRE2_SPTR,BITTWO))p+offset, len, utf, f)

#define PCRE2_COMPILE(a,b,c,d,e,f,g) \
  if (test_mode == G(G(PCRE,BITONE),_MODE)) \
    G(a,BITONE) = G(pcre2_compile,BITONE)(G(b,BITONE),c,d,e,f,G(g,BITONE)); \
  else \
    G(a,BITTWO) = G(pcre2_compile,BITTWO)(G(b,BITTWO),c,d,e,f,G(g,BITTWO))

#define PCRE2_JIT_COMPILE(a,b) \
  if (test_mode == G(G(PCRE,BITONE),_MODE)) \
    G(pcre2_jit_compile,BITONE)(G(a,BITONE),b); \
  else \
    G(pcre2_jit_compile,BITTWO)(G(a,BITTWO),b)

#define PCRE2_PATTERN_INFO(a,b,c,d) \
  if (test_mode == G(G(PCRE,BITONE),_MODE)) \
    a = G(pcre2_pattern_info,BITONE)(G(b,BITONE),c,d); \
  else \
    a = G(pcre2_pattern_info,BITTWO)(G(b,BITTWO),c,d)

#define PTR(x) ( \
  (test_mode == G(G(PCRE,BITONE),_MODE))? (void *)G(x,BITONE) : \
  (void *)G(x,BITTWO))

#define SETOP(x,y,z) \
  if (test_mode == G(G(PCRE,BITONE),_MODE)) G(x,BITONE) z y; \
  else G(x,BITTWO) z y

#define SETCASTPTR(x,y) \
  if (test_mode == PCRE8_MODE) \
    G(x,BITONE) = (G(G(uint,BITONE),_t) *)y; \
  else \
    G(x,BITTWO) = (G(G(uint,BITTWO),_t) *)y

#define STRLEN(p) ((test_mode == G(G(PCRE,BITONE),_MODE))? \
  G(strlen,BITONE)((G(PCRE2_SPTR,BITONE))p)) : \
  G(strlen(BITTWO)((G(PCRE2_SPTR,BITTWO))p)))

#define SUB1(a,b) \
  if (test_mode == G(G(PCRE,BITONE),_MODE)) \
    G(a,BITONE))(G(b,BITONE)); \
  else \
    G(a,BITTWO))(G(b,BITTWO))

#define SUB2(a,b,c) \
  if (test_mode == G(G(PCRE,BITONE),_MODE)) \
    G(a,BITONE))(G(b,BITONE),G(c,BITONE)); \
  else \
    G(a,BITTWO))(G(b,BITTWO),G(c,BITTWO))

#define TEST(x,r,y) ( \
  (test_mode == G(G(PCRE,BITONE),_MODE) && G(x,BITONE) r (y)) || \
  (test_mode == G(G(PCRE,BITTWO),_MODE) && G(x,BITTWO) r (y)))

#define VAR(x) ( \
  (test_mode == G(G(PCRE,BITONE(,_MODE))? G(x,BITONE) : G(x,BITTWO))

#endif  /* Two out of three modes */

/* ----- End of cases where more than one mode is supported ----- */


/* ----- Only 8-bit mode is supported ----- */

#elif defined SUPPORT_PCRE8
#define CASTVAR(t,x) (t)G(x,8)
#define DATCTXCPY(a,b) memcpy(G(a,8),G(b,8),sizeof(pcre2_match_context_8))
#define FLD(a,b) G(a,8)->b
#define PATCTXCPY(a,b) memcpy(G(a,8),G(b,8),sizeof(pcre2_compile_context_8))
#define PCHARS(lv, p, offset, len, utf, f) \
  lv = pchars8(p, offset, len, utf, f)
#define PCHARSV(p, offset, len, utf, f) \
  (void)pchars8((PCRE2_SPTR8)p+offset, len, utf, f)
#define PCRE2_COMPILE(a,b,c,d,e,f,g) \
  G(a,8) = G(pcre2_compile,8)(G(b,8),c,d,e,f,G(g,8))
#define PCRE2_JIT_COMPILE(a,b) pcre2_jit_compile_8(G(a,8),b)
#define PCRE2_PATTERN_INFO(a,b,c,d) \
  a = pcre2_pattern_info_8(G(b,8),c,d)
#define PTR(x) (void *)G(x,8)
#define SETOP(x,y,z) G(x,8) z y
#define SETCASTPTR(x,y) G(x,8) = (uint8_t) *)y
#define STRLEN(p) (int)strlen(p)
#define SUB1(a,b) G(a,8)(G(b,8))
#define SUB2(a,b,c) G(a,8)(G(b,8),G(c,8))
#define TEST(x,r,y) (G(a,8) r (y))
#define VAR(x) G(x,8)


/* ----- Only 16-bit mode is supported ----- */

#elif defined SUPPORT_PCRE16
#define CASTVAR(t,x) (t)G(x,16)
#define DATCTXCPY(a,b) memcpy(G(a,16),G(b,16),sizeof(pcre2_match_context_16))
#define FLD(a,b) G(a,16)->b
#define PATCTXCPY(a,b) memcpy(G(a,16),G(b,16),sizeof(pcre2_compile_context_16))
#define PCHARS(lv, p, offset, len, utf, f) \
  lv = pchars16(p, offset, len, utf, f)
#define PCHARSV(p, offset, len, utf, f) \
  (void)pchars16((PCRE2_SPTR16)p+offset, len, utf, f)
#define PCRE2_COMPILE(a,b,c,d,e,f,g) \
  G(a,16) = G(pcre2_compile,16)(G(b,16),c,d,e,f,G(g,16))
#define PCRE2_JIT_COMPILE(a,b) pcre2_jit_compile_16(G(a,16),b)
#define PCRE2_PATTERN_INFO(a,b,c,d) \
  a = pcre2_pattern_info_16(G(b,16),c,d)
#define PTR(x) (void *)G(x,16)
#define SET(x,y,z) G(x,16) z y
#define SETCASTPTR(x,y) G(x,16) = (uint16_t) *)y
#define STRLEN(p) (int)strlen16(p)
#define SUB1(a,b) G(a,16)(G(b,16))
#define SUB2(a,b,c) G(a,16)(G(b,16),G(c,16))
#define TEST(x,r,y) (G(a,16) r (y))
#define VAR(x) G(x,16)


/* ----- Only 32-bit mode is supported ----- */

#elif defined SUPPORT_PCRE32
#define CASTVAR(t,x) (t)G(x,32)
#define DATCTXCPY(a,b) memcpy(G(a,32),G(b,32),sizeof(pcre2_match_context_32))
#define FLD(a,b) G(a,32)->b
#define PATCTXCPY(a,b) memcpy(G(a,32),G(b,32),sizeof(pcre2_compile_context_32))
#define PCHARS(lv, p, offset, len, utf, f) \
  lv = pchars32(p, offset, len, utf, f)
#define PCHARSV(p, offset, len, utf, f) \
  (void)pchars32((PCRE2_SPTR32)p+offset, len, utf, f)
#define PCRE2_COMPILE(a,b,c,d,e,f,g) \
  G(a,32) = G(pcre2_compile,32)(G(b,32),c,d,e,f,G(g,32))
#define PCRE2_JIT_COMPILE(a,b) pcre2_jit_compile_32(G(a,32),b)
#define PCRE2_PATTERN_INFO(a,b,c,d) \
  a = pcre2_pattern_info_32(G(b,32),c,d)
#define PTR(x) (void *)G(x,32)
#define SET(x,y,z) G(x,32) z y
#define SETCASTPTR(x,y) G(x,32) = (uint32_t) *)y
#define STRLEN(p) (int)strle32(p)
#define SUB1(a,b) G(a,32)(G(b,32))
#define SUB2(a,b,c) G(a,32)(G(b,32),G(c,32))
#define TEST(x,r,y) (G(a,32) r (y))
#define VAR(x) G(x,32)

#endif

/* ----- End of mode-specific function call macros ----- */




/*************************************************
*      Convert UTF-8 character to code point     *
*************************************************/

/* This function reads one or more bytes that represent a UTF-8 character,
and returns the codepoint of that character. Note that the function supports
the original UTF-8 definition of RFC 2279, allowing for values in the range 0
to 0x7fffffff, up to 6 bytes long. This makes it possible to generate
codepoints greater than 0x10ffff which are useful for testing PCRE's error
checking, and also for generating 32-bit non-UTF data values above the UTF
limit.

Argument:
  utf8bytes   a pointer to the byte vector
  vptr        a pointer to an int to receive the value

Returns:      >  0 => the number of bytes consumed
              -6 to 0 => malformed UTF-8 character at offset = (-return)
*/

static int
utf82ord(PCRE2_SPTR8 utf8bytes, uint32_t *vptr)
{
uint32_t c = *utf8bytes++;
uint32_t d = c;
int i, j, s;

for (i = -1; i < 6; i++)               /* i is number of additional bytes */
  {
  if ((d & 0x80) == 0) break;
  d <<= 1;
  }

if (i == -1) { *vptr = c; return 1; }  /* ascii character */
if (i == 0 || i == 6) return 0;        /* invalid UTF-8 */

/* i now has a value in the range 1-5 */

s = 6*i;
d = (c & utf8_table3[i]) << s;

for (j = 0; j < i; j++)
  {
  c = *utf8bytes++;
  if ((c & 0xc0) != 0x80) return -(j+1);
  s -= 6;
  d |= (c & 0x3f) << s;
  }

/* Check that encoding was the correct unique one */

for (j = 0; j < utf8_table1_size; j++)
  if (d <= (uint32_t)utf8_table1[j]) break;
if (j != i) return -(i+1);

/* Valid value */

*vptr = d;
return i+1;
}



/*************************************************
*             Print one character                *
*************************************************/

/* Print a single character either literally, or as a hex escape.

Arguments:
  c            the character
  utf          TRUE in UTF mode
  f            the FILE to print to

Returns:       number of characters written
*/

static int
pchar(uint32_t c, BOOL utf, FILE *f)
{
int n = 0;
if (PRINTOK(c))
  {
  if (f != NULL) fprintf(f, "%c", c);
  return 1;
  }

if (c < 0x100)
  {
  if (utf)
    {
    if (f != NULL) fprintf(f, "\\x{%02x}", c);
    return 6;
    }
  else
    {
    if (f != NULL) fprintf(f, "\\x%02x", c);
    return 4;
    }
  }

if (f != NULL) n = fprintf(f, "\\x{%02x}", c);
return n >= 0 ? n : 0;
}



#ifdef SUPPORT_PCRE16
/*************************************************
*    Find length of 0-terminated 16-bit string   *
*************************************************/

static int strlen16(PCRE2_SPTR16 p)
{
PCRE2_SPTR16 pp = p;
while (*pp != 0) pp++;
return (int)(pp - p);
}
#endif  /* SUPPORT_PCRE16 */



#ifdef SUPPORT_PCRE32
/*************************************************
*    Find length of 0-terminated 32-bit string   *
*************************************************/

static int strlen32(PCRE2_SPTR32 p)
{
PCRE2_SPTR32 pp = p;
while (*pp != 0) pp++;
return (int)(pp - p);
}
#endif  /* SUPPORT_PCRE32 */


#ifdef SUPPORT_PCRE8
/*************************************************
*         Print 8-bit character string           *
*************************************************/

/* Must handle UTF-8 strings in utf8 mode. Yields number of characters printed.
If handed a NULL file, just counts chars without printing. */

static int pchars8(PCRE2_SPTR8 p, int length, BOOL utf, FILE *f)
{
uint32_t c = 0;
int yield = 0;
if (length < 0) length = strlen((char *)p);
while (length-- > 0)
  {
  if (utf)
    {
    int rc = utf82ord(p, &c);
    if (rc > 0 && rc <= length + 1)   /* Mustn't run over the end */
      {
      length -= rc - 1;
      p += rc;
      yield += pchar(c, utf, f);
      continue;
      }
    }
  c = *p++;
  yield += pchar(c, utf, f);
  }
return yield;
}
#endif


#ifdef SUPPORT_PCRE16
/*************************************************
*           Print 16-bit character string        *
*************************************************/

/* Must handle UTF-16 strings in utf mode. Yields number of characters printed.
If handed a NULL file, just counts chars without printing. */

static int pchars16(PCRE2_SPTR16 p, int length, BOOL utf, FILE *f)
{
int yield = 0;
if (length < 0) length = strlen16(p);
while (length-- > 0)
  {
  uint32_t c = *p++ & 0xffff;
  if (utf && c >= 0xD800 && c < 0xDC00 && length > 0)
    {
    int d = *p & 0xffff;
    if (d >= 0xDC00 && d <= 0xDFFF)
      {
      c = ((c & 0x3ff) << 10) + (d & 0x3ff) + 0x10000;
      length--;
      p++;
      }
    }
  yield += pchar(c, utf, f);
  }
return yield;
}
#endif  /* SUPPORT_PCRE16 */



#ifdef SUPPORT_PCRE32
/*************************************************
*           Print 32-bit character string        *
*************************************************/

/* Must handle UTF-32 strings in utf mode. Yields number of characters printed.
If handed a NULL file, just counts chars without printing. */

static int pchars32(PCRE2_SPTR32 p, int length, BOOL utf, FILE *f)
{
int yield = 0;
(void)(utf);  /* Avoid compiler warning */
if (length < 0) length = strlen32(p);
while (length-- > 0)
  {
  uint32_t c = *p++;
  yield += pchar(c, utf, f);
  }
return yield;
}
#endif  /* SUPPORT_PCRE32 */




/*************************************************
*       Convert character value to UTF-8         *
*************************************************/

/* This function takes an integer value in the range 0 - 0x7fffffff
and encodes it as a UTF-8 character in 0 to 6 bytes.

Arguments:
  cvalue     the character value
  utf8bytes  pointer to buffer for result - at least 6 bytes long

Returns:     number of characters placed in the buffer
*/

static int
ord2utf8(uint32_t cvalue, uint8_t *utf8bytes)
{
register int i, j;
if (cvalue > 0x7fffffffu)
  return -1;
for (i = 0; i < utf8_table1_size; i++)
  if (cvalue <= (uint32_t)utf8_table1[i]) break;
utf8bytes += i;
for (j = i; j > 0; j--)
 {
 *utf8bytes-- = 0x80 | (cvalue & 0x3f);
 cvalue >>= 6;
 }
*utf8bytes = utf8_table2[i] | cvalue;
return i + 1;
}



#ifdef SUPPORT_PCRE16
/*************************************************
*         Convert a string to 16-bit             *
*************************************************/

/* The input is always interpreted as a string of UTF-8 bytes. If all the input
bytes are ASCII, the space needed for a 16-bit string is exactly double the
8-bit size. Otherwise, the size needed for a 16-bit string is no more than
double, because up to 0xffff uses no more than 3 bytes in UTF-8 but possibly 4
in UTF-16. Higher values use 4 bytes in UTF-8 and up to 4 bytes in UTF-16. The
result is always left in pbuffer16.

Note that this function does not object to surrogate values. This is
deliberate; it makes it possible to construct UTF-16 strings that are invalid,
for the purpose of testing that they are correctly faulted.

Arguments:
  p          points to a byte string
  utf        non-zero if converting to UTF-16
  len        number of bytes in the string (excluding trailing zero)

Returns:     number of 16-bit data items used (excluding trailing zero)
             OR -1 if a UTF-8 string is malformed
             OR -2 if a value > 0x10ffff is encountered in UTF mode
             OR -3 if a value > 0xffff is encountered when not in UTF mode
*/

static int
to16(uint8_t *p, int utf, int len)
{
uint16_t *pp;

if (pbuffer16_size < 2*len + 2)
  {
  if (pbuffer16 != NULL) free(pbuffer16);
  pbuffer16_size = 2*len + 2;
  pbuffer16 = (uint16_t *)malloc(pbuffer16_size);
  if (pbuffer16 == NULL)
    {
    fprintf(stderr, "pcretest: malloc(%d) failed for pbuffer16\n", pbuffer16_size);
    exit(1);
    }
  }
pp = pbuffer16;

while (len > 0)
  {
  uint32_t c;
  int chlen = utf82ord(p, &c);
  if (chlen <= 0) return -1;
  if (c > 0x10ffff) return -2;
  p += chlen;
  len -= chlen;
  if (c < 0x10000) *pp++ = c; else
    {
    if (!utf) return -3;
    c -= 0x10000;
    *pp++ = 0xD800 | (c >> 10);
    *pp++ = 0xDC00 | (c & 0x3ff);
    }
  }

*pp = 0;
return pp - pbuffer16;
}
#endif



#ifdef SUPPORT_PCRE32
/*************************************************
*         Convert a string to 32-bit             *
*************************************************/

/* The input is always interpreted as a string of UTF-8 bytes. If all the input
bytes are ASCII, the space needed for a 32-bit string is exactly four times the
8-bit size. Otherwise, the size needed for a 32-bit string is no more than four
times, because the number of characters must be less than the number of bytes.
The result is always left in pbuffer32.

Note that this function does not object to surrogate values. This is
deliberate; it makes it possible to construct UTF-32 strings that are invalid,
for the purpose of testing that they are correctly faulted.

Arguments:
  p          points to a byte string
  utf        true if UTF-8 (to be converted to UTF-32)
  len        number of bytes in the string (excluding trailing zero)

Returns:     number of 32-bit data items used (excluding trailing zero)
             OR -1 if a UTF-8 string is malformed
             OR -2 if a value > 0x10ffff is encountered in UTF mode
*/

static int
to32(uint8_t *p, int utf, int len)
{
uint32_t *pp;

if (pbuffer32_size < 4*len + 4)
  {
  if (pbuffer32 != NULL) free(pbuffer32);
  pbuffer32_size = 4*len + 4;
  pbuffer32 = (uint32_t *)malloc(pbuffer32_size);
  if (pbuffer32 == NULL)
    {
    fprintf(stderr, "pcretest: malloc(%d) failed for pbuffer32\n", pbuffer32_size);
    exit(1);
    }
  }
pp = pbuffer32;

while (len > 0)
  {
  uint32_t c;
  int chlen = utf82ord(p, &c);
  if (chlen <= 0) return -1;
  if (utf && c > 0x10ffff) return -2;
  p += chlen;
  len -= chlen;
  *pp++ = c;
  }

*pp = 0;
return pp - pbuffer32;
}
#endif /* SUPPORT_PCRE32 */



/*************************************************
*        Read or extend an input line            *
*************************************************/

/* Input lines are read into buffer, but both patterns and data lines can be
continued over multiple input lines. In addition, if the buffer fills up, we
want to automatically expand it so as to be able to handle extremely large
lines that are needed for certain stress tests. When the input buffer is
expanded, the other two buffers must also be expanded likewise, and the
contents of pbuffer, which are a copy of the input for callouts, must be
preserved (for when expansion happens for a data line). This is not the most
optimal way of handling this, but hey, this is just a test program!

Arguments:
  f            the file to read
  start        where in buffer to start (this *must* be within buffer)
  prompt       for stdin or readline()

Returns:       pointer to the start of new data
               could be a copy of start, or could be moved
               NULL if no data read and EOF reached
*/

static uint8_t *
extend_inputline(FILE *f, uint8_t *start, const char *prompt)
{
uint8_t *here = start;

for (;;)
  {
  size_t rlen = (size_t)(buffer_size - (here - buffer));

  if (rlen > 1000)
    {
    int dlen;

    /* If libreadline or libedit support is required, use readline() to read a
    line if the input is a terminal. Note that readline() removes the trailing
    newline, so we must put it back again, to be compatible with fgets(). */

#if defined(SUPPORT_LIBREADLINE) || defined(SUPPORT_LIBEDIT)
    if (isatty(fileno(f)))
      {
      size_t len;
      char *s = readline(prompt);
      if (s == NULL) return (here == start)? NULL : start;
      len = strlen(s);
      if (len > 0) add_history(s);
      if (len > rlen - 1) len = rlen - 1;
      memcpy(here, s, len);
      here[len] = '\n';
      here[len+1] = 0;
      free(s);
      }
    else
#endif

    /* Read the next line by normal means, prompting if the file is stdin. */

      {
      if (f == stdin) printf("%s", prompt);
      if (fgets((char *)here, rlen,  f) == NULL)
        return (here == start)? NULL : start;
      }

    dlen = (int)strlen((char *)here);
    if (dlen > 0 && here[dlen - 1] == '\n') return start;
    here += dlen;
    }

  else
    {
    int new_buffer_size = 2*buffer_size;
    uint8_t *new_buffer = (uint8_t *)malloc(new_buffer_size);
    uint8_t *new_pbuffer = (uint8_t *)malloc(new_buffer_size);

    if (new_buffer == NULL || new_pbuffer == NULL)
      {
      fprintf(stderr, "pcre2test: malloc(%d) failed\n", new_buffer_size);
      exit(1);
      }

    memcpy(new_buffer, buffer, buffer_size);
    memcpy(new_pbuffer, pbuffer, buffer_size);

    buffer_size = new_buffer_size;

    start = new_buffer + (start - buffer);
    here = new_buffer + (here - buffer);

    free(buffer);
    free(pbuffer);

    buffer = new_buffer;
    pbuffer = pbuffer8 = new_pbuffer;
    }
  }

/* Control never gets here */
}



/*************************************************
*         Case-independent strncmp() function    *
*************************************************/

/*
Arguments:
  s         first string
  t         second string
  n         number of characters to compare

Returns:    < 0, = 0, or > 0, according to the comparison
*/

static int
strncmpic(const uint8_t *s, const uint8_t *t, int n)
{
while (n--)
  {
  int c = tolower(*s++) - tolower(*t++);
  if (c) return c;
  }
return 0;
}



/*************************************************
*          Read number from string               *
*************************************************/

/* We don't use strtoul() because SunOS4 doesn't have it. Rather than mess
around with conditional compilation, just do the job by hand. It is only used
for unpicking arguments, so just keep it simple.

Arguments:
  str           string to be converted
  endptr        where to put the end pointer

Returns:        the unsigned long
*/

static int
get_value(const char *str, const char **endptr)
{
int result = 0;
while(*str != 0 && isspace(*str)) str++;
while (isdigit(*str)) result = result * 10 + (int)(*str++ - '0');
*endptr = str;
return(result);
}



/*************************************************
*          Scan the main modifier list           *
*************************************************/

/* This function searches the modifier list for a long modifier name.

Argument:
  p         start of the name
  lenp      length of the name

Returns:    an index in the modifier list, or -1 on failure
*/

static int
scan_modifiers(const uint8_t *p, unsigned int len)
{
int bot = 0;
int top = MODLISTCOUNT;

while (top > bot)
  {
  int mid = (bot + top)/2;
  unsigned int mlen = strlen(modlist[mid].name);
  int c = strncmp((char *)p, modlist[mid].name, (len < mlen)? len : mlen);
  if (c == 0)
    {
    if (len == mlen) return mid;
    c = len - mlen;
    }  
  if (c > 0) bot = mid + 1; else top = mid;
  }

return -1;

}



/*************************************************
*        Check a modifer and find its field      *
*************************************************/

/* This function is called when a modifier has been identified. We check that
it is allowed here and find the field that is to be changed.

Arguments:
  m          the modifier list entry
  ctx        CTX_PAT     => pattern context
             CTX_DEFPAT  => default pattern context
             CTX_DAT     => data context
             CTX_DEFDAT  => default data context
             CTX_DEFANY  => any default context (depends on the modifier)
  pctl       point to pattern control block
  dctl       point to data control block
  c          a single character or 0

Returns:     a field pointer or NULL
*/

static void *
check_modifier(modstruct *m, int ctx, patctl *pctl, datctl *dctl, uint32_t c)
{
void *field = NULL;
switch (m->which)
  {
  case MOD_CTC:
  if (ctx == CTX_DEFPAT || ctx == CTX_DEFANY) field = PTR(default_pat_context);
    else if (ctx == CTX_PAT) field = PTR(pat_context);
  break;

  case MOD_CTM:
  if (ctx == CTX_DEFDAT || ctx == CTX_DEFANY) field = PTR(default_dat_context);
    else if (ctx == CTX_DAT) field = PTR(dat_context);
  break;

  case MOD_DAT:
  if (dctl != NULL) field = dctl;
  break;

  case MOD_PAT:
  if (pctl != NULL) field = pctl;
  break;

  case MOD_PD:
  if (dctl != NULL) field = dctl;
    else if (pctl != NULL) field = pctl;
  break;
  }

if (field == NULL)
  {
  if (c == 0)
    fprintf(outfile, "** '%s' is not valid here\n", m->name);
  else
    fprintf(outfile, "** /%c is not valid here\n", c);
  return NULL;
  }

return (char *)field + m->offset;
}



/*************************************************
*            Decode a modifier list              *
*************************************************/

/* A pointers to a context or control block is NULL when called in cases when
that block is not relevant. They are never all relevant in one call. In
particular, at least one of patctl and datctl is always NULL.

Arguments:
  p          point to modifier string
  ctx        CTX_PAT     => pattern context
             CTX_DEFPAT  => default pattern context
             CTX_DAT     => data context
             CTX_DEFDAT  => default data context
             CTX_DEFANY  => any default context (depends on the modifier)
  pctl       point to pattern control block
  dctl       point to data control block

Returns: TRUE if successful decode, FALSE otherwise
*/

static BOOL
decode_modifiers(uint8_t *p, int ctx, patctl *pctl, datctl *dctl)
{
uint8_t *ep, *pp;
BOOL first = TRUE;

for (;;)
  {
  void *field;
  modstruct *m;
  BOOL off = FALSE;
  unsigned int i, len;
  int index;
  char *endptr;
  
  /* Skip white space and commas; after a comma we have passed the first
  item. */

  while (isspace(*p)) p++;
  if (*p == ',') first = FALSE;
  while (isspace(*p) || *p == ',') p++;
  if (*p == 0) break;

  /* Find the end of the item. */

  for (ep = p; *ep != 0 && *ep != ',' && !isspace(*ep); ep++);

  /* Remember if the first character is '-'. */

  if (*p == '-')
    {
    off = TRUE;
    p++;
    }
    
  /* Find the length of a full-length modifier name, and scan for it. */
   
  pp = p;
  while (pp < ep && *pp != '=') pp++; 
  index = scan_modifiers(p, pp - p);

  /* If the first modifier is unrecognized, try to interpret it as a sequence
  of single-character abbreviated modifiers. None of these modifiers have any
  associated data. They just set options or control bits. */

  if (index < 0)
    {
    uint32_t cc;

    if (!first)
      {
      fprintf(outfile, "** Unrecognized modifier '%.*s'\n", (int)(ep-p), p);
      if (ep - p == 1)
        fprintf(outfile, "** Single-character modifiers must come first\n");
      return FALSE;
      }

    for (cc = *p; cc != ',' && cc != '\n' && cc != 0; cc = *(++p))
      {
      if (p[1] == cc)           /* Handle doubled characters */
        {
        cc = (cc << 8) | cc;
        p++;
        }

      for (i = 0; i < C1MODLISTCOUNT; i++)
        if (cc == c1modlist[i].onechar) break;

      if (i >= C1MODLISTCOUNT)
        {
        fprintf(outfile, "** Unrecognized single-character modifier '%c'\n",
          *p);
        return FALSE;
        }

      if (c1modlist[i].index >= 0)
        {
        index = c1modlist[i].index;
        }

      else
        {
        index = scan_modifiers((uint8_t *)(c1modlist[i].fullname), 
          strlen(c1modlist[i].fullname));
        if (index < 0)
          {
          fprintf(outfile, "** Internal error: single-character equivalent "
            "modifier '%s' not found\n", c1modlist[i].fullname);
          return FALSE;
          }
        c1modlist[i].index = index;     /* Cache for next time */
        }

      field = check_modifier(modlist + index, ctx, pctl, dctl, *p);
      if (field == NULL) return FALSE;
      *((uint32_t *)field) |= modlist[index].value;
      }

    continue;    /* With tne next (fullname) modifier */
    }

  /* We have a match on a full-name modifier. Check for the existence of data
  when needed. */

  m = modlist + index;      /* Save typing */
  if (m->type != MOD_CTL && m->type != MOD_OPT)
    {
    if (*pp++ != '=')
      {
      fprintf(outfile, "** '=' expected after '%s'\n", m->name);
      return FALSE;
      }
    if (off)
      {
      fprintf(outfile, "** '-' is not valid for '%s'\n", m->name);
      return FALSE;
      }
    }

  /* These on/off types have no data. */

  else if (*pp != ',' && *pp != '\n' && *pp != 0)
    {
    fprintf(outfile, "** Unrecognized modifier '%.*s'\n", (int)(ep-p), p);
    return FALSE;
    }

  /* Set the data length for those types that have data. Then find the field
  that is to be set. If check_modifier() returns NULL, it has already output an
  error message. */

  len = ep - pp;
  field = check_modifier(m, ctx, pctl, dctl, 0);
  if (field == NULL) return FALSE;

  /* Process according to data type. */

  switch (m->type)
    {
    case MOD_CTL:
    case MOD_OPT:
    if (off) *((uint32_t *)field) &= ~m->value;
      else *((uint32_t *)field) |= m->value;
    break;

    case MOD_BSR:
    if (len == 7 && strncmpic(pp, (const uint8_t *)"anycrlf", 7) == 0)
      *((uint16_t *)field) = PCRE2_BSR_ANYCRLF;
    else if (len == 7 && strncmpic(pp, (const uint8_t *)"unicode", 7) == 0)
      *((uint16_t *)field) = PCRE2_BSR_UNICODE;
    else goto INVALID_VALUE;
    pp = ep;
    break;

    case MOD_IN2:
    if (!isdigit(*pp)) goto INVALID_VALUE;
    ((uint32_t *)field)[0] = (uint32_t)strtoul((const char *)pp, &endptr, 10);
    if (*endptr == '/')
      ((uint32_t *)field)[1] = (uint32_t)strtoul((const char *)endptr+1, &endptr, 10);
    else ((uint32_t *)field)[1] = 0;
    pp = (uint8_t *)endptr;
    break;

    case MOD_INT:
    if (!isdigit(*pp)) goto INVALID_VALUE;
    *((uint32_t *)field) = (uint32_t)strtoul((const char *)pp, &endptr, 10);
    pp = (uint8_t *)endptr;
    break;

    case MOD_NL:
    for (i = 0; i < sizeof(newlines)/sizeof(char *); i++)
      if (len == strlen(newlines[i]) &&
        strncmpic(pp, (const uint8_t *)newlines[i], len) == 0) break;
    if (i >= sizeof(newlines)/sizeof(char *)) goto INVALID_VALUE;
    *((uint16_t *)field) = i;
    pp = ep;
    break;

    case MOD_NN:              /* Name or number; may be several */
    if (isdigit(*pp))
      {
      int ct = MAXCPYGET - 1;
      field = (char *)field - m->offset + m->value;   /* Adjust field ptr */
      while (*((uint32_t *)field) != 0 && ct-- > 0)    /* Skip previous */
        field = (char *)field + sizeof(uint32_t);
      if (ct <= 0)
        {
        fprintf(outfile, "** Too many numeric '%s' modifiers\n", m->name);
        return FALSE;
        }
      *((uint32_t *)field) = (uint32_t)strtoul((const char *)pp, &endptr, 10);
      pp = (uint8_t *)endptr;
      }

    /* Multiple strings are put end to end. */

    else
      {
      char *nn = (char *)field;
      while (*nn != 0) nn += strlen(nn) + 1;
      if (nn + len + 1 - (char *)field > LENCPYGET)
        {
        fprintf(outfile, "** Too many named '%s' modifiers\n", m->name);
        return FALSE;
        }
      memcpy(nn, pp, len);
      nn[len] = 0 ;
      nn[len+1] = 0;
      pp = ep;
      }
    break;

    case MOD_STR:
    memcpy(field, pp, len);
    ((uint8_t *)field)[len] = 0;
    pp = ep;
    break;
    }

  if (*pp != ',' && *pp != '\n' && *pp != 0)
    {
    fprintf(outfile, "** Comma expected after modifier item '%s'\n", m->name);
    return FALSE;
    }

  p = pp;
  }

return TRUE;

INVALID_VALUE:
fprintf(outfile, "** Invalid value in '%.*s'\n", (int)(ep-p), p);
return FALSE;
}


/*************************************************
*             Get info from a pattern            *
*************************************************/

/* A wrapped call to pcre2_pattern_info(), applied to the current compiled
pattern.

Arguments:
  what        code for the required information
  where       where to put the answer

Returns:      the return from pcre2_pattern_info()
*/

static int
pattern_info(int what, void *where)
{
int rc;
PCRE2_PATTERN_INFO(rc, compiled_code, what, where);
if (rc >= 0 || rc == PCRE2_ERROR_UNSET) return 0;
fprintf(outfile, "Error %d from pcre2_pattern_info_%d(%d)\n", rc, test_mode,
  what);
if (rc == PCRE2_ERROR_BADMODE)
  fprintf(outfile, "Running in %d-bit mode but pattern was compiled in "
    "%d-bit mode\n", test_mode,
    8 * (FLD(compiled_code, flags) & PCRE2_MODE_MASK));
return rc;
}



/*************************************************
*             Show something in a list           *
*************************************************/

/* This function just helps to keep the code that uses it tidier. It's used for 
various lists of things where there needs to be introductory text before the 
first item. */

static void
prmsg(const char **msg, const char *s)
{
fprintf(outfile, "%s %s", *msg, s);
*msg = "";
}



/*************************************************
*                Show compile controls           *
*************************************************/

/* Called for unsupported POSIX modifiers. */

static void
show_compile_controls(uint32_t controls)
{
fprintf(outfile, "%s%s%s%s%s%s%s%s%s%s%s%s%s",
  ((controls & CTL_AFTERTEXT) != 0)? " aftertext" : "",
  ((controls & CTL_ALLAFTERTEXT) != 0)? " allaftertext" : "",
  ((controls & CTL_ALLCAPTURES) != 0)? " allcaptures" : "",
  ((controls & CTL_ALTGLOBMATCH) != 0)? " altglobmatch" : "",
  ((controls & CTL_BYTECODE) != 0)? " bytecode" : "",
  ((controls & CTL_FLIPBYTES) != 0)? " flipbytes" : "",
  ((controls & CTL_FULLBYTECODE) != 0)? " fullbytecode" : "",
  ((controls & CTL_GLOBMATCH) != 0)? " globmatch" : "",
  ((controls & CTL_INFO) != 0)? " info" : "",
  ((controls & CTL_JITVERIFY) != 0)? " jitverify" : "",
  ((controls & CTL_MARKS) != 0)? " marks" : "",
  ((controls & CTL_PERLCOMPAT) != 0)? " perlcompat" : "",
  ((controls & CTL_POSIX) != 0)? " posix" : "");
}



/*************************************************
*                Show compile options            *
*************************************************/

/* Called from show_pattern_info() and for unsupported POSIX options. */

static void
show_compile_options(uint32_t options)
{
fprintf(outfile, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
  ((options & PCRE2_ANCHORED) != 0)? " anchored" : "",
  ((options & PCRE2_CASELESS) != 0)? " caseless" : "",
  ((options & PCRE2_EXTENDED) != 0)? " extended" : "",
  ((options & PCRE2_MULTILINE) != 0)? " multiline" : "",
  ((options & PCRE2_FIRSTLINE) != 0)? " firstline" : "",
  ((options & PCRE2_DOTALL) != 0)? " dotall" : "",
  ((options & PCRE2_DOLLAR_ENDONLY) != 0)? " dollar_endonly" : "",
  ((options & PCRE2_UNGREEDY) != 0)? " ungreedy" : "",
  ((options & PCRE2_NO_AUTO_CAPTURE) != 0)? " no_auto_capture" : "",
  ((options & PCRE2_NO_AUTO_POSSESS) != 0)? " no_auto_possessify" : "",
  ((options & PCRE2_UTF) != 0)? " utf" : "",
  ((options & PCRE2_UCP) != 0)? " ucp" : "",
  ((options & PCRE2_NO_UTF_CHECK) != 0)? " no_utf_check" : "",
  ((options & PCRE2_NO_START_OPTIMIZE) != 0)? " no_start_optimize" : "",
  ((options & PCRE2_DUPNAMES) != 0)? " dupnames" : "",
  ((options & PCRE2_ALT_BSUX) != 0)? " alt_bsux" : "",
  ((options & PCRE2_ALLOW_EMPTY_CLASS) != 0)? " allow_empty_class" : "",
  ((options & PCRE2_AUTO_CALLOUT) != 0)? " auto_callout" : "",
  ((options & PCRE2_MATCH_UNSET_BACKREF) != 0)? " match_unset_backref" : "",
  ((options & PCRE2_NEVER_UCP) != 0)? " never_ucp" : "",
  ((options & PCRE2_NEVER_UTF) != 0)? " never_utf" : "");
}



/*************************************************
*                Show match controls           *
*************************************************/

/* Called for unsupported POSIX modifiers. */

static void
show_match_controls(uint32_t controls)
{
fprintf(outfile, "%s%s%s%s%s%s",
  ((controls & CTL_CALLOUT_CAPTURE) != 0)? " callout_capture" : "",
  ((controls & CTL_CALLOUT_NONE) != 0)? " callout_none" : "",
  ((controls & CTL_DFA) != 0)? " dfa" : "",
  ((controls & CTL_GETLIST) != 0)? " getlist" : "",
  ((controls & CTL_LIMITS) != 0)? " limits" : "",
  ((controls & CTL_MEMORY) != 0)? " memory" : "");
}



/*************************************************
*                Show match options              *
*************************************************/

/* Called for unsupported POSIX options. */

static void
show_match_options(uint32_t options)
{
fprintf(outfile, "%s%s%s%s%s%s%s%s%s%s%s",
  ((options & PCRE2_ANCHORED) != 0)? " anchored" : "",
  ((options & PCRE2_DFA_RESTART) != 0)? " dfa_restart" : "",
  ((options & PCRE2_DFA_SHORTEST) != 0)? " dfa_shortest" : "",
  ((options & PCRE2_NO_START_OPTIMIZE) != 0)? " no_start_optimize" : "",
  ((options & PCRE2_NO_UTF_CHECK) != 0)? " no_utf_check" : "",
  ((options & PCRE2_NOTBOL) != 0)? " notbol" : "",
  ((options & PCRE2_NOTEMPTY) != 0)? " notempty" : "",
  ((options & PCRE2_NOTEMPTY_ATSTART) != 0)? " notempty_atstart" : "",
  ((options & PCRE2_NOTEOL) != 0)? " noteol" : "",
  ((options & PCRE2_PARTIAL_HARD) != 0)? " partial_hard" : "",
  ((options & PCRE2_PARTIAL_SOFT) != 0)? " partial_soft" : "");
}



/*************************************************
*        Show information about a pattern        *
*************************************************/

/* This function is called after a pattern has been compiled or loaded from a
file, if any of the information-requesting controls have been set.

Arguments:  none

Returns:    PR_OK     continue processing next line
            PR_SKIP   skip to a blank line
            PR_ABEND  abort the pcre2test run
*/

static int
show_pattern_info(void)
{
uint32_t compile_options, pattern_options;

if ((pat_patctl.control & (CTL_BYTECODE|CTL_FULLBYTECODE)) != 0)
  {
  fprintf(outfile, "------------------------------------------------------------------\n");


  fprintf(outfile, "Code output not yet available\n");
#ifdef FIXME
  PCRE2_PRINTINT(outfile, (pat_patctl.control & CTL_FULLBYTECODE) != 0);
#endif
  }

if ((pat_patctl.control & CTL_INFO) != 0)
  {
  const void *nametable;
  const uint8_t *start_bits;
  int count, backrefmax, first_ctype, last_ctype, jchanged,
    hascrorlf, maxlookbehind, match_empty, minlength;
  int nameentrysize, namecount;
  uint32_t bsr_convention, newline_convention;
  uint32_t first_cunit, last_cunit;
  uint32_t match_limit = 0, recursion_limit = 0;

  /* These info requests should always succeed. */

  if (pattern_info(PCRE2_INFO_BACKREFMAX, &backrefmax) +
      pattern_info(PCRE2_INFO_BSR_CONVENTION, &bsr_convention) +
      pattern_info(PCRE2_INFO_CAPTURECOUNT, &count) +
      pattern_info(PCRE2_INFO_FIRSTBITMAP, &start_bits) +
      pattern_info(PCRE2_INFO_FIRSTCODEUNIT, &first_cunit) +
      pattern_info(PCRE2_INFO_FIRSTCODETYPE, &first_ctype) +
      pattern_info(PCRE2_INFO_HASCRORLF, &hascrorlf) +
      pattern_info(PCRE2_INFO_JCHANGED, &jchanged) +
      pattern_info(PCRE2_INFO_LASTCODEUNIT, &last_cunit) +
      pattern_info(PCRE2_INFO_LASTCODETYPE, &last_ctype) +
      pattern_info(PCRE2_INFO_MATCH_EMPTY, &match_empty) +
      pattern_info(PCRE2_INFO_MATCH_LIMIT, &match_limit) +
      pattern_info(PCRE2_INFO_MAXLOOKBEHIND, &maxlookbehind) +
      pattern_info(PCRE2_INFO_MINLENGTH, &minlength) +
      pattern_info(PCRE2_INFO_NAMECOUNT, &namecount) +
      pattern_info(PCRE2_INFO_NAMEENTRYSIZE, &nameentrysize) +
      pattern_info(PCRE2_INFO_NAMETABLE, &nametable) +
      pattern_info(PCRE2_INFO_NEWLINE_CONVENTION, &newline_convention) +
      pattern_info(PCRE2_INFO_RECURSION_LIMIT, &recursion_limit)
      != 0)
    return PR_ABEND;

  fprintf(outfile, "Capturing subpattern count = %d\n", count);

  if (backrefmax > 0)
    fprintf(outfile, "Max back reference = %d\n", backrefmax);

  if (maxlookbehind > 0)
    fprintf(outfile, "Max lookbehind = %d\n", maxlookbehind);

  if (match_limit > 0)
    fprintf(outfile, "Match limit = %u\n", match_limit);

  if (recursion_limit > 0)
    fprintf(outfile, "Recursion limit = %u\n", recursion_limit);

  if (namecount > 0)
    {
    fprintf(outfile, "Named capturing subpatterns:\n");
    while (namecount-- > 0)
      {
      int imm2_size = test_mode == PCRE8_MODE ? 2 : 1;
      int length = (int)STRLEN(nametable + imm2_size);
      fprintf(outfile, "  ");
      PCHARSV(nametable, imm2_size, length, FALSE, outfile);
      while (length++ < nameentrysize - imm2_size) putc(' ', outfile);
#ifdef SUPPORT_PCRE32
      if (test_mode == PCRE32_MODE)
        fprintf(outfile, "%3d\n", (int)(((PCRE2_SPTR32)nametable)[0]));
#endif
#ifdef SUPPORT_PCRE16
      if (test_mode == PCRE16_MODE)
        fprintf(outfile, "%3d\n", (int)(((PCRE2_SPTR16)nametable)[0]));
#endif
#ifdef SUPPORT_PCRE8
      if (test_mode == PCRE8_MODE)
        fprintf(outfile, "%3d\n", (int)(
        ((((PCRE2_SPTR8)nametable)[0]) << 8) | ((PCRE2_SPTR8)nametable)[1]));
#endif
      nametable = (void*)((PCRE2_SPTR8)nametable + nameentrysize * code_unit_size);
      }
    }

  if (hascrorlf)   fprintf(outfile, "Contains explicit CR or LF match\n");
  if (match_empty) fprintf(outfile, "May match empty string\n");

  pattern_info(PCRE2_INFO_COMPILE_OPTIONS, &compile_options);
  if (compile_options == 0) fprintf(outfile, "No options\n"); else
    {
    fprintf(outfile, "Options: ");
    show_compile_options(compile_options);
    fprintf(outfile, "\n"); 
    }    

  pattern_info(PCRE2_INFO_PATTERN_OPTIONS, &pattern_options);
  if (pattern_options != 0) fprintf(outfile, "Pattern options:FIXME\n");
    {
/* FIXME */     
    }

  if (jchanged) fprintf(outfile, "Duplicate name status changes\n");

  if (bsr_convention != PCRE2_BSR_DEFAULT)
    fprintf(outfile, "\\R matches %s\n", (bsr_convention == PCRE2_BSR_UNICODE)?
      "any Unicode newline" : "CR, LF, or CRLF");

  switch (newline_convention)
    {
    case PCRE2_NEWLINE_CR:
    fprintf(outfile, "Newline is CR\n");
    break;

    case PCRE2_NEWLINE_LF:
    fprintf(outfile, "Newline is LF\n");
    break;

    case PCRE2_NEWLINE_CRLF:
    fprintf(outfile, "Newline is CRLF\n");
    break;

    case PCRE2_NEWLINE_ANYCRLF:
    fprintf(outfile, "Newline is CR, LF, or CRLF\n");
    break;

    case PCRE2_NEWLINE_ANY:
    fprintf(outfile, "Newline is any Unicode newline\n");
    break;

    default:
    break;
    }

  if (first_ctype == 2)
    {
    fprintf(outfile, "First char at start or follows newline\n");
    }
  else if (first_ctype == 1)
    {
    const char *caseless =
      ((FLD(compiled_code, flags) & PCRE2_FIRSTCASELESS) == 0)?
      "" : " (caseless)";
    if (PRINTOK(first_cunit))
      fprintf(outfile, "First code unit = \'%c\'%s\n", first_cunit, caseless);
    else
      {
      fprintf(outfile, "First code unit = ");
      pchar(first_cunit, FALSE, outfile);
      fprintf(outfile, "%s\n", caseless);
      }
    }
  else
    {
    fprintf(outfile, "No first code unit\n");
    }

  if (last_ctype == 0)
    {
    fprintf(outfile, "No last code unit\n");
    }
  else
    {
    const char *caseless =
      ((FLD(compiled_code, flags) & PCRE2_LASTCASELESS) == 0)?
      "" : " (caseless)";
    if (PRINTOK(last_cunit))
      fprintf(outfile, "Last code unit = \'%c\'%s\n", last_cunit, caseless);
    else
      {
      fprintf(outfile, "Last code unit = ");
      pchar(last_cunit, FALSE, outfile);
      fprintf(outfile, "%s\n", caseless);
      }
    }

  fprintf(outfile, "Subject length lower bound = %d\n", minlength);

  if (start_bits == NULL)
    fprintf(outfile, "No starting code unit list\n");
  else
    {
    int i;
    int c = 24;
    fprintf(outfile, "Starting code units: ");
    for (i = 0; i < 256; i++)
      {
      if ((start_bits[i/8] & (1<<(i&7))) != 0)
        {
        if (c > 75)
          {
          fprintf(outfile, "\n  ");
          c = 2;
          }
        if (PRINTOK(i) && i != ' ')
          {
          fprintf(outfile, "%c ", i);
          c += 2;
          }
        else
          {
          fprintf(outfile, "\\x%02x ", i);
          c += 5;
          }
        }
      }
    fprintf(outfile, "\n");
    }

  if (pat_patctl.jit != 0)
    {
    size_t jitsize;
    if (pattern_info(PCRE2_INFO_JITSIZE, &jitsize) == 0)
      {
      if (jitsize > 0)
        fprintf(outfile, "JIT study was successful\n");
      else
#ifdef SUPPORT_JIT
        fprintf(outfile, "JIT study was not successful\n");
#else
        fprintf(outfile, "JIT support is not available in this version of PCRE\n");
#endif
      }
    }
  }

return PR_OK;
}



/*************************************************
*               Process command line             *
*************************************************/

/* This function is called for lines beginning with # and a character that is
not ! or whitespace, when encountered between tests. The line is in buffer.

Arguments:  none

Returns:    PR_OK     continue processing next line
            PR_SKIP   skip to a blank line
            PR_ABEND  abort the pcre2test run
*/

static int
process_command(void)
{
if (strncmp((char *)buffer, "#pattern", 8) == 0 && isspace(buffer[8]))
  {
  (void)decode_modifiers(buffer + 8, CTX_DEFPAT, &def_patctl, NULL);
  }
else if (strncmp((char *)buffer, "#data", 5) == 0 && isspace(buffer[5]))
  {
  (void)decode_modifiers(buffer + 5, CTX_DEFDAT, NULL, &def_datctl);
  }
else if (strncmp((char *)buffer, "#load", 5) == 0 && isspace(buffer[5]))
  {
/* FIXME */
fprintf(outfile, "** #load not yet implemented\n");
return PR_ABEND;

#ifdef FIXME


/* See if the pattern is to be loaded pre-compiled from a file. */

if (*p == '<' && strchr((char *)(p+1), '<') == NULL)
  {
  uint32_t magic;
  uint8_t sbuf[8];
  FILE *f;

  p++;
  if (*p == '!')
    {
    do_debug = TRUE;
    do_showinfo = TRUE;
    p++;
    }

  pp = p + (int)strlen((char *)p);
  while (isspace(pp[-1])) pp--;
  *pp = 0;

  f = fopen((char *)p, "rb");
  if (f == NULL)
    {
    fprintf(outfile, "Failed to open %s: %s\n", p, strerror(errno));
    continue;
    }
  if (fread(sbuf, 1, 8, f) != 8) goto FAIL_READ;

  true_size =
    (sbuf[0] << 24) | (sbuf[1] << 16) | (sbuf[2] << 8) | sbuf[3];
  true_study_size =
    (sbuf[4] << 24) | (sbuf[5] << 16) | (sbuf[6] << 8) | sbuf[7];

  re = (pcre *)new_malloc(true_size);
  if (re == NULL)
    {
    printf("** Failed to get %d bytes of memory for pcre object\n",
      (int)true_size);
    yield = 1;
    goto EXIT;
    }
  if (fread(re, 1, true_size, f) != true_size) goto FAIL_READ;

  magic = REAL_PCRE_MAGIC(re);
  if (magic != MAGIC_NUMBER)
    {
    if (swap_uint32(magic) == MAGIC_NUMBER)
      {
      do_flip = 1;
      }
    else
      {
      fprintf(outfile, "Data in %s is not a compiled PCRE regex\n", p);
      new_free(re);
      fclose(f);
      continue;
      }
    }

  /* We hide the byte-invert info for little and big endian tests. */
  fprintf(outfile, "Compiled pattern%s loaded from %s\n",
    do_flip && (p[-1] == '<') ? " (byte-inverted)" : "", p);

  /* Now see if there is any following study data. */

  if (true_study_size != 0)
    {
    pcre_study_data *psd;

    extra = (pcre_extra *)new_malloc(sizeof(pcre_extra) + true_study_size);
    extra->flags = PCRE_EXTRA_STUDY_DATA;

    psd = (pcre_study_data *)(((char *)extra) + sizeof(pcre_extra));
    extra->study_data = psd;

    if (fread(psd, 1, true_study_size, f) != true_study_size)
      {
      FAIL_READ:
      fprintf(outfile, "Failed to read data from %s\n", p);
      if (extra != NULL)
        {
        PCRE_FREE_STUDY(extra);
        }
      new_free(re);
      fclose(f);
      continue;
      }
    fprintf(outfile, "Study data loaded from %s\n", p);
    do_study = 1;     /* To get the data output if requested */
    }
  else fprintf(outfile, "No study data\n");

  /* Flip the necessary bytes. */
  if (do_flip)
    {
    int rc;
    PCRE_PATTERN_TO_HOST_BYTE_ORDER(rc, re, extra, NULL);
    if (rc == PCRE_ERROR_BADMODE)
      {
      uint32_t flags_in_host_byte_order;
      if (REAL_PCRE_MAGIC(re) == MAGIC_NUMBER)
        flags_in_host_byte_order = REAL_PCRE_FLAGS(re);
      else
        flags_in_host_byte_order = swap_uint32(REAL_PCRE_FLAGS(re));
      /* Simulate the result of the function call below. */
      fprintf(outfile, "Error %d from pcre%s_fullinfo(%d)\n", rc,
        test_mode == PCRE32_MODE ? "32" : test_mode == PCRE16_MODE ? "16" : "",
        PCRE_INFO_OPTIONS);
      fprintf(outfile, "Running in %d-bit mode but pattern was compiled in "
        "%d-bit mode\n", test_mode, 8 * (flags_in_host_byte_order & test_mode_MASK));
      new_free(re);
      fclose(f);
      continue;
      }
    }

  /* Need to know if UTF-8 for printing data strings. */

  if (new_info(re, NULL, PCRE_INFO_OPTIONS, &get_options) < 0)
    {
    new_free(re);
    fclose(f);
    continue;
    }
  use_utf = (get_options & PCRE_UTF8) != 0;

  fclose(f);
  goto SHOW_INFO;
  }

#endif  /* FIXME */


  }
return PR_OK;
}



/*************************************************
*               Process pattern line             *
*************************************************/

/* This function is called when the input buffer contains the start of a
pattern. The first character is known to be a valid delimiter. The pattern is
read, modifiers are interpreted, and a suitable local context is set up for
this test. The pattern is then compiled.

Arguments:  none

Returns:    PR_OK     continue processing next line
            PR_SKIP   skip to a blank line
            PR_ABEND  abort the pcre2test run
*/

static int
process_pattern(void)
{
uint8_t *p = buffer;
unsigned int delimiter = *p++;
int patlen, errorcode;
size_t erroroffset;

/* Initialize the context and pattern/data controls for this test from the
defaults. */

PATCTXCPY(pat_context, default_pat_context);
memcpy(&pat_patctl, &def_patctl, sizeof(patctl));

/* Find the end of the pattern, reading more lines if necessary. */

for(;;)
  {
  while (*p != 0)
    {
    if (*p == '\\' && p[1] != 0) p++;
      else if (*p == delimiter) break;
    p++;
    }
  if (*p != 0) break;
  if ((p = extend_inputline(infile, p, "    > ")) == NULL)
    {
    fprintf(outfile, "** Unexpected EOF\n");
    return PR_ABEND;
    }
  if (infile != stdin) fprintf(outfile, "%s", (char *)p);
  }

/* If the first character after the delimiter is backslash, make
the pattern end with backslash. This is purely to provide a way
of testing for the error message when a pattern ends with backslash. */

if (p[1] == '\\') *p++ = '\\';

/* Terminate the pattern at the delimiter, and save a copy of the pattern
for callouts. */

*p++ = 0;
patlen = p - buffer - 1;
strncpy((char *)pbuffer, (char *)(buffer+1), patlen);

/* Look for modifiers and options after the final delimiter. If successful,
compile the pattern. */

if (!decode_modifiers(p, CTX_PAT, &pat_patctl, NULL)) return PR_SKIP;

/* Handle compiling via the POSIX interface, which doesn't support the
timing, showing, or debugging options, nor the ability to pass over
local character tables. Neither does it have 16-bit or 32-bit support. */

if ((pat_patctl.control & CTL_POSIX) != 0)
  {
  int rc;
  int cflags = 0;
  const char *msg = "** Ignored with POSIX interface:"; 
  
  if (test_mode != 8)
    {
    fprintf(outfile, "** The POSIX interface is available only in 8-bit mode\n");
    return PR_SKIP;
    }
  
#ifdef SUPPORT_PCRE8   
  /* Check for features that the POSIX interface does not support. */
  
  if (pat_patctl.locale[0] != 0) prmsg(&msg, "locale");
  if (pat_patctl.tables_id != 0) prmsg(&msg, "tables");
  if (pat_patctl.stackguard_test != 0) prmsg(&msg, "stackguard");
  if (timeit > 0) prmsg(&msg, "timing");
  if (pat_patctl.jit != 0) prmsg(&msg, "JIT");
  if (pat_patctl.save[0] != 0) prmsg(&msg, "save");
   
  if ((pat_patctl.options & ~POSIX_SUPPORTED_COMPILE_OPTIONS) != 0)
    {
    fprintf(outfile, "%s", msg);
    show_compile_options(pat_patctl.options & ~POSIX_SUPPORTED_COMPILE_OPTIONS);
    msg = "";
    }  
  if ((pat_patctl.control & ~POSIX_SUPPORTED_COMPILE_CONTROLS) != 0)
    {
    fprintf(outfile, "%s", msg);
    show_compile_controls(pat_patctl.control & ~POSIX_SUPPORTED_COMPILE_CONTROLS);
    msg = "";
    }  
    
  if (msg[0] == 0) fprintf(outfile, "\n");
   
  /* Translate PCRE2 options to POSIX options and then compile. On success, set 
  up a match_data block to be used for all matches. */

  if ((pat_patctl.options & PCRE2_UTF) != 0) cflags |= REG_UTF;
  if ((pat_patctl.options & PCRE2_UCP) != 0) cflags |= REG_UCP;
  if ((pat_patctl.options & PCRE2_CASELESS) != 0) cflags |= REG_ICASE;
  if ((pat_patctl.options & PCRE2_MULTILINE) != 0) cflags |= REG_NEWLINE;
  if ((pat_patctl.options & PCRE2_DOTALL) != 0) cflags |= REG_DOTALL;
  if ((pat_patctl.options & PCRE2_NO_AUTO_CAPTURE) != 0) cflags |= REG_NOSUB;
  if ((pat_patctl.options & PCRE2_UNGREEDY) != 0) cflags |= REG_UNGREEDY;

  rc = regcomp(&preg, (char *)pbuffer, cflags);
  if (rc != 0)   /* Failure */
    {
    (void)regerror(rc, &preg, (char *)buffer, buffer_size);
    fprintf(outfile, "Failed: POSIX code %d: %s\n", rc, buffer);
    return PR_SKIP;
    }
  return PR_OK;
#endif  /* SUPPORT_PCRE8 */
  }

/* Handle compiling via the native interface, converting the input in non-8-bit
modes. */

#ifdef SUPPORT_PCRE16
if (test_mode == PCRE16_MODE)
  patlen = to16(pbuffer, pat_patctl.options & PCRE2_UTF,
    (int)strlen((char *)pbuffer));
#endif

#ifdef SUPPORT_PCRE32
if (test_mode == PCRE32_MODE)
  patlen = to32(pbuffer, pat_patctl.options & PCRE2_UTF,
    (int)strlen((char *)pbuffer));
#endif

switch(patlen)
  {
  case -1:
  fprintf(outfile, "** Failed: invalid UTF-8 string cannot be "
    "converted to %d-bit string\n", (test_mode == PCRE16_MODE)? 16:32);
  return PR_SKIP;

  case -2:
  fprintf(outfile, "** Failed: character value greater than 0x10ffff "
    "cannot be converted to UTF\n");
  return PR_SKIP;

  case -3:
  fprintf(outfile, "** Failed: character value greater than 0xffff "
    "cannot be converted to 16-bit in non-UTF mode\n");
  return PR_SKIP;

  default:
  break;
  }

/* Compile many times when timing */

if (timeit > 0)
  {
  register int i;
  clock_t time_taken;
  clock_t start_time = clock();
  for (i = 0; i < timeit; i++)
    {
    PCRE2_COMPILE(compiled_code, pbuffer, patlen,
      pat_patctl.options, &errorcode, &erroroffset, pat_context);
    if (TEST(compiled_code, !=, NULL))
      { SUB1(pcre2_code_free, compiled_code); }
    }
  total_compile_time += (time_taken = clock() - start_time);
  fprintf(outfile, "Compile time %.4f milliseconds\n",
    (((double)time_taken * 1000.0) / (double)timeit) /
      (double)CLOCKS_PER_SEC);
  }

/* FIXME: implement timing for JIT compile. */

/* A final compile that is used "for real". */

PCRE2_COMPILE(compiled_code, pbuffer, patlen, pat_patctl.options, &errorcode,
  &erroroffset, pat_context);

/* Compilation failed; go back for another re, skipping to blank line
if non-interactive. */

if (TEST(compiled_code, ==, NULL))
  {
  fprintf(outfile, "Failed: %d at offset %d\n", errorcode, (int)erroroffset);

/* FIXME get error message */

  return PR_SKIP;
  }

/* Call the JIT compiler if requested. */

if (pat_patctl.jit != 0)
  { PCRE2_JIT_COMPILE(compiled_code, pat_patctl.jit); }

/* Output code size and other information if requested. */

if ((pat_patctl.control & CTL_MEMORY) != 0)
  {
  size_t size;
  size_t name_entry_size;
  int name_count;
  (void)pattern_info(PCRE2_INFO_SIZE, &size);
  (void)pattern_info(PCRE2_INFO_NAMECOUNT, &name_count);
  (void)pattern_info(PCRE2_INFO_NAMEENTRYSIZE, &name_entry_size);
  fprintf(outfile, "Memory allocation (code space): %d\n",
    (int)(size - name_count * name_entry_size -
      FLD(compiled_code, name_table_offset)));
  if (pat_patctl.jit != 0)
    {
    (void)pattern_info(PCRE2_INFO_JITSIZE, &size);
    fprintf(outfile, "Memory allocation (JIT code): %d\n", (int)size);
    }
  }

if ((pat_patctl.control & CTL_ANYINFO) != 0)
  {
  int rc = show_pattern_info();
  if (rc != PR_OK) return rc;
  }


#ifdef FIXME

/* If the '>' option was present, we write out the regex to a file, and
that is all. The first 8 bytes of the file are the regex length and then
the study length, in big-endian order. */

if (to_file != NULL)
  {
  FILE *f = fopen((char *)to_file, "wb");
  if (f == NULL)
    {
    fprintf(outfile, "Unable to open %s: %s\n", to_file, strerror(errno));
    }
  else
    {
    uint8_t sbuf[8];

/* Extract the size for possible writing before possibly flipping it,
and remember the store that was got. */

true_size = REAL_PCRE_SIZE(re);

    if (do_flip) regexflip(re, extra);
    sbuf[0] = (uint8_t)((true_size >> 24) & 255);
    sbuf[1] = (uint8_t)((true_size >> 16) & 255);
    sbuf[2] = (uint8_t)((true_size >>  8) & 255);
    sbuf[3] = (uint8_t)((true_size) & 255);
    sbuf[4] = (uint8_t)((true_study_size >> 24) & 255);
    sbuf[5] = (uint8_t)((true_study_size >> 16) & 255);
    sbuf[6] = (uint8_t)((true_study_size >>  8) & 255);
    sbuf[7] = (uint8_t)((true_study_size) & 255);

    if (fwrite(sbuf, 1, 8, f) < 8 ||
        fwrite(re, 1, true_size, f) < true_size)
      {
      fprintf(outfile, "Write error on %s: %s\n", to_file, strerror(errno));
      }
    else
      {
      fprintf(outfile, "Compiled pattern written to %s\n", to_file);

      /* If there is study data, write it. */

      if (extra != NULL)
        {
        if (fwrite(extra->study_data, 1, true_study_size, f) <
            true_study_size)
          {
          fprintf(outfile, "Write error on %s: %s\n", to_file,
            strerror(errno));
          }
        else fprintf(outfile, "Study data written to %s\n", to_file);
        }
      }
    fclose(f);
    }

  new_free(re);
  if (extra != NULL)
    {
    PCRE_FREE_STUDY(extra);
    }
  if (locale_set)
    {
    new_free((void *)tables);
    setlocale(LC_CTYPE, "C");
    locale_set = FALSE;
    }
  continue;  /* With next regex */
  }

#endif /* FIXME */





return PR_OK;
}




/*************************************************
*               Process data line                *
*************************************************/

/* The line is in buffer; it will not be empty.

Arguments:  none

Returns:    PR_OK     continue processing next line
            PR_SKIP   skip to a blank line
            PR_ABEND  abort the pcre2test run
*/

static int
process_data(void)
{
int len;
uint32_t c;
uint8_t *p, *pp, *start_dup;
size_t needlen;
BOOL utf;

#ifdef SUPPORT_PCRE8
uint8_t *q8;
#endif
#ifdef SUPPORT_PCRE16
uint16_t *q16;
#endif
#ifdef SUPPORT_PCRE32
uint32_t *q32;
#endif

DATCTXCPY(dat_context, default_dat_context);
memcpy(&dat_datctl, &def_datctl, sizeof(datctl));

utf = (pat_patctl.control & CTL_POSIX) == 0 &&
  (FLD(compiled_code, compile_options) & PCRE2_UTF) != 0;
start_dup = NULL;
len = strlen((const char *)buffer);
while (len > 0 && isspace(buffer[len-1])) len--;
buffer[len] = 0;
p = buffer;
while (isspace(*p)) p++;

/* Check that the data is well-formed UTF-8 if we're in UTF mode. To create
invalid input to pcre2_exec, you must use \x?? or \x{} sequences. */

if (utf)
  {
  uint8_t *q;
  uint32_t cc;
  int n = 1;
  for (q = p; n > 0 && *q; q += n) n = utf82ord(q, &cc);
  if (n <= 0)
    {
    fprintf(outfile, "** Failed: invalid UTF-8 string cannot be used as input "
      "in UTF mode\n");
    return PR_OK;
    }
  }

#ifdef SUPPORT_VALGRIND
/* Mark the dbuffer as addressable but undefined again. */
if (dbuffer != NULL)
  {
  VALGRIND_MAKE_MEM_UNDEFINED(dbuffer, dbuffer_size);
  }
#endif

/* Allocate a buffer to hold the data line; len+1 is an upper bound on
the number of code units that will be needed. */

needlen = (size_t)(len * code_unit_size);
while (dbuffer == NULL || needlen >= dbuffer_size)
  {
  dbuffer_size *= 2;
  dbuffer = (uint8_t *)realloc(dbuffer, dbuffer_size);
  if (dbuffer == NULL)
    {
    fprintf(stderr, "pcre2test: realloc(%d) failed\n", (int)dbuffer_size);
    exit(1);
    }
  }
SETCASTPTR(q, dbuffer);

/* Scan the data line, interpreting data escapes, and put the result into a
buffer the appropriate width buffer. In UTF mode, input can be UTF-8. */

while ((c = *p++) != 0)
  {
  int i = 0;
  size_t duplen;

  /* ] may mark the end of a duplicated sequence */

  if (c == ']' && start_dup != NULL)
    {
    size_t qoffset = CAST8VAR(q) - (uint8_t *)dbuffer;

    if (*p++ != '{')
      {
      fprintf(outfile, "** Expected '{' after \\[....]\n");
      return PR_OK;
      }
    while (isdigit(*p)) i = i * 10 + *p++ - '0';
    if (*p++ != '}')
      {
      fprintf(outfile, "** Expected '}' after \\[...]{...\n");
      return PR_OK;
      }
    if (i-- == 0)
      {
      fprintf(outfile, "** Zero repeat not allowed\n");
      return PR_OK;
      }

    duplen = CAST8VAR(q) - start_dup;
    needlen += duplen * (i - 1);
    while (needlen >= dbuffer_size)
      {
      dbuffer_size *= 2;
      dbuffer = (uint8_t *)realloc(dbuffer, dbuffer_size);
      if (dbuffer == NULL)
        {
        fprintf(stderr, "pcre2test: realloc(%d) failed\n", (int)dbuffer_size);
        exit(1);
        }
      SETCASTPTR(q, dbuffer + qoffset);
      }

    while (i-- > 0)
      {
      memcpy(CAST8VAR(q), start_dup, duplen);
      SETPLUS(q, duplen/code_unit_size);
      }

    start_dup = NULL;
    continue;
    }

  /* Handle a non-escaped character */

  if (c != '\\')
    {
    if (utf && HASUTF8EXTRALEN(c)) { GETUTF8INC(c, p); }
    }

  /* Handle backslash escapes */

  else switch ((c = *p++))
    {
    case '\\': break;
    case 'a': c =    7; break;
    case 'b': c = '\b'; break;
    case 'e': c =   27; break;
    case 'f': c = '\f'; break;
    case 'n': c = '\n'; break;
    case 'r': c = '\r'; break;
    case 't': c = '\t'; break;
    case 'v': c = '\v'; break;

    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
    c -= '0';
    while (i++ < 2 && isdigit(*p) && *p != '8' && *p != '9')
      c = c * 8 + *p++ - '0';
    break;

    case 'o':
    if (*p == '{')
      {
      uint8_t *pt = p;
      c = 0;
      for (pt++; isdigit(*pt) && *pt != '8' && *pt != '9'; pt++)
        {
        if (++i == 12)
          fprintf(outfile, "** Too many octal digits in \\o{...} item; "
                           "using only the first twelve.\n");
        else c = c * 8 + *pt - '0';
        }
      if (*pt == '}') p = pt + 1;
        else fprintf(outfile, "** Missing } after \\o{ (assumed)\n");
      }
    break;

    case 'x':
    if (*p == '{')
      {
      uint8_t *pt = p;
      c = 0;

      /* We used to have "while (isxdigit(*(++pt)))" here, but it fails
      when isxdigit() is a macro that refers to its argument more than
      once. This is banned by the C Standard, but apparently happens in at
      least one MacOS environment. */

      for (pt++; isxdigit(*pt); pt++)
        {
        if (++i == 9)
          fprintf(outfile, "** Too many hex digits in \\x{...} item; "
                           "using only the first eight.\n");
        else c = c * 16 + tolower(*pt) - ((isdigit(*pt))? '0' : 'a' - 10);
        }
      if (*pt == '}')
        {
        p = pt + 1;
        break;
        }
      /* Not correct form for \x{...}; fall through */
      }

    /* \x without {} always defines just one byte in 8-bit mode. This
    allows UTF-8 characters to be constructed byte by byte, and also allows
    invalid UTF-8 sequences to be made. Just copy the byte in UTF-8 mode.
    Otherwise, pass it down as data. */

    c = 0;
    while (i++ < 2 && isxdigit(*p))
      {
      c = c * 16 + tolower(*p) - ((isdigit(*p))? '0' : 'a' - 10);
      p++;
      }
#if defined SUPPORT_PCRE8
    if (utf && (test_mode == PCRE8_MODE))
      {
      *q8++ = c;
      continue;
      }
#endif
    break;

    case 0:     /* \ followed by EOF allows for an empty line */
    p--;
    continue;

    case '=':   /* \= terminates the data, starts modifiers */
    goto ENDSTRING;

    case '[':   /* \[ introduces a duplicated character sequence */
    if (start_dup != NULL)
      {
      fprintf(outfile, "** Nested duplication is not supported\n");
      return PR_OK;
      }
    start_dup = CAST8VAR(q);
    continue;

    default:
    fprintf(outfile, "** Unrecognized escape sequence \"\\%c\"\n", c);
    return PR_OK;
    }

  /* We now have a character value in c that may be greater than 255.
  In 8-bit mode we convert to UTF-8 if we are in UTF mode. Values greater
  than 127 in UTF mode must have come from \x{...} or octal constructs
  because values from \x.. get this far only in non-UTF mode. */

#ifdef SUPPORT_PCRE8
  if (test_mode == PCRE8_MODE)
    {
    if (utf)
      {
      if (c > 0x7fffffff)
        {
        fprintf(outfile, "** Character \\x{%x} is greater than 0x7fffffff "
          "and so cannot be converted to UTF-8\n", c);
        return PR_OK;
        }
      q8 += ord2utf8(c, q8);
      }
    else
      {
      if (c > 0xffu)
        {
        fprintf(outfile, "** Character \\x{%x} is greater than 255 "
          "and UTF-8 mode is not enabled.\n", c);
        fprintf(outfile, "** Truncation will probably give the wrong "
          "result.\n");
        }
      *q8++ = c;
      }
    }
#endif
#ifdef SUPPORT_PCRE16
  if (test_mode == PCRE16_MODE)
    {
    if (utf)
      {
      if (c > 0x10ffffu)
        {
        fprintf(outfile, "** Failed: character \\x{%x} is greater than "
          "0x10ffff and so cannot be converted to UTF-16\n", c);
        return PR_OK;
        }
      else if (c >= 0x10000u)
        {
        c-= 0x10000u;
        *q16++ = 0xD800 | (c >> 10);
        *q16++ = 0xDC00 | (c & 0x3ff);
        }
      else
        *q16++ = c;
      }
    else
      {
      if (c > 0xffffu)
        {
        fprintf(outfile, "** Character \\x{%x} is greater than 0xffff "
          "and UTF-16 mode is not enabled.\n", c);
        fprintf(outfile, "** Truncation will probably give the wrong "
          "result.\n");
        }

      *q16++ = c;
      }
    }
#endif
#ifdef SUPPORT_PCRE32
  if (test_mode == PCRE32_MODE)
    {
    *q32++ = c;
    }
#endif
  }

ENDSTRING:
SET(*q, 0);
len = CASTVAR(uint8_t *, q) - dbuffer;    /* Length in bytes */

/* If we have explicit valgrind support, mark the data from after its end to
the end of the buffer as unaddressable, so that a read over the end of the
buffer will be seen by valgrind, even if it doesn't cause a crash. If we're not
building with valgrind support, at least move the data to the end of the buffer
so that it might at least cause a crash. If we are using the POSIX interface,
we must include the terminating zero. */

pp = dbuffer;
c = code_unit_size * ((pat_patctl.control & CTL_POSIX) != 0)? 1:0;

#ifdef SUPPORT_VALGRIND
  VALGRIND_MAKE_MEM_NOACCESS(dbuffer + len + c, dbuffer_size - (len + c));
#else
  pp = memmove(pp + dbuffer_size - len - c, pp, len + c);
#endif

/* If the string was terminated by \= we must now interpret modifiers. */

if (p[-1] != 0 && !decode_modifiers(p, CTX_DAT, NULL, &dat_datctl))
  return PR_OK;
  
if ((dat_datctl.control & (CTL_DFA|CTL_LIMITS)) == (CTL_DFA|CTL_LIMITS))
  {
  printf("** Find match limits not relevant for DFA matching: ignored\n");
  }

/* Now run the pattern match: len contains the byte length and pp points to the 
subject string. POSIX matching is only possible in 8-bit mode, and it does not 
support timing or other fancy features. Some were checked at compile time, but
we need to check the match-time settings here. */

if ((pat_patctl.control & CTL_POSIX) != 0)
  {
  int rc;
  int eflags = 0;
  regmatch_t *pmatch = NULL;
  const char *msg = "** Ignored with POSIX interface:"; 
  
  if (dat_datctl.cfail[0] != 0 || dat_datctl.cfail[1] != 0)
    prmsg(&msg, "callout_fail"); 
  if (dat_datctl.copy_numbers[0] != 0 || dat_datctl.copy_names[0] != 0)
    prmsg(&msg, "copy"); 
  if (dat_datctl.get_numbers[0] != 0 || dat_datctl.get_names[0] != 0)
    prmsg(&msg, "get"); 
  if (dat_datctl.jitstack != 0) prmsg(&msg, "jitstack");
   
  if ((dat_datctl.options & ~POSIX_SUPPORTED_MATCH_OPTIONS) != 0)
    {
    fprintf(outfile, "%s", msg);
    show_match_options(dat_datctl.options & ~POSIX_SUPPORTED_MATCH_OPTIONS);
    msg = ""; 
    }  
  if ((dat_datctl.control & ~POSIX_SUPPORTED_MATCH_CONTROLS) != 0)
    {
    fprintf(outfile, "%s", msg);
    show_match_controls(dat_datctl.control & ~POSIX_SUPPORTED_MATCH_CONTROLS);
    msg = ""; 
    }
     
  if (msg[0] == 0) fprintf(outfile, "\n");     
  
  if (dat_datctl.oveccount > 0)
    pmatch = (regmatch_t *)malloc(sizeof(regmatch_t) * dat_datctl.oveccount);
  if ((dat_datctl.options & PCRE2_NOTBOL) != 0) eflags |= REG_NOTBOL;
  if ((dat_datctl.options & PCRE2_NOTEOL) != 0) eflags |= REG_NOTEOL;
  if ((dat_datctl.options & PCRE2_NOTEMPTY) != 0) eflags |= REG_NOTEMPTY;

  rc = regexec(&preg, (const char *)pp + dat_datctl.offset, 
    dat_datctl.oveccount, pmatch, eflags);
  if (rc != 0)
    {
    (void)regerror(rc, &preg, (char *)buffer, buffer_size);
    fprintf(outfile, "No match: POSIX code %d: %s\n", rc, buffer);
    }
  else if ((pat_patctl.options & PCRE2_NO_AUTO_CAPTURE) != 0)
    fprintf(outfile, "Matched with REG_NOSUB\n");
  else if (dat_datctl.oveccount == 0)
    fprintf(outfile, "Matched without capture\n");    
  else
    {
    size_t i;
    for (i = 0; i < (size_t)dat_datctl.oveccount; i++)
      {
      if (pmatch[i].rm_so >= 0)
        {
        fprintf(outfile, "%2d: ", (int)i);
        PCHARSV(dbuffer, pmatch[i].rm_so,
          pmatch[i].rm_eo - pmatch[i].rm_so, FALSE, outfile);
        fprintf(outfile, "\n");
        if ((i == 0 && (pat_patctl.control & CTL_AFTERTEXT) != 0) ||
            (pat_patctl.control & CTL_ALLAFTERTEXT) != 0) 
          {
          fprintf(outfile, "%2d+ ", (int)i);
          PCHARSV(dbuffer, pmatch[i].rm_eo, len - pmatch[i].rm_eo,
            FALSE, outfile);
          fprintf(outfile, "\n");
          }
        }
      }
    }
  free(pmatch);
  return PR_OK;
  }


#ifdef FIXME

/* Handle matching via the native interface - repeats for /g and /G */

/* Ensure that there is a JIT callback if we want to verify that JIT was
actually used. If jit_stack == NULL, no stack has yet been assigned. */

if (verify_jit && jit_stack == NULL && extra != NULL)
   { PCRE_ASSIGN_JIT_STACK(extra, jit_callback, jit_stack); }

for (;; gmatched++)    /* Loop for /g or /G */
  {
  markptr = NULL;
  jit_was_used = FALSE;

  if (timeitm > 0)
    {
    register int i;
    clock_t time_taken;
    clock_t start_time = clock();

#if !defined NODFA
    if (all_use_dfa || use_dfa)
      {
      if ((options & PCRE_DFA_RESTART) != 0)
        {
        fprintf(outfile, "Timing DFA restarts is not supported\n");
        break;
        }
      if (dfa_workspace == NULL)
        dfa_workspace = (int *)malloc(DFA_WS_DIMENSION*sizeof(int));
      for (i = 0; i < timeitm; i++)
        {
        PCRE_DFA_EXEC(count, re, extra, bptr, len, start_offset,
          (options | g_notempty), use_offsets, use_size_offsets,
          dfa_workspace, DFA_WS_DIMENSION);
        }
      }
    else
#endif

    for (i = 0; i < timeitm; i++)
      {
      PCRE_EXEC(count, re, extra, bptr, len, start_offset,
        (options | g_notempty), use_offsets, use_size_offsets);
      }
    total_match_time += (time_taken = clock() - start_time);
    fprintf(outfile, "Execute time %.4f milliseconds\n",
      (((double)time_taken * 1000.0) / (double)timeitm) /
        (double)CLOCKS_PER_SEC);
    }

  /* If find_match_limit is set, we want to do repeated matches with
  varying limits in order to find the minimum value for the match limit and
  for the recursion limit. The match limits are relevant only to the normal
  running of pcre_exec(), so disable the JIT optimization. This makes it
  possible to run the same set of tests with and without JIT externally
  requested. */

  if (find_match_limit)
    {
    if (extra != NULL) { PCRE_FREE_STUDY(extra); }
    extra = (pcre_extra *)malloc(sizeof(pcre_extra));
    extra->flags = 0;

    (void)check_match_limit(re, extra, bptr, len, start_offset,
      options|g_notempty, use_offsets, use_size_offsets,
      PCRE_EXTRA_MATCH_LIMIT, &(extra->match_limit),
      PCRE_ERROR_MATCHLIMIT, "match()");

    count = check_match_limit(re, extra, bptr, len, start_offset,
      options|g_notempty, use_offsets, use_size_offsets,
      PCRE_EXTRA_MATCH_LIMIT_RECURSION, &(extra->match_limit_recursion),
      PCRE_ERROR_RECURSIONLIMIT, "match() recursion");
    }

  /* If callout_data is set, use the interface with additional data */

  else if (callout_data_set)
    {
    if (extra == NULL)
      {
      extra = (pcre_extra *)malloc(sizeof(pcre_extra));
      extra->flags = 0;
      }
    extra->flags |= PCRE_EXTRA_CALLOUT_DATA;
    extra->callout_data = &callout_data;
    PCRE_EXEC(count, re, extra, bptr, len, start_offset,
      options | g_notempty, use_offsets, use_size_offsets);
    extra->flags &= ~PCRE_EXTRA_CALLOUT_DATA;
    }

  /* The normal case is just to do the match once, with the default
  value of match_limit. */

#if !defined NODFA
  else if (all_use_dfa || use_dfa)
    {
    if (dfa_workspace == NULL)
      dfa_workspace = (int *)malloc(DFA_WS_DIMENSION*sizeof(int));
    if (dfa_matched++ == 0)
      dfa_workspace[0] = -1;  /* To catch bad restart */
    PCRE_DFA_EXEC(count, re, extra, bptr, len, start_offset,
      (options | g_notempty), use_offsets, use_size_offsets, dfa_workspace,
      DFA_WS_DIMENSION);
    if (count == 0)
      {
      fprintf(outfile, "Matched, but offsets vector is too small to show all matches\n");
      count = use_size_offsets/2;
      }
    }
#endif

  else
    {
    PCRE_EXEC(count, re, extra, bptr, len, start_offset,
      options | g_notempty, use_offsets, use_size_offsets);
    if (count == 0)
      {
      fprintf(outfile, "Matched, but too many substrings\n");
      /* 2 is a special case; match can be returned */
      count = (use_size_offsets == 2)? 1 : use_size_offsets/3;
      }
    }

  /* Matched */

  if (count >= 0)
    {
    int i, maxcount;
    void *cnptr, *gnptr;

#if !defined NODFA
    if (all_use_dfa || use_dfa) maxcount = use_size_offsets/2; else
#endif
      /* 2 is a special case; match can be returned */
      maxcount = (use_size_offsets == 2)? 1 : use_size_offsets/3;

    /* This is a check against a lunatic return value. */

    if (count > maxcount)
      {
      fprintf(outfile,
        "** PCRE error: returned count %d is too big for offset size %d\n",
        count, use_size_offsets);
      count = use_size_offsets/3;
      if (do_g || do_G)
        {
        fprintf(outfile, "** /%c loop abandoned\n", do_g? 'g' : 'G');
        do_g = do_G = FALSE;        /* Break g/G loop */
        }
      }

    /* do_allcaps requests showing of all captures in the pattern, to check
    unset ones at the end. */

    if (do_allcaps)
      {
      if (new_info(re, NULL, PCRE_INFO_CAPTURECOUNT, &count) < 0)
        goto SKIP_DATA;
      count++;   /* Allow for full match */
      if (count * 2 > use_size_offsets) count = use_size_offsets/2;
      }

    /* Output the captured substrings. Note that, for the matched string,
    the use of \K in an assertion can make the start later than the end. */

    for (i = 0; i < count * 2; i += 2)
      {
      if (use_offsets[i] < 0)
        {
        if (use_offsets[i] != -1)
          fprintf(outfile, "ERROR: bad negative value %d for offset %d\n",
            use_offsets[i], i);
        if (use_offsets[i+1] != -1)
          fprintf(outfile, "ERROR: bad negative value %d for offset %d\n",
            use_offsets[i+1], i+1);
        fprintf(outfile, "%2d: <unset>\n", i/2);
        }
      else
        {
        int start = use_offsets[i];
        int end = use_offsets[i+1];

        if (start > end)
          {
          start = use_offsets[i+1];
          end = use_offsets[i];
          fprintf(outfile, "Start of matched string is beyond its end - "
            "displaying from end to start.\n");
          }

        fprintf(outfile, "%2d: ", i/2);
        PCHARSV(bptr, start, end - start, outfile);
        if (verify_jit && jit_was_used) fprintf(outfile, " (JIT)");
        fprintf(outfile, "\n");

        /* Note: don't use the start/end variables here because we want to
        show the text from what is reported as the end. */

        if (do_showcaprest || (i == 0 && do_showrest))
          {
          fprintf(outfile, "%2d+ ", i/2);
          PCHARSV(bptr, use_offsets[i+1], len - use_offsets[i+1],
            outfile);
          fprintf(outfile, "\n");
          }
        }
      }

    if (markptr != NULL)
      {
      fprintf(outfile, "MK: ");
      PCHARSV(markptr, 0, -1, outfile);
      fprintf(outfile, "\n");
      }

    for (i = 0; i < 32; i++)
      {
      if ((copystrings & (1 << i)) != 0)
        {
        int rc;
        char copybuffer[256];
        PCRE_COPY_SUBSTRING(rc, bptr, use_offsets, count, i,
          copybuffer, sizeof(copybuffer));
        if (rc < 0)
          fprintf(outfile, "copy substring %d failed %d\n", i, rc);
        else
          {
          fprintf(outfile, "%2dC ", i);
          PCHARSV(copybuffer, 0, rc, outfile);
          fprintf(outfile, " (%d)\n", rc);
          }
        }
      }

    cnptr = copynames;
    for (;;)
      {
      int rc;
      char copybuffer[256];

#ifdef SUPPORT_PCRE32
      if (pcre_mode == PCRE32_MODE)
        {
        if (*(pcre_uint32 *)cnptr == 0) break;
        }
#endif
#ifdef SUPPORT_PCRE16
      if (pcre_mode == PCRE16_MODE)
        {
        if (*(pcre_uint16 *)cnptr == 0) break;
        }
#endif
#ifdef SUPPORT_PCRE8
      if (pcre_mode == PCRE8_MODE)
        {
        if (*(pcre_uint8 *)cnptr == 0) break;
        }
#endif

      PCRE_COPY_NAMED_SUBSTRING(rc, re, bptr, use_offsets, count,
        cnptr, copybuffer, sizeof(copybuffer));

      if (rc < 0)
        {
        fprintf(outfile, "copy substring ");
        PCHARSV(cnptr, 0, -1, outfile);
        fprintf(outfile, " failed %d\n", rc);
        }
      else
        {
        fprintf(outfile, "  C ");
        PCHARSV(copybuffer, 0, rc, outfile);
        fprintf(outfile, " (%d) ", rc);
        PCHARSV(cnptr, 0, -1, outfile);
        putc('\n', outfile);
        }

      cnptr = (char *)cnptr + (STRLEN(cnptr) + 1) * CHAR_SIZE;
      }

    for (i = 0; i < 32; i++)
      {
      if ((getstrings & (1 << i)) != 0)
        {
        int rc;
        const char *substring;
        PCRE_GET_SUBSTRING(rc, bptr, use_offsets, count, i, &substring);
        if (rc < 0)
          fprintf(outfile, "get substring %d failed %d\n", i, rc);
        else
          {
          fprintf(outfile, "%2dG ", i);
          PCHARSV(substring, 0, rc, outfile);
          fprintf(outfile, " (%d)\n", rc);
          PCRE_FREE_SUBSTRING(substring);
          }
        }
      }

    gnptr = getnames;
    for (;;)
      {
      int rc;
      const char *substring;

#ifdef SUPPORT_PCRE32
      if (pcre_mode == PCRE32_MODE)
        {
        if (*(pcre_uint32 *)gnptr == 0) break;
        }
#endif
#ifdef SUPPORT_PCRE16
      if (pcre_mode == PCRE16_MODE)
        {
        if (*(pcre_uint16 *)gnptr == 0) break;
        }
#endif
#ifdef SUPPORT_PCRE8
      if (pcre_mode == PCRE8_MODE)
        {
        if (*(pcre_uint8 *)gnptr == 0) break;
        }
#endif

      PCRE_GET_NAMED_SUBSTRING(rc, re, bptr, use_offsets, count,
        gnptr, &substring);
      if (rc < 0)
        {
        fprintf(outfile, "get substring ");
        PCHARSV(gnptr, 0, -1, outfile);
        fprintf(outfile, " failed %d\n", rc);
        }
      else
        {
        fprintf(outfile, "  G ");
        PCHARSV(substring, 0, rc, outfile);
        fprintf(outfile, " (%d) ", rc);
        PCHARSV(gnptr, 0, -1, outfile);
        PCRE_FREE_SUBSTRING(substring);
        putc('\n', outfile);
        }

      gnptr = (char *)gnptr + (STRLEN(gnptr) + 1) * CHAR_SIZE;
      }

    if (getlist)
      {
      int rc;
      const char **stringlist;
      PCRE_GET_SUBSTRING_LIST(rc, bptr, use_offsets, count, &stringlist);
      if (rc < 0)
        fprintf(outfile, "get substring list failed %d\n", rc);
      else
        {
        for (i = 0; i < count; i++)
          {
          fprintf(outfile, "%2dL ", i);
          PCHARSV(stringlist[i], 0, -1, outfile);
          putc('\n', outfile);
          }
        if (stringlist[i] != NULL)
          fprintf(outfile, "string list not terminated by NULL\n");
        PCRE_FREE_SUBSTRING_LIST(stringlist);
        }
      }
    }

  /* There was a partial match. If the bumpalong point is not the same as
  the first inspected character, show the offset explicitly. */

  else if (count == PCRE_ERROR_PARTIAL)
    {
    fprintf(outfile, "Partial match");
    if (use_size_offsets > 2 && use_offsets[0] != use_offsets[2])
      fprintf(outfile, " at offset %d", use_offsets[2]);
    if (markptr != NULL)
      {
      fprintf(outfile, ", mark=");
      PCHARSV(markptr, 0, -1, outfile);
      }
    if (use_size_offsets > 1)
      {
      fprintf(outfile, ": ");
      PCHARSV(bptr, use_offsets[0], use_offsets[1] - use_offsets[0],
        outfile);
      }
    if (verify_jit && jit_was_used) fprintf(outfile, " (JIT)");
    fprintf(outfile, "\n");
    break;  /* Out of the /g loop */
    }

  /* Failed to match. If this is a /g or /G loop and we previously set
  g_notempty after a null match, this is not necessarily the end. We want
  to advance the start offset, and continue. We won't be at the end of the
  string - that was checked before setting g_notempty.

  Complication arises in the case when the newline convention is "any",
  "crlf", or "anycrlf". If the previous match was at the end of a line
  terminated by CRLF, an advance of one character just passes the \r,
  whereas we should prefer the longer newline sequence, as does the code in
  pcre_exec(). Fudge the offset value to achieve this. We check for a
  newline setting in the pattern; if none was set, use PCRE_CONFIG() to
  find the default.

  Otherwise, in the case of UTF-8 matching, the advance must be one
  character, not one byte. */

  else
    {
    if (g_notempty != 0)
      {
      int onechar = 1;
      unsigned int obits = REAL_PCRE_OPTIONS(re);
      use_offsets[0] = start_offset;
      if ((obits & PCRE_NEWLINE_BITS) == 0)
        {
        int d;
        (void)PCRE_CONFIG(PCRE_CONFIG_NEWLINE, &d);
        /* Note that these values are always the ASCII ones, even in
        EBCDIC environments. CR = 13, NL = 10. */
        obits = (d == 13)? PCRE_NEWLINE_CR :
                (d == 10)? PCRE_NEWLINE_LF :
                (d == (13<<8 | 10))? PCRE_NEWLINE_CRLF :
                (d == -2)? PCRE_NEWLINE_ANYCRLF :
                (d == -1)? PCRE_NEWLINE_ANY : 0;
        }
      if (((obits & PCRE_NEWLINE_BITS) == PCRE_NEWLINE_ANY ||
           (obits & PCRE_NEWLINE_BITS) == PCRE_NEWLINE_CRLF ||
           (obits & PCRE_NEWLINE_BITS) == PCRE_NEWLINE_ANYCRLF)
          &&
          start_offset < len - 1 && (
#ifdef SUPPORT_PCRE8
          (pcre_mode == PCRE8_MODE &&
           bptr[start_offset] == '\r' &&
           bptr[start_offset + 1] == '\n') ||
#endif
#ifdef SUPPORT_PCRE16
          (pcre_mode == PCRE16_MODE &&
           ((PCRE_SPTR16)bptr)[start_offset] == '\r' &&
           ((PCRE_SPTR16)bptr)[start_offset + 1] == '\n') ||
#endif
#ifdef SUPPORT_PCRE32
          (pcre_mode == PCRE32_MODE &&
           ((PCRE_SPTR32)bptr)[start_offset] == '\r' &&
           ((PCRE_SPTR32)bptr)[start_offset + 1] == '\n') ||
#endif
          0))
        onechar++;
      else if (utf)
        {
        while (start_offset + onechar < len)
          {
          if ((bptr[start_offset+onechar] & 0xc0) != 0x80) break;
          onechar++;
          }
        }
      use_offsets[1] = start_offset + onechar;
      }
    else
      {
      switch(count)
        {
        case PCRE_ERROR_NOMATCH:
        if (gmatched == 0)
          {
          if (markptr == NULL)
            {
            fprintf(outfile, "No match");
            }
          else
            {
            fprintf(outfile, "No match, mark = ");
            PCHARSV(markptr, 0, -1, outfile);
            }
          if (verify_jit && jit_was_used) fprintf(outfile, " (JIT)");
          putc('\n', outfile);
          }
        break;

        case PCRE_ERROR_BADUTF8:
        case PCRE_ERROR_SHORTUTF8:
        fprintf(outfile, "Error %d (%s UTF-%d string)", count,
          (count == PCRE_ERROR_BADUTF8)? "bad" : "short",
          8 * CHAR_SIZE);
        if (use_size_offsets >= 2)
          fprintf(outfile, " offset=%d reason=%d", use_offsets[0],
            use_offsets[1]);
        fprintf(outfile, "\n");
        break;

        case PCRE_ERROR_BADUTF8_OFFSET:
        fprintf(outfile, "Error %d (bad UTF-%d offset)\n", count,
          8 * CHAR_SIZE);
        break;

        default:
        if (count < 0 &&
            (-count) < (int)(sizeof(errtexts)/sizeof(const char *)))
          fprintf(outfile, "Error %d (%s)\n", count, errtexts[-count]);
        else
          fprintf(outfile, "Error %d (Unexpected value)\n", count);
        break;
        }

      break;  /* Out of the /g loop */
      }
    }

  /* If not /g or /G we are done */

  if (!do_g && !do_G) break;

  /* If we have matched an empty string, first check to see if we are at
  the end of the subject. If so, the /g loop is over. Otherwise, mimic what
  Perl's /g options does. This turns out to be rather cunning. First we set
  PCRE_NOTEMPTY_ATSTART and PCRE_ANCHORED and try the match again at the
  same point. If this fails (picked up above) we advance to the next
  character. */

  g_notempty = 0;

  if (use_offsets[0] == use_offsets[1])
    {
    if (use_offsets[0] == len) break;
    g_notempty = PCRE_NOTEMPTY_ATSTART | PCRE_ANCHORED;
    }

  /* For /g, update the start offset, leaving the rest alone */

  if (do_g) start_offset = use_offsets[1];

  /* For /G, update the pointer and length */

  else
    {
    bptr += use_offsets[1] * CHAR_SIZE;
    len -= use_offsets[1];
    }
  }  /* End of loop for /g and /G */

NEXT_DATA: continue;
}    /* End of loop for data lines */

#endif


return PR_OK;
}




/*************************************************
*                Print PCRE version              *
*************************************************/

/* The version string was read into 'version' at the start of execution. */

static void
print_version(FILE *f)
{
VERSION_TYPE *vp;
fprintf(f, "PCRE version ");
for (vp = version; *vp != 0; vp++) fprintf(f, "%c", *vp);
fprintf(f, "\n");
}



/*************************************************
*       Print newline configuration              *
*************************************************/

/* Output is always to stdout.

Arguments:
  rc         the return code from PCRE_CONFIG_NEWLINE
  isc        TRUE if called from "-C newline"
Returns:     nothing
*/

static void
print_newline_config(unsigned int rc, BOOL isc)
{
if (!isc) printf("  Newline sequence is ");
if (rc < sizeof(newlines)/sizeof(char *))
  printf("%s\n", newlines[rc]);
else
  printf("a non-standard value: %d\n", rc);
}



/*************************************************
*             Usage function                     *
*************************************************/

static void
usage(void)
{
printf("Usage:     pcre2test [options] [<input file> [<output file>]]\n\n");
printf("Input and output default to stdin and stdout.\n");
#if defined(SUPPORT_LIBREADLINE) || defined(SUPPORT_LIBEDIT)
printf("If input is a terminal, readline() is used to read from it.\n");
#else
printf("This version of pcre2test is not linked with readline().\n");
#endif
printf("\nOptions:\n");
#ifdef SUPPORT_PCRE8
printf("  -8            use the 8-bit library\n");
#endif
#ifdef SUPPORT_PCRE16
printf("  -16           use the 16-bit library\n");
#endif
#ifdef SUPPORT_PCRE32
printf("  -32           use the 32-bit library\n");
#endif
printf("  -b            set default pattern control 'fullbytecode'\n");
printf("  -C            show PCRE2 compile-time options and exit\n");
printf("  -C arg        show a specific compile-time option and exit\n");
printf("                with its value if numeric (else 0). The arg can be:\n");
printf("     linksize     internal link size [2, 3, 4]\n");
printf("     pcre8        8 bit library support enabled [0, 1]\n");
printf("     pcre16       16 bit library support enabled [0, 1]\n");
printf("     pcre32       32 bit library support enabled [0, 1]\n");
printf("     utf          Unicode Transformation Format supported [0, 1]\n");
printf("     jit          Just-in-time compiler supported [0, 1]\n");
printf("     newline      Newline type [CR, LF, CRLF, ANYCRLF, ANY]\n");
printf("     bsr          \\R type [ANYCRLF, ANY]\n");
printf("  -d            set default pattern control 'debug'\n");
printf("  -data <s>     set default data control fields\n");
printf("  -help         show usage information\n");
printf("  -i            set default pattern control 'info'\n");
printf("  -q            quiet: do not output PCRE version number at start\n");
printf("  -pattern <s>  set default pattern control fields\n");
printf("  -S <n>        set stack size to <n> megabytes\n");
printf("  -t [<n>]      time compilation and execution, repeating <n> times\n");
printf("  -tm [<n>]     time execution (matching) only, repeating <n> times\n");
printf("  -T            same as -t, but show total times at the end\n");
printf("  -TM           same as -tm, but show total time at the end\n");
}



/*************************************************
*             Handle -C option                   *
*************************************************/

/* This option outputs configuration options and sets an appropriate return
code when asked for a single option. The code is abstracted into a separate
function because of its size. Use whichever pcre2_config() function is
available.

Argument:   an option name or NULL
Returns:    the return code
*/

static int
c_option(const char *arg)
{
unsigned long int lrc;
int rc;
int yield = 0;

if (arg != NULL)
  {
  unsigned int i;

  for (i = 0; i < COPTLISTCOUNT; i++)
    if (strcmp(arg, coptlist[i].name) == 0) break;

  if (i >= COPTLISTCOUNT)
    {
    fprintf(stderr, "** Unknown -C option '%s'\n", arg);
    return -1;
    }

  switch (coptlist[i].type)
    {
    case CONF_BSR:
    (void)PCRE2_CONFIG(coptlist[i].value, &rc);
    printf("%s\n", rc? "ANYCRLF" : "ANY");
    break;

    case CONF_FIX:
    yield = coptlist[i].value;
    printf("%d\n", yield);
    break;

    case CONF_FIZ:
    rc = coptlist[i].value;
    printf("%d\n", rc);
    break;

    case CONF_INT:
    (void)PCRE2_CONFIG(coptlist[i].value, &yield);
    printf("%d\n", yield);
    break;

    case CONF_NL:
    (void)PCRE2_CONFIG(coptlist[i].value, &rc);
    print_newline_config(rc, TRUE);
    break;
    }

/* For VMS, return the value by setting a symbol, for certain values only. */

#ifdef __VMS
  if (copytlist[i].type == CONF_FIX || coptlist[i].type == CONF_INT)
    {
    char ucname[16];
    strcpy(ucname, coptlist[i].name);
    for (i = 0; ucname[i] != 0; i++) ucname[i] = toupper[ucname[i];
    vms_setsymbol(ucname, 0, rc);
    }
#endif

  return yield;
  }

/* No argument for -C: output all configuration information. */

print_version(stdout);
printf("\nCompiled with\n");

#ifdef EBCDIC
printf("  EBCDIC code support: LF is 0x%02x\n", CHAR_LF);
#endif

#ifdef SUPPORT_PCRE8
printf("  8-bit support\n");
#endif
#ifdef SUPPORT_PCRE16
printf("  16-bit support\n");
#endif
#ifdef SUPPORT_PCRE32
printf("  32-bit support\n");
#endif

(void)PCRE2_CONFIG(PCRE2_CONFIG_UTF, &rc);
printf ("  %sUTF support\n", rc ? "" : "No ");
(void)PCRE2_CONFIG(PCRE2_CONFIG_JIT, &rc);
if (rc != 0)
  {
  const char *arch;
  (void)PCRE2_CONFIG(PCRE2_CONFIG_JITTARGET, (void *)(&arch));
  printf("  Just-in-time compiler support: %s\n", arch);
  }
else
  {
  printf("  No just-in-time compiler support\n");
  }
(void)PCRE2_CONFIG(PCRE2_CONFIG_NEWLINE, &rc);
print_newline_config(rc, FALSE);
(void)PCRE2_CONFIG(PCRE2_CONFIG_BSR, &rc);
printf("  \\R matches %s\n", rc? "CR, LF, or CRLF only" :
                                 "all Unicode newlines");
(void)PCRE2_CONFIG(PCRE2_CONFIG_LINK_SIZE, &rc);
printf("  Internal link size = %d\n", rc);
(void)PCRE2_CONFIG(PCRE2_CONFIG_PARENS_LIMIT, &lrc);
printf("  Parentheses nest limit = %ld\n", lrc);
(void)PCRE2_CONFIG(PCRE2_CONFIG_MATCH_LIMIT, &lrc);
printf("  Default match limit = %ld\n", lrc);
(void)PCRE2_CONFIG(PCRE2_CONFIG_MATCH_LIMIT_RECURSION, &lrc);
printf("  Default recursion depth limit = %ld\n", lrc);
(void)PCRE2_CONFIG(PCRE2_CONFIG_STACKRECURSE, &rc);
printf("  Match recursion uses %s", rc? "stack" : "heap");

#ifdef FIXME
if (showstore)
  {
  PCRE_EXEC(stack_size, NULL, NULL, NULL, -999, -999, 0, NULL, 0);
  printf(": %sframe size = %d bytes", rc? "approximate " : "", -stack_size);
  }
#endif

printf("\n");
return 0;
}



/*************************************************
*                Main Program                    *
*************************************************/

int
main(int argc, char **argv)
{
uint32_t yield = 0;
uint32_t op = 1;
uint32_t stack_size;
BOOL notdone = TRUE;
BOOL quiet = FALSE;
BOOL showtotaltimes = FALSE;
BOOL skipping = FALSE;
char *arg_data = NULL;
char *arg_pattern = NULL;

PCRE2_JIT_STACK *jit_stack = NULL;

/* The offsets to the options and control bits fields of the pattern and data
control blocks must be the same so that common options and controls such as
"anchored" or "memory" can work for either of them from a single table entry. 
We cannot test this till runtime because "offsetof" does not work in the 
preprocessor. */

if (PO(options) != DO(options) || PO(control) != DO(control))
  {
  fprintf(stderr, "** Coding error: "
    "options and control offsets for pattern and data must be the same.\n");
  return 1;
  }    

/* Get the PCRE version number. */

PCRE2_VERSION(version, VERSION_SIZE);

/* Get buffers from malloc() so that valgrind will check their misuse when
debugging. They grow automatically when very long lines are read. The 16-
and 32-bit buffers (pbuffer16, pbuffer32) are obtained only if needed. */

buffer = (uint8_t *)malloc(buffer_size);
pbuffer = pbuffer8 = (uint8_t *)malloc(buffer_size);

/* The following  _setmode() stuff is some Windows magic that tells its runtime
library to translate CRLF into a single LF character. At least, that's what
I've been told: never having used Windows I take this all on trust. Originally
it set 0x8000, but then I was advised that _O_BINARY was better. */

#if defined(_WIN32) || defined(WIN32)
_setmode( _fileno( stdout ), _O_BINARY );
#endif

/* Initialization that does not depend on the running mode. */

memset(&def_patctl, sizeof(patctl), 0);
memset(&def_datctl, sizeof(datctl), 0);
def_datctl.oveccount = DEFAULT_OVECCOUNT;

/* Scan command line options. */

while (argc > 1 && argv[op][0] == '-')
  {
  const char *endptr;
  char *arg = argv[op];

  /* Display and/or set return code for configuration options. */

  if (strcmp(arg, "-C") == 0)
    {
    yield = c_option(argv[op + 1]);
    goto EXIT;
    }

  /* Select operating mode */

  if (strcmp(arg, "-8") == 0)
    {
#ifdef SUPPORT_PCRE8
    test_mode = PCRE8_MODE;
#else
    fprintf(stderr,
      "** This version of PCRE was built without 8-bit support\n");
    exit(1);
#endif
    }
  else if (strcmp(arg, "-16") == 0)
    {
#ifdef SUPPORT_PCRE16
    test_mode = PCRE16_MODE;
#else
    fprintf(stderr,
      "** This version of PCRE was built without 16-bit support\n");
    exit(1);
#endif
    }
  else if (strcmp(arg, "-32") == 0)
    {
#ifdef SUPPORT_PCRE32
    test_mode = PCRE32_MODE;
#else
    fprintf(stderr,
      "** This version of PCRE was built without 32-bit support\n");
    exit(1);
#endif
    }

  /* Set quiet (no version verification) */

  else if (strcmp(arg, "-q") == 0) quiet = TRUE;

  /* Set system stack size */

  else if (strcmp(arg, "-S") == 0 && argc > 2 &&
      ((stack_size = get_value(argv[op+1], &endptr)), *endptr == 0))
    {
#if defined(_WIN32) || defined(WIN32) || defined(__minix) || defined(NATIVE_ZOS) || defined(__VMS)
    fprintf(stderr, "PCRE: -S is not supported on this OS\n");
    exit(1);
#else
    int rc;
    struct rlimit rlim;
    getrlimit(RLIMIT_STACK, &rlim);
    rlim.rlim_cur = stack_size * 1024 * 1024;
    rc = setrlimit(RLIMIT_STACK, &rlim);
    if (rc != 0)
      {
      fprintf(stderr, "PCRE: setrlimit() failed with error %d\n", rc);
      exit(1);
      }
    op++;
    argc--;
#endif
    }

  /* Set some common pattern controls */

  else if (strcmp(arg, "-b") == 0) def_patctl.control |= CTL_FULLBYTECODE;
  else if (strcmp(arg, "-d") == 0) def_patctl.control |= CTL_DEBUG;
  else if (strcmp(arg, "-i") == 0) def_patctl.control |= CTL_INFO;

  /* Set timing parameters */

  else if (strcmp(arg, "-t") == 0 || strcmp(arg, "-tm") == 0 ||
           strcmp(arg, "-T") == 0 || strcmp(arg, "-TM") == 0)
    {
    int temp;
    int both = arg[2] == 0;
    showtotaltimes = arg[1] == 'T';
    if (argc > 2 && (temp = get_value(argv[op+1], &endptr), *endptr == 0))
      {
      timeitm = temp;
      op++;
      argc--;
      }
    else timeitm = LOOPREPEAT;
    if (both) timeit = timeitm;
    }

  /* Give help */

  else if (strcmp(arg, "-help") == 0 ||
           strcmp(arg, "--help") == 0)
    {
    usage();
    goto EXIT;
    }

  /* The following options save their data for processing once we know what
  the running mode is. */

  else if (strcmp(arg, "-data") == 0)
    {
    arg_data = argv[op+1];
    goto CHECK_VALUE_EXISTS;
    }

  else if (strcmp(arg, "-pattern") == 0)
    {
    arg_pattern = argv[op+1];
    CHECK_VALUE_EXISTS:
    if (argc <= 2)
      {
      fprintf(stderr, "** Missing value for %s\n", arg);
      yield = 1;
      goto EXIT;
      }
    op++;
    argc--;
    }

  /* Unrecognized option */

  else
    {
    fprintf(stderr, "** Unknown or malformed option '%s'\n", arg);
    usage();
    yield = 1;
    goto EXIT;
    }
  op++;
  argc--;
  }

/* Initialize things that cannot be done until we know which test mode we are
running in. */

code_unit_size = test_mode/8;

#ifdef SUPPORT_PCRE8
if (test_mode == PCRE8_MODE)
  {
  default_pat_context8 = pcre2_compile_context_create_8(NULL);
  pat_context8 = pcre2_compile_context_create_8(NULL);
  default_dat_context8 = pcre2_match_context_create_8(NULL);
  dat_context8 = pcre2_match_context_create_8(NULL);
  }
#endif

#ifdef SUPPORT_PCRE16
if (test_mode == PCRE16_MODE)
  {
  default_pat_context16 = pcre2_compile_context_create_16(NULL);
  pat_context16 = pcre2_compile_context_create_16(NULL);
  default_dat_context16 = pcre2_match_context_create_16(NULL);
  dat_context16 = pcre2_match_context_create_16(NULL);
  }
#endif

#ifdef SUPPORT_PCRE32
if (test_mode == PCRE32_MODE)
  {
  default_pat_context32 = pcre2_compile_context_create_32(NULL);
  pat_context32 = pcre2_compile_context_create_32(NULL);
  default_dat_context32 = pcre2_match_context_create_32(NULL);
  dat_context32 = pcre2_match_context_create_32(NULL);
  }
#endif

/* Handle command line modifier settings, sending any error messages to
stderr. We need to know the mode before modifying the context, and it is tidier
to do them all in the same way. */

outfile = stderr;
if ((arg_pattern != NULL &&
    !decode_modifiers((uint8_t *)arg_pattern, CTX_DEFPAT, &def_patctl, NULL)) ||
    (arg_data != NULL &&
    !decode_modifiers((uint8_t *)arg_data, CTX_DEFDAT, NULL, &def_datctl)))
  {
  yield = 1;
  goto EXIT;
  }

/* Sort out the input and output files, defaulting to stdin/stdout. */

infile = stdin;
outfile = stdout;

if (argc > 1)
  {
  infile = fopen(argv[op], INPUT_MODE);
  if (infile == NULL)
    {
    printf("** Failed to open %s\n", argv[op]);
    yield = 1;
    goto EXIT;
    }
  }

if (argc > 2)
  {
  outfile = fopen(argv[op+1], OUTPUT_MODE);
  if (outfile == NULL)
    {
    printf("** Failed to open %s\n", argv[op+1]);
    yield = 1;
    goto EXIT;
    }
  }

/* Output a heading line unless quiet, then process input lines. */

if (!quiet) print_version(outfile);

SET(compiled_code, NULL);
preg.re_pcre2_code = NULL;
preg.re_match_data = NULL;

while (notdone)
  {
  uint8_t *p;
  int rc = PR_OK;
  BOOL expectdata = TEST(compiled_code, !=, NULL) || preg.re_pcre2_code != NULL;

  if (extend_inputline(infile, buffer, expectdata? "data> " : "  re> ") == NULL)
    break;
  if (infile != stdin) fprintf(outfile, "%s", (char *)buffer);
  fflush(outfile);
  p = buffer;

  /* If we have a pattern set up for testing, or we are skipping after a
  compile failure, a blank line terminates this test; otherwise process the
  line as a data line. */

  if (expectdata || skipping)
    {
    while (isspace(*p)) p++;
    if (*p == 0)
      {
      if (preg.re_pcre2_code != NULL)
        {  
        regfree(&preg);
        preg.re_pcre2_code = NULL;
        preg.re_match_data = NULL;
        }
      if (TEST(compiled_code, !=, NULL))
        {       
        SUB1(pcre2_code_free, compiled_code);
        SET(compiled_code, NULL);
        } 
      skipping = FALSE;
      }
    else if (!skipping) rc = process_data();
    }

  /* We do not have a pattern set up for testing. Lines starting with # are
  either comments or special commands. Blank lines are ignored. Otherwise, the
  line must start with a valid delimiter. It is then processed as a pattern
  line. */

  else if (*p == '#')
    {
    if (isspace(p[1]) || p[1] == '!' || p[1] == 0) continue;
    rc = process_command();
    }

  else if (strchr("\"/!'`-+=:;.,", *p) != NULL)
    {
    rc = process_pattern();
    }

  else
    {
    while (isspace(*p)) p++;
    if (*p != 0)
      {
      fprintf(stderr, "** Invalid pattern delimiter '%c'.\n", *buffer);
      rc = PR_SKIP;
      }
    }

  if (rc == PR_SKIP && infile != stdin) skipping = TRUE;
    else if (rc == PR_ABEND) goto EXIT;
  }

/* Finish off a normal run. */

if (infile == stdin) fprintf(outfile, "\n");

if (showtotaltimes)
  {
  fprintf(outfile, "--------------------------------------\n");
  if (timeit > 0)
    {
    fprintf(outfile, "Total compile time %.4f milliseconds\n",
      (((double)total_compile_time * 1000.0) / (double)timeit) /
        (double)CLOCKS_PER_SEC);
    }
  fprintf(outfile, "Total execute time %.4f milliseconds\n",
    (((double)total_match_time * 1000.0) / (double)timeitm) /
      (double)CLOCKS_PER_SEC);
  }


EXIT:

if (infile != NULL && infile != stdin) fclose(infile);
if (outfile != NULL && outfile != stdout) fclose(outfile);

free(buffer);
free(dbuffer);
free(pbuffer);
regfree(&preg);
SUB1(pcre2_code_free, compiled_code);

#ifdef SUPPORT_PCRE8
if (pat_context8 != NULL) pcre2_compile_context_free_8(pat_context8);
if (default_pat_context8 != NULL)
  pcre2_compile_context_free_8(default_pat_context8);
if (dat_context8 != NULL) pcre2_match_context_free_8(dat_context8);
if (default_dat_context8 != NULL)
  pcre2_match_context_free_8(default_dat_context8);
#endif

#ifdef SUPPORT_PCRE16
if (pbuffer16 != NULL) free(pbuffer16);
if (pat_context16 != NULL) pcre2_compile_context_free_16(pat_context16);
if (default_pat_context16 != NULL)
  pcre2_compile_context_free_16(default_pat_context16);
if (dat_context16 != NULL) pcre2_match_context_free_16(dat_context16);
if (default_dat_context16 != NULL)
  pcre2_match_context_free_16(default_dat_context16);
#endif

#ifdef SUPPORT_PCRE32
if (pbuffer32 != NULL) free(pbuffer32);
if (pat_context32 != NULL) pcre2_compile_context_free_32(pat_context32);
if (default_pat_context32 != NULL)
  pcre2_compile_context_free_32(default_pat_context32);
if (dat_context32 != NULL) pcre2_match_context_free_32(dat_context32);
if (default_dat_context32 != NULL)
  pcre2_match_context_free_32(default_dat_context32);
#endif

#if defined(__VMS)
  yield = SS$_NORMAL;  /* Return values via DCL symbols */
#endif

/* FIXME: temp avoid compiler warnings. */

(void)jit_stack;

return yield;
}

/* End of pcre2test.c */
