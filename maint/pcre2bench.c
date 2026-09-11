/*************************************************
*      PCRE2 match throughput benchmark driver   *
*************************************************/

/* This program measures whole-buffer scan throughput for a set of patterns
chosen to exercise the JIT's start-of-match fast-forward paths (single
character, character pair, character range, caseless) as well as control cases
that bypass fast-forward entirely.  It exists to compare two builds of the
library, for example before and after a change to the SIMD fast-forward code
for one architecture.

The subject is a deterministic pseudo-text corpus, generated identically on
every host and build, so runs from different builds can be compared directly.
Output is TSV on stdout.  The match count is reported for each pattern and must
be identical across every build under test.

Usage: pcre2bench [label] [nojit] [short]

  label   a string used in the first output column; defaults to "build"
  nojit   run the interpreter with PCRE2_NO_JIT instead of the JIT
  short   run the short-subject sweep instead of the throughput benchmarks

The code unit width is chosen at compile time by defining
PCRE2_CODE_UNIT_WIDTH to 8, 16 or 32; it defaults to 8.  All the text here is
ASCII, and the corpus is a fixed number of code units rather than of bytes, so
the same corpus is generated at every width and the match counts are directly
comparable between them.  Throughput is therefore reported in code units, not
in bytes, and the width appears as an output column.

This program is not built by any of the PCRE2 build systems.  Compile it
against an already-built tree with

  cc -O2 -DPCRE2_CODE_UNIT_WIDTH=8 -o pcre2bench maint/pcre2bench.c \
    -I<build>/src -L<build>/.libs -lpcre2-8

or use maint/RunBenchmark, which works the include and link flags out for a
CMake or an autotools build tree and then runs the result. */

#ifndef PCRE2_CODE_UNIT_WIDTH
#define PCRE2_CODE_UNIT_WIDTH 8
#endif

#include <pcre2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef SUBJECT_UNITS
#define SUBJECT_UNITS (4u * 1024u * 1024u)
#endif

#define MIN_SECONDS 0.5
#define REPEATS 5

/* Longest pattern or error message handled by widen() and narrow(). */

#define TEXT_MAX 256

static double
now_seconds(void)
{
struct timespec ts;
clock_gettime(CLOCK_MONOTONIC, &ts);
return (double)ts.tv_sec + 1e-9 * (double)ts.tv_nsec;
}


/* The patterns this program compiles, and the error messages it prints, are
ASCII held in ordinary C strings.  At widths other than 8 they have to be
widened into code units on the way into the library and narrowed on the way
out.  Both buffers are supplied by the caller and hold TEXT_MAX units; longer
text is truncated, which only ever costs a little of an error message. */

static PCRE2_SPTR
widen(const char *s, PCRE2_UCHAR *buffer)
{
size_t i;
for (i = 0; i < TEXT_MAX - 1 && s[i] != 0; i++)
  buffer[i] = (PCRE2_UCHAR)(unsigned char)s[i];
buffer[i] = 0;
return buffer;
}

static const char *
narrow(const PCRE2_UCHAR *s, char *buffer)
{
size_t i;
for (i = 0; i < TEXT_MAX - 1 && s[i] != 0; i++)
  buffer[i] = (s[i] < 128) ? (char)s[i] : '?';
buffer[i] = 0;
return buffer;
}



/*************************************************
*                Corpus generation               *
*************************************************/

/* xorshift64* so the corpus is bit-identical on every host and build. */

static unsigned long long rng_state = 0x9e3779b97f4a7c15ULL;

static unsigned long long
rng_next(void)
{
rng_state ^= rng_state >> 12;
rng_state ^= rng_state << 25;
rng_state ^= rng_state >> 27;
return rng_state * 0x2545f4914f6cdd1dULL;
}

/* A word list with a plausible letter distribution.  No word contains 'q',
'x', 'z' or an uppercase letter, so patterns built from those characters scan
the entire buffer without ever matching. */

