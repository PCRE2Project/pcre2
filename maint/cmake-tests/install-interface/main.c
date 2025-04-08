#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <stdio.h>

int main(void)
{
  char version_str[32];
  pcre2_config(PCRE2_CONFIG_VERSION, version_str);
  printf("Using PCRE2 version: %s\n", version_str);
  return 0;
}
