/***************************************************
* A program for testing the Unicode property table *
***************************************************/

/* Copyright (c) University of Cambridge 2008-2022 */

/* Compile thus:

   gcc -DHAVE_CONFIG_H -DPCRE2_CODE_UNIT_WIDTH=8 -o ucptest \
     ucptest.c ../src/pcre2_ucd.c ../src/pcre2_tables.c

   Add -lreadline or -ledit if PCRE2 was configured with readline or libedit
   support in pcre2test.
*/

/* This is a hacked-up program for testing the Unicode properties tables of
PCRE2. It can also be used for finding characters with certain properties. I
wrote it to help with debugging, and have added things that I found useful, in
a rather haphazard way. The code has never been seriously tidied or checked for
robustness, but it shouldn't now give compiler warnings.

There is only one option: "-s". If given, it applies only to the "findprop"
command. It causes the UTF-8 sequence of bytes that encode the character to be
output between angle brackets at the end of the line. On a UTF-8 terminal, this
will show the appropriate graphic for the code point.

If the command has arguments, they are concatenated into a buffer, separated by
spaces. If the first argument starts "U+" or consists entirely of hexadecimal
digits, "findprop" is inserted at the start. The buffer is then processed as a
single line file, after which the program exits. If there are no arguments, the
program reads commands line by line on stdin and writes output to stdout. The
return code is always zero.

There are three commands:

The command "findprop" must be followed by a space-separated list of Unicode
code points as hex numbers, either without any prefix or starting with "U+", or
as individual UTF-8 characters preceded by '+'. For example:

  findprop U+1234 5Abc +?

The output is one long line per character, listing Unicode properties that have
values, followed by its other case or cases if one or more exist, followed by
its Script Extension list if there is one. This list is in square brackets. A
second list in square brackets gives all the Boolean properties of the
character. The properties that come first are:

  Bidi class          e.g. NSM (most common is L)
  General type        e.g. Letter
  Specific type       e.g. Upper case letter
  Script              e.g. Medefaidrin
  Grapheme break type e.g. Extend (most common is Other)

Script names and Boolean property names are all in lower case, with underscores
and hyphens removed, because that's how they are stored for "loose" matching.

The command "find" must be followed by a list of property types and their
values. The values are case-sensitive, except for bidi class. This finds
characters that have those properties. If multiple properties are listed, they
must all be matched. Currently supported:

  script <name>    The character must have this script property. Only one
                     such script may be given.
  scriptx <name>   This script must be in the character's Script Extension
                     property list. If this is used many times, all the given
                     scripts must be present.
  type <abbrev>    The character's specific type (e.g. Lu or Nd) must match.
  gbreak <name>    The grapheme break property must match.
  bidi <class>     The character's bidi class must match.
  bool <name>      The character's Boolean property list must contain this
                     property.

If a <name> or <abbrev> is preceded by !, the value must NOT be present. For
Script Extensions and Boolean properties, there may be a mixture of positive
and negative requirements. All must be satisfied.

Sequences of two or more characters are shown as ranges, for example
U+0041..U+004A. No more than 100 lines are are output. If there are more
characters, the list ends with ...

The command "list" must be followed by one of property names script, bool,
type, gbreak or bidi. The defined values for that property are listed. */


#ifdef HAVE_CONFIG_H
#include "../src/config.h"
#endif

#ifndef SUPPORT_UNICODE
#define SUPPORT_UNICODE
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/pcre2_internal.h"
#include "../src/pcre2_ucp.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(SUPPORT_LIBREADLINE) || defined(SUPPORT_LIBEDIT)
#if defined(SUPPORT_LIBREADLINE)
#include <readline/readline.h>
#include <readline/history.h>
#else
#if defined(HAVE_EDITLINE_READLINE_H)
#include <editline/readline.h>
#else
#include <readline/readline.h>
#ifdef RL_VERSION_MAJOR
#include <readline/history.h>
#endif
#endif
#endif
#endif


/* -------------------------------------------------------------------*/

#define CS   (char *)
#define CCS  (const char *)
#define CSS  (char **)
#define US   (unsigned char *)
#define CUS  (const unsigned char *)
#define USS  (unsigned char **)

