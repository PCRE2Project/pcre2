#! /usr/bin/python

#                   PCRE2 UNICODE PROPERTY SUPPORT
#                   ------------------------------

# This file is a Python module containing common lists and functions for the
# GenerateXXX scripts that create various.c and .h files from Unicode data
# files. It was created as part of a re-organizaton of these scripts in
# December 2021.


# ---------------------------------------------------------------------------
#                             DATA LISTS
# ---------------------------------------------------------------------------

# The lists of script names and script abbreviations must be kept in step. Note
# that the pcre2pattern and pcre2syntax documentation has lists of scripts.

script_names = ['Unknown', 'Arabic', 'Armenian', 'Bengali', 'Bopomofo', 'Braille', 'Buginese', 'Buhid', 'Canadian_Aboriginal', \
 'Cherokee', 'Common', 'Coptic', 'Cypriot', 'Cyrillic', 'Deseret', 'Devanagari', 'Ethiopic', 'Georgian', \
 'Glagolitic', 'Gothic', 'Greek', 'Gujarati', 'Gurmukhi', 'Han', 'Hangul', 'Hanunoo', 'Hebrew', 'Hiragana', \
 'Inherited', 'Kannada', 'Katakana', 'Kharoshthi', 'Khmer', 'Lao', 'Latin', 'Limbu', 'Linear_B', 'Malayalam', \
 'Mongolian', 'Myanmar', 'New_Tai_Lue', 'Ogham', 'Old_Italic', 'Old_Persian', 'Oriya', 'Osmanya', 'Runic', \
 'Shavian', 'Sinhala', 'Syloti_Nagri', 'Syriac', 'Tagalog', 'Tagbanwa', 'Tai_Le', 'Tamil', 'Telugu', 'Thaana', \
 'Thai', 'Tibetan', 'Tifinagh', 'Ugaritic', 'Yi', \
 # New for Unicode 5.0
 'Balinese', 'Cuneiform', 'Nko', 'Phags_Pa', 'Phoenician', \
 # New for Unicode 5.1
 'Carian', 'Cham', 'Kayah_Li', 'Lepcha', 'Lycian', 'Lydian', 'Ol_Chiki', 'Rejang', 'Saurashtra', 'Sundanese', 'Vai', \
 # New for Unicode 5.2
 'Avestan', 'Bamum', 'Egyptian_Hieroglyphs', 'Imperial_Aramaic', \
 'Inscriptional_Pahlavi', 'Inscriptional_Parthian', \
 'Javanese', 'Kaithi', 'Lisu', 'Meetei_Mayek', \
 'Old_South_Arabian', 'Old_Turkic', 'Samaritan', 'Tai_Tham', 'Tai_Viet', \
 # New for Unicode 6.0.0
 'Batak', 'Brahmi', 'Mandaic', \
# New for Unicode 6.1.0
 'Chakma', 'Meroitic_Cursive', 'Meroitic_Hieroglyphs', 'Miao', 'Sharada', 'Sora_Sompeng', 'Takri',
# New for Unicode 7.0.0
 'Bassa_Vah', 'Caucasian_Albanian', 'Duployan', 'Elbasan', 'Grantha', 'Khojki', 'Khudawadi',
 'Linear_A', 'Mahajani', 'Manichaean', 'Mende_Kikakui', 'Modi', 'Mro', 'Nabataean',
 'Old_North_Arabian', 'Old_Permic', 'Pahawh_Hmong', 'Palmyrene', 'Psalter_Pahlavi',
 'Pau_Cin_Hau', 'Siddham', 'Tirhuta', 'Warang_Citi',
# New for Unicode 8.0.0
 'Ahom', 'Anatolian_Hieroglyphs', 'Hatran', 'Multani', 'Old_Hungarian',
 'SignWriting',
# New for Unicode 10.0.0
 'Adlam', 'Bhaiksuki', 'Marchen', 'Newa', 'Osage', 'Tangut', 'Masaram_Gondi',
 'Nushu', 'Soyombo', 'Zanabazar_Square',
# New for Unicode 11.0.0
  'Dogra', 'Gunjala_Gondi', 'Hanifi_Rohingya', 'Makasar', 'Medefaidrin',
  'Old_Sogdian', 'Sogdian',
# New for Unicode 12.0.0
  'Elymaic', 'Nandinagari', 'Nyiakeng_Puachue_Hmong', 'Wancho',
# New for Unicode 13.0.0
  'Chorasmian', 'Dives_Akuru', 'Khitan_Small_Script', 'Yezidi',
# New for Unicode 14.0.0
  'Cypro_Minoan', 'Old_Uyghur', 'Tangsa', 'Toto', 'Vithkuqi'
 ]

