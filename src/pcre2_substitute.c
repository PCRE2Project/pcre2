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

#define PTR_STACK_SIZE 20


/*************************************************
*           Find end of substitute text          *
*************************************************/

/* In extended mode, we recognize ${name:+set text:unset text} and similar
constructions. This requires the identification of unescaped : and }
characters. This function scans for such. It must deal with nested ${
constructions. The pointer to the text is updated, either to the required end
character, or to where an error was detected.

Arguments:
  code      points to the compiled expression (for options)
  ptrptr    points to the pointer to the start of the text (updated)
  ptrend    end of the whole string
  last      TRUE if the last expected string (only } recognized)

Returns:    0 on success
            negative error code on failure
*/

static int
find_text_end(const pcre2_code *code, PCRE2_SPTR *ptrptr, PCRE2_SPTR ptrend,
  BOOL last)
{
int rc = 0;
uint32_t nestlevel = 0;
BOOL literal = FALSE;
PCRE2_SPTR ptr = *ptrptr;

for (; ptr < ptrend; ptr++)
  {
  if (literal)
    {
    if (ptr[0] == CHAR_BACKSLASH && ptr < ptrend - 1 && ptr[1] == CHAR_E)
      {
      literal = FALSE;
      ptr += 1;
      }
    }

  else if (*ptr == CHAR_RIGHT_CURLY_BRACKET)
    {
    if (nestlevel == 0) goto EXIT;
    nestlevel--;
    }

  else if (*ptr == CHAR_COLON && !last && nestlevel == 0) goto EXIT;

  else if (*ptr == CHAR_DOLLAR_SIGN)
    {
    if (ptr < ptrend - 1 && ptr[1] == CHAR_LEFT_CURLY_BRACKET)
      {
      nestlevel++;
      ptr += 1;
      }
    }

  else if (*ptr == CHAR_BACKSLASH)
    {
    int erc;
    int errorcode = 0;
    uint32_t ch;

    if (ptr < ptrend - 1) switch (ptr[1])
      {
      case CHAR_L:
      case CHAR_l:
      case CHAR_U:
      case CHAR_u:
      ptr += 1;
      continue;
      }

    erc = PRIV(check_escape)(&ptr, ptrend, &ch, &errorcode,
      code->overall_options, FALSE, NULL);
    if (errorcode != 0)
      {
      rc = errorcode;
      goto EXIT;
      }

    switch(erc)
      {
      case 0:      /* Data character */
      case ESC_E:  /* Isolated \E is ignored */
      break;

      case ESC_Q:
      literal = TRUE;
      break;

      default:
      rc = PCRE2_ERROR_BADREPESCAPE;
      goto EXIT;
      }
    }
  }

rc = PCRE2_ERROR_REPMISSINGBRACE;   /* Terminator not found */

EXIT:
*ptrptr = ptr;
return rc;
}



/*************************************************
*              Match and substitute              *
*************************************************/

/* This function applies a compiled re to a subject string and creates a new
string with substitutions. The first 7 arguments are the same as for
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

Returns:          >= 0 number of substitutions made
                  < 0 an error code
                  PCRE2_ERROR_BADREPLACEMENT means invalid use of $
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_substitute(const pcre2_code *code, PCRE2_SPTR subject, PCRE2_SIZE length,
  PCRE2_SIZE start_offset, uint32_t options, pcre2_match_data *match_data,
  pcre2_match_context *mcontext, PCRE2_SPTR replacement, PCRE2_SIZE rlength,
  PCRE2_UCHAR *buffer, PCRE2_SIZE *blength)
{
int rc;
int subs;
int forcecase = 0;
int forcecasereset = 0;
uint32_t ovector_count;
uint32_t goptions = 0;
BOOL match_data_created = FALSE;
BOOL global = FALSE;
BOOL extended = FALSE;
BOOL literal = FALSE;
BOOL uempty = FALSE;    /* Unset/unknown groups => empty string */
#ifdef SUPPORT_UNICODE
BOOL utf = (code->overall_options & PCRE2_UTF) != 0;
#endif
PCRE2_SPTR ptr;
PCRE2_SPTR repend;
PCRE2_SIZE buff_offset, buff_length, lengthleft, fraglength;
PCRE2_SIZE *ovector;

buff_length = *blength;
*blength = PCRE2_UNSET;

/* Partial matching is not valid. */

