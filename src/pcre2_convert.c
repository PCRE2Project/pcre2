/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
          New API code Copyright (c) 2016-2017 University of Cambridge

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

#define TYPE_OPTIONS (PCRE2_CONVERT_GLOB_BASIC|PCRE2_CONVERT_GLOB_BASH| \
  PCRE2_CONVERT_POSIX_BASIC|PCRE2_CONVERT_POSIX_EXTENDED)

#define ALL_OPTIONS (PCRE2_CONVERT_UTF|PCRE2_CONVERT_NO_UTF_CHECK| \
  TYPE_OPTIONS)

#define DUMMY_BUFFER_SIZE 100

/* Some pcre2_compile() error numbers are used herein. */

#define ERROR_END_BACKSLASH 101
#define ERROR_MISSING_SQUARE_BRACKET 106
#define ERROR_MISSING_CLOSING_PARENTHESIS 114
#define ERROR_TOO_DEEP_NESTING 119
#define ERROR_NO_UNICODE 132

/* Generated pattern fragments */

#define STR_BACKSLASH_A STR_BACKSLASH STR_A
#define STR_BACKSLASH_z STR_BACKSLASH STR_z
#define STR_COLON_RIGHT_SQUARE_BRACKET STR_COLON STR_RIGHT_SQUARE_BRACKET
#define STR_DOT_STAR_LOOKBEHIND STR_DOT STR_ASTERISK STR_LEFT_PARENTHESIS STR_QUESTION_MARK STR_LESS_THAN_SIGN STR_EQUALS_SIGN
#define STR_LOOKAHEAD_NOT_DOT STR_LEFT_PARENTHESIS STR_QUESTION_MARK STR_EXCLAMATION_MARK STR_BACKSLASH STR_DOT STR_RIGHT_PARENTHESIS
#define STR_QUERY_s STR_LEFT_PARENTHESIS STR_QUESTION_MARK STR_s STR_RIGHT_PARENTHESIS

/* States for range and POSIX processing */

enum { RANGE_NOT_STARTED, RANGE_STARTING, RANGE_STARTED };
enum { POSIX_START_REGEX, POSIX_ANCHORED, POSIX_NOT_BRACKET, 
       POSIX_CLASS_NOT_STARTED, POSIX_CLASS_STARTING, POSIX_CLASS_STARTED };

/* Macro to add a character string to the output buffer, checking for overflow. */

#define PUTCHARS(string) \
  { \
  for (s = (char *)(string); *s != 0; s++) \
    { \
    if (p >= endp) return PCRE2_ERROR_NOMEMORY; \
    *p++ = *s; \
    } \
  }

static const char *pcre2_escaped_literals = "\\{}?*+[]()|.^$";


/*************************************************
*           Convert a POSIX pattern              *
*************************************************/

/* This function handles both basic and extended POSIX patterns.

Arguments:
  pattype        the pattern type
  pattern        the pattern
  plength        length in code units
  utf            TRUE if UTF
  use_buffer     where to put the output
  use_length     length of use_buffer
  bufflenptr     where to put the used length
  dummyrun       TRUE if a dummy run
  ccontext       the convert context

Returns:         0 => success
                !0 => error code
*/

