#ifndef _path_h
#define _path_h

#include <stddef.h>
#include <inttypes.h>

#include "event.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// A path is a collection of events that is associated with an object.


//==============================================================================
//
// Constants
//
//==============================================================================

#define sky_path_event_data_length_t uint32_t

#define SKY_PATH_HEADER_LENGTH (sizeof(sky_object_id_t) + sizeof(sky_path_event_data_length_t))


//==============================================================================
//
// Definitions
//
//==============================================================================

typedef struct sky_path {
    sky_object_id_t object_id;
    uint32_t event_count;
    sky_event **events;
} sky_path;

// This structure is used for spanning blocks. It contains positional
// information about where an event in a path is located and also allows for
// placeholders and size changes to account for events being added.
typedef struct sky_path_event_stat {
    sky_timestamp_t timestamp;
    size_t start_pos;
    size_t end_pos;
    size_t sz;
} sky_path_event_stat;



//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_path *sky_path_create(sky_object_id_t object_id);

void sky_path_free(sky_path *path);


//======================================
// Serialization
//======================================

size_t sky_path_sizeof(sky_path *path);

size_t sky_path_sizeof_raw(void *ptr);

int sky_path_pack(sky_path *path, void *addr, size_t *length);

int sky_path_pack_hdr(sky_object_id_t object_id, uint32_t event_data_length,
    void *addr, size_t *length);

int sky_path_unpack(sky_path *path, void *addr, size_t *length);

int sky_path_unpack_hdr(sky_object_id_t *object_id, uint32_t *event_data_length,
    void *addr, size_t *length);


//======================================
// Stats
//======================================

int sky_path_get_event_stats(void *path_ptr, sky_event *event,
    sky_path_event_stat **events, uint32_t *event_count);


//======================================
// Event Management
//======================================

int sky_path_add_event(sky_path *path, sky_event *event);

int sky_path_remove_event(sky_path *path, sky_event *event);

#endif
