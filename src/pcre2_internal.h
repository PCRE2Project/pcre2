/* This is a placeholder, just enough to allow dummy functions to compile, but
with some of the new PCRE2 context stuff added. */

#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcre2.h"

#define PUBL(name) pcre2_##name

#ifndef PRIV
#define PRIV(name) _pcre2_##name
#endif

#define PCRE2_CALL_CONVENTION

extern const uint8_t PRIV(default_tables)[];


/* What follows is "real" code for PCRE2. */


typedef int BOOL;

#ifndef FALSE
#define FALSE   0
#define TRUE    1
#endif


/* Valgrind (memcheck) support */

#ifdef SUPPORT_VALGRIND
#include <valgrind/memcheck.h>
#endif

/* When UTF encoding is being used, a character is no longer just a single
byte in 8-bit mode or a single short in 16-bit mode. The macros for character
handling generate simple sequences when used in the basic mode, and more
complicated ones for UTF characters. GETCHARLENTEST and other macros are not
used when UTF is not supported. To make sure they can never even appear when
UTF support is omitted, we don't even define them. */

#ifndef SUPPORT_UTF

/* #define MAX_VALUE_FOR_SINGLE_CHAR */
/* #define HAS_EXTRALEN(c) */
/* #define GET_EXTRALEN(c) */
/* #define NOT_FIRSTCHAR(c) */
#define GETCHAR(c, eptr) c = *eptr;
#define GETCHARTEST(c, eptr) c = *eptr;
#define GETCHARINC(c, eptr) c = *eptr++;
#define GETCHARINCTEST(c, eptr) c = *eptr++;
#define GETCHARLEN(c, eptr, len) c = *eptr;
/* #define GETCHARLENTEST(c, eptr, len) */
/* #define BACKCHAR(eptr) */
/* #define FORWARDCHAR(eptr) */
/* #define ACROSSCHAR(condition, eptr, action) */

#else   /* SUPPORT_UTF */

/* Tests whether a UTF-8 code point needs extra bytes to decode. */

#define HASUTF8EXTRALEN(c) ((c) >= 0xc0)

/* Base macro to pick up the remaining bytes of a UTF-8 character, not
advancing the pointer. */

#define GETUTF8(c, eptr) \
    { \
    if ((c & 0x20) == 0) \
      c = ((c & 0x1f) << 6) | (eptr[1] & 0x3f); \
    else if ((c & 0x10) == 0) \
      c = ((c & 0x0f) << 12) | ((eptr[1] & 0x3f) << 6) | (eptr[2] & 0x3f); \
    else if ((c & 0x08) == 0) \
      c = ((c & 0x07) << 18) | ((eptr[1] & 0x3f) << 12) | \
      ((eptr[2] & 0x3f) << 6) | (eptr[3] & 0x3f); \
    else if ((c & 0x04) == 0) \
      c = ((c & 0x03) << 24) | ((eptr[1] & 0x3f) << 18) | \
          ((eptr[2] & 0x3f) << 12) | ((eptr[3] & 0x3f) << 6) | \
          (eptr[4] & 0x3f); \
    else \
      c = ((c & 0x01) << 30) | ((eptr[1] & 0x3f) << 24) | \
          ((eptr[2] & 0x3f) << 18) | ((eptr[3] & 0x3f) << 12) | \
          ((eptr[4] & 0x3f) << 6) | (eptr[5] & 0x3f); \
    }

/* Base macro to pick up the remaining bytes of a UTF-8 character, advancing
the pointer. */

#define GETUTF8INC(c, eptr) \
    { \
    if ((c & 0x20) == 0) \
      c = ((c & 0x1f) << 6) | (*eptr++ & 0x3f); \
    else if ((c & 0x10) == 0) \
      { \
      c = ((c & 0x0f) << 12) | ((*eptr & 0x3f) << 6) | (eptr[1] & 0x3f); \
      eptr += 2; \
      } \
    else if ((c & 0x08) == 0) \
      { \
      c = ((c & 0x07) << 18) | ((*eptr & 0x3f) << 12) | \
          ((eptr[1] & 0x3f) << 6) | (eptr[2] & 0x3f); \
      eptr += 3; \
      } \
    else if ((c & 0x04) == 0) \
      { \
      c = ((c & 0x03) << 24) | ((*eptr & 0x3f) << 18) | \
          ((eptr[1] & 0x3f) << 12) | ((eptr[2] & 0x3f) << 6) | \
          (eptr[3] & 0x3f); \
      eptr += 4; \
      } \
    else \
      { \
      c = ((c & 0x01) << 30) | ((*eptr & 0x3f) << 24) | \
          ((eptr[1] & 0x3f) << 18) | ((eptr[2] & 0x3f) << 12) | \
          ((eptr[3] & 0x3f) << 6) | (eptr[4] & 0x3f); \
      eptr += 5; \
      } \
    }

