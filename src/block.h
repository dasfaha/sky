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

#ifndef _block_h
#define _block_h

#include <inttypes.h>

#include "object_file.h"
#include "path.h"
#include "event.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// A block represents a contiguous area of storage for paths. A block can store
// multiple paths or paths can span across multiple blocks (block spanning).


//==============================================================================
//
// Typedefs
//
//==============================================================================

// The block stores an array of paths.
typedef struct Block {
    ObjectFile *object_file;
    BlockInfo *info;
    uint32_t path_count;
    Path **paths;
} Block;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

Block *Block_create();

void Block_destroy(Block *block);


//======================================
// Serialization
//======================================

int Block_serialize(Block *block, FILE *file);

int Block_deserialize(Block *block, FILE *file);


//======================================
// Event Management
//======================================

int Block_add_event(Block *block, Event *event);

int Block_remove_event(Block *block, Event *event);

#endif
