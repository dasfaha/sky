#ifndef _event_data_h
#define _event_data_h

#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>

#include "bstring.h"
#include "types.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

typedef struct sky_event_data {
    sky_property_id_t key;
    bstring data_type;
    union {
        bool boolean_value;
        int64_t int_value;
        double float_value;
        bstring string_value;
    };
} sky_event_data;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_event_data *sky_event_data_create(sky_property_id_t key);

sky_event_data *sky_event_data_create_int(sky_property_id_t key, int64_t value);

sky_event_data *sky_event_data_create_float(sky_property_id_t key, double value);

sky_event_data *sky_event_data_create_boolean(sky_property_id_t key, bool value);

sky_event_data *sky_event_data_create_string(sky_property_id_t key, bstring value);

void sky_event_data_free(sky_event_data *event);

int sky_event_data_copy(sky_event_data *source, sky_event_data **target);


//--------------------------------------
// Serialization
//--------------------------------------

size_t sky_event_data_sizeof(sky_event_data *data);

int sky_event_data_pack(sky_event_data *data, void *addr, size_t *length);

int sky_event_data_unpack(sky_event_data *data, void *addr, size_t *length);


#endif
