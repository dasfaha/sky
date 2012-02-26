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

#include <stdlib.h>

#include "dbg.h"
#include "bstring.h"
#include "database.h"
#include "object_file.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Header Management
//======================================

/**
 * Loads the header data into the object file structure.
 */
int load_header(ObjectFile *object_file)
{
    // TODO: If header file does not exist, set empty header data.
    // TODO: Otherwise open header file.
    // TODO: Read object id ranges into structs.
    // TODO: Close header file.
    // TODO: Sort ranges by starting object id.
    
    return 0;
}


//======================================
// Locking
//======================================

/**
 * Obtains a write lock on the object file.
 */
int lock(ObjectFile *object_file)
{
    // TODO: Check for lock file in object file directory.
    // TODO: If lock exists, check if owner process still exists.
    // TODO: If owner is gone then remove lock.
    
    // TODO: Write PID to lock file in object file directory.
}

/**
 * Removes a lock on the object file obtained by this process.
 */
int unlock(ObjectFile *object_file)
{
    // TODO: Check for lock file in object file directory.
    // TODO: If contents of lock file are this file's PID then remove lock file.
}


//======================================
// Lifecycle
//======================================

/*
 * Creates a reference to an object file.
 *
 * database - A reference to the database that the object file belongs to.
 * name - The name of the object file.
 */
ObjectFile *ObjectFile_create(Database *database, bstring name)
{
    ObjectFile *object_file;
    
    check(database != NULL, "Cannot create object file without a database");
    check(name != NULL, "Cannot create unnamed object file");
    
    object_file = malloc(sizeof(ObjectFile));
    object_file->name = bstrcpy(name); check_mem(object_file->name);

    return object_file;
    
error:
    ObjectFile_destroy(object_file);
    return NULL;
}

/*
 * Removes an object file reference from memory.
 */
void ObjectFile_destroy(ObjectFile *object_file)
{
    if(object_file) {
        bdestroy(object_file->name);
        free(object_file);
    }
}


//======================================
// State
//======================================

/**
 * Opens the object file for reading and writing events.
 */
int *ObjectFile_open(ObjectFile *object_file)
{
    // Obtain lock.
    check(lock(object_file) == 0, "Unable to obtain lock");
    
    // Load header data.
    check(load_header() == 0, "Unable to load header data");
    
    return 0;

error:
    return -1;
}

/**
 * Closes the object file.
 */
int *ObjectFile_close(ObjectFile *object_file)
{
    check(unlock(object_file) == 0, "Unable to remove lock");
    return 0;
    
error:
    return -1;
}


//======================================
// Event Management
//======================================

int *ObjectFile_add_event(Event *event)
{
    // TODO: If there are no blocks then create a block.
    // TODO: Otherwise find block that contains existing event.
    // TODO: If no existing object found, find closest matching block.
    
    // TODO: Find existing object path or create object path.
    // TODO: Insert event into path.
    // TODO: Insert path into block.
    // TODO: If block is larger than limit then split the block into multiple blocks.
    
    // TODO: Write all blocks to disk.
    
    return 0;
}