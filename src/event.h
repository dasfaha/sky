#ifndef _event_h
#define _event_h

#include <stddef.h>
#include <inttypes.h>

#include "bstring.h"
#include "event_data.h"


//==============================================================================
//
// Overview
//
//==============================================================================

/**
 * The event is the basic unit of data in a behavioral database. It is composed
 * of several elements: a timestamp, an object identifier, an action and a set
 * of key/value data. The timestamp and object identifier are always required.
 * The timestamp states when the event occurred and the object identifier states
 * who (or what) the event is related to.
 *
 * The action and data are optional but at least one of them needs to be
 * present. The action represents a verb that the object has performed. This
 * could be something like a user signing up or a product being being
 * discontinued. The data represents the state of the object. These can be
 * things like the date of birth of a user or it could be the color of a car.
 *
 * Because there is a timestamp attached to every event, this data can
 * change over time without destroying data stored in the past. That also means
 * that searches across the data will take into account the state of an object
 * at a specific point in time.
 */


//==============================================================================
//
// Definitions
//
//==============================================================================

#define sky_event_flag_t char
#define sky_timestamp_t int64_t
#define sky_object_id_t int64_t
#define sky_action_id_t int32_t
#define sky_event_data_count_t uint16_t

#define SKY_EVENT_FLAG_ACTION  1
#define SKY_EVENT_FLAG_DATA    2

#define SKY_EVENT_HEADER_LENGTH sizeof(sky_event_flag_t) + sizeof(sky_timestamp_t)


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct sky_event {
    sky_timestamp_t timestamp;
    sky_object_id_t object_id;
    sky_action_id_t action_id;
    sky_event_data_count_t data_count;
    EventData **data;
} sky_event;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Create/Destroy
//======================================

sky_event *sky_event_create(sky_timestamp_t timestamp,
                            sky_object_id_t object_id,
                            sky_action_id_t action_id);

void sky_event_free(sky_event *event);

int sky_event_copy(sky_event *source, sky_event **target);


//======================================
// Serialization
//======================================

uint32_t sky_event_get_serialized_length(sky_event *event);

int sky_event_serialize(sky_event *event, void *addr, ptrdiff_t *length);

int sky_event_deserialize(sky_event *event, void *addr, ptrdiff_t *length);


//======================================
// Data Management
//======================================

int sky_event_get_data(sky_event *event, int16_t key, EventData **data);

int sky_event_set_data(sky_event *event, int16_t key, bstring value);

int sky_event_unset_data(sky_event *event, int16_t key);


#endif
