#ifndef _file_h
#define _file_h

#include <stdbool.h>
#include <sys/types.h>

#include "bstring.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// This file provides basic file related utility functions.


//==============================================================================
//
// Functions
//
//==============================================================================

bool sky_file_exists(bstring path);

off_t sky_file_get_size(bstring path);

int sky_file_cp(bstring src, bstring dest);


#endif
