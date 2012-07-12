#ifndef _table_h
#define _table_h

#include <inttypes.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef struct sky_table sky_table;

#include "bstring.h"
#include "database.h"
#include "action.h"
#include "event.h"
#include "types.h"
#include "data_file.h"
#include "action_file.h"

//==============================================================================
//
// Overview
//
//==============================================================================

// The table represents the storage for a type of object. The table
// is analogous to a table in a relational database. The table is
// represented on the file system as a directory that contains a header file
// and multiple data extent files numbered sequentially (1, 2, 3, etc).
//
// Extents are a collection of 64k blocks. Each block can store any number of
// paths that can fit into it. If the size of the paths is larger than the block
// can handle then the block is split into multiple blocks.
//
// Blocks are stored in the order in which they are created. This means that 
// while objects are stored in order within a block, they are not necessarily 
// stored in order from one block to the next. Because of this, the header file
// is used to store a range of object ids for each block and serves as an index
// when looking up a single object.
//
// Because of the redundancy of action names and data keys, those strings are
// cached and converted into integer identifiers. The action cache is located
// in the 'actions' file and the data keys cache is located in the 'keys' file.


//==============================================================================
//
// Typedefs
//
//==============================================================================

#define SKY_LOCK_NAME ".lock"

// The table is a reference to the disk location where data is stored. The
// table also maintains a cache of block info and predefined actions and
// properties.
struct sky_table {
    sky_database *database;
    sky_data_file *data_file;
    sky_action_file *action_file;
    bstring name;
    bstring path;
    bool opened;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_table *sky_table_create(sky_database *database, bstring name);

void sky_table_free(sky_table *table);


//======================================
// State
//======================================

int sky_table_open(sky_table *table);

int sky_table_close(sky_table *table);

#endif