/* -------------------------------------------------------------------*/

static BOOL show_character = FALSE;

static const unsigned char *type_names[] = {
  US"Cc", US"Control",
  US"Cf", US"Format",
  US"Cn", US"Unassigned",
  US"Co", US"Private use",
  US"Cs", US"Surrogate",
  US"Ll", US"Lower case letter",
  US"Lm", US"Modifier letter",
  US"Lo", US"Other letter",
  US"Lt", US"Title case letter",
  US"Lu", US"Upper case letter",
  US"Mc", US"Spacing mark",
  US"Me", US"Enclosing mark",
  US"Mn", US"Non-spacing mark",
  US"Nd", US"Decimal number",
  US"Nl", US"Letter number",
  US"No", US"Other number",
  US"Pc", US"Connector punctuation",
  US"Pd", US"Dash punctuation",
  US"Pe", US"Close punctuation",
  US"Pf", US"Final punctuation",
  US"Pi", US"Initial punctuation",
  US"Po", US"Other punctuation",
  US"Ps", US"Open punctuation",
  US"Sc", US"Currency symbol",
  US"Sk", US"Modifier symbol",
  US"Sm", US"Mathematical symbol",
  US"So", US"Other symbol",
  US"Zl", US"Line separator",
  US"Zp", US"Paragraph separator",
  US"Zs", US"Space separator"
};

static const unsigned char *gb_names[] = {
  US"CR",                    US"carriage return",
  US"LF",                    US"linefeed",
  US"Control",               US"",
  US"Extend",                US"",
  US"Prepend",               US"",
  US"SpacingMark",           US"",
  US"L",                     US"Hangul syllable type L",
  US"V",                     US"Hangul syllable type V",
  US"T",                     US"Hangul syllable type T",
  US"LV",                    US"Hangul syllable type LV",
  US"LVT",                   US"Hangul syllable type LVT",
  US"Regional_Indicator",    US"",
  US"Other",                 US"",
  US"ZWJ",                   US"zero width joiner",
  US"Extended_Pictographic", US""
};

static const unsigned char *bd_names[] = {
  US"AL",   US"Arabic letter",
  US"AN",   US"Arabid number",
  US"B",    US"Paragraph separator",
  US"BN",   US"Boundary neutral",
  US"CS",   US"Common separator",
  US"EN",   US"European number",
  US"ES",   US"European separator",
  US"ET",   US"European terminator",
  US"FSI",  US"First string isolate",
  US"L",    US"Left-to-right",
  US"LRE",  US"Left-to-right embedding",
  US"LRI",  US"Left-to-right isolate",
  US"LRO",  US"Left-to-right override",
  US"NSM",  US"Non-spacing mark",
  US"ON",   US"Other neutral",
  US"PDF",  US"Pop directional format",
  US"PDI",  US"Pop directional isolate",
  US"R",    US"Right-to-left",
  US"RLE",  US"Right-to-left embedding",
  US"RLI",  US"Right-to-left isolate",
  US"RLO",  US"Right-to-left override",
  US"S",    US"Segment separator",
  US"WS",   US"White space"
};

static const unsigned int utf8_table1[] = {
  0x0000007f, 0x000007ff, 0x0000ffff, 0x001fffff, 0x03ffffff, 0x7fffffff};

static const int utf8_table2[] = {
  0, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc};

/* Macro to pick up the remaining bytes of a UTF-8 character, advancing
the pointer. */

#define GETUTF8INC(c, eptr) \
    { \
    if ((c & 0x20u) == 0) \
      c = ((c & 0x1fu) << 6) | (*eptr++ & 0x3fu); \
    else if ((c & 0x10u) == 0) \
      { \
      c = ((c & 0x0fu) << 12) | ((*eptr & 0x3fu) << 6) | (eptr[1] & 0x3fu); \
      eptr += 2; \
      } \
    else if ((c & 0x08u) == 0) \
      { \
      c = ((c & 0x07u) << 18) | ((*eptr & 0x3fu) << 12) | \
          ((eptr[1] & 0x3fu) << 6) | (eptr[2] & 0x3fu); \
      eptr += 3; \
      } \
    else if ((c & 0x04u) == 0) \
      { \
      c = ((c & 0x03u) << 24) | ((*eptr & 0x3fu) << 18) | \
          ((eptr[1] & 0x3fu) << 12) | ((eptr[2] & 0x3fu) << 6) | \
          (eptr[3] & 0x3fu); \
      eptr += 4; \
      } \
    else \
      { \
      c = ((c & 0x01u) << 30) | ((*eptr & 0x3fu) << 24) | \
          ((eptr[1] & 0x3fu) << 18) | ((eptr[2] & 0x3fu) << 12) | \
          ((eptr[3] & 0x3fu) << 6) | (eptr[4] & 0x3fu); \
      eptr += 5; \
      } \
    }



