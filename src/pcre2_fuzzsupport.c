/***************************************************************************
Fuzzer driver for PCRE2. Given an arbitrary string of bytes and a length, it
tries to compile and match it, deriving options from the string itself. If
STANDALONE is defined, a main program that calls the driver with the contents
of specified files is compiled, and commentary on what is happening is output.
If an argument starts with '=' the rest of it it is taken as a literal string
rather than a file name. This allows easy testing of short strings.

Written by Philip Hazel, October 2016
***************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include "pcre2.h"

#define ALLOWED_COMPILE_OPTIONS \
  (PCRE2_ANCHORED|PCRE2_ALLOW_EMPTY_CLASS|PCRE2_ALT_BSUX|PCRE2_ALT_CIRCUMFLEX| \
   PCRE2_ALT_VERBNAMES|PCRE2_AUTO_CALLOUT|PCRE2_CASELESS|PCRE2_DOLLAR_ENDONLY| \
   PCRE2_DOTALL|PCRE2_DUPNAMES|PCRE2_EXTENDED|PCRE2_FIRSTLINE| \
   PCRE2_MATCH_UNSET_BACKREF|PCRE2_MULTILINE|PCRE2_NEVER_BACKSLASH_C| \
   PCRE2_NO_AUTO_CAPTURE| \
   PCRE2_NO_AUTO_POSSESS|PCRE2_NO_DOTSTAR_ANCHOR|PCRE2_NO_START_OPTIMIZE| \
   PCRE2_UCP|PCRE2_UNGREEDY|PCRE2_USE_OFFSET_LIMIT| \
   PCRE2_UTF)

#define ALLOWED_MATCH_OPTIONS \
  (PCRE2_ANCHORED|PCRE2_NOTBOL|PCRE2_NOTEOL|PCRE2_NOTEMPTY| \
   PCRE2_NOTEMPTY_ATSTART|PCRE2_PARTIAL_HARD| \
   PCRE2_PARTIAL_SOFT|PCRE2_NO_JIT)
   
/* Putting in this apparently unnecessary prototype prevents gcc from giving a 
"no previous prototype" warning when compiling at high warning level. */ 

int LLVMFuzzerTestOneInput(const unsigned char *, size_t);

/* Here's the driving function. */

