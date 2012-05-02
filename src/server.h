#ifndef _server_h
#define _server_h

#include <inttypes.h>
#include <stdbool.h>

#include "bstring.h"
#include "database.h"
#include "object_file.h"
#include "event.h"
#include "dbg.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// The server acts as the interface to external applications. It communicates
// over TCP sockets using a specific Sky protocol. See the message.h file for
// more detail on the protocol.


//==============================================================================
//
// Definitions
//
//==============================================================================


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
