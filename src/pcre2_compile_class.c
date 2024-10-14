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

#define PARSE_CLASS_UTF               0x1
#define PARSE_CLASS_CASELESS_UTF      0x2
#define PARSE_CLASS_RESTRICTED_UTF    0x4
#define PARSE_CLASS_TURKISH_UTF       0x8

/* Get the range of nocase characters which includes the
'c' character passed as argument, or directly follows 'c'. */

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

/* Get the list of othercase characters, which belongs to the passed range.
Create ranges from these characters, and append them to the buffer argument. */

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

#if PCRE2_CODE_UNIT_WIDTH == 8
PCRE2_ASSERT(options & PARSE_CLASS_UTF);
#endif

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

  if ((options & (PARSE_CLASS_TURKISH_UTF|PARSE_CLASS_RESTRICTED_UTF)) ==
        PARSE_CLASS_TURKISH_UTF &&
      UCD_ANY_I(c))
    {
    co = PRIV(ucd_turkish_dotted_i_caseset) + (UCD_DOTTED_I(c)? 0 : 3);
    }
  else if ((co = UCD_CASESET(c)) != 0 &&
           (options & PARSE_CLASS_RESTRICTED_UTF) != 0 &&
           PRIV(ucd_caseless_sets)[co] < 128)
    {
    co = 0;  /* Ignore the caseless set if it's restricted. */
    }

  if (co != 0)
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
#if PCRE2_CODE_UNIT_WIDTH == 16
    if (!(options & PARSE_CLASS_UTF) && *list > 0xffff) continue;
#endif

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

/* Add a character list to a buffer. */

static size_t
append_char_list(const uint32_t *p, uint32_t *buffer)
{
const uint32_t *n;
size_t result = 0;

while (*p != NOTACHAR)
  {
  n = p;
  while (n[0] == n[1] - 1) n++;

  PCRE2_ASSERT(*p < 0xffff);

  if (buffer != NULL)
    {
    buffer[0] = *p;
    buffer[1] = *n;
    buffer += 2;
    }

  result += 2;
  p = n + 1;
  }

  return result;
}

static uint32_t
get_highest_char(uint32_t options)
{
(void)options; /* Avoid compiler warning. */

#if PCRE2_CODE_UNIT_WIDTH == 8
return MAX_UTF_CODE_POINT;
#else
#ifdef SUPPORT_UNICODE
return GET_MAX_CHAR_VALUE((options & PARSE_CLASS_UTF) != 0);
#else
return MAX_UCHAR_VALUE;
#endif
#endif
}

/* Add a negated character list to a buffer. */
static size_t
append_negated_char_list(const uint32_t *p, uint32_t options, uint32_t *buffer)
{
const uint32_t *n;
uint32_t start = 0;
size_t result = 2;

PCRE2_ASSERT(*p > 0);

while (*p != NOTACHAR)
  {
  n = p;
  while (n[0] == n[1] - 1) n++;

  PCRE2_ASSERT(*p < 0xffff);

  if (buffer != NULL)
    {
    buffer[0] = start;
    buffer[1] = *p - 1;
    buffer += 2;
    }

  result += 2;
  start = *n + 1;
  p = n + 1;
  }

  if (buffer != NULL)
    {
    buffer[0] = start;
    buffer[1] = get_highest_char(options);
    buffer += 2;
    }

  return result;
}

static uint32_t *
append_non_ascii_range(uint32_t options, uint32_t *buffer)
{
  if (buffer == NULL) return NULL;

  buffer[0] = 0x100;
  buffer[1] = get_highest_char(options);
  return buffer + 2;
}

