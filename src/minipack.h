#ifndef _sky_minipack_h
#define _sky_minipack_h

#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>

#include "minipack/minipack.h"
#include "bstring.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Bstring
//--------------------------------------

int sky_minipack_fread_bstring(FILE *file, bstring *ret);

int sky_minipack_fwrite_bstring(FILE *file, bstring str);


#endif