int LLVMFuzzerTestOneInput(const unsigned char *data, size_t size)
{
uint32_t compile_options;
uint32_t match_options;
pcre2_match_data *match_data = NULL;
int r1, r2;
int i;

if (size < 1) return 0;

/* Figure out some options to use. Initialize the random number to ensure
repeatability. Ensure that we get a 32-bit unsigned random number for testing
options. (RAND_MAX is required to be at least 32767, but is commonly
2147483647, which excludes the top bit.) */

srand((unsigned int)(data[size/2]));
r1 = rand();
r2 = rand();

/* Ensure that all undefined option bits are zero (waste of time trying them)
and also that PCRE2_NO_UTF_CHECK is unset, as there is no guarantee that the
input is UTF-8. Also unset PCRE2_NEVER_UTF and PCRE2_NEVER_UCP as there is no
reason to disallow UTF and UCP. Force PCRE2_NEVER_BACKSLASH_C to be set because
\C in random patterns is highly likely to cause a crash.  */

compile_options =
  ((((uint32_t)r1 << 16) | ((uint32_t)r2 & 0xffff)) & ALLOWED_COMPILE_OPTIONS) |
  PCRE2_NEVER_BACKSLASH_C;

match_options =
  ((((uint32_t)r1 << 16) | ((uint32_t)r2 & 0xffff)) & ALLOWED_MATCH_OPTIONS);

/* Do the compile with and without the options, and after a successful compile,
likewise do the match with and without the options. */

for (i = 0; i < 2; i++)
  {
  int errorcode;
  PCRE2_SIZE erroroffset;
  pcre2_code *code;

#ifdef STANDALONE
  printf("Compile with options %.8x", compile_options);
  printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
    ((compile_options & PCRE2_ALT_BSUX) != 0)? ",alt_bsux" : "",
    ((compile_options & PCRE2_ALT_CIRCUMFLEX) != 0)? ",alt_circumflex" : "",
    ((compile_options & PCRE2_ALT_VERBNAMES) != 0)? ",alt_verbnames" : "",
    ((compile_options & PCRE2_ALLOW_EMPTY_CLASS) != 0)? ",allow_empty_class" : "",
    ((compile_options & PCRE2_ANCHORED) != 0)? ",anchored" : "",
    ((compile_options & PCRE2_AUTO_CALLOUT) != 0)? ",auto_callout" : "",
    ((compile_options & PCRE2_CASELESS) != 0)? ",caseless" : "",
    ((compile_options & PCRE2_DOLLAR_ENDONLY) != 0)? ",dollar_endonly" : "",
    ((compile_options & PCRE2_DOTALL) != 0)? ",dotall" : "",
    ((compile_options & PCRE2_DUPNAMES) != 0)? ",dupnames" : "",
    ((compile_options & PCRE2_EXTENDED) != 0)? ",extended" : "",
    ((compile_options & PCRE2_FIRSTLINE) != 0)? ",firstline" : "",
    ((compile_options & PCRE2_MATCH_UNSET_BACKREF) != 0)? ",match_unset_backref" : "",
    ((compile_options & PCRE2_MULTILINE) != 0)? ",multiline" : "",
    ((compile_options & PCRE2_NEVER_BACKSLASH_C) != 0)? ",never_backslash_c" : "",
    ((compile_options & PCRE2_NEVER_UCP) != 0)? ",never_ucp" : "",
    ((compile_options & PCRE2_NEVER_UTF) != 0)? ",never_utf" : "",
    ((compile_options & PCRE2_NO_AUTO_CAPTURE) != 0)? ",no_auto_capture" : "",
    ((compile_options & PCRE2_NO_AUTO_POSSESS) != 0)? ",no_auto_possess" : "",
    ((compile_options & PCRE2_NO_DOTSTAR_ANCHOR) != 0)? ",no_dotstar_anchor" : "",
    ((compile_options & PCRE2_NO_UTF_CHECK) != 0)? ",no_utf_check" : "",
    ((compile_options & PCRE2_NO_START_OPTIMIZE) != 0)? ",no_start_optimize" : "",
    ((compile_options & PCRE2_UCP) != 0)? ",ucp" : "",
    ((compile_options & PCRE2_UNGREEDY) != 0)? ",ungreedy" : "",
    ((compile_options & PCRE2_USE_OFFSET_LIMIT) != 0)? ",use_offset_limit" : "",
    ((compile_options & PCRE2_UTF) != 0)? ",utf" : "");
#endif

  code = pcre2_compile((PCRE2_SPTR)data, (PCRE2_SIZE)size, compile_options,
    &errorcode, &erroroffset, NULL);

  if (code != NULL)
    {
    int j;
    uint32_t save_match_options = match_options;

    /* Create a match data block only when we first need it. */

    if (match_data == NULL)
      {
      match_data = pcre2_match_data_create(32, NULL);
      if (match_data == NULL)
        {
#ifdef STANDALONE
        printf("** Failed to create match data block\n");
#endif
        return 0;
        }
      }

    /* Match twice, with and without options */

    for (j = 0; j < 2; j++)
      {
#ifdef STANDALONE
      printf("Match with options %.8x", match_options);
      printf("%s%s%s%s%s%s%s%s\n",
        ((match_options & PCRE2_ANCHORED) != 0)? ",anchored" : "",
        ((match_options & PCRE2_NO_UTF_CHECK) != 0)? ",no_utf_check" : "",
        ((match_options & PCRE2_NOTBOL) != 0)? ",notbol" : "",
        ((match_options & PCRE2_NOTEMPTY) != 0)? ",notempty" : "",
        ((match_options & PCRE2_NOTEMPTY_ATSTART) != 0)? ",notempty_atstart" : "",
        ((match_options & PCRE2_NOTEOL) != 0)? ",noteol" : "",
        ((match_options & PCRE2_PARTIAL_HARD) != 0)? ",partial_hard" : "",
        ((match_options & PCRE2_PARTIAL_SOFT) != 0)? ",partial_soft" : "");
#endif

      errorcode = pcre2_match(code, (PCRE2_SPTR)data, (PCRE2_SIZE)size, 0,
        match_options, match_data, NULL);

#ifdef STANDALONE
      if (errorcode >= 0) printf("Match returned %d\n", errorcode); else
        {
        unsigned char buffer[256];
        pcre2_get_error_message(errorcode, buffer, 256);
        printf("Match failed: error %d: %s\n", errorcode, buffer);
        }
#endif

      match_options = 0;  /* For second time */
      }

    match_options = save_match_options;  /* Reset for the second compile */
    pcre2_code_free(code);
    }

#ifdef STANDALONE
  else
    {
    unsigned char buffer[256];
    pcre2_get_error_message(errorcode, buffer, 256);
    printf("Compile failed: error %d at offset %lu: %s\n", errorcode,
      erroroffset, buffer);
    }
#endif

  compile_options = PCRE2_NEVER_BACKSLASH_C;  /* For second time */
  }

if (match_data != NULL) pcre2_match_data_free(match_data);
return 0;
}


/* Optional main program.  */

#ifdef STANDALONE
int main(int argc, char **argv)
{
int i;

if (argc < 2)
  {
  printf("** No arguments given\n");
  return 0;
  }

for (i = 1; i < argc; i++)
  {
  size_t filelen;
  size_t readsize;
  unsigned char *buffer;
  FILE *f;
  
  /* Handle a literal string. Copy to an exact size buffer so that checks for 
  overrunning work. */
  
  if (argv[i][0] == '=')
    {    
    readsize = strlen(argv[i]) - 1;
    printf("------ <Literal> ------\n"); 
    printf("Length = %lu\n", readsize);
    printf("%.*s\n", (int)readsize, argv[i]+1); 
    buffer = (unsigned char *)malloc(readsize);
    if (buffer == NULL)
      printf("** Failed to allocate %lu bytes of memory\n", readsize);
    else
      {    
      memcpy(buffer, argv[i]+1, readsize); 
      LLVMFuzzerTestOneInput(buffer, readsize);
      free(buffer);    
      } 
    continue;
    }  

  /* Handle a string given in a file */
 
  f = fopen(argv[i], "rb");
  if (f == NULL)
    {
    printf("** Failed to open %s: %s\n", argv[i], strerror(errno));
    continue;
    }

  printf("------ %s ------\n", argv[i]);

  fseek(f, 0, SEEK_END);
  filelen = ftell(f);
  fseek(f, 0, SEEK_SET);

  buffer = (unsigned char *)malloc(filelen);
  if (buffer == NULL)
    {
    printf("** Failed to allocate %lu bytes of memory\n", filelen);
    fclose(f);
    continue;
    }

  readsize = fread(buffer, 1, filelen, f);
  fclose(f);

  if (readsize != filelen)
    printf("** File size is %lu but fread() returned %lu\n", filelen, readsize);
  else
    {
    printf("Length = %lu\n", filelen);
    LLVMFuzzerTestOneInput(buffer, filelen);
    }
  free(buffer);
  }

return 0;
}
#endif  /* STANDALONE */

/* End */
