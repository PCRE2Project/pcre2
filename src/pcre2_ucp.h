/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
          New API code Copyright (c) 2016-2021 University of Cambridge

This module is auto-generated from Unicode data files. DO NOT EDIT MANUALLY!
Instead, modify the maint/GenerateUcpHeader.py script and run it to generate
a new version of this code.

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

#ifndef PCRE2_UCP_H_IDEMPOTENT_GUARD
#define PCRE2_UCP_H_IDEMPOTENT_GUARD

/* This file contains definitions of the Unicode property values that are
returned by the UCD access macros and used throughout PCRE2.

IMPORTANT: The specific values of the first two enums (general and particular
character categories) are assumed by the table called catposstab in the file
pcre2_auto_possess.c. They are unlikely to change, but should be checked after
an update. */

/* These are the general character categories. */

enum {
  ucp_C,
  ucp_L,
  ucp_M,
  ucp_N,
  ucp_P,
  ucp_S,
  ucp_Z,
};

/* These are the particular character categories. */

enum {
  ucp_Cc,    /* Control */
  ucp_Cf,    /* Format */
  ucp_Cn,    /* Unassigned */
  ucp_Co,    /* Private use */
  ucp_Cs,    /* Surrogate */
  ucp_Ll,    /* Lower case letter */
  ucp_Lm,    /* Modifier letter */
  ucp_Lo,    /* Other letter */
  ucp_Lt,    /* Title case letter */
  ucp_Lu,    /* Upper case letter */
  ucp_Mc,    /* Spacing mark */
  ucp_Me,    /* Enclosing mark */
  ucp_Mn,    /* Non-spacing mark */
  ucp_Nd,    /* Decimal number */
  ucp_Nl,    /* Letter number */
  ucp_No,    /* Other number */
  ucp_Pc,    /* Connector punctuation */
  ucp_Pd,    /* Dash punctuation */
  ucp_Pe,    /* Close punctuation */
  ucp_Pf,    /* Final punctuation */
  ucp_Pi,    /* Initial punctuation */
  ucp_Po,    /* Other punctuation */
  ucp_Ps,    /* Open punctuation */
  ucp_Sc,    /* Currency symbol */
  ucp_Sk,    /* Modifier symbol */
  ucp_Sm,    /* Mathematical symbol */
  ucp_So,    /* Other symbol */
  ucp_Zl,    /* Line separator */
  ucp_Zp,    /* Paragraph separator */
  ucp_Zs,    /* Space separator */
};

/* These are the bidi class values. */

enum {
  ucp_bidiAL,   /* Arabic letter */
  ucp_bidiAN,   /* Arabic number */
  ucp_bidiB,    /* Paragraph separator */
  ucp_bidiBN,   /* Boundary neutral */
  ucp_bidiCS,   /* Common separator */
  ucp_bidiEN,   /* European number */
  ucp_bidiES,   /* European separator */
  ucp_bidiET,   /* European terminator */
  ucp_bidiFSI,  /* First strong isolate */
  ucp_bidiL,    /* Left to right */
  ucp_bidiLRE,  /* Left to right embedding */
  ucp_bidiLRI,  /* Left to right isolate */
  ucp_bidiLRO,  /* Left to right override */
  ucp_bidiNSM,  /* Non-spacing mark */
  ucp_bidiON,   /* Other neutral */
  ucp_bidiPDF,  /* Pop directional format */
  ucp_bidiPDI,  /* Pop directional isolate */
  ucp_bidiR,    /* Right to left */
  ucp_bidiRLE,  /* Right to left embedding */
  ucp_bidiRLI,  /* Right to left isolate */
  ucp_bidiRLO,  /* Right to left override */
  ucp_bidiS,    /* Segment separator */
  ucp_bidiWS,   /* White space */
};

/* These are grapheme break properties. The Extended Pictographic property
comes from the emoji-data.txt file. */

enum {
  ucp_gbCR,                    /*  0 */
  ucp_gbLF,                    /*  1 */
  ucp_gbControl,               /*  2 */
  ucp_gbExtend,                /*  3 */
  ucp_gbPrepend,               /*  4 */
  ucp_gbSpacingMark,           /*  5 */
  ucp_gbL,                     /*  6 Hangul syllable type L */
  ucp_gbV,                     /*  7 Hangul syllable type V */
  ucp_gbT,                     /*  8 Hangul syllable type T */
  ucp_gbLV,                    /*  9 Hangul syllable type LV */
  ucp_gbLVT,                   /* 10 Hangul syllable type LVT */
  ucp_gbRegional_Indicator,    /* 11 */
  ucp_gbOther,                 /* 12 */
  ucp_gbZWJ,                   /* 13 */
  ucp_gbExtended_Pictographic, /* 14 */
};

/* These are the script identifications. */

