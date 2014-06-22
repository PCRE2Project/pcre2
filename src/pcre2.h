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

#define PCRE2_MAJOR          10
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

/* The following options can be passed to pcre2_compile(), pcre2_match(), or
pcre2_dfa_match(). PCRE2_NO_UTF_CHECK affects only the function to which it is
passed. */

#define PCRE2_ANCHORED            0x00000001
#define PCRE2_NO_START_OPTIMIZE   0x00000002
#define PCRE2_NO_UTF_CHECK        0x00000004

/* Other options that can be passed to pcre2_compile(). They may affect
compilation, JIT compilation, and/or interpretive execution. The following tags
indicate which:

C   alters what is compiled
J   alters what JIT compiles
E   is inspected during pcre2_match() execution
D   is inspected during pcre2_dfa_match() execution
*/

#define PCRE2_ALLOW_EMPTY_CLASS   0x00000008   /* C       */
#define PCRE2_ALT_BSUX            0x00000010   /* C       */
#define PCRE2_AUTO_CALLOUT        0x00000020   /* C       */
#define PCRE2_CASELESS            0x00000040   /* C       */
#define PCRE2_DOLLAR_ENDONLY      0x00000080   /*   J E D */
#define PCRE2_DOTALL              0x00000100   /* C       */
#define PCRE2_DUPNAMES            0x00000200   /* C       */
#define PCRE2_EXTENDED            0x00000400   /* C       */
#define PCRE2_FIRSTLINE           0x00000800   /*   J E D */
#define PCRE2_MATCH_UNSET_BACKREF 0x00001000   /* C J E   */
#define PCRE2_MULTILINE           0x00002000   /* C       */
#define PCRE2_NEVER_UCP           0x00004000   /* C       */
#define PCRE2_NEVER_UTF           0x00008000   /* C       */
#define PCRE2_NO_AUTO_CAPTURE     0x00010000   /* C       */
#define PCRE2_NO_AUTO_POSSESS     0x00020000   /* C       */
#define PCRE2_UCP                 0x00040000   /* C J E D */
#define PCRE2_UNGREEDY            0x00080000   /* C       */
#define PCRE2_UTF                 0x00100000   /* C J E D */

/* These are for pcre2_jit_compile(). */

#define PCRE2_JIT                 0x00000001   /* For full matching */
#define PCRE2_JIT_PARTIAL_SOFT    0x00000002
#define PCRE2_JIT_PARTIAL_HARD    0x00000004

/* These are for pcre2_match() and pcre2_dfa_match(). */

#define PCRE2_NOTBOL              0x00000001
#define PCRE2_NOTEOL              0x00000002
#define PCRE2_NOTEMPTY            0x00000004
#define PCRE2_NOTEMPTY_ATSTART    0x00000008
#define PCRE2_PARTIAL_SOFT        0x00000010
#define PCRE2_PARTIAL_HARD        0x00000020

/* These are additional options for pcre2_dfa_match(). */

#define PCRE2_DFA_RESTART         0x00000040
#define PCRE2_DFA_SHORTEST        0x00000080

/* Newline and \R settings, for use in the compile context. */

#define PCRE2_NEWLINE_CR          1
#define PCRE2_NEWLINE_LF          2
#define PCRE2_NEWLINE_CRLF        3
#define PCRE2_NEWLINE_ANY         4
#define PCRE2_NEWLINE_ANYCRLF     5

#define PCRE2_BSR_UNICODE         1
#define PCRE2_BSR_ANYCRLF         2

/* Error codes: no match and partial match are "expected" errors. */

#define PCRE2_ERROR_NOMATCH          (-1)
#define PCRE2_ERROR_PARTIAL          (-2)

/* Error codes for UTF-8 validity checks */

