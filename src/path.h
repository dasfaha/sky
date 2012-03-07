/*
 * Copyright (c) 2012 Ben Johnson, http://skylandlabs.com
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _path_h
#define _path_h

#include <inttypes.h>

#include "event.h"


//==============================================================================
//
// Overview
//
//==============================================================================

/**
 * A path is a collection of events that is associated with an object.
 */

//==============================================================================
//
// Typedefs
//
//==============================================================================

/**
 * The path stores an array of events.
 */
typedef struct Path {
    uint32_t event_count;
    Event *events;
} Path;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

Path *Path_create();

void Path_destroy(Path *path);


//======================================
// Serialization
//======================================

//int Path_serialize(Path *path, FILE *file);

//int Path_deserialize(Path *path, FILE *file);


//======================================
// Event Management
//======================================

int Path_add_event(Path *path, Event *event);

#endif
