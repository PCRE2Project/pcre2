/****************************************************
* PCRE maintainers' helper program: UTF-8 converter *
****************************************************/

/* This is a test program for converting character codepoints to UTF-8 and
vice versa. Note that this program conforms to the original definition of
UTF-8, which allows codepoints up to 7fffffff. The more recent definition
limits the validity of Unicode UTF-8 codepoints to a maximum of 10ffff, and
forbids the "surrogate" codepoints. This program now gives warnings for these
invalid codepoints.

The arguments are either single codepoint values written as U+hh.. or 0xhh..
for conversion to UTF-8, or sequences of hex values, written without a prefix
and optionally including spaces (but such arguments must be quoted), for
encoding from UTF-8 code units to Unicode codepoints. For example:

  ./utf8 0x1234
  U+00001234 => e1 88 b4

  ./utf8 "e1 88 b4"
  U+00001234 <= e1 88 b4

In the second case, a number of UTF-8 characters can be present in one
argument. In other words, each such argument is interpreted (after ignoring
spaces) as a string of UTF-8 bytes representing a string of characters:

  ./utf8 "65 e188b4 77"
  0x00000065 <= 65
  0x00001234 <= e1 88 b4
  0x00000077 <= 77

If the option -s is given, the sequence of UTF-bytes is written out between
angle brackets at the end of the line, if valid. On a UTF-8 terminal, this
should show the appropriate graphic for the character or a question mark.

if the option -b is given, a file with the encoded bytes is written for use
with pcre2test in utf8_input format.

Errors provoke error messages, but the program carries on with the next
argument. The return code is always zero unless there was nothing to process
or an invalid option was provided and the "usage" was printed.

Philip Hazel
Original creation date: unknown
Code extended and tidied to avoid compiler warnings: 26 March 2020
Support for encoding utf8_input; 31 August 2025
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

/* The valid ranges for UTF-8 characters are:

0000 0000  to  0000 007f   1 byte (ascii)
0000 0080  to  0000 07ff   2 bytes
0000 0800  to  0000 ffff   3 bytes
0001 0000  to  001f ffff   4 bytes
0020 0000  to  03ff ffff   5 bytes
0400 0000  to  7fff ffff   6 bytes
*/


static const unsigned int utf8_table1[] = {
  0x0000007f, 0x000007ff, 0x0000ffff, 0x001fffff, 0x03ffffff, 0x7fffffff};

static const unsigned char utf8_table2[] = {
  0, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc};

static const unsigned char utf8_table3[] = {
  0xff, 0x1f, 0x0f, 0x07, 0x03, 0x01};


/*************************************************
*       Convert character value to UTF-8         *
*************************************************/

/* This function takes an unsigned long integer value in the range 0 -
0x7fffffff and encodes it as a UTF-8 character in 1 to 6 bytes.

Arguments:
  cvalue     the character value
  buffer     pointer to buffer for result - at least 6 bytes long

Returns:     number of bytes placed in the buffer
             0 if input codepoint is too big
*/

static size_t
ord2utf8(unsigned long int cvalue, unsigned char *buffer)
{
size_t i, j;
for (i = 0; i < sizeof(utf8_table1)/sizeof(int); i++)
  if (cvalue <= utf8_table1[i]) break;
if (i >= sizeof(utf8_table1)/sizeof(int)) return 0;
buffer += i;
for (j = i; j > 0; j--)
 {
 *buffer-- = 0x80 | (cvalue & 0x3f);
 cvalue >>= 6;
 }
*buffer = (unsigned char)(utf8_table2[i] | cvalue);
return i + 1;
}



/*************************************************
*            Convert UTF-8 string to value       *
*************************************************/