static const char *const words[] = {
  "the", "of", "and", "to", "in", "a", "is", "that", "for", "it",
  "as", "with", "was", "on", "be", "at", "by", "this", "had", "not",
  "are", "but", "from", "or", "have", "an", "they", "which", "one", "you",
  "were", "her", "all", "she", "there", "would", "their", "we", "him", "been",
  "has", "when", "who", "will", "more", "no", "if", "out", "so", "said",
  "what", "up", "its", "about", "into", "than", "them", "can", "only", "other",
  "new", "some", "could", "time", "these", "two", "may", "then", "do", "first",
  "any", "my", "now", "such", "like", "our", "over", "man", "me", "even",
  "most", "made", "after", "also", "did", "many", "before", "must", "through", "back",
  "years", "where", "much", "your", "way", "well", "down", "should", "because", "each",
  "just", "those", "people", "mr", "how", "too", "little", "state", "good", "very",
  "make", "world", "still", "own", "see", "men", "work", "long", "get", "here",
  "between", "both", "life", "being", "under", "never", "day", "same", "another", "know",
  "while", "last", "might", "us", "great", "old", "year", "off", "come", "since",
  "against", "go", "came", "right", "used", "take", "three", "hello", "consideration", "development"
};

#define WORD_COUNT ((int)(sizeof(words) / sizeof(words[0])))

/* Rare needles seeded into the corpus so that "found" cases are exercised too.
Each is injected roughly once every RARE_PERIOD words. */

#define RARE_PERIOD 4096

static const char *const rare_needles[] = {
  "wombat", "user.name@example.com", "1234567890"
};

#define RARE_COUNT ((int)(sizeof(rare_needles) / sizeof(rare_needles[0])))

/* Capitalize one word in CAPITAL_PERIOD.  This must be a power of two that
divides RARE_PERIOD, because needles are exempt: see below. */

#define CAPITAL_PERIOD 32

static PCRE2_UCHAR *
build_subject(size_t length)
{
PCRE2_UCHAR *buffer = malloc((length + 1) * sizeof(PCRE2_UCHAR));
size_t offset = 0;
unsigned long long words_emitted = 0;

if (buffer == NULL) return NULL;

while (offset < length)
  {
  const char *token;
  size_t token_length;
  size_t i;
  int is_needle;

  is_needle = (words_emitted != 0 && words_emitted % RARE_PERIOD == 0);

  if (is_needle)
    token = rare_needles[(words_emitted / RARE_PERIOD - 1) % RARE_COUNT];
  else
    token = words[rng_next() % (unsigned long long)WORD_COUNT];

  token_length = strlen(token);
  if (offset + token_length + 1 > length) break;

  for (i = 0; i < token_length; i++)
    buffer[offset + i] = (PCRE2_UCHAR)(unsigned char)token[i];

  /* Capitalize the first letter of one word in CAPITAL_PERIOD, as prose does
  at the start of a sentence.  An entirely lowercase corpus is a bad subject
  for the interpreter: pcre2_match() searches for a caseless first code unit
  with one memchr call per case, and if the uppercase variant never occurs its
  memchr runs to the end of the subject.  That result is cached only for the
  duration of the call, so a find-all-matches loop pays a full-buffer memchr
  per match and becomes quadratic.  Measured at width 8 on a 4M code unit
  all-lowercase subject with (?i)Hello, that is 1579us per call against 42us
  once capitals are present.  The JIT never uses that code path.

  Needles are exempt.  RARE_PERIOD is a multiple of CAPITAL_PERIOD, so every
  needle would otherwise be capitalized, which would leave "wombat" absent
  from a corpus that is meant to contain it, and would turn the first digit of
  "1234567890" into a control character. */

  if (!is_needle && words_emitted % CAPITAL_PERIOD == 0)
    buffer[offset] = (PCRE2_UCHAR)(buffer[offset] - 'a' + 'A');

  offset += token_length;

  /* Break the text into lines so that '^', '$' and '\n'-sensitive patterns
  see realistic structure. */

  buffer[offset++] = ((rng_next() & 15) == 0) ? '\n' : ' ';
  words_emitted++;
  }

while (offset < length) buffer[offset++] = ' ';
buffer[length] = '\0';
return buffer;
}



