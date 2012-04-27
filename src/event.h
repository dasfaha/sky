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

#define EVENT_FLAG_ACTION  1
#define EVENT_FLAG_DATA    2

#define EVENT_FLAG_LENGTH sizeof(char)
#define EVENT_TIMESTAMP_LENGTH sizeof(int64_t)
#define EVENT_HEADER_LENGTH EVENT_FLAG_LENGTH + EVENT_TIMESTAMP_LENGTH


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct Event {
    int64_t timestamp;
    int64_t object_id;
    int32_t action_id;
    uint16_t data_count;
    EventData **data;
} Event;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Create/Destroy
//======================================

Event *Event_create(int64_t timestamp, int64_t object_id, int32_t action_id);

void Event_destroy(Event *event);

int Event_copy(Event *source, Event **target);


//======================================
// Serialization
//======================================

uint32_t Event_get_serialized_length(Event *event);

int Event_serialize(Event *event, void *addr, ptrdiff_t *length);

int Event_deserialize(Event *event, void *addr, ptrdiff_t *length);


//======================================
// Data Management
//======================================

int Event_get_data(Event *event, int16_t key, EventData **data);

int Event_set_data(Event *event, int16_t key, bstring value);

int Event_unset_data(Event *event, int16_t key);


#endif