enum {
  /* Scripts which has characters in other scripts. */
  ucp_Arabic,
  ucp_Bengali,
  ucp_Bopomofo,
  ucp_Buginese,
  ucp_Buhid,
  ucp_Coptic,
  ucp_Cypriot,
  ucp_Cyrillic,
  ucp_Devanagari,
  ucp_Georgian,
  ucp_Glagolitic,
  ucp_Greek,
  ucp_Gujarati,
  ucp_Gurmukhi,
  ucp_Han,
  ucp_Hangul,
  ucp_Hanunoo,
  ucp_Hiragana,
  ucp_Kannada,
  ucp_Katakana,
  ucp_Latin,
  ucp_Limbu,
  ucp_Linear_B,
  ucp_Malayalam,
  ucp_Mongolian,
  ucp_Myanmar,
  ucp_Oriya,
  ucp_Sinhala,
  ucp_Syloti_Nagri,
  ucp_Syriac,
  ucp_Tagalog,
  ucp_Tagbanwa,
  ucp_Tai_Le,
  ucp_Tamil,
  ucp_Telugu,
  ucp_Thaana,
  ucp_Yi,
  ucp_Nko,
  ucp_Phags_Pa,
  ucp_Kayah_Li,
  ucp_Javanese,
  ucp_Kaithi,
  ucp_Mandaic,
  ucp_Chakma,
  ucp_Sharada,
  ucp_Takri,
  ucp_Duployan,
  ucp_Grantha,
  ucp_Khojki,
  ucp_Khudawadi,
  ucp_Linear_A,
  ucp_Mahajani,
  ucp_Manichaean,
  ucp_Modi,
  ucp_Old_Permic,
  ucp_Psalter_Pahlavi,
  ucp_Tirhuta,
  ucp_Multani,
  ucp_Adlam,
  ucp_Masaram_Gondi,
  ucp_Dogra,
  ucp_Gunjala_Gondi,
  ucp_Hanifi_Rohingya,
  ucp_Sogdian,
  ucp_Nandinagari,
  ucp_Yezidi,
  ucp_Cypro_Minoan,
  ucp_Old_Uyghur,

  /* Scripts which has no characters in other scripts. */
  ucp_Unknown,
  ucp_Armenian,
  ucp_Braille,
  ucp_Canadian_Aboriginal,
  ucp_Cherokee,
  ucp_Common,
  ucp_Deseret,
  ucp_Ethiopic,
  ucp_Gothic,
  ucp_Hebrew,
  ucp_Inherited,
  ucp_Kharoshthi,
  ucp_Khmer,
  ucp_Lao,
  ucp_New_Tai_Lue,
  ucp_Ogham,
  ucp_Old_Italic,
  ucp_Old_Persian,
  ucp_Osmanya,
  ucp_Runic,
  ucp_Shavian,
  ucp_Thai,
  ucp_Tibetan,
  ucp_Tifinagh,
  ucp_Ugaritic,
  ucp_Balinese,
  ucp_Cuneiform,
  ucp_Phoenician,
  ucp_Carian,
  ucp_Cham,
  ucp_Lepcha,
  ucp_Lycian,
  ucp_Lydian,
  ucp_Ol_Chiki,
  ucp_Rejang,
  ucp_Saurashtra,
  ucp_Sundanese,
  ucp_Vai,
  ucp_Avestan,
  ucp_Bamum,
  ucp_Egyptian_Hieroglyphs,
  ucp_Imperial_Aramaic,
  ucp_Inscriptional_Pahlavi,
  ucp_Inscriptional_Parthian,
  ucp_Lisu,
  ucp_Meetei_Mayek,
  ucp_Old_South_Arabian,
  ucp_Old_Turkic,
  ucp_Samaritan,
  ucp_Tai_Tham,
  ucp_Tai_Viet,
  ucp_Batak,
  ucp_Brahmi,
  ucp_Meroitic_Cursive,
  ucp_Meroitic_Hieroglyphs,
  ucp_Miao,
  ucp_Sora_Sompeng,
  ucp_Bassa_Vah,
  ucp_Caucasian_Albanian,
  ucp_Elbasan,
  ucp_Mende_Kikakui,
  ucp_Mro,
  ucp_Nabataean,
  ucp_Old_North_Arabian,
  ucp_Pahawh_Hmong,
  ucp_Palmyrene,
  ucp_Pau_Cin_Hau,
  ucp_Siddham,
  ucp_Warang_Citi,
  ucp_Ahom,
  ucp_Anatolian_Hieroglyphs,
  ucp_Hatran,
  ucp_Old_Hungarian,
  ucp_SignWriting,
  ucp_Bhaiksuki,
  ucp_Marchen,
  ucp_Newa,
  ucp_Osage,
  ucp_Tangut,
  ucp_Nushu,
  ucp_Soyombo,
  ucp_Zanabazar_Square,
  ucp_Makasar,
  ucp_Medefaidrin,
  ucp_Old_Sogdian,
  ucp_Elymaic,
  ucp_Nyiakeng_Puachue_Hmong,
  ucp_Wancho,
  ucp_Chorasmian,
  ucp_Dives_Akuru,
  ucp_Khitan_Small_Script,
  ucp_Tangsa,
  ucp_Toto,
  ucp_Vithkuqi,

  /* This must be last */
  ucp_Script_Count
};

#endif  /* PCRE2_UCP_H_IDEMPOTENT_GUARD */

/* End of pcre2_ucp.h */
