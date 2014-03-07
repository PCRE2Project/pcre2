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
*     Match a pattern using the DFA algorithm    *
*************************************************/

/* This function matches a compiled pattern to a subject string, using the
alternate matching algorithm that finds all matches at once.

Arguments:
  context       points to a PCRE2 context
  code          points to the compiled pattern
  subject       subject string
  length        length of subject string
  startoffset   where to start matching in the subject
  options       option bits
  match_data    points to a match data structure
  workspace     pointer to workspace
  wscount       size of workspace

Returns:        > 0 => number of match offset pairs placed in offsets
                = 0 => offsets overflowed; longest matches are present
                 -1 => failed to match
               < -1 => some kind of unexpected problem
*/                

/* FIXME: this is currently a placeholder function */

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_dfa_exec(pcre2_context *context, const pcre2_code *code,
  PCRE2_SPTR subject, int length, size_t startoffset, uint32_t options,
  pcre2_match_data *match_data, int *workspace, size_t wscount)
{

context = context; code = code; subject = subject; length = length;
startoffset = startoffset; options = options; match_data = match_data;
workspace = workspace; wscount = wscount;

return PCRE2_ERROR_NOMATCH;
}

/* End of pcre2_dfa_exec.c */
