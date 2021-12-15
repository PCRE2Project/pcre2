/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
          New API code Copyright (c) 2016-2021 University of Cambridge

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

/* This module contains some fixed tables that are used by more than one of the
PCRE2 code modules. The tables are also #included by the pcre2test program,
which uses macros to change their names from _pcre2_xxx to xxxx, thereby
avoiding name clashes with the library. In this case, PCRE2_PCRE2TEST is
defined. */

#ifndef PCRE2_PCRE2TEST           /* We're compiling the library */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "pcre2_internal.h"
#endif /* PCRE2_PCRE2TEST */


/* Table of sizes for the fixed-length opcodes. It's defined in a macro so that
the definition is next to the definition of the opcodes in pcre2_internal.h.
This is mode-dependent, so is skipped when this file is included by pcre2test. */

#ifndef PCRE2_PCRE2TEST
const uint8_t PRIV(OP_lengths)[] = { OP_LENGTHS };
#endif

/* Tables of horizontal and vertical whitespace characters, suitable for
adding to classes. */

const uint32_t PRIV(hspace_list)[] = { HSPACE_LIST };
const uint32_t PRIV(vspace_list)[] = { VSPACE_LIST };

/* These tables are the pairs of delimiters that are valid for callout string
arguments. For each starting delimiter there must be a matching ending
delimiter, which in fact is different only for bracket-like delimiters. */

const uint32_t PRIV(callout_start_delims)[] = {
  CHAR_GRAVE_ACCENT, CHAR_APOSTROPHE, CHAR_QUOTATION_MARK,
  CHAR_CIRCUMFLEX_ACCENT, CHAR_PERCENT_SIGN, CHAR_NUMBER_SIGN,
  CHAR_DOLLAR_SIGN, CHAR_LEFT_CURLY_BRACKET, 0 };

const uint32_t PRIV(callout_end_delims[]) = {
  CHAR_GRAVE_ACCENT, CHAR_APOSTROPHE, CHAR_QUOTATION_MARK,
  CHAR_CIRCUMFLEX_ACCENT, CHAR_PERCENT_SIGN, CHAR_NUMBER_SIGN,
  CHAR_DOLLAR_SIGN, CHAR_RIGHT_CURLY_BRACKET, 0 };


/*************************************************
*           Tables for UTF-8 support             *
*************************************************/

/* These tables are required by pcre2test in 16- or 32-bit mode, as well
as for the library in 8-bit mode, because pcre2test uses UTF-8 internally for
handling wide characters. */

#if defined PCRE2_PCRE2TEST || \
   (defined SUPPORT_UNICODE && \
    defined PCRE2_CODE_UNIT_WIDTH && \
    PCRE2_CODE_UNIT_WIDTH == 8)

/* These are the breakpoints for different numbers of bytes in a UTF-8
character. */

const int PRIV(utf8_table1)[] =
  { 0x7f, 0x7ff, 0xffff, 0x1fffff, 0x3ffffff, 0x7fffffff};

const int PRIV(utf8_table1_size) = sizeof(PRIV(utf8_table1)) / sizeof(int);

/* These are the indicator bits and the mask for the data bits to set in the
first byte of a character, indexed by the number of additional bytes. */

const int PRIV(utf8_table2)[] = { 0,    0xc0, 0xe0, 0xf0, 0xf8, 0xfc};
const int PRIV(utf8_table3)[] = { 0xff, 0x1f, 0x0f, 0x07, 0x03, 0x01};

/* Table of the number of extra bytes, indexed by the first byte masked with
0x3f. The highest number for a valid UTF-8 first byte is in fact 0x3d. */

const uint8_t PRIV(utf8_table4)[] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5 };

#endif /* UTF-8 support needed */


#ifdef SUPPORT_UNICODE

/* Table to translate from particular type value to the general value. */

const uint32_t PRIV(ucp_gentype)[] = {
  ucp_C, ucp_C, ucp_C, ucp_C, ucp_C,  /* Cc, Cf, Cn, Co, Cs */
  ucp_L, ucp_L, ucp_L, ucp_L, ucp_L,  /* Ll, Lu, Lm, Lo, Lt */
  ucp_M, ucp_M, ucp_M,                /* Mc, Me, Mn */
  ucp_N, ucp_N, ucp_N,                /* Nd, Nl, No */
  ucp_P, ucp_P, ucp_P, ucp_P, ucp_P,  /* Pc, Pd, Pe, Pf, Pi */
  ucp_P, ucp_P,                       /* Ps, Po */
  ucp_S, ucp_S, ucp_S, ucp_S,         /* Sc, Sk, Sm, So */
  ucp_Z, ucp_Z, ucp_Z                 /* Zl, Zp, Zs */
};

/* This table encodes the rules for finding the end of an extended grapheme
cluster. Every code point has a grapheme break property which is one of the
ucp_gbXX values defined in pcre2_ucp.h. These changed between Unicode versions
10 and 11. The 2-dimensional table is indexed by the properties of two adjacent
code points. The left property selects a word from the table, and the right
property selects a bit from that word like this:

  PRIV(ucp_gbtable)[left-property] & (1u << right-property)

The value is non-zero if a grapheme break is NOT permitted between the relevant
two code points. The breaking rules are as follows:

1. Break at the start and end of text (pretty obviously).

2. Do not break between a CR and LF; otherwise, break before and after
   controls.

3. Do not break Hangul syllable sequences, the rules for which are:

    L may be followed by L, V, LV or LVT
    LV or V may be followed by V or T
    LVT or T may be followed by T

4. Do not break before extending characters or zero-width-joiner (ZWJ).

The following rules are only for extended grapheme clusters (but that's what we
are implementing).

5. Do not break before SpacingMarks.

6. Do not break after Prepend characters.

7. Do not break within emoji modifier sequences or emoji zwj sequences. That
   is, do not break between characters with the Extended_Pictographic property.
   Extend and ZWJ characters are allowed between the characters; this cannot be
   represented in this table, the code has to deal with it.

8. Do not break within emoji flag sequences. That is, do not break between
   regional indicator (RI) symbols if there are an odd number of RI characters
   before the break point. This table encodes "join RI characters"; the code
   has to deal with checking for previous adjoining RIs.

9. Otherwise, break everywhere.
*/

#define ESZ (1<<ucp_gbExtend)|(1<<ucp_gbSpacingMark)|(1<<ucp_gbZWJ)

