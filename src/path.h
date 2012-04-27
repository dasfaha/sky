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

//
// A path is a collection of events that is associated with an object.
//


//==============================================================================
//
// Constants
//
//==============================================================================

// The length of non-event data bytes in a serialized path.
#define PATH_HEADER_LENGTH sizeof(int64_t) + sizeof(uint32_t)


//==============================================================================
//
// Typedefs
//
//==============================================================================

/**
 * The path stores an array of events.
 */
typedef struct Path {
    int64_t object_id;
    uint32_t event_count;
    Event **events;
} Path;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

Path *Path_create(int64_t object_id);

void Path_destroy(Path *path);


//======================================
// Serialization
//======================================

uint32_t Path_get_serialized_length(Path *path);

uint32_t Path_get_length(const void *ptr);

int Path_serialize(Path *path, void *addr, ptrdiff_t *length);

int Path_deserialize(Path *path, void *addr, ptrdiff_t *length);


//======================================
// Event Management
//======================================

int Path_add_event(Path *path, Event *event);

int Path_remove_event(Path *path, Event *event);

#endif
