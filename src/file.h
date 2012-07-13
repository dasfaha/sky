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

off_t sky_get_file_size(bstring path);


#endif
