/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
          New API code Copyright (c) 2016-2023 University of Cambridge

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


/* This program tests the POSIX wrapper to the PCRE2 regular expression library.
The main PCRE2 test program is pcre2test, which also tests these function
calls. This little program is needed to test the case where the client includes
pcre2posix.h but not pcre2.h, mainly to make sure that it builds successfully.
However, the code is written as a flexible test program to which extra tests
can be added.

Compile with -lpcre2-posix -lpcre2-8

If run with no options, there is no output on success, and the return code is
zero. If any test fails there is output to stderr, and the return code is 1.

For testing purposes, the "-v" option causes verification output to be written
to stdout. */


#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pcre2posix.h>

#define CAPCOUNT    5 /* Number of captures supported */
#define PRINTF(...)   /* Shorthand for testing output */ \
  do                                                     \
  {                                                      \
    if (v)                                               \
      printf(__VA_ARGS__);                               \
  } while (0)

/* This vector contains compiler flags for each pattern that is tested. */

static int cflags[] = {
  0,           // Test 0
  REG_ICASE,   // Test 1
  0,           // Test 2
  REG_NEWLINE, // Test 3
  0,           // Test 4
};

/* This vector contains match flags for each pattern that is tested. */

static int mflags[] = {
  0,          // Test 0
  0,          // Test 1
  0,          // Test 2
  REG_NOTBOL, // Test 3
  0,          // Test 4
};

/* Automate the number of patterns */

#define count (int)(sizeof(cflags) / sizeof(int))

/* The data for each pattern consists of a pattern string, followed by any
number of subject strings, terminated by NULL. Some tests share data, but use
different flags. */

static const char *data0_1[] = { "posix", "lower posix", "upper POSIX", NULL };
static const char *data2_3[] = { "(*LF)^(cat|dog)", "catastrophic\ncataclysm", "dogfight",
                                 "no animals", NULL };
static const char *data4[] = { "*badpattern", NULL };

/* Index the data strings */

static char **data[] = {
  // clang-format off
  (char **)(&data0_1),
  (char **)(&data0_1),
  (char **)(&data2_3),
  (char **)(&data2_3),
  (char **)(&data4),
  // clang-format on
};

/* The expected results for each pattern consist of a compiler return code,
optionally followed, for each subject string, by a match return code and, for a
successful match, up to CAPCOUNT pairs of returned match data. */

static int results0[] = {
  // clang-format off
  0,             // Compiler rc
  0, 6, 11,      // 1st match
  REG_NOMATCH,   // 2nd match
  // clang-format on
};

static int results1[] = {
  // clang-format off
  0,             // Compiler rc
  0, 6, 11,      // 1st match
  0, 6, 11,      // 2nd match
  // clang-format on
};

static int results2[] = {
  // clang-format off
  0,             // Compiler rc
  0, 0, 3, 0, 3, // 1st match
  0, 0, 3, 0, 3, // 2nd match
  REG_NOMATCH,   // 3rd match
  // clang-format on
};

static int results3[] = {
  // clang-format off
  0,                 // Compiler rc
  0, 13, 16, 13, 16, // 1st match
  REG_NOMATCH,       // 2nd match
  REG_NOMATCH,       // 3rd match
  // clang-format on
};

static int results4[] = {
  // clang-format off
  REG_BADRPT,        // Compiler rc
  // clang-format on
};

/* Index the result vectors */

static int *results[] = {
  // clang-format off
  (int *)(&results0),
  (int *)(&results1),
  (int *)(&results2),
  (int *)(&results3),
  (int *)(&results4),
  // clang-format on
};

/* And here is the program */