/* This function takes one or more bytes that represent a UTF-8 character from
the start of a string of bytes. It returns the value of the character, or the
offset of a malformation. For an overlong encoding that works but is not the
correct (shortest) one, the error offset is just after the last byte.

Argument:
  buffer   a pointer to the byte vector
  buffend  a pointer to the end of the buffer
  vptr     a pointer to a variable to receive the value
  lenptr   a pointer to a variable to receive the offset when error detected

Returns:   > 0 => the number of bytes consumed
             0 => invalid UTF-8: first byte missing 0x40 bit
            -1 => invalid UTF-8: first byte has too many high-order 1-bits
            -2 => incomplete sequence at end of string
            -3 => incomplete sequence within string
            -4 => overlong code sequence
*/

static int
utf82ord(const unsigned char *buffer, const unsigned char *buffend,
  long unsigned int *vptr, int *lenptr)
{
unsigned int c = *buffer++;
unsigned int d = c;
int i, j, s;

/* Check for an ASCII character, or find the number of additional bytes in a
multibyte character. */

for (i = -1; i < 6; i++)
  {
  if ((d & 0x80) == 0) break;
  d <<= 1;
  }

switch (i)
  {
  case -1:      /* ASCII character; first byte does not have 0x80 bit */
  *vptr = c;
  return 1;

  case 0:       /* First byte has 0x80 but is missing 0x40 bit */
  *lenptr = 0;
  return 0;

  case 6:
  *lenptr = 0;  /* Too many high bits */
  return -1;

  default:
  break;
  }

/* i now has a value in the range 1-5 */

s = 6 * i;
d = (c & utf8_table3[i]) << s;

for (j = 0; j < i; j++)
  {
  if (buffer >= buffend)
    {
    *lenptr = j + 1;
    return -2;
    }
  c = *buffer++;
  if ((c & 0xc0) != 0x80)
    {
    *lenptr = j + 1;
    return -3;
    }
  s -= 6;
  d |= (c & 0x3f) << s;
  }

/* Valid UTF-8 syntax */

*vptr = d;

/* Check that encoding was the correct one, not overlong */

for (j = 0; j < (int)(sizeof(utf8_table1)/sizeof(int)); j++)
  if (d <= utf8_table1[j]) break;
if (j != i)
  {
  *lenptr = i + 1;
  return -4;
  }

/* Valid value */

return i + 1;
}

/**********************************************
*                    Usage                    *
**********************************************/

static void
usage(const char *argv0)
{
printf("%s [option ..] argument ..\n\n", argv0);
puts("Encode/decode Unicode codepoints with UTF-8 code units\n");
puts("The arguments are either single codepoint values written as U+hh..");
puts("or 0xhh.. for conversion to UTF-8, or sequences of hex values,");
puts("written without a prefix and optionally including spaces (but such");
puts("arguments must be quoted), for encoding from UTF-8 code units to");
puts("Unicode codepoints.");
puts("For details on usage and examples read the comments in source code.\n");
puts("Options:\n");
puts("  -h|--help\tthis help");
puts("  -s\t\tprint character");
puts("  -b[=<file>]\twrite encoded data to file (default: testinput11)\n");
}

/*************************************************
*                 Main Program                   *
*************************************************/