/*************************************************
*             Throughput benchmarks              *
*************************************************/

struct benchmark {
  const char *name;
  const char *pattern;
  const char *note;
};

static const struct benchmark benchmarks[] = {
  /* Pure fast-forward cost: the first code unit never occurs, so the JIT
  scans the whole buffer and reports NOMATCH without a single false start. */

  { "ff-char-absent",      "Q",                    "single absent char" },
  { "ff-lit-absent",       "zqjx",                 "absent literal, char pair start" },
  { "ff-lit-found",        "wombat",               "rare literal, present" },

  /* Character-range fast-forward.  Q, X and Z are the only letters that start
  none of the corpus words, so they stay absent once capitalization is
  applied. */

  { "ff-range-absent",     "[QXZ]",                "absent range" },
  { "ff-range-digits",     "[0-9]{6}",             "sparse digit run" },
  { "ff-class-dense",      "[aeiou]qzx",           "dense first char, all false starts" },

  /* Character-range fast-forward at a relative offset: the leading positions
  are too complex to search, but a sparse class follows them. */

  { "ff-off-range",        ".{3}[QXZ]",            "absent range at offset 3" },
  { "ff-off-digits",       ".{2}[0-9]{6}",         "sparse digit run at offset 2" },
  { "ff-off-dense",        ".{3}\\w{12}",          "dense class at offset 3" },
  { "ff-off-mixed",        "a.{4}[QXZ]",           "literal start, range at offset 5" },

  /* Caseless and pair-based fast-forward. */

  { "ff-caseless-absent",  "(?i)zqjx",             "caseless absent literal" },
  { "ff-caseless-found",   "(?i)Hello",            "caseless literal, present" },
  { "ff-pair-common",      "th",                   "very common char pair" },

  /* Realistic mixed patterns. */

  { "re-email",            "[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]{2,}", "email address" },
  { "re-longword",         "\\b\\w{12,}\\b",       "long word boundary scan" },
  { "re-alt",              "(?:wombat|numbat|quokka)", "alternation of literals" },

  /* Controls that do not use fast-forward at all.  These should be flat
  between the builds under test and act as a noise floor.  ctl-anchored
  matches at the first offset it is given, so each scan is a few hundred
  nanoseconds and its throughput figure is mostly call overhead; compare it
  between builds, but do not read anything into the absolute number. */

  { "ctl-anchored",        "\\A\\w+",              "anchored, no fast-forward" },
  { "ctl-startline",       "(?m)^the\\b",          "multiline start-of-line" }
};

#define BENCHMARK_COUNT ((int)(sizeof(benchmarks) / sizeof(benchmarks[0])))

/* Scan the whole subject, counting every non-overlapping match.  Returns the
match count, or -1 on error. */

static long
scan_all(pcre2_code *code, pcre2_match_data *match_data,
  PCRE2_SPTR subject, size_t length, int use_jit)
{
PCRE2_SIZE offset = 0;
long matches = 0;

for (;;)
  {
  int rc;
  PCRE2_SIZE *ovector;

  if (use_jit)
    rc = pcre2_jit_match(code, subject, length, offset, 0, match_data, NULL);
  else
    rc = pcre2_match(code, subject, length, offset, PCRE2_NO_JIT,
      match_data, NULL);

  if (rc == PCRE2_ERROR_NOMATCH) break;
  if (rc < 0) return -1;

  matches++;
  ovector = pcre2_get_ovector_pointer(match_data);
  offset = (ovector[1] > ovector[0]) ? ovector[1] : ovector[0] + 1;
  if (offset > length) break;
  }

return matches;
}


