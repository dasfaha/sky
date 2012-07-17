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
#define sky_object_id_t uint32_t

// Stores an action identifier.
#define sky_action_id_t uint16_t

// Stores a property identifier.
#define sky_property_id_t int8_t


#endif
