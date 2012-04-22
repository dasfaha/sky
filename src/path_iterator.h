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

#ifndef _path_iterator_h
#define _path_iterator_h

#include <inttypes.h>
#include <stdbool.h>

#include "bstring.h"
#include "object_file.h"
#include "cursor.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// The path iterator is used to sequentially loop over a set of paths in a set
// of blocks. The next path can be requested from the iterator by calling the
// `PathIterator_next()` function. When calling the `next()` function, a cursor
// is returned instead of a reference to a deserialized path. The cursor can be
// used to iterate over the raw path data.
//
// To perform the relational database equivalent of a full table scan, the
// `ObjectFile_create_iterator()` function can be used to create a path
// iterator. That contains all the paths in the object file.
//
// The path iterator operates as a forward-only iterator. Jumping to the
// previous path or jumping to a path by index is not allowed.
//
// The path iterator does not currently support full consistency if events are
// added or removed after the iterator has been created and before the iteration
// is complete. The biggest issue is that a block split can cause paths to not
// be counted. This will be fixed in a future version.


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct PathIterator {
    ObjectFile *object_file;
    uint32_t block_index;
    uint32_t byte_index;
    bool eof;
} PathIterator;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

PathIterator *PathIterator_create(ObjectFile *object_file);

void PathIterator_destroy(PathIterator *iterator);


//======================================
// Iteration
//======================================

int PathIterator_next(PathIterator *iterator, Cursor *cursor);

int PathIterator_eof(PathIterator *iterator);


#endif