static int
convert_posix(uint32_t pattype, PCRE2_SPTR pattern, PCRE2_SIZE plength,
  BOOL utf, PCRE2_UCHAR *use_buffer, PCRE2_SIZE use_length,
  PCRE2_SIZE *bufflenptr, BOOL dummyrun, pcre2_convert_context *ccontext)
{
char *s;
PCRE2_SPTR posix = pattern;
PCRE2_UCHAR *p = use_buffer;
PCRE2_UCHAR *pp = p;
PCRE2_UCHAR *endp = p + use_length - 1;  /* Allow for trailing zero */
PCRE2_SIZE convlength = 0;

uint32_t bracount = 0;
uint32_t posix_state = POSIX_START_REGEX;
uint32_t lastspecial = 0;
BOOL extended = (pattype & PCRE2_CONVERT_POSIX_EXTENDED) != 0;
BOOL nextisliteral = FALSE;

(void)utf;       /* Not used when Unicode not supported */
(void)ccontext;  /* Not currently used */

/* Initialize default for error offset as end of input. */

*bufflenptr = plength;

/* Now scan the input. */

while (plength > 0)
  {
  uint32_t c, sc;
  int clength = 1;

  /* Add in the length of the last item, then, if in the dummy run, pull the
  pointer back to the start of the (temporary) buffer and then remember the
  start of the next item. */

  convlength += p - pp;
  if (dummyrun) p = use_buffer;
  pp = p;

  /* Pick up the next character */

#ifndef SUPPORT_UNICODE
  c = *posix;
#else
  GETCHARLENTEST(c, posix, clength);
#endif
  posix += clength;
  plength -= clength;

  sc = nextisliteral? 0 : c;
  nextisliteral = FALSE;

  /* Handle a character within a class. */

  if (posix_state >= POSIX_CLASS_NOT_STARTED)
    {
    if (c == CHAR_RIGHT_SQUARE_BRACKET) 
      {
      PUTCHARS(STR_RIGHT_SQUARE_BRACKET); 
      posix_state = POSIX_NOT_BRACKET;
      }
      
    /* Not the end of the class */
       
    else 
      {
      switch (posix_state)
        {
        case POSIX_CLASS_STARTED:
        if (c <= 127 && islower(c)) break;  /* Remain in started state */
        posix_state = POSIX_CLASS_NOT_STARTED;       
        if (c == CHAR_COLON  && plength > 0 && 
            *posix == CHAR_RIGHT_SQUARE_BRACKET)
          {
          PUTCHARS(STR_COLON_RIGHT_SQUARE_BRACKET);
          plength--; 
          posix++;
          continue;    /* With next character after :] */ 
          }
        /* Fall through */  
          
        case POSIX_CLASS_NOT_STARTED:  
        if (c == CHAR_LEFT_SQUARE_BRACKET) 
          posix_state = POSIX_CLASS_STARTING;
        break;
        
        case POSIX_CLASS_STARTING:
        if (c == CHAR_COLON) posix_state = POSIX_CLASS_STARTED;
        break;
        }   
 
      if (c == CHAR_BACKSLASH) PUTCHARS(STR_BACKSLASH);
      if (p + clength > endp) return PCRE2_ERROR_NOMEMORY;
      memcpy(p, posix - clength, CU2BYTES(clength));
      p += clength;
      } 
    }

  /* Handle a character not within a class. */

  else switch(sc)
    {
    case CHAR_LEFT_SQUARE_BRACKET:
    PUTCHARS(STR_LEFT_SQUARE_BRACKET);
    
    /* Handle special cases [[:<:]] and [[:>:]] (which PCRE does support) */
     
    if (plength >= 6)
      {
      if (posix[0] == CHAR_LEFT_SQUARE_BRACKET &&
          posix[1] == CHAR_COLON &&
          (posix[2] == CHAR_LESS_THAN_SIGN || 
           posix[2] == CHAR_GREATER_THAN_SIGN) &&
          posix[3] == CHAR_COLON &&
          posix[4] == CHAR_RIGHT_SQUARE_BRACKET &&    
          posix[5] == CHAR_RIGHT_SQUARE_BRACKET)
        {
        if (p + 6 > endp) return PCRE2_ERROR_NOMEMORY;
        memcpy(p, posix, CU2BYTES(6));
        p += 6;
        posix += 6;
        plength -= 6;  
        continue;  /* With next character */ 
        }
      }   
      
    /* Handle "normal" character classes */
 
    posix_state = POSIX_CLASS_NOT_STARTED; 

    /* Handle ^ and ] as first characters */

    if (plength > 0)
      {
      if (*posix == CHAR_CIRCUMFLEX_ACCENT)
        {
        posix++;
        plength--;
        PUTCHARS(STR_CIRCUMFLEX_ACCENT);
        }
      if (plength > 0 && *posix == CHAR_RIGHT_SQUARE_BRACKET)
        {
        posix++;
        plength--;
        PUTCHARS(STR_RIGHT_SQUARE_BRACKET);
        }    
      }
    break;

    case CHAR_BACKSLASH:
    if (plength <= 0) return ERROR_END_BACKSLASH;
    if (!extended && *posix < 127 && strchr("?+|()0123456789", *posix) != NULL)  
      {
      if (isdigit(*posix)) PUTCHARS(STR_BACKSLASH); 
      if (p + 1 > endp) return PCRE2_ERROR_NOMEMORY;
      lastspecial = *p++ = *posix++;
      plength--;  
      }
    else nextisliteral = TRUE;
    break;
    
    case CHAR_RIGHT_PARENTHESIS:
    if (!extended || bracount == 0) goto ESCAPE_LITERAL;
    bracount--;
    goto COPY_SPECIAL;

    case CHAR_LEFT_PARENTHESIS:
    bracount++;
    /* Fall through */  

    case CHAR_QUESTION_MARK:
    case CHAR_PLUS:
    case CHAR_LEFT_CURLY_BRACKET:   
    case CHAR_RIGHT_CURLY_BRACKET:   
    case CHAR_VERTICAL_LINE:
    if (!extended) goto ESCAPE_LITERAL;
    /* Fall through */ 
    
    case CHAR_DOT:
    case CHAR_DOLLAR_SIGN:   
    posix_state = POSIX_NOT_BRACKET; 
    COPY_SPECIAL:
    lastspecial = c; 
    if (p + 1 > endp) return PCRE2_ERROR_NOMEMORY;
    *p++ = c;
    break; 

    case CHAR_ASTERISK:
    if (lastspecial != CHAR_ASTERISK) 
      {
      if (!extended && posix_state < POSIX_NOT_BRACKET)
        goto ESCAPE_LITERAL; 
      goto COPY_SPECIAL;
      } 
    break;   /* Ignore second and subsequent asterisks */  

    case CHAR_CIRCUMFLEX_ACCENT:
    if (extended) goto COPY_SPECIAL;
    if (posix_state == POSIX_START_REGEX || 
        lastspecial == CHAR_LEFT_PARENTHESIS) 
      {
      posix_state = POSIX_ANCHORED;
      goto COPY_SPECIAL;
      } 
    /* Fall through */      

    default:
    if (c < 128 && strchr(pcre2_escaped_literals, c) != NULL)
      {
      ESCAPE_LITERAL: 
      PUTCHARS(STR_BACKSLASH);
      }
    lastspecial = 0xff;  /* Indicates nothing special */   
    if (p + clength > endp) return PCRE2_ERROR_NOMEMORY;
    memcpy(p, posix - clength, CU2BYTES(clength));
    p += clength;
    posix_state = POSIX_NOT_BRACKET; 
    break;
    }
  }

if (posix_state >= POSIX_CLASS_NOT_STARTED) 
  return ERROR_MISSING_SQUARE_BRACKET;
convlength += p - pp;        /* Final segment */
*bufflenptr = convlength;
*p++ = 0;
return 0;
}