#endif  /* SUPPORT_UTF */


/* Private flags containing information about the compiled pattern. The first 
three must not be changed, because whichever is set is actually the number of 
bytes in a code unit in that mode. */

#define PCRE2_MODE8         0x00000001  /* compiled in 8 bit mode */
#define PCRE2_MODE16        0x00000002  /* compiled in 16 bit mode */
#define PCRE2_MODE32        0x00000004  /* compiled in 32 bit mode */
#define PCRE2_FIRSTSET      0x00000010  /* first_char is set */
#define PCRE2_FCH_CASELESS  0x00000020  /* caseless first char */
#define PCRE2_REQCHSET      0x00000040  /* req_byte is set */                 
#define PCRE2_RCH_CASELESS  0x00000080  /* caseless requested char */   
#define PCRE2_STARTLINE     0x00000100  /* start after \n for multiline */
#define PCRE2_JCHANGED      0x00000200  /* j option used in pattern */
#define PCRE2_HASCRORLF     0x00000400  /* explicit \r or \n in pattern */
#define PCRE2_HASTHEN       0x00000800  /* pattern contains (*THEN) */        
#define PCRE2_MLSET         0x00001000  /* match limit set by pattern */
#define PCRE2_RLSET         0x00002000  /* recursion limit set by pattern */  
#define PCRE2_MATCH_EMPTY   0x00004000  /* pattern can match empty string */
                                                                            
#define PCRE2_MODE_MASK     (PCRE2_MODE8 | PCRE2_MODE16 | PCRE2_MODE32)       
                                                              

/* Magic number to provide a small check against being handed junk. */

#define MAGIC_NUMBER  0x50435245UL   /* 'PCRE' */

/* This variable is used to detect a loaded regular expression
in different endianness. */

#define REVERSED_MAGIC_NUMBER  0x45524350UL   /* 'ERCP' */



/* -------------------- Character and string names ------------------------ */

/* If PCRE is to support UTF-8 on EBCDIC platforms, we cannot use normal
character constants like '*' because the compiler would emit their EBCDIC code,
which is different from their ASCII/UTF-8 code. Instead we define macros for
the characters so that they always use the ASCII/UTF-8 code when UTF-8 support
is enabled. When UTF-8 support is not enabled, the definitions use character
literals. Both character and string versions of each character are needed, and
there are some longer strings as well.

This means that, on EBCDIC platforms, the PCRE library can handle either
EBCDIC, or UTF-8, but not both. To support both in the same compiled library
would need different lookups depending on whether PCRE_UTF8 was set or not.
This would make it impossible to use characters in switch/case statements,
which would reduce performance. For a theoretical use (which nobody has asked
for) in a minority area (EBCDIC platforms), this is not sensible. Any
application that did need both could compile two versions of the library, using
macros to give the functions distinct names. */

#ifndef SUPPORT_UTF

/* UTF-8 support is not enabled; use the platform-dependent character literals
so that PCRE works in both ASCII and EBCDIC environments, but only in non-UTF
mode. Newline characters are problematic in EBCDIC. Though it has CR and LF
characters, a common practice has been to use its NL (0x15) character as the
line terminator in C-like processing environments. However, sometimes the LF
(0x25) character is used instead, according to this Unicode document:

http://unicode.org/standard/reports/tr13/tr13-5.html

PCRE defaults EBCDIC NL to 0x15, but has a build-time option to select 0x25
instead. Whichever is *not* chosen is defined as NEL.

In both ASCII and EBCDIC environments, CHAR_NL and CHAR_LF are synonyms for the
same code point. */

#ifdef EBCDIC

#ifndef EBCDIC_NL25
#define CHAR_NL                     '\x15'
#define CHAR_NEL                    '\x25'
#define STR_NL                      "\x15"
#define STR_NEL                     "\x25"
#else
#define CHAR_NL                     '\x25'
#define CHAR_NEL                    '\x15'
#define STR_NL                      "\x25"
#define STR_NEL                     "\x15"
#endif

