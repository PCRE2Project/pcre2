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

# ---------------------------------------------------------------------------
#                     COLLECTING PROPERTY NAMES
# ---------------------------------------------------------------------------

import re

script_names = ['Unknown']
abbreviations = {}

def collect_property_names():
  global script_names
  global abbreviations

  names_re = re.compile(r'^[0-9A-F]{4,6}(?:\.\.[0-9A-F]{4,6})? +; ([A-Za-z_]+) #')

  last_script_name = ""
  with open("Unicode.tables/Scripts.txt") as f:
    for line in f:
      match_obj = names_re.match(line)

      if match_obj == None or match_obj.group(1) == last_script_name:
        continue

      last_script_name = match_obj.group(1)
      script_names.append(last_script_name)

  # Sometimes there is comment in the line
  # so splitting around semicolon is not enough
  value_alias_re = re.compile(r' *([A-Za-z_]+) *; *([A-Za-z_]+) *; *([A-Za-z_]+)(?: *; *([A-Za-z_ ]+))?')

  with open("Unicode.tables/PropertyValueAliases.txt") as f:
    for line in f:
      match_obj = value_alias_re.match(line)

      if match_obj == None:
        continue

      if match_obj.group(1) == "sc":
        if match_obj.group(2) == match_obj.group(3):
          abbreviations[match_obj.group(3)] = ()
        elif match_obj.group(4) == None:
          abbreviations[match_obj.group(3)] = (match_obj.group(2),)
        else:
          abbreviations[match_obj.group(3)] = (match_obj.group(2), match_obj.group(4))

collect_property_names()

# ---------------------------------------------------------------------------
#                      REORDERING SCRIPT NAMES
# ---------------------------------------------------------------------------

script_abbrevs = []

def reorder_scripts():
  global script_names
  global script_abbrevs
  global abbreviations

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