/*************************************************
*           Convert a glob pattern               *
*************************************************/

/* For a basic glob, only * ? and [...] are recognized.

Arguments:
  pattype        the pattern type
  pattern        the pattern
  plength        length in code units
  utf            TRUE if UTF
  use_buffer     where to put the output
  use_length     length of use_buffer
  bufflenptr     where to put the used length
  dummyrun       TRUE if a dummy run
  ccontext       the convert context

Returns:         0 => success
                !0 => error code
*/

static int
convert_glob(uint32_t pattype, PCRE2_SPTR pattern, PCRE2_SIZE plength,
  BOOL utf, PCRE2_UCHAR *use_buffer, PCRE2_SIZE use_length,
  PCRE2_SIZE *bufflenptr, BOOL dummyrun, pcre2_convert_context *ccontext)
{
char *s;
char not_sep_class[8];
char lookbehind_not_sep[12];
PCRE2_SPTR glob = pattern;
PCRE2_UCHAR *p = use_buffer;
PCRE2_UCHAR *pp = p;
PCRE2_UCHAR *endp = p + use_length - 1;  /* Allow for trailing zero */
PCRE2_SIZE convlength = 0;
uint32_t range_start = 0;
uint32_t range_state = RANGE_NOT_STARTED;
uint32_t posix_class_state = POSIX_CLASS_NOT_STARTED;
BOOL inclass = FALSE;
BOOL nextisliteral = FALSE;
BOOL endswith = FALSE;
BOOL sep_in_range = FALSE;

(void)utf;  /* Not used when Unicode not supported */
(void)pattype; /* Pro tem */

/* Set up a string containing [^<sep>] where <sep> is the possibly escaped part
separator. */

s = not_sep_class;
*s++ = CHAR_LEFT_SQUARE_BRACKET;
*s++ = CHAR_CIRCUMFLEX_ACCENT;
if (ccontext->glob_separator == CHAR_BACKSLASH) *s++ = CHAR_BACKSLASH;
*s++ = ccontext->glob_separator;
*s++ = CHAR_RIGHT_SQUARE_BRACKET;
*s++ = 0;

/* Set up a string containing (?<!<sep>) where <sep> is the possibly escaped
part separator. */

s = lookbehind_not_sep;
*s++ = CHAR_LEFT_PARENTHESIS;
*s++ = CHAR_QUESTION_MARK;
*s++ = CHAR_LESS_THAN_SIGN;
*s++ = CHAR_EXCLAMATION_MARK;
if (ccontext->glob_separator == CHAR_BACKSLASH) *s++ = CHAR_BACKSLASH;
*s++ = ccontext->glob_separator;
*s++ = CHAR_RIGHT_PARENTHESIS;
*s++ = 0;

/* Initialize default for error offset as end of input. */

*bufflenptr = plength;

/* If the pattern starts with * and contains at least one more character but no
other asterisks or part separators, it means "ends with what follows". This can
be optimized. */

if (plength > 1 && *glob == CHAR_ASTERISK)
  {
  PCRE2_SPTR pt;
  for (pt = glob + plength - 1; pt > glob; pt--)
    if (*pt == ccontext->glob_separator || *pt == CHAR_ASTERISK) break;
  endswith = pt == glob;
  if (endswith) PUTCHARS(STR_QUERY_s);
  }

/* Output starts with \A and ends with \z and a binary zero. */

PUTCHARS(STR_BACKSLASH_A);

/* If the pattern starts with a wildcard, it must not match a subject that 
starts with a dot. */

if (plength > 1 && 
    (*glob == CHAR_ASTERISK || *glob == CHAR_QUESTION_MARK ||
     *glob == CHAR_LEFT_SQUARE_BRACKET))
  PUTCHARS(STR_LOOKAHEAD_NOT_DOT);

/* Now scan the input */

while (plength > 0)
  {
  uint32_t c, sc;
  int clength = 1;

  /* Add in the length of the last item, then, if in the dummy run, pull the
  pointer back to the start of the (temporary) buffer and then remember the
  start of the next item. */

  convlength += p - pp;
  if (dummyrun) p = use_buffer;
  pp = p;

  /* Pick up the next character */

#ifndef SUPPORT_UNICODE
  c = *glob;
#else
  GETCHARLENTEST(c, glob, clength);
#endif
  glob += clength;
  plength -= clength;

  sc = nextisliteral? 0 : c;
  nextisliteral = FALSE;

  /* Handle a character within a class. */

  if (inclass)
    {
    /* A literal part separator is a syntax error */
      
    if (c == ccontext->glob_separator)
      {
      *bufflenptr = glob - pattern - 1;
      return PCRE2_ERROR_CONVERT_SYNTAX;
      }
      
    /* At the end of the class, add a lookbehind for not the separator if any
    range in the class includes the separator. */ 
          
    if (c == CHAR_RIGHT_SQUARE_BRACKET) 
      {
      PUTCHARS(STR_RIGHT_SQUARE_BRACKET); 
      if (sep_in_range) PUTCHARS(lookbehind_not_sep);
      inclass = FALSE;
      }
      
    /* Not the end of the class */
       
    else 
      {
      switch (posix_class_state)
        {
        case POSIX_CLASS_STARTED:
        if (c <= 127 && islower(c)) break;  /* Remain in started state */
        posix_class_state = POSIX_CLASS_NOT_STARTED;       
        if (c == CHAR_COLON  && plength > 0 && 
            *glob == CHAR_RIGHT_SQUARE_BRACKET)
          {
          PUTCHARS(STR_COLON_RIGHT_SQUARE_BRACKET);
          plength--; 
          glob++;
          continue;    /* With next character after :] */ 
          }
        /* Fall through */  
          
        case POSIX_CLASS_NOT_STARTED:  
        if (c == CHAR_LEFT_SQUARE_BRACKET) 
          posix_class_state = POSIX_CLASS_STARTING;
        break;
        
        case POSIX_CLASS_STARTING:
        if (c == CHAR_COLON) posix_class_state = POSIX_CLASS_STARTED;
        break;
        }   
 
      if (range_state == RANGE_STARTING && c == CHAR_MINUS)
        range_state = RANGE_STARTED;
      else if (range_state == RANGE_STARTED)
        { 
        if (range_start <= ccontext->glob_separator &&
            c >= ccontext->glob_separator)
          sep_in_range = TRUE;   
        range_state = RANGE_NOT_STARTED;
        }
      else
        {      
        range_state = RANGE_STARTING;
        range_start = c; 
        }
         
      if (c == CHAR_BACKSLASH) PUTCHARS(STR_BACKSLASH);
      if (p + clength > endp) return PCRE2_ERROR_NOMEMORY;
      memcpy(p, glob - clength, CU2BYTES(clength));
      p += clength;
      } 
    }

  /* Handle a character not within a class. */

  else switch(sc)
    {
    case CHAR_ASTERISK:
    if (endswith)
      {
      PUTCHARS(STR_DOT_STAR_LOOKBEHIND);
      }
    else
      {
      PUTCHARS(not_sep_class);
      PUTCHARS(STR_ASTERISK);
      }
    break;

    case CHAR_QUESTION_MARK:
    PUTCHARS(not_sep_class);
    break;

    case CHAR_LEFT_SQUARE_BRACKET:
    posix_class_state = POSIX_CLASS_NOT_STARTED; 
    range_state = RANGE_NOT_STARTED; 
    sep_in_range = FALSE;
    inclass = TRUE;
    PUTCHARS(STR_LEFT_SQUARE_BRACKET);

    /* Handle ! and ] as first characters */

    if (plength > 0)
      {
      if (*glob == CHAR_EXCLAMATION_MARK)
        {
        glob++;
        plength--;
        PUTCHARS(STR_CIRCUMFLEX_ACCENT);
        }
      if (plength > 0 && *glob == CHAR_RIGHT_SQUARE_BRACKET)
        {
        glob++;
        plength--;
        PUTCHARS(STR_RIGHT_SQUARE_BRACKET);
        range_start = CHAR_RIGHT_SQUARE_BRACKET;
        range_state = RANGE_STARTING;
        }    
      }
    break;

    case CHAR_BACKSLASH:
    if (plength <= 0) return ERROR_END_BACKSLASH;
    nextisliteral = TRUE;
    break;

    default:
    if (c < 128 && strchr(pcre2_escaped_literals, c) != NULL)
      {
      PUTCHARS(STR_BACKSLASH);
      }
    if (p + clength > endp) return PCRE2_ERROR_NOMEMORY;
    memcpy(p, glob - clength, CU2BYTES(clength));
    p += clength;
    break;
    }
  }

if (inclass) return ERROR_MISSING_SQUARE_BRACKET;

if (endswith) PUTCHARS(STR_RIGHT_PARENTHESIS);
PUTCHARS(STR_BACKSLASH_z);
convlength += p - pp;        /* Final segment */
*bufflenptr = convlength;
*p++ = 0;
return 0;
}