#define CHAR_LF                     CHAR_NL
#define STR_LF                      STR_NL

#define CHAR_ESC                    '\047'
#define CHAR_DEL                    '\007'
#define STR_ESC                     "\047"
#define STR_DEL                     "\007"

#else  /* Not EBCDIC */

/* In ASCII/Unicode, linefeed is '\n' and we equate this to NL for
compatibility. NEL is the Unicode newline character; make sure it is
a positive value. */

#define CHAR_LF                     '\n'
#define CHAR_NL                     CHAR_LF
#define CHAR_NEL                    ((unsigned char)'\x85')
#define CHAR_ESC                    '\033'
#define CHAR_DEL                    '\177'

#define STR_LF                      "\n"
#define STR_NL                      STR_LF
#define STR_NEL                     "\x85"
#define STR_ESC                     "\033"
#define STR_DEL                     "\177"

#endif  /* EBCDIC */

/* The remaining definitions work in both environments. */

#define CHAR_NULL                   '\0'
#define CHAR_HT                     '\t'
#define CHAR_VT                     '\v'
#define CHAR_FF                     '\f'
#define CHAR_CR                     '\r'
#define CHAR_BS                     '\b'
#define CHAR_BEL                    '\a'

#define CHAR_SPACE                  ' '
#define CHAR_EXCLAMATION_MARK       '!'
#define CHAR_QUOTATION_MARK         '"'
#define CHAR_NUMBER_SIGN            '#'
#define CHAR_DOLLAR_SIGN            '$'
#define CHAR_PERCENT_SIGN           '%'
#define CHAR_AMPERSAND              '&'
#define CHAR_APOSTROPHE             '\''
#define CHAR_LEFT_PARENTHESIS       '('
#define CHAR_RIGHT_PARENTHESIS      ')'
#define CHAR_ASTERISK               '*'
#define CHAR_PLUS                   '+'
#define CHAR_COMMA                  ','
#define CHAR_MINUS                  '-'
#define CHAR_DOT                    '.'
#define CHAR_SLASH                  '/'
#define CHAR_0                      '0'
#define CHAR_1                      '1'
#define CHAR_2                      '2'
#define CHAR_3                      '3'
#define CHAR_4                      '4'
#define CHAR_5                      '5'
#define CHAR_6                      '6'
#define CHAR_7                      '7'
#define CHAR_8                      '8'
#define CHAR_9                      '9'
#define CHAR_COLON                  ':'
#define CHAR_SEMICOLON              ';'
#define CHAR_LESS_THAN_SIGN         '<'
#define CHAR_EQUALS_SIGN            '='
#define CHAR_GREATER_THAN_SIGN      '>'
#define CHAR_QUESTION_MARK          '?'
#define CHAR_COMMERCIAL_AT          '@'
#define CHAR_A                      'A'
#define CHAR_B                      'B'
#define CHAR_C                      'C'
#define CHAR_D                      'D'
#define CHAR_E                      'E'
#define CHAR_F                      'F'
#define CHAR_G                      'G'
#define CHAR_H                      'H'
#define CHAR_I                      'I'
#define CHAR_J                      'J'
#define CHAR_K                      'K'
#define CHAR_L                      'L'
#define CHAR_M                      'M'
#define CHAR_N                      'N'
#define CHAR_O                      'O'
#define CHAR_P                      'P'
#define CHAR_Q                      'Q'
#define CHAR_R                      'R'
#define CHAR_S                      'S'
#define CHAR_T                      'T'
#define CHAR_U                      'U'
#define CHAR_V                      'V'
#define CHAR_W                      'W'
#define CHAR_X                      'X'
#define CHAR_Y                      'Y'
#define CHAR_Z                      'Z'
#define CHAR_LEFT_SQUARE_BRACKET    '['
#define CHAR_BACKSLASH              '\\'
#define CHAR_RIGHT_SQUARE_BRACKET   ']'
#define CHAR_CIRCUMFLEX_ACCENT      '^'
#define CHAR_UNDERSCORE             '_'
#define CHAR_GRAVE_ACCENT           '`'
#define CHAR_a                      'a'
#define CHAR_b                      'b'
#define CHAR_c                      'c'
#define CHAR_d                      'd'
#define CHAR_e                      'e'
#define CHAR_f                      'f'
#define CHAR_g                      'g'
#define CHAR_h                      'h'
#define CHAR_i                      'i'
#define CHAR_j                      'j'
#define CHAR_k                      'k'
#define CHAR_l                      'l'
#define CHAR_m                      'm'
#define CHAR_n                      'n'
#define CHAR_o                      'o'
#define CHAR_p                      'p'
#define CHAR_q                      'q'
#define CHAR_r                      'r'
#define CHAR_s                      's'
#define CHAR_t                      't'
#define CHAR_u                      'u'
#define CHAR_v                      'v'
#define CHAR_w                      'w'
#define CHAR_x                      'x'
#define CHAR_y                      'y'
#define CHAR_z                      'z'
#define CHAR_LEFT_CURLY_BRACKET     '{'
#define CHAR_VERTICAL_LINE          '|'
#define CHAR_RIGHT_CURLY_BRACKET    '}'
#define CHAR_TILDE                  '~'

