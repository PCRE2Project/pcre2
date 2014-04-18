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
*        Return info about compiled pattern      *
*************************************************/

/* 
Arguments:
  code          points to compiled code
  what          what information is required
  where         where to put the information

Returns:        0 if data returned, negative on error
*/

/* FIXME: this is currently incomplete */

PCRE2_EXP_DEFN int PCRE2_CALL_CONVENTION
pcre2_pattern_info(const pcre2_code *code, uint32_t what, void *where)
{
const pcre2_real_code *re = (pcre2_real_code *)code;
                                                                          
if (re == NULL || where == NULL) return PCRE2_ERROR_NULL;
                                                                          
/* Check that the first field in the block is the magic number. If it is not,
return with PCRE2_ERROR_BADMAGIC. However, if the magic number is equal to
REVERSED_MAGIC_NUMBER we return with PCRE2_ERROR_BADENDIANNESS, which      
means that the pattern is likely compiled with different endianness. */ 
                                                                   
if (re->magic_number != MAGIC_NUMBER)                               
  return re->magic_number == REVERSED_MAGIC_NUMBER?                     
    PCRE2_ERROR_BADENDIANNESS:PCRE2_ERROR_BADMAGIC;                      
                                                                       
/* Check that this pattern was compiled in the correct bit mode */        
                                                            
if ((re->flags & (PCRE2_CODE_UNIT_WIDTH/8)) == 0) 
  return PCRE2_ERROR_BADMODE;                 

switch(what)
  {
  case PCRE2_INFO_NAMEENTRYSIZE:
  *((int *)where) = re->name_entry_size;
  break;
       
  case PCRE2_INFO_NAMECOUNT:
  *((int *)where) = re->name_count;
  break;
        
  case PCRE2_INFO_SIZE:
  *((size_t *)where) = re->size;                                
  break;   
  
  case PCRE2_INFO_JITSIZE:
#ifdef SUPPORT_JIT
  *((size_t *)where) =                                                     
      (re->flags & PCRE2_EXTRA_EXECUTABLE_JIT) != 0 &&             
      re->executable_jit != NULL)?                              
    PRIV(jit_get_size)(re->executable_jit) : 0;            
#else                                                               
  *((size_t *)where) = 0;                                               
#endif              
  break;   
  
  default: return PCRE2_ERROR_BADOPTION;   
  } 

return 0;
}

/* End of pcre2_pattern_info.c */