/*************************************************
*           Convert a glob pattern               *
*************************************************/

/* Context for writing the output into a buffer. */

typedef struct pcre2_output_context {
  PCRE2_UCHAR *output;                  /* current output position */
  PCRE2_SPTR output_end;                /* output end */
  PCRE2_SIZE output_size;               /* size of the output */
  uint8_t out_str[8];                   /* string copied to the output */
} pcre2_output_context;


/* Write a character into the output.

Arguments:
  context        the bash glob context
  chr            the next character
*/

static void
convert_glob_bash_write(pcre2_output_context *context, PCRE2_UCHAR chr)
{
context->output_size++;

if (context->output < context->output_end)
  *context->output++ = chr;
}


/* Write a string into the output.

Arguments:
  context        the bash glob context
  length         length of context->out_str
*/

static void
convert_glob_bash_write_str(pcre2_output_context *context, PCRE2_SIZE length)
{
uint8_t *out_str = context->out_str;
PCRE2_UCHAR *output = context->output;
PCRE2_SPTR output_end = context->output_end;
PCRE2_SIZE output_size = context->output_size;

do
  {
  output_size++;

  if (output < output_end)
    *output++ = *out_str++;
  }
while (--length != 0);

context->output = output;
context->output_size = output_size;
}

/* Bash glob reading modes. */