# BIDI classes in the DerivedBidiClass.txt file, with comments.

bidi_classes = [
  'AL',  'Arabic letter',
  'AN',  'Arabic number',
  'B',   'Paragraph separator',
  'BN',  'Boundary neutral',
  'CS',  'Common separator',
  'EN',  'European number',
  'ES',  'European separator',
  'ET',  'European terminator',
  'FSI', 'First strong isolate',
  'L',   'Left to right',
  'LRE', 'Left to right embedding',
  'LRI', 'Left to right isolate',
  'LRO', 'Left to right override',
  'NSM', 'Non-spacing mark',
  'ON',  'Other neutral',
  'PDF', 'Pop directional format',
  'PDI', 'Pop directional isolate',
  'R',   'Right to left',
  'RLE', 'Right to left embedding',
  'RLI', 'Right to left isolate',
  'RLO', 'Right to left override',
  'S',   'Segment separator',
  'WS',  'White space'
  ]

# Particular category property names, with comments. NOTE: If ever this list
# is changed, the table called "catposstab" in the pcre2_auto_possess.c file
# must be edited to keep in step.

category_names = [
  'Cc', 'Control',
  'Cf', 'Format',
  'Cn', 'Unassigned',
  'Co', 'Private use',
  'Cs', 'Surrogate',
  'Ll', 'Lower case letter',
  'Lm', 'Modifier letter',
  'Lo', 'Other letter',
  'Lt', 'Title case letter',
  'Lu', 'Upper case letter',
  'Mc', 'Spacing mark',
  'Me', 'Enclosing mark',
  'Mn', 'Non-spacing mark',
  'Nd', 'Decimal number',
  'Nl', 'Letter number',
  'No', 'Other number',
  'Pc', 'Connector punctuation',
  'Pd', 'Dash punctuation',
  'Pe', 'Close punctuation',
  'Pf', 'Final punctuation',
  'Pi', 'Initial punctuation',
  'Po', 'Other punctuation',
  'Ps', 'Open punctuation',
  'Sc', 'Currency symbol',
  'Sk', 'Modifier symbol',
  'Sm', 'Mathematical symbol',
  'So', 'Other symbol',
  'Zl', 'Line separator',
  'Zp', 'Paragraph separator',
  'Zs', 'Space separator'
  ]

# The Extended_Pictographic property is not found in the file where all the
# others are (GraphemeBreakProperty.txt). It comes from the emoji-data.txt
# file, but we list it here so that the name has the correct index value.

break_properties = [
  'CR',                    ' 0',
  'LF',                    ' 1',
  'Control',               ' 2',
  'Extend',                ' 3',
  'Prepend',               ' 4',
  'SpacingMark',           ' 5',
  'L',                     ' 6 Hangul syllable type L',
  'V',                     ' 7 Hangul syllable type V',
  'T',                     ' 8 Hangul syllable type T',
  'LV',                    ' 9 Hangul syllable type LV',
  'LVT',                   '10 Hangul syllable type LVT',
  'Regional_Indicator',    '11',
  'Other',                 '12',
  'ZWJ',                   '13',
  'Extended_Pictographic', '14'
  ]

# List of abbreviations for various properties

