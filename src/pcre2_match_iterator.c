/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
          New API code Copyright (c) 2016-2024 University of Cambridge

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



/*************************************************
*             Create a match iterator            *
*************************************************/

PCRE2_EXP_DEFN pcre2_match_iterator * PCRE2_CALL_CONVENTION
pcre2_match_iterator_create(pcre2_general_context *gcontext)
{
pcre2_match_iterator *yield;
yield = PRIV(memctl_malloc)(
  sizeof(pcre2_match_iterator), (pcre2_memctl *)gcontext);
if (yield == NULL) return NULL;
yield->data = NULL;
return yield;
}



/*************************************************
*            Free a match iterator               *
*************************************************/

PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_match_iterator_free(pcre2_match_iterator *match_iterator)
{
if (match_iterator != NULL)
  {
  match_iterator->memctl.free(match_iterator, match_iterator->memctl.memory_data);
  }
}



/*************************************************
*           Reset a match iterator               *
*************************************************/

PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_match_iterator_reset(pcre2_match_iterator *match_iterator,
  pcre2_match_data *match_data)
{
match_iterator->data = match_data;
}



/*************************************************
*         Advance the match iterator             *
*************************************************/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_match_iterator_advance(pcre2_match_iterator *match_iterator,
  PCRE2_SIZE *pstart_offset, uint32_t *poptions)
{
pcre2_match_data *match_data = match_iterator->data;
int rc;


// rc = match
return FALSE;
}

/* End of pcre2_match_iterator.c */
