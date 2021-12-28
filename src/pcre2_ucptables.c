/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
          New API code Copyright (c) 2016-2021 University of Cambridge

This module is auto-generated from Unicode data files. DO NOT EDIT MANUALLY!
Instead, modify the maint/GenerateUcpTables.py script and run it to generate
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

#ifdef SUPPORT_UNICODE

/* The PRIV(utt)[] table below translates Unicode property names into type and
code values. It is searched by binary chop, so must be in collating sequence of
name. Originally, the table contained pointers to the name strings in the first
field of each entry. However, that leads to a large number of relocations when
a shared library is dynamically loaded. A significant reduction is made by
putting all the names into a single, large string and using offsets instead.
All letters are lower cased, and underscores are removed, in accordance with
the "loose matching" rules that Unicode advises and Perl uses. */

#define STRING_adlam0 STR_a STR_d STR_l STR_a STR_m "\0"
#define STRING_adlm0 STR_a STR_d STR_l STR_m "\0"
#define STRING_aghb0 STR_a STR_g STR_h STR_b "\0"
#define STRING_ahom0 STR_a STR_h STR_o STR_m "\0"
#define STRING_anatolianhieroglyphs0 STR_a STR_n STR_a STR_t STR_o STR_l STR_i STR_a STR_n STR_h STR_i STR_e STR_r STR_o STR_g STR_l STR_y STR_p STR_h STR_s "\0"
#define STRING_any0 STR_a STR_n STR_y "\0"
#define STRING_arab0 STR_a STR_r STR_a STR_b "\0"
#define STRING_arabic0 STR_a STR_r STR_a STR_b STR_i STR_c "\0"
#define STRING_armenian0 STR_a STR_r STR_m STR_e STR_n STR_i STR_a STR_n "\0"
#define STRING_armi0 STR_a STR_r STR_m STR_i "\0"
#define STRING_armn0 STR_a STR_r STR_m STR_n "\0"
#define STRING_avestan0 STR_a STR_v STR_e STR_s STR_t STR_a STR_n "\0"
#define STRING_avst0 STR_a STR_v STR_s STR_t "\0"
#define STRING_bali0 STR_b STR_a STR_l STR_i "\0"
#define STRING_balinese0 STR_b STR_a STR_l STR_i STR_n STR_e STR_s STR_e "\0"
#define STRING_bamu0 STR_b STR_a STR_m STR_u "\0"
#define STRING_bamum0 STR_b STR_a STR_m STR_u STR_m "\0"
#define STRING_bass0 STR_b STR_a STR_s STR_s "\0"
#define STRING_bassavah0 STR_b STR_a STR_s STR_s STR_a STR_v STR_a STR_h "\0"
#define STRING_batak0 STR_b STR_a STR_t STR_a STR_k "\0"
#define STRING_batk0 STR_b STR_a STR_t STR_k "\0"
#define STRING_beng0 STR_b STR_e STR_n STR_g "\0"
#define STRING_bengali0 STR_b STR_e STR_n STR_g STR_a STR_l STR_i "\0"
#define STRING_bhaiksuki0 STR_b STR_h STR_a STR_i STR_k STR_s STR_u STR_k STR_i "\0"
#define STRING_bhks0 STR_b STR_h STR_k STR_s "\0"
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
#define STRING_bopo0 STR_b STR_o STR_p STR_o "\0"
#define STRING_bopomofo0 STR_b STR_o STR_p STR_o STR_m STR_o STR_f STR_o "\0"
#define STRING_brah0 STR_b STR_r STR_a STR_h "\0"
#define STRING_brahmi0 STR_b STR_r STR_a STR_h STR_m STR_i "\0"
#define STRING_brai0 STR_b STR_r STR_a STR_i "\0"
#define STRING_braille0 STR_b STR_r STR_a STR_i STR_l STR_l STR_e "\0"
#define STRING_bugi0 STR_b STR_u STR_g STR_i "\0"
#define STRING_buginese0 STR_b STR_u STR_g STR_i STR_n STR_e STR_s STR_e "\0"
#define STRING_buhd0 STR_b STR_u STR_h STR_d "\0"
#define STRING_buhid0 STR_b STR_u STR_h STR_i STR_d "\0"
#define STRING_c0 STR_c "\0"
#define STRING_cakm0 STR_c STR_a STR_k STR_m "\0"
#define STRING_canadianaboriginal0 STR_c STR_a STR_n STR_a STR_d STR_i STR_a STR_n STR_a STR_b STR_o STR_r STR_i STR_g STR_i STR_n STR_a STR_l "\0"
#define STRING_cans0 STR_c STR_a STR_n STR_s "\0"
#define STRING_cari0 STR_c STR_a STR_r STR_i "\0"
#define STRING_carian0 STR_c STR_a STR_r STR_i STR_a STR_n "\0"
#define STRING_caucasianalbanian0 STR_c STR_a STR_u STR_c STR_a STR_s STR_i STR_a STR_n STR_a STR_l STR_b STR_a STR_n STR_i STR_a STR_n "\0"
#define STRING_cc0 STR_c STR_c "\0"
#define STRING_cf0 STR_c STR_f "\0"
#define STRING_chakma0 STR_c STR_h STR_a STR_k STR_m STR_a "\0"
#define STRING_cham0 STR_c STR_h STR_a STR_m "\0"
#define STRING_cher0 STR_c STR_h STR_e STR_r "\0"
#define STRING_cherokee0 STR_c STR_h STR_e STR_r STR_o STR_k STR_e STR_e "\0"
#define STRING_chorasmian0 STR_c STR_h STR_o STR_r STR_a STR_s STR_m STR_i STR_a STR_n "\0"
#define STRING_chrs0 STR_c STR_h STR_r STR_s "\0"
#define STRING_cn0 STR_c STR_n "\0"
#define STRING_co0 STR_c STR_o "\0"
#define STRING_common0 STR_c STR_o STR_m STR_m STR_o STR_n "\0"
#define STRING_copt0 STR_c STR_o STR_p STR_t "\0"
#define STRING_coptic0 STR_c STR_o STR_p STR_t STR_i STR_c "\0"
#define STRING_cpmn0 STR_c STR_p STR_m STR_n "\0"
#define STRING_cprt0 STR_c STR_p STR_r STR_t "\0"
#define STRING_cs0 STR_c STR_s "\0"
#define STRING_cuneiform0 STR_c STR_u STR_n STR_e STR_i STR_f STR_o STR_r STR_m "\0"
#define STRING_cypriot0 STR_c STR_y STR_p STR_r STR_i STR_o STR_t "\0"
#define STRING_cyprominoan0 STR_c STR_y STR_p STR_r STR_o STR_m STR_i STR_n STR_o STR_a STR_n "\0"
#define STRING_cyrillic0 STR_c STR_y STR_r STR_i STR_l STR_l STR_i STR_c "\0"
#define STRING_cyrl0 STR_c STR_y STR_r STR_l "\0"
#define STRING_deseret0 STR_d STR_e STR_s STR_e STR_r STR_e STR_t "\0"
#define STRING_deva0 STR_d STR_e STR_v STR_a "\0"
#define STRING_devanagari0 STR_d STR_e STR_v STR_a STR_n STR_a STR_g STR_a STR_r STR_i "\0"
#define STRING_diak0 STR_d STR_i STR_a STR_k "\0"
#define STRING_divesakuru0 STR_d STR_i STR_v STR_e STR_s STR_a STR_k STR_u STR_r STR_u "\0"
#define STRING_dogr0 STR_d STR_o STR_g STR_r "\0"
#define STRING_dogra0 STR_d STR_o STR_g STR_r STR_a "\0"
#define STRING_dsrt0 STR_d STR_s STR_r STR_t "\0"
#define STRING_dupl0 STR_d STR_u STR_p STR_l "\0"
#define STRING_duployan0 STR_d STR_u STR_p STR_l STR_o STR_y STR_a STR_n "\0"
#define STRING_egyp0 STR_e STR_g STR_y STR_p "\0"
#define STRING_egyptianhieroglyphs0 STR_e STR_g STR_y STR_p STR_t STR_i STR_a STR_n STR_h STR_i STR_e STR_r STR_o STR_g STR_l STR_y STR_p STR_h STR_s "\0"
#define STRING_elba0 STR_e STR_l STR_b STR_a "\0"
#define STRING_elbasan0 STR_e STR_l STR_b STR_a STR_s STR_a STR_n "\0"
#define STRING_elym0 STR_e STR_l STR_y STR_m "\0"
#define STRING_elymaic0 STR_e STR_l STR_y STR_m STR_a STR_i STR_c "\0"
#define STRING_ethi0 STR_e STR_t STR_h STR_i "\0"
#define STRING_ethiopic0 STR_e STR_t STR_h STR_i STR_o STR_p STR_i STR_c "\0"
#define STRING_geor0 STR_g STR_e STR_o STR_r "\0"
#define STRING_georgian0 STR_g STR_e STR_o STR_r STR_g STR_i STR_a STR_n "\0"
#define STRING_glag0 STR_g STR_l STR_a STR_g "\0"
#define STRING_glagolitic0 STR_g STR_l STR_a STR_g STR_o STR_l STR_i STR_t STR_i STR_c "\0"
#define STRING_gong0 STR_g STR_o STR_n STR_g "\0"
#define STRING_gonm0 STR_g STR_o STR_n STR_m "\0"
#define STRING_goth0 STR_g STR_o STR_t STR_h "\0"
#define STRING_gothic0 STR_g STR_o STR_t STR_h STR_i STR_c "\0"
#define STRING_gran0 STR_g STR_r STR_a STR_n "\0"
#define STRING_grantha0 STR_g STR_r STR_a STR_n STR_t STR_h STR_a "\0"
#define STRING_greek0 STR_g STR_r STR_e STR_e STR_k "\0"
#define STRING_grek0 STR_g STR_r STR_e STR_k "\0"
#define STRING_gujarati0 STR_g STR_u STR_j STR_a STR_r STR_a STR_t STR_i "\0"
#define STRING_gujr0 STR_g STR_u STR_j STR_r "\0"
#define STRING_gunjalagondi0 STR_g STR_u STR_n STR_j STR_a STR_l STR_a STR_g STR_o STR_n STR_d STR_i "\0"
#define STRING_gurmukhi0 STR_g STR_u STR_r STR_m STR_u STR_k STR_h STR_i "\0"
#define STRING_guru0 STR_g STR_u STR_r STR_u "\0"
#define STRING_han0 STR_h STR_a STR_n "\0"
#define STRING_hang0 STR_h STR_a STR_n STR_g "\0"
#define STRING_hangul0 STR_h STR_a STR_n STR_g STR_u STR_l "\0"
#define STRING_hani0 STR_h STR_a STR_n STR_i "\0"
#define STRING_hanifirohingya0 STR_h STR_a STR_n STR_i STR_f STR_i STR_r STR_o STR_h STR_i STR_n STR_g STR_y STR_a "\0"
#define STRING_hano0 STR_h STR_a STR_n STR_o "\0"
#define STRING_hanunoo0 STR_h STR_a STR_n STR_u STR_n STR_o STR_o "\0"
#define STRING_hatr0 STR_h STR_a STR_t STR_r "\0"
#define STRING_hatran0 STR_h STR_a STR_t STR_r STR_a STR_n "\0"
#define STRING_hebr0 STR_h STR_e STR_b STR_r "\0"
#define STRING_hebrew0 STR_h STR_e STR_b STR_r STR_e STR_w "\0"
#define STRING_hira0 STR_h STR_i STR_r STR_a "\0"
#define STRING_hiragana0 STR_h STR_i STR_r STR_a STR_g STR_a STR_n STR_a "\0"
#define STRING_hluw0 STR_h STR_l STR_u STR_w "\0"
#define STRING_hmng0 STR_h STR_m STR_n STR_g "\0"
#define STRING_hmnp0 STR_h STR_m STR_n STR_p "\0"
#define STRING_hung0 STR_h STR_u STR_n STR_g "\0"
#define STRING_imperialaramaic0 STR_i STR_m STR_p STR_e STR_r STR_i STR_a STR_l STR_a STR_r STR_a STR_m STR_a STR_i STR_c "\0"
#define STRING_inherited0 STR_i STR_n STR_h STR_e STR_r STR_i STR_t STR_e STR_d "\0"
#define STRING_inscriptionalpahlavi0 STR_i STR_n STR_s STR_c STR_r STR_i STR_p STR_t STR_i STR_o STR_n STR_a STR_l STR_p STR_a STR_h STR_l STR_a STR_v STR_i "\0"
#define STRING_inscriptionalparthian0 STR_i STR_n STR_s STR_c STR_r STR_i STR_p STR_t STR_i STR_o STR_n STR_a STR_l STR_p STR_a STR_r STR_t STR_h STR_i STR_a STR_n "\0"
#define STRING_ital0 STR_i STR_t STR_a STR_l "\0"
#define STRING_java0 STR_j STR_a STR_v STR_a "\0"
#define STRING_javanese0 STR_j STR_a STR_v STR_a STR_n STR_e STR_s STR_e "\0"
#define STRING_kaithi0 STR_k STR_a STR_i STR_t STR_h STR_i "\0"
#define STRING_kali0 STR_k STR_a STR_l STR_i "\0"
#define STRING_kana0 STR_k STR_a STR_n STR_a "\0"
#define STRING_kannada0 STR_k STR_a STR_n STR_n STR_a STR_d STR_a "\0"
#define STRING_katakana0 STR_k STR_a STR_t STR_a STR_k STR_a STR_n STR_a "\0"
#define STRING_kayahli0 STR_k STR_a STR_y STR_a STR_h STR_l STR_i "\0"
#define STRING_khar0 STR_k STR_h STR_a STR_r "\0"
#define STRING_kharoshthi0 STR_k STR_h STR_a STR_r STR_o STR_s STR_h STR_t STR_h STR_i "\0"
#define STRING_khitansmallscript0 STR_k STR_h STR_i STR_t STR_a STR_n STR_s STR_m STR_a STR_l STR_l STR_s STR_c STR_r STR_i STR_p STR_t "\0"
#define STRING_khmer0 STR_k STR_h STR_m STR_e STR_r "\0"
#define STRING_khmr0 STR_k STR_h STR_m STR_r "\0"
#define STRING_khoj0 STR_k STR_h STR_o STR_j "\0"
#define STRING_khojki0 STR_k STR_h STR_o STR_j STR_k STR_i "\0"
#define STRING_khudawadi0 STR_k STR_h STR_u STR_d STR_a STR_w STR_a STR_d STR_i "\0"
#define STRING_kits0 STR_k STR_i STR_t STR_s "\0"
#define STRING_knda0 STR_k STR_n STR_d STR_a "\0"
#define STRING_kthi0 STR_k STR_t STR_h STR_i "\0"
#define STRING_l0 STR_l "\0"
#define STRING_l_AMPERSAND0 STR_l STR_AMPERSAND "\0"
#define STRING_lana0 STR_l STR_a STR_n STR_a "\0"
#define STRING_lao0 STR_l STR_a STR_o "\0"
#define STRING_laoo0 STR_l STR_a STR_o STR_o "\0"
#define STRING_latin0 STR_l STR_a STR_t STR_i STR_n "\0"
#define STRING_latn0 STR_l STR_a STR_t STR_n "\0"
#define STRING_lc0 STR_l STR_c "\0"
#define STRING_lepc0 STR_l STR_e STR_p STR_c "\0"
#define STRING_lepcha0 STR_l STR_e STR_p STR_c STR_h STR_a "\0"
#define STRING_limb0 STR_l STR_i STR_m STR_b "\0"
#define STRING_limbu0 STR_l STR_i STR_m STR_b STR_u "\0"
#define STRING_lina0 STR_l STR_i STR_n STR_a "\0"
#define STRING_linb0 STR_l STR_i STR_n STR_b "\0"
#define STRING_lineara0 STR_l STR_i STR_n STR_e STR_a STR_r STR_a "\0"
#define STRING_linearb0 STR_l STR_i STR_n STR_e STR_a STR_r STR_b "\0"
#define STRING_lisu0 STR_l STR_i STR_s STR_u "\0"
#define STRING_ll0 STR_l STR_l "\0"
#define STRING_lm0 STR_l STR_m "\0"
#define STRING_lo0 STR_l STR_o "\0"
#define STRING_lt0 STR_l STR_t "\0"
#define STRING_lu0 STR_l STR_u "\0"
#define STRING_lyci0 STR_l STR_y STR_c STR_i "\0"
#define STRING_lycian0 STR_l STR_y STR_c STR_i STR_a STR_n "\0"
#define STRING_lydi0 STR_l STR_y STR_d STR_i "\0"
#define STRING_lydian0 STR_l STR_y STR_d STR_i STR_a STR_n "\0"
#define STRING_m0 STR_m "\0"
#define STRING_mahajani0 STR_m STR_a STR_h STR_a STR_j STR_a STR_n STR_i "\0"
#define STRING_mahj0 STR_m STR_a STR_h STR_j "\0"
#define STRING_maka0 STR_m STR_a STR_k STR_a "\0"
#define STRING_makasar0 STR_m STR_a STR_k STR_a STR_s STR_a STR_r "\0"
#define STRING_malayalam0 STR_m STR_a STR_l STR_a STR_y STR_a STR_l STR_a STR_m "\0"
#define STRING_mand0 STR_m STR_a STR_n STR_d "\0"
#define STRING_mandaic0 STR_m STR_a STR_n STR_d STR_a STR_i STR_c "\0"
#define STRING_mani0 STR_m STR_a STR_n STR_i "\0"
#define STRING_manichaean0 STR_m STR_a STR_n STR_i STR_c STR_h STR_a STR_e STR_a STR_n "\0"
#define STRING_marc0 STR_m STR_a STR_r STR_c "\0"
#define STRING_marchen0 STR_m STR_a STR_r STR_c STR_h STR_e STR_n "\0"
#define STRING_masaramgondi0 STR_m STR_a STR_s STR_a STR_r STR_a STR_m STR_g STR_o STR_n STR_d STR_i "\0"
#define STRING_mc0 STR_m STR_c "\0"
#define STRING_me0 STR_m STR_e "\0"
#define STRING_medefaidrin0 STR_m STR_e STR_d STR_e STR_f STR_a STR_i STR_d STR_r STR_i STR_n "\0"
#define STRING_medf0 STR_m STR_e STR_d STR_f "\0"
#define STRING_meeteimayek0 STR_m STR_e STR_e STR_t STR_e STR_i STR_m STR_a STR_y STR_e STR_k "\0"
#define STRING_mend0 STR_m STR_e STR_n STR_d "\0"
#define STRING_mendekikakui0 STR_m STR_e STR_n STR_d STR_e STR_k STR_i STR_k STR_a STR_k STR_u STR_i "\0"
#define STRING_merc0 STR_m STR_e STR_r STR_c "\0"
#define STRING_mero0 STR_m STR_e STR_r STR_o "\0"
#define STRING_meroiticcursive0 STR_m STR_e STR_r STR_o STR_i STR_t STR_i STR_c STR_c STR_u STR_r STR_s STR_i STR_v STR_e "\0"
#define STRING_meroitichieroglyphs0 STR_m STR_e STR_r STR_o STR_i STR_t STR_i STR_c STR_h STR_i STR_e STR_r STR_o STR_g STR_l STR_y STR_p STR_h STR_s "\0"
#define STRING_miao0 STR_m STR_i STR_a STR_o "\0"
#define STRING_mlym0 STR_m STR_l STR_y STR_m "\0"
#define STRING_mn0 STR_m STR_n "\0"
#define STRING_modi0 STR_m STR_o STR_d STR_i "\0"
#define STRING_mong0 STR_m STR_o STR_n STR_g "\0"
#define STRING_mongolian0 STR_m STR_o STR_n STR_g STR_o STR_l STR_i STR_a STR_n "\0"
#define STRING_mro0 STR_m STR_r STR_o "\0"
#define STRING_mroo0 STR_m STR_r STR_o STR_o "\0"
#define STRING_mtei0 STR_m STR_t STR_e STR_i "\0"
#define STRING_mult0 STR_m STR_u STR_l STR_t "\0"
#define STRING_multani0 STR_m STR_u STR_l STR_t STR_a STR_n STR_i "\0"
#define STRING_myanmar0 STR_m STR_y STR_a STR_n STR_m STR_a STR_r "\0"
#define STRING_mymr0 STR_m STR_y STR_m STR_r "\0"
#define STRING_n0 STR_n "\0"
#define STRING_nabataean0 STR_n STR_a STR_b STR_a STR_t STR_a STR_e STR_a STR_n "\0"
#define STRING_nand0 STR_n STR_a STR_n STR_d "\0"
#define STRING_nandinagari0 STR_n STR_a STR_n STR_d STR_i STR_n STR_a STR_g STR_a STR_r STR_i "\0"
#define STRING_narb0 STR_n STR_a STR_r STR_b "\0"
#define STRING_nbat0 STR_n STR_b STR_a STR_t "\0"
#define STRING_nd0 STR_n STR_d "\0"
#define STRING_newa0 STR_n STR_e STR_w STR_a "\0"
#define STRING_newtailue0 STR_n STR_e STR_w STR_t STR_a STR_i STR_l STR_u STR_e "\0"
#define STRING_nko0 STR_n STR_k STR_o "\0"
#define STRING_nkoo0 STR_n STR_k STR_o STR_o "\0"
#define STRING_nl0 STR_n STR_l "\0"
#define STRING_no0 STR_n STR_o "\0"
#define STRING_nshu0 STR_n STR_s STR_h STR_u "\0"
#define STRING_nushu0 STR_n STR_u STR_s STR_h STR_u "\0"
#define STRING_nyiakengpuachuehmong0 STR_n STR_y STR_i STR_a STR_k STR_e STR_n STR_g STR_p STR_u STR_a STR_c STR_h STR_u STR_e STR_h STR_m STR_o STR_n STR_g "\0"
#define STRING_ogam0 STR_o STR_g STR_a STR_m "\0"
#define STRING_ogham0 STR_o STR_g STR_h STR_a STR_m "\0"
#define STRING_olchiki0 STR_o STR_l STR_c STR_h STR_i STR_k STR_i "\0"
#define STRING_olck0 STR_o STR_l STR_c STR_k "\0"
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
#define STRING_orkh0 STR_o STR_r STR_k STR_h "\0"
#define STRING_orya0 STR_o STR_r STR_y STR_a "\0"
#define STRING_osage0 STR_o STR_s STR_a STR_g STR_e "\0"
#define STRING_osge0 STR_o STR_s STR_g STR_e "\0"
#define STRING_osma0 STR_o STR_s STR_m STR_a "\0"
#define STRING_osmanya0 STR_o STR_s STR_m STR_a STR_n STR_y STR_a "\0"
#define STRING_ougr0 STR_o STR_u STR_g STR_r "\0"
#define STRING_p0 STR_p "\0"
#define STRING_pahawhhmong0 STR_p STR_a STR_h STR_a STR_w STR_h STR_h STR_m STR_o STR_n STR_g "\0"
#define STRING_palm0 STR_p STR_a STR_l STR_m "\0"
#define STRING_palmyrene0 STR_p STR_a STR_l STR_m STR_y STR_r STR_e STR_n STR_e "\0"
#define STRING_pauc0 STR_p STR_a STR_u STR_c "\0"
#define STRING_paucinhau0 STR_p STR_a STR_u STR_c STR_i STR_n STR_h STR_a STR_u "\0"
#define STRING_pc0 STR_p STR_c "\0"
#define STRING_pd0 STR_p STR_d "\0"
#define STRING_pe0 STR_p STR_e "\0"
#define STRING_perm0 STR_p STR_e STR_r STR_m "\0"
#define STRING_pf0 STR_p STR_f "\0"
#define STRING_phag0 STR_p STR_h STR_a STR_g "\0"
#define STRING_phagspa0 STR_p STR_h STR_a STR_g STR_s STR_p STR_a "\0"
#define STRING_phli0 STR_p STR_h STR_l STR_i "\0"
#define STRING_phlp0 STR_p STR_h STR_l STR_p "\0"
#define STRING_phnx0 STR_p STR_h STR_n STR_x "\0"
#define STRING_phoenician0 STR_p STR_h STR_o STR_e STR_n STR_i STR_c STR_i STR_a STR_n "\0"
#define STRING_pi0 STR_p STR_i "\0"
#define STRING_plrd0 STR_p STR_l STR_r STR_d "\0"
#define STRING_po0 STR_p STR_o "\0"
#define STRING_prti0 STR_p STR_r STR_t STR_i "\0"
#define STRING_ps0 STR_p STR_s "\0"
#define STRING_psalterpahlavi0 STR_p STR_s STR_a STR_l STR_t STR_e STR_r STR_p STR_a STR_h STR_l STR_a STR_v STR_i "\0"
#define STRING_rejang0 STR_r STR_e STR_j STR_a STR_n STR_g "\0"
#define STRING_rjng0 STR_r STR_j STR_n STR_g "\0"
#define STRING_rohg0 STR_r STR_o STR_h STR_g "\0"
#define STRING_runic0 STR_r STR_u STR_n STR_i STR_c "\0"
#define STRING_runr0 STR_r STR_u STR_n STR_r "\0"
#define STRING_s0 STR_s "\0"
#define STRING_samaritan0 STR_s STR_a STR_m STR_a STR_r STR_i STR_t STR_a STR_n "\0"
#define STRING_samr0 STR_s STR_a STR_m STR_r "\0"
#define STRING_sarb0 STR_s STR_a STR_r STR_b "\0"
#define STRING_saur0 STR_s STR_a STR_u STR_r "\0"
#define STRING_saurashtra0 STR_s STR_a STR_u STR_r STR_a STR_s STR_h STR_t STR_r STR_a "\0"
#define STRING_sc0 STR_s STR_c "\0"
#define STRING_sgnw0 STR_s STR_g STR_n STR_w "\0"
#define STRING_sharada0 STR_s STR_h STR_a STR_r STR_a STR_d STR_a "\0"
#define STRING_shavian0 STR_s STR_h STR_a STR_v STR_i STR_a STR_n "\0"
#define STRING_shaw0 STR_s STR_h STR_a STR_w "\0"
#define STRING_shrd0 STR_s STR_h STR_r STR_d "\0"
#define STRING_sidd0 STR_s STR_i STR_d STR_d "\0"
#define STRING_siddham0 STR_s STR_i STR_d STR_d STR_h STR_a STR_m "\0"
#define STRING_signwriting0 STR_s STR_i STR_g STR_n STR_w STR_r STR_i STR_t STR_i STR_n STR_g "\0"
#define STRING_sind0 STR_s STR_i STR_n STR_d "\0"
#define STRING_sinh0 STR_s STR_i STR_n STR_h "\0"
#define STRING_sinhala0 STR_s STR_i STR_n STR_h STR_a STR_l STR_a "\0"
#define STRING_sk0 STR_s STR_k "\0"
#define STRING_sm0 STR_s STR_m "\0"
#define STRING_so0 STR_s STR_o "\0"
#define STRING_sogd0 STR_s STR_o STR_g STR_d "\0"
#define STRING_sogdian0 STR_s STR_o STR_g STR_d STR_i STR_a STR_n "\0"
#define STRING_sogo0 STR_s STR_o STR_g STR_o "\0"
#define STRING_sora0 STR_s STR_o STR_r STR_a "\0"
#define STRING_sorasompeng0 STR_s STR_o STR_r STR_a STR_s STR_o STR_m STR_p STR_e STR_n STR_g "\0"
#define STRING_soyo0 STR_s STR_o STR_y STR_o "\0"
#define STRING_soyombo0 STR_s STR_o STR_y STR_o STR_m STR_b STR_o "\0"
#define STRING_sund0 STR_s STR_u STR_n STR_d "\0"
#define STRING_sundanese0 STR_s STR_u STR_n STR_d STR_a STR_n STR_e STR_s STR_e "\0"
#define STRING_sylo0 STR_s STR_y STR_l STR_o "\0"
#define STRING_sylotinagri0 STR_s STR_y STR_l STR_o STR_t STR_i STR_n STR_a STR_g STR_r STR_i "\0"
#define STRING_syrc0 STR_s STR_y STR_r STR_c "\0"
#define STRING_syriac0 STR_s STR_y STR_r STR_i STR_a STR_c "\0"
#define STRING_tagalog0 STR_t STR_a STR_g STR_a STR_l STR_o STR_g "\0"
#define STRING_tagb0 STR_t STR_a STR_g STR_b "\0"
#define STRING_tagbanwa0 STR_t STR_a STR_g STR_b STR_a STR_n STR_w STR_a "\0"
#define STRING_taile0 STR_t STR_a STR_i STR_l STR_e "\0"
#define STRING_taitham0 STR_t STR_a STR_i STR_t STR_h STR_a STR_m "\0"
#define STRING_taiviet0 STR_t STR_a STR_i STR_v STR_i STR_e STR_t "\0"
#define STRING_takr0 STR_t STR_a STR_k STR_r "\0"
#define STRING_takri0 STR_t STR_a STR_k STR_r STR_i "\0"
#define STRING_tale0 STR_t STR_a STR_l STR_e "\0"
#define STRING_talu0 STR_t STR_a STR_l STR_u "\0"
#define STRING_tamil0 STR_t STR_a STR_m STR_i STR_l "\0"
#define STRING_taml0 STR_t STR_a STR_m STR_l "\0"
#define STRING_tang0 STR_t STR_a STR_n STR_g "\0"
#define STRING_tangsa0 STR_t STR_a STR_n STR_g STR_s STR_a "\0"
#define STRING_tangut0 STR_t STR_a STR_n STR_g STR_u STR_t "\0"
#define STRING_tavt0 STR_t STR_a STR_v STR_t "\0"
#define STRING_telu0 STR_t STR_e STR_l STR_u "\0"
#define STRING_telugu0 STR_t STR_e STR_l STR_u STR_g STR_u "\0"
#define STRING_tfng0 STR_t STR_f STR_n STR_g "\0"
#define STRING_tglg0 STR_t STR_g STR_l STR_g "\0"
#define STRING_thaa0 STR_t STR_h STR_a STR_a "\0"
#define STRING_thaana0 STR_t STR_h STR_a STR_a STR_n STR_a "\0"
#define STRING_thai0 STR_t STR_h STR_a STR_i "\0"
#define STRING_tibetan0 STR_t STR_i STR_b STR_e STR_t STR_a STR_n "\0"
#define STRING_tibt0 STR_t STR_i STR_b STR_t "\0"
#define STRING_tifinagh0 STR_t STR_i STR_f STR_i STR_n STR_a STR_g STR_h "\0"
#define STRING_tirh0 STR_t STR_i STR_r STR_h "\0"
#define STRING_tirhuta0 STR_t STR_i STR_r STR_h STR_u STR_t STR_a "\0"
#define STRING_tngs0 STR_t STR_n STR_g STR_s "\0"
#define STRING_toto0 STR_t STR_o STR_t STR_o "\0"
#define STRING_ugar0 STR_u STR_g STR_a STR_r "\0"
#define STRING_ugaritic0 STR_u STR_g STR_a STR_r STR_i STR_t STR_i STR_c "\0"
#define STRING_unknown0 STR_u STR_n STR_k STR_n STR_o STR_w STR_n "\0"
#define STRING_vai0 STR_v STR_a STR_i "\0"
#define STRING_vaii0 STR_v STR_a STR_i STR_i "\0"
#define STRING_vith0 STR_v STR_i STR_t STR_h "\0"
#define STRING_vithkuqi0 STR_v STR_i STR_t STR_h STR_k STR_u STR_q STR_i "\0"
#define STRING_wancho0 STR_w STR_a STR_n STR_c STR_h STR_o "\0"
#define STRING_wara0 STR_w STR_a STR_r STR_a "\0"
#define STRING_warangciti0 STR_w STR_a STR_r STR_a STR_n STR_g STR_c STR_i STR_t STR_i "\0"
#define STRING_wcho0 STR_w STR_c STR_h STR_o "\0"
#define STRING_xan0 STR_x STR_a STR_n "\0"
#define STRING_xpeo0 STR_x STR_p STR_e STR_o "\0"
#define STRING_xps0 STR_x STR_p STR_s "\0"
#define STRING_xsp0 STR_x STR_s STR_p "\0"
#define STRING_xsux0 STR_x STR_s STR_u STR_x "\0"
#define STRING_xuc0 STR_x STR_u STR_c "\0"
#define STRING_xwd0 STR_x STR_w STR_d "\0"
#define STRING_yezi0 STR_y STR_e STR_z STR_i "\0"
#define STRING_yezidi0 STR_y STR_e STR_z STR_i STR_d STR_i "\0"
#define STRING_yi0 STR_y STR_i "\0"
#define STRING_yiii0 STR_y STR_i STR_i STR_i "\0"
#define STRING_z0 STR_z "\0"
#define STRING_zanabazarsquare0 STR_z STR_a STR_n STR_a STR_b STR_a STR_z STR_a STR_r STR_s STR_q STR_u STR_a STR_r STR_e "\0"
#define STRING_zanb0 STR_z STR_a STR_n STR_b "\0"
#define STRING_zinh0 STR_z STR_i STR_n STR_h "\0"
#define STRING_zl0 STR_z STR_l "\0"
#define STRING_zp0 STR_z STR_p "\0"
#define STRING_zs0 STR_z STR_s "\0"
#define STRING_zyyy0 STR_z STR_y STR_y STR_y "\0"
#define STRING_zzzz0 STR_z STR_z STR_z STR_z "\0"

