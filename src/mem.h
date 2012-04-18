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

#ifndef _mem_h
#define _mem_h

#include <string.h>

//==============================================================================
//
// Overview
//
//==============================================================================

// This file contains a collection of useful utility functions for reading and
// writing to memory-backed data. This is commonly used for serialization and
// deserialization of the Sky data types.


//==============================================================================
//
// Definitions
//
//==============================================================================

//--------------------------------------
// Memory writes
//--------------------------------------

// Writes "n" bytes from the source memory location to the destination memory
// location. If successful, the destination location is incremented by "n"
// bytes.
//
// DEST - The memory location to be written to.
// SRC  - The memory location to be read from.
// N    - The number of bytes to write.
// MSG  - The error message to display if the write fails.
#define memwrite(DEST, SRC, N, MSG) do {\
    void *result = memcpy(DEST, SRC, N);\
    check(result != NULL, "Unable to write " MSG);\
    DEST += N;\
} while(0)\

// Writes a bstring to memory.
#define memwrite_bstr(DEST, SRC, N, MSG)\
    if(N > 0) {\
        memwrite(DEST, bdata(SRC), N, MSG);\
    }


//--------------------------------------
// Memory reads
//--------------------------------------

// Reads "n" bytes from the source memory location to the destination memory
// location. If successful, the source location is incremented by "n"
// bytes.
//
// SRC  - The memory location to be read from.
// DEST - The memory location to be written to.
// N    - The number of bytes to write.
// MSG  - The error message to display if the write fails.
#define memread(SRC, DEST, N, MSG) do {\
    void *result = memcpy(DEST, SRC, N);\
    check(result != NULL, "Unable to read " MSG);\
    SRC += N;\
} while(0)\

// Reads a bstring from memory.
#define memread_bstr(SRC, DEST, N, MSG) do {\
    if(DEST != NULL) {\
        bdestroy(DEST);\
        DEST = NULL;\
    }\
    char *str = calloc(1, N+1); check_mem(str);\
    memread(SRC, str, N, MSG);\
    DEST = bfromcstr(str);\
    free(str);\
    check_mem(DEST);\
} while(0)
    


#endif
