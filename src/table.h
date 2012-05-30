#ifndef _table_h
#define _table_h

#include <inttypes.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "bstring.h"
#include "database.h"
#include "event.h"
#include "types.h"

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
// Definitions
//
//==============================================================================

#define SKY_OBJECT_FILE_LOCK_NAME ".lock"

#define SKY_DEFAULT_BLOCK_SIZE 0x10000


//==============================================================================
//
// Typedefs
//
//==============================================================================

#define sky_block_info_id_t uint32_t

// The block info stores the sequential block identifier as well as the object
// identifier range that is stored in that block. The block info is used in the
// header file as an index when looking up block data.
typedef struct sky_block_info {
    sky_block_info_id_t id;
    sky_object_id_t min_object_id;
    sky_object_id_t max_object_id;
    sky_timestamp_t min_timestamp;
    sky_timestamp_t max_timestamp;
    bool spanned;
} sky_block_info;

// The various states that an table can be in.
typedef enum sky_table_state_e {
    SKY_OBJECT_FILE_STATE_CLOSED,
    SKY_OBJECT_FILE_STATE_OPEN,
    SKY_OBJECT_FILE_STATE_LOCKED
} sky_table_state_e;


#define sky_table_version_t uint32_t
#define sky_table_block_size_t uint32_t
#define sky_table_block_count_t uint32_t
#define sky_table_action_count_t uint32_t
#define sky_table_property_count_t uint16_t

// The table is a reference to the disk location where data is stored. The
// table also maintains a cache of block info and predefined actions and
// properties.
typedef struct sky_table {
    sky_database *database;
    bstring name;
    bstring path;
    sky_table_state_e state;
    sky_table_version_t version;
    sky_table_block_size_t block_size;
    sky_table_block_count_t block_count;
    sky_block_info **infos;
    sky_action **actions;
    sky_table_action_count_t action_count;
    sky_property **properties;
    sky_table_property_count_t property_count;
    int data_fd;
    void *data;
    size_t data_length;
} sky_table;


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


//======================================
// Locking
//======================================

int sky_table_lock(sky_table *table);

int sky_table_unlock(sky_table *table);


//======================================
// Block Management
//======================================

int sky_table_get_block_span_count(sky_table *table, uint32_t block_index, uint32_t *span_count);


//======================================
// Event Management
//======================================

int sky_table_add_event(sky_table *table, sky_event *event);


//======================================
// Action Management
//======================================

int sky_table_find_or_create_action_id_by_name(sky_table *table,
                                                     bstring name,
                                                     sky_action_id_t *action_id);

//======================================
// Property Management
//======================================

int sky_table_find_or_create_property_id_by_name(sky_table *table,
                                                       bstring name,
                                                       sky_property_id_t *property_id);

#endif
