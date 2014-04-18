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


/* FIXME: these are all dummy functions */

/*************************************************
*  Create a match data block given ovector size  *
*************************************************/

PCRE2_EXP_DEFN pcre2_match_data * PCRE2_CALL_CONVENTION
pcre2_match_data_create(size_t ovecsize, pcre2_general_context *gcontext)
{
gcontext=gcontext;ovecsize=ovecsize;
return NULL;
}



/*************************************************
*  Create a match data block using pattern data  *
*************************************************/

PCRE2_EXP_DEFN pcre2_match_data * PCRE2_CALL_CONVENTION
pcre2_match_data_create_from_pattern(pcre2_code *code, 
  pcre2_general_context *gcontext)
{
code=code;gcontext=gcontext;
return NULL;
}



/*************************************************
*            Free a match data block             *
*************************************************/

PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_match_data_free(pcre2_match_data *match_data)
{
match_data=match_data;
return;
}



/*************************************************
*         Get left-most code unit in match       *
*************************************************/

PCRE2_EXP_DEFN size_t PCRE2_CALL_CONVENTION
pcre2_get_leftchar(pcre2_match_data *match_data)
{
match_data=match_data;
return 0;
}



/*************************************************
*         Get last mark in match                 *
*************************************************/

PCRE2_EXP_DEFN PCRE2_SPTR PCRE2_CALL_CONVENTION
pcre2_get_mark(pcre2_match_data *match_data)
{
match_data=match_data;
return NULL;
}



/*************************************************
*          Get pointer to ovector                *
*************************************************/

PCRE2_EXP_DEFN size_t * PCRE2_CALL_CONVENTION
pcre2_get_ovector_pointer(pcre2_match_data *match_data)
{
match_data=match_data;
return NULL;
}



/*************************************************
*          Get number of ovector slots           *
*************************************************/

PCRE2_EXP_DEFN size_t PCRE2_CALL_CONVENTION
pcre2_get_ovector_count(pcre2_match_data *match_data)
{
match_data=match_data;
return 0;
}



/*************************************************
*         Get right-most code unit in match      *
*************************************************/

PCRE2_EXP_DEFN size_t PCRE2_CALL_CONVENTION
pcre2_get_rightchar(pcre2_match_data *match_data)
{
match_data=match_data;
return 0;
}



/*************************************************
*         Get starting code unit in match        *
*************************************************/

PCRE2_EXP_DEFN size_t PCRE2_CALL_CONVENTION
pcre2_get_startchar(pcre2_match_data *match_data)
{
match_data=match_data;
return 0;
}

/* End of pcre2_match_data.c */
