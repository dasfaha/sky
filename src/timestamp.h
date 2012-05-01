#ifndef _timestamp_h
#define _timestamp_h

#include <inttypes.h>

#include "bstring.h"
#include "types.h"

//==============================================================================
//
// Overview
//
//==============================================================================

// Timestamps in Sky are standardized as signed 64 bit integers that represent
// the number of microseconds since the epoch (Midnight on Jan 1, 1970 UTC).
// The functions provided by the library can parse and format human readable
// ISO 8601 dates (YYYY-MM-DDTHH:MM:SSZ) to and from this format.

//==============================================================================
//
// Functions
//
//==============================================================================

int sky_timestamp_parse(bstring str, sky_timestamp_t *ret);

int sky_timestamp_now(sky_timestamp_t *ret);

#endif