#define STR_HT                      "\t"
#define STR_VT                      "\v"
#define STR_FF                      "\f"
#define STR_CR                      "\r"
#define STR_BS                      "\b"
#define STR_BEL                     "\a"

#define STR_SPACE                   " "
#define STR_EXCLAMATION_MARK        "!"
#define STR_QUOTATION_MARK          "\""
#define STR_NUMBER_SIGN             "#"
#define STR_DOLLAR_SIGN             "$"
#define STR_PERCENT_SIGN            "%"
#define STR_AMPERSAND               "&"
#define STR_APOSTROPHE              "'"
#define STR_LEFT_PARENTHESIS        "("
#define STR_RIGHT_PARENTHESIS       ")"
#define STR_ASTERISK                "*"
#define STR_PLUS                    "+"
#define STR_COMMA                   ","
#define STR_MINUS                   "-"
#define STR_DOT                     "."
#define STR_SLASH                   "/"
#define STR_0                       "0"
#define STR_1                       "1"
#define STR_2                       "2"
#define STR_3                       "3"
#define STR_4                       "4"
#define STR_5                       "5"
#define STR_6                       "6"
#define STR_7                       "7"
#define STR_8                       "8"
#define STR_9                       "9"
#define STR_COLON                   ":"
#define STR_SEMICOLON               ";"
#define STR_LESS_THAN_SIGN          "<"
#define STR_EQUALS_SIGN             "="
#define STR_GREATER_THAN_SIGN       ">"
#define STR_QUESTION_MARK           "?"
#define STR_COMMERCIAL_AT           "@"
#define STR_A                       "A"
#define STR_B                       "B"
#define STR_C                       "C"
#define STR_D                       "D"
#define STR_E                       "E"
#define STR_F                       "F"
#define STR_G                       "G"
#define STR_H                       "H"
#define STR_I                       "I"
#define STR_J                       "J"
#define STR_K                       "K"
#define STR_L                       "L"
#define STR_M                       "M"
#define STR_N                       "N"
#define STR_O                       "O"
#define STR_P                       "P"
#define STR_Q                       "Q"
#define STR_R                       "R"
#define STR_S                       "S"
#define STR_T                       "T"
#define STR_U                       "U"
#define STR_V                       "V"
#define STR_W                       "W"
#define STR_X                       "X"
#define STR_Y                       "Y"
#define STR_Z                       "Z"
#define STR_LEFT_SQUARE_BRACKET     "["
#define STR_BACKSLASH               "\\"
#define STR_RIGHT_SQUARE_BRACKET    "]"
#define STR_CIRCUMFLEX_ACCENT       "^"
#define STR_UNDERSCORE              "_"
#define STR_GRAVE_ACCENT            "`"
#define STR_a                       "a"
#define STR_b                       "b"
#define STR_c                       "c"
#define STR_d                       "d"
#define STR_e                       "e"
#define STR_f                       "f"
#define STR_g                       "g"
#define STR_h                       "h"
#define STR_i                       "i"
#define STR_j                       "j"
#define STR_k                       "k"
#define STR_l                       "l"
#define STR_m                       "m"
#define STR_n                       "n"
#define STR_o                       "o"
#define STR_p                       "p"
#define STR_q                       "q"
#define STR_r                       "r"
#define STR_s                       "s"
#define STR_t                       "t"
#define STR_u                       "u"
#define STR_v                       "v"
#define STR_w                       "w"
#define STR_x                       "x"
#define STR_y                       "y"
#define STR_z                       "z"
#define STR_LEFT_CURLY_BRACKET      "{"
#define STR_VERTICAL_LINE           "|"
#define STR_RIGHT_CURLY_BRACKET     "}"
#define STR_TILDE                   "~"