const char PRIV(utt_names)[] =
  STRING_adlam0
  STRING_adlm0
  STRING_aghb0
  STRING_ahom0
  STRING_anatolianhieroglyphs0
  STRING_any0
  STRING_arab0
  STRING_arabic0
  STRING_armenian0
  STRING_armi0
  STRING_armn0
  STRING_avestan0
  STRING_avst0
  STRING_bali0
  STRING_balinese0
  STRING_bamu0
  STRING_bamum0
  STRING_bass0
  STRING_bassavah0
  STRING_batak0
  STRING_batk0
  STRING_beng0
  STRING_bengali0
  STRING_bhaiksuki0
  STRING_bhks0
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
  STRING_bopo0
  STRING_bopomofo0
  STRING_brah0
  STRING_brahmi0
  STRING_brai0
  STRING_braille0
  STRING_bugi0
  STRING_buginese0
  STRING_buhd0
  STRING_buhid0
  STRING_c0
  STRING_cakm0
  STRING_canadianaboriginal0
  STRING_cans0
  STRING_cari0
  STRING_carian0
  STRING_caucasianalbanian0
  STRING_cc0
  STRING_cf0
  STRING_chakma0
  STRING_cham0
  STRING_cher0
  STRING_cherokee0
  STRING_chorasmian0
  STRING_chrs0
  STRING_cn0
  STRING_co0
  STRING_common0
  STRING_copt0
  STRING_coptic0
  STRING_cpmn0
  STRING_cprt0
  STRING_cs0
  STRING_cuneiform0
  STRING_cypriot0
  STRING_cyprominoan0
  STRING_cyrillic0
  STRING_cyrl0
  STRING_deseret0
  STRING_deva0
  STRING_devanagari0
  STRING_diak0
  STRING_divesakuru0
  STRING_dogr0
  STRING_dogra0
  STRING_dsrt0
  STRING_dupl0
  STRING_duployan0
  STRING_egyp0
  STRING_egyptianhieroglyphs0
  STRING_elba0
  STRING_elbasan0
  STRING_elym0
  STRING_elymaic0
  STRING_ethi0
  STRING_ethiopic0
  STRING_geor0
  STRING_georgian0
  STRING_glag0
  STRING_glagolitic0
  STRING_gong0
  STRING_gonm0
  STRING_goth0
  STRING_gothic0
  STRING_gran0
  STRING_grantha0
  STRING_greek0
  STRING_grek0
  STRING_gujarati0
  STRING_gujr0
  STRING_gunjalagondi0
  STRING_gurmukhi0
  STRING_guru0
  STRING_han0
  STRING_hang0
  STRING_hangul0
  STRING_hani0
  STRING_hanifirohingya0
  STRING_hano0
  STRING_hanunoo0
  STRING_hatr0
  STRING_hatran0
  STRING_hebr0
  STRING_hebrew0
  STRING_hira0
  STRING_hiragana0
  STRING_hluw0
  STRING_hmng0
  STRING_hmnp0
  STRING_hung0
  STRING_imperialaramaic0
  STRING_inherited0
  STRING_inscriptionalpahlavi0
  STRING_inscriptionalparthian0
  STRING_ital0
  STRING_java0
  STRING_javanese0
  STRING_kaithi0
  STRING_kali0
  STRING_kana0
  STRING_kannada0
  STRING_katakana0
  STRING_kayahli0
  STRING_khar0
  STRING_kharoshthi0
  STRING_khitansmallscript0
  STRING_khmer0
  STRING_khmr0
  STRING_khoj0
  STRING_khojki0
  STRING_khudawadi0
  STRING_kits0
  STRING_knda0
  STRING_kthi0
  STRING_l0
  STRING_l_AMPERSAND0
  STRING_lana0
  STRING_lao0
  STRING_laoo0
  STRING_latin0
  STRING_latn0
  STRING_lc0
  STRING_lepc0
  STRING_lepcha0
  STRING_limb0
  STRING_limbu0
  STRING_lina0
  STRING_linb0
  STRING_lineara0
  STRING_linearb0
  STRING_lisu0
  STRING_ll0
  STRING_lm0
  STRING_lo0
  STRING_lt0
  STRING_lu0
  STRING_lyci0
  STRING_lycian0
  STRING_lydi0
  STRING_lydian0
  STRING_m0
  STRING_mahajani0
  STRING_mahj0
  STRING_maka0
  STRING_makasar0
  STRING_malayalam0
  STRING_mand0
  STRING_mandaic0
  STRING_mani0
  STRING_manichaean0
  STRING_marc0
  STRING_marchen0
  STRING_masaramgondi0
  STRING_mc0
  STRING_me0
  STRING_medefaidrin0
  STRING_medf0
  STRING_meeteimayek0
  STRING_mend0
  STRING_mendekikakui0
  STRING_merc0
  STRING_mero0
  STRING_meroiticcursive0
  STRING_meroitichieroglyphs0
  STRING_miao0
  STRING_mlym0
  STRING_mn0
  STRING_modi0
  STRING_mong0
  STRING_mongolian0
  STRING_mro0
  STRING_mroo0
  STRING_mtei0
  STRING_mult0
  STRING_multani0
  STRING_myanmar0
  STRING_mymr0
  STRING_n0
  STRING_nabataean0
  STRING_nand0
  STRING_nandinagari0
  STRING_narb0
  STRING_nbat0
  STRING_nd0
  STRING_newa0
  STRING_newtailue0
  STRING_nko0
  STRING_nkoo0
  STRING_nl0
  STRING_no0
  STRING_nshu0
  STRING_nushu0
  STRING_nyiakengpuachuehmong0
  STRING_ogam0
  STRING_ogham0
  STRING_olchiki0
  STRING_olck0
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
  STRING_orkh0
  STRING_orya0
  STRING_osage0
  STRING_osge0
  STRING_osma0
  STRING_osmanya0
  STRING_ougr0
  STRING_p0
  STRING_pahawhhmong0
  STRING_palm0
  STRING_palmyrene0
  STRING_pauc0
  STRING_paucinhau0
  STRING_pc0
  STRING_pd0
  STRING_pe0
  STRING_perm0
  STRING_pf0
  STRING_phag0
  STRING_phagspa0
  STRING_phli0
  STRING_phlp0
  STRING_phnx0
  STRING_phoenician0
  STRING_pi0
  STRING_plrd0
  STRING_po0
  STRING_prti0
  STRING_ps0
  STRING_psalterpahlavi0
  STRING_rejang0
  STRING_rjng0
  STRING_rohg0
  STRING_runic0
  STRING_runr0
  STRING_s0
  STRING_samaritan0
  STRING_samr0
  STRING_sarb0
  STRING_saur0
  STRING_saurashtra0
  STRING_sc0
  STRING_sgnw0
  STRING_sharada0
  STRING_shavian0
  STRING_shaw0
  STRING_shrd0
  STRING_sidd0
  STRING_siddham0
  STRING_signwriting0
  STRING_sind0
  STRING_sinh0
  STRING_sinhala0
  STRING_sk0
  STRING_sm0
  STRING_so0
  STRING_sogd0
  STRING_sogdian0
  STRING_sogo0
  STRING_sora0
  STRING_sorasompeng0
  STRING_soyo0
  STRING_soyombo0
  STRING_sund0
  STRING_sundanese0
  STRING_sylo0
  STRING_sylotinagri0
  STRING_syrc0
  STRING_syriac0
  STRING_tagalog0
  STRING_tagb0
  STRING_tagbanwa0
  STRING_taile0
  STRING_taitham0
  STRING_taiviet0
  STRING_takr0
  STRING_takri0
  STRING_tale0
  STRING_talu0
  STRING_tamil0
  STRING_taml0
  STRING_tang0
  STRING_tangsa0
  STRING_tangut0
  STRING_tavt0
  STRING_telu0
  STRING_telugu0
  STRING_tfng0
  STRING_tglg0
  STRING_thaa0
  STRING_thaana0
  STRING_thai0
  STRING_tibetan0
  STRING_tibt0
  STRING_tifinagh0
  STRING_tirh0
  STRING_tirhuta0
  STRING_tngs0
  STRING_toto0
  STRING_ugar0
  STRING_ugaritic0
  STRING_unknown0
  STRING_vai0
  STRING_vaii0
  STRING_vith0
  STRING_vithkuqi0
  STRING_wancho0
  STRING_wara0
  STRING_warangciti0
  STRING_wcho0
  STRING_xan0
  STRING_xpeo0
  STRING_xps0
  STRING_xsp0
  STRING_xsux0
  STRING_xuc0
  STRING_xwd0
  STRING_yezi0
  STRING_yezidi0
  STRING_yi0
  STRING_yiii0
  STRING_z0
  STRING_zanabazarsquare0
  STRING_zanb0
  STRING_zinh0
  STRING_zl0
  STRING_zp0
  STRING_zs0
  STRING_zyyy0
  STRING_zzzz0;

