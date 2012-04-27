#ifndef _event_data_h
#define _event_data_h

#include <stddef.h>
#include <inttypes.h>

#include "bstring.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// Event data is a simple hash of keys and values. Keys are stored as the id
// of the property they represent. Additional property information such as the
// property name is stored globally in the object file.


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct EventData {
    int16_t key;
    bstring value;
} EventData;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

EventData *EventData_create(int16_t key, bstring value);

void EventData_destroy(EventData *event);

int EventData_copy(EventData *source, EventData **target);


//======================================
// Serialization
//======================================

uint32_t EventData_get_serialized_length(EventData *data);

int EventData_serialize(EventData *data, void *addr, ptrdiff_t *length);

int EventData_deserialize(EventData *data, void *addr, ptrdiff_t *length);


#endif
