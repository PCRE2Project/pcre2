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
pcre2_substring_copy_byname(pcre2_match_data *match_data, PCRE2_SPTR stringname,
  PCRE2_UCHAR *buffer, size_t size)
{
PCRE2_SPTR first;
PCRE2_SPTR last;
PCRE2_SPTR entry;
int entrysize = pcre2_substring_nametable_scan(match_data->code, stringname,
  &first, &last);
if (entrysize <= 0) return entrysize;
for (entry = first; entry <= last; entry += entrysize)
  {
  uint16_t n = GET2(entry, 0);
  if (n < match_data->oveccount && match_data->ovector[n*2] != PCRE2_UNSET)
    return pcre2_substring_copy_bynumber(match_data, n, buffer, size); 
  }
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
pcre2_substring_copy_bynumber(pcre2_match_data *match_data, int stringnumber,
  PCRE2_UCHAR *buffer, size_t size)
{
size_t left, right;
size_t p = 0;
PCRE2_SPTR subject = match_data->subject;
if (stringnumber >= match_data->oveccount ||
    (left = match_data->ovector[stringnumber*2]) == PCRE2_UNSET)
  return PCRE2_ERROR_NOSUBSTRING;
right = match_data->ovector[stringnumber*2+1];
if (right - left + 1 > size) return PCRE2_ERROR_NOMEMORY; 
while (left < right) buffer[p++] = subject[left++];
buffer[p] = 0;
return p;
}



/*************************************************
*          Extract named captured string         *
*************************************************/

/* This function copies a single captured substring, identified by name, into
new memory. If the regex permits duplicate names, the first substring that is
set is chosen.

Arguments:
  match_data     pointer to match_data
  stringname     the name of the required substring
  stringptr      where to put the pointer to the new memory

Returns:         if successful:
                   the length of the copied string, not including the zero
                   that is put on the end; can be zero
                 if not successful, a negative value:
                   PCRE2_ERROR_NOMEMORY: couldn't get memory
                   PCRE2_ERROR_NOSUBSTRING: no such captured substring
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_substring_get_byname(pcre2_match_data *match_data,
  PCRE2_SPTR stringname, PCRE2_UCHAR **stringptr)
{
PCRE2_SPTR first;
PCRE2_SPTR last;
PCRE2_SPTR entry;
int entrysize = pcre2_substring_nametable_scan(match_data->code, stringname,
  &first, &last);
if (entrysize <= 0) return entrysize;
for (entry = first; entry <= last; entry += entrysize)
  {
  uint16_t n = GET2(entry, 0);
  if (n < match_data->oveccount && match_data->ovector[n*2] != PCRE2_UNSET)
    return pcre2_substring_get_bynumber(match_data, n, stringptr); 
  }
return PCRE2_ERROR_NOSUBSTRING;
}



/*************************************************
*      Extract captured string to new memory     *
*************************************************/

/* This function copies a single captured substring into a piece of new
memory.

Arguments:
  match_data     points to match data
  stringnumber   the number of the required substring
  stringptr      where to put a pointer to the new memory

Returns:         if successful:
                   the length of the string, not including the zero that
                   is put on the end; can be zero
                 if not successful a negative error code:
                   PCRE2_ERROR_NOMEMORY: failed to get memory
                   PCRE2_ERROR_NOSUBSTRING: substring not present
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_substring_get_bynumber(pcre2_match_data *match_data, int stringnumber, 
  PCRE2_UCHAR **stringptr)
{
size_t left, right;
size_t p = 0;
void *block;
PCRE2_UCHAR *yield;

PCRE2_SPTR subject = match_data->subject;
if (stringnumber >= match_data->oveccount ||
    (left = match_data->ovector[stringnumber*2]) == PCRE2_UNSET)
  return PCRE2_ERROR_NOSUBSTRING;
right = match_data->ovector[stringnumber*2+1];

block = PRIV(memctl_malloc)(sizeof(pcre2_memctl) + 
  (right-left+1)*PCRE2_CODE_UNIT_WIDTH, 0, &(match_data->memctl)); 
if (block == NULL) return PCRE2_ERROR_NOMEMORY;

yield = (PCRE2_UCHAR *)((char *)block + sizeof(pcre2_memctl));
while (left < right) yield[p++] = subject[left++];
yield[p] = 0;
*stringptr = yield;
return p;
}



/*************************************************
*       Free memory obtained by get_substring    *
*************************************************/

/* 
Argument:     the result of a previous pcre2_substring_get_byxxx()
Returns:      nothing
*/

PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_substring_free(PCRE2_UCHAR *string)
{
pcre2_memctl *memctl = (pcre2_memctl *)((char *)string - sizeof(pcre2_memctl));
memctl->free(memctl, memctl->memory_data);
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
pcre2_substring_length_byname(pcre2_match_data *match_data,
  PCRE2_SPTR stringname)
{
PCRE2_SPTR first;
PCRE2_SPTR last;
PCRE2_SPTR entry;
int entrysize = pcre2_substring_nametable_scan(match_data->code, stringname,
  &first, &last);
if (entrysize <= 0) return entrysize;
for (entry = first; entry <= last; entry += entrysize)
  {
  uint16_t n = GET2(entry, 0);
  if (n < match_data->oveccount && match_data->ovector[n*2] != PCRE2_UNSET)
    return pcre2_substring_length_bynumber(match_data, n); 
  }
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
pcre2_substring_length_bynumber(pcre2_match_data *match_data,
  int stringnumber)
{
if (stringnumber >= match_data->oveccount ||
    match_data->ovector[stringnumber*2] == PCRE2_UNSET)
  return PCRE2_ERROR_NOSUBSTRING;
return match_data->ovector[stringnumber*2 + 1] -
       match_data->ovector[stringnumber*2];
}



/*************************************************
*    Extract all captured strings to new memory  *
*************************************************/

/* This function gets one chunk of memory and builds a list of pointers and all
the captured substrings in it. A NULL pointer is put on the end of the list.
The substrings are zero-terminated, but also, if the final argument is 
non-NULL, a list of lengths is also returned. This allows binary data to be 
handled.

Arguments:
  match_data     points to the match data
  listptr        set to point to the list of pointers
  lengthsptr     set to point to the list of lengths (may be NULL) 

Returns:         if successful: 0
                 if not successful, a negative error code:
                   PCRE2_ERROR_NOMEMORY: failed to get memory,
                   or a match failure code   
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_substring_list_get(pcre2_match_data *match_data, PCRE2_UCHAR ***listptr, 
  size_t **lengthsptr)
{
int i, count, count2;
size_t size;
size_t *lensp;
pcre2_memctl *memp;
PCRE2_UCHAR **listp;
PCRE2_UCHAR *sp;
PCRE2_OFFSET *ovector;

if ((count = match_data->rc) < 0) return count;

count2 = 2*count;
ovector = match_data->ovector;
size = sizeof(pcre2_memctl) + sizeof(PCRE2_UCHAR *);     /* For final NULL */
if (lengthsptr != NULL) size += sizeof(size_t) * count;  /* For lengths */

for (i = 0; i < count2; i += 2)
   size += sizeof(PCRE2_UCHAR *) + CU2BYTES(ovector[i+1] - ovector[i] + 1);
memp = PRIV(memctl_malloc)(size, 0, &(match_data->memctl)); 
if (memp == NULL) return PCRE2_ERROR_NOMEMORY;

*listptr = listp = (PCRE2_UCHAR **)((char *)memp + sizeof(pcre2_memctl));
lensp = (size_t *)((char *)listp + sizeof(PCRE2_UCHAR *) * (count + 1));
                                                          
if (lengthsptr == NULL)
  {
  sp = (PCRE2_UCHAR *)lensp; 
  lensp = NULL;
  }
else
  {   
  *lengthsptr = lensp; 
  sp = (PCRE2_UCHAR *)((char *)lensp + sizeof(size_t) * count); 
  } 

for (i = 0; i < count2; i += 2)
  {
  size = ovector[i+1] - ovector[i];
  memcpy(sp, match_data->subject + ovector[i], CU2BYTES(size));
  *listp++ = sp;
  if (lensp != NULL) *lensp++ = size;
  sp += size;
  *sp++ = 0;
  }

*listp = NULL;
return 0;
}



/*************************************************
*   Free memory obtained by substring_list_get   *
*************************************************/

/*
Argument:     the result of a previous pcre2_substring_list_get()
Returns:      nothing
*/

PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_substring_list_free(PCRE2_SPTR *list)
{
pcre2_memctl *memctl = (pcre2_memctl *)((char *)list - sizeof(pcre2_memctl));
memctl->free(memctl, memctl->memory_data);
}



/*************************************************
*     Find (multiple) entries for named string   *
*************************************************/

/* This function scans the nametable for a given name, using binary chop. It 
returns either two pointers to the entries in the table, or, if no pointers are 
given, the number of a group with the given name. If duplicate names are 
permitted, this may not be unique.

Arguments:
  code        the compiled regex
  stringname  the name whose entries required
  firstptr    where to put the pointer to the first entry
  lastptr     where to put the pointer to the last entry

Returns:      if firstptr and lastptr are NULL, a group number;
              otherwise, the length of each entry, or a negative number
                (PCRE2_ERROR_NOSUBSTRING) if not found
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_substring_nametable_scan(const pcre2_code *code, PCRE2_SPTR stringname,
  PCRE2_SPTR *firstptr, PCRE2_SPTR *lastptr)
{
uint16_t bot = 0;
uint16_t top = code->name_count;
uint16_t entrysize = code->name_entry_size;
PCRE2_SPTR nametable = (PCRE2_SPTR)((char *)code + sizeof(pcre2_real_code));

while (top > bot)
  {
  uint16_t mid = (top + bot) / 2;
  PCRE2_SPTR entry = nametable + entrysize*mid;
  int c = PRIV(strcmp)(stringname, entry + IMM2_SIZE);
  if (c == 0) 
    {
    PCRE2_SPTR first;
    PCRE2_SPTR last;
    PCRE2_SPTR lastentry; 
    if (firstptr == NULL) return GET2(entry, 0);
    lastentry = nametable + entrysize * (code->name_count - 1);
    first = last = entry;
    while (first > nametable)
      {
      if (PRIV(strcmp)(stringname, (first - entrysize + IMM2_SIZE)) != 0) break;
      first -= entrysize;
      }
    while (last < lastentry)
      {
      if (PRIV(strcmp)(stringname, (last + entrysize + IMM2_SIZE)) != 0) break;
      last += entrysize;
      }
    *firstptr = first;
    *lastptr = last; 
    return entrysize;
    }
  if (c > 0) bot = mid + 1; else top = mid;
  }

return PCRE2_ERROR_NOSUBSTRING;
}


/*************************************************
*           Find number for named string         *
*************************************************/

/* This function is a convenience wrapper for pcre2_substring_nametable_scan()
when it is known that names are unique. If there are duplicate names, it is not 
defined which number is returned.

Arguments:
  code        the compiled regex
  stringname  the name whose number is required

Returns:      the number of the named parenthesis, or a negative number
                (PCRE2_ERROR_NOSUBSTRING) if not found
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_substring_number_from_name(const pcre2_code *code, 
  PCRE2_SPTR stringname)
{
return pcre2_substring_nametable_scan(code, stringname, NULL, NULL);
}

/* End of pcre2_substring.c */
