/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
         New API code Copyright (c) 2014 University of Cambridge

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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre2_internal.h"


/* FIXME: check out the errors */

#define STRING(a)  # a
#define XSTRING(s) STRING(s)

/* The texts of compile-time error messages. Do not ever re-use any error
number, because they are documented. Always add a new error instead. Messages
marked DEAD below are no longer used. This used to be a table of strings, but
in order to reduce the number of relocations needed when a shared library is
loaded dynamically, it is now one long string. We cannot use a table of
offsets, because the lengths of inserts such as XSTRING(MAX_NAME_SIZE) are not
known. Instead, pcre2_get_error_message() counts through to the one it wants -
this isn't a performance issue because these strings are used only when there
is an error.

Each substring ends with \0 to insert a null character. This includes the final
substring, so that the whole string ends with \0\0, which can be detected when
counting through. */

static const char compile_error_texts[] =
  "no error\0"
  "\\ at end of pattern\0"
  "\\c at end of pattern\0"
  "unrecognized character follows \\\0"
  "numbers out of order in {} quantifier\0"
  /* 5 */
  "number too big in {} quantifier\0"
  "missing terminating ] for character class\0"
  "invalid escape sequence in character class\0"
  "range out of order in character class\0"
  "nothing to repeat\0"
  /* 10 */
  "operand of unlimited repeat could match the empty string\0"  /** DEAD **/
  "internal error: unexpected repeat\0"
  "unrecognized character after (? or (?-\0"
  "POSIX named classes are supported only within a class\0"
  "missing )\0"
  /* 15 */
  "reference to non-existent subpattern\0"
  "erroffset passed as NULL\0"
  "unknown option bit(s) set\0"
  "missing ) after comment\0"
  "parentheses nested too deeply\0"  /** DEAD **/
  /* 20 */
  "regular expression is too large\0"
  "failed to get memory\0"
  "unmatched parentheses\0"
  "internal error: code overflow\0"
  "unrecognized character after (?<\0"
  /* 25 */
  "lookbehind assertion is not fixed length\0"
  "malformed number or name after (?(\0"
  "conditional group contains more than two branches\0"
  "assertion expected after (?(\0"
  "(?R or (?[+-]digits must be followed by )\0"
  /* 30 */
  "unknown POSIX class name\0"
  "POSIX collating elements are not supported\0"
  "this version of PCRE is compiled without UTF support\0"
  "spare error\0"  /** DEAD **/
  "character value in \\x{} or \\o{} is too large\0"
  /* 35 */
  "invalid condition (?(0)\0"
  "\\C not allowed in lookbehind assertion\0"
  "PCRE does not support \\L, \\l, \\N{name}, \\U, or \\u\0"
  "number after (?C is > 255\0"
  "closing ) for (?C expected\0"
  /* 40 */
  "recursive call could loop indefinitely\0"
  "unrecognized character after (?P\0"
  "syntax error in subpattern name (missing terminator)\0"
  "two named subpatterns have the same name\0"
  "invalid UTF-8 string\0"
  /* 45 */
  "support for \\P, \\p, and \\X has not been compiled\0"
  "malformed \\P or \\p sequence\0"
  "unknown property name after \\P or \\p\0"
  "subpattern name is too long (maximum " XSTRING(MAX_NAME_SIZE) " characters)\0"
  "too many named subpatterns (maximum " XSTRING(MAX_NAME_COUNT) ")\0"
  /* 50 */
  "repeated subpattern is too long\0"    /** DEAD **/
  "octal value is greater than \\377 in 8-bit non-UTF-8 mode\0"
  "internal error: overran compiling workspace\0"
  "internal error: previously-checked referenced subpattern not found\0"
  "DEFINE group contains more than one branch\0"
  /* 55 */
  "repeating a DEFINE group is not allowed\0"  /** DEAD **/
  "inconsistent NEWLINE options\0"
  "\\g is not followed by a braced, angle-bracketed, or quoted name/number or by a plain number\0"
  "a numbered reference must not be zero\0"
  "an argument is not allowed for (*ACCEPT), (*FAIL), or (*COMMIT)\0"
  /* 60 */
  "(*VERB) not recognized or malformed\0"
  "number is too big\0"
  "subpattern name expected\0"
  "digit expected after (?+\0"
  "] is an invalid data character in JavaScript compatibility mode\0"
  /* 65 */
  "different names for subpatterns of the same number are not allowed\0"
  "(*MARK) must have an argument\0"
  "this version of PCRE is not compiled with Unicode property support\0"
  "\\c must be followed by an ASCII character\0"
  "\\k is not followed by a braced, angle-bracketed, or quoted name\0"
  /* 70 */
  "internal error: unknown opcode in find_fixedlength()\0"
  "\\N is not supported in a class\0"
  "too many forward references\0"
  "disallowed Unicode code point (>= 0xd800 && <= 0xdfff)\0"
  "invalid UTF-16 string\0"
  /* 75 */
  "name is too long in (*MARK), (*PRUNE), (*SKIP), or (*THEN)\0"
  "character value in \\u.... sequence is too large\0"
  "invalid UTF-32 string\0"
  "setting UTF is disabled by the application\0"
  "non-hex character in \\x{} (closing brace missing?)\0"
  /* 80 */
  "non-octal character in \\o{} (closing brace missing?)\0"
  "missing opening brace after \\o\0"
  "parentheses are too deeply nested\0"
  "invalid range in character class\0"
  "group name must start with a non-digit\0"
  /* 85 */
  "parentheses are too deeply nested (stack check)\0"
  ;