#define PCRE2_BASH_GLOB_NORMAL           0
#define PCRE2_BASH_GLOB_QUOTED           1
#define PCRE2_BASH_GLOB_DOUBLE_QUOTED    2
#define PCRE2_BASH_GLOB_BACKSLASH        3

/* Maximum nesting level of enclosed groups. */

#define PCRE2_BASH_GLOB_MAX_NESTING      16

typedef struct pcre2_bash_glob_context {
  PCRE2_SPTR pattern;
  PCRE2_SPTR pattern_end;
  pcre2_output_context out;
  int read_mode;
  BOOL is_control_char;
} pcre2_bash_glob_context;

/* Read the next character from the glob. If the character
   is a control character context->is_control_char is set
   to TRUE. Otherwise this field is FALSE.

Arguments:
  context        the bash glob context
  utf            TRUE if UTF
*/

static BOOL
convert_glob_bash_read(pcre2_bash_glob_context *context, BOOL utf)
{
while (TRUE)
  {
  if (context->pattern >= context->pattern_end)
    return FALSE;

  context->pattern++;

#ifdef SUPPORT_UNICODE
  /* Intermediate unicode octets are always normal characters. */
  if (utf && NOT_FIRSTCU(context->pattern[-1]))
    {
    context->is_control_char = FALSE;
    return TRUE;
    }
#endif

  if (context->read_mode == PCRE2_BASH_GLOB_QUOTED)
    {
    if (context->pattern[-1] != CHAR_APOSTROPHE)
      return TRUE;

    context->read_mode = PCRE2_BASH_GLOB_NORMAL;
    continue;
    }
  else if (context->read_mode == PCRE2_BASH_GLOB_DOUBLE_QUOTED)
    {
    if (context->pattern[-1] == CHAR_BACKSLASH &&
        context->pattern < context->pattern_end &&
        (context->pattern[0] == CHAR_QUOTATION_MARK ||
         context->pattern[0] == CHAR_BACKSLASH))
      {
        context->pattern++;
        return TRUE;
      }
    else if (context->pattern[-1] != CHAR_QUOTATION_MARK)
      return TRUE;

    context->read_mode = PCRE2_BASH_GLOB_NORMAL;
    continue;
    }

  context->is_control_char = FALSE;

  if (context->pattern[-1] == CHAR_APOSTROPHE)
    {
    context->read_mode = PCRE2_BASH_GLOB_QUOTED;
    continue;
    }

  if (context->pattern[-1] == CHAR_QUOTATION_MARK)
    {
    context->read_mode = PCRE2_BASH_GLOB_DOUBLE_QUOTED;
    continue;
    }

  if (context->pattern[-1] == CHAR_BACKSLASH)
    {
    if (context->pattern < context->pattern_end)
      {
      context->pattern++;
      return TRUE;
      }

    context->read_mode = PCRE2_BASH_GLOB_BACKSLASH;
    return FALSE;
    }

  context->is_control_char = TRUE;
  return TRUE;
  }
}