#define PCRE2_ERROR_UTF8_ERR1        (-3)
#define PCRE2_ERROR_UTF8_ERR2        (-4)
#define PCRE2_ERROR_UTF8_ERR3        (-5)
#define PCRE2_ERROR_UTF8_ERR4        (-6)
#define PCRE2_ERROR_UTF8_ERR5        (-7)
#define PCRE2_ERROR_UTF8_ERR6        (-8)
#define PCRE2_ERROR_UTF8_ERR7        (-9)
#define PCRE2_ERROR_UTF8_ERR8       (-10)
#define PCRE2_ERROR_UTF8_ERR9       (-11)
#define PCRE2_ERROR_UTF8_ERR10      (-12)
#define PCRE2_ERROR_UTF8_ERR11      (-13)
#define PCRE2_ERROR_UTF8_ERR12      (-14)
#define PCRE2_ERROR_UTF8_ERR13      (-15)
#define PCRE2_ERROR_UTF8_ERR14      (-16)
#define PCRE2_ERROR_UTF8_ERR15      (-17)
#define PCRE2_ERROR_UTF8_ERR16      (-18)
#define PCRE2_ERROR_UTF8_ERR17      (-19)
#define PCRE2_ERROR_UTF8_ERR18      (-20)
#define PCRE2_ERROR_UTF8_ERR19      (-21)
#define PCRE2_ERROR_UTF8_ERR20      (-22)
#define PCRE2_ERROR_UTF8_ERR21      (-23)

/* Error codes for UTF-16 validity checks */

#define PCRE2_ERROR_UTF16_ERR1      (-24)
#define PCRE2_ERROR_UTF16_ERR2      (-25)
#define PCRE2_ERROR_UTF16_ERR3      (-26)

/* Error codes for UTF-32 validity checks */

#define PCRE2_ERROR_UTF32_ERR1      (-27)
#define PCRE2_ERROR_UTF32_ERR2      (-28)

/* Error codes for pcre2[_dfa]_match() */

#define PCRE2_ERROR_BADCOUNT        (-29)
#define PCRE2_ERROR_BADENDIANNESS   (-30)
#define PCRE2_ERROR_BADLENGTH       (-31)
#define PCRE2_ERROR_BADMAGIC        (-32)
#define PCRE2_ERROR_BADMODE         (-33)
#define PCRE2_ERROR_BADOFFSET       (-34)
#define PCRE2_ERROR_BADOPTION       (-35)
#define PCRE2_ERROR_BADUTF_OFFSET   (-36)
#define PCRE2_ERROR_CALLOUT         (-37)  /* Never used by PCRE2 itself */
#define PCRE2_ERROR_DFA_BADRESTART  (-38)
#define PCRE2_ERROR_DFA_RECURSE     (-39)
#define PCRE2_ERROR_DFA_UCOND       (-40)
#define PCRE2_ERROR_DFA_UITEM       (-41)
#define PCRE2_ERROR_DFA_UMLIMIT     (-42)
#define PCRE2_ERROR_DFA_WSSIZE      (-43)
#define PCRE2_ERROR_INTERNAL        (-44)
#define PCRE2_ERROR_JIT_BADOPTION   (-45)
#define PCRE2_ERROR_JIT_STACKLIMIT  (-46)
#define PCRE2_ERROR_MATCHLIMIT      (-47)
#define PCRE2_ERROR_NOMEMORY        (-48)
#define PCRE2_ERROR_NOSUBSTRING     (-49)
#define PCRE2_ERROR_NULL            (-50)
#define PCRE2_ERROR_RECURSELOOP     (-51)
#define PCRE2_ERROR_RECURSIONLIMIT  (-52)
#define PCRE2_ERROR_UNKNOWN_OPCODE  (-53)
#define PCRE2_ERROR_UNSET           (-54)

/* Request types for pcre2_pattern_info() */

#define PCRE2_INFO_BACKREFMAX            0
#define PCRE2_INFO_BSR_CONVENTION        1
#define PCRE2_INFO_CAPTURECOUNT          2
#define PCRE2_INFO_COMPILE_OPTIONS       3
#define PCRE2_INFO_FIRSTCODEUNIT         4
#define PCRE2_INFO_FIRSTCODETYPE         5
#define PCRE2_INFO_FIRSTBITMAP           6
#define PCRE2_INFO_HASCRORLF             7
#define PCRE2_INFO_JCHANGED              8
#define PCRE2_INFO_JIT                   9
#define PCRE2_INFO_JITSIZE              10
#define PCRE2_INFO_LASTCODEUNIT         11
#define PCRE2_INFO_LASTCODETYPE         12
#define PCRE2_INFO_MATCH_EMPTY          13
#define PCRE2_INFO_MATCH_LIMIT          14
#define PCRE2_INFO_MAXLOOKBEHIND        15
#define PCRE2_INFO_MINLENGTH            16
#define PCRE2_INFO_NAMECOUNT            17
#define PCRE2_INFO_NAMEENTRYSIZE        18
#define PCRE2_INFO_NAMETABLE            19
#define PCRE2_INFO_NEWLINE_CONVENTION   20
#define PCRE2_INFO_PATTERN_OPTIONS      21
#define PCRE2_INFO_RECURSION_LIMIT      22
#define PCRE2_INFO_SIZE                 23

