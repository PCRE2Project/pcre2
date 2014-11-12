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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre2_internal.h"


/*************************************************
*        Return info about compiled pattern      *
*************************************************/

/*
Arguments:
  code          points to compiled code
  what          what information is required
  where         where to put the information; if NULL, return length

Returns:        0 when data returned
                > 0 when length requested
                < 0 on error or unset value
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_pattern_info(const pcre2_code *code, uint32_t what, void *where)
{
const pcre2_real_code *re = (pcre2_real_code *)code;

if (where == NULL)   /* Requests field length */
  {
  switch(what)
    {
    case PCRE2_INFO_ALLOPTIONS:
    case PCRE2_INFO_ARGOPTIONS:
    case PCRE2_INFO_BACKREFMAX:
    case PCRE2_INFO_BSR:
    case PCRE2_INFO_CAPTURECOUNT:
    case PCRE2_INFO_FIRSTCODETYPE:
    case PCRE2_INFO_FIRSTCODEUNIT:
    case PCRE2_INFO_HASCRORLF:
    case PCRE2_INFO_JCHANGED:
    case PCRE2_INFO_LASTCODETYPE:
    case PCRE2_INFO_LASTCODEUNIT:
    case PCRE2_INFO_MATCHEMPTY:
    case PCRE2_INFO_MATCHLIMIT:
    case PCRE2_INFO_MAXLOOKBEHIND:
    case PCRE2_INFO_MINLENGTH:
    case PCRE2_INFO_NAMEENTRYSIZE:
    case PCRE2_INFO_NAMECOUNT:
    case PCRE2_INFO_NEWLINE:
    case PCRE2_INFO_RECURSIONLIMIT:
    return sizeof(uint32_t);

    case PCRE2_INFO_FIRSTBITMAP:
    return sizeof(const uint8_t *);

    case PCRE2_INFO_JITSIZE:
    case PCRE2_INFO_SIZE:
    return sizeof(size_t);

    case PCRE2_INFO_NAMETABLE:
    return sizeof(PCRE2_SPTR);
    }
  }

if (re == NULL) return PCRE2_ERROR_NULL;

/* Check that the first field in the block is the magic number. If it is not,
return with PCRE2_ERROR_BADMAGIC. */

if (re->magic_number != MAGIC_NUMBER) return PCRE2_ERROR_BADMAGIC;

/* Check that this pattern was compiled in the correct bit mode */

if ((re->flags & (PCRE2_CODE_UNIT_WIDTH/8)) == 0) return PCRE2_ERROR_BADMODE;

switch(what)
  {
  case PCRE2_INFO_ALLOPTIONS:
  *((uint32_t *)where) = re->overall_options;
  break;

  case PCRE2_INFO_ARGOPTIONS:
  *((uint32_t *)where) = re->compile_options;
  break;

  case PCRE2_INFO_BACKREFMAX:
  *((uint32_t *)where) = re->top_backref;
  break;

  case PCRE2_INFO_BSR:
  *((uint32_t *)where) = re->bsr_convention;
  break;

  case PCRE2_INFO_CAPTURECOUNT:
  *((uint32_t *)where) = re->top_bracket;
  break;

  case PCRE2_INFO_FIRSTCODETYPE:
  *((uint32_t *)where) = ((re->flags & PCRE2_FIRSTSET) != 0)? 1 :
                         ((re->flags & PCRE2_STARTLINE) != 0)? 2 : 0;
  break;

  case PCRE2_INFO_FIRSTCODEUNIT:
  *((uint32_t *)where) = ((re->flags & PCRE2_FIRSTSET) != 0)?
    re->first_codeunit : 0;
  break;

  case PCRE2_INFO_FIRSTBITMAP:
  *((const uint8_t **)where) = ((re->flags & PCRE2_FIRSTMAPSET) != 0)?
    &(re->start_bitmap[0]) : NULL;
  break;

  case PCRE2_INFO_HASCRORLF:
  *((uint32_t *)where) = (re->flags & PCRE2_HASCRORLF) != 0;
  break;

  case PCRE2_INFO_JCHANGED:
  *((uint32_t *)where) = (re->flags & PCRE2_JCHANGED) != 0;
  break;

  case PCRE2_INFO_JITSIZE:
#ifdef SUPPORT_JIT
  *((size_t *)where) = (re->executable_jit != NULL)?
    PRIV(jit_get_size)(re->executable_jit) : 0;
#else
  *((size_t *)where) = 0;
#endif
  break;

  case PCRE2_INFO_LASTCODETYPE:
  *((uint32_t *)where) = ((re->flags & PCRE2_LASTSET) != 0)? 1 : 0;
  break;

  case PCRE2_INFO_LASTCODEUNIT:
  *((uint32_t *)where) = ((re->flags & PCRE2_LASTSET) != 0)?
    re->last_codeunit : 0;
  break;

  case PCRE2_INFO_MATCHEMPTY:
  *((uint32_t *)where) = (re->flags & PCRE2_MATCH_EMPTY) != 0;
  break;

  case PCRE2_INFO_MATCHLIMIT:
  *((uint32_t *)where) = re->limit_match;
  if (re->limit_match == UINT32_MAX) return PCRE2_ERROR_UNSET;
  break;

  case PCRE2_INFO_MAXLOOKBEHIND:
  *((uint32_t *)where) = re->max_lookbehind;
  break;

  case PCRE2_INFO_MINLENGTH:
  *((uint32_t *)where) = re->minlength;
  break;

  case PCRE2_INFO_NAMEENTRYSIZE:
  *((uint32_t *)where) = re->name_entry_size;
  break;

  case PCRE2_INFO_NAMECOUNT:
  *((uint32_t *)where) = re->name_count;
  break;

  case PCRE2_INFO_NAMETABLE:
  *((PCRE2_SPTR *)where) = (PCRE2_SPTR)((char *)re + sizeof(pcre2_real_code));
  break;

  case PCRE2_INFO_NEWLINE:
  *((uint32_t *)where) = re->newline_convention;
  break;

  case PCRE2_INFO_RECURSIONLIMIT:
  *((uint32_t *)where) = re->limit_recursion;
  if (re->limit_recursion == UINT32_MAX) return PCRE2_ERROR_UNSET;
  break;

  case PCRE2_INFO_SIZE:
  *((size_t *)where) = re->blocksize;
  break;

  default: return PCRE2_ERROR_BADOPTION;
  }

return 0;
}

/* End of pcre2_pattern_info.c */