static size_t
parse_class(uint32_t *ptr, uint32_t options, uint32_t *buffer)
{
size_t total_size = 0;
size_t size;
uint32_t meta_arg;
uint32_t start_char;

while (*ptr != META_CLASS_END)
  {
  switch (META_CODE(*ptr))
    {
    case META_ESCAPE:
      meta_arg = META_DATA(*ptr);
      switch (meta_arg)
        {
        case ESC_D:
        case ESC_W:
        case ESC_S:
        buffer = append_non_ascii_range(options, buffer);
        total_size += 2;
        break;

        case ESC_h:
        size = append_char_list(PRIV(hspace_list), buffer);
        total_size += size;
        if (buffer != NULL) buffer += size;
        break;

        case ESC_H:
        size = append_negated_char_list(PRIV(hspace_list), options, buffer);
        total_size += size;
        if (buffer != NULL) buffer += size;
        break;

        case ESC_v:
        size = append_char_list(PRIV(vspace_list), buffer);
        total_size += size;
        if (buffer != NULL) buffer += size;
        break;

        case ESC_V:
        size = append_negated_char_list(PRIV(vspace_list), options, buffer);
        total_size += size;
        if (buffer != NULL) buffer += size;
        break;

        case ESC_p:
        case ESC_P:
        ptr++;
        if (meta_arg == ESC_p && *ptr == PT_ANY)
          {
          if (buffer != NULL)
            {
            buffer[0] = 0;
            buffer[1] = get_highest_char(options);
            buffer += 2;
            }
          total_size += 2;
          }
        break;
        }
      ptr++;
      continue;
    case META_POSIX_NEG:
      buffer = append_non_ascii_range(options, buffer);
      total_size += 2;
      ptr += 2;
      continue;
    case META_POSIX:
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

#ifdef EBCDIC
#error "Missing EBCDIC support"
#endif
      }

#ifdef SUPPORT_UNICODE
    if (options & PARSE_CLASS_CASELESS_UTF)
      {
      size = utf_caseless_extend(start_char, *ptr++, options, buffer);
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

class_ranges *PRIV(optimize_class)(uint32_t *start_ptr,
  uint32_t options, uint32_t xoptions, compile_block* cb)
{
class_ranges* cranges;
uint32_t *ptr = start_ptr + 1;
uint32_t *buffer;
uint32_t *dst;
uint32_t class_options = 0;
size_t range_list_size = 0, i;
uint32_t tmp1, tmp2;

PCRE2_ASSERT(*start_ptr == META_CLASS || *start_ptr == META_CLASS_NOT);

#ifdef SUPPORT_UNICODE
if (options & PCRE2_UTF)
  class_options |= PARSE_CLASS_UTF;

if ((options & PCRE2_CASELESS) && (options & (PCRE2_UTF|PCRE2_UCP)))
  class_options |= PARSE_CLASS_CASELESS_UTF;

if (xoptions & PCRE2_EXTRA_CASELESS_RESTRICT)
  class_options |= PARSE_CLASS_RESTRICTED_UTF;

if (xoptions & PCRE2_EXTRA_TURKISH_CASING)
  class_options |= PARSE_CLASS_TURKISH_UTF;
#endif

/* Compute required space for the range. */

range_list_size = parse_class(start_ptr + 1, class_options, NULL);

/* Allocate buffer. */

cranges = cb->cx->memctl.malloc(
  sizeof(class_ranges) + range_list_size * sizeof(uint32_t),
  cb->cx->memctl.memory_data);

if (cranges == NULL) return NULL;

cranges->next = NULL;
cranges->range_list_size = range_list_size;

if (range_list_size == 0) return cranges;

buffer = (uint32_t*)(cranges + 1);
parse_class(start_ptr + 1, class_options, buffer);

if (range_list_size == 2) return cranges;

/* In-place sorting of ranges. */

i = (((range_list_size >> 2) - 1) << 1);
while (TRUE)
  {
  do_heapify(buffer, range_list_size, i);
  if (i == 0) break;
  i -= 2;
  }

i = range_list_size - 2;
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
range_list_size -= 2;

/* The second condition is a very rare corner case, where the end of the last
range is the maximum character. This range cannot be extended further. */

while (range_list_size > 0 && dst[1] != ~(uint32_t)0)
  {
  if (dst[1] + 1 < ptr[0])
    {
    dst += 2;
    dst[0] = ptr[0];
    dst[1] = ptr[1];
    }
  else if (dst[1] < ptr[1]) dst[1] = ptr[1];

  ptr += 2;
  range_list_size -= 2;
  }

cranges->range_list_size = (size_t)(dst + 2 - buffer);

PCRE2_ASSERT(dst[1] <= get_highest_char(class_options));
return cranges;
}

#endif /* SUPPORT_WIDE_CHARS */

#ifdef SUPPORT_UNICODE

void PRIV(update_classbits)(uint32_t ptype, uint32_t pdata, BOOL negated,
  uint8_t *classbits)
{
/* Update PRIV(xclass) when this function is changed. */
int c, chartype;
const ucd_record *prop;
uint32_t gentype;
BOOL set_bit;

if (ptype == PT_ANY)
  {
  if (!negated) memset(classbits, 0xff, 32 * sizeof(uint8_t));
  return;
  }

for (c = 0; c < 256; c++)
  {
  prop = GET_UCD(c);
  set_bit = FALSE;

  switch (ptype)
    {
    case PT_LAMP:
    chartype = prop->chartype;
    set_bit = (chartype == ucp_Lu || chartype == ucp_Ll || chartype == ucp_Lt);
    break;

    case PT_GC:
    set_bit = (PRIV(ucp_gentype)[prop->chartype] == pdata);
    break;

    case PT_PC:
    set_bit = (prop->chartype == pdata);
    break;

    case PT_SC:
    set_bit = (prop->script == pdata);
    break;

    case PT_SCX:
    set_bit = (prop->script == pdata ||
      MAPBIT(PRIV(ucd_script_sets) + UCD_SCRIPTX_PROP(prop), pdata) != 0);
    break;

    case PT_ALNUM:
    gentype = PRIV(ucp_gentype)[prop->chartype];
    set_bit = (gentype == ucp_L || gentype == ucp_N);
    break;

    case PT_SPACE:    /* Perl space */
    case PT_PXSPACE:  /* POSIX space */
    switch(c)
      {
      HSPACE_BYTE_CASES:
      VSPACE_BYTE_CASES:
      set_bit = TRUE;
      break;

      default:
      set_bit = (PRIV(ucp_gentype)[prop->chartype] == ucp_Z);
      break;
      }
    break;

    case PT_WORD:
    chartype = prop->chartype;
    gentype = PRIV(ucp_gentype)[chartype];
    set_bit = (gentype == ucp_L || gentype == ucp_N ||
               chartype == ucp_Mn || chartype == ucp_Pc);
    break;

    case PT_UCNC:
    set_bit = (c == CHAR_DOLLAR_SIGN || c == CHAR_COMMERCIAL_AT ||
               c == CHAR_GRAVE_ACCENT || c >= 0xa0);
    break;

    case PT_BIDICL:
    set_bit = (UCD_BIDICLASS_PROP(prop) == pdata);
    break;

    case PT_BOOL:
    set_bit = MAPBIT(PRIV(ucd_boolprop_sets) +
                     UCD_BPROPS_PROP(prop), pdata) != 0;
    break;

    case PT_PXGRAPH:
    chartype = prop->chartype;
    gentype = PRIV(ucp_gentype)[chartype];
    set_bit = (gentype != ucp_Z && (gentype != ucp_C || chartype == ucp_Cf));
    break;

    case PT_PXPRINT:
    chartype = prop->chartype;
    set_bit = (chartype != ucp_Zl && chartype != ucp_Zp &&
       (PRIV(ucp_gentype)[chartype] != ucp_C || chartype == ucp_Cf));
    break;

    case PT_PXPUNCT:
    gentype = PRIV(ucp_gentype)[prop->chartype];
    set_bit = (gentype == ucp_P || (c < 128 && gentype == ucp_S));
    break;

    default:
    PCRE2_ASSERT(ptype == PT_PXXDIGIT);
    set_bit = (c >= CHAR_0 && c <= CHAR_9) ||
              (c >= CHAR_A && c <= CHAR_F) ||
              (c >= CHAR_a && c <= CHAR_f);
    break;
    }

  if (negated) set_bit = !set_bit;
  if (set_bit) *classbits |= (uint8_t)(1 << (c & 0x7));
  if ((c & 0x7) == 0x7) classbits++;
  }
}

#endif /* SUPPORT_UNICODE */

/* End of pcre2_compile_class.c */
