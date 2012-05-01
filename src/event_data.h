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
// Definitions
//
//==============================================================================

#define sky_event_data_key_t int16_t


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct sky_event_data {
    sky_event_data_key_t key;
    bstring value;
} sky_event_data;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_event_data *sky_event_data_create(sky_event_data_key_t key, bstring value);

void sky_event_data_free(sky_event_data *event);

int sky_event_data_copy(sky_event_data *source, sky_event_data **target);


//======================================
// Serialization
//======================================

uint32_t sky_event_data_get_serialized_length(sky_event_data *data);

int sky_event_data_serialize(sky_event_data *data, void *addr, ptrdiff_t *length);

int sky_event_data_deserialize(sky_event_data *data, void *addr, ptrdiff_t *length);


#endif