/* Request types for pcre2_config(). */

#define PCRE2_CONFIG_BSR                     0
#define PCRE2_CONFIG_JIT                     1
#define PCRE2_CONFIG_JITTARGET               2
#define PCRE2_CONFIG_LINK_SIZE               3
#define PCRE2_CONFIG_MATCH_LIMIT             4
#define PCRE2_CONFIG_MATCH_LIMIT_RECURSION   5
#define PCRE2_CONFIG_NEWLINE                 6
#define PCRE2_CONFIG_PARENS_LIMIT            7
#define PCRE2_CONFIG_STACKRECURSE            8
#define PCRE2_CONFIG_UTF                     9

/* A value that is used to indicate 'unset' in unsigned size_t fields. In 
particular, this value is used in the ovector. */

#define PCRE2_UNSET (~(size_t)0)

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
struct pcre2_real_general_context; \
typedef struct pcre2_real_general_context pcre2_general_context; \
\
struct pcre2_real_compile_context; \
typedef struct pcre2_real_compile_context pcre2_compile_context; \
\
struct pcre2_real_match_context; \
typedef struct pcre2_real_match_context pcre2_match_context; \
\
struct pcre2_real_code; \
typedef struct pcre2_real_code pcre2_code; \
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


/* List the generic forms of all other functions in macros, which will be
expanded for each width below. Start with functions that give general
information. */

#define PCRE2_GENERAL_INFO_FUNCTIONS \
PCRE2_EXP_DECL int       pcre2_config(int, void *); \
PCRE2_EXP_DECL size_t    pcre2_get_match_frame_size(void); \
PCRE2_EXP_DECL int       pcre2_version(PCRE2_UCHAR *, size_t);


/* Functions for manipulating contexts. */

#define PCRE2_GENERAL_CONTEXT_FUNCTIONS \
PCRE2_EXP_DECL \
  pcre2_general_context *pcre2_general_context_copy(pcre2_general_context *); \
PCRE2_EXP_DECL \
  pcre2_general_context *pcre2_general_context_create( \
    void *(*)(size_t, void *), \
    void (*)(void *, void *), void *); \
PCRE2_EXP_DECL void      pcre2_general_context_free(pcre2_general_context *);

#define PCRE2_COMPILE_CONTEXT_FUNCTIONS \
PCRE2_EXP_DECL \
  pcre2_compile_context *pcre2_compile_context_copy(pcre2_compile_context *); \
PCRE2_EXP_DECL \
  pcre2_compile_context *pcre2_compile_context_create(pcre2_general_context *);\
PCRE2_EXP_DECL void      pcre2_compile_context_free(pcre2_compile_context *); \
PCRE2_EXP_DECL int       pcre2_set_bsr_convention(pcre2_compile_context *, \
                           uint32_t); \
PCRE2_EXP_DECL int       pcre2_set_character_tables(pcre2_compile_context *, \
                           const unsigned char *); \
PCRE2_EXP_DECL int       pcre2_set_newline_convention(pcre2_compile_context *, \
                           uint32_t); \
PCRE2_EXP_DECL int       pcre2_set_parens_nest_limit(pcre2_compile_context *, \
                           uint32_t); \
PCRE2_EXP_DECL int       pcre2_set_compile_recursion_guard(\
                           pcre2_compile_context *, int (*)(uint32_t)); \

#define PCRE2_MATCH_CONTEXT_FUNCTIONS \
PCRE2_EXP_DECL \
  pcre2_match_context   *pcre2_match_context_copy(pcre2_match_context *); \
PCRE2_EXP_DECL \
  pcre2_match_context   *pcre2_match_context_create(pcre2_general_context *); \
PCRE2_EXP_DECL void      pcre2_match_context_free(pcre2_match_context *); \
PCRE2_EXP_DECL int       pcre2_set_callout(pcre2_match_context *, \
                           int (*)(pcre2_callout_block *, void *)); \
