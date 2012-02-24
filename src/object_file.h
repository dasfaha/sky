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

#ifndef _object_file_h
#define _object_file_h

#include "bstring.h"
#include "database.h"

//==============================================================================
//
// Overview
//
//==============================================================================

/**
 * The object file represents the storage for a type of object. The object file
 * is analogous to a table in a relational database. The object file is
 * represented on the file system as a directory that contains a header file
 * and multiple data extent files numbered sequentially (1, 2, 3, etc).
 *
 * Extents are a collection of 64k blocks. Each block can store any number of
 * paths that can fit into it. If the size of the paths is larger than the block
 * can handle then the block is split into multiple blocks.
 *
 * Blocks are stored in the order in which they are created. This means that 
 * while objects are stored in order within a block, they are not necessarily 
 * stored in order from one block to the next. Because of this, the header file
 * is used to store a range of object ids for each block and serves as an index
 * when looking up a single object.
 *
 * Because of the redundancy of action names and data keys, those strings are
 * cached and converted into integer identifiers. The action cache is located
 * in the 'actions' file and the data keys cache is located in the 'keys' file.
 */



//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct ObjectFile {
    Database *database;
    bstring name;
} ObjectFile;


//==============================================================================
//
// Functions
//
//==============================================================================

ObjectFile *ObjectFile_create(Database *database, bstring name);

void ObjectFile_destroy(ObjectFile *object_file);


#endif
