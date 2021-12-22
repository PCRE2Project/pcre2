#! /usr/bin/python

# Multistage table builder
# (c) Peter Kankowski, 2008

##############################################################################
# This script was submitted to the PCRE project by Peter Kankowski as part of
# the upgrading of Unicode property support. The new code speeds up property
# matching many times. The script is for the use of PCRE maintainers, to
# generate the pcre2_ucd.c file that contains a digested form of the Unicode
# data tables. A number of extensions have been added to the original script.
#
# The script has now been upgraded to Python 3 for PCRE2, and should be run in
# the maint subdirectory, using the command
#
# [python3] ./MultiStage2.py >../src/pcre2_ucd.c
#
# It requires eight Unicode data tables: DerivedBidiClass.txt,
# DerivedGeneralCategory.txt, GraphemeBreakProperty.txt, PropList.txt,
# Scripts.txt, ScriptExtensions.txt, CaseFolding.txt, and emoji-data.txt. These
# must be in the maint/Unicode.tables subdirectory.
#
# DerivedBidiClass.txt and DerivedGeneralCategory.txt are in the "extracted"
# subdirectory of the Unicode database (UCD) on the Unicode web site;
# GraphemeBreakProperty.txt is in the "auxiliary" subdirectory. PropList.txt,
# Scripts.txt, ScriptExtensions.txt, and CaseFolding.txt are directly in the
# UCD directory.
#
# The emoji-data.txt file is found in the "emoji" subdirectory even though it
# is technically part of a different (but coordinated) standard as shown
# in files associated with Unicode Technical Standard #51 ("Unicode Emoji"),
# for example:
#
# http://unicode.org/Public/emoji/13.0/ReadMe.txt
#
# -----------------------------------------------------------------------------
# Minor modifications made to this script:
#  Added #! line at start
#  Removed tabs
#  Made it work with Python 2.4 by rewriting two statements that needed 2.5
#  Consequent code tidy
#  Adjusted data file names to take from the Unicode.tables directory
#  Adjusted global table names by prefixing _pcre_.
#  Commented out stuff relating to the casefolding table, which isn't used;
#    removed completely in 2012.
#  Corrected size calculation
#  Add #ifndef SUPPORT_UCP to use dummy tables when no UCP support is needed.
#  Update for PCRE2: name changes, and SUPPORT_UCP is abolished.
#
# Major modifications made to this script:
#  Added code to add a grapheme break property field to records.
#
#  Added code to search for sets of more than two characters that must match
#  each other caselessly. A new table is output containing these sets, and
#  offsets into the table are added to the main output records. This new
#  code scans CaseFolding.txt instead of UnicodeData.txt, which is no longer
#  used.
#
#  Update for Python3:
#    . Processed with 2to3, but that didn't fix everything
#    . Changed string.strip to str.strip
#    . Added encoding='utf-8' to the open() call
#    . Inserted 'int' before blocksize/ELEMS_PER_LINE because an int is
#        required and the result of the division is a float
#
#  Added code to scan the emoji-data.txt file to find the Extended Pictographic
#  property, which is used by PCRE2 as a grapheme breaking property. This was
#  done when updating to Unicode 11.0.0 (July 2018).
#
#  Added code to add a Script Extensions field to records. This has increased
#  their size from 8 to 12 bytes, only 10 of which are currently used.
#
#  Added code to add a bidi class field to records by scanning the
#  DerivedBidiClass.txt and PropList.txt files. This uses one of the two spare
#  bytes, so now 11 out of 12 are in use.
#
# 01-March-2010:     Updated list of scripts for Unicode 5.2.0
# 30-April-2011:     Updated list of scripts for Unicode 6.0.0
#     July-2012:     Updated list of scripts for Unicode 6.1.0
# 20-August-2012:    Added scan of GraphemeBreakProperty.txt and added a new
#                      field in the record to hold the value. Luckily, the
#                      structure had a hole in it, so the resulting table is
#                      not much bigger than before.
# 18-September-2012: Added code for multiple caseless sets. This uses the
#                      final hole in the structure.
# 30-September-2012: Added RegionalIndicator break property from Unicode 6.2.0
# 13-May-2014:       Updated for PCRE2
# 03-June-2014:      Updated for Python 3
# 20-June-2014:      Updated for Unicode 7.0.0
# 12-August-2014:    Updated to put Unicode version into the file
# 19-June-2015:      Updated for Unicode 8.0.0
# 02-July-2017:      Updated for Unicode 10.0.0
# 03-July-2018:      Updated for Unicode 11.0.0
# 07-July-2018:      Added code to scan emoji-data.txt for the Extended
#                      Pictographic property.
# 01-October-2018:   Added the 'Unknown' script name
# 03-October-2018:   Added new field for Script Extensions
# 27-July-2019:      Updated for Unicode 12.1.0
# 10-March-2020:     Updated for Unicode 13.0.0
# PCRE2-10.39:       Updated for Unicode 14.0.0
# 05-December-2021:  Added code to scan DerivedBidiClass.txt for bidi class,
#                      and also PropList.txt for the Bidi_Control property
# 19-December-2021:  Reworked script extensions lists to be bit maps instead
#                      of zero-terminated lists of script numbers.
# ----------------------------------------------------------------------------
#
#
# The main tables generated by this script are used by macros defined in
# pcre2_internal.h. They look up Unicode character properties using short
# sequences of code that contains no branches, which makes for greater speed.
#
# Conceptually, there is a table of records (of type ucd_record), one for each
# Unicode character. Each record contains the script number, script extension
# value, character type, grapheme break type, offset to caseless matching set,
# offset to the character's other case, and the bidi class/control. However, a
# real table covering all Unicode characters would be far too big. It can be
# efficiently compressed by observing that many characters have the same
# record, and many blocks of characters (taking 128 characters in a block) have
# the same set of records as other blocks. This leads to a 2-stage lookup
# process.
#
# This script constructs six tables. The ucd_caseless_sets table contains
# lists of characters that all match each other caselessly. Each list is
# in order, and is terminated by NOTACHAR (0xffffffff), which is larger than
# any valid character. The first list is empty; this is used for characters
# that are not part of any list.
#
# The ucd_digit_sets table contains the code points of the '9' characters in
# each set of 10 decimal digits in Unicode. This is used to ensure that digits
# in script runs all come from the same set. The first element in the vector
# contains the number of subsequent elements, which are in ascending order.
#
# The ucd_script_sets vector contains bitmaps that represent lists of scripts
# for the Script Extensions properties of certain characters. Each bitmap
# consists of a fixed number of unsigned 32-bit numbers, enough to allocate
# a bit for every known script. A character with more than one script listed
# for its Script Extension property has a negative value in its record. This is
# the negated offset to the start of the relevant bitmap in the ucd_script_sets
# vector.
#
# The ucd_records table contains one instance of every unique record that is
# required. The ucd_stage1 table is indexed by a character's block number,
# which is the character's code point divided by 128, since 128 is the size
# of each block. The result of a lookup in ucd_stage1 a "virtual" block number.
#
# The ucd_stage2 table is a table of "virtual" blocks; each block is indexed by
# the offset of a character within its own block, and the result is the index
# number of the required record in the ucd_records vector.
#
# The following examples are correct for the Unicode 14.0.0 database. Future
# updates may make change the actual lookup values.
#
# Example: lowercase "a" (U+0061) is in block 0
#          lookup 0 in stage1 table yields 0
#          lookup 97 (0x61) in the first table in stage2 yields 22
#          record 22 is { 34, 5, 12, 0, -32, 34, 2, 0 }
#            34 = ucp_Latin   => Latin script
#             5 = ucp_Ll      => Lower case letter
#            12 = ucp_gbOther => Grapheme break property "Other"
#             0               => Not part of a caseless set
#           -32 (-0x20)       => Other case is U+0041
#            34 = ucp_Latin   => No special Script Extension property
#             2 = ucp_bidiL   => Bidi class left-to-right
#             0               => Dummy value, unused at present
#
# Almost all lowercase latin characters resolve to the same record. One or two
# are different because they are part of a multi-character caseless set (for
# example, k, K and the Kelvin symbol are such a set).
#
# Example: hiragana letter A (U+3042) is in block 96 (0x60)
#          lookup 96 in stage1 table yields 91
#          lookup 66 (0x42) in table 91 in stage2 yields 613
#          record 613 is { 27, 7, 12, 0, 0, 27, 2, 0 }
#            27 = ucp_Hiragana => Hiragana script
#             7 = ucp_Lo       => Other letter
#            12 = ucp_gbOther  => Grapheme break property "Other"
#             0                => Not part of a caseless set
#             0                => No other case
#            27 = ucp_Hiragana => No special Script Extension property
#             2 = ucp_bidiL    => Bidi class left-to-right
#             0                => Dummy value, unused at present
#
# Example: vedic tone karshana (U+1CD0) is in block 57 (0x39)
#          lookup 57 in stage1 table yields 55
#          lookup 80 (0x50) in table 55 in stage2 yields 485
#          record 485 is { 28, 12, 3, 0, 0, -122, 19, 0 }
#            28 = ucp_Inherited => Script inherited from predecessor
#            12 = ucp_Mn        => Non-spacing mark
#             3 = ucp_gbExtend  => Grapheme break property "Extend"
#             0                 => Not part of a caseless set
#             0                 => No other case
#          -228                 => Script Extension list offset = 228
#            13 = ucp_bidiNSM   => Bidi class non-spacing mark
#             0                 => Dummy value, unused at present
#
# At offset 228 in the ucd_script_sets vector we find a bitmap with bits 3, 15,
# 29, and 107 set. This means that this character is expected to be used with
# any of those scripts, which are Bengali, Devanagari, Kannada, and Grantha.
#
#  Philip Hazel, last updated 19 December 2021.
##############################################################################


