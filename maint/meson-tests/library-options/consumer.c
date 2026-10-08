#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <pcre2posix.h>

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
