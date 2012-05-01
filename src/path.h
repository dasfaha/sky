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

#define sky_path_event_count_t uint32_t
#define sky_path_events_length_t uint32_t

// The length of non-event data bytes in a serialized path.
#define SKY_PATH_HEADER_LENGTH sizeof(sky_object_id_t) + sizeof(sky_path_event_count_t)


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct sky_path {
    sky_object_id_t object_id;
    sky_path_event_count_t event_count;
    sky_event **events;
} sky_path;


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

uint32_t sky_path_get_serialized_length(sky_path *path);

uint32_t sky_path_get_length(const void *ptr);

int sky_path_serialize(sky_path *path, void *addr, ptrdiff_t *length);

int sky_path_deserialize(sky_path *path, void *addr, ptrdiff_t *length);


//======================================
// Event Management
//======================================

int sky_path_add_event(sky_path *path, sky_event *event);

int sky_path_remove_event(sky_path *path, sky_event *event);

#endif
