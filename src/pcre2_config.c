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
* Return info about what features are configured *
*************************************************/

/* This function has an extensible interface so that additional items can be
added compatibly.

Arguments:
  what             what information is required
  where            where to put the information

Returns:           0 if data returned, negative on error
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_config(int what, void *where)
{
switch (what)
  {
  case PCRE2_CONFIG_BSR:
#ifdef BSR_ANYCRLF
  *((int *)where) = 1;
#else
  *((int *)where) = 0;
#endif
  break;

  case PCRE2_CONFIG_JIT:
#ifdef SUPPORT_JIT
  *((int *)where) = 1;
#else
  *((int *)where) = 0;
#endif
  break;

  case PCRE2_CONFIG_JITTARGET:
#ifdef SUPPORT_JIT
  *((const char **)where) = PRIV(jit_get_target)();
#else
  *((const char **)where) = NULL;
#endif
  break;

  case PCRE2_CONFIG_LINK_SIZE:
  *((int *)where) = LINK_SIZE;
  break;

  case PCRE2_CONFIG_MATCH_LIMIT:
  *((unsigned long int *)where) = MATCH_LIMIT;
  break;

  case PCRE2_CONFIG_MATCH_LIMIT_RECURSION:
  *((unsigned long int *)where) = MATCH_LIMIT_RECURSION;
  break;

  case PCRE2_CONFIG_NEWLINE:
  *((int *)where) = NEWLINE;
  break;

  case PCRE2_CONFIG_PARENS_LIMIT:
  *((unsigned long int *)where) = PARENS_NEST_LIMIT;
  break;

  case PCRE2_CONFIG_STACKRECURSE:
#ifdef NO_RECURSE
  *((int *)where) = 0;
#else
  *((int *)where) = 1;
#endif
  break;

  case PCRE2_CONFIG_UTF:
#if defined SUPPORT_UTF
  *((int *)where) = 1;
#else
  *((int *)where) = 0;
#endif
  break;

  default: return PCRE2_ERROR_BADOPTION;
  }

return 0;
}

/* End of pcre2_config.c */