int
main(int argc, char **argv)
{
int i = 1;
int show = 0;
unsigned char buffer[64];
const char *argv0 = "utf8";
FILE *f = NULL;

for (int c = argc; c-- > 1; i++)
  {
  const char *x = argv[i];

  if (*x++ != '-') break;
  if (*x == '-' && *++x == 0)
    {
    i++;
    break;
    }
  switch (*x++)
    {
    case 's': show = 1; break;
    case 'b':
      {
      const char *output = "testinput11";
      if (*x++ == '=' && *x != 0) output = x;
      f = fopen(output, "wb");
      }
    break;
    default:
      {
      const char last_option = x[-1];
      argv0 = argv[0];
      usage(argv0);
      return (last_option != 'h');
      }
    }
  }

if (i >= argc)
  {
  usage(argv0);
  return 1;
  }

for (; i < argc; i++)
  {
  const char *x = argv[i];

  if (strlen(x) >= 3 &&
      (strncmp(x, "0x", 2) == 0 || strncmp(x, "U+", 2) == 0) &&
      isxdigit(x[2]))
    {
    size_t rc;
    unsigned long d;
    char *endptr;
    int utf8_input = 0;

    errno = 0;
    d = strtoul(x + 2, &endptr, 16);
    if (errno != 0 || *endptr != 0)
      {
      printf("** Invalid hex number %s\n", x);
      continue;   /* With next argument */
      }
    if (d > 0xffffffff)
      {
      puts("** Code points must fit an uint32_t");
      continue;
      }
    else if (f != NULL && d > 0x7fffffff)
      {
      buffer[0] = 0xff;
      fwrite(buffer, 1, 1, f);
      utf8_input = 1;
      d &= 0x7fffffff;
      }

    rc = ord2utf8(d, buffer);
    printf("U+%08lx => ", d);
    if (rc == 0)
      fputs("** -b needed for codepoints greater than 0x7fffffff", stdout);
    else
      {
      size_t j;

      for (j = 0; j < rc; j++) printf("%02x ", buffer[j]);
      if (f != NULL) fwrite(buffer, rc, 1, f);
      if (utf8_input)
        fputs("** Not valid UTF-8, top bit set", stdout);
      else if (d > 0x10ffff)
        fputs("** Invalid Unicode (greater than U+10ffff)", stdout);
      else if (0xd800 <= d && d <= 0xdfff)
        fputs("** Invalid Unicode (UTF-16 surrogate)", stdout);
      else if (show)
        {
        putchar('>');
        for (j = 0; j < rc; j++) printf("%c", buffer[j]);
        putchar('<');
        }
      }
    putchar('\n');
    }
  else
    {
    unsigned char *bptr;
    const unsigned char *buffend;
    unsigned char y = 0;
    int len = 0;
    int z = 0;

    for (;;)
      {
      while (*x == ' ') x++;
      if (*x == 0 && !z) break;
      if (!isxdigit(*x))
        {
        printf("** Malformed hex string: %s\n", argv[i]);
        len = -1;
        break;
        }
      y = y * 16 +
          (unsigned char)(tolower(*x) - ((isdigit(*x))? '0' : 'a' - 10));
      x++;
      if (z)
        {
        buffer[len++] = y;
        y = 0;
        }
      z ^= 1;
      }

    if (len < 0) continue;  /* With next argument after malformation */

    bptr = buffer;
    buffend = buffer + len;

    while (bptr < buffend)
      {
      unsigned long int d;
      int j;
      int offset;
      int rc = utf82ord(bptr, buffend, &d, &offset);

      if (rc > 0)
        {
        printf("U+%08lx <= ", d);
        for (j = 0; j < rc; j++) printf("%02x ", bptr[j]);
        if (d <= 0x10ffff && (d < 0xd800 || 0xdfff < d) && show)
          {
          putchar('>');
          for (j = 0; j < rc; j++) printf("%c", bptr[j]);
          putchar('<');
          }
        putchar('\n');
        bptr += rc;
        }
      else if (rc == -4)
        {
        printf("U+%08lx <= ", d);
        for (j = 0; j < offset; j++) printf("%02x ", bptr[j]);
        puts("** Overlong UTF-8 sequence");
        bptr += offset;
        }
      else
        {
        switch (rc)
          {
          case 0:  fputs("** First byte missing 0x40 bit", stdout);
          break;

          case -1: fputs("** First byte has too many high-order bits", stdout);
          break;

          case -2: fputs("** Incomplete UTF-8 sequence at end of string",
                         stdout);
          break;

          case -3: fputs("** Incomplete UTF-8 sequence", stdout);
          break;

          default: printf("** Unexpected return %d from utf82ord()", rc);
          break;
          }
        printf(" at offset %d in string ", offset);
        while (bptr < buffend) printf("%02x ", *bptr++);
        putchar('\n');
        break;
        }
      }
    }
  }

if (f != NULL) fclose(f);

return 0;
}

/* End */