/* Prints a wildcard into the output.

Arguments:
  context           the bash glob context
  separator         glob separator
  after_sep         whether the wildcard is right after a separator
*/

static void
convert_glob_bash_wildcard(pcre2_bash_glob_context *context,
  PCRE2_UCHAR separator, BOOL after_sep)
{
int len = 2;

context->out.out_str[0] = CHAR_LEFT_SQUARE_BRACKET;
context->out.out_str[1] = CHAR_CIRCUMFLEX_ACCENT;

if (separator == CHAR_BACKSLASH)
  {
  context->out.out_str[2] = CHAR_BACKSLASH;
  len = 3;
  }

if (after_sep)
  {
  context->out.out_str[len] = CHAR_DOT;
  len++;
  }

convert_glob_bash_write_str(&context->out, len);

convert_glob_bash_write(&context->out, separator);
convert_glob_bash_write(&context->out, CHAR_RIGHT_SQUARE_BRACKET);
}


/* Bash glob converter.

Arguments:
  pattype        the pattern type
  pattern        the pattern
  plength        length in code units
  utf            TRUE if UTF
  use_buffer     where to put the output
  use_length     length of use_buffer
  bufflenptr     where to put the used length
  dummyrun       TRUE if a dummy run
  ccontext       the convert context

Returns:         0 => success
                !0 => error code
*/

