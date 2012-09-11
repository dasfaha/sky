#ifndef _event_data_h
#define _event_data_h

#include <stddef.h>
#include <inttypes.h>

#include "bstring.h"
#include "types.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// Event data is a simple hash of keys and values. Keys are stored as the id
// of the property they represent. Additional property information such as the
// property name is stored globally in the table.


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct sky_event_data {
    sky_property_id_t key;
    bstring value;
} sky_event_data;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_event_data *sky_event_data_create(sky_property_id_t key, bstring value);

void sky_event_data_free(sky_event_data *event);

int sky_event_data_copy(sky_event_data *source, sky_event_data **target);


//--------------------------------------
// Serialization
//--------------------------------------

size_t sky_event_data_sizeof(sky_event_data *data);

int sky_event_data_pack(sky_event_data *data, void *addr, size_t *length);

int sky_event_data_unpack(sky_event_data *data, void *addr, size_t *length);


#endif
