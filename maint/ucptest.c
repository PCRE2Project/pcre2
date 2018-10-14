/***************************************************
* A program for testing the Unicode property table *
***************************************************/

/* Copyright (c) University of Cambridge 2008 - 2018 */

/* Compile thus:
   gcc -DHAVE_CONFIG_H -DPCRE2_CODE_UNIT_WIDTH=8 -o ucptest \
     ucptest.c ../src/pcre2_ucd.c ../src/pcre2_tables.c
   Add -lreadline or -ledit if required.
*/

/* This is a hacked-up program for testing the Unicode properties tables of
PCRE2. It can also be used for finding characters with certain properties.
I wrote it to help with debugging PCRE, and have added things that I found
useful, in a rather haphazard way. The code has never been "tidied" or checked
for robustness.

If there are arguments, they are a list of hexadecimal code points whose
properties are to be output. Otherwise, the program expects to read commands on
stdin, and it writes output to stdout. There are two commands:

"findprop" must be followed by a list of Unicode code points as hex numbers
(without any prefixes). The output is one line per character, giving its
Unicode properties followed by its other case if there is one, followed by its
Script Extension list if it is not just the same as the base script.

"find" must be followed by a list of property names and their values. This
finds characters that have those properties. If multiple properties are listed,
they must all be matched. Currently supported:

  script <name>    The character must have this script property. Only one
                     such script may be given.
  scriptx <name>   This script must be in the character's Script Extension
                     property list. If this is used many times, all the given
                     scripts must be present.
  type <abbrev>    The character's type (e.g. Lu or Nd) must match.
  gbreak <name>    The grapheme break property must match.

If a <name> or <abbrev> is preceded by !, the value must NOT be present. For
Script Extensions, there may be a mixture of positive and negative
requirements. All must be satisfied.

No more than 100 characters are output. If there are more, the list ends with
... */

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


