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


#include "pcre2_compile.h"

/*************************************************
*   Compute the hash code from a capture name    *
*************************************************/

/* This function returns with a simple hash code
computed from the name of a capture group.

Arguments:
  name         name of the capture group
  length       the length of the name

Returns:       hash code
*/

uint16_t
PRIV(compile_get_hash_from_name)(PCRE2_SPTR name, uint32_t length)
{
uint16_t hash;

PCRE2_ASSERT(length > 0);

hash = (uint16_t)((name[0] & 0x7f) | ((name[length - 1] & 0xff) << 7));
PCRE2_ASSERT(hash <= NAMED_GROUP_HASH_MASK);
return hash;
}


/*************************************************
*   Get the descriptor of a known named capture  *
*************************************************/

/* This function returns the descriptor in the
named group list of a known capture group.

Arguments:
  name         name of the capture group
  length       the length of the name

Returns:       pointer to the descriptor when found,
               NULL otherwise
 */

named_group *
PRIV(compile_find_named_group)(PCRE2_SPTR name,
  uint32_t length, compile_block *cb)
{
uint16_t hash = PRIV(compile_get_hash_from_name)(name, length);
named_group *ng;
named_group *end = cb->named_groups + cb->names_found;

for (ng = cb->named_groups; ng < end; ng++)
  if (length == ng->length && hash == NAMED_GROUP_GET_HASH(ng) &&
      PRIV(strncmp)(name, ng->name, length) == 0) return ng;

return NULL;
}


/*************************************************
*     Add an entry to the name/number table      *
*************************************************/

/* This function is called between compiling passes to add an entry to the
name/number table, maintaining alphabetical order. Checking for permitted
and forbidden duplicates has already been done.

Arguments:
  cb           the compile data block
  nb           named group entry
  tablecount   the count of names in the table so far

Returns:       new tablecount
*/

uint32_t
PRIV(compile_add_name_to_table)(compile_block *cb,
  named_group *ng, uint32_t tablecount)
{
uint32_t i;
PCRE2_SPTR name = ng->name;
int length = ng->length;
uint32_t duplicate_count = 1;

PCRE2_UCHAR *slot = cb->name_table;

PCRE2_ASSERT(length > 0);

if ((ng->hash_dup & NAMED_GROUP_IS_DUPNAME) != 0)
  {
  named_group *ng_it;
  named_group *end = cb->named_groups + cb->names_found;

  for (ng_it = ng + 1; ng_it < end; ng_it++)
    if (ng_it->name == name) duplicate_count++;
  }

for (i = 0; i < tablecount; i++)
  {
  int crc = memcmp(name, slot + IMM2_SIZE, CU2BYTES(length));
  if (crc == 0 && slot[IMM2_SIZE + length] != 0)
    crc = -1; /* Current name is a substring */

  /* Make space in the table and break the loop for an earlier name. For a
  duplicate or later name, carry on. We do this for duplicates so that in the
  simple case (when ?(| is not used) they are in order of their numbers. In all
  cases they are in the order in which they appear in the pattern. */

  if (crc < 0)
    {
    (void)memmove(slot + cb->name_entry_size * duplicate_count, slot,
      CU2BYTES((tablecount - i) * cb->name_entry_size));
    break;
    }

  /* Continue the loop for a later or duplicate name */

  slot += cb->name_entry_size;
  }

tablecount += duplicate_count;

while (TRUE)
  {
  PUT2(slot, 0, ng->number);
  memcpy(slot + IMM2_SIZE, name, CU2BYTES(length));

  /* Add a terminating zero and fill the rest of the slot with zeroes so that
  the memory is all initialized. Otherwise valgrind moans about uninitialized
  memory when saving serialized compiled patterns. */

  memset(slot + IMM2_SIZE + length, 0,
    CU2BYTES(cb->name_entry_size - length - IMM2_SIZE));

  if (--duplicate_count == 0) break;

  while (TRUE)
    {
    ++ng;
    if (ng->name == name) break;
    }

  slot += cb->name_entry_size;
  }

return tablecount;
}


/*************************************************
*    Find details of duplicate group names       *
*************************************************/

/* This is called from compile_branch() when it needs to know the index and
count of duplicates in the names table when processing named backreferences,
either directly, or as conditions.

Arguments:
  name          points to the name
  length        the length of the name
  indexptr      where to put the index
  countptr      where to put the count of duplicates
  errorcodeptr  where to put an error code
  cb            the compile block

Returns:        TRUE if OK, FALSE if not, error code set
*/

BOOL
PRIV(compile_find_dupname_details)(PCRE2_SPTR name, uint32_t length,
  int *indexptr, int *countptr, int *errorcodeptr, compile_block *cb)
{
uint32_t i, groupnumber;
int count;
PCRE2_UCHAR *slot = cb->name_table;

/* Find the first entry in the table */

for (i = 0; i < cb->names_found; i++)
  {
  if (PRIV(strncmp)(name, slot + IMM2_SIZE, length) == 0 &&
      slot[IMM2_SIZE + length] == 0) break;
  slot += cb->name_entry_size;
  }

/* This should not occur, because this function is called only when we know we
have duplicate names. Give an internal error. */

if (i >= cb->names_found)
  {
  PCRE2_DEBUG_UNREACHABLE();
  *errorcodeptr = ERR53;
  cb->erroroffset = name - cb->start_pattern;
  return FALSE;
  }

/* Record the index and then see how many duplicates there are, updating the
backref map and maximum back reference as we do. */

*indexptr = i;
count = 0;

for (;;)
  {
  count++;
  groupnumber = GET2(slot, 0);
  cb->backref_map |= (groupnumber < 32)? (1u << groupnumber) : 1;
  if (groupnumber > cb->top_backref) cb->top_backref = groupnumber;
  if (++i >= cb->names_found) break;
  slot += cb->name_entry_size;
  if (PRIV(strncmp)(name, slot + IMM2_SIZE, length) != 0 ||
    (slot + IMM2_SIZE)[length] != 0) break;
  }

*countptr = count;
return TRUE;
}


/* End of pcre2_compile_cgroup.c */
