/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
          New API code Copyright (c) 2016-2024 University of Cambridge

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


#include "pcre2_internal.h"



#define STRING(a)  #a
#define XSTRING(s) STRING(s)

/* The texts of compile-time error messages. Compile-time error numbers start
at COMPILE_ERROR_BASE (100).

To avoid a relocation for each message in a shared library, the strings are
stored as char array members of one struct. The same macro list generates the
members, their initializers, and a table of offsets for direct lookup. Each
member includes its terminating zero; no end-of-table sentinel is needed.
The struct sizes are checked to ensure that the offsets fit in uint16_t. */

#ifndef EBCDIC
#define PCRE2_ERROR_BACKSLASH_C_SYNTAX_MESSAGE "\\c must be followed by a printable ASCII character"
#else
#define PCRE2_ERROR_BACKSLASH_C_SYNTAX_MESSAGE "\\c must be followed by a letter or one of @[\\]^_?"
#endif

// clang-format off
#define COMPILE_ERROR_TEXTS                                                                                           \
  X(e100, "no error")                                                                                                 \
  X(e101, "\\ at end of pattern")                                                                                     \
  X(e102, "\\c at end of pattern")                                                                                    \
  X(e103, "unrecognized character follows \\")                                                                        \
  X(e104, "numbers out of order in {} quantifier")                                                                    \
  X(e105, "number too big in {} quantifier")                                                                          \
  X(e106, "missing terminating ] for character class")                                                                \
  X(e107, "escape sequence is invalid in character class")                                                            \
  X(e108, "range out of order in character class")                                                                    \
  X(e109, "quantifier does not follow a repeatable item")                                                             \
  X(e110, "internal error: unexpected repeat")                                                                        \
  X(e111, "unrecognized character after (? or (?-")                                                                   \
  X(e112, "POSIX named classes are supported only within a class")                                                    \
  X(e113, "POSIX collating elements are not supported")                                                               \
  X(e114, "missing closing parenthesis")                                                                              \
  X(e115, "reference to non-existent subpattern")                                                                     \
  X(e116, "pattern passed as NULL with non-zero length")                                                              \
  X(e117, "unrecognised compile-time option bit(s)")                                                                  \
  X(e118, "missing ) after (?# comment")                                                                              \
  X(e119, "parentheses are too deeply nested")                                                                        \
  X(e120, "regular expression is too large")                                                                          \
  X(e121, "failed to allocate heap memory")                                                                           \
  X(e122, "unmatched closing parenthesis")                                                                            \
  X(e123, "internal error: code overflow")                                                                            \
  X(e124, "missing closing parenthesis for condition")                                                                \
  X(e125, "length of lookbehind assertion is not limited")                                                            \
  X(e126, "a relative value of zero is not allowed")                                                                  \
  X(e127, "conditional subpattern contains more than two branches")                                                   \
  X(e128, "atomic assertion expected after (?( or (?(?C)")                                                            \
  X(e129, "digit expected after (?+")                                                                                 \
  X(e130, "unknown POSIX class name")                                                                                 \
  X(e131, "internal error in pcre2_study(): should not occur")                                                        \
  X(e132, "this version of PCRE2 does not have Unicode support")                                                      \
  X(e133, "parentheses are too deeply nested (stack check)")                                                          \
  X(e134, "character code point value in \\x{} or \\o{} is too large")                                                \
  X(e135, "lookbehind is too complicated")                                                                            \
  X(e136, "\\C is not allowed in a lookbehind assertion in UTF-" XSTRING(PCRE2_CODE_UNIT_WIDTH) " mode")              \
  X(e137, "PCRE2 does not support \\F, \\L, \\l, \\N{name}, \\U, or \\u")                                             \
  X(e138, "number after (?C is greater than 255")                                                                     \
  X(e139, "closing parenthesis for (?C expected")                                                                     \
  X(e140, "invalid escape sequence in (*VERB) name")                                                                  \
  X(e141, "unrecognized character after (?P")                                                                         \
  X(e142, "syntax error in subpattern name (missing terminator?)")                                                    \
  X(e143, "two named subpatterns have the same name (PCRE2_DUPNAMES not set)")                                        \
  X(e144, "subpattern name must start with a non-digit")                                                              \
  X(e145, "this version of PCRE2 does not have support for \\P, \\p, or \\X")                                         \
  X(e146, "malformed \\P or \\p sequence")                                                                            \
  X(e147, "unknown property after \\P or \\p")                                                                        \
  X(e148, "subpattern name is too long (maximum " XSTRING(MAX_NAME_SIZE) " code units)")                              \
  X(e149, "too many named subpatterns (maximum " XSTRING(MAX_NAME_COUNT) ")")                                         \
  X(e150, "invalid range in character class")                                                                         \
  X(e151, "octal value is greater than \\377 in 8-bit non-UTF-8 mode")                                                \
  X(e152, "internal error: overran compiling workspace")                                                              \
  X(e153, "internal error: previously-checked referenced subpattern not found")                                       \
  X(e154, "DEFINE subpattern contains more than one branch")                                                          \
  X(e155, "missing opening brace after \\o")                                                                          \
  X(e156, "internal error: unknown newline setting")                                                                  \
  X(e157, "\\g is not followed by a braced, angle-bracketed, or quoted name/number or by a plain number")             \
  X(e158, "(?R (recursive pattern call) must be followed by a closing parenthesis")                                   \
  X(e159, "obsolete error (should not occur)") /* Was: "an argument is not allowed for (*ACCEPT), (*FAIL), or (*COMMIT)" */ \
  X(e160, "(*VERB) not recognized or malformed")                                                                      \
  X(e161, "subpattern number is too big")                                                                             \
  X(e162, "subpattern name expected")                                                                                 \
  X(e163, "internal error: parsed pattern overflow")                                                                  \
  X(e164, "non-octal character in \\o{} (closing brace missing?)")                                                    \
  X(e165, "different names for subpatterns of the same number are not allowed")                                       \
  X(e166, "(*MARK) must have an argument")                                                                            \
  X(e167, "non-hex character in \\x{} (closing brace missing?)")                                                      \
  X(e168, PCRE2_ERROR_BACKSLASH_C_SYNTAX_MESSAGE)                                                                     \
  X(e169, "\\k is not followed by a braced, angle-bracketed, or quoted name")                                         \
  X(e170, "internal error: unknown meta code in check_lookbehinds()")                                                 \
  X(e171, "\\N is not supported in a class")                                                                          \
  X(e172, "callout string is too long")                                                                               \
  X(e173, "disallowed Unicode code point (>= 0xd800 && <= 0xdfff)")                                                   \
  X(e174, "using UTF is disabled by the application")                                                                 \
  X(e175, "using UCP is disabled by the application")                                                                 \
  X(e176, "name is too long in (*MARK), (*PRUNE), (*SKIP), or (*THEN)")                                               \
  X(e177, "character code point value in \\u.... sequence is too large")                                              \
  X(e178, "digits missing after \\x or in \\x{} or \\o{} or \\N{U+}")                                                 \
  X(e179, "syntax error or number too big in (?(VERSION condition")                                                   \
  X(e180, "internal error: unknown opcode in auto_possessify()")                                                      \
  X(e181, "missing terminating delimiter for callout with string argument")                                           \
  X(e182, "unrecognized string delimiter follows (?C")                                                                \
  X(e183, "using \\C is disabled by the application")                                                                 \
  X(e184, "(?| and/or (?J: or (?x: parentheses are too deeply nested")                                                \
  X(e185, "using \\C is disabled in this PCRE2 library")                                                              \
  X(e186, "regular expression is too complicated")                                                                    \
  X(e187, "lookbehind assertion is too long")                                                                         \
  X(e188, "pattern string is longer than the limit set by the application")                                           \
  X(e189, "internal error: unknown code in parsed pattern")                                                           \
  X(e190, "internal error: bad code value in parsed_skip()")                                                          \
  X(e191, "PCRE2_EXTRA_ALLOW_SURROGATE_ESCAPES is not allowed in UTF-16 mode")                                        \
  X(e192, "invalid option bits with PCRE2_LITERAL")                                                                   \
  X(e193, "\\N{U+dddd} is supported only in Unicode (UTF) mode")                                                      \
  X(e194, "invalid hyphen in option setting")                                                                         \
  X(e195, "(*alpha_assertion) not recognized")                                                                        \
  X(e196, "script runs require Unicode support, which this version of PCRE2 does not have")                           \
  X(e197, "too many capturing groups (maximum 65535)")                                                                \
  X(e198, "octal digit missing after \\0 (PCRE2_EXTRA_NO_BS0 is set)")                                                \
  X(e199, "\\K is not allowed in lookarounds (but see PCRE2_EXTRA_ALLOW_LOOKAROUND_BSK)")                             \
  X(e200, "branch too long in variable-length lookbehind assertion")                                                  \
  X(e201, "compiled pattern would be longer than the limit set by the application")                                   \
  X(e202, "octal value given by \\ddd is greater than \\377 (forbidden by PCRE2_EXTRA_PYTHON_OCTAL)")                 \
  X(e203, "using callouts is disabled by the application")                                                            \
  X(e204, "PCRE2_EXTRA_TURKISH_CASING requires Unicode (UTF or UCP) mode")                                            \
  X(e205, "PCRE2_EXTRA_TURKISH_CASING requires UTF in 8-bit mode")                                                    \
  X(e206, "PCRE2_EXTRA_TURKISH_CASING and PCRE2_EXTRA_CASELESS_RESTRICT are not compatible")                          \
  X(e207, "extended character class nesting is too deep")                                                             \
  X(e208, "invalid operator in extended character class")                                                             \
  X(e209, "unexpected operator in extended character class (no preceding operand)")                                   \
  X(e210, "expected operand after operator in extended character class")                                              \
  X(e211, "square brackets needed to clarify operator precedence in extended character class")                        \
  X(e212, "missing terminating ] for extended character class (note '[' must be escaped under PCRE2_ALT_EXTENDED_CLASS)") \
  X(e213, "unexpected expression in extended character class (no preceding operator)")                                \
  X(e214, "empty expression in extended character class")                                                             \
  X(e215, "terminating ] with no following closing parenthesis in (?[...]")                                           \
  X(e216, "unexpected character in (?[...]) extended character class")                                                \
  X(e217, "expected capture group number or name")                                                                    \
  X(e218, "missing opening parenthesis")                                                                              \
  X(e219, "syntax error in subpattern number (missing terminator?)")                                                  \
  X(e220, "erroroffset passed as NULL")
