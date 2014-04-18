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
*              Create contexts                   *
*************************************************/

PCRE2_EXP_DEFN pcre2_general_context * PCRE2_CALL_CONVENTION
pcre2_general_context_create(void *(*private_malloc)(size_t, void *), 
  void (*private_free)(void *, void *), void *memory_data)
{
pcre2_general_context *gcontext;
if (private_malloc == NULL) private_malloc = default_malloc;
if (private_free == NULL) private_free = default_free;
gcontext = private_malloc(sizeof(pcre2_real_general_context), memory_data);
gcontext->malloc = private_malloc;
gcontext->free = private_free;      
gcontext->memory_data = memory_data;
return gcontext;
}


PCRE2_EXP_DEFN pcre2_compile_context * PCRE2_CALL_CONVENTION
pcre2_compile_context_create(pcre2_general_context *gcontext)
{
pcre2_compile_context *ccontext;
void *(*compile_malloc)(size_t, void *);
void (*compile_free)(void *, void *);
void *memory_data;
if (gcontext == NULL)
  {
  compile_malloc = default_malloc;
  compile_free = default_free;
  memory_data = NULL;
  }
else
  {     
  compile_malloc = gcontext->malloc;
  compile_free = gcontext->free;
  memory_data = gcontext->memory_data;
  }  
ccontext = compile_malloc(sizeof(pcre2_real_compile_context), memory_data);
if (ccontext == NULL) return NULL;  
ccontext->malloc = compile_malloc;
ccontext->free = compile_free;
ccontext->memory_data = memory_data;
ccontext->stack_guard = NULL;
ccontext->tables = PRIV(default_tables);
#ifdef BSR_ANYCRLF
ccontext->bsr_convention = PCRE2_BSR_ANYCRLF;
#else
ccontext->bsr_convention = PCRE2_BSR_UNICODE;
#endif
ccontext->newline_convention = NEWLINE;
ccontext->parens_nest_limit = PARENS_NEST_LIMIT;
return ccontext;
}


PCRE2_EXP_DEFN pcre2_match_context * PCRE2_CALL_CONVENTION
pcre2_match_context_create(pcre2_general_context *gcontext)
{
pcre2_match_context *mcontext;
void *(*match_malloc)(size_t, void *);
void (*match_free)(void *, void *);
void *memory_data;
if (gcontext == NULL)
  {
  match_malloc = default_malloc;
  match_free = default_free;
  memory_data = NULL;
  }
else
  {     
  match_malloc = gcontext->malloc;
  match_free = gcontext->free;
  memory_data = gcontext->memory_data;
  }
mcontext = match_malloc(sizeof(pcre2_real_match_context), memory_data);
if (mcontext == NULL) return NULL;   
mcontext->malloc = match_malloc;
mcontext->free = match_free;
mcontext->memory_data = memory_data;
#ifdef NO_RECURSE  
mcontext->stack_malloc = match_malloc;
mcontext->stack_free = match_free; 
#endif
mcontext->callout = NULL;
mcontext->match_limit = MATCH_LIMIT;
mcontext->recursion_limit = MATCH_LIMIT_RECURSION;
return mcontext;
}


/*************************************************
*              Context copy functions            *
*************************************************/

PCRE2_EXP_DEFN pcre2_general_context * PCRE2_CALL_CONVENTION
pcre2_general_context_copy(pcre2_general_context *gcontext)
{
pcre2_general_context *new = 
  gcontext->malloc(sizeof(pcre2_real_general_context), gcontext->memory_data);
if (new == NULL) return NULL;
memcpy(new, gcontext, sizeof(pcre2_real_general_context));
return new;
}


PCRE2_EXP_DEFN pcre2_compile_context * PCRE2_CALL_CONVENTION
pcre2_compile_context_copy(pcre2_compile_context *ccontext)
{
pcre2_compile_context *new = 
  ccontext->malloc(sizeof(pcre2_real_compile_context), ccontext->memory_data);
if (new == NULL) return NULL;
memcpy(new, ccontext, sizeof(pcre2_real_compile_context));
return new;
}


PCRE2_EXP_DEFN pcre2_match_context * PCRE2_CALL_CONVENTION
pcre2_match_context_copy(pcre2_match_context *mcontext)
{
pcre2_match_context *new = 
  mcontext->malloc(sizeof(pcre2_real_match_context), mcontext->memory_data);
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
gcontext->free(gcontext, gcontext->memory_data);
}


PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_compile_context_free(pcre2_compile_context *ccontext)
{
ccontext->free(ccontext, ccontext->memory_data);
}


PCRE2_EXP_DEFN void PCRE2_CALL_CONVENTION
pcre2_match_context_free(pcre2_match_context *mcontext)
{
mcontext->free(mcontext, mcontext->memory_data);
}




/*************************************************
*             Set values in contexts             *
*************************************************/

/* All these functions return 1 for success or 0 if invalid data is given. Only 
some of the functions are able to test the validity of the data. */

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_bsr_convention(pcre2_compile_context *ccontext, uint32_t value)
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
pcre2_set_character_tables(pcre2_compile_context *ccontext, 
  const unsigned char *tables)
{
ccontext->tables = tables;
return 1;
}


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_newline_convention(pcre2_compile_context *ccontext, uint32_t newline)
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


PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION 
pcre2_set_callout(pcre2_match_context *mcontext, 
  int (*callout)(pcre2_callout_block *, void *))
{
mcontext->callout = callout;
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