abbreviations = {
# Script abbreviations
  'Unknown': 'Zzzz',
  'Arabic': 'Arab',
  'Armenian': 'Armn',
  'Bengali': 'Beng',
  'Bopomofo': 'Bopo',
  'Braille': 'Brai',
  'Buginese': 'Bugi',
  'Buhid': 'Buhd',
  'Canadian_Aboriginal': 'Cans',
  'Cherokee': 'Cher',
  'Common': 'Zyyy',
  'Coptic': ('Copt', 'Qaac'),
  'Cypriot': 'Cprt',
  'Cyrillic': 'Cyrl',
  'Deseret': 'Dsrt',
  'Devanagari': 'Deva',
  'Ethiopic': 'Ethi',
  'Georgian': 'Geor',
  'Glagolitic': 'Glag',
  'Gothic': 'Goth',
  'Greek': 'Grek',
  'Gujarati': 'Gujr',
  'Gurmukhi': 'Guru',
  'Han': 'Hani',
  'Hangul': 'Hang',
  'Hanunoo': 'Hano',
  'Hebrew': 'Hebr',
  'Hiragana': 'Hira',
  'Inherited': ('Zinh', 'Qaai'),
  'Kannada': 'Knda',
  'Katakana': 'Kana',
  'Kharoshthi': 'Khar',
  'Khmer': 'Khmr',
  'Lao': 'Laoo',
  'Latin': 'Latn',
  'Limbu': 'Limb',
  'Linear_B': 'Linb',
  'Malayalam': 'Mlym',
  'Mongolian': 'Mong',
  'Myanmar': 'Mymr',
  'New_Tai_Lue': 'Talu',
  'Ogham': 'Ogam',
  'Old_Italic': 'Ital',
  'Old_Persian': 'Xpeo',
  'Oriya': 'Orya',
  'Osmanya': 'Osma',
  'Runic': 'Runr',
  'Shavian': 'Shaw',
  'Sinhala': 'Sinh',
  'Syloti_Nagri': 'Sylo',
  'Syriac': 'Syrc',
  'Tagalog': 'Tglg',
  'Tagbanwa': 'Tagb',
  'Tai_Le': 'Tale',
  'Tamil': 'Taml',
  'Telugu': 'Telu',
  'Thaana': 'Thaa',
  'Thai': (),
  'Tibetan': 'Tibt',
  'Tifinagh': 'Tfng',
  'Ugaritic': 'Ugar',
  'Yi': 'Yiii',
# New for Unicode 5.0
  'Balinese': 'Bali',
  'Cuneiform': 'Xsux',
  'Nko': 'Nkoo',
  'Phags_Pa': 'Phag',
  'Phoenician': 'Phnx',
# New for Unicode 5.1
  'Carian': 'Cari',
  'Cham': (),
  'Kayah_Li': 'Kali',
  'Lepcha': 'Lepc',
  'Lycian': 'Lyci',
  'Lydian': 'Lydi',
  'Ol_Chiki': 'Olck',
  'Rejang': 'Rjng',
  'Saurashtra': 'Saur',
  'Sundanese': 'Sund',
  'Vai': 'Vaii',
# New for Unicode 5.2
  'Avestan': 'Avst',
  'Bamum': 'Bamu',
  'Egyptian_Hieroglyphs': 'Egyp',
  'Imperial_Aramaic': 'Armi',
  'Inscriptional_Pahlavi': 'Phli',
  'Inscriptional_Parthian': 'Prti',
  'Javanese': 'Java',
  'Kaithi': 'Kthi',
  'Lisu': (),
  'Meetei_Mayek': 'Mtei',
  'Old_South_Arabian': 'Sarb',
  'Old_Turkic': 'Orkh',
  'Samaritan': 'Samr',
  'Tai_Tham': 'Lana',
  'Tai_Viet': 'Tavt',
# New for Unicode 6.0.0
  'Batak': 'Batk',
  'Brahmi': 'Brah',
  'Mandaic': 'Mand',
# New for Unicode 6.1.0
  'Chakma': 'Cakm',
  'Meroitic_Cursive': 'Merc',
  'Meroitic_Hieroglyphs': 'Mero',
  'Miao': 'Plrd',
  'Sharada': 'Shrd',
  'Sora_Sompeng': 'Sora',
  'Takri': 'Takr',
# New for Unicode 7.0.0
  'Bassa_Vah': 'Bass',
  'Caucasian_Albanian': 'Aghb',
  'Duployan': 'Dupl',
  'Elbasan': 'Elba',
  'Grantha': 'Gran',
  'Khojki': 'Khoj',
  'Khudawadi': 'Sind',
  'Linear_A': 'Lina',
  'Mahajani': 'Mahj',
  'Manichaean': 'Mani',
  'Mende_Kikakui': 'Mend',
  'Modi': (),
  'Mro': 'Mroo',
  'Nabataean': 'Nbat',
  'Old_North_Arabian': 'Narb',
  'Old_Permic': 'Perm',
  'Pahawh_Hmong': 'Hmng',
  'Palmyrene': 'Palm',
  'Psalter_Pahlavi': 'Phlp',
  'Pau_Cin_Hau': 'Pauc',
  'Siddham': 'Sidd',
  'Tirhuta': 'Tirh',
  'Warang_Citi': 'Wara',
# New for Unicode 8.0.0
  'Ahom': (),
  'Anatolian_Hieroglyphs': 'Hluw',
  'Hatran': 'Hatr',
  'Multani': 'Mult',
  'Old_Hungarian': 'Hung',
  'SignWriting': 'Sgnw',
# New for Unicode 10.0.0
  'Adlam': 'Adlm',
  'Bhaiksuki': 'Bhks',
  'Marchen': 'Marc',
  'Newa': (),
  'Osage': 'Osge',
  'Tangut': 'Tang',
  'Masaram_Gondi': 'Gonm',
  'Nushu': 'Nshu',
  'Soyombo': 'Soyo',
  'Zanabazar_Square': 'Zanb',
# New for Unicode 11.0.0
  'Dogra': 'Dogr',
  'Gunjala_Gondi': 'Gong',
  'Hanifi_Rohingya': 'Rohg',
  'Makasar': 'Maka',
  'Medefaidrin': 'Medf',
  'Old_Sogdian': 'Sogo',
  'Sogdian': 'Sogd',
# New for Unicode 12.0.0
  'Elymaic': 'Elym',
  'Nandinagari': 'Nand',
  'Nyiakeng_Puachue_Hmong': 'Hmnp',
  'Wancho': 'Wcho',
# New for Unicode 13.0.0
  'Chorasmian': 'Chrs',
  'Dives_Akuru': 'Diak',
  'Khitan_Small_Script': 'Kits',
  'Yezidi': 'Yezi',
# New for Unicode 14.0.0
  'Cypro_Minoan': 'Cpmn',
  'Old_Uyghur': 'Ougr',
  'Tangsa': 'Tngs',
  'Toto': (),
  'Vithkuqi': 'Vith',
  }

