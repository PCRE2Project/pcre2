/* This is a placeholder, just enough to allow dummy functions to compile */
#include <ctype.h>                                                          
#include <limits.h>                                                         
#include <stddef.h>                                                           
#include <stdint.h>
#include <stdio.h>                                                        
#include <stdlib.h>                                                        
#include <string.h>

#include "pcre2.h"

#define PUBL(name) pcre_##name
#define PRIV(name) _pcre_##name

