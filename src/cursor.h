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

#ifndef _cursor_h
#define _cursor_h

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "bstring.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// The cursor is used to iterate over the events of a path. It provides fast
// access to events in a path by performing data access on the raw bytes of the
// data file. It also abstracts away the underlying storage of the events by
// seamlessly combining spanned blocks into a single path.
//
// The current API to the cursor is simple. It provides forward-only access to
// basic event data in a path. However, future releases will allow bidirectional
// traversal, event search, & object state management.


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct Cursor {
    void **paths;
    uint32_t path_count;
    uint32_t path_index;
    uint32_t event_index;
    uint32_t path_length;
    void *ptr;
    void *endptr;
    bool eof;
} Cursor;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

Cursor *Cursor_create();

void Cursor_destroy(Cursor *cursor);


//======================================
// Path Management
//======================================

int Cursor_set_path(Cursor *cursor, void *ptr);

int Cursor_set_paths(Cursor *cursor, void **ptrs, int count);


//======================================
// Iteration
//======================================

int Cursor_next_event(Cursor *cursor);


#endif