if ((options & (PCRE2_PARTIAL_HARD|PCRE2_PARTIAL_SOFT)) != 0)
  return PCRE2_ERROR_BADOPTION;

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

/* Find lengths of zero-terminated strings and the end of the replacement. */

if (length == PCRE2_ZERO_TERMINATED) length = PRIV(strlen)(subject);
if (rlength == PCRE2_ZERO_TERMINATED) rlength = PRIV(strlen)(replacement);
repend = replacement + rlength;

/* Check UTF replacement string if necessary. */

#ifdef SUPPORT_UNICODE
if (utf && (options & PCRE2_NO_UTF_CHECK) == 0)
  {
  rc = PRIV(valid_utf)(replacement, rlength, &(match_data->rightchar));
  if (rc != 0)
    {
    match_data->leftchar = 0;
    goto EXIT;
    }
  }
#endif  /* SUPPORT_UNICODE */

/* Notice the global and extended options and remove them from the options that
are passed to pcre2_match(). */

if ((options & PCRE2_SUBSTITUTE_GLOBAL) != 0)
  {
  options &= ~PCRE2_SUBSTITUTE_GLOBAL;
  global = TRUE;
  }

if ((options & PCRE2_SUBSTITUTE_EXTENDED) != 0)
  {
  options &= ~PCRE2_SUBSTITUTE_EXTENDED;
  extended = TRUE;
  }

if ((options & PCRE2_SUBSTITUTE_UNSET_EMPTY) != 0)
  {
  options &= ~PCRE2_SUBSTITUTE_UNSET_EMPTY;
  uempty = TRUE;
  }

/* Copy up to the start offset */

if (start_offset > buff_length) goto NOROOM;
memcpy(buffer, subject, start_offset * (PCRE2_CODE_UNIT_WIDTH/8));
buff_offset = start_offset;
lengthleft = buff_length - start_offset;

/* Loop for global substituting. */

