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
*              Match and substitute              *
*************************************************/

/* This function applies a compiled re to a subject string and creates a new
string with substitutione. The first 7 arguments are the same as for
pcre2_match(). Either string length may be PCRE2_ZERO_TERMINATED.

Arguments:
  code            points to the compiled expression
  subject         points to the subject string
  length          length of subject string (may contain binary zeros)
  start_offset    where to start in the subject string
  options         option bits
  match_data      points to a match_data block, or is NULL
  context         points a PCRE2 context
  replacement     points to the replacement string
  rlength         length of replacement string
  buffer          where to put the substituted string
  blength         points to length of buffer; updated to length of string

Returns:        > 0 number of substitutions made
                < 0 an error code, including PCRE2_ERROR_NOMATCH if no match
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_substitute(const pcre2_code *code, PCRE2_SPTR subject, PCRE2_SIZE length,
  PCRE2_SIZE start_offset, uint32_t options, pcre2_match_data *match_data,
  pcre2_match_context *mcontext, PCRE2_SPTR replacement, PCRE2_SIZE rlength,
  PCRE2_UCHAR *buffer, PCRE2_SIZE *blength)
{
int rc = 0;
int subs = 0;
uint32_t ovector_count;
uint32_t goptions = 0;
BOOL match_data_created = FALSE;
BOOL global = FALSE;
PCRE2_SIZE buff_offset, lengthleft, endlength;
PCRE2_SIZE *ovector;

/* If no match data block is provided, create one. */

if (match_data == NULL)
  {
  pcre2_general_context *gcontext = (mcontext == NULL)?
    (pcre2_general_context *)code :
    (pcre2_general_context *)mcontext;
  match_data = pcre2_match_data_create_from_pattern(code, gcontext);
  if (match_data == NULL) return PCRE2_ERROR_NOMEMORY;
  match_data_created = TRUE;
  }
ovector = pcre2_get_ovector_pointer(match_data);
ovector_count = pcre2_get_ovector_count(match_data);

/* Notice the global option and remove it from the options that are passed to
pcre2_match(). */

if ((options & PCRE2_SUBSTITUTE_GLOBAL) != 0)
  {
  options &= ~PCRE2_SUBSTITUTE_GLOBAL;
  global = TRUE;
  }

/* Find lengths of zero-terminated strings. */

if (length == PCRE2_ZERO_TERMINATED) length = PRIV(strlen)(subject);
if (rlength == PCRE2_ZERO_TERMINATED) rlength = PRIV(strlen)(replacement);

/* Copy up to the start offset */

if (start_offset > *blength) goto NOROOM;
memcpy(buffer, subject, start_offset * (PCRE2_CODE_UNIT_WIDTH/8));
buff_offset = start_offset;
lengthleft = *blength - start_offset;

/* Loop for global substituting. */

do
  {
  PCRE2_SIZE i;

  rc = pcre2_match(code, subject, length, start_offset, options|goptions,
    match_data, mcontext);

  if (rc < 0)
    {
    if (goptions == 0 || rc != PCRE2_ERROR_NOMATCH || start_offset >= length)
      break;
    start_offset++;
    if ((code->overall_options & PCRE2_UTF) != 0)
      {
#if PCRE2_CODE_UNIT_WIDTH == 8
      while (start_offset < length && (subject[start_offset] & 0xc0) == 0x80)
        start_offset++;
#elif PCRE2_CODE_UNIT_WIDTH == 16
      while (start_offset < length &&
            (subject[start_offset] & 0xfc00) == 0xdc00)
        start_offset++;
#endif
      }
    goptions = 0;
    continue;
    }

  subs++;
  if (rc == 0) rc = ovector_count;
  endlength = ovector[0] - start_offset;
  if (endlength >= lengthleft) goto NOROOM;
  memcpy(buffer + buff_offset, subject + start_offset, 
    endlength*(PCRE2_CODE_UNIT_WIDTH/8));
  buff_offset += endlength;
  lengthleft -= endlength;

  for (i = 0; i < rlength; i++)
    {
    if (replacement[i] == CHAR_DOLLAR_SIGN && i != rlength - 1)
      {
      int group = -1;
      int n = 0;
      BOOL inparens = FALSE;
      PCRE2_SIZE j = i + 1;
      PCRE2_SIZE sublength; 
      PCRE2_UCHAR next = replacement[j];
      PCRE2_UCHAR name[33];

      if (next == CHAR_LEFT_CURLY_BRACKET)
        {
        if (j == rlength - 1) goto LITERAL;
        inparens = TRUE;
        next = replacement[++j];
        }

      if (next >= CHAR_0 && next <= CHAR_9)
        {
        group = next - CHAR_0;
        while (j < rlength - 1)
          {
          next = replacement[++j];
          if (next < CHAR_0 || next > CHAR_9) break;
          group = group * 10 + next - CHAR_0;
          }
        }
      else
        {
        const uint8_t *ctypes = code->tables + ctypes_offset;
        while (MAX_255(next) && (ctypes[next] & ctype_word) != 0)
          {
          name[n++] = next;
          if (n > 32) goto LITERAL;
          if (j == rlength - 1) break;
          next = replacement[++j];
          }
        name[n] = 0;
        }

      if (inparens)
        {
        if (j == rlength || next != CHAR_RIGHT_CURLY_BRACKET) goto LITERAL;
        }
      else j--;   /* Last code unit of name/number */

      /* Have found a syntactically correct group number or name. */

      i = j;   /* Where to continue from */

      if (group < 0)
        rc = pcre2_substring_copy_byname(match_data, name,
          buffer + buff_offset, &sublength);
      else
        rc = pcre2_substring_copy_bynumber(match_data, group,
          buffer + buff_offset, &sublength);

      if (rc < 0) goto EXIT;
      buff_offset += sublength;
      lengthleft -= sublength;
      }

   /* Handle a literal code unit */

   else
      {
      LITERAL:
      if (lengthleft-- < 1) goto NOROOM;
      buffer[buff_offset++] = replacement[i];
      }
    }

  /* The replacement has been copied to the output. Update the start offset to
  point to the rest of the subject string. If we matched an empty string,
  do the magic for global matches. */

  start_offset = ovector[1];
  goptions = (ovector[0] != ovector[1])? 0 :
    PCRE2_ANCHORED|PCRE2_NOTEMPTY_ATSTART;
  } while (global);  /* Repeat "do" loop */

/* No match is a "normal" end; copy the rest of the subject and return the
number of substitutions. */

if (rc == PCRE2_ERROR_NOMATCH)
  {
  rc = subs;
  endlength = length - start_offset;
  if (endlength + 1 >= lengthleft) goto NOROOM;
  memcpy(buffer + buff_offset, subject + start_offset,
    endlength*(PCRE2_CODE_UNIT_WIDTH/8));
  buff_offset += endlength;
  buffer[buff_offset] = 0;
  *blength = buff_offset;
  }

EXIT:
if (match_data_created) pcre2_match_data_free(match_data);
return rc;

NOROOM:
rc = PCRE2_ERROR_NOMEMORY;
goto EXIT;
}

/* End of pcre2_substitute.c */