PCRE2_EXP_DECL int       pcre2_set_match_limit(pcre2_match_context *, \
                           uint32_t); \
PCRE2_EXP_DECL int       pcre2_set_recursion_limit(pcre2_match_context *, \
                           uint32_t); \
PCRE2_EXP_DECL int       pcre2_set_recursion_memory_management( \
                           pcre2_match_context *, void *(*)(size_t, void *), \
                           void (*)(void *, void *));


/* Functions concerned with compiling a pattern to PCRE internal code. */

#define PCRE2_COMPILE_FUNCTIONS \
PCRE2_EXP_DECL \
  pcre2_code            *pcre2_compile(PCRE2_SPTR, int, uint32_t, \
                           int *, size_t *, pcre2_compile_context *); \
PCRE2_EXP_DECL void      pcre2_code_free(pcre2_code *);


/* Functions that give information about a compiled pattern. */

#define PCRE2_PATTERN_INFO_FUNCTIONS \
PCRE2_EXP_DECL int       pcre2_pattern_info(const pcre2_code *, uint32_t, \
                           void *);


/* Functions for running a match and inspecting the result. */

#define PCRE2_MATCH_FUNCTIONS \
PCRE2_EXP_DECL \
  pcre2_match_data      *pcre2_match_data_create(size_t, \
                           pcre2_general_context *); \
PCRE2_EXP_DECL \
  pcre2_match_data      *pcre2_match_data_create_from_pattern(pcre2_code *, \
                           pcre2_general_context *); \
PCRE2_EXP_DECL int       pcre2_dfa_match(const pcre2_code *, \
                           PCRE2_SPTR, int, size_t, uint32_t, \
                           pcre2_match_data *, pcre2_match_context *, int *, \
                           size_t); \
PCRE2_EXP_DECL int       pcre2_match(const pcre2_code *, \
                           PCRE2_SPTR, int, size_t, uint32_t, \
                           pcre2_match_data *, pcre2_match_context *); \
PCRE2_EXP_DECL void      pcre2_match_data_free(pcre2_match_data *); \
PCRE2_EXP_DECL size_t    pcre2_get_leftchar(pcre2_match_data *); \
PCRE2_EXP_DECL \
  PCRE2_SPTR             pcre2_get_mark(pcre2_match_data *); \
PCRE2_EXP_DECL size_t    pcre2_get_ovector_count(pcre2_match_data *); \
PCRE2_EXP_DECL size_t   *pcre2_get_ovector_pointer(pcre2_match_data *); \
PCRE2_EXP_DECL size_t    pcre2_get_rightchar(pcre2_match_data *); \
PCRE2_EXP_DECL size_t    pcre2_get_startchar(pcre2_match_data *);


/* Convenience functions for handling matched substrings. */

#define PCRE2_SUBSTRING_FUNCTIONS \
PCRE2_EXP_DECL int       pcre2_substring_copy_byname(pcre2_match_data *, \
                           PCRE2_SPTR, PCRE2_UCHAR *, size_t); \
PCRE2_EXP_DECL int       pcre2_substring_copy_bynumber(pcre2_match_data *, \
                           int, PCRE2_UCHAR *, size_t); \
PCRE2_EXP_DECL void      pcre2_substring_free(PCRE2_UCHAR *); \
PCRE2_EXP_DECL int       pcre2_substring_get_byname(pcre2_match_data *, \
                           PCRE2_SPTR, PCRE2_UCHAR **); \
PCRE2_EXP_DECL int       pcre2_substring_get_bynumber(pcre2_match_data *, \
                           int, PCRE2_UCHAR **); \
PCRE2_EXP_DECL int       pcre2_substring_length_byname(pcre2_match_data *, \
                           PCRE2_SPTR); \
PCRE2_EXP_DECL int       pcre2_substring_length_bynumber(pcre2_match_data *, \
                           int); \
PCRE2_EXP_DECL int       pcre2_substring_nametable_scan(const pcre2_code *, \
                           PCRE2_SPTR, PCRE2_SPTR *, PCRE2_SPTR *); \
PCRE2_EXP_DECL int       pcre2_substring_number_from_name(\
                           const pcre2_code *, PCRE2_SPTR); \
PCRE2_EXP_DECL void      pcre2_substring_list_free(PCRE2_SPTR *); \
PCRE2_EXP_DECL int       pcre2_substring_list_get(pcre2_match_data *, \
                           PCRE2_UCHAR ***, size_t **);