const uint32_t PRIV(ucp_gbtable)[] = {
   (1u<<ucp_gbLF),                                      /*  0 CR */
   0,                                                   /*  1 LF */
   0,                                                   /*  2 Control */
   ESZ,                                                 /*  3 Extend */
   ESZ|(1u<<ucp_gbPrepend)|                             /*  4 Prepend */
       (1u<<ucp_gbL)|(1u<<ucp_gbV)|(1u<<ucp_gbT)|
       (1u<<ucp_gbLV)|(1u<<ucp_gbLVT)|(1u<<ucp_gbOther)|
       (1u<<ucp_gbRegionalIndicator),
   ESZ,                                                 /*  5 SpacingMark */
   ESZ|(1u<<ucp_gbL)|(1u<<ucp_gbV)|(1u<<ucp_gbLV)|      /*  6 L */
       (1u<<ucp_gbLVT),
   ESZ|(1u<<ucp_gbV)|(1u<<ucp_gbT),                     /*  7 V */
   ESZ|(1u<<ucp_gbT),                                   /*  8 T */
   ESZ|(1u<<ucp_gbV)|(1u<<ucp_gbT),                     /*  9 LV */
   ESZ|(1u<<ucp_gbT),                                   /* 10 LVT */
   (1u<<ucp_gbRegionalIndicator),                       /* 11 RegionalIndicator */
   ESZ,                                                 /* 12 Other */
   ESZ,                                                 /* 13 ZWJ */
   ESZ|(1u<<ucp_gbExtended_Pictographic)                /* 14 Extended Pictographic */
};

#undef ESZ

#ifdef SUPPORT_JIT
/* This table reverses PRIV(ucp_gentype). We can save the cost
of a memory load. */

const int PRIV(ucp_typerange)[] = {
  ucp_Cc, ucp_Cs,
  ucp_Ll, ucp_Lu,
  ucp_Mc, ucp_Mn,
  ucp_Nd, ucp_No,
  ucp_Pc, ucp_Ps,
  ucp_Sc, ucp_So,
  ucp_Zl, ucp_Zs,
};
#endif /* SUPPORT_JIT */

/* The PRIV(utt)[] table below translates Unicode property names into type and
code values. It is searched by binary chop, so must be in collating sequence of
name. Originally, the table contained pointers to the name strings in the first
field of each entry. However, that leads to a large number of relocations when
a shared library is dynamically loaded. A significant reduction is made by
putting all the names into a single, large string and then using offsets in the
table itself. Maintenance is more error-prone, but frequent changes to this
data are unlikely.

July 2008: There is now a script called maint/GenerateUtt.py that can be used
to generate this data automatically instead of maintaining it by hand.

The script was updated in March 2009 to generate a new EBCDIC-compliant
version. Like all other character and string literals that are compared against
the regular expression pattern, we must use STR_ macros instead of literal
strings to make sure that UTF-8 support works on EBCDIC platforms. 

December 2021: the script now ensures that all letters are lower cased, and
that underscores are removed, in accordance with the "loose matching" rules
that Unicode advises and Perl uses. */