#define STRING_ACCEPT0              "ACCEPT\0"
#define STRING_COMMIT0              "COMMIT\0"
#define STRING_F0                   "F\0"
#define STRING_FAIL0                "FAIL\0"
#define STRING_MARK0                "MARK\0"
#define STRING_PRUNE0               "PRUNE\0"
#define STRING_SKIP0                "SKIP\0"
#define STRING_THEN                 "THEN"

#define STRING_alpha0               "alpha\0"
#define STRING_lower0               "lower\0"
#define STRING_upper0               "upper\0"
#define STRING_alnum0               "alnum\0"
#define STRING_ascii0               "ascii\0"
#define STRING_blank0               "blank\0"
#define STRING_cntrl0               "cntrl\0"
#define STRING_digit0               "digit\0"
#define STRING_graph0               "graph\0"
#define STRING_print0               "print\0"
#define STRING_punct0               "punct\0"
#define STRING_space0               "space\0"
#define STRING_word0                "word\0"
#define STRING_xdigit               "xdigit"

#define STRING_DEFINE               "DEFINE"
#define STRING_WEIRD_STARTWORD      "[:<:]]"
#define STRING_WEIRD_ENDWORD        "[:>:]]"

#define STRING_CR_RIGHTPAR              "CR)"
#define STRING_LF_RIGHTPAR              "LF)"
#define STRING_CRLF_RIGHTPAR            "CRLF)"
#define STRING_ANY_RIGHTPAR             "ANY)"
#define STRING_ANYCRLF_RIGHTPAR         "ANYCRLF)"
#define STRING_BSR_ANYCRLF_RIGHTPAR     "BSR_ANYCRLF)"
#define STRING_BSR_UNICODE_RIGHTPAR     "BSR_UNICODE)"
#define STRING_UTF8_RIGHTPAR            "UTF8)"
#define STRING_UTF16_RIGHTPAR           "UTF16)"
#define STRING_UTF32_RIGHTPAR           "UTF32)"
#define STRING_UTF_RIGHTPAR             "UTF)"
#define STRING_UCP_RIGHTPAR             "UCP)"
#define STRING_NO_AUTO_POSSESS_RIGHTPAR "NO_AUTO_POSSESS)"
#define STRING_NO_START_OPT_RIGHTPAR    "NO_START_OPT)"
#define STRING_LIMIT_MATCH_EQ           "LIMIT_MATCH="
#define STRING_LIMIT_RECURSION_EQ       "LIMIT_RECURSION="

#else  /* SUPPORT_UTF */

/* UTF-8 support is enabled; always use UTF-8 (=ASCII) character codes. This
works in both modes non-EBCDIC platforms, and on EBCDIC platforms in UTF-8 mode
only. */

#define CHAR_HT                     '\011'
#define CHAR_VT                     '\013'
#define CHAR_FF                     '\014'
#define CHAR_CR                     '\015'
#define CHAR_LF                     '\012'
#define CHAR_NL                     CHAR_LF
#define CHAR_NEL                    ((unsigned char)'\x85')
#define CHAR_BS                     '\010'
#define CHAR_BEL                    '\007'
#define CHAR_ESC                    '\033'
#define CHAR_DEL                    '\177'