static int
run_throughput(const char *label, PCRE2_SPTR subject, size_t length,
  int use_jit)
{
int i;

printf("#build\twidth\tbench\tmatches\tmcu_per_s\tns_per_cu\tjit\tnote\n");

for (i = 0; i < BENCHMARK_COUNT; i++)
  {
  int errorcode;
  PCRE2_SIZE erroroffset;
  pcre2_code *code;
  pcre2_match_data *match_data;
  PCRE2_UCHAR pattern[TEXT_MAX];
  long matches;
  double best = 0.0;
  int rep;

  code = pcre2_compile(widen(benchmarks[i].pattern, pattern),
    PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroroffset, NULL);
  if (code == NULL)
    {
    PCRE2_UCHAR message[TEXT_MAX];
    char text[TEXT_MAX];
    pcre2_get_error_message(errorcode, message, TEXT_MAX);
    fprintf(stderr, "%s: compile failed at %d: %s\n", benchmarks[i].name,
      (int)erroroffset, narrow(message, text));
    return 1;
    }

  if (use_jit && pcre2_jit_compile(code, PCRE2_JIT_COMPLETE) != 0)
    {
    fprintf(stderr, "%s: JIT compile failed\n", benchmarks[i].name);
    return 1;
    }

  match_data = pcre2_match_data_create_from_pattern(code, NULL);

  /* Warm up, and establish the match count. */

  matches = scan_all(code, match_data, subject, length, use_jit);
  if (matches < 0)
    {
    fprintf(stderr, "%s: match error\n", benchmarks[i].name);
    return 1;
    }

  for (rep = 0; rep < REPEATS; rep++)
    {
    double start = now_seconds();
    double elapsed;
    long iterations = 0;
    double throughput;

    do
      {
      if (scan_all(code, match_data, subject, length, use_jit) != matches)
        {
        fprintf(stderr, "%s: unstable match count\n", benchmarks[i].name);
        return 1;
        }
      iterations++;
      elapsed = now_seconds() - start;
      }
    while (elapsed < MIN_SECONDS);

    throughput = ((double)length * (double)iterations) / elapsed /
      (1024.0 * 1024.0);
    if (throughput > best) best = throughput;
    }

  printf("%s\t%d\t%s\t%ld\t%.2f\t%.3f\t%d\t%s\n", label,
    PCRE2_CODE_UNIT_WIDTH, benchmarks[i].name, matches, best,
    1e9 / (best * 1024.0 * 1024.0), use_jit, benchmarks[i].note);
  fflush(stdout);

  pcre2_match_data_free(match_data);
  pcre2_code_free(code);
  }

return 0;
}



/*************************************************
*            Short-subject benchmarks            *
*************************************************/

/* The throughput benchmarks all scan one large buffer, so they say nothing
about the fixed cost paid before the first comparison: prologue, alignment,
loop setup.  For an application matching against lines or fields, that fixed
cost is most of the work.  Slice the same corpus into runs of a few dozen code
units and report the per-call cost. */

static const struct benchmark short_benchmarks[] = {
  { "s-char-absent",  "Q",           "single absent char" },
  { "s-lit-absent",   "zqjx",        "absent literal, char pair start" },
  { "s-caseless",     "(?i)zqjx",    "absent caseless literal" },
  { "s-lit-early",    "th",          "common pair, usually hits early" }
};

#define SHORT_BENCHMARK_COUNT \
  ((int)(sizeof(short_benchmarks) / sizeof(short_benchmarks[0])))

static const size_t short_lengths[] = { 8, 16, 32, 64, 128, 512 };

#define SHORT_LENGTH_COUNT \
  ((int)(sizeof(short_lengths) / sizeof(short_lengths[0])))

