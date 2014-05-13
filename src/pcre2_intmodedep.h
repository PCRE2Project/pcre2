/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
         New API code Copyright (c) 2014 University of Cambridge

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


/* This module contains mode-dependent macro and structure definitions. The 
file is #included by pcre2_internal.h if PCRE2_CODE_UNIT_WIDTH is defined.
These mode-dependent items are kept in a separate file so that they can also be
#included multiple times for different code unit widths by pcre2test. Start by 
undefining all the new macros defined herein so that they can be redefined for 
multiple inclusions. */

#undef CU2BYTES
#undef GET
#undef GET2
#undef IMM2_SIZE
#undef MAX_PATTERN_SIZE
#undef PUT
#undef PUT2
#undef PUTINC


/* ---------------------------MACROS ----------------------------- */

/* PCRE keeps offsets in its compiled code as at least 16-bit quantities
(always stored in big-endian order in 8-bit mode) by default. These are used,
for example, to link from the start of a subpattern to its alternatives and its
end. The use of 16 bits per offset limits the size of an 8-bit compiled regex
to around 64K, which is big enough for almost everybody. However, I received a
request for an even bigger limit. For this reason, and also to make the code
easier to maintain, the storing and loading of offsets from the compiled code
unit string is now handled by the macros that are defined here.

The macros are controlled by the value of LINK_SIZE. This defaults to 2, but 
values of 2 or 4 are also supported. */


/* ------------------- 8-bit support  ------------------ */

#if PCRE2_CODE_UNIT_WIDTH == 8

#if LINK_SIZE == 2
#define PUT(a,n,d)   \
  (a[n] = (d) >> 8), \
  (a[(n)+1] = (d) & 255)
#define GET(a,n) \
  (((a)[n] << 8) | (a)[(n)+1])
#define MAX_PATTERN_SIZE (1 << 16)

#elif LINK_SIZE == 3
#define PUT(a,n,d)       \
  (a[n] = (d) >> 16),    \
  (a[(n)+1] = (d) >> 8), \
  (a[(n)+2] = (d) & 255)
#define GET(a,n) \
  (((a)[n] << 16) | ((a)[(n)+1] << 8) | (a)[(n)+2])
#define MAX_PATTERN_SIZE (1 << 24)

#elif LINK_SIZE == 4
#define PUT(a,n,d)        \
  (a[n] = (d) >> 24),     \
  (a[(n)+1] = (d) >> 16), \
  (a[(n)+2] = (d) >> 8),  \
  (a[(n)+3] = (d) & 255)
#define GET(a,n) \
  (((a)[n] << 24) | ((a)[(n)+1] << 16) | ((a)[(n)+2] << 8) | (a)[(n)+3])
#define MAX_PATTERN_SIZE (1 << 30)   /* Keep it positive */

#else
#error LINK_SIZE must be either 2, 3, or 4
#endif


/* ------------------- 16-bit support  ------------------ */

#elif PCRE2_CODE_UNIT_WIDTH == 16

#if LINK_SIZE == 2
#undef LINK_SIZE
#define LINK_SIZE 1
#define PUT(a,n,d)   \
  (a[n] = (d))
#define GET(a,n) \
  (a[n])
#define MAX_PATTERN_SIZE (1 << 16)

#elif LINK_SIZE == 3 || LINK_SIZE == 4
#undef LINK_SIZE
#define LINK_SIZE 2
#define PUT(a,n,d)   \
  (a[n] = (d) >> 16), \
  (a[(n)+1] = (d) & 65535)
#define GET(a,n) \
  (((a)[n] << 16) | (a)[(n)+1])
#define MAX_PATTERN_SIZE (1 << 30)  /* Keep it positive */

#else
#error LINK_SIZE must be either 2, 3, or 4
#endif


/* ------------------- 32-bit support  ------------------ */

#elif PCRE2_CODE_UNIT_WIDTH == 32
#undef LINK_SIZE
#define LINK_SIZE 1
#define PUT(a,n,d)   \
  (a[n] = (d))
#define GET(a,n) \
  (a[n])
#define MAX_PATTERN_SIZE (1 << 30)  /* Keep it positive */

#else
#error Unsupported compiling mode
#endif 

/* -------------------------------------------------------*/


