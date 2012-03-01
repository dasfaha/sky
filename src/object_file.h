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

#include <inttypes.h>
#include <sys/stat.h>

#include "bstring.h"
#include "database.h"
#include "event.h"

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

/**
 * The block info stores the sequential block identifier as well as the object
 * identifier range that is stored in that block. The block info is used in the
 * header file as an index when looking up block data.
 */
typedef struct BlockInfo {
    uint32_t id;
    uint64_t min_object_id;
    uint64_t max_object_id;
} BlockInfo;

/**
 * An action defines a verb that is performed in an event. 4 billion (2^32)
 * unique types of actions can be defined within an object file. The name of the
 * action is stored in the 'actions' file and the action identifier is used when
 * storing event data in a block.
 */
typedef struct Action {
    int32_t id;
    bstring name;
} Action;

/**
 * A property is a key used on data in an event. The property identifier's range
 * is split up: positive ids are used for data attached to an object, negative
 * ids are used for data attached to an action and id 0 is reserved.
 *
 * Property identifiers are used when storing events in blocks because of their
 * redundancy. Property definitions are stored in the 'properties' file.
 */
typedef struct Property {
    int16_t id;
    bstring name;
} Property;

/**
 * The object file is a reference to the disk location where data is stored. The
 * object file also maintains a cache of block info and predefined actions and
 * properties.
 */
typedef struct ObjectFile {
    Database *database;
    bstring name;
    bstring path;
    uint32_t block_count;
    BlockInfo *infos;
    Action *actions;
    uint32_t action_count;
    Property *properties;
    uint16_t property_count;
} ObjectFile;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

ObjectFile *ObjectFile_create(Database *database, bstring name);

void ObjectFile_destroy(ObjectFile *object_file);


//======================================
// State
//======================================

int ObjectFile_open(ObjectFile *object_file);

int ObjectFile_close(ObjectFile *object_file);


//======================================
// Event Management
//======================================

int ObjectFile_add_event(ObjectFile *object_file, Event *event);

#endif