#define CHAR_NULL                   '\0'
#define CHAR_SPACE                  '\040'
#define CHAR_EXCLAMATION_MARK       '\041'
#define CHAR_QUOTATION_MARK         '\042'
#define CHAR_NUMBER_SIGN            '\043'
#define CHAR_DOLLAR_SIGN            '\044'
#define CHAR_PERCENT_SIGN           '\045'
#define CHAR_AMPERSAND              '\046'
#define CHAR_APOSTROPHE             '\047'
#define CHAR_LEFT_PARENTHESIS       '\050'
#define CHAR_RIGHT_PARENTHESIS      '\051'
#define CHAR_ASTERISK               '\052'
#define CHAR_PLUS                   '\053'
#define CHAR_COMMA                  '\054'
#define CHAR_MINUS                  '\055'
#define CHAR_DOT                    '\056'
#define CHAR_SLASH                  '\057'
#define CHAR_0                      '\060'
#define CHAR_1                      '\061'
#define CHAR_2                      '\062'
#define CHAR_3                      '\063'
#define CHAR_4                      '\064'
#define CHAR_5                      '\065'
#define CHAR_6                      '\066'
#define CHAR_7                      '\067'
#define CHAR_8                      '\070'
#define CHAR_9                      '\071'
#define CHAR_COLON                  '\072'
#define CHAR_SEMICOLON              '\073'
#define CHAR_LESS_THAN_SIGN         '\074'
#define CHAR_EQUALS_SIGN            '\075'
#define CHAR_GREATER_THAN_SIGN      '\076'
#define CHAR_QUESTION_MARK          '\077'
#define CHAR_COMMERCIAL_AT          '\100'
#define CHAR_A                      '\101'
#define CHAR_B                      '\102'
#define CHAR_C                      '\103'
#define CHAR_D                      '\104'
#define CHAR_E                      '\105'
#define CHAR_F                      '\106'
#define CHAR_G                      '\107'
#define CHAR_H                      '\110'
#define CHAR_I                      '\111'
#define CHAR_J                      '\112'
#define CHAR_K                      '\113'
#define CHAR_L                      '\114'
#define CHAR_M                      '\115'
#define CHAR_N                      '\116'
#define CHAR_O                      '\117'
#define CHAR_P                      '\120'
#define CHAR_Q                      '\121'
#define CHAR_R                      '\122'
#define CHAR_S                      '\123'
#define CHAR_T                      '\124'
#define CHAR_U                      '\125'
#define CHAR_V                      '\126'
#define CHAR_W                      '\127'
#define CHAR_X                      '\130'
#define CHAR_Y                      '\131'
#define CHAR_Z                      '\132'
#define CHAR_LEFT_SQUARE_BRACKET    '\133'
#define CHAR_BACKSLASH              '\134'
#define CHAR_RIGHT_SQUARE_BRACKET   '\135'
#define CHAR_CIRCUMFLEX_ACCENT      '\136'
#define CHAR_UNDERSCORE             '\137'
#define CHAR_GRAVE_ACCENT           '\140'
#define CHAR_a                      '\141'
#define CHAR_b                      '\142'
#define CHAR_c                      '\143'
#define CHAR_d                      '\144'
#define CHAR_e                      '\145'
#define CHAR_f                      '\146'
#define CHAR_g                      '\147'
#define CHAR_h                      '\150'
#define CHAR_i                      '\151'
#define CHAR_j                      '\152'
#define CHAR_k                      '\153'
#define CHAR_l                      '\154'
#define CHAR_m                      '\155'
#define CHAR_n                      '\156'
#define CHAR_o                      '\157'
#define CHAR_p                      '\160'
#define CHAR_q                      '\161'
#define CHAR_r                      '\162'
#define CHAR_s                      '\163'
#define CHAR_t                      '\164'
#define CHAR_u                      '\165'
#define CHAR_v                      '\166'
#define CHAR_w                      '\167'
#define CHAR_x                      '\170'
#define CHAR_y                      '\171'
#define CHAR_z                      '\172'
#define CHAR_LEFT_CURLY_BRACKET     '\173'
#define CHAR_VERTICAL_LINE          '\174'
#define CHAR_RIGHT_CURLY_BRACKET    '\175'
#define CHAR_TILDE                  '\176'

#define STR_HT                      "\011"
#define STR_VT                      "\013"
#define STR_FF                      "\014"
#define STR_CR                      "\015"
#define STR_NL                      "\012"
#define STR_BS                      "\010"
#define STR_BEL                     "\007"
#define STR_ESC                     "\033"
#define STR_DEL                     "\177"

