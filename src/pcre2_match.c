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
*           Match a Regular Expression           *
*************************************************/

/* This function applies a compiled re to a subject string and picks out
portions of the string if it matches. Two elements in the vector are set for
each substring: the offsets to the start and end of the substring.

Arguments:
  context         points a PCRE2 context
  code            points to the compiled expression
  subject         points to the subject string
  length          length of subject string (may contain binary zeros)
  start_offset    where to start in the subject string
  options         option bits
  match_data      points to a match_data block 

Returns:          > 0 => success; value is the number of ovector pairs filled
                  = 0 => success, but ovector is not big enough
                   -1 => failed to match (PCRE_ERROR_NOMATCH)
                 < -1 => some kind of unexpected problem
*/

/* FIXME: this is currently a placeholder function */

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_match(const pcre2_code *code, PCRE2_SPTR subject, int length, 
  size_t start_offset, uint32_t options, pcre2_match_data *match_data, 
  pcre2_match_context *mcontext)
{

/* Fudge for testing pcre2test */

if (subject[start_offset] == 'Y')
  {
  match_data->leftchar = 0;
  match_data->rightchar = 3;
  match_data->startchar = 0;
  match_data->mark = NULL;
  
  switch (match_data->oveccount)
    {
    case 0: return 0;
    
    case 1: match_data->ovector[0] = start_offset; 
            match_data->ovector[1] = start_offset + 4;
            return 0;
            
    default: match_data->ovector[0] = start_offset; 
             match_data->ovector[1] = start_offset + 4;
             match_data->ovector[2] = start_offset + 1;    
             match_data->ovector[3] = start_offset + 3;
             return 2;
    }  
  } 


mcontext=mcontext;code=code;subject=subject;length=length;
start_offset=start_offset; options=options; match_data=match_data;
return PCRE2_ERROR_NOMATCH;
}    

/* End of pcre2_match.c */
