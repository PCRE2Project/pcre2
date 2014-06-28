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



/*************************************************
*          Default malloc/free functions         *
*************************************************/

/* Ignore the "user data" argument in each case. */

static void *default_malloc(size_t size, void *data)
{
(void)data;
return malloc(size);
}


static void default_free(void *block, void *data)
{
(void)data;
free(block);
}



/*************************************************
*        Get a block and save memory control     *
*************************************************/

/* This internal function is called to get a block of memory in which the 
memory control data is to be stored for future use.

Arguments:
  size        amount of memory required
  offset      offset in memory block to memctl structure
  memctl      pointer to a memctl block or NULL
  
Returns:      pointer to memory or NULL on failure
*/   

PCRE2_EXP_DEFN void *
PRIV(memctl_malloc)(size_t size, size_t offset, pcre2_memctl *memctl)
{
pcre2_memctl *newmemctl;
void *yield = (memctl == NULL)? malloc(size) :
  memctl->malloc(size, memctl->memory_data);
if (yield == NULL) return NULL; 
newmemctl = (pcre2_memctl *)(((uint8_t *)yield) + offset);
if (memctl == NULL)
  {
  newmemctl->malloc = default_malloc;
  newmemctl->free = default_free;
  newmemctl->memory_data = NULL;
  }
else *newmemctl = *memctl;       
return yield;
}   



/*************************************************
*          Create and initialize contexts        *
*************************************************/

/* Initializing for compile and match contexts is done in separate, private
functions so that these can be called from functions such as pcre2_compile()
when an external context is not supplied. The initializing functions have an 
option to set up default memory management. */

PCRE2_EXP_DEFN pcre2_general_context * PCRE2_CALL_CONVENTION
pcre2_general_context_create(void *(*private_malloc)(size_t, void *), 
  void (*private_free)(void *, void *), void *memory_data)
{
pcre2_general_context *gcontext;
if (private_malloc == NULL) private_malloc = default_malloc;
if (private_free == NULL) private_free = default_free;
gcontext = private_malloc(sizeof(pcre2_real_general_context), memory_data);
if (gcontext == NULL) return NULL;
gcontext->memctl.malloc = private_malloc;
gcontext->memctl.free = private_free;      
gcontext->memctl.memory_data = memory_data;
return gcontext;
}


PCRE2_EXP_DEFN void
PRIV(compile_context_init)(pcre2_compile_context *ccontext, BOOL defmemctl)
{
if (defmemctl)
  {
  ccontext->memctl.malloc = default_malloc;
  ccontext->memctl.free = default_free;
  ccontext->memctl.memory_data = NULL;
  } 
ccontext->stack_guard = NULL;
ccontext->tables = PRIV(default_tables);
ccontext->parens_nest_limit = PARENS_NEST_LIMIT;
ccontext->newline_convention = NEWLINE_DEFAULT;
ccontext->bsr_convention = BSR_DEFAULT;
}


PCRE2_EXP_DEFN pcre2_compile_context * PCRE2_CALL_CONVENTION
pcre2_compile_context_create(pcre2_general_context *gcontext)
{
pcre2_compile_context *ccontext = PRIV(memctl_malloc)(
  sizeof(pcre2_real_compile_context), 
  offsetof(pcre2_real_compile_context, memctl),
  &(gcontext->memctl)); 
if (ccontext == NULL) return NULL;  
PRIV(compile_context_init)(ccontext, FALSE);
return ccontext;
}


PCRE2_EXP_DEFN void
PRIV(match_context_init)(pcre2_match_context *mcontext, BOOL defmemctl)
{
if (defmemctl)
  {
  mcontext->memctl.malloc = default_malloc;
  mcontext->memctl.free = default_free;
  mcontext->memctl.memory_data = NULL;
  } 
#ifdef NO_RECURSE  
mcontext->stack_malloc = mcontext->malloc;
mcontext->stack_free = mcontext->free; 
#endif
mcontext->callout = NULL;
mcontext->callout_data = NULL;
mcontext->newline_convention = 0;
mcontext->bsr_convention = 0;
mcontext->match_limit = MATCH_LIMIT;
mcontext->recursion_limit = MATCH_LIMIT_RECURSION;
}


PCRE2_EXP_DEFN pcre2_match_context * PCRE2_CALL_CONVENTION
pcre2_match_context_create(pcre2_general_context *gcontext)
{
pcre2_match_context *mcontext = PRIV(memctl_malloc)(
  sizeof(pcre2_real_match_context),
  offsetof(pcre2_real_compile_context, memctl),
  &(gcontext->memctl));  
if (mcontext == NULL) return NULL;   
PRIV(match_context_init)(mcontext, FALSE);
return mcontext;
}


/*************************************************
*              Context copy functions            *
*************************************************/