int
main(int argc, char **argv)
{
  regex_t re;
  regmatch_t match[CAPCOUNT];
  int v = argc > 1 && strcmp(argv[1], "-v") == 0;

  PRINTF("Test of pcre2posix.h without pcre2.h\n");

  for (int i = 0; i < count; i++)
  {
    char *pattern = data[i][0];
    char **subjects = data[i] + 1;
    int *rd = results[i];
    int rc = regcomp(&re, pattern, cflags[i]);

    PRINTF("Pattern: %s flags=0x%02x\n", pattern, cflags[i]);

    if (rc != *rd)
    {
      fprintf(stderr, "Unexpected compile error %d (expected %d)\n", rc, *rd);
      fprintf(stderr, "Pattern is: %s\n", pattern);
      return 1;
    }

    if (rc != 0)
    {
      char buffer[256];
      (void)regerror(rc, &re, buffer, sizeof(buffer));
      PRINTF("Compile error %d: %s (expected)\n", rc, buffer);
      continue;
    }

    for (; *subjects != NULL; subjects++)
    {
      rc = regexec(&re, *subjects, CAPCOUNT, match, mflags[i]);

      PRINTF("Subject: %s\n", *subjects);
      PRINTF("Return:  %d", rc);

      if (rc != *(++rd))
      {
        PRINTF("\n");
        fprintf(stderr, "Unexpected match error %d (expected %d)\n", rc, *rd);
        fprintf(stderr, "Pattern is: %s\n", pattern);
        fprintf(stderr, "Subject is: %s\n", *subjects);
        return 1;
      }

      if (rc == 0)
      {
        for (int j = 0; j < CAPCOUNT; j++)
        {
          regmatch_t *m = match + j;
          if (m->rm_so < 0)
            continue;
          if (m->rm_so != *(++rd) || m->rm_eo != *(++rd))
          {
            PRINTF("\n");
            fprintf(stderr, "Mismatched results for successful match\n");
            fprintf(stderr, "Pattern is: %s\n", pattern);
            fprintf(stderr, "Subject is: %s\n", *subjects);
            fprintf(stderr, "Result %d: expected %d %d received %d %d\n", j, rd[-1], rd[0],
                    m->rm_so, m->rm_eo);
            return 1;
          }

          PRINTF(" (%d %d %d)", j, m->rm_so, m->rm_eo);
        }
      }

      else
      {
        char buffer[256];
        (void)regerror(rc, &re, buffer, sizeof(buffer));
        PRINTF(": %s (expected)", buffer);
      }

      PRINTF("\n");
    }

    regfree(&re);
  }

  /* Validation and edge case tests */
  PRINTF("Testing NULL and boundary validation\n");
  regfree(NULL);

  if (regcomp(NULL, "pattern", 0) != REG_INVARG)
  {
    fprintf(stderr, "regcomp(NULL, ...) did not return REG_INVARG\n");
    return 1;
  }

  if (regcomp(&re, NULL, 0) != REG_INVARG)
  {
    fprintf(stderr, "regcomp(..., NULL, ...) did not return REG_INVARG\n");
    return 1;
  }

  re.re_endp = NULL;
  if (regcomp(&re, "pattern", REG_PEND) != REG_INVARG)
  {
    fprintf(stderr, "regcomp with NULL re_endp did not return REG_INVARG\n");
    return 1;
  }

  {
    const int invalid_flags[] = { REG_NOTBOL, REG_NOTEOL, REG_STARTEND, REG_NOTEMPTY, 0x2000,
                                  INT_MIN,    -1 };
    for (size_t i = 0; i < sizeof(invalid_flags) / sizeof(invalid_flags[0]); i++)
    {
      char errbuf[64];
      if (regcomp(&re, "pattern", invalid_flags[i]) != REG_INVARG)
      {
        fprintf(stderr, "regcomp accepted invalid flags %d\n", invalid_flags[i]);
        regfree(&re);
        return 1;
      }
      if (re.re_pcre2_code != NULL || re.re_match_data != NULL || re.re_nsub != 0 ||
          re.re_cflags != 0 || re.re_erroffset != SIZE_MAX ||
          regerror(REG_INVARG, &re, errbuf, sizeof(errbuf)) != sizeof("bad argument") ||
          strcmp(errbuf, "bad argument") != 0)
      {
        fprintf(stderr, "regcomp left invalid state after rejecting flags\n");
        return 1;
      }
    }
  }

  if (regexec(NULL, "subject", 0, NULL, 0) != REG_INVARG)
  {
    fprintf(stderr, "regexec(NULL, ...) did not return REG_INVARG\n");
    return 1;
  }

  if (regcomp(&re, "test", 0) != 0)
  {
    fprintf(stderr, "regcomp failed for validation test pattern\n");
    return 1;
  }
  else
  {
    const int invalid_flags[] = { REG_ICASE,  REG_NEWLINE,  REG_DOTALL, REG_NOSUB,
                                  REG_UTF,    REG_UNGREEDY, REG_UCP,    REG_PEND,
                                  REG_NOSPEC, 0x2000,       INT_MIN,    -1 };
    for (size_t i = 0; i < sizeof(invalid_flags) / sizeof(invalid_flags[0]); i++)
    {
      match[0].rm_so = match[0].rm_eo = -2;
      if (regexec(&re, "test", CAPCOUNT, match, invalid_flags[i]) != REG_INVARG ||
          match[0].rm_so != -2 || match[0].rm_eo != -2)
      {
        fprintf(stderr, "regexec mishandled invalid flags %d\n", invalid_flags[i]);
        regfree(&re);
        return 1;
      }
    }

    if (regexec(&re, NULL, 0, NULL, 0) != REG_INVARG)
    {
      fprintf(stderr, "regexec(..., NULL, ...) did not return REG_INVARG\n");
      regfree(&re);
      return 1;
    }

    match[0].rm_so = -1;
    match[0].rm_eo = 2;
    if (regexec(&re, "test", 1, match, REG_STARTEND) != REG_INVARG)
    {
      fprintf(stderr,
              "regexec with negative rm_so did not return REG_INVARG\n");
      regfree(&re);
      return 1;
    }

    match[0].rm_so = 3;
    match[0].rm_eo = 2;
    if (regexec(&re, "test", 1, match, REG_STARTEND) != REG_INVARG)
    {
      fprintf(stderr,
              "regexec with rm_eo < rm_so did not return REG_INVARG\n");
      regfree(&re);
      return 1;
    }

    regfree(&re);
    regfree(&re); /* Test idempotency */
  }

  {
    char errbuf[64];
    if (regerror(REG_INVARG, NULL, errbuf, sizeof(errbuf)) == 0)
    {
      fprintf(stderr, "regerror returned 0\n");
      return 1;
    }
  }

  {
    const size_t offsets[] = { 0, INT_MAX, (size_t)INT_MAX + 1, SIZE_MAX - 1, SIZE_MAX };
    for (size_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++)
    {
      char expected[sizeof("bad argument at offset ") + sizeof(size_t) * CHAR_BIT];
      size_t needed;
      re.re_erroffset = offsets[i];
      if (offsets[i] == SIZE_MAX)
        strcpy(expected, "bad argument");
      else
        sprintf(expected, "bad argument at offset %llu", (unsigned long long)offsets[i]);
      needed = strlen(expected) + 1;

      if (regerror(REG_INVARG, &re, NULL, 0) != needed)
      {
        fprintf(stderr, "regerror returned an incorrect required size\n");
        return 1;
      }
      for (size_t size = 0; size <= needed; size++)
      {
        char errbuf[sizeof(expected)];
        size_t written = size == 0 ? 0 : size - 1;
        if (regerror(REG_INVARG, &re, errbuf, size) != needed ||
            (size > 0 && (memcmp(errbuf, expected, written) != 0 || errbuf[written] != 0)))
        {
          fprintf(stderr, "regerror mishandled an offset or truncated buffer\n");
          return 1;
        }
      }
    }
  }

  {
    char *pattern = malloc(2 * 65534 + sizeof("(?(DEFINE))(a)"));
    char *p;
    int rc;
    if (pattern == NULL)
    {
      fprintf(stderr, "Failed to allocate capture-limit test pattern\n");
      return 1;
    }
    strcpy(pattern, "(?(DEFINE)");
    p = pattern + strlen(pattern);
    for (size_t i = 0; i < 65534; i++)
    {
      *p++ = '(';
      *p++ = ')';
    }
    strcpy(p, ")(a)");
    rc = regcomp(&re, pattern, 0);
    free(pattern);
    if (rc == REG_ESIZE)
      PRINTF("Skipping capture-limit test: compiled pattern size limit\n");
    else if (rc != 0)
    {
      fprintf(stderr, "Capture-limit test pattern failed to compile: %d\n", rc);
      return 1;
    }
    else
    {
      PRINTF("Testing truncated native capture vector\n");
      if (re.re_nsub != 65535 || regexec(&re, "a", CAPCOUNT, match, 0) != 0 ||
          match[0].rm_so != 0 || match[0].rm_eo != 1)
      {
        fprintf(stderr, "Truncated capture vector lost the overall match\n");
        regfree(&re);
        return 1;
      }
      for (size_t i = 1; i < CAPCOUNT; i++)
      {
        if (match[i].rm_so != -1 || match[i].rm_eo != -1)
        {
          fprintf(stderr, "Truncated capture vector returned an unmatched capture\n");
          regfree(&re);
          return 1;
        }
      }
      match[0].rm_so = 1;
      match[0].rm_eo = 2;
      if (regexec(&re, "xa", 1, match, REG_STARTEND) != 0 || match[0].rm_so != 1 ||
          match[0].rm_eo != 2 || regexec(&re, "a", 0, match, 0) != 0 || match[0].rm_so != 1 ||
          match[0].rm_eo != 2 || regexec(&re, "a", 0, NULL, 0) != 0)
      {
        fprintf(stderr, "Truncated capture vector mishandled match bounds\n");
        regfree(&re);
        return 1;
      }
      regfree(&re);
    }
  }

  PRINTF("End of test\n");
  return 0;
}

/* End of pcre2posix_test.c */