# Convert string abbreviations to tuples
for key in abbreviations:
  value = abbreviations[key]
  if isinstance(value, str):
    abbreviations[key] = (value,)

# ---------------------------------------------------------------------------
#                      REORDERING SCRIPT NAMES
# ---------------------------------------------------------------------------

import re

script_abbrevs = []

def reorder_scripts():
  global script_names
  global script_abbrevs

  for name in script_names:
    abbrevs = abbreviations[name]
    script_abbrevs.append(name if len(abbrevs) == 0 else abbrevs[0])

  extended_script_abbrevs = set()
  with open("Unicode.tables/ScriptExtensions.txt") as f:
    names_re = re.compile(r'^[0-9A-F]{4,6}(?:\.\.[0-9A-F]{4,6})? +; ([A-Za-z_ ]+) #')

    for line in f:
      match_obj = names_re.match(line)

      if match_obj == None:
        continue

      for name in match_obj.group(1).split(" "):
        extended_script_abbrevs.add(name)

  new_script_names = []
  new_script_abbrevs = []

  for idx, abbrev in enumerate(script_abbrevs):
    if abbrev in extended_script_abbrevs:
      new_script_names.append(script_names[idx])
      new_script_abbrevs.append(abbrev)

  for idx, abbrev in enumerate(script_abbrevs):
    if abbrev not in extended_script_abbrevs:
      new_script_names.append(script_names[idx])
      new_script_abbrevs.append(abbrev)

  script_names = new_script_names
  script_abbrevs = new_script_abbrevs

reorder_scripts()

# ---------------------------------------------------------------------------
#                         DERIVED LISTS
# ---------------------------------------------------------------------------

# Create general character property names from the first letters of the
# particular categories.

gcn_set = set(category_names[i][0] for i in range(0, len(category_names), 2))
general_category_names = list(gcn_set)
general_category_names.sort()


# ---------------------------------------------------------------------------
#                           FUNCTIONS
# ---------------------------------------------------------------------------

import sys

# Open an output file, using the command's argument or a default. Write common
# preliminary header information.

def open_output(default):
  if len(sys.argv) > 2:
    print('** Too many arguments: just give a file name')
    sys.exit(1)
  if len(sys.argv) == 2:
    output_name = sys.argv[1]
  else:
    output_name = default
  try:
    file = open(output_name, "w")
  except IOError:
    print ("** Couldn't open %s" % output_name)
    sys.exit(1)

  script_name = sys.argv[0]
  i = script_name.rfind('/')
  if i >= 0:
    script_name = script_name[i+1:]

  file.write("""\
/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
          New API code Copyright (c) 2016-2021 University of Cambridge

This module is auto-generated from Unicode data files. DO NOT EDIT MANUALLY!
""")

  file.write("Instead, modify the maint/%s script and run it to generate\n"
  "a new version of this code.\n\n" % script_name)

  file.write("""\
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
\n""")
  return file

# End of UcpCommon.py