#define STRING_adlam0 STR_a STR_d STR_l STR_a STR_m "\0"
#define STRING_ahom0 STR_a STR_h STR_o STR_m "\0"
#define STRING_anatolianhieroglyphs0 STR_a STR_n STR_a STR_t STR_o STR_l STR_i STR_a STR_n STR_h STR_i STR_e STR_r STR_o STR_g STR_l STR_y STR_p STR_h STR_s "\0"
#define STRING_any0 STR_a STR_n STR_y "\0"
#define STRING_arabic0 STR_a STR_r STR_a STR_b STR_i STR_c "\0"
#define STRING_armenian0 STR_a STR_r STR_m STR_e STR_n STR_i STR_a STR_n "\0"
#define STRING_avestan0 STR_a STR_v STR_e STR_s STR_t STR_a STR_n "\0"
#define STRING_balinese0 STR_b STR_a STR_l STR_i STR_n STR_e STR_s STR_e "\0"
#define STRING_bamum0 STR_b STR_a STR_m STR_u STR_m "\0"
#define STRING_bassavah0 STR_b STR_a STR_s STR_s STR_a STR_v STR_a STR_h "\0"
#define STRING_batak0 STR_b STR_a STR_t STR_a STR_k "\0"
#define STRING_bengali0 STR_b STR_e STR_n STR_g STR_a STR_l STR_i "\0"
#define STRING_bhaiksuki0 STR_b STR_h STR_a STR_i STR_k STR_s STR_u STR_k STR_i "\0"
#define STRING_bidial0 STR_b STR_i STR_d STR_i STR_a STR_l "\0"
#define STRING_bidian0 STR_b STR_i STR_d STR_i STR_a STR_n "\0"
#define STRING_bidib0 STR_b STR_i STR_d STR_i STR_b "\0"
#define STRING_bidibn0 STR_b STR_i STR_d STR_i STR_b STR_n "\0"
#define STRING_bidic0 STR_b STR_i STR_d STR_i STR_c "\0"
#define STRING_bidicontrol0 STR_b STR_i STR_d STR_i STR_c STR_o STR_n STR_t STR_r STR_o STR_l "\0"
#define STRING_bidics0 STR_b STR_i STR_d STR_i STR_c STR_s "\0"
#define STRING_bidien0 STR_b STR_i STR_d STR_i STR_e STR_n "\0"
#define STRING_bidies0 STR_b STR_i STR_d STR_i STR_e STR_s "\0"
#define STRING_bidiet0 STR_b STR_i STR_d STR_i STR_e STR_t "\0"
#define STRING_bidifsi0 STR_b STR_i STR_d STR_i STR_f STR_s STR_i "\0"
#define STRING_bidil0 STR_b STR_i STR_d STR_i STR_l "\0"
#define STRING_bidilre0 STR_b STR_i STR_d STR_i STR_l STR_r STR_e "\0"
#define STRING_bidilri0 STR_b STR_i STR_d STR_i STR_l STR_r STR_i "\0"
#define STRING_bidilro0 STR_b STR_i STR_d STR_i STR_l STR_r STR_o "\0"
#define STRING_bidinsm0 STR_b STR_i STR_d STR_i STR_n STR_s STR_m "\0"
#define STRING_bidion0 STR_b STR_i STR_d STR_i STR_o STR_n "\0"
#define STRING_bidipdf0 STR_b STR_i STR_d STR_i STR_p STR_d STR_f "\0"
#define STRING_bidipdi0 STR_b STR_i STR_d STR_i STR_p STR_d STR_i "\0"
#define STRING_bidir0 STR_b STR_i STR_d STR_i STR_r "\0"
#define STRING_bidirle0 STR_b STR_i STR_d STR_i STR_r STR_l STR_e "\0"
#define STRING_bidirli0 STR_b STR_i STR_d STR_i STR_r STR_l STR_i "\0"
#define STRING_bidirlo0 STR_b STR_i STR_d STR_i STR_r STR_l STR_o "\0"
#define STRING_bidis0 STR_b STR_i STR_d STR_i STR_s "\0"
#define STRING_bidiws0 STR_b STR_i STR_d STR_i STR_w STR_s "\0"
#define STRING_bopomofo0 STR_b STR_o STR_p STR_o STR_m STR_o STR_f STR_o "\0"
#define STRING_brahmi0 STR_b STR_r STR_a STR_h STR_m STR_i "\0"
#define STRING_braille0 STR_b STR_r STR_a STR_i STR_l STR_l STR_e "\0"
#define STRING_buginese0 STR_b STR_u STR_g STR_i STR_n STR_e STR_s STR_e "\0"
#define STRING_buhid0 STR_b STR_u STR_h STR_i STR_d "\0"
#define STRING_c0 STR_c "\0"
#define STRING_canadianaboriginal0 STR_c STR_a STR_n STR_a STR_d STR_i STR_a STR_n STR_a STR_b STR_o STR_r STR_i STR_g STR_i STR_n STR_a STR_l "\0"
#define STRING_carian0 STR_c STR_a STR_r STR_i STR_a STR_n "\0"
#define STRING_caucasianalbanian0 STR_c STR_a STR_u STR_c STR_a STR_s STR_i STR_a STR_n STR_a STR_l STR_b STR_a STR_n STR_i STR_a STR_n "\0"
#define STRING_cc0 STR_c STR_c "\0"
#define STRING_cf0 STR_c STR_f "\0"
#define STRING_chakma0 STR_c STR_h STR_a STR_k STR_m STR_a "\0"
#define STRING_cham0 STR_c STR_h STR_a STR_m "\0"
#define STRING_cherokee0 STR_c STR_h STR_e STR_r STR_o STR_k STR_e STR_e "\0"
#define STRING_chorasmian0 STR_c STR_h STR_o STR_r STR_a STR_s STR_m STR_i STR_a STR_n "\0"
#define STRING_cn0 STR_c STR_n "\0"
#define STRING_co0 STR_c STR_o "\0"
#define STRING_common0 STR_c STR_o STR_m STR_m STR_o STR_n "\0"
#define STRING_coptic0 STR_c STR_o STR_p STR_t STR_i STR_c "\0"
#define STRING_cs0 STR_c STR_s "\0"
#define STRING_cuneiform0 STR_c STR_u STR_n STR_e STR_i STR_f STR_o STR_r STR_m "\0"
#define STRING_cypriot0 STR_c STR_y STR_p STR_r STR_i STR_o STR_t "\0"
#define STRING_cyprominoan0 STR_c STR_y STR_p STR_r STR_o STR_m STR_i STR_n STR_o STR_a STR_n "\0"
#define STRING_cyrillic0 STR_c STR_y STR_r STR_i STR_l STR_l STR_i STR_c "\0"
#define STRING_deseret0 STR_d STR_e STR_s STR_e STR_r STR_e STR_t "\0"
#define STRING_devanagari0 STR_d STR_e STR_v STR_a STR_n STR_a STR_g STR_a STR_r STR_i "\0"
#define STRING_divesakuru0 STR_d STR_i STR_v STR_e STR_s STR_a STR_k STR_u STR_r STR_u "\0"
#define STRING_dogra0 STR_d STR_o STR_g STR_r STR_a "\0"
#define STRING_duployan0 STR_d STR_u STR_p STR_l STR_o STR_y STR_a STR_n "\0"
#define STRING_egyptianhieroglyphs0 STR_e STR_g STR_y STR_p STR_t STR_i STR_a STR_n STR_h STR_i STR_e STR_r STR_o STR_g STR_l STR_y STR_p STR_h STR_s "\0"
#define STRING_elbasan0 STR_e STR_l STR_b STR_a STR_s STR_a STR_n "\0"
#define STRING_elymaic0 STR_e STR_l STR_y STR_m STR_a STR_i STR_c "\0"
#define STRING_ethiopic0 STR_e STR_t STR_h STR_i STR_o STR_p STR_i STR_c "\0"
#define STRING_georgian0 STR_g STR_e STR_o STR_r STR_g STR_i STR_a STR_n "\0"
#define STRING_glagolitic0 STR_g STR_l STR_a STR_g STR_o STR_l STR_i STR_t STR_i STR_c "\0"
#define STRING_gothic0 STR_g STR_o STR_t STR_h STR_i STR_c "\0"
#define STRING_grantha0 STR_g STR_r STR_a STR_n STR_t STR_h STR_a "\0"
#define STRING_greek0 STR_g STR_r STR_e STR_e STR_k "\0"
#define STRING_gujarati0 STR_g STR_u STR_j STR_a STR_r STR_a STR_t STR_i "\0"
#define STRING_gunjalagondi0 STR_g STR_u STR_n STR_j STR_a STR_l STR_a STR_g STR_o STR_n STR_d STR_i "\0"
#define STRING_gurmukhi0 STR_g STR_u STR_r STR_m STR_u STR_k STR_h STR_i "\0"
#define STRING_han0 STR_h STR_a STR_n "\0"
#define STRING_hangul0 STR_h STR_a STR_n STR_g STR_u STR_l "\0"
#define STRING_hanifirohingya0 STR_h STR_a STR_n STR_i STR_f STR_i STR_r STR_o STR_h STR_i STR_n STR_g STR_y STR_a "\0"
#define STRING_hanunoo0 STR_h STR_a STR_n STR_u STR_n STR_o STR_o "\0"
#define STRING_hatran0 STR_h STR_a STR_t STR_r STR_a STR_n "\0"
#define STRING_hebrew0 STR_h STR_e STR_b STR_r STR_e STR_w "\0"
#define STRING_hiragana0 STR_h STR_i STR_r STR_a STR_g STR_a STR_n STR_a "\0"
#define STRING_imperialaramaic0 STR_i STR_m STR_p STR_e STR_r STR_i STR_a STR_l STR_a STR_r STR_a STR_m STR_a STR_i STR_c "\0"
#define STRING_inherited0 STR_i STR_n STR_h STR_e STR_r STR_i STR_t STR_e STR_d "\0"
#define STRING_inscriptionalpahlavi0 STR_i STR_n STR_s STR_c STR_r STR_i STR_p STR_t STR_i STR_o STR_n STR_a STR_l STR_p STR_a STR_h STR_l STR_a STR_v STR_i "\0"
#define STRING_inscriptionalparthian0 STR_i STR_n STR_s STR_c STR_r STR_i STR_p STR_t STR_i STR_o STR_n STR_a STR_l STR_p STR_a STR_r STR_t STR_h STR_i STR_a STR_n "\0"
#define STRING_javanese0 STR_j STR_a STR_v STR_a STR_n STR_e STR_s STR_e "\0"
#define STRING_kaithi0 STR_k STR_a STR_i STR_t STR_h STR_i "\0"
#define STRING_kannada0 STR_k STR_a STR_n STR_n STR_a STR_d STR_a "\0"
#define STRING_katakana0 STR_k STR_a STR_t STR_a STR_k STR_a STR_n STR_a "\0"
#define STRING_kayahli0 STR_k STR_a STR_y STR_a STR_h STR_l STR_i "\0"
#define STRING_kharoshthi0 STR_k STR_h STR_a STR_r STR_o STR_s STR_h STR_t STR_h STR_i "\0"
#define STRING_khitansmallscript0 STR_k STR_h STR_i STR_t STR_a STR_n STR_s STR_m STR_a STR_l STR_l STR_s STR_c STR_r STR_i STR_p STR_t "\0"
#define STRING_khmer0 STR_k STR_h STR_m STR_e STR_r "\0"
#define STRING_khojki0 STR_k STR_h STR_o STR_j STR_k STR_i "\0"
#define STRING_khudawadi0 STR_k STR_h STR_u STR_d STR_a STR_w STR_a STR_d STR_i "\0"
#define STRING_l0 STR_l "\0"
#define STRING_l_AMPERSAND0 STR_l STR_AMPERSAND "\0"
#define STRING_lao0 STR_l STR_a STR_o "\0"
#define STRING_latin0 STR_l STR_a STR_t STR_i STR_n "\0"
#define STRING_lc0 STR_l STR_c "\0"
#define STRING_lepcha0 STR_l STR_e STR_p STR_c STR_h STR_a "\0"
#define STRING_limbu0 STR_l STR_i STR_m STR_b STR_u "\0"
#define STRING_lineara0 STR_l STR_i STR_n STR_e STR_a STR_r STR_a "\0"
#define STRING_linearb0 STR_l STR_i STR_n STR_e STR_a STR_r STR_b "\0"
#define STRING_lisu0 STR_l STR_i STR_s STR_u "\0"
#define STRING_ll0 STR_l STR_l "\0"
#define STRING_lm0 STR_l STR_m "\0"
#define STRING_lo0 STR_l STR_o "\0"
#define STRING_lt0 STR_l STR_t "\0"
#define STRING_lu0 STR_l STR_u "\0"
#define STRING_lycian0 STR_l STR_y STR_c STR_i STR_a STR_n "\0"
#define STRING_lydian0 STR_l STR_y STR_d STR_i STR_a STR_n "\0"
#define STRING_m0 STR_m "\0"
#define STRING_mahajani0 STR_m STR_a STR_h STR_a STR_j STR_a STR_n STR_i "\0"
#define STRING_makasar0 STR_m STR_a STR_k STR_a STR_s STR_a STR_r "\0"
#define STRING_malayalam0 STR_m STR_a STR_l STR_a STR_y STR_a STR_l STR_a STR_m "\0"
#define STRING_mandaic0 STR_m STR_a STR_n STR_d STR_a STR_i STR_c "\0"
#define STRING_manichaean0 STR_m STR_a STR_n STR_i STR_c STR_h STR_a STR_e STR_a STR_n "\0"
#define STRING_marchen0 STR_m STR_a STR_r STR_c STR_h STR_e STR_n "\0"
#define STRING_masaramgondi0 STR_m STR_a STR_s STR_a STR_r STR_a STR_m STR_g STR_o STR_n STR_d STR_i "\0"
#define STRING_mc0 STR_m STR_c "\0"
#define STRING_me0 STR_m STR_e "\0"
#define STRING_medefaidrin0 STR_m STR_e STR_d STR_e STR_f STR_a STR_i STR_d STR_r STR_i STR_n "\0"
#define STRING_meeteimayek0 STR_m STR_e STR_e STR_t STR_e STR_i STR_m STR_a STR_y STR_e STR_k "\0"
#define STRING_mendekikakui0 STR_m STR_e STR_n STR_d STR_e STR_k STR_i STR_k STR_a STR_k STR_u STR_i "\0"
#define STRING_meroiticcursive0 STR_m STR_e STR_r STR_o STR_i STR_t STR_i STR_c STR_c STR_u STR_r STR_s STR_i STR_v STR_e "\0"
#define STRING_meroitichieroglyphs0 STR_m STR_e STR_r STR_o STR_i STR_t STR_i STR_c STR_h STR_i STR_e STR_r STR_o STR_g STR_l STR_y STR_p STR_h STR_s "\0"
#define STRING_miao0 STR_m STR_i STR_a STR_o "\0"
#define STRING_mn0 STR_m STR_n "\0"
#define STRING_modi0 STR_m STR_o STR_d STR_i "\0"
#define STRING_mongolian0 STR_m STR_o STR_n STR_g STR_o STR_l STR_i STR_a STR_n "\0"
#define STRING_mro0 STR_m STR_r STR_o "\0"
#define STRING_multani0 STR_m STR_u STR_l STR_t STR_a STR_n STR_i "\0"
#define STRING_myanmar0 STR_m STR_y STR_a STR_n STR_m STR_a STR_r "\0"
#define STRING_n0 STR_n "\0"
#define STRING_nabataean0 STR_n STR_a STR_b STR_a STR_t STR_a STR_e STR_a STR_n "\0"
#define STRING_nandinagari0 STR_n STR_a STR_n STR_d STR_i STR_n STR_a STR_g STR_a STR_r STR_i "\0"
#define STRING_nd0 STR_n STR_d "\0"
#define STRING_newa0 STR_n STR_e STR_w STR_a "\0"
#define STRING_newtailue0 STR_n STR_e STR_w STR_t STR_a STR_i STR_l STR_u STR_e "\0"
#define STRING_nko0 STR_n STR_k STR_o "\0"
#define STRING_nl0 STR_n STR_l "\0"
#define STRING_no0 STR_n STR_o "\0"
#define STRING_nushu0 STR_n STR_u STR_s STR_h STR_u "\0"
#define STRING_nyiakengpuachuehmong0 STR_n STR_y STR_i STR_a STR_k STR_e STR_n STR_g STR_p STR_u STR_a STR_c STR_h STR_u STR_e STR_h STR_m STR_o STR_n STR_g "\0"
#define STRING_ogham0 STR_o STR_g STR_h STR_a STR_m "\0"
#define STRING_olchiki0 STR_o STR_l STR_c STR_h STR_i STR_k STR_i "\0"
#define STRING_oldhungarian0 STR_o STR_l STR_d STR_h STR_u STR_n STR_g STR_a STR_r STR_i STR_a STR_n "\0"
#define STRING_olditalic0 STR_o STR_l STR_d STR_i STR_t STR_a STR_l STR_i STR_c "\0"
#define STRING_oldnortharabian0 STR_o STR_l STR_d STR_n STR_o STR_r STR_t STR_h STR_a STR_r STR_a STR_b STR_i STR_a STR_n "\0"
#define STRING_oldpermic0 STR_o STR_l STR_d STR_p STR_e STR_r STR_m STR_i STR_c "\0"
#define STRING_oldpersian0 STR_o STR_l STR_d STR_p STR_e STR_r STR_s STR_i STR_a STR_n "\0"
#define STRING_oldsogdian0 STR_o STR_l STR_d STR_s STR_o STR_g STR_d STR_i STR_a STR_n "\0"
#define STRING_oldsoutharabian0 STR_o STR_l STR_d STR_s STR_o STR_u STR_t STR_h STR_a STR_r STR_a STR_b STR_i STR_a STR_n "\0"
#define STRING_oldturkic0 STR_o STR_l STR_d STR_t STR_u STR_r STR_k STR_i STR_c "\0"
#define STRING_olduyghur0 STR_o STR_l STR_d STR_u STR_y STR_g STR_h STR_u STR_r "\0"
#define STRING_oriya0 STR_o STR_r STR_i STR_y STR_a "\0"
#define STRING_osage0 STR_o STR_s STR_a STR_g STR_e "\0"
#define STRING_osmanya0 STR_o STR_s STR_m STR_a STR_n STR_y STR_a "\0"
#define STRING_p0 STR_p "\0"
#define STRING_pahawhhmong0 STR_p STR_a STR_h STR_a STR_w STR_h STR_h STR_m STR_o STR_n STR_g "\0"
#define STRING_palmyrene0 STR_p STR_a STR_l STR_m STR_y STR_r STR_e STR_n STR_e "\0"
#define STRING_paucinhau0 STR_p STR_a STR_u STR_c STR_i STR_n STR_h STR_a STR_u "\0"
#define STRING_pc0 STR_p STR_c "\0"
#define STRING_pd0 STR_p STR_d "\0"
#define STRING_pe0 STR_p STR_e "\0"
#define STRING_pf0 STR_p STR_f "\0"
#define STRING_phagspa0 STR_p STR_h STR_a STR_g STR_s STR_p STR_a "\0"
#define STRING_phoenician0 STR_p STR_h STR_o STR_e STR_n STR_i STR_c STR_i STR_a STR_n "\0"
#define STRING_pi0 STR_p STR_i "\0"
#define STRING_po0 STR_p STR_o "\0"
#define STRING_ps0 STR_p STR_s "\0"
#define STRING_psalterpahlavi0 STR_p STR_s STR_a STR_l STR_t STR_e STR_r STR_p STR_a STR_h STR_l STR_a STR_v STR_i "\0"
#define STRING_rejang0 STR_r STR_e STR_j STR_a STR_n STR_g "\0"
#define STRING_runic0 STR_r STR_u STR_n STR_i STR_c "\0"
#define STRING_s0 STR_s "\0"
#define STRING_samaritan0 STR_s STR_a STR_m STR_a STR_r STR_i STR_t STR_a STR_n "\0"
#define STRING_saurashtra0 STR_s STR_a STR_u STR_r STR_a STR_s STR_h STR_t STR_r STR_a "\0"
#define STRING_sc0 STR_s STR_c "\0"
#define STRING_sharada0 STR_s STR_h STR_a STR_r STR_a STR_d STR_a "\0"
#define STRING_shavian0 STR_s STR_h STR_a STR_v STR_i STR_a STR_n "\0"
#define STRING_siddham0 STR_s STR_i STR_d STR_d STR_h STR_a STR_m "\0"
#define STRING_signwriting0 STR_s STR_i STR_g STR_n STR_w STR_r STR_i STR_t STR_i STR_n STR_g "\0"
#define STRING_sinhala0 STR_s STR_i STR_n STR_h STR_a STR_l STR_a "\0"
#define STRING_sk0 STR_s STR_k "\0"
#define STRING_sm0 STR_s STR_m "\0"
#define STRING_so0 STR_s STR_o "\0"
#define STRING_sogdian0 STR_s STR_o STR_g STR_d STR_i STR_a STR_n "\0"
#define STRING_sorasompeng0 STR_s STR_o STR_r STR_a STR_s STR_o STR_m STR_p STR_e STR_n STR_g "\0"
#define STRING_soyombo0 STR_s STR_o STR_y STR_o STR_m STR_b STR_o "\0"
#define STRING_sundanese0 STR_s STR_u STR_n STR_d STR_a STR_n STR_e STR_s STR_e "\0"
#define STRING_sylotinagri0 STR_s STR_y STR_l STR_o STR_t STR_i STR_n STR_a STR_g STR_r STR_i "\0"
#define STRING_syriac0 STR_s STR_y STR_r STR_i STR_a STR_c "\0"
#define STRING_tagalog0 STR_t STR_a STR_g STR_a STR_l STR_o STR_g "\0"
#define STRING_tagbanwa0 STR_t STR_a STR_g STR_b STR_a STR_n STR_w STR_a "\0"
#define STRING_taile0 STR_t STR_a STR_i STR_l STR_e "\0"
#define STRING_taitham0 STR_t STR_a STR_i STR_t STR_h STR_a STR_m "\0"
#define STRING_taiviet0 STR_t STR_a STR_i STR_v STR_i STR_e STR_t "\0"
#define STRING_takri0 STR_t STR_a STR_k STR_r STR_i "\0"
#define STRING_tamil0 STR_t STR_a STR_m STR_i STR_l "\0"
#define STRING_tangsa0 STR_t STR_a STR_n STR_g STR_s STR_a "\0"
#define STRING_tangut0 STR_t STR_a STR_n STR_g STR_u STR_t "\0"
#define STRING_telugu0 STR_t STR_e STR_l STR_u STR_g STR_u "\0"
#define STRING_thaana0 STR_t STR_h STR_a STR_a STR_n STR_a "\0"
#define STRING_thai0 STR_t STR_h STR_a STR_i "\0"
#define STRING_tibetan0 STR_t STR_i STR_b STR_e STR_t STR_a STR_n "\0"
#define STRING_tifinagh0 STR_t STR_i STR_f STR_i STR_n STR_a STR_g STR_h "\0"
#define STRING_tirhuta0 STR_t STR_i STR_r STR_h STR_u STR_t STR_a "\0"
#define STRING_toto0 STR_t STR_o STR_t STR_o "\0"
#define STRING_ugaritic0 STR_u STR_g STR_a STR_r STR_i STR_t STR_i STR_c "\0"
#define STRING_unknown0 STR_u STR_n STR_k STR_n STR_o STR_w STR_n "\0"
#define STRING_vai0 STR_v STR_a STR_i "\0"
#define STRING_vithkuqi0 STR_v STR_i STR_t STR_h STR_k STR_u STR_q STR_i "\0"
#define STRING_wancho0 STR_w STR_a STR_n STR_c STR_h STR_o "\0"
#define STRING_warangciti0 STR_w STR_a STR_r STR_a STR_n STR_g STR_c STR_i STR_t STR_i "\0"
#define STRING_xan0 STR_x STR_a STR_n "\0"
#define STRING_xps0 STR_x STR_p STR_s "\0"
#define STRING_xsp0 STR_x STR_s STR_p "\0"
#define STRING_xuc0 STR_x STR_u STR_c "\0"
#define STRING_xwd0 STR_x STR_w STR_d "\0"
#define STRING_yezidi0 STR_y STR_e STR_z STR_i STR_d STR_i "\0"
#define STRING_yi0 STR_y STR_i "\0"
#define STRING_z0 STR_z "\0"
#define STRING_zanabazarsquare0 STR_z STR_a STR_n STR_a STR_b STR_a STR_z STR_a STR_r STR_s STR_q STR_u STR_a STR_r STR_e "\0"
#define STRING_zl0 STR_z STR_l "\0"
#define STRING_zp0 STR_z STR_p "\0"
#define STRING_zs0 STR_z STR_s "\0"