// clang-format on

static const struct compile_error_texts {
#define X(code, text) char code[sizeof(text)];
  COMPILE_ERROR_TEXTS
#undef X
} compile_error_texts = {
#define X(code, text) text,
  COMPILE_ERROR_TEXTS
#undef X
};

STATIC_ASSERT(sizeof(compile_error_texts) <= UINT16_MAX, compile_error_texts_size);

static const uint16_t compile_error_offsets[] = {
#define X(code, text) offsetof(struct compile_error_texts, code),
  COMPILE_ERROR_TEXTS
#undef X
};

#undef COMPILE_ERROR_TEXTS
#undef PCRE2_ERROR_BACKSLASH_C_SYNTAX_MESSAGE

/* Match-time and UTF error texts are in the same format. */

// clang-format off
#define MATCH_ERROR_TEXTS                                                                      \
  X(e000, "no error")                                                                          \
  X(e001, "no match")                                                                          \
  X(e002, "partial match")                                                                     \
  X(e003, "UTF-8 error: 1 byte missing at end")                                                \
  X(e004, "UTF-8 error: 2 bytes missing at end")                                               \
  X(e005, "UTF-8 error: 3 bytes missing at end")                                               \
  X(e006, "UTF-8 error: 4 bytes missing at end")                                               \
  X(e007, "UTF-8 error: 5 bytes missing at end")                                               \
  X(e008, "UTF-8 error: byte 2 top bits not 0x80")                                             \
  X(e009, "UTF-8 error: byte 3 top bits not 0x80")                                             \
  X(e010, "UTF-8 error: byte 4 top bits not 0x80")                                             \
  X(e011, "UTF-8 error: byte 5 top bits not 0x80")                                             \
  X(e012, "UTF-8 error: byte 6 top bits not 0x80")                                             \
  X(e013, "UTF-8 error: 5-byte character is not allowed (RFC 3629)")                           \
  X(e014, "UTF-8 error: 6-byte character is not allowed (RFC 3629)")                           \
  X(e015, "UTF-8 error: code points greater than 0x10ffff are not defined")                    \
  X(e016, "UTF-8 error: code points 0xd800-0xdfff are not defined")                            \
  X(e017, "UTF-8 error: overlong 2-byte sequence")                                             \
  X(e018, "UTF-8 error: overlong 3-byte sequence")                                             \
  X(e019, "UTF-8 error: overlong 4-byte sequence")                                             \
  X(e020, "UTF-8 error: overlong 5-byte sequence")                                             \
  X(e021, "UTF-8 error: overlong 6-byte sequence")                                             \
  X(e022, "UTF-8 error: isolated byte with 0x80 bit set")                                      \
  X(e023, "UTF-8 error: illegal byte (0xfe or 0xff)")                                          \
  X(e024, "UTF-16 error: missing low surrogate at end")                                        \
  X(e025, "UTF-16 error: invalid low surrogate")                                               \
  X(e026, "UTF-16 error: isolated low surrogate")                                              \
  X(e027, "UTF-32 error: code points 0xd800-0xdfff are not defined")                           \
  X(e028, "UTF-32 error: code points greater than 0x10ffff are not defined")                   \
  X(e029, "bad data value")                                                                    \
  X(e030, "patterns do not all use the same character tables")                                 \
  X(e031, "magic number missing")                                                              \
  X(e032, "pattern compiled in wrong mode: 8/16/32-bit error")                                 \
  X(e033, "bad offset value")                                                                  \
  X(e034, "bad option value")                                                                  \
  X(e035, "invalid replacement string")                                                        \
  X(e036, "bad offset into UTF string")                                                        \
  X(e037, "callout error code") /* Never returned by PCRE2 itself */                           \
  X(e038, "invalid data in workspace for DFA restart")                                         \
  X(e039, "too much recursion for DFA matching")                                               \
  X(e040, "backreference condition or recursion test is not supported for DFA matching")       \
  X(e041, "function is not supported for DFA matching")                                        \
  X(e042, "pattern contains an item that is not supported for DFA matching")                   \
  X(e043, "workspace size exceeded in DFA matching")                                           \
  X(e044, "internal error - pattern overwritten?")                                             \
  X(e045, "bad JIT option")                                                                    \
  X(e046, "JIT stack limit reached")                                                           \
  X(e047, "match limit exceeded")                                                              \
  X(e048, "no more memory")                                                                    \
  X(e049, "unknown substring")                                                                 \
  X(e050, "non-unique substring name")                                                         \
  X(e051, "NULL argument passed")                                                              \
  X(e052, "nested recursion at the same subject position")                                     \
  X(e053, "matching depth limit exceeded")                                                     \
  X(e054, "requested value is not available")                                                  \
  X(e055, "requested value is not set")                                                        \
  X(e056, "offset limit set without PCRE2_USE_OFFSET_LIMIT")                                   \
  X(e057, "bad escape sequence in replacement string")                                         \
  X(e058, "expected closing curly bracket in replacement string")                              \
  X(e059, "bad substitution in replacement string")                                            \
  X(e060, "match with end before start or start moved backwards is not supported")             \
  X(e061, "too many replacements (more than INT_MAX)")                                         \
  X(e062, "bad serialized data")                                                               \
  X(e063, "heap limit exceeded")                                                               \
  X(e064, "invalid syntax")                                                                    \
  X(e065, "internal error: duplicate substitution match")                                      \
  X(e066, "PCRE2_MATCH_INVALID_UTF is not supported for DFA matching")                         \
  X(e067, "internal error: invalid substring offset")                                          \
  X(e068, "feature is not supported by the JIT compiler")                                      \
  X(e069, "error performing replacement case transformation")                                  \
  X(e070, "replacement too large (string would be longer than SIZE_MAX)")                      \
  X(e071, "substitute pattern differs from prior match call")                                  \
  X(e072, "substitute subject differs from prior match call")                                  \
  X(e073, "substitute start offset differs from prior match call")                             \
  X(e074, "substitute options differ from prior match call")                                   \
  X(e075, "disallowed use of \\K in lookaround")                                               \
  X(e076, "replacement $' or $_ not supported with partial match")