static int
convert_glob_bash(uint32_t pattype, PCRE2_SPTR pattern, PCRE2_SIZE plength,
  BOOL utf, PCRE2_UCHAR *use_buffer, PCRE2_SIZE use_length,
  PCRE2_SIZE *bufflenptr, BOOL dummyrun, pcre2_convert_context *ccontext)
{
pcre2_bash_glob_context context;
uint8_t group_types[PCRE2_BASH_GLOB_MAX_NESTING];
int nesting_level, result;
BOOL after_sep = TRUE;
PCRE2_UCHAR c;

/* Initialize default for error offset as end of input. */
context.pattern = pattern;
context.pattern_end = pattern + plength;
context.read_mode = PCRE2_BASH_GLOB_NORMAL;
context.out.output = use_buffer;
context.out.output_end = use_buffer + use_length;
context.out.output_size = 0;

context.out.out_str[0] = CHAR_BACKSLASH;
context.out.out_str[1] = CHAR_A;
convert_glob_bash_write_str(&context.out, 2);

nesting_level = 0;
result = 0;

while (convert_glob_bash_read(&context, utf))
  {
  c = context.pattern[-1];

  if (context.is_control_char)
    {
    if (c == CHAR_LEFT_PARENTHESIS)
      {
      /* ! Unexpected open parenthesis ! */
      result = ERROR_END_BACKSLASH;
      break;
      }

    if (c == CHAR_RIGHT_PARENTHESIS)
      {
      if (nesting_level == 0)
        {
        /* ! Unexpected open parenthesis ! */
        result = ERROR_END_BACKSLASH;
        break;
        }

      c = group_types[--nesting_level];

      convert_glob_bash_write(&context.out, CHAR_RIGHT_PARENTHESIS);
      if (c != CHAR_COMMERCIAL_AT)
        {
        convert_glob_bash_write(&context.out, c);
        convert_glob_bash_write(&context.out, CHAR_QUESTION_MARK);
        }

      after_sep = FALSE;
      continue;
      }

    if (c == CHAR_VERTICAL_LINE && nesting_level > 0)
      {
      convert_glob_bash_write(&context.out, CHAR_VERTICAL_LINE);

      after_sep = FALSE;
      continue;
      }

    if ((c == CHAR_QUESTION_MARK || c == CHAR_ASTERISK ||
         c == CHAR_PLUS || c == CHAR_COMMERCIAL_AT) &&
        context.pattern < context.pattern_end &&
        context.pattern[0] == CHAR_LEFT_PARENTHESIS)
      {
      if (nesting_level >= PCRE2_BASH_GLOB_MAX_NESTING)
        {
        result = ERROR_TOO_DEEP_NESTING;
        break;
        }

      if (after_sep)
        {
        context.out.out_str[0] = CHAR_LEFT_PARENTHESIS;
        context.out.out_str[1] = CHAR_QUESTION_MARK;
        context.out.out_str[2] = CHAR_EXCLAMATION_MARK;
        context.out.out_str[3] = CHAR_BACKSLASH;
        context.out.out_str[4] = CHAR_DOT;
        context.out.out_str[5] = CHAR_RIGHT_PARENTHESIS;
        convert_glob_bash_write_str(&context.out, 6);
        }

      context.pattern++;
      group_types[nesting_level++] = (uint8_t) c;

      context.out.out_str[0] = CHAR_LEFT_PARENTHESIS;
      context.out.out_str[1] = CHAR_QUESTION_MARK;
      context.out.out_str[2] = CHAR_COLON;
      convert_glob_bash_write_str(&context.out, 3);

      after_sep = FALSE;
      continue;
      }

    if (c == CHAR_ASTERISK)
      {
      if (nesting_level == 0 && context.pattern != pattern + 1)
        {
        context.out.out_str[0] = CHAR_LEFT_PARENTHESIS;
        context.out.out_str[1] = CHAR_ASTERISK;
        context.out.out_str[2] = CHAR_C;
        context.out.out_str[3] = CHAR_O;
        context.out.out_str[4] = CHAR_M;
        context.out.out_str[5] = CHAR_M;
        context.out.out_str[6] = CHAR_I;
        context.out.out_str[7] = CHAR_T;
        convert_glob_bash_write_str(&context.out, 8);
        convert_glob_bash_write(&context.out, CHAR_RIGHT_PARENTHESIS);
        }

      if (after_sep)
        {
        context.out.out_str[0] = CHAR_LEFT_PARENTHESIS;
        context.out.out_str[1] = CHAR_QUESTION_MARK;
        context.out.out_str[2] = CHAR_COLON;
        convert_glob_bash_write_str(&context.out, 3);

        convert_glob_bash_wildcard(&context, ccontext->glob_separator, TRUE);
        convert_glob_bash_wildcard(&context, ccontext->glob_separator, FALSE);

        context.out.out_str[0] = CHAR_ASTERISK;
        context.out.out_str[1] = CHAR_QUESTION_MARK;
        context.out.out_str[2] = CHAR_RIGHT_PARENTHESIS;
        context.out.out_str[3] = CHAR_QUESTION_MARK;
        context.out.out_str[4] = CHAR_QUESTION_MARK;
        convert_glob_bash_write_str(&context.out, 5);
        }
      else
        {
        convert_glob_bash_wildcard(&context, ccontext->glob_separator, FALSE);
        context.out.out_str[0] = CHAR_ASTERISK;
        context.out.out_str[1] = CHAR_QUESTION_MARK;
        convert_glob_bash_write_str(&context.out, 2);
        }

      after_sep = FALSE;
      continue;
      }

    if (c == CHAR_QUESTION_MARK)
      {
      convert_glob_bash_wildcard(&context,
        ccontext->glob_separator, after_sep);

      after_sep = FALSE;
      continue;
      }
    }

  after_sep = (c == ccontext->glob_separator);

  if (after_sep && nesting_level > 0)
    {
    context.out.out_str[0] = CHAR_LEFT_PARENTHESIS;
    context.out.out_str[1] = CHAR_ASTERISK;
    context.out.out_str[2] = CHAR_F;
    context.out.out_str[3] = CHAR_RIGHT_PARENTHESIS;
    convert_glob_bash_write_str(&context.out, 4);

    after_sep = FALSE;
    continue;
    }

  if (c < 128 && strchr(pcre2_escaped_literals, c) != NULL)
    convert_glob_bash_write(&context.out, CHAR_BACKSLASH);

  convert_glob_bash_write(&context.out, c);
  }

if (result == 0)
  {
  /* ! Unexpected end of input ! */
  if (nesting_level > 0 || context.read_mode != PCRE2_BASH_GLOB_NORMAL)
    result = ERROR_MISSING_CLOSING_PARENTHESIS;
  else
    {
    context.out.out_str[0] = CHAR_BACKSLASH;
    context.out.out_str[1] = CHAR_z;
    context.out.out_str[2] = CHAR_NULL;
    convert_glob_bash_write_str(&context.out, 3);
    }
  }

if (result != 0)
  {
  *bufflenptr = context.out.output - use_buffer;
  return result;
  }

*bufflenptr = context.out.output_size - 1;
return 0;
}


/*************************************************
*                Convert pattern                 *
*************************************************/

