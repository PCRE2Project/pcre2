/*************************************************
*       Perl-Compatible Regular Expressions      *
*************************************************/

/* This is the public header file for the PCRE library, second API, to be
#included by applications that call PCRE2 functions.

           Copyright (c) 2014 University of Cambridge

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

#ifndef _PCRE2_H
#define _PCRE2_H

/* The current PCRE version information. */

#define PCRE2_MAJOR          9
#define PCRE2_MINOR          00
#define PCRE2_PRERELEASE     -DEV
#define PCRE2_DATE           2014-99-99

/* When an application links to a PCRE DLL in Windows, the symbols that are
imported have to be identified as such. When building PCRE, the appropriate
export setting is defined in pcre2_internal.h, which includes this file. So we
don't change existing definitions of PCRE2_EXP_DECL. */

#if defined(_WIN32) && !defined(PCRE2_STATIC)
#  ifndef PCRE2_EXP_DECL
#    define PCRE2_EXP_DECL  extern __declspec(dllimport)
#  endif
#endif

/* By default, we use the standard "extern" declarations. */

#ifndef PCRE2_EXP_DECL
#  ifdef __cplusplus
#    define PCRE2_EXP_DECL  extern "C"
#  else
#    define PCRE2_EXP_DECL  extern
#  endif
#endif

/* Have to include stdlib.h and stdint.h to ensure that size_t and uint8_t etc 
are defined. */

#include <stdlib.h>
#include <stdint.h>

/* Allow for C++ users compiling this directly. */