/*************************************************
*       Convert character value to UTF-8         *
*************************************************/

/* This function takes an unsigned long integer value in the range 0 -
0x7fffffff and encodes it as a UTF-8 character in 1 to 6 bytes.

Arguments:
  cvalue     the character value
  buffer     pointer to buffer for result - at least 6 bytes long

Returns:     number of bytes placed in the buffer
             0 if input code point is too big
*/

static size_t
ord2utf8(unsigned int cvalue, unsigned char *buffer)
{
size_t i, j;
for (i = 0; i < sizeof(utf8_table1)/sizeof(int); i++)
  if (cvalue <= utf8_table1[i]) break;
if (i >= sizeof(utf8_table1)/sizeof(int)) return 0;
buffer += i;
for (j = i; j > 0; j--)
 {
 *buffer-- = 0x80 | (cvalue & 0x3f);
 cvalue >>= 6;
 }
*buffer = utf8_table2[i] | cvalue;
return i + 1;
}



/*************************************************
*             Test for interaction               *
*************************************************/

static BOOL
is_stdin_tty(void)
{
#if defined WIN32
return _isatty(_fileno(stdin));
#else
return isatty(fileno(stdin));
#endif
}


/*************************************************
*            Get  name from ucp ident            *
*************************************************/

/* The utt table contains both full names and abbreviations. So search for both
and use the longer if two are found, unless the first one is only 3 characters
and we are looking for a script (some scripts have 3-character names). If this
were not just a test program it might be worth making some kind of reverse
index. */

static const char *
get_propname(int prop, int type)
{
size_t i, j, len;
size_t foundlist[2];
const char *yield;
int typex = (type == PT_SC)? PT_SCX : type;

j = 0;
for (i = 0; i < PRIV(utt_size); i++)
  {
  const ucp_type_table *u = PRIV(utt) + i;
  if ((u->type == type || u->type == typex) && u->value == prop)
    {
    foundlist[j++] = i;
    if (j >= 2) break;
    }
  }
  
if (j == 0) return "??";

yield = NULL;
len = 0;

for (i = 0; i < j; i++)
  {
  const char *s = PRIV(utt_names) + (PRIV(utt) + foundlist[i])->name_offset;
  size_t sl = strlen(s);

  if (sl > len)
    {
    yield = s;
    if (sl == 3 && type == PT_SC) break;
    len = sl;
    }
  }

return yield;
}


/*************************************************
*      Print Unicode property info for a char    *
*************************************************/