static int
run_short(const char *label, PCRE2_SPTR subject, size_t total, int use_jit)
{
int i, j;

printf("#build\twidth\tbench\tsubject_len\tmatches\tns_per_call\tmcu_per_s"
  "\tjit\tnote\n");

for (i = 0; i < SHORT_BENCHMARK_COUNT; i++)
  for (j = 0; j < SHORT_LENGTH_COUNT; j++)
    {
    size_t slice = short_lengths[j];
    int errorcode;
    PCRE2_SIZE erroroffset;
    pcre2_code *code;
    pcre2_match_data *match_data;
    PCRE2_UCHAR pattern[TEXT_MAX];
    double best = 0.0;
    long expected = -1;
    int rep;

    code = pcre2_compile(widen(short_benchmarks[i].pattern, pattern),
      PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroroffset, NULL);
    if (code == NULL)
      {
      PCRE2_UCHAR message[TEXT_MAX];
      char text[TEXT_MAX];
      pcre2_get_error_message(errorcode, message, TEXT_MAX);
      fprintf(stderr, "%s: compile failed at %d: %s\n",
        short_benchmarks[i].name, (int)erroroffset, narrow(message, text));
      return 1;
      }

    if (use_jit && pcre2_jit_compile(code, PCRE2_JIT_COMPLETE) != 0)
      {
      fprintf(stderr, "%s: JIT compile failed\n", short_benchmarks[i].name);
      return 1;
      }

    match_data = pcre2_match_data_create_from_pattern(code, NULL);

    for (rep = 0; rep < REPEATS; rep++)
      {
      double start = now_seconds();
      double elapsed;
      long calls = 0;
      long sweeps = 0;
      long hits = 0;
      double per_call;

      do
        {
        size_t offset;
        for (offset = 0; offset + slice <= total; offset += slice)
          {
          int rc;

          if (use_jit)
            rc = pcre2_jit_match(code, subject + offset, slice, 0, 0,
              match_data, NULL);
          else
            rc = pcre2_match(code, subject + offset, slice, 0, PCRE2_NO_JIT,
              match_data, NULL);

          if (rc >= 0) hits++;
          else if (rc != PCRE2_ERROR_NOMATCH)
            {
            fprintf(stderr, "%s: match error %d\n",
              short_benchmarks[i].name, rc);
            return 1;
            }
          calls++;
          }
        sweeps++;
        elapsed = now_seconds() - start;
        }
      while (elapsed < MIN_SECONDS);

      /* One sweep visits every slice exactly once, so the hit count per sweep
      is a constant for each pattern.  Check it. */

      if (expected < 0) expected = hits / sweeps;
      if (hits != expected * sweeps)
        {
        fprintf(stderr, "%s: unstable match count\n",
          short_benchmarks[i].name);
        return 1;
        }

      per_call = 1e9 * elapsed / (double)calls;
      if (best == 0.0 || per_call < best) best = per_call;
      }

    printf("%s\t%d\t%s\t%lu\t%ld\t%.1f\t%.2f\t%d\t%s\n", label,
      PCRE2_CODE_UNIT_WIDTH, short_benchmarks[i].name, (unsigned long)slice,
      expected, best, (double)slice / (best * 1e-9) / (1024.0 * 1024.0),
      use_jit, short_benchmarks[i].note);
    fflush(stdout);

    pcre2_match_data_free(match_data);
    pcre2_code_free(code);
    }

return 0;
}



/*************************************************
*                     Main                       *
*************************************************/

int
main(int argc, char **argv)
{
size_t length = SUBJECT_UNITS;
PCRE2_UCHAR *subject;
const char *label = "build";
int use_jit = 1;
int short_mode = 0;
int rc;
int i;

for (i = 1; i < argc; i++)
  {
  if (strcmp(argv[i], "nojit") == 0) use_jit = 0;
  else if (strcmp(argv[i], "short") == 0) short_mode = 1;
  else if (i == 1) label = argv[i];
  else
    {
    fprintf(stderr, "usage: %s [label] [nojit] [short]\n", argv[0]);
    return 2;
    }
  }

subject = build_subject(length);
if (subject == NULL)
  {
  fprintf(stderr, "out of memory\n");
  return 1;
  }

rc = short_mode ?
  run_short(label, subject, length, use_jit) :
  run_throughput(label, subject, length, use_jit);

free(subject);
return rc;
}

/* End of pcre2bench.c */
