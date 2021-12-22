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


/* This module is auto-generated from the Unicode data files, and #included at 
the end of pcre2_tables.c. Do not edit by hand. */

#ifdef SUPPORT_UNICODE

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
  {   0, PT_SCX, ucp_Adlam },
  {   6, PT_SCX, ucp_Ahom },
  {  11, PT_SCX, ucp_Anatolian_Hieroglyphs },
  {  32, PT_ANY, 0 },
  {  36, PT_SCX, ucp_Arabic },
  {  43, PT_SCX, ucp_Armenian },
  {  52, PT_SCX, ucp_Avestan },
  {  60, PT_SCX, ucp_Balinese },
  {  69, PT_SCX, ucp_Bamum },
  {  75, PT_SCX, ucp_Bassa_Vah },
  {  84, PT_SCX, ucp_Batak },
  {  90, PT_SCX, ucp_Bengali },
  {  98, PT_SCX, ucp_Bhaiksuki },
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
  { 293, PT_SCX, ucp_Bopomofo },
  { 302, PT_SCX, ucp_Brahmi },
  { 309, PT_SCX, ucp_Braille },
  { 317, PT_SCX, ucp_Buginese },
  { 326, PT_SCX, ucp_Buhid },
  { 332, PT_GC, ucp_C },
  { 334, PT_SCX, ucp_Canadian_Aboriginal },
  { 353, PT_SCX, ucp_Carian },
  { 360, PT_SCX, ucp_Caucasian_Albanian },
  { 378, PT_PC, ucp_Cc },
  { 381, PT_PC, ucp_Cf },
  { 384, PT_SCX, ucp_Chakma },
  { 391, PT_SCX, ucp_Cham },
  { 396, PT_SCX, ucp_Cherokee },
  { 405, PT_SCX, ucp_Chorasmian },
  { 416, PT_PC, ucp_Cn },
  { 419, PT_PC, ucp_Co },
  { 422, PT_SCX, ucp_Common },
  { 429, PT_SCX, ucp_Coptic },
  { 436, PT_PC, ucp_Cs },
  { 439, PT_SCX, ucp_Cuneiform },
  { 449, PT_SCX, ucp_Cypriot },
  { 457, PT_SCX, ucp_Cypro_Minoan },
  { 469, PT_SCX, ucp_Cyrillic },
  { 478, PT_SCX, ucp_Deseret },
  { 486, PT_SCX, ucp_Devanagari },
  { 497, PT_SCX, ucp_Dives_Akuru },
  { 508, PT_SCX, ucp_Dogra },
  { 514, PT_SCX, ucp_Duployan },
  { 523, PT_SCX, ucp_Egyptian_Hieroglyphs },
  { 543, PT_SCX, ucp_Elbasan },
  { 551, PT_SCX, ucp_Elymaic },
  { 559, PT_SCX, ucp_Ethiopic },
  { 568, PT_SCX, ucp_Georgian },
  { 577, PT_SCX, ucp_Glagolitic },
  { 588, PT_SCX, ucp_Gothic },
  { 595, PT_SCX, ucp_Grantha },
  { 603, PT_SCX, ucp_Greek },
  { 609, PT_SCX, ucp_Gujarati },
  { 618, PT_SCX, ucp_Gunjala_Gondi },
  { 631, PT_SCX, ucp_Gurmukhi },
  { 640, PT_SCX, ucp_Han },
  { 644, PT_SCX, ucp_Hangul },
  { 651, PT_SCX, ucp_Hanifi_Rohingya },
  { 666, PT_SCX, ucp_Hanunoo },
  { 674, PT_SCX, ucp_Hatran },
  { 681, PT_SCX, ucp_Hebrew },
  { 688, PT_SCX, ucp_Hiragana },
  { 697, PT_SCX, ucp_Imperial_Aramaic },
  { 713, PT_SCX, ucp_Inherited },
  { 723, PT_SCX, ucp_Inscriptional_Pahlavi },
  { 744, PT_SCX, ucp_Inscriptional_Parthian },
  { 766, PT_SCX, ucp_Javanese },
  { 775, PT_SCX, ucp_Kaithi },
  { 782, PT_SCX, ucp_Kannada },
  { 790, PT_SCX, ucp_Katakana },
  { 799, PT_SCX, ucp_Kayah_Li },
  { 807, PT_SCX, ucp_Kharoshthi },
  { 818, PT_SCX, ucp_Khitan_Small_Script },
  { 836, PT_SCX, ucp_Khmer },
  { 842, PT_SCX, ucp_Khojki },
  { 849, PT_SCX, ucp_Khudawadi },
  { 859, PT_GC, ucp_L },
  { 861, PT_LAMP, 0 },
  { 864, PT_SCX, ucp_Lao },
  { 868, PT_SCX, ucp_Latin },
  { 874, PT_LAMP, 0 },
  { 877, PT_SCX, ucp_Lepcha },
  { 884, PT_SCX, ucp_Limbu },
  { 890, PT_SCX, ucp_Linear_A },
  { 898, PT_SCX, ucp_Linear_B },
  { 906, PT_SCX, ucp_Lisu },
  { 911, PT_PC, ucp_Ll },
  { 914, PT_PC, ucp_Lm },
  { 917, PT_PC, ucp_Lo },
  { 920, PT_PC, ucp_Lt },
  { 923, PT_PC, ucp_Lu },
  { 926, PT_SCX, ucp_Lycian },
  { 933, PT_SCX, ucp_Lydian },
  { 940, PT_GC, ucp_M },
  { 942, PT_SCX, ucp_Mahajani },
  { 951, PT_SCX, ucp_Makasar },
  { 959, PT_SCX, ucp_Malayalam },
  { 969, PT_SCX, ucp_Mandaic },
  { 977, PT_SCX, ucp_Manichaean },
  { 988, PT_SCX, ucp_Marchen },
  { 996, PT_SCX, ucp_Masaram_Gondi },
  { 1009, PT_PC, ucp_Mc },
  { 1012, PT_PC, ucp_Me },
  { 1015, PT_SCX, ucp_Medefaidrin },
  { 1027, PT_SCX, ucp_Meetei_Mayek },
  { 1039, PT_SCX, ucp_Mende_Kikakui },
  { 1052, PT_SCX, ucp_Meroitic_Cursive },
  { 1068, PT_SCX, ucp_Meroitic_Hieroglyphs },
  { 1088, PT_SCX, ucp_Miao },
  { 1093, PT_PC, ucp_Mn },
  { 1096, PT_SCX, ucp_Modi },
  { 1101, PT_SCX, ucp_Mongolian },
  { 1111, PT_SCX, ucp_Mro },
  { 1115, PT_SCX, ucp_Multani },
  { 1123, PT_SCX, ucp_Myanmar },
  { 1131, PT_GC, ucp_N },
  { 1133, PT_SCX, ucp_Nabataean },
  { 1143, PT_SCX, ucp_Nandinagari },
  { 1155, PT_PC, ucp_Nd },
  { 1158, PT_SCX, ucp_Newa },
  { 1163, PT_SCX, ucp_New_Tai_Lue },
  { 1173, PT_SCX, ucp_Nko },
  { 1177, PT_PC, ucp_Nl },
  { 1180, PT_PC, ucp_No },
  { 1183, PT_SCX, ucp_Nushu },
  { 1189, PT_SCX, ucp_Nyiakeng_Puachue_Hmong },
  { 1210, PT_SCX, ucp_Ogham },
  { 1216, PT_SCX, ucp_Ol_Chiki },
  { 1224, PT_SCX, ucp_Old_Hungarian },
  { 1237, PT_SCX, ucp_Old_Italic },
  { 1247, PT_SCX, ucp_Old_North_Arabian },
  { 1263, PT_SCX, ucp_Old_Permic },
  { 1273, PT_SCX, ucp_Old_Persian },
  { 1284, PT_SCX, ucp_Old_Sogdian },
  { 1295, PT_SCX, ucp_Old_South_Arabian },
  { 1311, PT_SCX, ucp_Old_Turkic },
  { 1321, PT_SCX, ucp_Old_Uyghur },
  { 1331, PT_SCX, ucp_Oriya },
  { 1337, PT_SCX, ucp_Osage },
  { 1343, PT_SCX, ucp_Osmanya },
  { 1351, PT_GC, ucp_P },
  { 1353, PT_SCX, ucp_Pahawh_Hmong },
  { 1365, PT_SCX, ucp_Palmyrene },
  { 1375, PT_SCX, ucp_Pau_Cin_Hau },
  { 1385, PT_PC, ucp_Pc },
  { 1388, PT_PC, ucp_Pd },
  { 1391, PT_PC, ucp_Pe },
  { 1394, PT_PC, ucp_Pf },
  { 1397, PT_SCX, ucp_Phags_Pa },
  { 1405, PT_SCX, ucp_Phoenician },
  { 1416, PT_PC, ucp_Pi },
  { 1419, PT_PC, ucp_Po },
  { 1422, PT_PC, ucp_Ps },
  { 1425, PT_SCX, ucp_Psalter_Pahlavi },
  { 1440, PT_SCX, ucp_Rejang },
  { 1447, PT_SCX, ucp_Runic },
  { 1453, PT_GC, ucp_S },
  { 1455, PT_SCX, ucp_Samaritan },
  { 1465, PT_SCX, ucp_Saurashtra },
  { 1476, PT_PC, ucp_Sc },
  { 1479, PT_SCX, ucp_Sharada },
  { 1487, PT_SCX, ucp_Shavian },
  { 1495, PT_SCX, ucp_Siddham },
  { 1503, PT_SCX, ucp_SignWriting },
  { 1515, PT_SCX, ucp_Sinhala },
  { 1523, PT_PC, ucp_Sk },
  { 1526, PT_PC, ucp_Sm },
  { 1529, PT_PC, ucp_So },
  { 1532, PT_SCX, ucp_Sogdian },
  { 1540, PT_SCX, ucp_Sora_Sompeng },
  { 1552, PT_SCX, ucp_Soyombo },
  { 1560, PT_SCX, ucp_Sundanese },
  { 1570, PT_SCX, ucp_Syloti_Nagri },
  { 1582, PT_SCX, ucp_Syriac },
  { 1589, PT_SCX, ucp_Tagalog },
  { 1597, PT_SCX, ucp_Tagbanwa },
  { 1606, PT_SCX, ucp_Tai_Le },
  { 1612, PT_SCX, ucp_Tai_Tham },
  { 1620, PT_SCX, ucp_Tai_Viet },
  { 1628, PT_SCX, ucp_Takri },
  { 1634, PT_SCX, ucp_Tamil },
  { 1640, PT_SCX, ucp_Tangsa },
  { 1647, PT_SCX, ucp_Tangut },
  { 1654, PT_SCX, ucp_Telugu },
  { 1661, PT_SCX, ucp_Thaana },
  { 1668, PT_SCX, ucp_Thai },
  { 1673, PT_SCX, ucp_Tibetan },
  { 1681, PT_SCX, ucp_Tifinagh },
  { 1690, PT_SCX, ucp_Tirhuta },
  { 1698, PT_SCX, ucp_Toto },
  { 1703, PT_SCX, ucp_Ugaritic },
  { 1712, PT_SCX, ucp_Unknown },
  { 1720, PT_SCX, ucp_Vai },
  { 1724, PT_SCX, ucp_Vithkuqi },
  { 1733, PT_SCX, ucp_Wancho },
  { 1740, PT_SCX, ucp_Warang_Citi },
  { 1751, PT_ALNUM, 0 },
  { 1755, PT_PXSPACE, 0 },
  { 1759, PT_SPACE, 0 },
  { 1763, PT_UCNC, 0 },
  { 1767, PT_WORD, 0 },
  { 1771, PT_SCX, ucp_Yezidi },
  { 1778, PT_SCX, ucp_Yi },
  { 1781, PT_GC, ucp_Z },
  { 1783, PT_SCX, ucp_Zanabazar_Square },
  { 1799, PT_PC, ucp_Zl },
  { 1802, PT_PC, ucp_Zp },
  { 1805, PT_PC, ucp_Zs }
};

const size_t PRIV(utt_size) = sizeof(PRIV(utt)) / sizeof(ucp_type_table);

#endif /* SUPPORT_UNICODE */

/* End of pcre2_ucptables.c */
