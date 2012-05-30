#ifndef _types_h
#define _types_h

#include <inttypes.h>

#include "bstring.h"

//==============================================================================
//
// Overview
//
//==============================================================================

// This header file includes definitions of the most basic types used in the Sky
// database.


//==============================================================================
//
// Typedefs
//
//==============================================================================

// Stores the number of microseconds since the epoch (Jan 1, 1970 UTC).
#define sky_timestamp_t int64_t

// Stores an object identifier.
#define sky_object_id_t int64_t

// Stores an action identifier.
#define sky_action_id_t int32_t

// Stores a property identifier.
#define sky_property_id_t int16_t

// An action defines a verb that is performed in an event. 4 billion (2^32)
// unique types of actions can be defined within an table. The name of the
// action is stored in the 'actions' file and the action identifier is used when
// storing event data in a block.
typedef struct sky_action {
    sky_action_id_t id;
    bstring name;
} sky_action;

// A property is a key used on data in an event. The property identifier's range
// is split up: positive ids are used for data attached to an object, negative
// ids are used for data attached to an action and id 0 is reserved.
//
// Property identifiers are used when storing events in blocks because of their
// redundancy. Property definitions are stored in the 'properties' file.
typedef struct sky_property {
    sky_property_id_t id;
    bstring name;
} sky_property;

#endif
