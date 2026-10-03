/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
          New API code Copyright (c) 2016-2020 University of Cambridge

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


#include "pcre2_internal.h"



/* These macros are the standard way of turning unquoted text into C strings.
They allow macros like PCRE2_MAJOR to be defined without quotes, which is
convenient for user programs that want to test their values. */

#define STRING(a)  #a
#define XSTRING(s) STRING(s)


/*************************************************
* Return info about what features are configured *
*************************************************/

/* If where is NULL, the required size is returned: bytes for numerical values,
or code units including the terminating zero for strings. Otherwise, the
requested information is written to where.

Arguments:
  what             what information is required
  where            where to put the information, or NULL to query its size

Returns:           sizeof(uint32_t) for a numerical value if where is NULL
                   0 if a numerical value is written
                   number of code units including the terminating zero
                     if a string is requested
                   PCRE2_ERROR_BADOPTION if "what" not recognized
                     or JIT target requested when JIT not enabled
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_config(uint32_t what, void *where)
{
  if (where == NULL) // Requests a length
  {
    switch (what)
    {
    default:
      return PCRE2_ERROR_BADOPTION;

    case PCRE2_CONFIG_BSR:
    case PCRE2_CONFIG_COMPILED_WIDTHS:
    case PCRE2_CONFIG_DEPTHLIMIT:
    case PCRE2_CONFIG_EFFECTIVE_LINKSIZE:
    case PCRE2_CONFIG_HEAPLIMIT:
    case PCRE2_CONFIG_JIT:
    case PCRE2_CONFIG_LINKSIZE:
    case PCRE2_CONFIG_MATCHLIMIT:
    case PCRE2_CONFIG_NEVER_BACKSLASH_C:
    case PCRE2_CONFIG_NEWLINE:
    case PCRE2_CONFIG_PARENSLIMIT:
    case PCRE2_CONFIG_STACKRECURSE: // Obsolete
    case PCRE2_CONFIG_TABLES_LENGTH:
    case PCRE2_CONFIG_UNICODE:
      return sizeof(uint32_t);

      /* These are handled below */

    case PCRE2_CONFIG_JITTARGET:
    case PCRE2_CONFIG_UNICODE_VERSION:
    case PCRE2_CONFIG_VERSION:
      break;
    }
  }

  switch (what)
  {
  default:
    return PCRE2_ERROR_BADOPTION;

  case PCRE2_CONFIG_BSR:
    *((uint32_t *)where) = BSR_DEFAULT;
    break;

  case PCRE2_CONFIG_COMPILED_WIDTHS:
    *((uint32_t *)where) = 0
#ifdef SUPPORT_PCRE2_8
                           + (1 << 0)
#endif
#ifdef SUPPORT_PCRE2_16
                           + (1 << 1)
#endif
#ifdef SUPPORT_PCRE2_32
                           + (1 << 2)
#endif
        ;
    break;

  case PCRE2_CONFIG_DEPTHLIMIT:
    *((uint32_t *)where) = MATCH_LIMIT_DEPTH;
    break;

  case PCRE2_CONFIG_EFFECTIVE_LINKSIZE:
    *((uint32_t *)where) = LINK_SIZE * sizeof(PCRE2_UCHAR);
    break;

  case PCRE2_CONFIG_HEAPLIMIT:
    *((uint32_t *)where) = HEAP_LIMIT;
    break;

  case PCRE2_CONFIG_JIT:
#ifdef SUPPORT_JIT
    *((uint32_t *)where) = 1;
#else
    *((uint32_t *)where) = 0;
#endif
    break;

  case PCRE2_CONFIG_JITTARGET:
#ifdef SUPPORT_JIT
    {
      const char *v = PRIV(jit_get_target)();
      return (int)(1 + ((where == NULL) ? strlen(v) : PRIV(strcpy_c8)((PCRE2_UCHAR *)where, v)));
    }
#else
    return PCRE2_ERROR_BADOPTION;
#endif

  case PCRE2_CONFIG_LINKSIZE:
    *((uint32_t *)where) = (uint32_t)CONFIGURED_LINK_SIZE;
    break;

  case PCRE2_CONFIG_MATCHLIMIT:
    *((uint32_t *)where) = MATCH_LIMIT;
    break;

  case PCRE2_CONFIG_NEWLINE:
    *((uint32_t *)where) = NEWLINE_DEFAULT;
    break;

  case PCRE2_CONFIG_NEVER_BACKSLASH_C:
#ifdef NEVER_BACKSLASH_C
    *((uint32_t *)where) = 1;
#else
    *((uint32_t *)where) = 0;
#endif
    break;

  case PCRE2_CONFIG_PARENSLIMIT:
    *((uint32_t *)where) = PARENS_NEST_LIMIT;
    break;

    /* This is now obsolete. The stack is no longer used via recursion for
    handling backtracking in pcre2_match(). */

  case PCRE2_CONFIG_STACKRECURSE:
    *((uint32_t *)where) = 0;
    break;

  case PCRE2_CONFIG_TABLES_LENGTH:
    *((uint32_t *)where) = TABLES_LENGTH;
    break;

  case PCRE2_CONFIG_UNICODE_VERSION:
    {
#if defined SUPPORT_UNICODE
      const char *v = PRIV(unicode_version);
#else
      const char *v = "Unicode not supported";
#endif
      return (int)(1 + ((where == NULL) ? strlen(v) : PRIV(strcpy_c8)((PCRE2_UCHAR *)where, v)));
    }

  case PCRE2_CONFIG_UNICODE:
#if defined SUPPORT_UNICODE
    *((uint32_t *)where) = 1;
#else
    *((uint32_t *)where) = 0;
#endif
    break;

  case PCRE2_CONFIG_VERSION:
    {
      /* Note that XSTRING(PCRE2_PRERELEASE) attempts to apply the STRING() macro to an empty sequence
      of tokens in release builds (when PCRE2_PRERELEASE is empty). This is valid as of C99, but
      historically some compilers did not support this. */

      const char *v = XSTRING(PCRE2_MAJOR) "." XSTRING(PCRE2_MINOR) XSTRING(PCRE2_PRERELEASE) " " XSTRING(PCRE2_DATE);
      return (int)(1 + ((where == NULL) ? strlen(v) : PRIV(strcpy_c8)((PCRE2_UCHAR *)where, v)));
    }
  }

  return 0;
}

/* End of pcre2_config.c */