import re
import string
import sys

MAX_UNICODE = 0x110000
NOTACHAR = 0xffffffff


# Parse a line of Scripts.txt, GraphemeBreakProperty.txt,
# DerivedBidiClass.txt or DerivedGeneralCategory.txt

def make_get_names(enum):
        return lambda chardata: enum.index(chardata[1])

# Parse a line of CaseFolding.txt

def get_other_case(chardata):
        if chardata[1] == 'C' or chardata[1] == 'S':
          return int(chardata[2], 16) - int(chardata[0], 16)
        return 0

# Parse a line of ScriptExtensions.txt

def get_script_extension(chardata):
        this_script_list = list(chardata[1].split(' '))
        if len(this_script_list) == 1:
          return script_abbrevs.index(this_script_list[0])

        script_numbers = []
        for d in this_script_list:
          script_numbers.append(script_abbrevs.index(d))
        script_numbers.append(0)
        script_numbers_length = len(script_numbers)

        for i in range(1, len(script_lists) - script_numbers_length + 1):
          for j in range(0, script_numbers_length):
            found = True
            if script_lists[i+j] != script_numbers[j]:
              found = False
              break
          if found:
            return -i

        # Not found in existing lists

        return_value = len(script_lists)
        script_lists.extend(script_numbers)
        return -return_value

# Read the whole table in memory, setting/checking the Unicode version