PCRE2_EXP_DEFN pcre2_general_context * PCRE2_CALL_CONVENTION
pcre2_general_context_copy(pcre2_general_context *gcontext)
{
pcre2_general_context *new = 
  gcontext->memctl.malloc(sizeof(pcre2_real_general_context), 
  gcontext->memctl.memory_data);
if (new == NULL) return NULL;
memcpy(new, gcontext, sizeof(pcre2_real_general_context));
return new;
}


PCRE2_EXP_DEFN pcre2_compile_context * PCRE2_CALL_CONVENTION
pcre2_compile_context_copy(pcre2_compile_context *ccontext)
{
pcre2_compile_context *new = 
  ccontext->memctl.malloc(sizeof(pcre2_real_compile_context), 
  ccontext->memctl.memory_data);
if (new == NULL) return NULL;
memcpy(new, ccontext, sizeof(pcre2_real_compile_context));
return new;
}


PCRE2_EXP_DEFN pcre2_match_context * PCRE2_CALL_CONVENTION
pcre2_match_context_copy(pcre2_match_context *mcontext)
{
pcre2_match_context *new = 
  mcontext->memctl.malloc(sizeof(pcre2_real_match_context), 
  mcontext->memctl.memory_data);
if (new == NULL) return NULL;
memcpy(new, mcontext, sizeof(pcre2_real_match_context));
return new;
}



/*************************************************
*              Context free functions            *
*************************************************/


PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_general_context_free(pcre2_general_context *gcontext)
{
if (gcontext != NULL)
  gcontext->memctl.free(gcontext, gcontext->memctl.memory_data);
}


PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_compile_context_free(pcre2_compile_context *ccontext)
{
if (ccontext != NULL)
  ccontext->memctl.free(ccontext, ccontext->memctl.memory_data);
}


PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_match_context_free(pcre2_match_context *mcontext)
{
if (mcontext != NULL)
  mcontext->memctl.free(mcontext, mcontext->memctl.memory_data);
}




/*************************************************
*             Set values in contexts             *
*************************************************/

/* All these functions return 1 for success or 0 if invalid data is given. Only 
some of the functions are able to test the validity of the data. */


/* ------------ Compile contexts ------------ */

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_character_tables(pcre2_compile_context *ccontext, 
  const unsigned char *tables)
{
ccontext->tables = tables;
return 1;
}

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_bsr_compile(pcre2_compile_context *ccontext, uint32_t value)
{
switch(value)
  {
  case PCRE2_BSR_ANYCRLF:
  case PCRE2_BSR_UNICODE:
  ccontext->bsr_convention = value;
  return 1;
  
  default:
  return 0;  
  }
}

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_newline_compile(pcre2_compile_context *ccontext, uint32_t newline)
{
switch(newline)
  {
  case PCRE2_NEWLINE_CR:
  case PCRE2_NEWLINE_LF:
  case PCRE2_NEWLINE_CRLF:
  case PCRE2_NEWLINE_ANY:
  case PCRE2_NEWLINE_ANYCRLF:
  ccontext->newline_convention = newline;
  return 1;
     
  default: 
  return 0;  
  }   
}

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_parens_nest_limit(pcre2_compile_context *ccontext, uint32_t limit)
{
ccontext->parens_nest_limit = limit;
return 1;
}

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_compile_recursion_guard(pcre2_compile_context *ccontext, 
  int (*guard)(uint32_t))
{
ccontext->stack_guard = guard;
return 1;
}


/* ------------ Match contexts ------------ */

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_bsr_match(pcre2_match_context *mcontext, uint32_t value)
{
switch(value)
  {
  case PCRE2_BSR_ANYCRLF:
  case PCRE2_BSR_UNICODE:
  mcontext->bsr_convention = value;
  return 1;
  
  default:
  return 0;  
  }
}

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_newline_match(pcre2_match_context *mcontext, uint32_t newline)
{
switch(newline)
  {
  case PCRE2_NEWLINE_CR:
  case PCRE2_NEWLINE_LF:
  case PCRE2_NEWLINE_CRLF:
  case PCRE2_NEWLINE_ANY:
  case PCRE2_NEWLINE_ANYCRLF:
  mcontext->newline_convention = newline;
  return 1;
     
  default: 
  return 0;  
  }   
}

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_callout(pcre2_match_context *mcontext, 
  int (*callout)(pcre2_callout_block *), void *callout_data)
{
mcontext->callout = callout;
mcontext->callout_data = callout_data;
return 1;
}

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_match_limit(pcre2_match_context *mcontext, uint32_t limit)
{
mcontext->match_limit = limit;
return 1;
}

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_recursion_limit(pcre2_match_context *mcontext, uint32_t limit)
{
mcontext->recursion_limit = limit;
return 1;
}

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_set_recursion_memory_management(pcre2_match_context *mcontext, 
  void *(*mymalloc)(size_t, void *),
  void (*myfree)(void *, void *))
{
#ifdef NORECURSE
mcontext->stack_malloc = mymalloc;
mcontext->stack_free = myfree;
#else
(void)mcontext;
(void)mymalloc;
(void)myfree;
#endif
return 1;
}   

/* End of pcre2_context.c */
