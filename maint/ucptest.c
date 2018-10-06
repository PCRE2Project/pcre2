/***************************************************
* A program for testing the Unicode property table *
***************************************************/

/* Copyright (c) University of Cambridge 2008 - 2018 */

/* Compile thus:
   gcc -DHAVE_CONFIG_H -DPCRE2_CODE_UNIT_WIDTH=8 -o ucptest \
     ucptest.c ../src/pcre2_ucd.c ../src/pcre2_tables.c
*/

/* If there are arguments, they are a list of hexadecimal code points whose
properties are to be output. Otherwise, the program expects to read commands on
stdin, and it writes output to stdout. There is only one command, "findprop",
followed by a list of Unicode code points as hex numbers (without any
prefixes). The output is one line per character, giving its Unicode properties
followed by its other case if there is one. */

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



/* -------------------------------------------------------------------*/

#define CS   (char *)
#define CCS  (const char *)
#define CSS  (char **)
#define US   (unsigned char *)
#define CUS  (const unsigned char *)
#define USS  (unsigned char **)

/* -------------------------------------------------------------------*/




/*************************************************
*          Find a script name                    *
*************************************************/

static unsigned char *
find_script_name(int script)
{
switch(script)
  {
  default:              return US"??"; 
  case ucp_Unknown:     return US"Unknown";  
  case ucp_Arabic:      return US"Arabic"; 
  case ucp_Armenian:    return US"Armenian"; 
  case ucp_Balinese:    return US"Balinese"; 
  case ucp_Bengali:     return US"Bengali"; 
  case ucp_Bopomofo:    return US"Bopomofo"; 
  case ucp_Braille:     return US"Braille"; 
  case ucp_Buginese:    return US"Buginese"; 
  case ucp_Buhid:       return US"Buhid"; 
  case ucp_Canadian_Aboriginal: return US"Canadian_Aboriginal"; 
  case ucp_Cherokee:    return US"Cherokee"; 
  case ucp_Common:      return US"Common"; 
  case ucp_Coptic:      return US"Coptic"; 
  case ucp_Cuneiform:   return US"Cuneiform"; 
  case ucp_Cypriot:     return US"Cypriot"; 
  case ucp_Cyrillic:    return US"Cyrillic"; 
  case ucp_Deseret:     return US"Deseret"; 
  case ucp_Devanagari:  return US"Devanagari"; 
  case ucp_Ethiopic:    return US"Ethiopic"; 
  case ucp_Georgian:    return US"Georgian"; 
  case ucp_Glagolitic:  return US"Glagolitic"; 
  case ucp_Gothic:      return US"Gothic"; 
  case ucp_Greek:       return US"Greek"; 
  case ucp_Gujarati:    return US"Gujarati"; 
  case ucp_Gurmukhi:    return US"Gurmukhi"; 
  case ucp_Han:         return US"Han"; 
  case ucp_Hangul:      return US"Hangul"; 
  case ucp_Hanunoo:     return US"Hanunoo"; 
  case ucp_Hebrew:      return US"Hebrew"; 
  case ucp_Hiragana:    return US"Hiragana"; 
  case ucp_Inherited:   return US"Inherited"; 
  case ucp_Kannada:     return US"Kannada"; 
  case ucp_Katakana:    return US"Katakana"; 
  case ucp_Kharoshthi:  return US"Kharoshthi"; 
  case ucp_Khmer:       return US"Khmer"; 
  case ucp_Lao:         return US"Lao"; 
  case ucp_Latin:       return US"Latin"; 
  case ucp_Limbu:       return US"Limbu"; 
  case ucp_Linear_B:    return US"Linear_B"; 
  case ucp_Malayalam:   return US"Malayalam"; 
  case ucp_Mongolian:   return US"Mongolian"; 
  case ucp_Myanmar:     return US"Myanmar"; 
  case ucp_New_Tai_Lue: return US"New_Tai_Lue"; 
  case ucp_Nko:         return US"Nko"; 
  case ucp_Ogham:       return US"Ogham"; 
  case ucp_Old_Italic:  return US"Old_Italic"; 
  case ucp_Old_Persian: return US"Old_Persian"; 
  case ucp_Oriya:       return US"Oriya"; 
  case ucp_Osmanya:     return US"Osmanya"; 
  case ucp_Phags_Pa:    return US"Phags_Pa"; 
  case ucp_Phoenician:  return US"Phoenician"; 
  case ucp_Runic:       return US"Runic"; 
  case ucp_Shavian:     return US"Shavian"; 
  case ucp_Sinhala:     return US"Sinhala"; 
  case ucp_Syloti_Nagri: return US"Syloti_Nagri"; 
  case ucp_Syriac:      return US"Syriac"; 
  case ucp_Tagalog:     return US"Tagalog"; 
  case ucp_Tagbanwa:    return US"Tagbanwa"; 
  case ucp_Tai_Le:      return US"Tai_Le"; 
  case ucp_Tamil:       return US"Tamil"; 
  case ucp_Telugu:      return US"Telugu"; 
  case ucp_Thaana:      return US"Thaana"; 
  case ucp_Thai:        return US"Thai"; 
  case ucp_Tibetan:     return US"Tibetan"; 
  case ucp_Tifinagh:    return US"Tifinagh"; 
  case ucp_Ugaritic:    return US"Ugaritic"; 
  case ucp_Yi:          return US"Yi"; 
  /* New for Unicode 5.1: */
  case ucp_Carian:      return US"Carian"; 
  case ucp_Cham:        return US"Cham"; 
  case ucp_Kayah_Li:    return US"Kayah_Li"; 
  case ucp_Lepcha:      return US"Lepcha"; 
  case ucp_Lycian:      return US"Lycian"; 
  case ucp_Lydian:      return US"Lydian"; 
  case ucp_Ol_Chiki:    return US"Ol_Chiki"; 
  case ucp_Rejang:      return US"Rejang"; 
  case ucp_Saurashtra:  return US"Saurashtra"; 
  case ucp_Sundanese:   return US"Sundanese"; 
  case ucp_Vai:         return US"Vai"; 
  /* New for Unicode 5.2: */
  case ucp_Avestan:     return US"Avestan"; 
  case ucp_Bamum:       return US"Bamum"; 
  case ucp_Egyptian_Hieroglyphs: return US"Egyptian_Hieroglyphs"; 
  case ucp_Imperial_Aramaic: return US"Imperial_Aramaic"; 
  case ucp_Inscriptional_Pahlavi: return US"Inscriptional_Pahlavi"; 
  case ucp_Inscriptional_Parthian: return US"Inscriptional_Parthian"; 
  case ucp_Javanese:    return US"Javanese"; 
  case ucp_Kaithi:      return US"Kaithi"; 
  case ucp_Lisu:        return US"Lisu"; 
  case ucp_Meetei_Mayek: return US"Meetei_Mayek"; 
  case ucp_Old_South_Arabian: return US"Old_South_Arabian"; 
  case ucp_Old_Turkic:  return US"Old_Turkic"; 
  case ucp_Samaritan:   return US"Samaritan"; 
  case ucp_Tai_Tham:    return US"Tai_Tham"; 
  case ucp_Tai_Viet:    return US"Tai_Viet"; 
  /* New for Unicode 6.0.0 */
  case ucp_Batak:       return US"Batak"; 
  case ucp_Brahmi:      return US"Brahmi"; 
  case ucp_Mandaic:     return US"Mandaic"; 

  /* New for Unicode 6.1.0 */
  case ucp_Chakma:               return US"Chakma"; 
  case ucp_Meroitic_Cursive:     return US"Meroitic_Cursive"; 
  case ucp_Meroitic_Hieroglyphs: return US"Meroitic_Hieroglyphs"; 
  case ucp_Miao:                 return US"Miao"; 
  case ucp_Sharada:              return US"Sharada"; 
  case ucp_Sora_Sompeng:         return US"Sora Sompent"; 
  case ucp_Takri:                return US"Takri"; 

  /* New for Unicode 7.0.0 */
  case ucp_Bassa_Vah:          return US"Bassa_Vah"; 
  case ucp_Caucasian_Albanian: return US"Caucasian_Albanian"; 
  case ucp_Duployan:           return US"Duployan"; 
  case ucp_Elbasan:            return US"Elbasan"; 
  case ucp_Grantha:            return US"Grantha"; 
  case ucp_Khojki:             return US"Khojki"; 
  case ucp_Khudawadi:          return US"Khudawadi"; 
  case ucp_Linear_A:           return US"Linear_A"; 
  case ucp_Mahajani:           return US"Mahajani"; 
  case ucp_Manichaean:         return US"Manichaean"; 
  case ucp_Mende_Kikakui:      return US"Mende_Kikakui"; 
  case ucp_Modi:               return US"Modi"; 
  case ucp_Mro:                return US"Mro"; 
  case ucp_Nabataean:          return US"Nabataean"; 
  case ucp_Old_North_Arabian:  return US"Old_North_Arabian"; 
  case ucp_Old_Permic:         return US"Old_Permic"; 
  case ucp_Pahawh_Hmong:       return US"Pahawh_Hmong"; 
  case ucp_Palmyrene:          return US"Palmyrene"; 
  case ucp_Psalter_Pahlavi:    return US"Psalter_Pahlavi"; 
  case ucp_Pau_Cin_Hau:        return US"Pau_Cin_Hau"; 
  case ucp_Siddham:            return US"Siddham"; 
  case ucp_Tirhuta:            return US"Tirhuta"; 
  case ucp_Warang_Citi:        return US"Warang_Citi"; 

  /* New for Unicode 8.0.0 */
  case ucp_Ahom:                  return US"Ahom"; 
  case ucp_Anatolian_Hieroglyphs: return US"Anatolian_Hieroglyphs"; 
  case ucp_Hatran:                return US"Hatran"; 
  case ucp_Multani:               return US"Multani"; 
  case ucp_Old_Hungarian:         return US"Old_Hungarian"; 
  case ucp_SignWriting:           return US"SignWriting"; 

  /* New for Unicode 10.0.0 (no update since 8.0.0) */
  case ucp_Adlam:               return US"Adlam"; 
  case ucp_Bhaiksuki:           return US"Bhaiksuki"; 
  case ucp_Marchen:             return US"Marchen"; 
  case ucp_Newa:                return US"Newa"; 
  case ucp_Osage:               return US"Osage"; 
  case ucp_Tangut:              return US"Tangut"; 
  case ucp_Masaram_Gondi:       return US"Masaram_Gondi"; 
  case ucp_Nushu:               return US"Nushu"; 
  case ucp_Soyombo:             return US"Soyombo"; 
  case ucp_Zanabazar_Square:    return US"Zanabazar_Square"; 

  /* New for Unicode 11.0.0 */ 
  case ucp_Dogra:               return US"Dogra";  
  case ucp_Gunjala_Gondi:       return US"Gunjala_Gondi";  
  case ucp_Hanifi_Rohingya:     return US"Hanifi_Rohingya";  
  case ucp_Makasar:             return US"Makasar";  
  case ucp_Medefaidrin:         return US"Medefaidrin"; 
  case ucp_Old_Sogdian:         return US"Old_Sogdian";  
  case ucp_Sogdian:             return US"Sogdian"; 
  }
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

unsigned char *fulltypename = US"??";
unsigned char *typename = US"??";
unsigned char *graphbreak = US"??";

unsigned char *scriptname = find_script_name(script); 

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
  if (scriptx >= 0) printf("%s", find_script_name(scriptx)); else
    {
    char *sep = ""; 
    const uint8_t *p = PRIV(ucd_script_sets) - scriptx;
    while (*p != 0)
      {
      printf("%s%s", sep, find_script_name(*p++));
      sep = ", "; 
      }   
    }  
  printf("]");
  } 
 
printf("\n");
}



/*************************************************
*               Main program                     *
*************************************************/

int
main(int argc, char **argv)
{
unsigned char buffer[1024];

if (argc > 1)
  {
  int i;
  for (i = 1; i < argc; i++)
    {
    unsigned char *endptr; 
    int c = strtoul(argv[i], CSS(&endptr), 16);
    print_prop(c); 
    }
  return 0;
  }    

while (fgets(CS buffer, sizeof(buffer), stdin) != NULL)
  {
  unsigned char name[24];
  unsigned char *s, *t;

  printf("%s", buffer);
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
      print_prop(c);
      s = endptr;
      while (isspace(*s)) s++;
      }
    }

  else printf("Unknown test command %s\n", name);
  }

return 0;
}

/* End */