def read_table(file_name, get_value, default_value):
        global unicode_version

        f = re.match(r'^[^/]+/([^.]+)\.txt$', file_name)
        file_base = f.group(1)
        version_pat = r"^# " + re.escape(file_base) + r"-(\d+\.\d+\.\d+)\.txt$"
        file = open(file_name, 'r', encoding='utf-8')
        f = re.match(version_pat, file.readline())
        version = f.group(1)
        if unicode_version == "":
                unicode_version = version
        elif unicode_version != version:
                print("WARNING: Unicode version differs in %s", file_name, file=sys.stderr)

        table = [default_value] * MAX_UNICODE
        for line in file:
                line = re.sub(r'#.*', '', line)
                chardata = list(map(str.strip, line.split(';')))
                if len(chardata) <= 1:
                        continue
                value = get_value(chardata)
                m = re.match(r'([0-9a-fA-F]+)(\.\.([0-9a-fA-F]+))?$', chardata[0])
                char = int(m.group(1), 16)
                if m.group(3) is None:
                        last = char
                else:
                        last = int(m.group(3), 16)
                for i in range(char, last + 1):
                        # It is important not to overwrite a previously set
                        # value because in the CaseFolding file there are lines
                        # to be ignored (returning the default value of 0)
                        # which often come after a line which has already set
                        # data.
                        if table[i] == default_value:
                          table[i] = value
        file.close()
        return table

# Get the smallest possible C language type for the values
def get_type_size(table):
        type_size = [("uint8_t", 1), ("uint16_t", 2), ("uint32_t", 4),
                                 ("signed char", 1), ("pcre_int16", 2), ("pcre_int32", 4)]
        limits = [(0, 255), (0, 65535), (0, 4294967295),
                          (-128, 127), (-32768, 32767), (-2147483648, 2147483647)]
        minval = min(table)
        maxval = max(table)
        for num, (minlimit, maxlimit) in enumerate(limits):
                if minlimit <= minval and maxval <= maxlimit:
                        return type_size[num]
        else:
                raise OverflowError("Too large to fit into C types")

def get_tables_size(*tables):
        total_size = 0
        for table in tables:
                type, size = get_type_size(table)
                total_size += size * len(table)
        return total_size

# Compress the table into the two stages
def compress_table(table, block_size):
        blocks = {} # Dictionary for finding identical blocks
        stage1 = [] # Stage 1 table contains block numbers (indices into stage 2 table)
        stage2 = [] # Stage 2 table contains the blocks with property values
        table = tuple(table)
        for i in range(0, len(table), block_size):
                block = table[i:i+block_size]
                start = blocks.get(block)
                if start is None:
                        # Allocate a new block
                        start = len(stage2) / block_size
                        stage2 += block
                        blocks[block] = start
                stage1.append(start)

        return stage1, stage2

# Print a table
def print_table(out_file, table, table_name, block_size = None):
        type, size = get_type_size(table)
        ELEMS_PER_LINE = 16

        s = "const %s %s[] = { /* %d bytes" % (type, table_name, size * len(table))
        if block_size:
                s += ", block = %d" % block_size
        out_file.write(s + " */\n")
        table = tuple(table)
        if block_size is None:
                fmt = "%3d," * ELEMS_PER_LINE + " /* U+%04X */\n"
                mult = MAX_UNICODE / len(table)
                for i in range(0, len(table), ELEMS_PER_LINE):
                        out_file.write(fmt % (table[i:i+ELEMS_PER_LINE] +
                          (int(i * mult),)))
        else:
                if block_size > ELEMS_PER_LINE:
                        el = ELEMS_PER_LINE
                else:
                        el = block_size
                fmt = "%3d," * el + "\n"
                if block_size > ELEMS_PER_LINE:
                        fmt = fmt * int(block_size / ELEMS_PER_LINE)
                for i in range(0, len(table), block_size):
                        out_file.write(("/* block %d */\n" + fmt + "\n") % ((i / block_size,) + table[i:i+block_size]))
        out_file.write("};\n\n")

# Extract the unique combinations of properties into records
def combine_tables(*tables):
        records = {}
        index = []
        for t in zip(*tables):
                i = records.get(t)
                if i is None:
                        i = records[t] = len(records)
                index.append(i)
        return index, records

def get_record_size_struct(records):
        size = 0
        structure = '/* When recompiling tables with a new Unicode version, please check the\n' + \
        'types in this structure definition from pcre2_internal.h (the actual\n' + \
        'field names will be different):\n\ntypedef struct {\n'
        for i in range(len(records[0])):
                record_slice = [record[i] for record in records]
                slice_type, slice_size = get_type_size(record_slice)
                # add padding: round up to the nearest power of slice_size
                size = (size + slice_size - 1) & -slice_size
                size += slice_size
                structure += '%s property_%d;\n' % (slice_type, i)

        # round up to the first item of the next structure in array
        record_slice = [record[0] for record in records]
        slice_type, slice_size = get_type_size(record_slice)
        size = (size + slice_size - 1) & -slice_size

        structure += '} ucd_record;\n*/\n\n'
        return size, structure

def test_record_size():
        tests = [ \
          ( [(3,), (6,), (6,), (1,)], 1 ), \
          ( [(300,), (600,), (600,), (100,)], 2 ), \
          ( [(25, 3), (6, 6), (34, 6), (68, 1)], 2 ), \
          ( [(300, 3), (6, 6), (340, 6), (690, 1)], 4 ), \
          ( [(3, 300), (6, 6), (6, 340), (1, 690)], 4 ), \
          ( [(300, 300), (6, 6), (6, 340), (1, 690)], 4 ), \
          ( [(3, 100000), (6, 6), (6, 123456), (1, 690)], 8 ), \
          ( [(100000, 300), (6, 6), (123456, 6), (1, 690)], 8 ), \
        ]
        for test in tests:
            size, struct = get_record_size_struct(test[0])
            assert(size == test[1])
            #print struct