#ifdef __cplusplus
extern "C" {
#endif

/* Public options. Those in the context may affect compilation, JIT
compilation, and/or interpretive execution. The following tags indicate which:

C   alters what is compiled
J   alters what JIT compiles
E   affects pcre_exec() execution
D   affects pcre_dfa_exec() execution
*/

/* The first group of options are those that are set in the context. */

#define PCRE2_ALT_BSUX            0x00000001   /* C       */
#define PCRE2_DOLLAR_ENDONLY      0x00000002   /*   J E D */
#define PCRE2_DUPNAMES            0x00000004   /* C       */
#define PCRE2_ALLOW_EMPTY_CLASS   0x00000008   /* C       */
#define PCRE2_MATCH_UNSET_BACKREF 0x00000010   /* C J E   */
#define PCRE2_NEVER_UTF           0x00000020   /* C       */
#define PCRE2_NEVER_UCP           0x00000040   /* C       */
#define PCRE2_UTF                 0x00000080   /* C J E D */
#define PCRE2_UCP                 0x00000100   /* C J E D */

/* The following can be passed to pcre2_compile(), pcre2_exec(), or
pcre2_dfa_exec(). PCRE2_NO_UTF_CHECK affects only the function to which it is
passed. */

#define PCRE2_ANCHORED            0x00000001
#define PCRE2_NO_START_OPTIMIZE   0x00000002
#define PCRE2_NO_UTF_CHECK        0x00000004

/* These are for pcre2_compile() only, affecting what is compiled, but not
otherwise affecting execution. */

#define PCRE2_AUTO_CALLOUT        0x00000100
#define PCRE2_CASELESS            0x00000200
#define PCRE2_DOTALL              0x00000400
#define PCRE2_EXTENDED            0x00000800
#define PCRE2_MULTILINE           0x00001000
#define PCRE2_NO_AUTO_CAPTURE     0x00002000
#define PCRE2_NO_AUTO_POSSESS     0x00004000
#define PCRE2_UNGREEDY            0x00008000

/* This pcre2_compile() option affects JIT compilation and interpretive
execution. */

#define PCRE2_FIRSTLINE           0x00010000

/* These are for pcre2_jit_compile(). */

#define PCRE2_JIT                 0x00020000
#define PCRE2_JIT_PARTIAL_SOFT    0x00040000
#define PCRE2_JIT_PARTIAL_HARD    0x00080000

/* These are for pcre2_exec() and pcre2_dfa_exec(). */

#define PCRE2_NOTBOL              0x00000100
#define PCRE2_NOTEOL              0x00000200
#define PCRE2_NOTEMPTY            0x00000400
#define PCRE2_NOTEMPTY_ATSTART    0x00000800
#define PCRE2_PARTIAL_SOFT        0x00001000
#define PCRE2_PARTIAL_HARD        0x00002000

/* These are additional options for pcre2_dfa_exec(). */

#define PCRE2_DFA_RESTART         0x00010000
#define PCRE2_DFA_SHORTEST        0x00020000

/* Newline and \R settings, for use in the context. */

#define PCRE2_NEWLINE_CR          0x00000001
#define PCRE2_NEWLINE_LF          0x00000002
#define PCRE2_NEWLINE_CRLF        0x00000004
#define PCRE2_NEWLINE_ANY         0x00000008
#define PCRE2_NEWLINE_ANYCRLF     0x00000010

#define PCRE2_BSR_ANYCRLF         0x00000020
#define PCRE2_BSR_UNICODE         0x00000040

/* Exec-time and get/set-time error codes */

#define PCRE2_ERROR_NOMATCH          (-1)
#define PCRE2_ERROR_NULL             (-2)
#define PCRE2_ERROR_BADOPTION        (-3)
#define PCRE2_ERROR_BADMAGIC         (-4)
#define PCRE2_ERROR_UNKNOWN_OPCODE   (-5)
#define PCRE2_ERROR_NOMEMORY         (-6)
#define PCRE2_ERROR_NOSUBSTRING      (-7)
#define PCRE2_ERROR_MATCHLIMIT       (-8)
#define PCRE2_ERROR_CALLOUT          (-9)  /* Never used by PCRE2 itself */
#define PCRE2_ERROR_BADUTF          (-10)
#define PCRE2_ERROR_BADUTF_OFFSET   (-11)
#define PCRE2_ERROR_PARTIAL         (-12)
#define PCRE2_ERROR_BADPARTIAL      (-13)
#define PCRE2_ERROR_INTERNAL        (-14)
#define PCRE2_ERROR_BADCOUNT        (-15)
#define PCRE2_ERROR_DFA_UITEM       (-16)
#define PCRE2_ERROR_DFA_UCOND       (-17)
#define PCRE2_ERROR_DFA_UMLIMIT     (-18)
#define PCRE2_ERROR_DFA_WSSIZE      (-19)
#define PCRE2_ERROR_DFA_RECURSE     (-20)
#define PCRE2_ERROR_RECURSIONLIMIT  (-21)
#define PCRE2_ERROR_BADNEWLINE      (-22)
#define PCRE2_ERROR_BADOFFSET       (-23)
#define PCRE2_ERROR_SHORTUTF        (-24)
#define PCRE2_ERROR_RECURSELOOP     (-25)
#define PCRE2_ERROR_JIT_STACKLIMIT  (-26)
#define PCRE2_ERROR_BADMODE         (-27)
#define PCRE2_ERROR_BADENDIANNESS   (-28)
#define PCRE2_ERROR_DFA_BADRESTART  (-29)
#define PCRE2_ERROR_JIT_BADOPTION   (-30)
#define PCRE2_ERROR_BADLENGTH       (-31)
#define PCRE2_ERROR_UNSET           (-32)

/* Specific error codes for UTF-8 validity checks */

#define PCRE2_UTF8_ERR0               0
#define PCRE2_UTF8_ERR1               1
#define PCRE2_UTF8_ERR2               2
#define PCRE2_UTF8_ERR3               3
#define PCRE2_UTF8_ERR4               4
#define PCRE2_UTF8_ERR5               5
#define PCRE2_UTF8_ERR6               6
#define PCRE2_UTF8_ERR7               7
#define PCRE2_UTF8_ERR8               8
#define PCRE2_UTF8_ERR9               9
#define PCRE2_UTF8_ERR10             10
#define PCRE2_UTF8_ERR11             11
#define PCRE2_UTF8_ERR12             12
#define PCRE2_UTF8_ERR13             13
#define PCRE2_UTF8_ERR14             14
#define PCRE2_UTF8_ERR15             15
#define PCRE2_UTF8_ERR16             16
#define PCRE2_UTF8_ERR17             17
#define PCRE2_UTF8_ERR18             18
#define PCRE2_UTF8_ERR19             19
#define PCRE2_UTF8_ERR20             20
#define PCRE2_UTF8_ERR21             21

/* Specific error codes for UTF-16 validity checks */

#define PCRE2_UTF16_ERR0              0
#define PCRE2_UTF16_ERR1              1
#define PCRE2_UTF16_ERR2              2
#define PCRE2_UTF16_ERR3              3

/* Specific error codes for UTF-32 validity checks */

#define PCRE2_UTF32_ERR0              0
#define PCRE2_UTF32_ERR1              1
#define PCRE2_UTF32_ERR3              2

/* Request types for pcre2_get_info() */

#define PCRE2_INFO_BACKREFMAX         0
#define PCRE2_INFO_CAPTURECOUNT       1
#define PCRE2_INFO_COMPILE_OPTIONS    2
#define PCRE2_INFO_FIRSTCODEUNIT      3
#define PCRE2_INFO_FIRSTCODETYPE      4
#define PCRE2_INFO_FIRSTTABLE         5
#define PCRE2_INFO_HASCRORLF          6
#define PCRE2_INFO_JCHANGED           7
#define PCRE2_INFO_JIT                8
#define PCRE2_INFO_JITSIZE            9
#define PCRE2_INFO_LASTCODEUNIT      10
#define PCRE2_INFO_LASTCODETYPE      11
#define PCRE2_INFO_MATCH_EMPTY       12
#define PCRE2_INFO_MATCHLIMIT        13
#define PCRE2_INFO_MAXLOOKBEHIND     14
#define PCRE2_INFO_MINLENGTH         15
#define PCRE2_INFO_NAMECOUNT         16
#define PCRE2_INFO_NAMEENTRYSIZE     17
#define PCRE2_INFO_NAMETABLE         18
#define PCRE2_INFO_PATTERN_OPTIONS   19
#define PCRE2_INFO_RECURSIONLIMIT    20
#define PCRE2_INFO_SIZE              21

/* Request types for pcre2_config(). */

#define PCRE2_CONFIG_BSR                     0
#define PCRE2_CONFIG_JIT                     1
#define PCRE2_CONFIG_JITTARGET               2
#define PCRE2_CONFIG_LINK_SIZE               3
#define PCRE2_CONFIG_MATCH_LIMIT             4
#define PCRE2_CONFIG_MATCH_LIMIT_RECURSION   5
#define PCRE2_CONFIG_NEWLINE                 6
#define PCRE2_CONFIG_PARENS_LIMIT            7
#define PCRE2_CONFIG_POSIX_MALLOC_THRESHOLD  8
#define PCRE2_CONFIG_STACKRECURSE            9
#define PCRE2_CONFIG_UTF                    10

/* Types for patterns and subject strings. */

typedef uint8_t  PCRE2_UCHAR8;
typedef uint16_t PCRE2_UCHAR16;
typedef uint32_t PCRE2_UCHAR32;

typedef const PCRE2_UCHAR8  *PCRE2_SPTR8;
typedef const PCRE2_UCHAR16 *PCRE2_SPTR16;
typedef const PCRE2_UCHAR32 *PCRE2_SPTR32;

/* Generic types for opaque structures and JIT callback functions. These
declarations are defined in a macro that is expanded for each width later. */

#define PCRE2_TYPES_LIST \
struct pcre2_real_context; \
typedef struct pcre2_real_context pcre2_context; \
\
struct pcre2_real; \
typedef struct pcre2_real pcre2; \
\
struct pcre2_real_match_data; \
typedef struct pcre2_real_match_data pcre2_match_data; \
\
struct pcre2_real_jit_stack; \
typedef struct pcre2_real_jit_stack pcre2_jit_stack; \
\
typedef pcre2_jit_stack *(*pcre2_jit_callback)(void *);

/* The structure for passing out data via the pcre_callout_function. We use a
structure so that new fields can be added on the end in future versions,
without changing the API of the function, thereby allowing old clients to work
without modification. Define the generic version in a macro; the width-specific
versions are generated from this macro below. */

#define PCRE2_STRUCTURE_LIST \
typedef struct pcre2_callout_block { \
  int          version;           /* Identifies version of block */ \
  /* ------------------------ Version 0 ------------------------------- */ \
  int          callout_number;    /* Number compiled into pattern */ \
  int         *offset_vector;     /* The offset vector */ \
  PCRE2_SPTR   subject;           /* The subject being matched */ \
  int          subject_length;    /* The length of the subject */ \
  int          start_match;       /* Offset to start of this match attempt */ \
  int          current_position;  /* Where we currently are in the subject */ \
  int          capture_top;       /* Max current capture */ \
  int          capture_last;      /* Most recently closed capture */ \
  void        *callout_data;      /* Data passed in with the call */ \
  /* ------------------- Added for Version 1 -------------------------- */ \
  int          pattern_position;  /* Offset to next item in the pattern */ \
  int          next_item_length;  /* Length of next item in the pattern */ \
  /* ------------------- Added for Version 2 -------------------------- */ \
  PCRE2_SPTR   mark;              /* Pointer to current mark or NULL    */ \
  /* ------------------------------------------------------------------ */ \
} pcre2_callout_block;

/* Utility functions for byte order swaps. These are not generic functions;
each appears only its own library. */

PCRE2_EXP_DECL int  pcre2_utf16_to_host_byte_order(PCRE2_UCHAR16 *,
                  PCRE2_SPTR16, int, int *, int);
PCRE2_EXP_DECL int  pcre2_utf32_to_host_byte_order(PCRE2_UCHAR32 *,
                  PCRE2_SPTR32, int, int *, int);

/* List the generic forms of all other functions in a macro. */

#define PCRE2_FUNCTION_LIST \
PCRE2_EXP_DECL pcre2    *pcre2_compile(pcre2_context *,  PCRE2_SPTR, int, \
                           uint32_t, int *, size_t *); \
PCRE2_EXP_DECL int       pcre2_config(int, void *); \
PCRE2_EXP_DECL size_t    pcre2_context_size(void); \
PCRE2_EXP_DECL \
  pcre2_context         *pcre2_copy_context(pcre2_context *); \
PCRE2_EXP_DECL int       pcre2_copy_named_substring(pcre2_match_data *, \
                           PCRE2_SPTR, PCRE2_UCHAR *, size_t); \
PCRE2_EXP_DECL int       pcre2_copy_substring(pcre2_match_data *, int, \
                           PCRE2_UCHAR *, size_t); \
PCRE2_EXP_DECL \
  pcre2_match_data      *pcre2_create_match_data(pcre2_context *, size_t); \
PCRE2_EXP_DECL int       pcre2_dfa_exec(pcre2_context *, const pcre2 *, \
                           PCRE2_SPTR, int, size_t, uint32_t, \
                           pcre2_match_data *, int *, size_t); \
PCRE2_EXP_DECL int       pcre2_exec(pcre2_context *, const pcre2 *, \
                           PCRE2_SPTR, int, size_t, uint32_t, \
                           pcre2_match_data *); \
PCRE2_EXP_DECL void      pcre2_free_compiled_code(pcre2_context *, pcre2 *); \
PCRE2_EXP_DECL void      pcre2_free_context(pcre2_context *); \
PCRE2_EXP_DECL void      pcre2_free_match_data(pcre2_context *, \
                           pcre2_match_data *); \
PCRE2_EXP_DECL void      pcre2_free_substring(pcre2_context *, PCRE2_SPTR); \
PCRE2_EXP_DECL void      pcre2_free_substring_list(pcre2_context *, \
                           PCRE2_SPTR *); \
PCRE2_EXP_DECL uint32_t  pcre2_get_bsr_convention(pcre2_context *); \
PCRE2_EXP_DECL int     (*pcre2_get_callout(pcre2_context *)) \
                           (pcre2_callout_block *, void *); \
PCRE2_EXP_DECL \
  unsigned char         *pcre2_get_character_tables(pcre2_context *); \
PCRE2_EXP_DECL uint32_t  pcre2_get_context_options(pcre2_context *); \
PCRE2_EXP_DECL int       pcre2_get_error_message(int, PCRE2_UCHAR *, size_t); \
PCRE2_EXP_DECL size_t    pcre2_get_error_offset(pcre2_match_data *); \
PCRE2_EXP_DECL int       pcre2_get_error_reason(pcre2_match_data *); \
PCRE2_EXP_DECL size_t    pcre2_get_exec_frame_size(void); \
PCRE2_EXP_DECL int       pcre2_get_info(const pcre2 *, uint32_t, void *); \
PCRE2_EXP_DECL size_t    pcre2_get_leftchar(pcre2_match_data *); \
PCRE2_EXP_DECL \
  PCRE2_SPTR             pcre2_get_mark(pcre2_match_data *); \
PCRE2_EXP_DECL uint32_t  pcre2_get_match_limit(pcre2_context *); \
PCRE2_EXP_DECL void      pcre2_get_memory_management(pcre2_context *, \
                           void *(**)(size_t, void *), \
                           void (**)(void *, void *)); \
PCRE2_EXP_DECL int       pcre2_get_named_substring(pcre2_context *, \
                           pcre2_match_data *, PCRE2_SPTR, PCRE2_UCHAR **); \
PCRE2_EXP_DECL int       pcre2_get_named_substring_length(pcre2_match_data *, \
                           PCRE2_SPTR); \
PCRE2_EXP_DECL uint32_t  pcre2_get_newline_convention(pcre2_context *); \
PCRE2_EXP_DECL size_t   *pcre2_get_ovector(pcre2_match_data *); \
PCRE2_EXP_DECL size_t   *pcre2_get_ovector_slots(pcre2_match_data *); \
PCRE2_EXP_DECL uint32_t  pcre2_get_parens_nest_limit(pcre2_context *); \
PCRE2_EXP_DECL int     (*pcre2_get_recursion_guard(pcre2_context *)) \
                           (int, void *); \
PCRE2_EXP_DECL uint32_t  pcre2_get_recursion_limit(pcre2_context *); \
PCRE2_EXP_DECL void      pcre2_get_recursion_memory_management(\
                           pcre2_context *, \
                           void *(**)(size_t, void *), \
                           void (**)(void *, void *)); \
PCRE2_EXP_DECL size_t    pcre2_get_rightchar(pcre2_match_data *); \
PCRE2_EXP_DECL size_t    pcre2_get_startchar(pcre2_match_data *); \
PCRE2_EXP_DECL int       pcre2_get_stringtable_entries(const pcre2 *, \
                           PCRE2_SPTR, PCRE2_UCHAR **, PCRE2_UCHAR **); \
PCRE2_EXP_DECL int       pcre2_get_substring(pcre2_context *, \
                           pcre2_match_data *, int, PCRE2_UCHAR **); \
PCRE2_EXP_DECL int       pcre2_get_substring_length(pcre2_match_data *, int); \
PCRE2_EXP_DECL int       pcre2_get_substring_list(pcre2_context *, \
                           pcre2_match_data *, PCRE2_UCHAR ***); \
PCRE2_EXP_DECL int       pcre2_get_substring_number(const pcre2 *, \
                           PCRE2_SPTR); \
PCRE2_EXP_DECL void     *pcre2_get_user_data(pcre2_context *); \
PCRE2_EXP_DECL \
  pcre2_context         *pcre2_init_context(pcre2_context *); \
PCRE2_EXP_DECL int       pcre2_jit_compile(pcre2_context *, pcre2 *, \
                           uint32_t); \
PCRE2_EXP_DECL int       pcre2_jit_exec(pcre2_context *, const pcre2 *, \
                           PCRE2_SPTR, int, size_t, uint32_t, \
                           pcre2_match_data *, pcre2_jit_stack *); \
PCRE2_EXP_DECL void      pcre2_jit_free_unused_memory(pcre2_context *); \
PCRE2_EXP_DECL \
  pcre2_jit_stack       *pcre2_jit_stack_alloc(pcre2_context *, size_t, \
                           size_t); \
PCRE2_EXP_DECL void      pcre2_jit_stack_assign(pcre2_context *, \
                           const pcre2 *, pcre2_jit_callback, void *); \
PCRE2_EXP_DECL void      pcre2_jit_stack_free(pcre2_context *, \
                           pcre2_jit_stack *); \
PCRE2_EXP_DECL \
  const unsigned char   *pcre2_maketables(void); \
PCRE2_EXP_DECL int       pcre2_pattern_to_host_byte_order(pcre2 *); \
PCRE2_EXP_DECL void      pcre2_set_bsr_convention(pcre2_context *, uint32_t); \
PCRE2_EXP_DECL void      pcre2_set_callout(pcre2_context *, \
                           int (*)(pcre2_callout_block *, void *)); \
PCRE2_EXP_DECL void      pcre2_set_character_tables(pcre2_context *, \
                           unsigned char *); \
PCRE2_EXP_DECL void      pcre2_set_context_options(pcre2_context *, uint32_t, \
                           uint32_t); \
PCRE2_EXP_DECL void      pcre2_set_match_limit(pcre2_context *, uint32_t); \
PCRE2_EXP_DECL void      pcre2_set_memory_management(pcre2_context *, \
                           void * (*)(size_t, void *), \
                           void (*)(void *, void *)); \
PCRE2_EXP_DECL void      pcre2_set_newline_convention(pcre2_context *, \
                           uint32_t); \
PCRE2_EXP_DECL void      pcre2_set_parens_nest_limit(pcre2_context *, \
                           uint32_t); \
PCRE2_EXP_DECL void      pcre2_set_recursion_guard(pcre2_context *, \
                           int (*)(uint32_t, void *)); \
PCRE2_EXP_DECL void      pcre2_set_recursion_limit(pcre2_context *, uint32_t); \
PCRE2_EXP_DECL void      pcre2_set_recursion_memory_management(\
                           pcre2_context *, void * (*)(size_t, void *), \
                           void (*)(void *, void *)); \
PCRE2_EXP_DECL void      pcre2_set_user_data(pcre2_context *, void *); \
PCRE2_EXP_DECL int       pcre2_version(PCRE2_UCHAR *, size_t);


/* Define macros that generate width-specific names from generic versions. The
three-level macro scheme is necessary to get the macros expanded when we want
them to be. First we get the width from PCRE2_LOCAL_WIDTH, which is used for
generating three versions of each external function below. After that,
PCRE2_ADDWIDTH will be re-defined to use PCRE2_DATA_WIDTH, for use when macros
such as pcre2_compile are called by application code. */

#define PCRE2_JOIN(a,b) a ## b
#define PCRE2_GLUE(a,b) PCRE2_JOIN(a,b)
#define PCRE2_ADDWIDTH(a) PCRE2_GLUE(a,PCRE2_LOCAL_WIDTH)

/* Data types */

#define PCRE2_UCHAR            PCRE2_ADDWIDTH(PCRE2_UCHAR)
#define PCRE2_SPTR             PCRE2_ADDWIDTH(PCRE2_SPTR)

#define pcre2                  PCRE2_ADDWIDTH(pcre2_)
#define pcre2_jit_callback     PCRE2_ADDWIDTH(pcre2_jit_callback_)
#define pcre2_jit_stack        PCRE2_ADDWIDTH(pcre2_jit_stack_)

#define pcre2_real             PCRE2_ADDWIDTH(pcre2_real_)
#define pcre2_real_context     PCRE2_ADDWIDTH(pcre2_real_context_)
#define pcre2_real_jit_stack   PCRE2_ADDWIDTH(pcre2_real_jit_stack_)
#define pcre2_real_match_data  PCRE2_ADDWIDTH(pcre2_real_match_data_)

/* Data blocks */

#define pcre2_callout_block    PCRE2_ADDWIDTH(pcre2_callout_block_)
#define pcre2_context          PCRE2_ADDWIDTH(pcre2_context_)
#define pcre2_match_data       PCRE2_ADDWIDTH(pcre2_match_data_)

/* Functions */

#define pcre2_compile                         PCRE2_ADDWIDTH(pcre2_compile_)
#define pcre2_config                          PCRE2_ADDWIDTH(pcre2_config_)
#define pcre2_context_size                    PCRE2_ADDWIDTH(pcre2_context_size_)
#define pcre2_copy_context                    PCRE2_ADDWIDTH(pcre2_copy_context_)
#define pcre2_copy_named_substring            PCRE2_ADDWIDTH(pcre2_copy_named_substring_)
#define pcre2_copy_substring                  PCRE2_ADDWIDTH(pcre2_copy_substring_)
#define pcre2_create_match_data               PCRE2_ADDWIDTH(pcre2_create_match_data_)
#define pcre2_dfa_exec                        PCRE2_ADDWIDTH(pcre2_dfa_exec_)
#define pcre2_exec                            PCRE2_ADDWIDTH(pcre2_exec_)
#define pcre2_free_compiled_code              PCRE2_ADDWIDTH(pcre2_free_compiled_code_)
#define pcre2_free_context                    PCRE2_ADDWIDTH(pcre2_free_context_)
#define pcre2_free_match_data                 PCRE2_ADDWIDTH(pcre2_free_match_data_)
#define pcre2_free_substring                  PCRE2_ADDWIDTH(pcre2_free_substring_)
#define pcre2_free_substring_list             PCRE2_ADDWIDTH(pcre2_free_substring_list_)
#define pcre2_get_bsr_convention              PCRE2_ADDWIDTH(pcre2_get_bsr_convention_)
#define pcre2_get_callout                     PCRE2_ADDWIDTH(pcre2_get_callout_)
#define pcre2_get_character_tables            PCRE2_ADDWIDTH(pcre2_get_character_tables_)
#define pcre2_get_context_options             PCRE2_ADDWIDTH(pcre2_get_context_options_)
#define pcre2_get_error_message               PCRE2_ADDWIDTH(pcre2_get_error_message_)
#define pcre2_get_error_offset                PCRE2_ADDWIDTH(pcre2_get_error_offset_)
#define pcre2_get_error_reason                PCRE2_ADDWIDTH(pcre2_get_error_reason_)
#define pcre2_get_info                        PCRE2_ADDWIDTH(pcre2_get_info_)
#define pcre2_get_leftchar                    PCRE2_ADDWIDTH(pcre2_get_leftchar_)
#define pcre2_get_mark                        PCRE2_ADDWIDTH(pcre2_get_mark_)
#define pcre2_get_match_limit                 PCRE2_ADDWIDTH(pcre2_get_match_limit_)
#define pcre2_get_memory_management           PCRE2_ADDWIDTH(pcre2_get_memory_management_)
#define pcre2_get_named_substring             PCRE2_ADDWIDTH(pcre2_get_named_substring_)
#define pcre2_get_named_substring_length      PCRE2_ADDWIDTH(pcre2_get_named_substring_length_)
#define pcre2_get_newline_convention          PCRE2_ADDWIDTH(pcre2_get_newline_convention_)
#define pcre2_get_ovector                     PCRE2_ADDWIDTH(pcre2_get_ovector_)
#define pcre2_get_ovector_slots               PCRE2_ADDWIDTH(pcre2_get_ovector_slots_)
#define pcre2_get_parens_nest_limit           PCRE2_ADDWIDTH(pcre2_get_parens_nest_limit_)
#define pcre2_get_recursion_guard             PCRE2_ADDWIDTH(pcre2_get_recursion_guard_)
#define pcre2_get_recursion_limit             PCRE2_ADDWIDTH(pcre2_get_recursion_limit_)
#define pcre2_get_recursion_memory_management PCRE2_ADDWIDTH(pcre2_get_recursion_memory_management_)
#define pcre2_get_rightchar                   PCRE2_ADDWIDTH(pcre2_get_rightchar_)
#define pcre2_get_startchar                   PCRE2_ADDWIDTH(pcre2_get_startchar_)
#define pcre2_get_stringtable_entries         PCRE2_ADDWIDTH(pcre2_get_stringtable_entries_)
#define pcre2_get_substring                   PCRE2_ADDWIDTH(pcre2_get_substring_)
#define pcre2_get_substring_length            PCRE2_ADDWIDTH(pcre2_get_substring_length_)
#define pcre2_get_substring_list              PCRE2_ADDWIDTH(pcre2_get_substring_list_)
#define pcre2_get_substring_number            PCRE2_ADDWIDTH(pcre2_get_substring_number_)
#define pcre2_get_user_data                   PCRE2_ADDWIDTH(pcre2_get_user_data_)
#define pcre2_init_context                    PCRE2_ADDWIDTH(pcre2_init_context_)
#define pcre2_jit_compile                     PCRE2_ADDWIDTH(pcre2_jit_compile_)
#define pcre2_jit_exec                        PCRE2_ADDWIDTH(pcre2_jit_exec_)
#define pcre2_jit_free_unused_memory          PCRE2_ADDWIDTH(pcre2_jit_free_unused_memory_)
#define pcre2_jit_stack_alloc                 PCRE2_ADDWIDTH(pcre2_jit_stack_alloc_)
#define pcre2_jit_stack_assign                PCRE2_ADDWIDTH(pcre2_jit_stack_assign_)
#define pcre2_jit_stack_free                  PCRE2_ADDWIDTH(pcre2_jit_stack_free_)
#define pcre2_maketables                      PCRE2_ADDWIDTH(pcre2_maketables_)
#define pcre2_pattern_to_host_byte_order      PCRE2_ADDWIDTH(pcre2_pattern_to_host_byte_order_)
#define pcre2_set_bsr_convention              PCRE2_ADDWIDTH(pcre2_set_bsr_convention_)
#define pcre2_set_callout                     PCRE2_ADDWIDTH(pcre2_set_callout_)
#define pcre2_set_character_tables            PCRE2_ADDWIDTH(pcre2_set_character_tables_)
#define pcre2_set_context_options             PCRE2_ADDWIDTH(pcre2_set_context_options_)
#define pcre2_set_match_limit                 PCRE2_ADDWIDTH(pcre2_set_match_limit_)
#define pcre2_set_memory_management           PCRE2_ADDWIDTH(pcre2_set_memory_management_)
#define pcre2_set_newline_convention          PCRE2_ADDWIDTH(pcre2_set_newline_convention_)
#define pcre2_set_recursion_guard             PCRE2_ADDWIDTH(pcre2_set_recursion_guard_)
#define pcre2_set_parens_nest_limit           PCRE2_ADDWIDTH(pcre2_set_parens_nest_limit_)
#define pcre2_set_recursion_limit             PCRE2_ADDWIDTH(pcre2_set_recursion_limit_)
#define pcre2_set_recursion_memory_management PCRE2_ADDWIDTH(pcre2_set_recursion_memory_management_)
#define pcre2_set_user_data                   PCRE2_ADDWIDTH(pcre2_set_user_data_)
#define pcre2_version                         PCRE2_ADDWIDTH(pcre2_version_)

/* Now generate all three sets of width-specific structures and function
prototypes. */

#define PCRE2_LOCAL_WIDTH 8
PCRE2_TYPES_LIST
PCRE2_STRUCTURE_LIST
PCRE2_FUNCTION_LIST
#undef PCRE2_LOCAL_WIDTH

#define PCRE2_LOCAL_WIDTH 16
PCRE2_TYPES_LIST
PCRE2_STRUCTURE_LIST
PCRE2_FUNCTION_LIST
#undef PCRE2_LOCAL_WIDTH

#define PCRE2_LOCAL_WIDTH 32
PCRE2_TYPES_LIST
PCRE2_STRUCTURE_LIST
PCRE2_FUNCTION_LIST
#undef PCRE2_LOCAL_WIDTH

#undef PCRE2_TYPES_LIST
#undef PCRE2_STRUCTURE_LIST
#undef PCRE2_FUNCTION_LIST
#undef PCRE2_ADDWIDTH

/* Re-define PCRE2_ADDWIDTH to use the external width value, if defined.
Otherwise, undefine the other macros and make PCRE2_ADDWIDTH a no-op, to reduce
confusion. */

#ifdef PCRE2_DATA_WIDTH
#if PCRE2_DATA_WIDTH != 8 && PCRE2_DATA_WIDTH != 16 && PCRE2_DATA_WIDTH != 32
#error PCRE2_DATA_WIDTH must be 8, 16, or 32
#endif
#define PCRE2_ADDWIDTH(a) PCRE2_GLUE(a,PCRE2_DATA_WIDTH)
#else
#undef PCRE2_JOIN
#undef PCRE2_GLUE
#define PCRE2_ADDWIDTH(a) a
#endif

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* End of pcre2.h */