/* This is the external-facing function for converting other forms of pattern
into PCRE2 regular expression patterns. On error, the bufflenptr argument is
used to return an offset in the original pattern.

Arguments:
  pattern     the input pattern
  plength     length of input, or PCRE2_ZERO_TERMINATED
  options     options bits
  buffptr     pointer to pointer to output buffer
  bufflenptr  pointer to length of output buffer
  ccontext    convert context or NULL

Returns:      0 for success, else an error code (+ve or -ve)
*/

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_pattern_convert(PCRE2_SPTR pattern, PCRE2_SIZE plength, uint32_t options,
  PCRE2_UCHAR **buffptr, PCRE2_SIZE *bufflenptr,
  pcre2_convert_context *ccontext)
{
int i, rc;
PCRE2_UCHAR dummy_buffer[DUMMY_BUFFER_SIZE];
PCRE2_UCHAR *use_buffer = dummy_buffer;
PCRE2_SIZE use_length = DUMMY_BUFFER_SIZE;
BOOL utf = (options & PCRE2_CONVERT_UTF) != 0;
uint32_t pattype = options & TYPE_OPTIONS;

if (pattern == NULL || bufflenptr == NULL) return PCRE2_ERROR_NULL;
if ((options & ~ALL_OPTIONS) != 0 ||        /* Undefined bit set */
    (pattype & (~pattype+1)) != pattype ||  /* More than one type set */
    pattype == 0)                           /* No type set */
  {
  *bufflenptr = 0;  /* Error offset */
  return PCRE2_ERROR_BADOPTION;
  }

if (plength == PCRE2_ZERO_TERMINATED) plength = PRIV(strlen)(pattern);
if (ccontext == NULL) ccontext =
  (pcre2_convert_context *)(&PRIV(default_convert_context));

/* Check UTF if required. */

#ifndef SUPPORT_UNICODE
if (utf) return ERROR_NO_UNICODE;
#else
if (utf && (options & PCRE2_CONVERT_NO_UTF_CHECK) == 0)
  {
  PCRE2_SIZE erroroffset;
  rc = PRIV(valid_utf)(pattern, plength, &erroroffset);
  if (rc != 0)
    {
    *bufflenptr = erroroffset;
    return rc;
    }
  }
#endif

/* If buffptr is not NULL, and what it points to is not NULL, we are being
provided with a buffer and a length, so set them as the buffer to use. */

if (buffptr != NULL && *buffptr != NULL)
  {
  use_buffer = *buffptr;
  use_length = *bufflenptr;
  }

/* Call an individual converter, either just once (if a buffer was provided or
just the length is needed), or twice (if a memory allocation is required). */

for (i = 0; i < 2; i++)
  {
  PCRE2_UCHAR *allocated;
  BOOL dummyrun = buffptr == NULL || *buffptr == NULL;

  switch(pattype)
    {
    case PCRE2_CONVERT_GLOB_BASIC:
    rc = convert_glob(pattype, pattern, plength, utf, use_buffer, use_length,
      bufflenptr, dummyrun, ccontext);
    break;

    case PCRE2_CONVERT_GLOB_BASH:
    rc = convert_glob_bash(pattype, pattern, plength, utf, use_buffer, use_length,
      bufflenptr, dummyrun, ccontext);
    break;

    case PCRE2_CONVERT_POSIX_BASIC:
    case PCRE2_CONVERT_POSIX_EXTENDED:
    rc = convert_posix(pattype, pattern, plength, utf, use_buffer, use_length,
      bufflenptr, dummyrun, ccontext);
    break;

    default:
    return PCRE2_ERROR_INTERNAL;
    }

  if (rc != 0 ||           /* Error */
      buffptr == NULL ||   /* Just the length is required */
      *buffptr != NULL)    /* Buffer was provided or allocated */
    return rc;

  /* Allocate memory for the buffer, with hidden space for an allocator at
  the start. The next time round the loop runs the conversion for real. */

  allocated = PRIV(memctl_malloc)(sizeof(pcre2_memctl) +
    (*bufflenptr + 1)*PCRE2_CODE_UNIT_WIDTH, (pcre2_memctl *)ccontext);
  if (allocated == NULL) return PCRE2_ERROR_NOMEMORY;
  *buffptr = (PCRE2_UCHAR *)(((char *)allocated) + sizeof(pcre2_memctl));

  use_buffer = *buffptr;
  use_length = *bufflenptr + 1;
  }

/* Control should never get here. */

return PCRE2_ERROR_INTERNAL;
}


/*************************************************
*            Free converted pattern              *
*************************************************/

/* This frees a converted pattern that was put in newly-allocated memory.

Argument:   the converted pattern
Returns:    nothing
*/

PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_converted_pattern_free(PCRE2_UCHAR *converted)
{
if (converted != NULL)
  {
  pcre2_memctl *memctl =
    (pcre2_memctl *)((char *)converted - sizeof(pcre2_memctl));
  memctl->free(memctl, memctl->memory_data);
  }
}

/* End of pcre2_convert.c */
