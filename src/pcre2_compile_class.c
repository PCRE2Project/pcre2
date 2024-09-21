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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre2_compile.h"

#ifdef SUPPORT_WIDE_CHARS

/* Heapsort algorithm. */

static void do_heapify(uint32_t *buffer, size_t size, size_t i)
{
size_t max;
size_t left;
size_t right;
uint32_t tmp1, tmp2;

while (TRUE)
  {
  max = i;
  left = (i << 1) + 2;
  right = left + 2;

  if (left < size && buffer[left] > buffer[max]) max = left;
  if (right < size && buffer[right] > buffer[max]) max = right;
  if (i == max) return;

  /* Swap items. */
  tmp1 = buffer[i];
  tmp2 = buffer[i + 1];
  buffer[i] = buffer[max];
  buffer[i + 1] = buffer[max + 1];
  buffer[max] = tmp1;
  buffer[max + 1] = tmp2;
  i = max;
  }
}

#ifdef SUPPORT_UNICODE

#define PARSE_CLASS_CASELESS_UTF      0x1
#define PARSE_CLASS_RESTRICTED_UTF    0x2

static const uint32_t*
get_nocase_range(uint32_t c)
{
uint32_t left = 0;
uint32_t right = PRIV(ucd_nocase_ranges_size);
uint32_t middle;

if (c > MAX_UTF_CODE_POINT) return PRIV(ucd_nocase_ranges) + right;

while (TRUE)
  {
  /* Range end of the middle element. */
  middle = ((left + right) >> 1) | 0x1;

  if (PRIV(ucd_nocase_ranges)[middle] <= c)
    left = middle + 1;
  else if (middle > 1 && PRIV(ucd_nocase_ranges)[middle - 2] > c)
    right = middle - 1;
  else
    return PRIV(ucd_nocase_ranges) + (middle - 1);
  }
}

static size_t
utf_caseless_extend(uint32_t start, uint32_t end, uint32_t options,
  uint32_t *buffer)
{
uint32_t new_start = start;
uint32_t new_end = end;
uint32_t c = start;
const uint32_t *list;
uint32_t tmp[3];
size_t result = 2;
const uint32_t *skip_range = get_nocase_range(c);
uint32_t skip_start = skip_range[0];

#if PCRE2_CODE_UNIT_WIDTH == 32
if (end > MAX_UTF_CODE_POINT) end = MAX_UTF_CODE_POINT;
#endif

while (c <= end)
  {
  uint32_t co;
    
  if (c > skip_start)
    {
    c = skip_range[1];
    skip_range += 2;
    skip_start = skip_range[0];
    continue;
    }

  /* Compute caseless set. */
  co = UCD_CASESET(c);

  if (co != 0 && (!(options & PARSE_CLASS_RESTRICTED_UTF)
                  || PRIV(ucd_caseless_sets)[co] > 127))
    list = PRIV(ucd_caseless_sets) + co;
  else
    {
    co = UCD_OTHERCASE(c);
    list = tmp;
    tmp[0] = c;
    tmp[1] = NOTACHAR;

    if (co != c)
      {
      tmp[1] = co;
      tmp[2] = NOTACHAR;
      }
    }
  c++;

  /* Add characters. */
  do
    {
    if (*list < new_start)
      {
      if (*list + 1 == new_start)
        {
        new_start--;
        continue;
        }
      }
    else if (*list > new_end)
      {
      if (*list - 1 == new_end)
        {
        new_end++;
        continue;
        }
      }
    else continue;

    result += 2;
    if (buffer != NULL)
      {
      buffer[0] = *list;
      buffer[1] = *list;
      buffer += 2;
      }
    }
  while (*(++list) != NOTACHAR);
  }

  if (buffer != NULL)
    {
    buffer[0] = new_start;
    buffer[1] = new_end;
    buffer += 2;
    }
  return result;
}

#endif