subs = 0;
do
  {
  PCRE2_SPTR ptrstack[PTR_STACK_SIZE];
  uint32_t ptrstackptr = 0;

  rc = pcre2_match(code, subject, length, start_offset, options|goptions,
    match_data, mcontext);

#ifdef SUPPORT_UNICODE
  if (utf) options |= PCRE2_NO_UTF_CHECK;  /* Only need to check once */
#endif

  /* Any error other than no match returns the error code. No match when not
  doing the special after-empty-match global rematch, or when at the end of the
  subject, breaks the global loop. Otherwise, advance the starting point by one
  character, copying it to the output, and try again. */

  if (rc < 0)
    {
    PCRE2_SIZE save_start;

    if (rc != PCRE2_ERROR_NOMATCH) goto EXIT;
    if (goptions == 0 || start_offset >= length) break;

    /* Advance by one code point. Then, if CRLF is a valid newline sequence and
    we have advanced into the middle of it, advance one more code point. In
    other words, do not start in the middle of CRLF, even if CR and LF on their
    own are valid newlines. */

    save_start = start_offset++;
    if (subject[start_offset-1] == CHAR_CR &&
        code->newline_convention != PCRE2_NEWLINE_CR &&
        code->newline_convention != PCRE2_NEWLINE_LF &&
        start_offset < length &&
        subject[start_offset] == CHAR_LF)
      start_offset++;

    /* Otherwise, in UTF mode, advance past any secondary code points. */

    else if ((code->overall_options & PCRE2_UTF) != 0)
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

    fraglength = start_offset - save_start;
    if (lengthleft < fraglength) goto NOROOM;
    memcpy(buffer + buff_offset, subject + save_start,
      fraglength*(PCRE2_CODE_UNIT_WIDTH/8));
    buff_offset += fraglength;
    lengthleft -= fraglength;

    goptions = 0;
    continue;
    }

  /* Handle a successful match. Matches that use \K to end before they start
  are not supported. */

  if (ovector[1] < ovector[0])
    {
    rc = PCRE2_ERROR_BADSUBSPATTERN;
    goto EXIT;
    }

  /* Paranoid check for integer overflow; surely no real call to this function
  would ever hit this! */

  if (subs == INT_MAX)
    {
    rc = PCRE2_ERROR_TOOMANYREPLACE;
    goto EXIT;
    }

  /* Count substitutions and proceed */

  subs++;
  if (rc == 0) rc = ovector_count;
  fraglength = ovector[0] - start_offset;
  if (fraglength >= lengthleft) goto NOROOM;
  memcpy(buffer + buff_offset, subject + start_offset,
    fraglength*(PCRE2_CODE_UNIT_WIDTH/8));
  buff_offset += fraglength;
  lengthleft -= fraglength;

  /* Process the replacement string. Literal mode is set by \Q, but only in
  extended mode when backslashes are being interpreted. In extended mode we
  must handle nested substrings that are to be reprocessed. */

  ptr = replacement;
  for (;;)
    {
    uint32_t ch;

    /* If at the end of a nested substring, pop the stack. */

    if (ptr >= repend)
      {
      if (ptrstackptr <= 0) break;
      repend = ptrstack[--ptrstackptr];
      ptr = ptrstack[--ptrstackptr];
      continue;
      }

    /* Handle the next character */

    if (literal)
      {
      if (ptr[0] == CHAR_BACKSLASH && ptr < repend - 1 && ptr[1] == CHAR_E)
        {
        literal = FALSE;
        ptr += 2;
        continue;
        }
      goto LOADLITERAL;
      }

    /* Not in literal mode. */

    if (*ptr == CHAR_DOLLAR_SIGN)
      {
      int group, n;
      uint32_t special = 0;
      BOOL inparens;
      BOOL star;
      PCRE2_SIZE sublength;
      PCRE2_SPTR text1_start = NULL;
      PCRE2_SPTR text1_end = NULL;
      PCRE2_SPTR text2_start = NULL;
      PCRE2_SPTR text2_end = NULL;
      PCRE2_UCHAR next;
      PCRE2_UCHAR name[33];

      if (++ptr >= repend) goto BAD;
      if ((next = *ptr) == CHAR_DOLLAR_SIGN) goto LOADLITERAL;

      group = -1;
      n = 0;
      inparens = FALSE;
      star = FALSE;

      if (next == CHAR_LEFT_CURLY_BRACKET)
        {
        if (++ptr >= repend) goto BAD;
        next = *ptr;
        inparens = TRUE;
        }

      if (next == CHAR_ASTERISK)
        {
        if (++ptr >= repend) goto BAD;
        next = *ptr;
        star = TRUE;
        }

      if (!star && next >= CHAR_0 && next <= CHAR_9)
        {
        group = next - CHAR_0;
        while (++ptr < repend)
          {
          next = *ptr;
          if (next < CHAR_0 || next > CHAR_9) break;
          group = group * 10 + next - CHAR_0;

          /* A check for a number greater than the hightest captured group
          is sufficient here; no need for a separate overflow check. */

          if (group > code->top_bracket)
            {
            rc = PCRE2_ERROR_NOSUBSTRING;
            goto PTREXIT;
            }
          }
        }
      else
        {
        const uint8_t *ctypes = code->tables + ctypes_offset;
        while (MAX_255(next) && (ctypes[next] & ctype_word) != 0)
          {
          name[n++] = next;
          if (n > 32) goto BAD;
          if (++ptr >= repend) break;
          next = *ptr;
          }
        if (n == 0) goto BAD;
        name[n] = 0;
        }

      /* In extended mode we recognize ${name:+set text:unset text} and
      ${name:-default text}. */

      if (inparens)
        {
        if (extended && !star && ptr < repend - 2 && next == CHAR_COLON)
          {
          special = *(++ptr);
          if (special != CHAR_PLUS && special != CHAR_MINUS)
            {
            rc = PCRE2_ERROR_BADSUBSTITUTION;
            goto PTREXIT;
            }

          text1_start = ++ptr;
          rc = find_text_end(code, &ptr, repend, special == CHAR_MINUS);
          if (rc != 0) goto PTREXIT;
          text1_end = ptr;

          if (special == CHAR_PLUS && *ptr == CHAR_COLON)
            {
            text2_start = ++ptr;
            rc = find_text_end(code, &ptr, repend, TRUE);
            if (rc != 0) goto PTREXIT;
            text2_end = ptr;
            }
          }

        else
          {
          if (ptr >= repend || *ptr != CHAR_RIGHT_CURLY_BRACKET)
            {
            rc = PCRE2_ERROR_REPMISSINGBRACE;
            goto PTREXIT;
            }
          }

        ptr++;
        }

      /* Have found a syntactically correct group number or name, or
      *name. Only *MARK is currently recognized. */

      if (star)
        {
        if (PRIV(strcmp_c8)(name, STRING_MARK) == 0)
          {
          PCRE2_SPTR mark = pcre2_get_mark(match_data);
          if (mark != NULL)
            {
            while (*mark != 0)
              {
              if (lengthleft-- < 1) goto NOROOM;
              buffer[buff_offset++] = *mark++;
              }
            }
          }
        else goto BAD;
        }

      /* Substitute the contents of a group. We don't use substring_copy
      functions any more, in order to support case forcing. */

      else
        {
        PCRE2_SPTR subptr, subptrend;

        /* Find a number for a named group. In case there are duplicate names,
        search for the first one that is set. */

        if (group < 0)
          {
          PCRE2_SPTR first, last, entry;
          rc = pcre2_substring_nametable_scan(code, name, &first, &last);
          if (rc < 0) goto PTREXIT;
          for (entry = first; entry <= last; entry += rc)
            {
            uint32_t ng = GET2(entry, 0);
            if (ng < ovector_count)
              {
              if (group < 0) group = ng;          /* First in ovector */
              if (ovector[ng*2] != PCRE2_UNSET)
                {
                group = ng;                       /* First that is set */
                break;
                }
              }
            }

          /* If group is still negative, it means we did not find a group that
          is in the ovector. Just set the first group. */

          if (group < 0) group = GET2(first, 0);
          }

        /* We now have a group that is identified by number. Find the length of
        the captured string. If a group in a non-special substitution is unset
        when PCRE2_SUBSTITUTE_UNSET_EMPTY is set, substitute nothing. */

        rc = pcre2_substring_length_bynumber(match_data, group, &sublength);
        if (rc < 0)
          {
          if (rc != PCRE2_ERROR_UNSET) goto PTREXIT;  /* Non-unset errors */
          if (special == 0)                           /* Plain substitution */
            {
            if (uempty) continue;                     /* Treat as empty */
            goto PTREXIT;                             /* Else error */
            }
          }

        /* If special is '+' we have a 'set' and possibly an 'unset' text,
        both of which are reprocessed when used. If special is '-' we have a
        default text for when the group is unset; it must be reprocessed. */

        if (special != 0)
          {
          if (special == CHAR_MINUS)
            {
            if (rc == 0) goto LITERAL_SUBSTITUTE;
            text2_start = text1_start;
            text2_end = text1_end;
            }

          if (ptrstackptr >= PTR_STACK_SIZE) goto BAD;
          ptrstack[ptrstackptr++] = ptr;
          ptrstack[ptrstackptr++] = repend;

          if (rc == 0)
            {
            ptr = text1_start;
            repend = text1_end;
            }
          else
            {
            ptr = text2_start;
            repend = text2_end;
            }
          continue;
          }

        /* Otherwise we have a literal substitution of a group's contents. */

        LITERAL_SUBSTITUTE:
        subptr = subject + ovector[group*2];
        subptrend = subject + ovector[group*2 + 1];

        /* Substitute a literal string, possibly forcing alphabetic case. */

        while (subptr < subptrend)
          {
          GETCHARINCTEST(ch, subptr);
          if (forcecase != 0)
            {
#ifdef SUPPORT_UNICODE
            if (utf)
              {
              uint32_t type = UCD_CHARTYPE(ch);
              if (PRIV(ucp_gentype)[type] == ucp_L &&
                  type != ((forcecase > 0)? ucp_Lu : ucp_Ll))
                ch = UCD_OTHERCASE(ch);
              }
            else
#endif
              {
              if (((code->tables + cbits_offset +
                  ((forcecase > 0)? cbit_upper:cbit_lower)
                  )[ch/8] & (1 << (ch%8))) == 0)
                ch = (code->tables + fcc_offset)[ch];
              }
            forcecase = forcecasereset;
            }

#ifdef SUPPORT_UNICODE
          if (utf)
            {
            unsigned int chlen;
#if PCRE2_CODE_UNIT_WIDTH == 8
            if (lengthleft < 6) goto NOROOM;
#elif PCRE2_CODE_UNIT_WIDTH == 16
            if (lengthleft < 2) goto NOROOM;
#else
            if (lengthleft < 1) goto NOROOM;
#endif
            chlen = PRIV(ord2utf)(ch, buffer + buff_offset);
            buff_offset += chlen;
            lengthleft -= chlen;
            }
          else
#endif
            {
            if (lengthleft-- < 1) goto NOROOM;
            buffer[buff_offset++] = ch;
            }
          }
        }
      }

    /* Handle an escape sequence in extended mode. We can use check_escape()
    to process \Q, \E, \c, \o, \x and \ followed by non-alphanumerics, but
    the case-forcing escapes are not supported in pcre2_compile() so must be
    recognized here. */

    else if (extended && *ptr == CHAR_BACKSLASH)
      {
      int errorcode = 0;

      if (ptr < repend - 1) switch (ptr[1])
        {
        case CHAR_L:
        forcecase = forcecasereset = -1;
        ptr += 2;
        continue;

        case CHAR_l:
        forcecase = -1;
        forcecasereset = 0;
        ptr += 2;
        continue;

        case CHAR_U:
        forcecase = forcecasereset = 1;
        ptr += 2;
        continue;

        case CHAR_u:
        forcecase = 1;
        forcecasereset = 0;
        ptr += 2;
        continue;

        default:
        break;
        }

      rc = PRIV(check_escape)(&ptr, repend, &ch, &errorcode,
        code->overall_options, FALSE, NULL);
      if (errorcode != 0) goto BADESCAPE;
      ptr++;

      switch(rc)
        {
        case ESC_E:
        forcecase = forcecasereset = 0;
        continue;

        case ESC_Q:
        literal = TRUE;
        continue;

        case 0:      /* Data character */
        goto LITERAL;

        default:
        goto BADESCAPE;
        }
      }

    /* Handle a literal code unit */

    else
      {
      LOADLITERAL:
      GETCHARINCTEST(ch, ptr);    /* Get character value, increment pointer */

      LITERAL:
      if (forcecase != 0)
        {
#ifdef SUPPORT_UNICODE
        if (utf)
          {
          uint32_t type = UCD_CHARTYPE(ch);
          if (PRIV(ucp_gentype)[type] == ucp_L &&
              type != ((forcecase > 0)? ucp_Lu : ucp_Ll))
            ch = UCD_OTHERCASE(ch);
          }
        else
#endif
          {
          if (((code->tables + cbits_offset +
              ((forcecase > 0)? cbit_upper:cbit_lower)
              )[ch/8] & (1 << (ch%8))) == 0)
            ch = (code->tables + fcc_offset)[ch];
          }

        forcecase = forcecasereset;
        }

#ifdef SUPPORT_UNICODE
      if (utf)
        {
        unsigned int chlen;
#if PCRE2_CODE_UNIT_WIDTH == 8
        if (lengthleft < 6) goto NOROOM;
#elif PCRE2_CODE_UNIT_WIDTH == 16
        if (lengthleft < 2) goto NOROOM;
#else
        if (lengthleft < 1) goto NOROOM;
#endif
        chlen = PRIV(ord2utf)(ch, buffer + buff_offset);
        buff_offset += chlen;
        lengthleft -= chlen;
        }
      else
#endif
        {
        if (lengthleft-- < 1) goto NOROOM;
        buffer[buff_offset++] = ch;
        }
      }
    }

  /* The replacement has been copied to the output. Update the start offset to
  point to the rest of the subject string. If we matched an empty string,
  do the magic for global matches. */

  start_offset = ovector[1];
  goptions = (ovector[0] != ovector[1])? 0 :
    PCRE2_ANCHORED|PCRE2_NOTEMPTY_ATSTART;
  } while (global);  /* Repeat "do" loop */

/* Copy the rest of the subject and return the number of substitutions. */

rc = subs;
fraglength = length - start_offset;
if (fraglength + 1 > lengthleft) goto NOROOM;
memcpy(buffer + buff_offset, subject + start_offset,
  fraglength*(PCRE2_CODE_UNIT_WIDTH/8));
buff_offset += fraglength;
buffer[buff_offset] = 0;
*blength = buff_offset;

EXIT:
if (match_data_created) pcre2_match_data_free(match_data);
  else match_data->rc = rc;
return rc;

NOROOM:
rc = PCRE2_ERROR_NOMEMORY;
goto EXIT;

BAD:
rc = PCRE2_ERROR_BADREPLACEMENT;
goto PTREXIT;

BADESCAPE:
rc = PCRE2_ERROR_BADREPESCAPE;

PTREXIT:
*blength = (PCRE2_SIZE)(ptr - replacement);
goto EXIT;
}

/* End of pcre2_substitute.c */