def print_records(out_file, records, record_size):
        out_file.write('const ucd_record PRIV(ucd_records)[] = { ' + \
                       '/* %d bytes, record size %d */\n' % (len(records) * record_size, record_size))

        records = list(zip(list(records.keys()), list(records.values())))
        records.sort(key = lambda x: x[1])
        for i, record in enumerate(records):
                out_file.write(('  {' + '%6d, ' * len(record[0]) + '}, /* %3d */\n') % (record[0] + (i,)))
        out_file.write('};\n\n')

script_names = ['Unknown', 'Arabic', 'Armenian', 'Bengali', 'Bopomofo', 'Braille', 'Buginese', 'Buhid', 'Canadian_Aboriginal',
 'Cherokee', 'Common', 'Coptic', 'Cypriot', 'Cyrillic', 'Deseret', 'Devanagari', 'Ethiopic', 'Georgian',
 'Glagolitic', 'Gothic', 'Greek', 'Gujarati', 'Gurmukhi', 'Han', 'Hangul', 'Hanunoo', 'Hebrew', 'Hiragana',
 'Inherited', 'Kannada', 'Katakana', 'Kharoshthi', 'Khmer', 'Lao', 'Latin', 'Limbu', 'Linear_B', 'Malayalam',
 'Mongolian', 'Myanmar', 'New_Tai_Lue', 'Ogham', 'Old_Italic', 'Old_Persian', 'Oriya', 'Osmanya', 'Runic',
 'Shavian', 'Sinhala', 'Syloti_Nagri', 'Syriac', 'Tagalog', 'Tagbanwa', 'Tai_Le', 'Tamil', 'Telugu', 'Thaana',
 'Thai', 'Tibetan', 'Tifinagh', 'Ugaritic', 'Yi',
# New for Unicode 5.0
 'Balinese', 'Cuneiform', 'Nko', 'Phags_Pa', 'Phoenician',
# New for Unicode 5.1
 'Carian', 'Cham', 'Kayah_Li', 'Lepcha', 'Lycian', 'Lydian', 'Ol_Chiki', 'Rejang', 'Saurashtra', 'Sundanese', 'Vai',
# New for Unicode 5.2
 'Avestan', 'Bamum', 'Egyptian_Hieroglyphs', 'Imperial_Aramaic',
 'Inscriptional_Pahlavi', 'Inscriptional_Parthian',
 'Javanese', 'Kaithi', 'Lisu', 'Meetei_Mayek',
 'Old_South_Arabian', 'Old_Turkic', 'Samaritan', 'Tai_Tham', 'Tai_Viet',
# New for Unicode 6.0.0
 'Batak', 'Brahmi', 'Mandaic',
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

script_abbrevs = [
  'Zzzz', 'Arab', 'Armn', 'Beng', 'Bopo', 'Brai', 'Bugi', 'Buhd', 'Cans',
  'Cher', 'Zyyy', 'Copt', 'Cprt', 'Cyrl', 'Dsrt', 'Deva', 'Ethi', 'Geor',
  'Glag', 'Goth', 'Grek', 'Gujr', 'Guru', 'Hani', 'Hang', 'Hano', 'Hebr',
  'Hira', 'Zinh', 'Knda', 'Kana', 'Khar', 'Khmr', 'Laoo', 'Latn', 'Limb',
  'Linb', 'Mlym', 'Mong', 'Mymr', 'Talu', 'Ogam', 'Ital', 'Xpeo', 'Orya',
  'Osma', 'Runr', 'Shaw', 'Sinh', 'Sylo', 'Syrc', 'Tglg', 'Tagb', 'Tale',
  'Taml', 'Telu', 'Thaa', 'Thai', 'Tibt', 'Tfng', 'Ugar', 'Yiii',
#New for Unicode 5.0
  'Bali', 'Xsux', 'Nkoo', 'Phag', 'Phnx',
#New for Unicode 5.1
  'Cari', 'Cham', 'Kali', 'Lepc', 'Lyci', 'Lydi', 'Olck', 'Rjng', 'Saur',
  'Sund', 'Vaii',
#New for Unicode 5.2
  'Avst', 'Bamu', 'Egyp', 'Armi', 'Phli', 'Prti', 'Java', 'Kthi', 'Lisu',
  'Mtei', 'Sarb', 'Orkh', 'Samr', 'Lana', 'Tavt',
#New for Unicode 6.0.0
  'Batk', 'Brah', 'Mand',
#New for Unicode 6.1.0
  'Cakm', 'Merc', 'Mero', 'Plrd', 'Shrd', 'Sora', 'Takr',
#New for Unicode 7.0.0
  'Bass', 'Aghb', 'Dupl', 'Elba', 'Gran', 'Khoj', 'Sind', 'Lina', 'Mahj',
  'Mani', 'Mend', 'Modi', 'Mroo', 'Nbat', 'Narb', 'Perm', 'Hmng', 'Palm',
  'Phlp', 'Pauc', 'Sidd', 'Tirh', 'Wara',
#New for Unicode 8.0.0
  'Ahom', 'Hluw', 'Hatr', 'Mult', 'Hung', 'Sgnw',
#New for Unicode 10.0.0
  'Adlm', 'Bhks', 'Marc', 'Newa', 'Osge', 'Tang', 'Gonm', 'Nshu', 'Soyo',
  'Zanb',
#New for Unicode 11.0.0
  'Dogr', 'Gong', 'Rohg', 'Maka', 'Medf', 'Sogo', 'Sogd',
#New for Unicode 12.0.0
  'Elym', 'Nand', 'Hmnp', 'Wcho',
#New for Unicode 13.0.0
  'Chrs', 'Diak', 'Kits', 'Yezi',
#New for Unicode 14.0.0
  'Cpmn', 'Ougr', 'Tngs', 'Toto', 'Vith'
 ]

category_names = ['Cc', 'Cf', 'Cn', 'Co', 'Cs', 'Ll', 'Lm', 'Lo', 'Lt', 'Lu',
  'Mc', 'Me', 'Mn', 'Nd', 'Nl', 'No', 'Pc', 'Pd', 'Pe', 'Pf', 'Pi', 'Po', 'Ps',
  'Sc', 'Sk', 'Sm', 'So', 'Zl', 'Zp', 'Zs' ]

# The Extended_Pictographic property is not found in the file where all the
# others are (GraphemeBreakProperty.txt). It comes from the emoji-data.txt
# file, but we list it here so that the name has the correct index value.

break_property_names = ['CR', 'LF', 'Control', 'Extend', 'Prepend',
  'SpacingMark', 'L', 'V', 'T', 'LV', 'LVT', 'Regional_Indicator', 'Other',
  'ZWJ', 'Extended_Pictographic' ]

# BIDI class property names in the DerivedBidiClass.txt file

bidiclass_names = ['AL', 'AN', 'B', 'BN', 'CS', 'EN', 'ES', 'ET', 'FSI', 'L',
  'LRE', 'LRI', 'LRO', 'NSM', 'ON',  'PDF', 'PDI', 'R', 'RLE', 'RLI', 'RLO',
  'S', 'WS' ]

# Create the various tables

test_record_size()
unicode_version = ""

script = read_table('Unicode.tables/Scripts.txt', make_get_names(script_names), script_names.index('Unknown'))
category = read_table('Unicode.tables/DerivedGeneralCategory.txt', make_get_names(category_names), category_names.index('Cn'))
break_props = read_table('Unicode.tables/GraphemeBreakProperty.txt', make_get_names(break_property_names), break_property_names.index('Other'))
other_case = read_table('Unicode.tables/CaseFolding.txt', get_other_case, 0)
bidi_class = read_table('Unicode.tables/DerivedBidiClass.txt', make_get_names(bidiclass_names), bidiclass_names.index('L'))

# The Bidi_Control property is a Y/N value, so needs only one bit. We scan the
# PropList.txt file and set 0x80 bit in the bidi_class table.

file = open('Unicode.tables/PropList.txt', 'r', encoding='utf-8')
for line in file:
        line = re.sub(r'#.*', '', line)
        chardata = list(map(str.strip, line.split(';')))
        if len(chardata) <= 1:
                continue
        if chardata[1] != "Bidi_Control":
                continue
        m = re.match(r'([0-9a-fA-F]+)(\.\.([0-9a-fA-F]+))?$', chardata[0])
        char = int(m.group(1), 16)
        if m.group(3) is None:
                last = char
        else:
                last = int(m.group(3), 16)
        for i in range(char, last + 1):
                bidi_class[i] |= 0x80;
file.close()

# The grapheme breaking rules were changed for Unicode 11.0.0 (June 2018). Now
# we need to find the Extended_Pictographic property for emoji characters. This
# can be set as an additional grapheme break property, because the default for
# all the emojis is "other". We scan the emoji-data.txt file and modify the
# break-props table.

file = open('Unicode.tables/emoji-data.txt', 'r', encoding='utf-8')
for line in file:
        line = re.sub(r'#.*', '', line)
        chardata = list(map(str.strip, line.split(';')))
        if len(chardata) <= 1:
                continue
        if chardata[1] != "Extended_Pictographic":
                continue
        m = re.match(r'([0-9a-fA-F]+)(\.\.([0-9a-fA-F]+))?$', chardata[0])
        char = int(m.group(1), 16)
        if m.group(3) is None:
                last = char
        else:
                last = int(m.group(3), 16)
        for i in range(char, last + 1):
                if break_props[i] != break_property_names.index('Other'):
                   print("WARNING: Emoji 0x%x has break property %s, not 'Other'",
                     i, break_property_names[break_props[i]], file=sys.stderr)
                break_props[i] = break_property_names.index('Extended_Pictographic')
file.close()

# The Script Extensions property default value is the Script value. Parse the
# file, setting 'Unknown' as the default (this will never be a Script Extension
# value), then scan it and fill in the default from Scripts. Code added by PH
# in October 2018. Positive values are used for just a single script for a
# code point. Negative values are negated offsets in a list of bitsets of
# multiple scripts. Initialize this list with a single entry, as the zeroth
# element is never used.

script_lists = [0]
script_abbrevs_default = script_abbrevs.index('Zzzz')
scriptx = read_table('Unicode.tables/ScriptExtensions.txt', get_script_extension, script_abbrevs_default)

# Scan all characters and set their default script extension to the main
# script. We also have to adjust negative scriptx values, following a change in
# the way these work. They are currently negated offsets into the script_lists
# list, but have to be changed into indices in the new ucd_script_sets vector,
# which has fixed-size entries. We can compute the new offset by counting the
# zeros that precede the current offset.

for i in range(0, MAX_UNICODE):
  if scriptx[i] == script_abbrevs_default:
    scriptx[i] = script[i]
  elif scriptx[i] < 0:
    count = 1
    for j in range(-scriptx[i], 0, -1):
      if script_lists[j] == 0:
        count += 1
    scriptx[i] = -count * (int(len(script_names)/32) + 1)

# With the addition of the Script Extensions field, we needed some padding to
# get the Unicode records up to 12 bytes (multiple of 4). Originally this was a
# 16-bit field and padding_dummy[0] was set to 256 to ensure this, but 8 bits
# are now used for the bidi class, so zero will do.

padding_dummy = [0] * MAX_UNICODE
padding_dummy[0] = 0

# This block of code was added by PH in September 2012. I am not a Python
# programmer, so the style is probably dreadful, but it does the job. It scans
# the other_case table to find sets of more than two characters that must all
# match each other caselessly. Later in this script a table of these sets is
# written out. However, we have to do this work here in order to compute the
# offsets in the table that are inserted into the main table.

# The CaseFolding.txt file lists pairs, but the common logic for reading data
# sets only one value, so first we go through the table and set "return"
# offsets for those that are not already set.

for c in range(MAX_UNICODE):
  if other_case[c] != 0 and other_case[c + other_case[c]] == 0:
    other_case[c + other_case[c]] = -other_case[c]

# Now scan again and create equivalence sets.

sets = []

for c in range(MAX_UNICODE):
  o = c + other_case[c]

  # Trigger when this character's other case does not point back here. We
  # now have three characters that are case-equivalent.

  if other_case[o] != -other_case[c]:
    t = o + other_case[o]

    # Scan the existing sets to see if any of the three characters are already
    # part of a set. If so, unite the existing set with the new set.

    appended = 0
    for s in sets:
      found = 0
      for x in s:
        if x == c or x == o or x == t:
          found = 1

      # Add new characters to an existing set

      if found:
        found = 0
        for y in [c, o, t]:
          for x in s:
            if x == y:
              found = 1
          if not found:
            s.append(y)
        appended = 1

    # If we have not added to an existing set, create a new one.

    if not appended:
      sets.append([c, o, t])

# End of loop looking for caseless sets.

# Now scan the sets and set appropriate offsets for the characters.

caseless_offsets = [0] * MAX_UNICODE

offset = 1;
for s in sets:
  for x in s:
    caseless_offsets[x] = offset
  offset += len(s) + 1

# End of block of code for creating offsets for caseless matching sets.


# Combine the tables

table, records = combine_tables(script, category, break_props,
  caseless_offsets, other_case, scriptx, bidi_class, padding_dummy)

record_size, record_struct = get_record_size_struct(list(records.keys()))

# Find the optimum block size for the two-stage table
min_size = sys.maxsize
for block_size in [2 ** i for i in range(5,10)]:
        size = len(records) * record_size
        stage1, stage2 = compress_table(table, block_size)
        size += get_tables_size(stage1, stage2)
        #print "/* block size %5d  => %5d bytes */" % (block_size, size)
        if size < min_size:
                min_size = size
                min_stage1, min_stage2 = stage1, stage2
                min_block_size = block_size

ucd_file = open("pcre2_ucd.c", "w")
ucp_file = open("pcre2_ucp.h", "w")

for out_file in (ucd_file, ucp_file):
    out_file.write("/*************************************************\n")
    out_file.write("*      Perl-Compatible Regular Expressions       *\n")
    out_file.write("*************************************************/\n")
    out_file.write("\n")
    out_file.write("/* PCRE is a library of functions to support regular expressions whose syntax\n")
    out_file.write("and semantics are as close as possible to those of the Perl 5 language.\n")
    out_file.write("\n")
    out_file.write("                       Written by Philip Hazel\n")
    out_file.write("     Original API code Copyright (c) 1997-2012 University of Cambridge\n")
    out_file.write("          New API code Copyright (c) 2016-2021 University of Cambridge\n")
    out_file.write("\n")
    out_file.write("-----------------------------------------------------------------------------\n")
    out_file.write("Redistribution and use in source and binary forms, with or without\n")
    out_file.write("modification, are permitted provided that the following conditions are met:\n")
    out_file.write("\n")
    out_file.write("    * Redistributions of source code must retain the above copyright notice,\n")
    out_file.write("      this list of conditions and the following disclaimer.\n")
    out_file.write("\n")
    out_file.write("    * Redistributions in binary form must reproduce the above copyright\n")
    out_file.write("      notice, this list of conditions and the following disclaimer in the\n")
    out_file.write("      documentation and/or other materials provided with the distribution.\n")
    out_file.write("\n")
    out_file.write("    * Neither the name of the University of Cambridge nor the names of its\n")
    out_file.write("      contributors may be used to endorse or promote products derived from\n")
    out_file.write("      this software without specific prior written permission.\n")
    out_file.write("\n")
    out_file.write("THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"\n")
    out_file.write("AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n")
    out_file.write("IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n")
    out_file.write("ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE\n")
    out_file.write("LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR\n")
    out_file.write("CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF\n")
    out_file.write("SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS\n")
    out_file.write("INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN\n")
    out_file.write("CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)\n")
    out_file.write("ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE\n")
    out_file.write("POSSIBILITY OF SUCH DAMAGE.\n")
    out_file.write("-----------------------------------------------------------------------------\n")
    out_file.write("*/\n")
    out_file.write("\n")
    out_file.write("/* This module is generated by the maint/MultiStage2.py script.\n")
    out_file.write("Do not modify it by hand. Instead modify the script and run it\n")
    out_file.write("to regenerate this code.\n")
    out_file.write("\n")

ucp_file.write("This file contains definitions of the property values that\n")
ucp_file.write("are returned bythe UCD access macros.\n")
ucp_file.write("\n")
ucp_file.write("IMPORTANT: The specific values of the first two enums are assumed\n")
ucp_file.write("for the table called catposstab in pcre2_compile.c. */\n")
ucp_file.write("\n")
ucp_file.write("\n")
ucp_file.write("#ifndef PCRE2_UCP_H_IDEMPOTENT_GUARD\n")
ucp_file.write("#define PCRE2_UCP_H_IDEMPOTENT_GUARD\n")
ucp_file.write("\n")
ucp_file.write("/* These are the general character categories. */\n")
ucp_file.write("\n")
ucp_file.write("enum {\n")
prev_cathegory = ""
for i in category_names:
    cathegory = i[0:1]
    if cathegory == prev_cathegory:
        continue
    prev_cathegory = cathegory
    ucp_file.write("  ucp_%s,\n" % cathegory)
ucp_file.write("};\n")
ucp_file.write("\n")
ucp_file.write("/* These are the particular character categories. */\n")
ucp_file.write("\n")
ucp_file.write("enum {\n")
for i in category_names:
    ucp_file.write("  ucp_%s,\n" % i)
ucp_file.write("};\n")
ucp_file.write("\n")
ucp_file.write("/* These are the bidi class values. */\n")
ucp_file.write("\n")
ucp_file.write("enum {\n")
for i in bidiclass_names:
    ucp_file.write("  ucp_bidi%s,\n" % i)
ucp_file.write("};\n")
ucp_file.write("\n")
ucp_file.write("/* These are grapheme break properties. The Extended Pictographic property\n")
ucp_file.write("comes from the emoji-data.txt file. */\n")
ucp_file.write("\n")
ucp_file.write("enum {\n")
for i in break_property_names:
    ucp_file.write("  ucp_gb%s,\n" % i)
ucp_file.write("};\n")
ucp_file.write("\n")
ucp_file.write("/* These are the script identifications. */\n")
ucp_file.write("\n")
ucp_file.write("enum {\n")
for i in script_names:
    ucp_file.write("  ucp_%s,\n" % i)
ucp_file.write("\n")
ucp_file.write("  /* This must be last */\n")
ucp_file.write("  ucp_Script_Count\n")
ucp_file.write("};\n")
ucp_file.write("\n")
ucp_file.write("#endif  /* PCRE2_UCP_H_IDEMPOTENT_GUARD */\n")
ucp_file.write("\n")
ucp_file.write("/* End of pcre2_ucp.h */\n")

ucd_file.write("As well as being part of the PCRE2 library, this module is #included\n")
ucd_file.write("by the pcre2test program, which redefines the PRIV macro to change\n")
ucd_file.write("table names from _pcre2_xxx to xxxx, thereby avoiding name clashes\n")
ucd_file.write("with the library. At present, just one of these tables is actually\n")
ucd_file.write("needed. */\n")
ucd_file.write("\n")
ucd_file.write("#ifndef PCRE2_PCRE2TEST\n")
ucd_file.write("\n")
ucd_file.write("#ifdef HAVE_CONFIG_H\n")
ucd_file.write("#include \"config.h\"\n")
ucd_file.write("#endif\n")
ucd_file.write("\n")
ucd_file.write("#include \"pcre2_internal.h\"\n")
ucd_file.write("\n")
ucd_file.write("#endif /* PCRE2_PCRE2TEST */\n")
ucd_file.write("\n")
ucd_file.write("/* Unicode character database. */\n")
ucd_file.write("/* This file was autogenerated by the MultiStage2.py script. */\n")
ucd_file.write("/* Total size: %d bytes, block size: %d. */\n" % (min_size, min_block_size))
ucd_file.write("\n")
ucd_file.write("/* The tables herein are needed only when UCP support is built,\n")
ucd_file.write("and in PCRE2 that happens automatically with UTF support.\n")
ucd_file.write("This module should not be referenced otherwise, so\n")
ucd_file.write("it should not matter whether it is compiled or not. However\n")
ucd_file.write("a comment was received about space saving - maybe the guy linked\n")
ucd_file.write("all the modules rather than using a library - so we include a\n")
ucd_file.write("condition to cut out the tables when not needed. But don't leave\n")
ucd_file.write("a totally empty module because some compilers barf at that.\n")
ucd_file.write("Instead, just supply some small dummy tables. */\n")
ucd_file.write("\n")
ucd_file.write("#ifndef SUPPORT_UNICODE\n")
ucd_file.write("const ucd_record PRIV(ucd_records)[] = {{0,0,0,0,0,0,0,0 }};\n")
ucd_file.write("const uint16_t PRIV(ucd_stage1)[] = {0};\n")
ucd_file.write("const uint16_t PRIV(ucd_stage2)[] = {0};\n")
ucd_file.write("const uint32_t PRIV(ucd_caseless_sets)[] = {0};\n")
ucd_file.write("#else\n")
ucd_file.write("\n")
ucd_file.write("const char *PRIV(unicode_version) = \"{}\";\n".format(unicode_version))
ucd_file.write("\n")
ucd_file.write("/* If the 32-bit library is run in non-32-bit mode, character values\n")
ucd_file.write("greater than 0x10ffff may be encountered. For these we set up a\n")
ucd_file.write("special record. */\n")
ucd_file.write("\n")
ucd_file.write("#if PCRE2_CODE_UNIT_WIDTH == 32\n")
ucd_file.write("const ucd_record PRIV(dummy_ucd_record)[] = {{\n")
ucd_file.write("  ucp_Unknown,    /* script */\n")
ucd_file.write("  ucp_Cn,         /* type unassigned */\n")
ucd_file.write("  ucp_gbOther,    /* grapheme break property */\n")
ucd_file.write("  0,              /* case set */\n")
ucd_file.write("  0,              /* other case */\n")
ucd_file.write("  ucp_Unknown,    /* script extension */\n")
ucd_file.write("  ucp_bidiL,      /* bidi class */\n")
ucd_file.write("  0,              /* dummy filler */\n")
ucd_file.write("  }};\n")
ucd_file.write("#endif\n")
ucd_file.write("\n")
ucd_file.write(record_struct)

# --- Added by PH: output the table of caseless character sets ---

ucd_file.write("/* This table contains lists of characters that are caseless sets of\n")
ucd_file.write("more than one character. Each list is terminated by NOTACHAR. */\n\n")

ucd_file.write("const uint32_t PRIV(ucd_caseless_sets)[] = {\n")
ucd_file.write("  NOTACHAR,\n")
for s in sets:
  s = sorted(s)
  for x in s:
    ucd_file.write('  0x%04x, ' % x)
  ucd_file.write('  NOTACHAR,\n')
ucd_file.write('};\n')
ucd_file.write("\n")

# ------

ucd_file.write("/* When #included in pcre2test, we don't need the table of digit\n")
ucd_file.write("sets, nor the the large main UCD tables. */\n")
ucd_file.write("\n")
ucd_file.write("#ifndef PCRE2_PCRE2TEST\n")
ucd_file.write("\n")

# --- Added by PH: read Scripts.txt again for the sets of 10 digits. ---

digitsets = []
file = open('Unicode.tables/Scripts.txt', 'r', encoding='utf-8')

for line in file:
  m = re.match(r'([0-9a-fA-F]+)\.\.([0-9a-fA-F]+)\s+;\s+\S+\s+#\s+Nd\s+', line)
  if m is None:
    continue
  first = int(m.group(1),16)
  last  = int(m.group(2),16)
  if ((last - first + 1) % 10) != 0:
    print("ERROR: %04x..%04x does not contain a multiple of 10 characters" % (first, last),
      file=sys.stderr)
  while first < last:
    digitsets.append(first + 9)
    first += 10
file.close()
digitsets.sort()

ucd_file.write("/* This table lists the code points for the '9' characters in each\n")
ucd_file.write("set of decimal digits. It is used to ensure that all the digits in\n")
ucd_file.write("a script run come from the same set. */\n\n")
ucd_file.write("const uint32_t PRIV(ucd_digit_sets)[] = {\n")

ucd_file.write("  %d,  /* Number of subsequent values */" % len(digitsets))
count = 8
for d in digitsets:
  if count == 8:
    ucd_file.write("\n ")
    count = 0
  ucd_file.write(" 0x%05x," % d)
  count += 1
ucd_file.write("\n};\n\n")

ucd_file.write("/* This vector is a list of script bitsets for the Script Extension\n")
ucd_file.write("property. */\n\n")
ucd_file.write("const uint32_t PRIV(ucd_script_sets)[] = {\n")

bitword_count = len(script_names)/32 + 1
bitwords = [0] * int(bitword_count)

for d in script_lists:
  if d == 0:
    s = " "
    ucd_file.write("  ")
    for x in bitwords:
      ucd_file.write("%s" %s)
      s = ", "
      ucd_file.write("0x%08xu" % x)
    ucd_file.write(",\n")
    bitwords = [0] * int(bitword_count)

  else:
    x = int(d/32)
    y = int(d%32)
    bitwords[x] = bitwords[x] | (1 << y)

ucd_file.write("};\n\n")

# Output the main UCD tables.

ucd_file.write("/* These are the main two-stage UCD tables. The fields in each record are:\n")
ucd_file.write("script (8 bits), character type (8 bits), grapheme break property (8 bits),\n")
ucd_file.write("offset to multichar other cases or zero (8 bits), offset to other case\n")
ucd_file.write("or zero (32 bits, signed), script extension (16 bits, signed), bidi class\n")
ucd_file.write("(8 bits), and a dummy 8-bit field to make the whole thing a multiple\n")
ucd_file.write("of 4 bytes. */\n\n")

print_records(ucd_file, records, record_size)
print_table(ucd_file, min_stage1, 'PRIV(ucd_stage1)')
print_table(ucd_file, min_stage2, 'PRIV(ucd_stage2)', min_block_size)
ucd_file.write("#if UCD_BLOCK_SIZE != %d\n" % min_block_size)
ucd_file.write("#error Please correct UCD_BLOCK_SIZE in pcre2_internal.h\n")
ucd_file.write("#endif\n")
ucd_file.write("#endif  /* SUPPORT_UNICODE */\n")
ucd_file.write("\n")
ucd_file.write("#endif  /* PCRE2_PCRE2TEST */\n")


# This code was part of the original contribution, but is commented out as it
# was never used. A two-stage table has sufficed.

"""

# Three-stage tables:

# Find the optimum block size for 3-stage table
min_size = sys.maxint
for stage3_block in [2 ** i for i in range(2,6)]:
        stage_i, stage3 = compress_table(table, stage3_block)
        for stage2_block in [2 ** i for i in range(5,10)]:
                size = len(records) * 4
                stage1, stage2 = compress_table(stage_i, stage2_block)
                size += get_tables_size(stage1, stage2, stage3)
                # print "/* %5d / %3d  => %5d bytes */" % (stage2_block, stage3_block, size)
                if size < min_size:
                        min_size = size
                        min_stage1, min_stage2, min_stage3 = stage1, stage2, stage3
                        min_stage2_block, min_stage3_block = stage2_block, stage3_block

print "/* Total size: %d bytes" % min_size */
print_records(records)
print_table(min_stage1, 'ucd_stage1')
print_table(min_stage2, 'ucd_stage2', min_stage2_block)
print_table(min_stage3, 'ucd_stage3', min_stage3_block)

"""