/* Functions for JIT processing */

#define PCRE2_JIT_FUNCTIONS \
PCRE2_EXP_DECL void      pcre2_jit_compile(pcre2_code *, uint32_t); \
PCRE2_EXP_DECL int       pcre2_jit_match(const pcre2_code *, \
                           PCRE2_SPTR, int, size_t, uint32_t, \
                           pcre2_match_data *, pcre2_jit_stack *); \
PCRE2_EXP_DECL void      pcre2_jit_free_unused_memory(pcre2_general_context *);\
PCRE2_EXP_DECL \
  pcre2_jit_stack       *pcre2_jit_stack_alloc(pcre2_general_context *, \
                           size_t, size_t); \
PCRE2_EXP_DECL void      pcre2_jit_stack_assign(const pcre2_code *, \
                           pcre2_jit_callback, void *); \
PCRE2_EXP_DECL void      pcre2_jit_stack_free(pcre2_jit_stack *);


/* Other miscellaneous functions. */

#define PCRE2_OTHER_FUNCTIONS \
PCRE2_EXP_DECL int       pcre2_get_error_message(int, PCRE2_UCHAR *, size_t); \
PCRE2_EXP_DECL size_t    pcre2_get_match_frame_size(void); \
PCRE2_EXP_DECL \
  const unsigned char   *pcre2_maketables(pcre2_general_context *); \
PCRE2_EXP_DECL int       pcre2_pattern_to_host_byte_order(pcre2_code *);


/* Define macros that generate width-specific names from generic versions. The
three-level macro scheme is necessary to get the macros expanded when we want
them to be. First we get the width from PCRE2_LOCAL_WIDTH, which is used for
generating three versions of everything below. After that, PCRE2_SUFFIX will be
re-defined to use PCRE2_CODE_UNIT_WIDTH, for use when macros such as
pcre2_compile are called by application code. */

#define PCRE2_JOIN(a,b) a ## b
#define PCRE2_GLUE(a,b) PCRE2_JOIN(a,b)
#define PCRE2_SUFFIX(a) PCRE2_GLUE(a,PCRE2_LOCAL_WIDTH)


/* Data types */

#define PCRE2_UCHAR                 PCRE2_SUFFIX(PCRE2_UCHAR)
#define PCRE2_SPTR                  PCRE2_SUFFIX(PCRE2_SPTR)

#define pcre2_code                  PCRE2_SUFFIX(pcre2_code_)
#define pcre2_jit_callback          PCRE2_SUFFIX(pcre2_jit_callback_)
#define pcre2_jit_stack             PCRE2_SUFFIX(pcre2_jit_stack_)

#define pcre2_real_code             PCRE2_SUFFIX(pcre2_real_code_)
#define pcre2_real_general_context  PCRE2_SUFFIX(pcre2_real_general_context_)
#define pcre2_real_compile_context  PCRE2_SUFFIX(pcre2_real_compile_context_)
#define pcre2_real_match_context    PCRE2_SUFFIX(pcre2_real_match_context_)
#define pcre2_real_jit_stack        PCRE2_SUFFIX(pcre2_real_jit_stack_)
#define pcre2_real_match_data       PCRE2_SUFFIX(pcre2_real_match_data_)


/* Data blocks */

#define pcre2_callout_block         PCRE2_SUFFIX(pcre2_callout_block_)
#define pcre2_general_context       PCRE2_SUFFIX(pcre2_general_context_)
#define pcre2_compile_context       PCRE2_SUFFIX(pcre2_compile_context_)
#define pcre2_match_context         PCRE2_SUFFIX(pcre2_match_context_)
#define pcre2_match_data            PCRE2_SUFFIX(pcre2_match_data_)


/* Functions: the complete list in alphabetical order */

