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

#define CSIZE sizeof(struct pcre2_real_context)


/*************************************************
*          Default malloc/free functions         *
*************************************************/

/* Ignore the "user data" argument in each case. */

static void *default_malloc(size_t size, void *data)
{
data = data;
return malloc(size);
}


static void default_free(void *block, void *data)
{
data = data;
free(block);
}



/*************************************************
*     Create/initialize, copy, free a context    *
*************************************************/

PCRE2_EXP_DEFN size_t PCRE2_CALL_CONVENTION
pcre2_context_size(void)
{
return CSIZE;
}


PCRE2_EXP_DEFN pcre2_context * PCRE2_CALL_CONVENTION
pcre2_init_context(pcre2_context *context)
{
if (context == NULL)
  {
  context = malloc(CSIZE);
  if (context == NULL) return NULL;
  }
context->callout = NULL;
context->malloc = default_malloc;
context->free = default_free;      
context->stack_malloc = default_malloc;
context->stack_free = default_free;      
context->stack_guard = NULL;
context->user_data = NULL;
context->tables = PRIV(default_tables);
#ifdef BSR_ANYCRLF
context->bsr_convention = PCRE2_BSR_ANYCRLF;
#else
context->bsr_convention = PCRE2_BSR_UNICODE;
#endif
context->newline_convention = NEWLINE;
context->options = 0;
context->match_limit = MATCH_LIMIT;
context->parens_nest_limit = PARENS_NEST_LIMIT;
context->recursion_limit = MATCH_LIMIT_RECURSION;
return context;
}


PCRE2_EXP_DEFN pcre2_context * PCRE2_CALL_CONVENTION
pcre2_copy_context(pcre2_context *context)
{
pcre2_context *new = context->malloc(CSIZE, context->user_data);
if (new == NULL) return NULL;
memcpy(new, context, CSIZE);
return new;
}


PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_free_context(pcre2_context *context)
{
context->free(context, context->user_data);
}


/*************************************************
*      Extract settings from a context           *
*************************************************/

PCRE2_EXP_DEFN uint32_t PCRE2_CALL_CONVENTION
pcre2_get_bsr_convention(pcre2_context *context)
{
return context->bsr_convention;
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
(*pcre2_get_callout(pcre2_context *context))(pcre2_callout_block *, void *)
{
return context->callout;
}                         


PCRE2_EXP_DEFN const unsigned char * PCRE2_CALL_CONVENTION
pcre2_get_character_tables(pcre2_context *context)
{
return context->tables;
}


PCRE2_EXP_DEFN uint32_t PCRE2_CALL_CONVENTION
pcre2_get_context_options(pcre2_context *context)
{
return context->options;
}


PCRE2_EXP_DEFN uint32_t PCRE2_CALL_CONVENTION 
pcre2_get_match_limit(pcre2_context *context)
{
return context->match_limit;
}


PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION 
pcre2_get_memory_management(pcre2_context *context, 
  void *(**mptr)(size_t, void *),
  void (**fptr)(void *, void *))
{
*mptr = context->malloc;
*fptr = context->free;
}


PCRE2_EXP_DEFN uint32_t PCRE2_CALL_CONVENTION 
pcre2_get_newline_convention(pcre2_context *context)
{
return context->newline_convention;
}


PCRE2_EXP_DEFN uint32_t PCRE2_CALL_CONVENTION 
pcre2_get_parens_nest_limit(pcre2_context *context)
{
return context->parens_nest_limit;
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
(*pcre2_get_recursion_guard(pcre2_context *context))(uint32_t, void *)
{
return context->stack_guard;
}


PCRE2_EXP_DEFN uint32_t PCRE2_CALL_CONVENTION
pcre2_get_recursion_limit(pcre2_context *context)
{
return context->recursion_limit;
}


PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_get_recursion_memory_management(pcre2_context *context,
  void *(**mptr)(size_t, void *),
  void (**fptr)(void *, void *))
{
*mptr = context->stack_malloc;
*fptr = context->stack_free;
}
   

PCRE2_EXP_DEFN void * PCRE2_CALL_CONVENTION
pcre2_get_user_data(pcre2_context *context)
{
return context->user_data;
}


/*************************************************
*             Set values in contexts             *
*************************************************/

/* All these functions return 1 for success or 0 if invalid data is given. Only 
some of the functions are able to test the validity of the data. */


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_bsr_convention(pcre2_context *context, uint32_t value)
{
switch(value)
  {
  case PCRE2_BSR_ANYCRLF:
  case PCRE2_BSR_UNICODE:
  context->bsr_convention = value;
  return 1;
  
  default:
  return 0;  
  }
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_callout(pcre2_context *context, 
  int (*callout)(pcre2_callout_block *, void *))
{
context->callout = callout;
return 1;
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_character_tables(pcre2_context *context, const unsigned char *tables)
{
context->tables = tables;
return 1;
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_context_options(pcre2_context *context, uint32_t unset_bits, 
  uint32_t set_bits)
{
if ((set_bits & ~PCRE2_CONTEXT_OPTIONS) != 0) return 0;
context->options = (context->options & ~unset_bits) | set_bits;
return 1;
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_match_limit(pcre2_context *context, uint32_t limit)
{
context->match_limit = limit;
return 1;
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_memory_management(pcre2_context *context, 
  void * (*mymalloc)(size_t, void*),
  void (*myfree)(void *, void *))
{
context->malloc = context->stack_malloc = mymalloc;
context->free = context->stack_free = myfree;
return 1;
}  
 
 
PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_newline_convention(pcre2_context *context, uint32_t newline)
{
switch(newline)
  {
  case PCRE2_NEWLINE_CR:
  case PCRE2_NEWLINE_LF:
  case PCRE2_NEWLINE_CRLF:
  case PCRE2_NEWLINE_ANY:
  case PCRE2_NEWLINE_ANYCRLF:
  context->newline_convention = newline;
  return 1;
     
  default: 
  return 0;  
  }   
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_parens_nest_limit(pcre2_context *context, uint32_t limit)
{
context->parens_nest_limit = limit;
return 1;
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_recursion_guard(pcre2_context *context, 
  int (*guard)(uint32_t, void *))
{
context->stack_guard = guard;
return 1;
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_recursion_limit(pcre2_context *context, uint32_t limit)
{
context->recursion_limit = limit;
return 1;
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_recursion_memory_management(pcre2_context *context, 
  void *(*mymalloc)(size_t, void *),
  void (*myfree)(void *, void *))
{
context->stack_malloc = mymalloc;
context->stack_free = myfree;
return 1;
}   
  

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_user_data(pcre2_context *context, void *data)
{
context->user_data = data;
return 1;
}


/* End of pcre2_context.c */
