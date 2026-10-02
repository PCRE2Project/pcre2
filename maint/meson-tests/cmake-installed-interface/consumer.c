#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <pcre2posix.h>

#if defined(EXPECT_STATIC) && !defined(PCRE2_STATIC)
#error Static PCRE2 CMake target did not define PCRE2_STATIC
#endif

#if defined(EXPECT_STATIC) && defined(PCRE2POSIX_SHARED)
#error Static PCRE2 POSIX CMake target defined PCRE2POSIX_SHARED
#endif

#if defined(EXPECT_SHARED) && defined(PCRE2_STATIC)
#error Shared PCRE2 CMake target defined PCRE2_STATIC
#endif

#if defined(EXPECT_SHARED) && !defined(PCRE2POSIX_SHARED)
#error Shared PCRE2 POSIX CMake target did not define PCRE2POSIX_SHARED
#endif

int main(void)
{
  PCRE2_UCHAR version[32];
  regex_t regex;
  int result;

  result = pcre2_config(PCRE2_CONFIG_VERSION, version);
  if (result < 0)
    return result;

  result = pcre2_regcomp(&regex, "", 0);
  if (result == 0)
    pcre2_regfree(&regex);
  return result;
}