#define pcre2_code_free                       PCRE2_SUFFIX(pcre2_code_free_)                            
#define pcre2_compile                         PCRE2_SUFFIX(pcre2_compile_)
#define pcre2_compile_context_copy            PCRE2_SUFFIX(pcre2_compile_context_copy_)                 
#define pcre2_compile_context_create          PCRE2_SUFFIX(pcre2_compile_context_create_)               
#define pcre2_compile_context_free            PCRE2_SUFFIX(pcre2_compile_context_free_)                 
#define pcre2_config                          PCRE2_SUFFIX(pcre2_config_)
#define pcre2_dfa_match                       PCRE2_SUFFIX(pcre2_dfa_match_)
#define pcre2_match                           PCRE2_SUFFIX(pcre2_match_)
#define pcre2_general_context_copy            PCRE2_SUFFIX(pcre2_general_context_copy_)                 
#define pcre2_general_context_create          PCRE2_SUFFIX(pcre2_general_context_create_)               
#define pcre2_general_context_free            PCRE2_SUFFIX(pcre2_general_context_free_)                 
#define pcre2_get_error_message               PCRE2_SUFFIX(pcre2_get_error_message_)
#define pcre2_get_leftchar                    PCRE2_SUFFIX(pcre2_get_leftchar_)
#define pcre2_get_mark                        PCRE2_SUFFIX(pcre2_get_mark_)
#define pcre2_get_match_frame_size            PCRE2_SUFFIX(pcre2_get_match_frame_size_)                 
#define pcre2_get_ovector_pointer             PCRE2_SUFFIX(pcre2_get_ovector_pointer_)
#define pcre2_get_ovector_count               PCRE2_SUFFIX(pcre2_get_ovector_count_)
#define pcre2_get_rightchar                   PCRE2_SUFFIX(pcre2_get_rightchar_)
#define pcre2_get_startchar                   PCRE2_SUFFIX(pcre2_get_startchar_)
#define pcre2_jit_compile                     PCRE2_SUFFIX(pcre2_jit_compile_)
#define pcre2_jit_match                       PCRE2_SUFFIX(pcre2_jit_match_)
#define pcre2_jit_free_unused_memory          PCRE2_SUFFIX(pcre2_jit_free_unused_memory_)
#define pcre2_jit_stack_alloc                 PCRE2_SUFFIX(pcre2_jit_stack_alloc_)
#define pcre2_jit_stack_assign                PCRE2_SUFFIX(pcre2_jit_stack_assign_)
#define pcre2_jit_stack_free                  PCRE2_SUFFIX(pcre2_jit_stack_free_)
#define pcre2_maketables                      PCRE2_SUFFIX(pcre2_maketables_)
#define pcre2_match_context_copy              PCRE2_SUFFIX(pcre2_match_context_copy_)                   
#define pcre2_match_context_create            PCRE2_SUFFIX(pcre2_match_context_create_)                 
#define pcre2_match_context_free              PCRE2_SUFFIX(pcre2_match_context_free_)               
#define pcre2_match_data_create               PCRE2_SUFFIX(pcre2_match_data_create_)                    
#define pcre2_match_data_create_from_pattern  PCRE2_SUFFIX(pcre2_match_data_create_from_pattern_)       
#define pcre2_match_data_free                 PCRE2_SUFFIX(pcre2_match_data_free_)                      
#define pcre2_pattern_info                    PCRE2_SUFFIX(pcre2_pattern_info_)
#define pcre2_pattern_to_host_byte_order      PCRE2_SUFFIX(pcre2_pattern_to_host_byte_order_)
#define pcre2_set_bsr_convention              PCRE2_SUFFIX(pcre2_set_bsr_convention_)
#define pcre2_set_callout                     PCRE2_SUFFIX(pcre2_set_callout_)
#define pcre2_set_character_tables            PCRE2_SUFFIX(pcre2_set_character_tables_)
#define pcre2_set_compile_recursion_guard     PCRE2_SUFFIX(pcre2_set_compile_recursion_guard_)
#define pcre2_set_match_limit                 PCRE2_SUFFIX(pcre2_set_match_limit_)
#define pcre2_set_newline_convention          PCRE2_SUFFIX(pcre2_set_newline_convention_)
#define pcre2_set_parens_nest_limit           PCRE2_SUFFIX(pcre2_set_parens_nest_limit_)
#define pcre2_set_recursion_limit             PCRE2_SUFFIX(pcre2_set_recursion_limit_)
#define pcre2_set_recursion_memory_management PCRE2_SUFFIX(pcre2_set_recursion_memory_management_)
#define pcre2_substring_copy_byname           PCRE2_SUFFIX(pcre2_substring_copy_byname_)                
#define pcre2_substring_copy_bynumber         PCRE2_SUFFIX(pcre2_substring_copy_bynumber_)              
#define pcre2_substring_free                  PCRE2_SUFFIX(pcre2_substring_free_)                       
#define pcre2_substring_get_byname            PCRE2_SUFFIX(pcre2_substring_get_byname_)                 
#define pcre2_substring_get_bynumber          PCRE2_SUFFIX(pcre2_substring_get_bynumber_)               
#define pcre2_substring_length_byname         PCRE2_SUFFIX(pcre2_substring_length_byname_)                     
#define pcre2_substring_length_bynumber       PCRE2_SUFFIX(pcre2_substring_length_bynumber_)                     
#define pcre2_substring_list_get              PCRE2_SUFFIX(pcre2_substring_list_get_)                   
#define pcre2_substring_list_free             PCRE2_SUFFIX(pcre2_substring_list_free_)                  
#define pcre2_substring_nametable_scan        PCRE2_SUFFIX(pcre2_substring_nametable_scan_)             
#define pcre2_substring_number_from_name      PCRE2_SUFFIX(pcre2_substring_number_from_name_)           
#define pcre2_version                         PCRE2_SUFFIX(pcre2_version_)


