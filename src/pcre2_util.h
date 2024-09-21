/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE2 is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
          New API code Copyright (c) 2016-2024 University of Cambridge

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

#ifndef PCRE2_UTIL_H_IDEMPOTENT_GUARD
#define PCRE2_UTIL_H_IDEMPOTENT_GUARD

/* Assertion macros */

#ifdef HAVE_BUILTIN_UNREACHABLE
#define PCRE2_UNREACHABLE() __builtin_unreachable()
#elif HAVE_BUILTIN_ASSUME
#define PCRE2_UNREACHABLE() __assume(0)
#else
#define PCRE2_UNREACHABLE() do {} while(0)
#endif

#ifdef PCRE2_DEBUG
#if defined(HAVE_ASSERT_H) && !defined(NDEBUG)
#include <assert.h>
#define PCRE2_ASSERT(x) assert(x)
#elif defined(HAVE_STDLIB_H) && defined(HAVE_STDIO_H)
#define PCRE2_ASSERT(x) do { if (!(x)) { fprintf(stderr, "Assertion failed at " __FILE__ ":%d\n", __LINE__); abort(); }} while(0)
#endif
#endif

#ifndef PCRE2_ASSERT
#define PCRE2_ASSERT(x) do {} while(0)
#endif

#endif  /* PCRE2_UTIL_H_IDEMPOTENT_GUARD */

/* End of pcre2_util.h */