const unsigned char *script_names[] = {
  US"Unknown",
  US"Arabic",
  US"Armenian",
  US"Bengali",
  US"Bopomofo",
  US"Braille",
  US"Buginese",
  US"Buhid",
  US"Canadian_Aboriginal",
  US"Cherokee",
  US"Common",
  US"Coptic",
  US"Cypriot",
  US"Cyrillic",
  US"Deseret",
  US"Devanagari",
  US"Ethiopic",
  US"Georgian",
  US"Glagolitic",
  US"Gothic",
  US"Greek",
  US"Gujarati",
  US"Gurmukhi",
  US"Han",
  US"Hangul",
  US"Hanunoo",
  US"Hebrew",
  US"Hiragana",
  US"Inherited",
  US"Kannada",
  US"Katakana",
  US"Kharoshthi",
  US"Khmer",
  US"Lao",
  US"Latin",
  US"Limbu",
  US"Linear_B",
  US"Malayalam",
  US"Mongolian",
  US"Myanmar",
  US"New_Tai_Lue",
  US"Ogham",
  US"Old_Italic",
  US"Old_Persian",
  US"Oriya",
  US"Osmanya",
  US"Runic",
  US"Shavian",
  US"Sinhala",
  US"Syloti_Nagri",
  US"Syriac",
  US"Tagalog",
  US"Tagbanwa",
  US"Tai_Le",
  US"Tamil",
  US"Telugu",
  US"Thaana",
  US"Thai",
  US"Tibetan",
  US"Tifinagh",
  US"Ugaritic",
  US"Yi",
  /* New for Unicode 5.0: */
  US"Balinese",
  US"Cuneiform",
  US"Nko",
  US"Phags_Pa",
  US"Phoenician",
  /* New for Unicode 5.1: */
  US"Carian",
  US"Cham",
  US"Kayah_Li",
  US"Lepcha",
  US"Lycian",
  US"Lydian",
  US"Ol_Chiki",
  US"Rejang",
  US"Saurashtra",
  US"Sundanese",
  US"Vai",
  /* New for Unicode 5.2: */
  US"Avestan",
  US"Bamum",
  US"Egyptian_Hieroglyphs",
  US"Imperial_Aramaic",
  US"Inscriptional_Pahlavi",
  US"Inscriptional_Parthian",
  US"Javanese",
  US"Kaithi",
  US"Lisu",
  US"Meetei_Mayek",
  US"Old_South_Arabian",
  US"Old_Turkic",
  US"Samaritan",
  US"Tai_Tham",
  US"Tai_Viet",
  /* New for Unicode 6.0.0 */
  US"Batak",
  US"Brahmi",
  US"Mandaic",
  /* New for Unicode 6.1.0 */
  US"Chakma",
  US"Meroitic_Cursive",
  US"Meroitic_Hieroglyphs",
  US"Miao",
  US"Sharada",
  US"Sora Sompent",
  US"Takri",
  /* New for Unicode 7.0.0 */
  US"Bassa_Vah",
  US"Caucasian_Albanian",
  US"Duployan",
  US"Elbasan",
  US"Grantha",
  US"Khojki",
  US"Khudawadi",
  US"Linear_A",
  US"Mahajani",
  US"Manichaean",
  US"Mende_Kikakui",
  US"Modi",
  US"Mro",
  US"Nabataean",
  US"Old_North_Arabian",
  US"Old_Permic",
  US"Pahawh_Hmong",
  US"Palmyrene",
  US"Psalter_Pahlavi",
  US"Pau_Cin_Hau",
  US"Siddham",
  US"Tirhuta",
  US"Warang_Citi",
  /* New for Unicode 8.0.0 */
  US"Ahom",
  US"Anatolian_Hieroglyphs",
  US"Hatran",
  US"Multani",
  US"Old_Hungarian",
  US"SignWriting",
  /* New for Unicode 10.0.0 (no update since 8.0.0) */
  US"Adlam",
  US"Bhaiksuki",
  US"Marchen",
  US"Newa",
  US"Osage",
  US"Tangut",
  US"Masaram_Gondi",
  US"Nushu",
  US"Soyombo",
  US"Zanabazar_Square",
  /* New for Unicode 11.0.0 */
  US"Dogra",
  US"Gunjala_Gondi",
  US"Hanifi_Rohingya",
  US"Makasar",
  US"Medefaidrin",
  US"Old_Sogdian",
  US"Sogdian"
};

const unsigned char *type_names[] = {
  US"Cc",
  US"Cf",
  US"Cn",
  US"Co",
  US"Cs",
  US"Ll",
  US"Lm",
  US"Lo",
  US"Lt",
  US"Lu",
  US"Mc",
  US"Me",
  US"Mn",
  US"Nd",
  US"Nl",
  US"No",
  US"Pc",
  US"Pd",
  US"Pe",
  US"Pf",
  US"Pi",
  US"Po",
  US"Ps",
  US"Sc",
  US"Sk",
  US"Sm",
  US"So",
  US"Zl",
  US"Zp",
  US"Zs"
};

const unsigned char *gb_names[] = {
  US"CR",
  US"LF",
  US"Control",
  US"Extend",
  US"Prepend",
  US"SpacingMark",
  US"L",
  US"V",
  US"T",
  US"LV",
  US"LVT",
  US"RegionalIndicator",
  US"Other",
  US"ZWJ",
  US"Extended_Pictographic"
};


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
*      Print Unicode property info for a char    *
*************************************************/

static void
print_prop(int c)
{
int type = UCD_CATEGORY(c);
int fulltype = UCD_CHARTYPE(c);
int script = UCD_SCRIPT(c);
int scriptx = UCD_SCRIPTX(c);
int gbprop = UCD_GRAPHBREAK(c);
int othercase = UCD_OTHERCASE(c);
int caseset = UCD_CASESET(c);

const unsigned char *fulltypename = US"??";
const unsigned char *typename = US"??";
const unsigned char *scriptname = US"??";
const unsigned char *graphbreak = US"??";

if (script < sizeof(script_names)/sizeof(char *))
  scriptname = script_names[script];

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
  case ucp_gbRegionalIndicator:
                           graphbreak = US"Regional Indicator"; break;
  case ucp_gbOther:        graphbreak = US"Other"; break;
  case ucp_gbZWJ:          graphbreak = US"Zero Width Joiner"; break;
  case ucp_gbExtended_Pictographic:
                           graphbreak = US"Extended Pictographic"; break;
  default:                 graphbreak = US"Unknown"; break;
  }

