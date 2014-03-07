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
#define PRIV(name) _pcre2_##name

#define PCRE2_CALL_CONVENTION

extern const uint8_t PRIV(default_tables)[];


/* What follows is "real" code for PCRE2. */

/* Only these bits are allowed when setting context options. */

#define PCRE2_CONTEXT_OPTIONS (\
  PCRE2_ALT_BSUX|PCRE2_DOLLAR_ENDONLY|PCRE2_DUPNAMES|PCRE2_ALLOW_EMPTY_CLASS|\
  PCRE2_MATCH_UNSET_BACKREF|PCRE2_NEVER_UTF|PCRE2_NEVER_UCP|PCRE2_UTF|\
  PCRE2_UCP)

/* The real context structure */

struct pcre2_real_context {
  const unsigned char *tables;
  int       (*callout)(pcre2_callout_block *, void *);
  void *    (*malloc)(size_t, void *);
  void      (*free)(void *, void *);
  void *    (*stack_malloc)(size_t, void *);
  void      (*stack_free)(void *, void *);
  int       (*stack_guard)(uint32_t, void *);
  void      *user_data;
  uint16_t  bsr_convention;
  uint16_t  newline_convention;
  uint32_t  options;
  uint32_t  match_limit;
  uint32_t  parens_nest_limit;
  uint32_t  recursion_limit;
};

/* End of pcre2_internal.h */
