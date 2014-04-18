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

/* This module contains the private structures needed by pcre2_internal.h. They 
are kept separate so that they can be #included multiple times for different 
code unit widths by pcre2test. */


/* The real general context structure */

typedef struct pcre2_real_general_context {
  void *    (*malloc)(size_t, void *);
  void      (*free)(void *, void *);
  void      *memory_data;
} pcre2_real_general_context;

/* The real compile context structure */

typedef struct pcre2_real_compile_context {
  void *    (*malloc)(size_t, void *);
  void      (*free)(void *, void *);
  void *    memory_data; 
  int       (*stack_guard)(uint32_t);
  const unsigned char *tables;
  uint16_t  bsr_convention;
  uint16_t  newline_convention;
  uint32_t  parens_nest_limit;
} pcre2_real_compile_context;

/* The real match context structure. */

typedef struct pcre2_real_match_context {
  void *    (*malloc)(size_t, void *);
  void      (*free)(void *, void *);
  void *    memory_data; 
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
  uint32_t magic_number;
  uint32_t size;                  /* Total that was malloc-ed */ 
  uint32_t compile_options;       /* Options passed to pcre2_compile() */
  uint32_t pattern_options;       /* Options taken from the pattern */
  uint32_t flags;                 /* Various state flags */
  uint32_t limit_match;           /* Limit set in the pattern */
  uint32_t limit_recursion;       /* Limit set in the pattern */
  uint32_t first_char;            /* Starting character */
  uint32_t req_char;              /* This character must be seen */
  uint16_t max_lookbehind;        /* Longest lookbehind (characters) */
  uint16_t top_bracket;           /* Highest numbered group */ 
  uint16_t top_backref;           /* Highest numbered back reference */
  uint16_t name_table_offset;     /* Offset to name table that follows */
  uint16_t name_entry_size;       /* Size of name items in the table */
  uint16_t name_count;            /* Number of name entries in the table */
} pcre2_real_code;

/* End of pcre2_intstructs.h */