printf("%04x %s: %s, %s, %s", c, typename, fulltypename, scriptname, graphbreak);
if (othercase != c)
  {
  printf(", %04x", othercase);
  if (caseset != 0)
    {
    const uint32_t *p = PRIV(ucd_caseless_sets) + caseset - 1;
    while (*(++p) < NOTACHAR)
      if (*p != othercase && *p != c) printf(", %04x", *p);
    }
  }

if (scriptx != script)
  {
  printf(", [");
  if (scriptx >= 0)
    {
    scriptname = (scriptx >= sizeof(script_names)/sizeof(char *))?
      US"??" : script_names[scriptx];
    printf("%s", scriptname);
    }
  else
    {
    char *sep = "";
    const uint8_t *p = PRIV(ucd_script_sets) - scriptx;
    while (*p != 0)
      {
      scriptname = (*p >= sizeof(script_names)/sizeof(char *))?
        US"??" : script_names[*p++];
      printf("%s%s", sep, scriptname);
      sep = ", ";
      }
    }
  printf("]");
  }

printf("\n");
}



/*************************************************
*   Find character(s) with given property/ies    *
*************************************************/

static void
find_chars(unsigned char *s)
{
unsigned char name[24];
unsigned char value[24];
unsigned char *t;
unsigned int count= 0;
int scriptx_list[24];
unsigned int scriptx_count = 0;
uint32_t i, c;
int script = -1;
int type = -1;
int gbreak = -1;
BOOL script_not = FALSE;
BOOL type_not = FALSE;
BOOL gbreak_not = FALSE;
BOOL hadrange = FALSE;
const ucd_record *ucd, *next_ucd;
const char *pad = "      ";

while (*s != 0)
  {
  unsigned int offset = 0;
  BOOL scriptx_not = FALSE;

  for (t = name; *s != 0 && !isspace(*s); s++) *t++ = *s;
  *t = 0;
  while (isspace(*s)) s++;

  for (t = value; *s != 0 && !isspace(*s); s++) *t++ = *s;
  *t = 0;
  while (isspace(*s)) s++;

  if (strcmp(CS name, "script") == 0 ||
      strcmp(CS name, "scriptx") == 0)
    {
    if (value[0] == '!')
      {
      if (name[6] == 'x') scriptx_not = TRUE;
        else script_not = TRUE;
      offset = 1;
      }

    for (i = 0; i < sizeof(script_names)/sizeof(char *); i++)
      {
      if (strcmp(CS value + offset, script_names[i]) == 0)
        {
        if (name[6] == 'x')
          {
          scriptx_list[scriptx_count++] = scriptx_not? (-i):i;
          }
        else
          {
          if (script < 0) script = i; else
            {
            printf("** Only 1 script value allowed\n");
            return;
            }
          }
        break;
        }
      }

    if (i >= sizeof(script_names)/sizeof(char *))
      {
      printf("** Unrecognized script name '%s'\n", value);
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

      for (i = 0; i < sizeof(type_names)/sizeof(char *); i++)
        {
        if (strcmp(CS (value + offset), type_names[i]) == 0)
          {
          type = i;
          break;
          }
        }
      if (i >= sizeof(type_names)/sizeof(char *))
        {
        printf("** Unrecognized type name '%s'\n", value);
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

      for (i = 0; i < sizeof(gb_names)/sizeof(char *); i++)
        {
        if (strcmp(CS (value + offset), gb_names[i]) == 0)
          {
          gbreak = i;
          break;
          }
        }
      if (i >= sizeof(gb_names)/sizeof(char *))
        {
        printf("** Unrecognized gbreak name '%s'\n", value);
        return;
        }
      }
    }

  else
    {
    printf("** Unrecognized property name '%s'\n", name);
    return;
    }
  }

if (script < 0 && scriptx_count == 0 && type < 0 && gbreak < 0)
  {
  printf("** No properties specified\n");
  return;
  }

for (c = 0; c <= 0x10ffff; c++)
  {
  if (script >= 0 && (script == UCD_SCRIPT(c)) == script_not) continue;

  if (scriptx_count > 0)
    {
    const uint8_t *char_scriptx = NULL;
    int found = 0;
    int scriptx = UCD_SCRIPTX(c);

    if (scriptx < 0) char_scriptx = PRIV(ucd_script_sets) - scriptx;

    for (i = 0; i < scriptx_count; i++)
      {
      /* Positive requirment */
      if (scriptx_list[i] >= 0)
        {
        if (scriptx >= 0)
          {
          if (scriptx == scriptx_list[i]) found++;
          }

        else
          {
          const uint8_t *p;
          for (p = char_scriptx; *p != 0; p++)
            {
            if (scriptx_list[i] == *p)
              {
              found++;
              break;
              }
            }
          }
        }
      /* Negative requirement */
      else
        {
        if (scriptx >= 0)
          {
          if (scriptx != -scriptx_list[i]) found++;
          }
        else
          {
          const uint8_t *p;
          for (p = char_scriptx; *p != 0; p++)
            if (-scriptx_list[i] == *p) break;
          if (*p == 0) found++;
          }
        }
      }

    if (found != scriptx_count) continue;
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

  /* All conditions are met. Look for runs. */

  ucd = GET_UCD(c);

  for (i = c + 1; i < 0x10ffff; i++)
    {
    next_ucd = GET_UCD(i);
    if (memcmp(ucd, next_ucd, sizeof(ucd_record)) != 0) break;
    }

  if (--i > c)
    {
    printf("%04x..", c);
    c = i;
    hadrange = TRUE;
    }
  else if (hadrange) printf("%s", pad);

  print_prop(c);
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
*               Main program                     *
*************************************************/

int
main(int argc, char **argv)
{
BOOL interactive;
unsigned char buffer[1024];

if (argc > 1)
  {
  int i;
  for (i = 1; i < argc; i++)
    {
    unsigned char *endptr;
    int c = strtoul(argv[i], CSS(&endptr), 16);
    if (*endptr != 0)
      printf("** Hex number expected; ignored '%s'\n", argv[i]);
    else print_prop(c);
    }
  return 0;
  }

interactive = is_stdin_tty();

#if defined(SUPPORT_LIBREADLINE) || defined(SUPPORT_LIBEDIT)
if (interactive) using_history();
#endif

for(;;)
  {
  unsigned char name[24];
  unsigned char *s, *t;

#if defined(SUPPORT_LIBREADLINE) || defined(SUPPORT_LIBEDIT)
  if (interactive)
    {
    size_t len;
    s = readline("> ");
    if (s == NULL) break;
    len = strlen(s);
    if (len > 0) add_history(s);
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

  s = buffer;
  while (isspace(*s)) s++;
  if (*s == 0) continue;

  for (t = name; *s != 0 && !isspace(*s); s++) *t++ = *s;
  *t = 0;
  while (isspace(*s)) s++;

  if (strcmp(CS name, "findprop") == 0)
    {
    while (*s != 0)
      {
      unsigned char *endptr;
      int c = strtoul(CS s, CSS(&endptr), 16);

      if (*endptr != 0 && !isspace(*endptr))
        {
        while (*endptr != 0 && !isspace(*endptr)) endptr++;
        printf("** Hex number expected; ignored '%.*s'\n", endptr-s, s);
        }
      else  print_prop(c);
      s = endptr;
      while (isspace(*s)) s++;
      }
    }

  else if (strcmp(CS name, "find") == 0)
    {
    find_chars(s);
    }

  else printf("** Unknown test command %s\n", name);
  }

if (interactive) printf("\n");

#if defined(SUPPORT_LIBREADLINE) || defined(SUPPORT_LIBEDIT)
if (interactive) clear_history();
#endif

return 0;
}

/* End */
