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

//--------------------------------------
// File Stats
//--------------------------------------

bool sky_file_exists(bstring path);

bool sky_file_is_dir(bstring path);

off_t sky_file_get_size(bstring path);


//--------------------------------------
// File Copy
//--------------------------------------

int sky_file_cp(bstring src, bstring dest);

int sky_file_cp_r(bstring src, bstring dest);


//--------------------------------------
// File Delete
//--------------------------------------

int sky_file_rm(bstring path);

// int sky_file_cp_r(bstring src, bstring dest);


#endif