const char PRIV(utt_names)[] =
  STRING_adlam0
  STRING_ahom0
  STRING_anatolianhieroglyphs0
  STRING_any0
  STRING_arabic0
  STRING_armenian0
  STRING_avestan0
  STRING_balinese0
  STRING_bamum0
  STRING_bassavah0
  STRING_batak0
  STRING_bengali0
  STRING_bhaiksuki0
  STRING_bidial0
  STRING_bidian0
  STRING_bidib0
  STRING_bidibn0
  STRING_bidic0
  STRING_bidicontrol0
  STRING_bidics0
  STRING_bidien0
  STRING_bidies0
  STRING_bidiet0
  STRING_bidifsi0
  STRING_bidil0
  STRING_bidilre0
  STRING_bidilri0
  STRING_bidilro0
  STRING_bidinsm0
  STRING_bidion0
  STRING_bidipdf0
  STRING_bidipdi0
  STRING_bidir0
  STRING_bidirle0
  STRING_bidirli0
  STRING_bidirlo0
  STRING_bidis0
  STRING_bidiws0
  STRING_bopomofo0
  STRING_brahmi0
  STRING_braille0
  STRING_buginese0
  STRING_buhid0
  STRING_c0
  STRING_canadianaboriginal0
  STRING_carian0
  STRING_caucasianalbanian0
  STRING_cc0
  STRING_cf0
  STRING_chakma0
  STRING_cham0
  STRING_cherokee0
  STRING_chorasmian0
  STRING_cn0
  STRING_co0
  STRING_common0
  STRING_coptic0
  STRING_cs0
  STRING_cuneiform0
  STRING_cypriot0
  STRING_cyprominoan0
  STRING_cyrillic0
  STRING_deseret0
  STRING_devanagari0
  STRING_divesakuru0
  STRING_dogra0
  STRING_duployan0
  STRING_egyptianhieroglyphs0
  STRING_elbasan0
  STRING_elymaic0
  STRING_ethiopic0
  STRING_georgian0
  STRING_glagolitic0
  STRING_gothic0
  STRING_grantha0
  STRING_greek0
  STRING_gujarati0
  STRING_gunjalagondi0
  STRING_gurmukhi0
  STRING_han0
  STRING_hangul0
  STRING_hanifirohingya0
  STRING_hanunoo0
  STRING_hatran0
  STRING_hebrew0
  STRING_hiragana0
  STRING_imperialaramaic0
  STRING_inherited0
  STRING_inscriptionalpahlavi0
  STRING_inscriptionalparthian0
  STRING_javanese0
  STRING_kaithi0
  STRING_kannada0
  STRING_katakana0
  STRING_kayahli0
  STRING_kharoshthi0
  STRING_khitansmallscript0
  STRING_khmer0
  STRING_khojki0
  STRING_khudawadi0
  STRING_l0
  STRING_l_AMPERSAND0
  STRING_lao0
  STRING_latin0
  STRING_lc0
  STRING_lepcha0
  STRING_limbu0
  STRING_lineara0
  STRING_linearb0
  STRING_lisu0
  STRING_ll0
  STRING_lm0
  STRING_lo0
  STRING_lt0
  STRING_lu0
  STRING_lycian0
  STRING_lydian0
  STRING_m0
  STRING_mahajani0
  STRING_makasar0
  STRING_malayalam0
  STRING_mandaic0
  STRING_manichaean0
  STRING_marchen0
  STRING_masaramgondi0
  STRING_mc0
  STRING_me0
  STRING_medefaidrin0
  STRING_meeteimayek0
  STRING_mendekikakui0
  STRING_meroiticcursive0
  STRING_meroitichieroglyphs0
  STRING_miao0
  STRING_mn0
  STRING_modi0
  STRING_mongolian0
  STRING_mro0
  STRING_multani0
  STRING_myanmar0
  STRING_n0
  STRING_nabataean0
  STRING_nandinagari0
  STRING_nd0
  STRING_newa0
  STRING_newtailue0
  STRING_nko0
  STRING_nl0
  STRING_no0
  STRING_nushu0
  STRING_nyiakengpuachuehmong0
  STRING_ogham0
  STRING_olchiki0
  STRING_oldhungarian0
  STRING_olditalic0
  STRING_oldnortharabian0
  STRING_oldpermic0
  STRING_oldpersian0
  STRING_oldsogdian0
  STRING_oldsoutharabian0
  STRING_oldturkic0
  STRING_olduyghur0
  STRING_oriya0
  STRING_osage0
  STRING_osmanya0
  STRING_p0
  STRING_pahawhhmong0
  STRING_palmyrene0
  STRING_paucinhau0
  STRING_pc0
  STRING_pd0
  STRING_pe0
  STRING_pf0
  STRING_phagspa0
  STRING_phoenician0
  STRING_pi0
  STRING_po0
  STRING_ps0
  STRING_psalterpahlavi0
  STRING_rejang0
  STRING_runic0
  STRING_s0
  STRING_samaritan0
  STRING_saurashtra0
  STRING_sc0
  STRING_sharada0
  STRING_shavian0
  STRING_siddham0
  STRING_signwriting0
  STRING_sinhala0
  STRING_sk0
  STRING_sm0
  STRING_so0
  STRING_sogdian0
  STRING_sorasompeng0
  STRING_soyombo0
  STRING_sundanese0
  STRING_sylotinagri0
  STRING_syriac0
  STRING_tagalog0
  STRING_tagbanwa0
  STRING_taile0
  STRING_taitham0
  STRING_taiviet0
  STRING_takri0
  STRING_tamil0
  STRING_tangsa0
  STRING_tangut0
  STRING_telugu0
  STRING_thaana0
  STRING_thai0
  STRING_tibetan0
  STRING_tifinagh0
  STRING_tirhuta0
  STRING_toto0
  STRING_ugaritic0
  STRING_unknown0
  STRING_vai0
  STRING_vithkuqi0
  STRING_wancho0
  STRING_warangciti0
  STRING_xan0
  STRING_xps0
  STRING_xsp0
  STRING_xuc0
  STRING_xwd0
  STRING_yezidi0
  STRING_yi0
  STRING_z0
  STRING_zanabazarsquare0
  STRING_zl0
  STRING_zp0
  STRING_zs0;