/* Now generate all three sets of width-specific structures and function
prototypes. */

#define PCRE2_TYPES_STRUCTURES_AND_FUNCTIONS \
PCRE2_TYPES_LIST \
PCRE2_STRUCTURE_LIST \
PCRE2_GENERAL_INFO_FUNCTIONS \
PCRE2_GENERAL_CONTEXT_FUNCTIONS \
PCRE2_COMPILE_CONTEXT_FUNCTIONS \
PCRE2_MATCH_CONTEXT_FUNCTIONS \
PCRE2_COMPILE_FUNCTIONS \
PCRE2_PATTERN_INFO_FUNCTIONS \
PCRE2_MATCH_FUNCTIONS \
PCRE2_SUBSTRING_FUNCTIONS \
PCRE2_JIT_FUNCTIONS \
PCRE2_OTHER_FUNCTIONS

#define PCRE2_LOCAL_WIDTH 8
PCRE2_TYPES_STRUCTURES_AND_FUNCTIONS
#undef PCRE2_LOCAL_WIDTH

#define PCRE2_LOCAL_WIDTH 16
PCRE2_TYPES_STRUCTURES_AND_FUNCTIONS
#undef PCRE2_LOCAL_WIDTH

#define PCRE2_LOCAL_WIDTH 32
PCRE2_TYPES_STRUCTURES_AND_FUNCTIONS
#undef PCRE2_LOCAL_WIDTH

/* Undefine the list macros; they are no longer needed. */

#undef PCRE2_TYPES_LIST
#undef PCRE2_STRUCTURE_LIST
#undef PCRE2_GENERAL_INFO_FUNCTIONS
#undef PCRE2_GENERAL_CONTEXT_FUNCTIONS
#undef PCRE2_COMPILE_CONTEXT_FUNCTIONS
#undef PCRE2_MATCH_CONTEXT_FUNCTIONS
#undef PCRE2_COMPILE_FUNCTIONS
#undef PCRE2_PATTERN_INFO_FUNCTIONS
#undef PCRE2_MATCH_FUNCTIONS
#undef PCRE2_SUBSTRING_FUNCTIONS
#undef PCRE2_JIT_FUNCTIONS
#undef PCRE2_OTHER_FUNCTIONS
#undef PCRE2_TYPES_STRUCTURES_AND_FUNCTIONS

/* Re-define PCRE2_SUFFIX to use the external width value, if defined.
Otherwise, undefine the other macros and make PCRE2_SUFFIX a no-op, to reduce
confusion. */

#undef PCRE2_SUFFIX
#ifdef PCRE2_CODE_UNIT_WIDTH
#if PCRE2_CODE_UNIT_WIDTH != 8 && \
    PCRE2_CODE_UNIT_WIDTH != 16 && \
    PCRE2_CODE_UNIT_WIDTH != 32
#error PCRE2_CODE_UNIT_WIDTH must be 8, 16, or 32
#endif
#define PCRE2_SUFFIX(a) PCRE2_GLUE(a, PCRE2_CODE_UNIT_WIDTH)
#else
#undef PCRE2_JOIN
#undef PCRE2_GLUE
#define PCRE2_SUFFIX(a) a
#endif


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* End of pcre2.h */
