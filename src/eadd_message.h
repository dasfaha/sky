#ifndef _sky_eadd_message_h
#define _sky_eadd_message_h

#include <inttypes.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "bstring.h"
#include "table.h"
#include "event.h"


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct sky_eadd_message_data sky_eadd_message_data;

// A message for adding events to the database.
typedef struct sky_eadd_message {
    sky_object_id_t object_id;
    sky_timestamp_t timestamp;
    sky_action_id_t action_id;
    uint32_t data_count;
    sky_eadd_message_data **data;
} sky_eadd_message;

// A key/value used to store event data.
struct sky_eadd_message_data {
    bstring key;
    bstring data_type;
    union {
        bool boolean_value;
        int64_t int_value;
        double float_value;
        bstring string_value;
    };
};


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_eadd_message *sky_eadd_message_create();

sky_eadd_message_data *sky_eadd_message_data_create();

void sky_eadd_message_free(sky_eadd_message *message);

void sky_eadd_message_data_free(sky_eadd_message_data *data);


//--------------------------------------
// Serialization
//--------------------------------------

size_t sky_eadd_message_sizeof(sky_eadd_message *message);

int sky_eadd_message_pack(sky_eadd_message *message, FILE *file);

int sky_eadd_message_unpack(sky_eadd_message *message, FILE *file);

//--------------------------------------
// Processing
//--------------------------------------

int sky_eadd_message_process(sky_eadd_message *message, sky_table *table,
    FILE *output);

#endif