// clang-format on

static const struct match_error_texts {
#define X(code, text) char code[sizeof(text)];
  MATCH_ERROR_TEXTS
#undef X
} match_error_texts = {
#define X(code, text) text,
  MATCH_ERROR_TEXTS
#undef X
};

STATIC_ASSERT(sizeof(match_error_texts) <= UINT16_MAX, match_error_texts_size);

static const uint16_t match_error_offsets[] = {
#define X(code, text) offsetof(struct match_error_texts, code),
  MATCH_ERROR_TEXTS
#undef X
};

#undef MATCH_ERROR_TEXTS


/*************************************************
*            Return error message                *
*************************************************/

/* This function copies an error message into a buffer whose units are of an
appropriate width. Error numbers are positive for compile-time errors, and
negative for match-time and UTF errors. Zero returns "no error".

The reason for the positive compile-time errors starting from value 100 is
to reduce confusion over whether error codes returned from API functions
should be negated. No arithmetic (including unary negation) should ever be
performed on error codes by clients.

Arguments:
  errorcode     error number
  buffer        where to put the message (zero terminated)
  bufflen       size of the buffer in code units

Returns:        length of message if all is well
                negative on error
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_get_error_message(int errorcode, PCRE2_UCHAR *buffer, PCRE2_SIZE bufflen)
{
  PCRE2_ASSERT(bufflen == 0 || buffer != NULL);

  const unsigned char *message;
  if (errorcode >= COMPILE_ERROR_BASE) // Compile error
  {
    int n = errorcode - COMPILE_ERROR_BASE;
    if (n >= (int)(sizeof(compile_error_offsets) / sizeof(compile_error_offsets[0])))
      return PCRE2_ERROR_BADDATA;
    message = (const unsigned char *)&compile_error_texts + compile_error_offsets[n];
  }
  else if (errorcode <= 0 && errorcode != INT_MIN) // Match or UTF error, or no error
  {
    int n = -errorcode;
    if (n >= (int)(sizeof(match_error_offsets) / sizeof(match_error_offsets[0])))
      return PCRE2_ERROR_BADDATA;
    message = (const unsigned char *)&match_error_texts + match_error_offsets[n];
  }
  else // Invalid error number
  {
    return PCRE2_ERROR_BADDATA;
  }

  if (bufflen == 0)
    return PCRE2_ERROR_NOMEMORY;

  int rc = 0;
  PCRE2_SIZE i;
  for (i = 0; *message != 0; i++)
  {
    if (i >= bufflen - 1)
    {
      rc = PCRE2_ERROR_NOMEMORY;
      break;
    }

    buffer[i] = *message++;
  }

#if defined EBCDIC && 'a' != 0x81
  /* If compiling for EBCDIC, but the compiler's string literals are not EBCDIC,
  then we are in the "force EBCDIC 1047" mode. I have chosen to add a few lines
  here to translate the error strings on the fly, rather than require the string
  literals above to be written out arduously using the "STR_XYZ" macros. */
  for (PCRE2_SIZE j = 0; j < i; ++j)
    buffer[j] = PRIV(ascii_to_ebcdic_1047)[buffer[j]];
#endif

  buffer[i] = 0; // Terminate message, even if truncated.
  return rc ? rc : (int)i;
}

/* End of pcre2_error.c */