/* PCRE uses some other (at least) 16-bit quantities that do not change when
the size of offsets changes. There are used for repeat counts and for other
things such as capturing parenthesis numbers in back references. 

Define the number of code units required to hold a 16-bit count/offset, and
macros to load and store such a value. For reasons that I do not understand,
the expression in the 8-bit GET2 macro is treated by gcc as a signed
expression, even when a is declared as unsigned. It seems that any kind of
arithmetic results in a signed value. Hence the cast. */

#if PCRE2_CODE_UNIT_WIDTH == 8
#define IMM2_SIZE 2
#define GET2(a,n) (unsigned int)(((a)[n] << 8) | (a)[(n)+1])
#define PUT2(a,n,d) { a[n] = (d) >> 8; a[(n)+1] = (d) & 255; }

#else  /* Code units are 16 or 32 bits */
#define IMM2_SIZE 1
#define GET2(a,n) a[n]
#define PUT2(a,n,d) a[n] = d  
#endif


/* Mode-dependent macros that have the same definition in all modes. */

#define CU2BYTES(x)     (x)*((PCRE2_CODE_UNIT_WIDTH/8))
#define PUTINC(a,n,d)   PUT(a,n,d), a += LINK_SIZE


/* --------------------------- STRUCTURES ----------------------------- */

/* The real general context structure. At present it hold only data for custom 
memory control. */

typedef struct pcre2_real_general_context {
  pcre2_memctl    memctl;
} pcre2_real_general_context;

/* The real compile context structure */

typedef struct pcre2_real_compile_context {
  pcre2_memctl    memctl;
  int       (*stack_guard)(uint32_t);
  const unsigned char *tables;
  uint16_t  bsr_convention;
  uint16_t  newline_convention;
  uint32_t  parens_nest_limit;
} pcre2_real_compile_context;

/* The real match context structure. */

typedef struct pcre2_real_match_context {
  pcre2_memctl    memctl;
#ifdef NO_RECURSE
  void *    (*stack_malloc)(size_t, void *);
  void      (*stack_free)(void *, void *);
#endif   
  int       (*callout)(pcre2_callout_block *, void *);
  uint32_t  match_limit;
  uint32_t  recursion_limit;
} pcre2_real_match_context;

/* The real compiled code structure */

typedef struct pcre2_real_code {
  pcre2_memctl   memctl;
  void    *executable_jit;        /* Pointer to JIT code */  
  uint8_t  start_bitmap[32];      /* Bitmap for starting code unit < 256 */
  uint32_t magic_number;          /* Paranoid and endianness check */
  uint32_t size;                  /* Total (bytes) that was malloc-ed */ 
  uint32_t compile_options;       /* Options passed to pcre2_compile() */
  uint32_t pattern_options;       /* Options taken from the pattern */
  uint32_t flags;                 /* Various state flags */
  uint32_t limit_match;           /* Limit set in the pattern */
  uint32_t limit_recursion;       /* Limit set in the pattern */
  uint32_t first_codeunit;        /* Starting code unit */
  uint32_t last_codeunit;         /* This codeunit must be seen */
  uint16_t bsr_convention;        /* What \R matches */
  uint16_t newline_convention;    /* What is a newline? */  
  uint16_t max_lookbehind;        /* Longest lookbehind (characters) */
  uint16_t minlength;             /* Minimum length of match */ 
  uint16_t top_bracket;           /* Highest numbered group */ 
  uint16_t top_backref;           /* Highest numbered back reference */
  uint16_t name_entry_size;       /* Size (code units) of table entries */
  uint16_t name_count;            /* Number of name entries in the table */
} pcre2_real_code;

/* The reat match data structure. */

typedef struct pcre2_real_match_data {
  pcre2_memctl     memctl;
  const pcre2_real_code *code;    /* The pattern used for the match */
  PCRE2_SPTR       subject;       /* The subject that was matched */
  int              rc;            /* The return code from the match */
  int              utf_reason;    /* Reason code for bad UTF */  
  size_t           leftchar;      /* Offset to leftmost code unit */
  size_t           rightchar;     /* Offset to rightmost code unit */
  size_t           startchar;     /* Offset to starting code unit */  
  PCRE2_SPTR       mark;          /* Pointer to last mark */  
  uint16_t         oveccount;     /* Number of pairs */
  size_t           ovector[1];    /* The first field */ 
} pcre2_real_match_data;

/* End of pcre2_intmodedep.h */
