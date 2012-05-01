#ifndef _object_file_h
#define _object_file_h

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

// The object file represents the storage for a type of object. The object file
// is analogous to a table in a relational database. The object file is
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

// The various states that an object file can be in.
typedef enum sky_object_file_state_e {
    SKY_OBJECT_FILE_STATE_CLOSED,
    SKY_OBJECT_FILE_STATE_OPEN,
    SKY_OBJECT_FILE_STATE_LOCKED
} sky_object_file_state_e;


#define sky_object_file_version_t uint32_t
#define sky_object_file_block_size_t uint32_t
#define sky_object_file_block_count_t uint32_t
#define sky_object_file_action_count_t uint32_t
#define sky_object_file_property_count_t uint16_t

// The object file is a reference to the disk location where data is stored. The
// object file also maintains a cache of block info and predefined actions and
// properties.
typedef struct sky_object_file {
    sky_database *database;
    bstring name;
    bstring path;
    sky_object_file_state_e state;
    sky_object_file_version_t version;
    sky_object_file_block_size_t block_size;
    sky_object_file_block_count_t block_count;
    sky_block_info **infos;
    sky_action **actions;
    sky_object_file_action_count_t action_count;
    sky_property **properties;
    sky_object_file_property_count_t property_count;
    int data_fd;
    void *data;
    size_t data_length;
} sky_object_file;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_object_file *sky_object_file_create(sky_database *database, bstring name);

void sky_object_file_free(sky_object_file *object_file);


//======================================
// State
//======================================

int sky_object_file_open(sky_object_file *object_file);

int sky_object_file_close(sky_object_file *object_file);


//======================================
// Locking
//======================================

int sky_object_file_lock(sky_object_file *object_file);

int sky_object_file_unlock(sky_object_file *object_file);


//======================================
// Block Management
//======================================

int sky_object_file_get_block_span_count(sky_object_file *object_file, uint32_t block_index, uint32_t *span_count);


//======================================
// Event Management
//======================================

int sky_object_file_add_event(sky_object_file *object_file, sky_event *event);

#endif