const ucp_type_table PRIV(utt)[] = {
  {   0, PT_SC, ucp_Adlam },
  {   6, PT_SC, ucp_Ahom },
  {  11, PT_SC, ucp_Anatolian_Hieroglyphs },
  {  32, PT_ANY, 0 },
  {  36, PT_SC, ucp_Arabic },
  {  43, PT_SC, ucp_Armenian },
  {  52, PT_SC, ucp_Avestan },
  {  60, PT_SC, ucp_Balinese },
  {  69, PT_SC, ucp_Bamum },
  {  75, PT_SC, ucp_Bassa_Vah },
  {  84, PT_SC, ucp_Batak },
  {  90, PT_SC, ucp_Bengali },
  {  98, PT_SC, ucp_Bhaiksuki },
  { 108, PT_BIDICL, ucp_bidiAL },
  { 115, PT_BIDICL, ucp_bidiAN },
  { 122, PT_BIDICL, ucp_bidiB },
  { 128, PT_BIDICL, ucp_bidiBN },
  { 135, PT_BIDICO, 0 },
  { 141, PT_BIDICO, 0 },
  { 153, PT_BIDICL, ucp_bidiCS },
  { 160, PT_BIDICL, ucp_bidiEN },
  { 167, PT_BIDICL, ucp_bidiES },
  { 174, PT_BIDICL, ucp_bidiET },
  { 181, PT_BIDICL, ucp_bidiFSI },
  { 189, PT_BIDICL, ucp_bidiL },
  { 195, PT_BIDICL, ucp_bidiLRE },
  { 203, PT_BIDICL, ucp_bidiLRI },
  { 211, PT_BIDICL, ucp_bidiLRO },
  { 219, PT_BIDICL, ucp_bidiNSM },
  { 227, PT_BIDICL, ucp_bidiON },
  { 234, PT_BIDICL, ucp_bidiPDF },
  { 242, PT_BIDICL, ucp_bidiPDI },
  { 250, PT_BIDICL, ucp_bidiR },
  { 256, PT_BIDICL, ucp_bidiRLE },
  { 264, PT_BIDICL, ucp_bidiRLI },
  { 272, PT_BIDICL, ucp_bidiRLO },
  { 280, PT_BIDICL, ucp_bidiS },
  { 286, PT_BIDICL, ucp_bidiWS },
  { 293, PT_SC, ucp_Bopomofo },
  { 302, PT_SC, ucp_Brahmi },
  { 309, PT_SC, ucp_Braille },
  { 317, PT_SC, ucp_Buginese },
  { 326, PT_SC, ucp_Buhid },
  { 332, PT_GC, ucp_C },
  { 334, PT_SC, ucp_Canadian_Aboriginal },
  { 353, PT_SC, ucp_Carian },
  { 360, PT_SC, ucp_Caucasian_Albanian },
  { 378, PT_PC, ucp_Cc },
  { 381, PT_PC, ucp_Cf },
  { 384, PT_SC, ucp_Chakma },
  { 391, PT_SC, ucp_Cham },
  { 396, PT_SC, ucp_Cherokee },
  { 405, PT_SC, ucp_Chorasmian },
  { 416, PT_PC, ucp_Cn },
  { 419, PT_PC, ucp_Co },
  { 422, PT_SC, ucp_Common },
  { 429, PT_SC, ucp_Coptic },
  { 436, PT_PC, ucp_Cs },
  { 439, PT_SC, ucp_Cuneiform },
  { 449, PT_SC, ucp_Cypriot },
  { 457, PT_SC, ucp_Cypro_Minoan },
  { 469, PT_SC, ucp_Cyrillic },
  { 478, PT_SC, ucp_Deseret },
  { 486, PT_SC, ucp_Devanagari },
  { 497, PT_SC, ucp_Dives_Akuru },
  { 508, PT_SC, ucp_Dogra },
  { 514, PT_SC, ucp_Duployan },
  { 523, PT_SC, ucp_Egyptian_Hieroglyphs },
  { 543, PT_SC, ucp_Elbasan },
  { 551, PT_SC, ucp_Elymaic },
  { 559, PT_SC, ucp_Ethiopic },
  { 568, PT_SC, ucp_Georgian },
  { 577, PT_SC, ucp_Glagolitic },
  { 588, PT_SC, ucp_Gothic },
  { 595, PT_SC, ucp_Grantha },
  { 603, PT_SC, ucp_Greek },
  { 609, PT_SC, ucp_Gujarati },
  { 618, PT_SC, ucp_Gunjala_Gondi },
  { 631, PT_SC, ucp_Gurmukhi },
  { 640, PT_SC, ucp_Han },
  { 644, PT_SC, ucp_Hangul },
  { 651, PT_SC, ucp_Hanifi_Rohingya },
  { 666, PT_SC, ucp_Hanunoo },
  { 674, PT_SC, ucp_Hatran },
  { 681, PT_SC, ucp_Hebrew },
  { 688, PT_SC, ucp_Hiragana },
  { 697, PT_SC, ucp_Imperial_Aramaic },
  { 713, PT_SC, ucp_Inherited },
  { 723, PT_SC, ucp_Inscriptional_Pahlavi },
  { 744, PT_SC, ucp_Inscriptional_Parthian },
  { 766, PT_SC, ucp_Javanese },
  { 775, PT_SC, ucp_Kaithi },
  { 782, PT_SC, ucp_Kannada },
  { 790, PT_SC, ucp_Katakana },
  { 799, PT_SC, ucp_Kayah_Li },
  { 807, PT_SC, ucp_Kharoshthi },
  { 818, PT_SC, ucp_Khitan_Small_Script },
  { 836, PT_SC, ucp_Khmer },
  { 842, PT_SC, ucp_Khojki },
  { 849, PT_SC, ucp_Khudawadi },
  { 859, PT_GC, ucp_L },
  { 861, PT_LAMP, 0 },
  { 864, PT_SC, ucp_Lao },
  { 868, PT_SC, ucp_Latin },
  { 874, PT_LAMP, 0 },
  { 877, PT_SC, ucp_Lepcha },
  { 884, PT_SC, ucp_Limbu },
  { 890, PT_SC, ucp_Linear_A },
  { 898, PT_SC, ucp_Linear_B },
  { 906, PT_SC, ucp_Lisu },
  { 911, PT_PC, ucp_Ll },
  { 914, PT_PC, ucp_Lm },
  { 917, PT_PC, ucp_Lo },
  { 920, PT_PC, ucp_Lt },
  { 923, PT_PC, ucp_Lu },
  { 926, PT_SC, ucp_Lycian },
  { 933, PT_SC, ucp_Lydian },
  { 940, PT_GC, ucp_M },
  { 942, PT_SC, ucp_Mahajani },
  { 951, PT_SC, ucp_Makasar },
  { 959, PT_SC, ucp_Malayalam },
  { 969, PT_SC, ucp_Mandaic },
  { 977, PT_SC, ucp_Manichaean },
  { 988, PT_SC, ucp_Marchen },
  { 996, PT_SC, ucp_Masaram_Gondi },
  { 1009, PT_PC, ucp_Mc },
  { 1012, PT_PC, ucp_Me },
  { 1015, PT_SC, ucp_Medefaidrin },
  { 1027, PT_SC, ucp_Meetei_Mayek },
  { 1039, PT_SC, ucp_Mende_Kikakui },
  { 1052, PT_SC, ucp_Meroitic_Cursive },
  { 1068, PT_SC, ucp_Meroitic_Hieroglyphs },
  { 1088, PT_SC, ucp_Miao },
  { 1093, PT_PC, ucp_Mn },
  { 1096, PT_SC, ucp_Modi },
  { 1101, PT_SC, ucp_Mongolian },
  { 1111, PT_SC, ucp_Mro },
  { 1115, PT_SC, ucp_Multani },
  { 1123, PT_SC, ucp_Myanmar },
  { 1131, PT_GC, ucp_N },
  { 1133, PT_SC, ucp_Nabataean },
  { 1143, PT_SC, ucp_Nandinagari },
  { 1155, PT_PC, ucp_Nd },
  { 1158, PT_SC, ucp_Newa },
  { 1163, PT_SC, ucp_New_Tai_Lue },
  { 1173, PT_SC, ucp_Nko },
  { 1177, PT_PC, ucp_Nl },
  { 1180, PT_PC, ucp_No },
  { 1183, PT_SC, ucp_Nushu },
  { 1189, PT_SC, ucp_Nyiakeng_Puachue_Hmong },
  { 1210, PT_SC, ucp_Ogham },
  { 1216, PT_SC, ucp_Ol_Chiki },
  { 1224, PT_SC, ucp_Old_Hungarian },
  { 1237, PT_SC, ucp_Old_Italic },
  { 1247, PT_SC, ucp_Old_North_Arabian },
  { 1263, PT_SC, ucp_Old_Permic },
  { 1273, PT_SC, ucp_Old_Persian },
  { 1284, PT_SC, ucp_Old_Sogdian },
  { 1295, PT_SC, ucp_Old_South_Arabian },
  { 1311, PT_SC, ucp_Old_Turkic },
  { 1321, PT_SC, ucp_Old_Uyghur },
  { 1331, PT_SC, ucp_Oriya },
  { 1337, PT_SC, ucp_Osage },
  { 1343, PT_SC, ucp_Osmanya },
  { 1351, PT_GC, ucp_P },
  { 1353, PT_SC, ucp_Pahawh_Hmong },
  { 1365, PT_SC, ucp_Palmyrene },
  { 1375, PT_SC, ucp_Pau_Cin_Hau },
  { 1385, PT_PC, ucp_Pc },
  { 1388, PT_PC, ucp_Pd },
  { 1391, PT_PC, ucp_Pe },
  { 1394, PT_PC, ucp_Pf },
  { 1397, PT_SC, ucp_Phags_Pa },
  { 1405, PT_SC, ucp_Phoenician },
  { 1416, PT_PC, ucp_Pi },
  { 1419, PT_PC, ucp_Po },
  { 1422, PT_PC, ucp_Ps },
  { 1425, PT_SC, ucp_Psalter_Pahlavi },
  { 1440, PT_SC, ucp_Rejang },
  { 1447, PT_SC, ucp_Runic },
  { 1453, PT_GC, ucp_S },
  { 1455, PT_SC, ucp_Samaritan },
  { 1465, PT_SC, ucp_Saurashtra },
  { 1476, PT_PC, ucp_Sc },
  { 1479, PT_SC, ucp_Sharada },
  { 1487, PT_SC, ucp_Shavian },
  { 1495, PT_SC, ucp_Siddham },
  { 1503, PT_SC, ucp_SignWriting },
  { 1515, PT_SC, ucp_Sinhala },
  { 1523, PT_PC, ucp_Sk },
  { 1526, PT_PC, ucp_Sm },
  { 1529, PT_PC, ucp_So },
  { 1532, PT_SC, ucp_Sogdian },
  { 1540, PT_SC, ucp_Sora_Sompeng },
  { 1552, PT_SC, ucp_Soyombo },
  { 1560, PT_SC, ucp_Sundanese },
  { 1570, PT_SC, ucp_Syloti_Nagri },
  { 1582, PT_SC, ucp_Syriac },
  { 1589, PT_SC, ucp_Tagalog },
  { 1597, PT_SC, ucp_Tagbanwa },
  { 1606, PT_SC, ucp_Tai_Le },
  { 1612, PT_SC, ucp_Tai_Tham },
  { 1620, PT_SC, ucp_Tai_Viet },
  { 1628, PT_SC, ucp_Takri },
  { 1634, PT_SC, ucp_Tamil },
  { 1640, PT_SC, ucp_Tangsa },
  { 1647, PT_SC, ucp_Tangut },
  { 1654, PT_SC, ucp_Telugu },
  { 1661, PT_SC, ucp_Thaana },
  { 1668, PT_SC, ucp_Thai },
  { 1673, PT_SC, ucp_Tibetan },
  { 1681, PT_SC, ucp_Tifinagh },
  { 1690, PT_SC, ucp_Tirhuta },
  { 1698, PT_SC, ucp_Toto },
  { 1703, PT_SC, ucp_Ugaritic },
  { 1712, PT_SC, ucp_Unknown },
  { 1720, PT_SC, ucp_Vai },
  { 1724, PT_SC, ucp_Vithkuqi },
  { 1733, PT_SC, ucp_Wancho },
  { 1740, PT_SC, ucp_Warang_Citi },
  { 1751, PT_ALNUM, 0 },
  { 1755, PT_PXSPACE, 0 },
  { 1759, PT_SPACE, 0 },
  { 1763, PT_UCNC, 0 },
  { 1767, PT_WORD, 0 },
  { 1771, PT_SC, ucp_Yezidi },
  { 1778, PT_SC, ucp_Yi },
  { 1781, PT_GC, ucp_Z },
  { 1783, PT_SC, ucp_Zanabazar_Square },
  { 1799, PT_PC, ucp_Zl },
  { 1802, PT_PC, ucp_Zp },
  { 1805, PT_PC, ucp_Zs }
};

const size_t PRIV(utt_size) = sizeof(PRIV(utt)) / sizeof(ucp_type_table);

#endif /* SUPPORT_UNICODE */

/* End of pcre2_tables.c */
