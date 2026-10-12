/* Set PCRE2_CODE_UNIT_WIDTH to indicate we will use 8-bit input. */
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include <string.h> /* for strlen */
#include <stdio.h>  /* for printf */

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pattern> <subject>\n", argv[0]);
        return 1;
    }

    const char *pattern = argv[1];
    const char *subject = argv[2];

    /* Compile the pattern. */
    int error_number;
    PCRE2_SIZE error_offset;
    pcre2_code *re = pcre2_compile(
        pattern,               /* the pattern */
        PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
        0,                     /* default options */
        &error_number,         /* for error number */
        &error_offset,         /* for error offset */
        NULL);                 /* use default compile context */
    if (re == NULL) {
        fprintf(stderr, "Invalid pattern: %s\n", pattern);
        return 1;
    }

    /* Match the pattern against the subject text. */
    pcre2_match_data *match_data =
        pcre2_match_data_create_from_pattern(re, NULL);
    int rc = pcre2_match(
        re,                   /* the compiled pattern */
        subject,              /* the subject text */
        strlen(subject),      /* the length of the subject */
        0,                    /* start at offset 0 in the subject */
        0,                    /* default options */
        match_data,           /* block for storing the result */
        NULL);                /* use default match context */

    /* Print the match result. */
    if (rc == PCRE2_ERROR_NOMATCH) {
        printf("No match\n");
    } else if (rc < 0) {
        fprintf(stderr, "Matching error\n");
    } else {
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
        printf("Found match: '%.*s'\n", (int)(ovector[1] - ovector[0]),
               subject + ovector[0]);
    }

    pcre2_match_data_free(match_data);   /* Free resources */
    pcre2_code_free(re);
    return 0;
}