#define STR_SPACE                   "\040"
#define STR_EXCLAMATION_MARK        "\041"
#define STR_QUOTATION_MARK          "\042"
#define STR_NUMBER_SIGN             "\043"
#define STR_DOLLAR_SIGN             "\044"
#define STR_PERCENT_SIGN            "\045"
#define STR_AMPERSAND               "\046"
#define STR_APOSTROPHE              "\047"
#define STR_LEFT_PARENTHESIS        "\050"
#define STR_RIGHT_PARENTHESIS       "\051"
#define STR_ASTERISK                "\052"
#define STR_PLUS                    "\053"
#define STR_COMMA                   "\054"
#define STR_MINUS                   "\055"
#define STR_DOT                     "\056"
#define STR_SLASH                   "\057"
#define STR_0                       "\060"
#define STR_1                       "\061"
#define STR_2                       "\062"
#define STR_3                       "\063"
#define STR_4                       "\064"
#define STR_5                       "\065"
#define STR_6                       "\066"
#define STR_7                       "\067"
#define STR_8                       "\070"
#define STR_9                       "\071"
#define STR_COLON                   "\072"
#define STR_SEMICOLON               "\073"
#define STR_LESS_THAN_SIGN          "\074"
#define STR_EQUALS_SIGN             "\075"
#define STR_GREATER_THAN_SIGN       "\076"
#define STR_QUESTION_MARK           "\077"
#define STR_COMMERCIAL_AT           "\100"
#define STR_A                       "\101"
#define STR_B                       "\102"
#define STR_C                       "\103"
#define STR_D                       "\104"
#define STR_E                       "\105"
#define STR_F                       "\106"
#define STR_G                       "\107"
#define STR_H                       "\110"
#define STR_I                       "\111"
#define STR_J                       "\112"
#define STR_K                       "\113"
#define STR_L                       "\114"
#define STR_M                       "\115"
#define STR_N                       "\116"
#define STR_O                       "\117"
#define STR_P                       "\120"
#define STR_Q                       "\121"
#define STR_R                       "\122"
#define STR_S                       "\123"
#define STR_T                       "\124"
#define STR_U                       "\125"
#define STR_V                       "\126"
#define STR_W                       "\127"
#define STR_X                       "\130"
#define STR_Y                       "\131"
#define STR_Z                       "\132"
#define STR_LEFT_SQUARE_BRACKET     "\133"
#define STR_BACKSLASH               "\134"
#define STR_RIGHT_SQUARE_BRACKET    "\135"
#define STR_CIRCUMFLEX_ACCENT       "\136"
#define STR_UNDERSCORE              "\137"
#define STR_GRAVE_ACCENT            "\140"
#define STR_a                       "\141"
#define STR_b                       "\142"
#define STR_c                       "\143"
#define STR_d                       "\144"
#define STR_e                       "\145"
#define STR_f                       "\146"
#define STR_g                       "\147"
#define STR_h                       "\150"
#define STR_i                       "\151"
#define STR_j                       "\152"
#define STR_k                       "\153"
#define STR_l                       "\154"
#define STR_m                       "\155"
#define STR_n                       "\156"
#define STR_o                       "\157"
#define STR_p                       "\160"
#define STR_q                       "\161"
#define STR_r                       "\162"
#define STR_s                       "\163"
#define STR_t                       "\164"
#define STR_u                       "\165"
#define STR_v                       "\166"
#define STR_w                       "\167"
#define STR_x                       "\170"
#define STR_y                       "\171"
#define STR_z                       "\172"
#define STR_LEFT_CURLY_BRACKET      "\173"
#define STR_VERTICAL_LINE           "\174"
#define STR_RIGHT_CURLY_BRACKET     "\175"
#define STR_TILDE                   "\176"

#define STRING_ACCEPT0              STR_A STR_C STR_C STR_E STR_P STR_T "\0"
#define STRING_COMMIT0              STR_C STR_O STR_M STR_M STR_I STR_T "\0"
#define STRING_F0                   STR_F "\0"
#define STRING_FAIL0                STR_F STR_A STR_I STR_L "\0"
#define STRING_MARK0                STR_M STR_A STR_R STR_K "\0"
#define STRING_PRUNE0               STR_P STR_R STR_U STR_N STR_E "\0"
#define STRING_SKIP0                STR_S STR_K STR_I STR_P "\0"
#define STRING_THEN                 STR_T STR_H STR_E STR_N

#define STRING_alpha0               STR_a STR_l STR_p STR_h STR_a "\0"
#define STRING_lower0               STR_l STR_o STR_w STR_e STR_r "\0"
#define STRING_upper0               STR_u STR_p STR_p STR_e STR_r "\0"
#define STRING_alnum0               STR_a STR_l STR_n STR_u STR_m "\0"
#define STRING_ascii0               STR_a STR_s STR_c STR_i STR_i "\0"
#define STRING_blank0               STR_b STR_l STR_a STR_n STR_k "\0"
#define STRING_cntrl0               STR_c STR_n STR_t STR_r STR_l "\0"
#define STRING_digit0               STR_d STR_i STR_g STR_i STR_t "\0"
#define STRING_graph0               STR_g STR_r STR_a STR_p STR_h "\0"
#define STRING_print0               STR_p STR_r STR_i STR_n STR_t "\0"
#define STRING_punct0               STR_p STR_u STR_n STR_c STR_t "\0"
#define STRING_space0               STR_s STR_p STR_a STR_c STR_e "\0"
#define STRING_word0                STR_w STR_o STR_r STR_d       "\0"
#define STRING_xdigit               STR_x STR_d STR_i STR_g STR_i STR_t