static void
print_prop(unsigned int c, BOOL is_just_one)
{
int type = UCD_CATEGORY(c);
int fulltype = UCD_CHARTYPE(c);
int script = UCD_SCRIPT(c);
int scriptx = UCD_SCRIPTX(c);
int gbprop = UCD_GRAPHBREAK(c);
int bidi = UCD_BIDICLASS(c);
unsigned int othercase = UCD_OTHERCASE(c);
int caseset = UCD_CASESET(c);
int bprops = UCD_BPROPS(c);

const unsigned char *fulltypename = US"??";
const unsigned char *typename = US"??";
const unsigned char *graphbreak = US"??";
const unsigned char *bidiclass = US"??";
const unsigned char *scriptname = CUS get_propname(script, PT_SC);

switch (type)
  {
  case ucp_C: typename = US"Control"; break;
  case ucp_L: typename = US"Letter"; break;
  case ucp_M: typename = US"Mark"; break;
  case ucp_N: typename = US"Number"; break;
  case ucp_P: typename = US"Punctuation"; break;
  case ucp_S: typename = US"Symbol"; break;
  case ucp_Z: typename = US"Separator"; break;
  }

switch (fulltype)
  {
  case ucp_Cc: fulltypename = US"Control"; break;
  case ucp_Cf: fulltypename = US"Format"; break;
  case ucp_Cn: fulltypename = US"Unassigned"; break;
  case ucp_Co: fulltypename = US"Private use"; break;
  case ucp_Cs: fulltypename = US"Surrogate"; break;
  case ucp_Ll: fulltypename = US"Lower case letter"; break;
  case ucp_Lm: fulltypename = US"Modifier letter"; break;
  case ucp_Lo: fulltypename = US"Other letter"; break;
  case ucp_Lt: fulltypename = US"Title case letter"; break;
  case ucp_Lu: fulltypename = US"Upper case letter"; break;
  case ucp_Mc: fulltypename = US"Spacing mark"; break;
  case ucp_Me: fulltypename = US"Enclosing mark"; break;
  case ucp_Mn: fulltypename = US"Non-spacing mark"; break;
  case ucp_Nd: fulltypename = US"Decimal number"; break;
  case ucp_Nl: fulltypename = US"Letter number"; break;
  case ucp_No: fulltypename = US"Other number"; break;
  case ucp_Pc: fulltypename = US"Connector punctuation"; break;
  case ucp_Pd: fulltypename = US"Dash punctuation"; break;
  case ucp_Pe: fulltypename = US"Close punctuation"; break;
  case ucp_Pf: fulltypename = US"Final punctuation"; break;
  case ucp_Pi: fulltypename = US"Initial punctuation"; break;
  case ucp_Po: fulltypename = US"Other punctuation"; break;
  case ucp_Ps: fulltypename = US"Open punctuation"; break;
  case ucp_Sc: fulltypename = US"Currency symbol"; break;
  case ucp_Sk: fulltypename = US"Modifier symbol"; break;
  case ucp_Sm: fulltypename = US"Mathematical symbol"; break;
  case ucp_So: fulltypename = US"Other symbol"; break;
  case ucp_Zl: fulltypename = US"Line separator"; break;
  case ucp_Zp: fulltypename = US"Paragraph separator"; break;
  case ucp_Zs: fulltypename = US"Space separator"; break;
  }

switch(gbprop)
  {
  case ucp_gbCR:           graphbreak = US"CR"; break;
  case ucp_gbLF:           graphbreak = US"LF"; break;
  case ucp_gbControl:      graphbreak = US"Control"; break;
  case ucp_gbExtend:       graphbreak = US"Extend"; break;
  case ucp_gbPrepend:      graphbreak = US"Prepend"; break;
  case ucp_gbSpacingMark:  graphbreak = US"SpacingMark"; break;
  case ucp_gbL:            graphbreak = US"Hangul syllable type L"; break;
  case ucp_gbV:            graphbreak = US"Hangul syllable type V"; break;
  case ucp_gbT:            graphbreak = US"Hangul syllable type T"; break;
  case ucp_gbLV:           graphbreak = US"Hangul syllable type LV"; break;
  case ucp_gbLVT:          graphbreak = US"Hangul syllable type LVT"; break;
  case ucp_gbRegional_Indicator:
                           graphbreak = US"Regional Indicator"; break;
  case ucp_gbOther:        graphbreak = US"Other"; break;
  case ucp_gbZWJ:          graphbreak = US"Zero Width Joiner"; break;
  case ucp_gbExtended_Pictographic:
                           graphbreak = US"Extended Pictographic"; break;
  default:                 graphbreak = US"Unknown"; break;
  }

switch(bidi)
  {
  case ucp_bidiAL:   bidiclass = US"AL "; break;
  case ucp_bidiFSI:  bidiclass = US"FSI"; break;
  case ucp_bidiL:    bidiclass = US"L  "; break;
  case ucp_bidiLRE:  bidiclass = US"LRE"; break;
  case ucp_bidiLRI:  bidiclass = US"LRI"; break;
  case ucp_bidiLRO:  bidiclass = US"LRO"; break;
  case ucp_bidiPDF:  bidiclass = US"PDF"; break;
  case ucp_bidiPDI:  bidiclass = US"PDI"; break;
  case ucp_bidiR:    bidiclass = US"R  "; break;
  case ucp_bidiRLE:  bidiclass = US"RLE"; break;
  case ucp_bidiRLI:  bidiclass = US"RLI"; break;
  case ucp_bidiRLO:  bidiclass = US"RLO"; break;
  case ucp_bidiAN:   bidiclass = US"AN "; break;
  case ucp_bidiB:    bidiclass = US"B  "; break;
  case ucp_bidiBN:   bidiclass = US"BN "; break;
  case ucp_bidiCS:   bidiclass = US"CS "; break;
  case ucp_bidiEN:   bidiclass = US"EN "; break;
  case ucp_bidiES:   bidiclass = US"ES "; break;
  case ucp_bidiET:   bidiclass = US"ET "; break;
  case ucp_bidiNSM:  bidiclass = US"NSM"; break;
  case ucp_bidiON:   bidiclass = US"ON "; break;
  case ucp_bidiS:    bidiclass = US"S  "; break;
  case ucp_bidiWS:   bidiclass = US"WS "; break;
  default:           bidiclass = US"???"; break;
  }

printf("U+%04X %s %s: %s, %s, %s", c, bidiclass, typename, fulltypename,
  scriptname, graphbreak);

if (is_just_one && othercase != c)
  {
  printf(", U+%04X", othercase);
  if (caseset != 0)
    {
    const uint32_t *p = PRIV(ucd_caseless_sets) + caseset - 1;
    while (*(++p) < NOTACHAR)
      {
      unsigned int d = *p;
      if (d != othercase && d != c) printf(", U+%04X", d);
      }
    }
  }

if (scriptx != 0)
  {
  const char *sep = "";
  const uint32_t *p = PRIV(ucd_script_sets) + scriptx;
  printf(", [");
  for (int i = 0; i < ucp_Unknown; i++)
  if (MAPBIT(p, i) != 0)
    {
    printf("%s%s", sep, get_propname(i, PT_SC));
    sep = ", ";
    }
  printf("]");
  }

if (bprops != 0)
  {
  const char *sep = "";
  const uint32_t *p = PRIV(ucd_boolprop_sets) + 
    bprops * ucd_boolprop_sets_item_size;
  printf(", [");
  for (int i = 0; i < ucp_Bprop_Count; i++)
  if (MAPBIT(p, i) != 0)
    {
    printf("%s%s", sep, get_propname(i, PT_BOOL));
    sep = ", ";
    }
  printf("]");
  }

if (show_character && is_just_one)
  {
  unsigned char buffer[8];
  size_t len = ord2utf8(c, buffer);
  printf(", >%.*s<", (int)len, buffer);
  }

printf("\n");
}