static size_t
parse_class(uint32_t *ptr, uint32_t options, uint32_t *buffer)
{
size_t total_size = 0;
uint32_t meta_arg;
uint32_t start_char;

(void)options; /* Avoid compiler warning. */

while (*ptr != META_CLASS_END)
  {
  switch (META_CODE(*ptr))
    {
    case META_ESCAPE:
      meta_arg = META_DATA(*ptr);
      if (meta_arg == ESC_P || meta_arg == ESC_p) ptr++;
      ptr++;
      continue;
    case META_POSIX:
    case META_POSIX_NEG:
      ptr += 2;
      continue;
    case META_BIGVALUE:
      /* Character literal */
      ptr++;
      break;
    default:
      PCRE2_ASSERT(*ptr < META_END);
      break;
    }

    start_char = *ptr;

    if (ptr[1] == META_RANGE_LITERAL || ptr[1] == META_RANGE_ESCAPED)
      {
      ptr += 2;
      PCRE2_ASSERT(*ptr < META_END || *ptr == META_BIGVALUE);

      if (*ptr == META_BIGVALUE) ptr++;
      }

#ifdef SUPPORT_UNICODE
    if (options & PARSE_CLASS_CASELESS_UTF)
      {
      size_t size = utf_caseless_extend(start_char, *ptr++, options, buffer);
      if (buffer != NULL) buffer += size;
      total_size += size;
      continue;
      }
#endif

    if (buffer != NULL)
      {
      buffer[0] = start_char;
      buffer[1] = *ptr;
      buffer += 2;
      }

    ptr++;
    total_size += 2;
  }

  return total_size;
}

uint32_t *PRIV(optimize_class)(uint32_t *start_ptr, uint32_t options,
  size_t *buffer_size, compile_block* cb)
{
uint32_t *ptr = start_ptr + 1;
uint32_t *buffer;
uint32_t *dst;
size_t size = 0, i;
uint32_t tmp1, tmp2;

PCRE2_ASSERT(*start_ptr == META_CLASS || *start_ptr == META_CLASS_NOT);

#ifdef SUPPORT_UNICODE
if ((options & PCRE2_CASELESS) && (options & (PCRE2_UTF|PCRE2_UCP)))
  options = PARSE_CLASS_CASELESS_UTF;
else
  options = 0;

if (cb->cx->extra_options & PCRE2_EXTRA_CASELESS_RESTRICT)
  options |= PARSE_CLASS_RESTRICTED_UTF;
#endif

/* Compute required space for the range. */

size = parse_class(start_ptr + 1, options, NULL);

*buffer_size = size;
if (size == 0) return NULL;

/* Allocate and buffer. */

buffer = (uint32_t*)
  cb->cx->memctl.malloc(size * sizeof(uint32_t), cb->cx->memctl.memory_data);

if (buffer == NULL) return NULL;

parse_class(start_ptr + 1, options, buffer);

if (size == 2) return buffer;

/* In-place sorting of ranges. */

i = (((size >> 2) - 1) << 1);
while (TRUE)
  {
  do_heapify(buffer, size, i);
  if (i == 0) break;
  i -= 2;
  }

i = size - 2;
while (TRUE)
  {
  tmp1 = buffer[i];
  tmp2 = buffer[i + 1];
  buffer[i] = buffer[0];
  buffer[i + 1] = buffer[1];
  buffer[0] = tmp1;
  buffer[1] = tmp2;

  do_heapify(buffer, i, 0);
  if (i == 0) break;
  i -= 2;
  }

/* Merge ranges whenever possible. */
dst = buffer;
ptr = buffer + 2;
size -= 2;

/* The second condition is a very rare corner case, where the end of the last
range is the maximum character. This range cannot be extended further. */

while (size > 0 && dst[1] != ~(uint32_t)0)
  {
  if (dst[1] + 1 < ptr[0])
    {
    dst += 2;
    dst[0] = ptr[0];
    dst[1] = ptr[1];
    }
  else if (dst[1] < ptr[1]) dst[1] = ptr[1];

  ptr += 2;
  size -= 2;
  }

*buffer_size = (size_t)(dst + 2 - buffer);
return buffer;
}

#endif /* SUPPORT_WIDE_CHARS */

/* End of pcre2_compile_class.c */
