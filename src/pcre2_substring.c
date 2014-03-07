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


/* FIXME: most of these are currently placeholder functions */


/*************************************************
*   Copy named captured string to given buffer   *
*************************************************/

/* This function copies a single captured substring into a given buffer,
identifying it by name. If the regex permits duplicate names, the first
substring that is set is chosen.

Arguments:
  match_data     points to the match data
  stringname     the name of the required substring
  buffer         where to put the substring
  size           the size of the buffer

Returns:         if successful:
                   the length of the copied string, not including the zero
                   that is put on the end; can be zero
                 if not successful, a negative error code:
                   PCRE2_ERROR_NOMEMORY: buffer too small
                   PCRE2_ERROR_NOSUBSTRING: no such captured substring
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_copy_named_substring(pcre2_match_data *match_data, PCRE2_SPTR stringname,
  PCRE2_UCHAR *buffer, size_t size)
{
match_data=match_data;stringname=stringname;buffer=buffer;size=size;
return PCRE2_ERROR_NOSUBSTRING;
}



/*************************************************
*  Copy numbered captured string to given buffer *
*************************************************/

/* This function copies a single captured substring into a given buffer,
identifying it by number.

Arguments:
  match_data     points to the match data
  stringnumber   the number of the required substring
  buffer         where to put the substring
  size           the size of the buffer

Returns:         if successful:
                   the length of the copied string, not including the zero
                   that is put on the end; can be zero
                 if not successful, a negative error code:
                   PCRE2_ERROR_NOMEMORY: buffer too small
                   PCRE2_ERROR_NOSUBSTRING: no such captured substring
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_copy_substring(pcre2_match_data *match_data, int stringnumber,
  PCRE2_UCHAR *buffer, size_t size)
{
match_data=match_data;stringnumber=stringnumber;buffer=buffer;size=size;
return PCRE2_ERROR_NOSUBSTRING;
}



/*************************************************
*       Free memory obtained by get_substring    *
*************************************************/

/* This function exists for the benefit of people calling PCRE from non-C
programs that can call its functions, but not free() itself.

Arguments:
  context     points to a PCRE2 context
  string      the result of a previous pcre2_get_substring()

Returns:      nothing
*/

PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_free_substring(pcre2_context *context, PCRE2_UCHAR *string)
{
context->free(string, context->user_data);
return;
}



/*************************************************
*   Free memory obtained by get_substring_list   *
*************************************************/

/* This function exists for the benefit of people calling PCRE from non-C
programs that can call its functions, but not free() itself.

Arguments:
  context     points to a PCRE2 context
  list        the result of a previous pcre2_get_substring_list()

Returns:      nothing
*/


PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_free_substring_list(pcre2_context *context, PCRE2_SPTR *list)
{
context->free(list, context->user_data);
return;
}



/*************************************************
*          Extract named captured string         *
*************************************************/

/* This function copies a single captured substring, identified by name, into
new memory. If the regex permits duplicate names, the first substring that is
set is chosen.

Arguments:
  context        points to a PCRE2 context
  match_data     pointer to match_data
  stringname     the name of the required substring
  stringptr      where to put the pointer

Returns:         if successful:
                   the length of the copied string, not including the zero
                   that is put on the end; can be zero
                 if not successful, a negative value:
                   PCRE2_ERROR_NOMEMORY: couldn't get memory
                   PCRE2_ERROR_NOSUBSTRING: no such captured substring
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_get_named_substring(pcre2_context *context, pcre2_match_data *match_data,
  PCRE2_SPTR stringname, PCRE2_UCHAR **stringptr)
{
context=context;match_data=match_data;stringname=stringname;stringptr=stringptr;
return PCRE2_ERROR_NOSUBSTRING;
}



/*************************************************
*         Get length of a named substring        *
*************************************************/

/* This function returns the length of a named captured substring. If the regex
permits duplicate names, the first substring that is set is chosen.

Arguments:
  match_data      pointer to match data
  stringname      the name of the required substring

Returns:          a non-negative length if successful
                  a negative error code otherwise
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_get_named_substring_length(pcre2_match_data *match_data,
  PCRE2_SPTR stringname)
{
match_data=match_data;stringname=stringname;
return PCRE2_ERROR_NOSUBSTRING;
}



/*************************************************
*     Find (multiple) entries for named string   *
*************************************************/

/* This is used by the local get_first_set() function, as well as being
generally available. It is used when duplicated names are permitted.

Arguments:
  code        the compiled regex
  stringname  the name whose entries required
  firstptr    where to put the pointer to the first entry
  lastptr     where to put the pointer to the last entry

Returns:      the length of each entry, or a negative number
                (PCRE2_ERROR_NOSUBSTRING) if not found
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_get_stringtable_entries(const pcre2_code *code, PCRE2_SPTR stringname,
  PCRE2_UCHAR **firstptr, PCRE2_UCHAR **lastptr)
{
code=code;stringname=stringname;firstptr=firstptr;lastptr=lastptr;
return PCRE2_ERROR_NOSUBSTRING;
}



/*************************************************
*      Extract captured string to new memory     *
*************************************************/

/* This function copies a single captured substring into a piece of new
memory.

Arguments:
  context        points to a PCRE2 context
  match_data     points to match data
  stringnumber   the number of the required substring
  stringptr      where to put a pointer to the substring

Returns:         if successful:
                   the length of the string, not including the zero that
                   is put on the end; can be zero
                 if not successful a negative error code:
                   PCRE2_ERROR_NOMEMORY: failed to get memory
                   PCRE2_ERROR_NOSUBSTRING: substring not present
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_get_substring(pcre2_context *context, pcre2_match_data *match_data,
  int stringnumber, PCRE2_UCHAR **stringptr)
{
context=context;match_data=match_data;stringnumber=stringnumber;
stringptr=stringptr;
return PCRE2_ERROR_NOSUBSTRING;
}



/*************************************************
*        Get length of a numbered substring      *
*************************************************/

/* This function returns the length of a captured substring.

Arguments:
  match_data      pointer to match data
  stringnumber    the number of the required substring

Returns:          a non-negative length if successful
                  a negative error code otherwise
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_get_substring_length(pcre2_match_data *match_data,
  int stringnumber)
{
match_data=match_data;stringnumber=stringnumber;
return PCRE2_ERROR_NOSUBSTRING;
}



/*************************************************
*    Extract all captured strings to new memory  *
*************************************************/

/* This function gets one chunk of memory and builds a list of pointers and all
the captured substrings in it. A NULL pointer is put on the end of the list.

Arguments:
  context        points to a PCRE2 context
  match_data     points to the match data
  listptr        set to point to the list of pointers

Returns:         if successful: 0
                 if not successful, a negative error code:
                   PCRE2_ERROR_NOMEMORY: failed to get memory
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_get_substring_list(pcre2_context *context, pcre2_match_data *match_data,
  PCRE2_UCHAR ***listptr)
{
context=context;match_data=match_data;listptr=listptr;
return PCRE2_ERROR_NOMEMORY;
}



/*************************************************
*           Find number for named string         *
*************************************************/

/* This function is used by the local get_first_set() function, as well
as being generally available. It assumes that names are unique.

Arguments:
  code        the compiled regex
  stringname  the name whose number is required

Returns:      the number of the named parentheses, or a negative number
                (PCRE2_ERROR_NOSUBSTRING) if not found
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
  pcre2_get_substring_number(const pcre2_code *code, PCRE2_SPTR stringname)
{
code=code;stringname=stringname;
return PCRE2_ERROR_NOSUBSTRING;
}


/* End of pcre2_substring.c */