/*************************************************
*   Find character(s) with given property/ies    *
*************************************************/

static void
find_chars(unsigned char *s)
{
unsigned char name[128];
unsigned char value[128];
unsigned char *t;
unsigned int count= 0;
int scriptx_list[128];
unsigned int scriptx_count = 0;
int bprop_list[128];
unsigned int bprop_count = 0;
uint32_t i, c;
int script = -1;
int type = -1;
int gbreak = -1;
int bidiclass = -1;
BOOL script_not = FALSE;
BOOL type_not = FALSE;
BOOL gbreak_not = FALSE;
BOOL bidiclass_not = FALSE;
BOOL hadrange = FALSE;
const ucd_record *ucd, *next_ucd;
const char *pad = "        ";

while (*s != 0)
  {
  unsigned int offset = 0;
  BOOL scriptx_not = FALSE;

  for (t = name; *s != 0 && !isspace(*s); s++) *t++ = *s;
  *t = 0;
  while (isspace(*s)) s++;

  for (t = value; *s != 0 && !isspace(*s); s++) 
    {
    if (*s != '_' && *s != '-') *t++ = *s;
    } 
  *t = 0;
  while (isspace(*s)) s++;

  if (strcmp(CS name, "script") == 0 ||
      strcmp(CS name, "scriptx") == 0)
    {
    for (t = value; *t != 0; t++) *t = tolower(*t);
 
    if (value[0] == '!')
      {
      if (name[6] == 'x') scriptx_not = TRUE;
        else script_not = TRUE;
      offset = 1;
      }

    for (i = 0; i < PRIV(utt_size); i++)
      {
      const ucp_type_table *u = PRIV(utt) + i;
      if ((u->type == PT_SCX || u->type == PT_SC) && strcmp(CS(value + offset),
            PRIV(utt_names) + u->name_offset) == 0)
        {
        c = u->value;
        if (name[6] == 'x')
          {
          scriptx_list[scriptx_count++] = scriptx_not? (-c):c;
          }
        else
          {
          if (script < 0) script = c; else
            {
            printf("** Only 1 script value allowed\n");
            return;
            }
          }
        break;
        }
      }

    if (i >= PRIV(utt_size))
      {
      printf("** Unrecognized script name \"%s\"\n", value);
      return;
      }
    }

  else if (strcmp(CS name, "bool") == 0)
    {
    int not = 1;
    if (value[0] == '!')
      {
      not = -1;
      offset = 1;
      }

    for (i = 0; i < PRIV(utt_size); i++)
      {
      const ucp_type_table *u = PRIV(utt) + i;
      if (u->type == PT_BOOL && strcmp(CS(value + offset),
            PRIV(utt_names) + u->name_offset) == 0)
        {
        bprop_list[bprop_count++] = u->value * not;
        break;
        }
      }

    if (i >= PRIV(utt_size))
      {
      printf("** Unrecognized property name \"%s\"\n", value);
      return;
      }
    }

  else if (strcmp(CS name, "type") == 0)
    {
    if (type >= 0)
      {
      printf("** Only 1 type value allowed\n");
      return;
      }
    else
      {
      if (value[0] == '!')
        {
        type_not = TRUE;
        offset = 1;
        }

      for (i = 0; i < sizeof(type_names)/sizeof(char *); i += 2)
        {
        if (strcmp(CS (value + offset), CS type_names[i]) == 0)
          {
          type = i/2;
          break;
          }
        }
      if (i >= sizeof(type_names)/sizeof(char *))
        {
        printf("** Unrecognized type name \"%s\"\n", value);
        return;
        }
      }
    }

  else if (strcmp(CS name, "gbreak") == 0)
    {
    if (gbreak >= 0)
      {
      printf("** Only 1 grapheme break value allowed\n");
      return;
      }
    else
      {
      if (value[0] == '!')
        {
        gbreak_not = TRUE;
        offset = 1;
        }

      for (i = 0; i < sizeof(gb_names)/sizeof(char *); i += 2)
        {
        if (strcmp(CS (value + offset), CS gb_names[i]) == 0)
          {
          gbreak = i/2;
          break;
          }
        }
      if (i >= sizeof(gb_names)/sizeof(char *))
        {
        printf("** Unrecognized gbreak name \"%s\"\n", value);
        return;
        }
      }
    }

  else if (strcmp(CS name, "bidi") == 0 ||
           strcmp(CS name, "bidiclass") == 0 ||
           strcmp(CS name, "bidi_class") == 0 )
    {
    if (bidiclass >= 0)
      {
      printf("** Only 1 bidi class value allowed\n");
      return;
      }
    else
      {
      if (value[0] == '!')
        {
        bidiclass_not = TRUE;
        offset = 1;
        }
      for (i = 0; i < sizeof(bd_names)/sizeof(char *); i += 2)
        {
        if (strcasecmp(CS (value + offset), CS bd_names[i]) == 0)
          {
          bidiclass = i/2;
          break;
          }
        }
      if (i >= sizeof(bd_names)/sizeof(char *))
        {
        printf("** Unrecognized bidi class name \"%s\"\n", value);
        return;
        }
      }
    }

  else
    {
    printf("** Unrecognized property name \"%s\"\n", name);
    return;
    }
  }

if (script < 0 && scriptx_count == 0 && bprop_count == 0 && type < 0 &&
    gbreak < 0 && bidiclass < 0)
  {
  printf("** No properties specified\n");
  return;
  }

for (c = 0; c <= 0x10ffff; c++)
  {
  if (script >= 0 && (script == UCD_SCRIPT(c)) == script_not) continue;

  if (scriptx_count > 0)
    {
    const uint32_t *bits_scriptx = PRIV(ucd_script_sets) + UCD_SCRIPTX(c);
    unsigned int found = 0;

    for (i = 0; i < scriptx_count; i++)
      {
      int x = scriptx_list[i]/32;
      int y = scriptx_list[i]%32;

      /* Positive requirment */
      if (scriptx_list[i] >= 0)
        {
        if ((bits_scriptx[x] & (1u<<y)) != 0) found++;
        }
      /* Negative requirement */
      else
        {
        if ((bits_scriptx[x] & (1u<<y)) == 0) found++;
        }
      }

    if (found != scriptx_count) continue;
    }

  if (bprop_count > 0)
    {
    const uint32_t *bits_bprop = PRIV(ucd_boolprop_sets) + 
      UCD_BPROPS(c) * ucd_boolprop_sets_item_size;
    unsigned int found = 0;

    for (i = 0; i < bprop_count; i++)
      {
      int x = bprop_list[i]/32;
      int y = bprop_list[i]%32;

      /* Positive requirement */
      if (bprop_list[i] >= 0)
        {
        if ((bits_bprop[x] & (1u<<y)) != 0) found++;
        }
      /* Negative requirement */
      else
        {
        if ((bits_bprop[-x] & (1u<<(-y))) == 0) found++;
        }
      }

    if (found != bprop_count) continue;
    }

  if (type >= 0)
    {
    if (type_not)
      {
      if (type == UCD_CHARTYPE(c)) continue;
      }
    else
      {
      if (type != UCD_CHARTYPE(c)) continue;
      }
    }

  if (gbreak >= 0)
    {
    if (gbreak_not)
      {
      if (gbreak == UCD_GRAPHBREAK(c)) continue;
      }
    else
      {
      if (gbreak != UCD_GRAPHBREAK(c)) continue;
      }
    }

  if (bidiclass >= 0)
    {
    if (bidiclass_not)
      {
      if (bidiclass == UCD_BIDICLASS(c)) continue;
      }
    else
      {
      if (bidiclass != UCD_BIDICLASS(c)) continue;
      }
    }

  /* All conditions are met. Look for runs. */

  ucd = GET_UCD(c);

  for (i = c + 1; i < 0x10ffff; i++)
    {
    next_ucd = GET_UCD(i);
    if (memcmp(ucd, next_ucd, sizeof(ucd_record)) != 0) break;
    }

  if (--i > c)
    {
    printf("U+%04X..", c);
    c = i;
    hadrange = TRUE;
    }
  else if (hadrange) printf("%s", pad);

  print_prop(c, FALSE);
  if (c >= 0x100000) pad = "        ";
    else if (c >= 0x10000) pad = "       ";
  count++;
  if (count >= 100)
    {
    printf("...\n");
    break;
    }
  }

if (count == 0) printf("No characters found\n");
}