/* Match-time error texts are in the same format. */

static const char match_error_texts[] =
  "no error\0"
  "no match\0" 
  "bad count value\0"
  "pattern compiled with other endianness\0"
  "bad length\0"
  /* -5 */ 
  "magic number missing\0"
  "pattern compiled in wrong mode: 8/16/32-bit error\0"
  "bad offset value\0"
  "bad option value\0"
  "bad UTF string\0"
  /* -10 */
  "bad offset into UTF string\0"
  "callout error code\0"              /* Never returned by PCRE2 itself */   
  "internal error - pattern overwritten?\0"
  "bad JIT option\0"
  "JIT stack limit reached\0"
  /* -15 */
  "match limit exceeded\0"
  "no more memory\0"
  "unknown substring\0" 
  "NULL argument passed\0"
  "partial match\0"
  /* -20 */ 
  "nested recursion at the same subject position\0"
  "recursion limit exceeded\0"
  "unknown opcode - pattern overwritten?\0"
  "value unset\0"                     /* Used by pcre2_pattern_info() */
  "spare -24\0"
  /* -25 */  
  "spare -25\0" 
  "spare -26\0" 
  "spare -27\0" 
  "spare -28\0" 
  "spare -29\0" 
  /* -30 */ 
  "invalid data in workspace for DFA restart\0"
  "too much recursion for DFA matching\0"
  "backreference condition or recursion test not supported for DFA matching\0"
  "item unsupported for DFA matching\0"
  "match limit not supported for DFA matching\0"
  /* -35 */ 
  "workspace size exceeded in DFA matching\0"
  "spare -36\0"
  "spare -37\0"
  "spare -38\0"
  "spare -39\0"
  /* -40 */
  "spare -39\0"
  ; 


/*************************************************
*            Return error message                *
*************************************************/

/* This function copies an error message into a buffer whose units are of an 
appropriate width. Error numbers are positive for compile-time errors, and 
negative for exec-time errors.

Arguments:
  enumber       error number
  buffer        where to put the message (zero terminated)
  size          size of the buffer
  
Returns:        length of message if all is well
                negative on error
*/                    

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_get_error_message(int enumber, PCRE2_UCHAR *buffer, size_t size)
{
char xbuff[128];
const char *message;
size_t i;
uint32_t n;

if (size == 0) return PCRE2_ERROR_NOMEMORY;

if (enumber > 0)    /* Compile-time error */
  {
  message = compile_error_texts;
  n = enumber;
  }  
  
else                /* Match-time error */
  {
  message = match_error_texts;
  n = -enumber;
  }    
     
for (; n > 0; n--)
  {
  while (*message++ != CHAR_NULL) {};
  if (*message == CHAR_NULL) 
    { 
    sprintf(xbuff, "Internal error: no text for error %d", enumber);
    break; 
    }
  }    

for (i = 0; *message != 0; i++)
  {
  if (i >= size - 1)
    {
    buffer[i] = 0;     /* Terminate partial message */
    return PCRE2_ERROR_NOMEMORY;
    }
  buffer[i] = *message++;
  }
 
buffer[i] = 0;
return i;            
}

/* End of pcre2_error.c */
