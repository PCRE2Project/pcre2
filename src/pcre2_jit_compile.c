/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
     Original API code Copyright (c) 1997-2012 University of Cambridge
         New API code Copyright (c) 2014 University of Cambridge

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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre2_internal.h"

#ifdef SUPPORT_JIT

/* All-in-one: Since we use the JIT compiler only from here,
we just include it. This way we don't need to touch the build
system files. */

#define SLJIT_CONFIG_AUTO 1
#define SLJIT_CONFIG_STATIC 1
#define SLJIT_VERBOSE 0
#define SLJIT_DEBUG 0

#define SLJIT_MALLOC(size, allocator_data) pcre2_jit_malloc(size, allocator_data)
#define SLJIT_FREE(ptr, allocator_data) pcre2_jit_free(ptr, allocator_data)

static void * pcre2_jit_malloc(size_t size, void *allocator_data)
{
pcre2_memctl *allocator = ((pcre2_memctl*)allocator_data);
return allocator->malloc(size, allocator->memory_data);
}

static void pcre2_jit_free(void *ptr, void *allocator_data)
{
pcre2_memctl *allocator = ((pcre2_memctl*)allocator_data);
return allocator->free(ptr, allocator->memory_data);
}

#include "sljit/sljitLir.c"

#if defined SLJIT_CONFIG_UNSUPPORTED && SLJIT_CONFIG_UNSUPPORTED
#error Unsupported architecture
#endif

/* Defines for debugging purposes. */

/* 1 - Use unoptimized capturing brackets.
   2 - Enable capture_last_ptr (includes option 1). */
/* #define DEBUG_FORCE_UNOPTIMIZED_CBRAS 2 */

/* 1 - Always have a control head. */
/* #define DEBUG_FORCE_CONTROL_HEAD 1 */

/* Allocate memory for the regex stack on the real machine stack.
Fast, but limited size. */
#define MACHINE_STACK_SIZE 32768

/* Growth rate for stack allocated by the OS. Should be the multiply
of page size. */
#define STACK_GROWTH_RATE 8192

/* Enable to check that the allocation could destroy temporaries. */
#if defined SLJIT_DEBUG && SLJIT_DEBUG
#define DESTROY_REGISTERS 1
#endif








#endif

/*************************************************
*        JIT compile a Regular Expression        *
*************************************************/

/* This function used JIT to convert a previously-compiled pattern into machine
code.

Arguments:
  code          a compiled pattern
  options       JIT option bits

Returns:        nothing
*/

/* FIXME: this is currently a placeholder function */

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_jit_compile(pcre2_code *code, uint32_t options)
{
#ifndef SUPPORT_JIT

(void)code;
(void)options;
return PCRE2_ERROR_JIT_BADOPTION;

#else  /* SUPPORT_JIT */

/* Dummy code */
code=code;
options = options;
return PCRE2_ERROR_JIT_BADOPTION;

#endif  /* SUPPORT_JIT */
}

/* JIT compiler uses an all-in-one approach. This improves security,
   since the code generator functions are not exported. */

#define INCLUDED_FROM_PCRE2_JIT_COMPILE

#include "pcre2_jit_match.c"
#include "pcre2_jit_misc.c"

/* End of pcre2_jit_compile.c */