/*************************************************
*        Process command line                    *
*************************************************/

static void
process_command_line(unsigned char *buffer)
{
unsigned char *s, *t;
unsigned char name[24];

s = buffer;
while (isspace(*s)) s++;
if (*s == 0) return;

for (t = name; *s != 0 && !isspace(*s); s++) *t++ = *s;
*t = 0;
while (isspace(*s)) s++;

if (strcmp(CS name, "findprop") == 0)
  {
  while (*s != 0)
    {
    unsigned int c;
    unsigned char *endptr;
    t = s;

    if (*t == '+')
      {
      c = *(++t);
      if (c > 0x7fu)
        {
        GETCHARINC(c, t);
        }
      endptr = t+1;
      }
    else
      {
      if (strncmp(CS t, "U+", 2) == 0) t += 2;
      c = strtoul(CS t, CSS(&endptr), 16);
      }

    if (*endptr != 0 && !isspace(*endptr))
      {
      while (*endptr != 0 && !isspace(*endptr)) endptr++;
      printf("** Invalid character specifier: ignored \"%.*s\"\n", (int)(endptr-s), s);
      }
    else
      {
      if (c > 0x10ffff)
        printf("** U+%x is too big for a Unicode code point\n", c);
      else
        print_prop(c, TRUE);
      }
    s = endptr;
    while (isspace(*s)) s++;
    }
  }

else if (strcmp(CS name, "find") == 0)
  {
  find_chars(s);
  }

else if (strcmp(CS name, "list") == 0)
  {
  while (*s != 0)
    {
    size_t i;
    for (t = name; *s != 0 && !isspace(*s); s++) *t++ = *s;
    *t = 0;
    while (isspace(*s)) s++;

    if (strcmp(CS name, "script") == 0 || strcmp(CS name, "scripts") == 0)
      {
      for (i = 0; i < PRIV(utt_size); i++)
        if (PRIV(utt)[i].type == PT_SCX || PRIV(utt)[i].type == PT_SC)
          printf("%s\n", PRIV(utt_names) + PRIV(utt)[i].name_offset);
      }

    else if (strcmp(CS name, "bool") == 0)
      {
      for (i = 0; i < PRIV(utt_size); i++)
        if (PRIV(utt)[i].type == PT_BOOL)
          printf("%s\n", PRIV(utt_names) + PRIV(utt)[i].name_offset);
      }

    else if (strcmp(CS name, "type") == 0 || strcmp(CS name, "types") == 0)
      {
      for (i = 0; i < sizeof(type_names)/sizeof(char *); i += 2)
        printf("%s %s\n", type_names[i], type_names[i+1]);
      }

    else if (strcmp(CS name, "gbreak") == 0 || strcmp(CS name, "gbreaks") == 0)
      {
      for (i = 0; i < sizeof(gb_names)/sizeof(char *); i += 2)
        {
        if (gb_names[i+1][0] != 0)
          printf("%-3s (%s)\n", gb_names[i], gb_names[i+1]);
        else
          printf("%s\n", gb_names[i]);
        }
      }

    else if (strcmp(CS name, "bidi") == 0 ||
             strcmp(CS name, "bidiclasses") == 0)
      {
      for (i = 0; i < sizeof(bd_names)/sizeof(char *); i += 2)
        printf("%3s %s\n", bd_names[i], bd_names[i+1]);
      }

    else
      {
      printf("** Unknown property \"%s\"\n", name);
      break;
      }
    }
  }

else printf("** Unknown test command \"%s\"\n", name);
}



