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
// Object File Management
//
//==============================================================================

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