#define STRING_DEFINE               STR_D STR_E STR_F STR_I STR_N STR_E
#define STRING_WEIRD_STARTWORD      STR_LEFT_SQUARE_BRACKET STR_COLON STR_LESS_THAN_SIGN STR_COLON STR_RIGHT_SQUARE_BRACKET STR_RIGHT_SQUARE_BRACKET
#define STRING_WEIRD_ENDWORD        STR_LEFT_SQUARE_BRACKET STR_COLON STR_GREATER_THAN_SIGN STR_COLON STR_RIGHT_SQUARE_BRACKET STR_RIGHT_SQUARE_BRACKET

#define STRING_CR_RIGHTPAR              STR_C STR_R STR_RIGHT_PARENTHESIS
#define STRING_LF_RIGHTPAR              STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_CRLF_RIGHTPAR            STR_C STR_R STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_ANY_RIGHTPAR             STR_A STR_N STR_Y STR_RIGHT_PARENTHESIS
#define STRING_ANYCRLF_RIGHTPAR         STR_A STR_N STR_Y STR_C STR_R STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_BSR_ANYCRLF_RIGHTPAR     STR_B STR_S STR_R STR_UNDERSCORE STR_A STR_N STR_Y STR_C STR_R STR_L STR_F STR_RIGHT_PARENTHESIS
#define STRING_BSR_UNICODE_RIGHTPAR     STR_B STR_S STR_R STR_UNDERSCORE STR_U STR_N STR_I STR_C STR_O STR_D STR_E STR_RIGHT_PARENTHESIS
#define STRING_UTF8_RIGHTPAR            STR_U STR_T STR_F STR_8 STR_RIGHT_PARENTHESIS
#define STRING_UTF16_RIGHTPAR           STR_U STR_T STR_F STR_1 STR_6 STR_RIGHT_PARENTHESIS
#define STRING_UTF32_RIGHTPAR           STR_U STR_T STR_F STR_3 STR_2 STR_RIGHT_PARENTHESIS
#define STRING_UTF_RIGHTPAR             STR_U STR_T STR_F STR_RIGHT_PARENTHESIS
#define STRING_UCP_RIGHTPAR             STR_U STR_C STR_P STR_RIGHT_PARENTHESIS
#define STRING_NO_AUTO_POSSESS_RIGHTPAR STR_N STR_O STR_UNDERSCORE STR_A STR_U STR_T STR_O STR_UNDERSCORE STR_P STR_O STR_S STR_S STR_E STR_S STR_S STR_RIGHT_PARENTHESIS
#define STRING_NO_START_OPT_RIGHTPAR    STR_N STR_O STR_UNDERSCORE STR_S STR_T STR_A STR_R STR_T STR_UNDERSCORE STR_O STR_P STR_T STR_RIGHT_PARENTHESIS
#define STRING_LIMIT_MATCH_EQ           STR_L STR_I STR_M STR_I STR_T STR_UNDERSCORE STR_M STR_A STR_T STR_C STR_H STR_EQUALS_SIGN
#define STRING_LIMIT_RECURSION_EQ       STR_L STR_I STR_M STR_I STR_T STR_UNDERSCORE STR_R STR_E STR_C STR_U STR_R STR_S STR_I STR_O STR_N STR_EQUALS_SIGN

#endif  /* SUPPORT_UTF */

/* -------------------- End of character and string names -------------------*/



/* Only these bits are allowed when setting context options. */

#define PCRE2_CONTEXT_OPTIONS (\
  PCRE2_ALT_BSUX|PCRE2_DOLLAR_ENDONLY|PCRE2_DUPNAMES|PCRE2_ALLOW_EMPTY_CLASS|\
  PCRE2_MATCH_UNSET_BACKREF|PCRE2_NEVER_UTF|PCRE2_NEVER_UCP|PCRE2_UTF|\
  PCRE2_UCP)
  
/* The private structures used by PCRE are defined in a separate file. When 
compiling the library, PCRE2_CODE_UNIT_WIDTH will be defined, so we include 
them at the appropriate width. When compiling pcretest, however, that macro is 
not set at this point because pcretest needs to include them at all supported 
widths. */ 
  
#ifdef PCRE2_CODE_UNIT_WIDTH 
#include "pcre2_intstructs.h"
#endif

/* End of pcre2_internal.h */