const ucp_type_table PRIV(utt)[] = {
  {   0, PT_SCX, ucp_Adlam },
  {   6, PT_SCX, ucp_Adlam },
  {  11, PT_SC, ucp_Caucasian_Albanian },
  {  16, PT_SC, ucp_Ahom },
  {  21, PT_SC, ucp_Anatolian_Hieroglyphs },
  {  42, PT_ANY, 0 },
  {  46, PT_SCX, ucp_Arabic },
  {  51, PT_SCX, ucp_Arabic },
  {  58, PT_SC, ucp_Armenian },
  {  67, PT_SC, ucp_Imperial_Aramaic },
  {  72, PT_SC, ucp_Armenian },
  {  77, PT_SC, ucp_Avestan },
  {  85, PT_SC, ucp_Avestan },
  {  90, PT_SC, ucp_Balinese },
  {  95, PT_SC, ucp_Balinese },
  { 104, PT_SC, ucp_Bamum },
  { 109, PT_SC, ucp_Bamum },
  { 115, PT_SC, ucp_Bassa_Vah },
  { 120, PT_SC, ucp_Bassa_Vah },
  { 129, PT_SC, ucp_Batak },
  { 135, PT_SC, ucp_Batak },
  { 140, PT_SCX, ucp_Bengali },
  { 145, PT_SCX, ucp_Bengali },
  { 153, PT_SC, ucp_Bhaiksuki },
  { 163, PT_SC, ucp_Bhaiksuki },
  { 168, PT_BIDICL, ucp_bidiAL },
  { 175, PT_BIDICL, ucp_bidiAN },
  { 182, PT_BIDICL, ucp_bidiB },
  { 188, PT_BIDICL, ucp_bidiBN },
  { 195, PT_BIDICO, 0 },
  { 201, PT_BIDICO, 0 },
  { 213, PT_BIDICL, ucp_bidiCS },
  { 220, PT_BIDICL, ucp_bidiEN },
  { 227, PT_BIDICL, ucp_bidiES },
  { 234, PT_BIDICL, ucp_bidiET },
  { 241, PT_BIDICL, ucp_bidiFSI },
  { 249, PT_BIDICL, ucp_bidiL },
  { 255, PT_BIDICL, ucp_bidiLRE },
  { 263, PT_BIDICL, ucp_bidiLRI },
  { 271, PT_BIDICL, ucp_bidiLRO },
  { 279, PT_BIDICL, ucp_bidiNSM },
  { 287, PT_BIDICL, ucp_bidiON },
  { 294, PT_BIDICL, ucp_bidiPDF },
  { 302, PT_BIDICL, ucp_bidiPDI },
  { 310, PT_BIDICL, ucp_bidiR },
  { 316, PT_BIDICL, ucp_bidiRLE },
  { 324, PT_BIDICL, ucp_bidiRLI },
  { 332, PT_BIDICL, ucp_bidiRLO },
  { 340, PT_BIDICL, ucp_bidiS },
  { 346, PT_BIDICL, ucp_bidiWS },
  { 353, PT_SCX, ucp_Bopomofo },
  { 358, PT_SCX, ucp_Bopomofo },
  { 367, PT_SC, ucp_Brahmi },
  { 372, PT_SC, ucp_Brahmi },
  { 379, PT_SC, ucp_Braille },
  { 384, PT_SC, ucp_Braille },
  { 392, PT_SCX, ucp_Buginese },
  { 397, PT_SCX, ucp_Buginese },
  { 406, PT_SCX, ucp_Buhid },
  { 411, PT_SCX, ucp_Buhid },
  { 417, PT_GC, ucp_C },
  { 419, PT_SCX, ucp_Chakma },
  { 424, PT_SC, ucp_Canadian_Aboriginal },
  { 443, PT_SC, ucp_Canadian_Aboriginal },
  { 448, PT_SC, ucp_Carian },
  { 453, PT_SC, ucp_Carian },
  { 460, PT_SC, ucp_Caucasian_Albanian },
  { 478, PT_PC, ucp_Cc },
  { 481, PT_PC, ucp_Cf },
  { 484, PT_SCX, ucp_Chakma },
  { 491, PT_SC, ucp_Cham },
  { 496, PT_SC, ucp_Cherokee },
  { 501, PT_SC, ucp_Cherokee },
  { 510, PT_SC, ucp_Chorasmian },
  { 521, PT_SC, ucp_Chorasmian },
  { 526, PT_PC, ucp_Cn },
  { 529, PT_PC, ucp_Co },
  { 532, PT_SC, ucp_Common },
  { 539, PT_SCX, ucp_Coptic },
  { 544, PT_SCX, ucp_Coptic },
  { 551, PT_SCX, ucp_Cypro_Minoan },
  { 556, PT_SCX, ucp_Cypriot },
  { 561, PT_PC, ucp_Cs },
  { 564, PT_SC, ucp_Cuneiform },
  { 574, PT_SCX, ucp_Cypriot },
  { 582, PT_SCX, ucp_Cypro_Minoan },
  { 594, PT_SCX, ucp_Cyrillic },
  { 603, PT_SCX, ucp_Cyrillic },
  { 608, PT_SC, ucp_Deseret },
  { 616, PT_SCX, ucp_Devanagari },
  { 621, PT_SCX, ucp_Devanagari },
  { 632, PT_SC, ucp_Dives_Akuru },
  { 637, PT_SC, ucp_Dives_Akuru },
  { 648, PT_SCX, ucp_Dogra },
  { 653, PT_SCX, ucp_Dogra },
  { 659, PT_SC, ucp_Deseret },
  { 664, PT_SCX, ucp_Duployan },
  { 669, PT_SCX, ucp_Duployan },
  { 678, PT_SC, ucp_Egyptian_Hieroglyphs },
  { 683, PT_SC, ucp_Egyptian_Hieroglyphs },
  { 703, PT_SC, ucp_Elbasan },
  { 708, PT_SC, ucp_Elbasan },
  { 716, PT_SC, ucp_Elymaic },
  { 721, PT_SC, ucp_Elymaic },
  { 729, PT_SC, ucp_Ethiopic },
  { 734, PT_SC, ucp_Ethiopic },
  { 743, PT_SCX, ucp_Georgian },
  { 748, PT_SCX, ucp_Georgian },
  { 757, PT_SCX, ucp_Glagolitic },
  { 762, PT_SCX, ucp_Glagolitic },
  { 773, PT_SCX, ucp_Gunjala_Gondi },
  { 778, PT_SCX, ucp_Masaram_Gondi },
  { 783, PT_SC, ucp_Gothic },
  { 788, PT_SC, ucp_Gothic },
  { 795, PT_SCX, ucp_Grantha },
  { 800, PT_SCX, ucp_Grantha },
  { 808, PT_SCX, ucp_Greek },
  { 814, PT_SCX, ucp_Greek },
  { 819, PT_SCX, ucp_Gujarati },
  { 828, PT_SCX, ucp_Gujarati },
  { 833, PT_SCX, ucp_Gunjala_Gondi },
  { 846, PT_SCX, ucp_Gurmukhi },
  { 855, PT_SCX, ucp_Gurmukhi },
  { 860, PT_SCX, ucp_Han },
  { 864, PT_SCX, ucp_Hangul },
  { 869, PT_SCX, ucp_Hangul },
  { 876, PT_SCX, ucp_Han },
  { 881, PT_SCX, ucp_Hanifi_Rohingya },
  { 896, PT_SCX, ucp_Hanunoo },
  { 901, PT_SCX, ucp_Hanunoo },
  { 909, PT_SC, ucp_Hatran },
  { 914, PT_SC, ucp_Hatran },
  { 921, PT_SC, ucp_Hebrew },
  { 926, PT_SC, ucp_Hebrew },
  { 933, PT_SCX, ucp_Hiragana },
  { 938, PT_SCX, ucp_Hiragana },
  { 947, PT_SC, ucp_Anatolian_Hieroglyphs },
  { 952, PT_SC, ucp_Pahawh_Hmong },
  { 957, PT_SC, ucp_Nyiakeng_Puachue_Hmong },
  { 962, PT_SC, ucp_Old_Hungarian },
  { 967, PT_SC, ucp_Imperial_Aramaic },
  { 983, PT_SC, ucp_Inherited },
  { 993, PT_SC, ucp_Inscriptional_Pahlavi },
  { 1014, PT_SC, ucp_Inscriptional_Parthian },
  { 1036, PT_SC, ucp_Old_Italic },
  { 1041, PT_SCX, ucp_Javanese },
  { 1046, PT_SCX, ucp_Javanese },
  { 1055, PT_SCX, ucp_Kaithi },
  { 1062, PT_SCX, ucp_Kayah_Li },
  { 1067, PT_SCX, ucp_Katakana },
  { 1072, PT_SCX, ucp_Kannada },
  { 1080, PT_SCX, ucp_Katakana },
  { 1089, PT_SCX, ucp_Kayah_Li },
  { 1097, PT_SC, ucp_Kharoshthi },
  { 1102, PT_SC, ucp_Kharoshthi },
  { 1113, PT_SC, ucp_Khitan_Small_Script },
  { 1131, PT_SC, ucp_Khmer },
  { 1137, PT_SC, ucp_Khmer },
  { 1142, PT_SCX, ucp_Khojki },
  { 1147, PT_SCX, ucp_Khojki },
  { 1154, PT_SCX, ucp_Khudawadi },
  { 1164, PT_SC, ucp_Khitan_Small_Script },
  { 1169, PT_SCX, ucp_Kannada },
  { 1174, PT_SCX, ucp_Kaithi },
  { 1179, PT_GC, ucp_L },
  { 1181, PT_LAMP, 0 },
  { 1184, PT_SC, ucp_Tai_Tham },
  { 1189, PT_SC, ucp_Lao },
  { 1193, PT_SC, ucp_Lao },
  { 1198, PT_SCX, ucp_Latin },
  { 1204, PT_SCX, ucp_Latin },
  { 1209, PT_LAMP, 0 },
  { 1212, PT_SC, ucp_Lepcha },
  { 1217, PT_SC, ucp_Lepcha },
  { 1224, PT_SCX, ucp_Limbu },
  { 1229, PT_SCX, ucp_Limbu },
  { 1235, PT_SCX, ucp_Linear_A },
  { 1240, PT_SCX, ucp_Linear_B },
  { 1245, PT_SCX, ucp_Linear_A },
  { 1253, PT_SCX, ucp_Linear_B },
  { 1261, PT_SC, ucp_Lisu },
  { 1266, PT_PC, ucp_Ll },
  { 1269, PT_PC, ucp_Lm },
  { 1272, PT_PC, ucp_Lo },
  { 1275, PT_PC, ucp_Lt },
  { 1278, PT_PC, ucp_Lu },
  { 1281, PT_SC, ucp_Lycian },
  { 1286, PT_SC, ucp_Lycian },
  { 1293, PT_SC, ucp_Lydian },
  { 1298, PT_SC, ucp_Lydian },
  { 1305, PT_GC, ucp_M },
  { 1307, PT_SCX, ucp_Mahajani },
  { 1316, PT_SCX, ucp_Mahajani },
  { 1321, PT_SC, ucp_Makasar },
  { 1326, PT_SC, ucp_Makasar },
  { 1334, PT_SCX, ucp_Malayalam },
  { 1344, PT_SCX, ucp_Mandaic },
  { 1349, PT_SCX, ucp_Mandaic },
  { 1357, PT_SCX, ucp_Manichaean },
  { 1362, PT_SCX, ucp_Manichaean },
  { 1373, PT_SC, ucp_Marchen },
  { 1378, PT_SC, ucp_Marchen },
  { 1386, PT_SCX, ucp_Masaram_Gondi },
  { 1399, PT_PC, ucp_Mc },
  { 1402, PT_PC, ucp_Me },
  { 1405, PT_SC, ucp_Medefaidrin },
  { 1417, PT_SC, ucp_Medefaidrin },
  { 1422, PT_SC, ucp_Meetei_Mayek },
  { 1434, PT_SC, ucp_Mende_Kikakui },
  { 1439, PT_SC, ucp_Mende_Kikakui },
  { 1452, PT_SC, ucp_Meroitic_Cursive },
  { 1457, PT_SC, ucp_Meroitic_Hieroglyphs },
  { 1462, PT_SC, ucp_Meroitic_Cursive },
  { 1478, PT_SC, ucp_Meroitic_Hieroglyphs },
  { 1498, PT_SC, ucp_Miao },
  { 1503, PT_SCX, ucp_Malayalam },
  { 1508, PT_PC, ucp_Mn },
  { 1511, PT_SCX, ucp_Modi },
  { 1516, PT_SCX, ucp_Mongolian },
  { 1521, PT_SCX, ucp_Mongolian },
  { 1531, PT_SC, ucp_Mro },
  { 1535, PT_SC, ucp_Mro },
  { 1540, PT_SC, ucp_Meetei_Mayek },
  { 1545, PT_SCX, ucp_Multani },
  { 1550, PT_SCX, ucp_Multani },
  { 1558, PT_SCX, ucp_Myanmar },
  { 1566, PT_SCX, ucp_Myanmar },
  { 1571, PT_GC, ucp_N },
  { 1573, PT_SC, ucp_Nabataean },
  { 1583, PT_SCX, ucp_Nandinagari },
  { 1588, PT_SCX, ucp_Nandinagari },
  { 1600, PT_SC, ucp_Old_North_Arabian },
  { 1605, PT_SC, ucp_Nabataean },
  { 1610, PT_PC, ucp_Nd },
  { 1613, PT_SC, ucp_Newa },
  { 1618, PT_SC, ucp_New_Tai_Lue },
  { 1628, PT_SCX, ucp_Nko },
  { 1632, PT_SCX, ucp_Nko },
  { 1637, PT_PC, ucp_Nl },
  { 1640, PT_PC, ucp_No },
  { 1643, PT_SC, ucp_Nushu },
  { 1648, PT_SC, ucp_Nushu },
  { 1654, PT_SC, ucp_Nyiakeng_Puachue_Hmong },
  { 1675, PT_SC, ucp_Ogham },
  { 1680, PT_SC, ucp_Ogham },
  { 1686, PT_SC, ucp_Ol_Chiki },
  { 1694, PT_SC, ucp_Ol_Chiki },
  { 1699, PT_SC, ucp_Old_Hungarian },
  { 1712, PT_SC, ucp_Old_Italic },
  { 1722, PT_SC, ucp_Old_North_Arabian },
  { 1738, PT_SCX, ucp_Old_Permic },
  { 1748, PT_SC, ucp_Old_Persian },
  { 1759, PT_SC, ucp_Old_Sogdian },
  { 1770, PT_SC, ucp_Old_South_Arabian },
  { 1786, PT_SC, ucp_Old_Turkic },
  { 1796, PT_SCX, ucp_Old_Uyghur },
  { 1806, PT_SCX, ucp_Oriya },
  { 1812, PT_SC, ucp_Old_Turkic },
  { 1817, PT_SCX, ucp_Oriya },
  { 1822, PT_SC, ucp_Osage },
  { 1828, PT_SC, ucp_Osage },
  { 1833, PT_SC, ucp_Osmanya },
  { 1838, PT_SC, ucp_Osmanya },
  { 1846, PT_SCX, ucp_Old_Uyghur },
  { 1851, PT_GC, ucp_P },
  { 1853, PT_SC, ucp_Pahawh_Hmong },
  { 1865, PT_SC, ucp_Palmyrene },
  { 1870, PT_SC, ucp_Palmyrene },
  { 1880, PT_SC, ucp_Pau_Cin_Hau },
  { 1885, PT_SC, ucp_Pau_Cin_Hau },
  { 1895, PT_PC, ucp_Pc },
  { 1898, PT_PC, ucp_Pd },
  { 1901, PT_PC, ucp_Pe },
  { 1904, PT_SCX, ucp_Old_Permic },
  { 1909, PT_PC, ucp_Pf },
  { 1912, PT_SCX, ucp_Phags_Pa },
  { 1917, PT_SCX, ucp_Phags_Pa },
  { 1925, PT_SC, ucp_Inscriptional_Pahlavi },
  { 1930, PT_SCX, ucp_Psalter_Pahlavi },
  { 1935, PT_SC, ucp_Phoenician },
  { 1940, PT_SC, ucp_Phoenician },
  { 1951, PT_PC, ucp_Pi },
  { 1954, PT_SC, ucp_Miao },
  { 1959, PT_PC, ucp_Po },
  { 1962, PT_SC, ucp_Inscriptional_Parthian },
  { 1967, PT_PC, ucp_Ps },
  { 1970, PT_SCX, ucp_Psalter_Pahlavi },
  { 1985, PT_SC, ucp_Rejang },
  { 1992, PT_SC, ucp_Rejang },
  { 1997, PT_SCX, ucp_Hanifi_Rohingya },
  { 2002, PT_SC, ucp_Runic },
  { 2008, PT_SC, ucp_Runic },
  { 2013, PT_GC, ucp_S },
  { 2015, PT_SC, ucp_Samaritan },
  { 2025, PT_SC, ucp_Samaritan },
  { 2030, PT_SC, ucp_Old_South_Arabian },
  { 2035, PT_SC, ucp_Saurashtra },
  { 2040, PT_SC, ucp_Saurashtra },
  { 2051, PT_PC, ucp_Sc },
  { 2054, PT_SC, ucp_SignWriting },
  { 2059, PT_SCX, ucp_Sharada },
  { 2067, PT_SC, ucp_Shavian },
  { 2075, PT_SC, ucp_Shavian },
  { 2080, PT_SCX, ucp_Sharada },
  { 2085, PT_SC, ucp_Siddham },
  { 2090, PT_SC, ucp_Siddham },
  { 2098, PT_SC, ucp_SignWriting },
  { 2110, PT_SCX, ucp_Khudawadi },
  { 2115, PT_SCX, ucp_Sinhala },
  { 2120, PT_SCX, ucp_Sinhala },
  { 2128, PT_PC, ucp_Sk },
  { 2131, PT_PC, ucp_Sm },
  { 2134, PT_PC, ucp_So },
  { 2137, PT_SCX, ucp_Sogdian },
  { 2142, PT_SCX, ucp_Sogdian },
  { 2150, PT_SC, ucp_Old_Sogdian },
  { 2155, PT_SC, ucp_Sora_Sompeng },
  { 2160, PT_SC, ucp_Sora_Sompeng },
  { 2172, PT_SC, ucp_Soyombo },
  { 2177, PT_SC, ucp_Soyombo },
  { 2185, PT_SC, ucp_Sundanese },
  { 2190, PT_SC, ucp_Sundanese },
  { 2200, PT_SCX, ucp_Syloti_Nagri },
  { 2205, PT_SCX, ucp_Syloti_Nagri },
  { 2217, PT_SCX, ucp_Syriac },
  { 2222, PT_SCX, ucp_Syriac },
  { 2229, PT_SCX, ucp_Tagalog },
  { 2237, PT_SCX, ucp_Tagbanwa },
  { 2242, PT_SCX, ucp_Tagbanwa },
  { 2251, PT_SCX, ucp_Tai_Le },
  { 2257, PT_SC, ucp_Tai_Tham },
  { 2265, PT_SC, ucp_Tai_Viet },
  { 2273, PT_SCX, ucp_Takri },
  { 2278, PT_SCX, ucp_Takri },
  { 2284, PT_SCX, ucp_Tai_Le },
  { 2289, PT_SC, ucp_New_Tai_Lue },
  { 2294, PT_SCX, ucp_Tamil },
  { 2300, PT_SCX, ucp_Tamil },
  { 2305, PT_SC, ucp_Tangut },
  { 2310, PT_SC, ucp_Tangsa },
  { 2317, PT_SC, ucp_Tangut },
  { 2324, PT_SC, ucp_Tai_Viet },
  { 2329, PT_SCX, ucp_Telugu },
  { 2334, PT_SCX, ucp_Telugu },
  { 2341, PT_SC, ucp_Tifinagh },
  { 2346, PT_SCX, ucp_Tagalog },
  { 2351, PT_SCX, ucp_Thaana },
  { 2356, PT_SCX, ucp_Thaana },
  { 2363, PT_SC, ucp_Thai },
  { 2368, PT_SC, ucp_Tibetan },
  { 2376, PT_SC, ucp_Tibetan },
  { 2381, PT_SC, ucp_Tifinagh },
  { 2390, PT_SCX, ucp_Tirhuta },
  { 2395, PT_SCX, ucp_Tirhuta },
  { 2403, PT_SC, ucp_Tangsa },
  { 2408, PT_SC, ucp_Toto },
  { 2413, PT_SC, ucp_Ugaritic },
  { 2418, PT_SC, ucp_Ugaritic },
  { 2427, PT_SC, ucp_Unknown },
  { 2435, PT_SC, ucp_Vai },
  { 2439, PT_SC, ucp_Vai },
  { 2444, PT_SC, ucp_Vithkuqi },
  { 2449, PT_SC, ucp_Vithkuqi },
  { 2458, PT_SC, ucp_Wancho },
  { 2465, PT_SC, ucp_Warang_Citi },
  { 2470, PT_SC, ucp_Warang_Citi },
  { 2481, PT_SC, ucp_Wancho },
  { 2486, PT_ALNUM, 0 },
  { 2490, PT_SC, ucp_Old_Persian },
  { 2495, PT_PXSPACE, 0 },
  { 2499, PT_SPACE, 0 },
  { 2503, PT_SC, ucp_Cuneiform },
  { 2508, PT_UCNC, 0 },
  { 2512, PT_WORD, 0 },
  { 2516, PT_SCX, ucp_Yezidi },
  { 2521, PT_SCX, ucp_Yezidi },
  { 2528, PT_SCX, ucp_Yi },
  { 2531, PT_SCX, ucp_Yi },
  { 2536, PT_GC, ucp_Z },
  { 2538, PT_SC, ucp_Zanabazar_Square },
  { 2554, PT_SC, ucp_Zanabazar_Square },
  { 2559, PT_SC, ucp_Inherited },
  { 2564, PT_PC, ucp_Zl },
  { 2567, PT_PC, ucp_Zp },
  { 2570, PT_PC, ucp_Zs },
  { 2573, PT_SC, ucp_Common },
  { 2578, PT_SC, ucp_Unknown }
};

const size_t PRIV(utt_size) = sizeof(PRIV(utt)) / sizeof(ucp_type_table);

#endif /* SUPPORT_UNICODE */

/* End of pcre2_ucptables.c */