/*************************************************
*               Main program                     *
*************************************************/

int
main(int argc, char **argv)
{
BOOL interactive;
int first_arg = 1;
unsigned char buffer[1024];

if (argc > 1 && strcmp(argv[1], "-s") == 0)
  {
  show_character = TRUE;
  first_arg++;
  }

if (argc > first_arg)
  {
  int i;
  BOOL datafirst = TRUE;
  char *arg = argv[first_arg];
  unsigned char *s = buffer;

  if (*arg != '+' && strncmp(arg, "U+", 2) != 0 && !isdigit(*arg))
    {
    while (*arg != 0)
      {
      if (!isxdigit(*arg++)) { datafirst = FALSE; break; }
      }
    }

  if (datafirst)
    {
    strcpy(CS s, "findprop ");
    s += 9;
    }

  for (i = first_arg; i < argc; i++)
    {
    s += sprintf(CS s, "%s ", argv[i]);
    }

  process_command_line(buffer);
  return 0;
  }

interactive = is_stdin_tty();

#if defined(SUPPORT_LIBREADLINE) || defined(SUPPORT_LIBEDIT)
if (interactive) using_history();
#endif

for(;;)
  {
#if defined(SUPPORT_LIBREADLINE) || defined(SUPPORT_LIBEDIT)
  if (interactive)
    {
    size_t len;
    unsigned char *s = US readline("> ");
    if (s == NULL) break;
    len = strlen(CS s);
    if (len > 0) add_history(CS s);
    memcpy(buffer, s, len);
    buffer[len] = '\n';
    buffer[len+1] = 0;
    free(s);
    }
  else
#endif

    {
    if (interactive) printf("> ");
    if (fgets(CS buffer, sizeof(buffer), stdin) == NULL) break;
    if (!interactive) printf("%s", buffer);
    }

  process_command_line(buffer);
  }

if (interactive) printf("\n");

#if defined(SUPPORT_LIBREADLINE) || defined(SUPPORT_LIBEDIT)
if (interactive) clear_history();
#endif

return 0;
}

/* End */
