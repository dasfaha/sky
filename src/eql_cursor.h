#ifndef _sky_eql_cursor_h
#define _sky_eql_cursor_h

#include <inttypes.h>

#include "cursor.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// The cursor iterates over events in a path.
typedef struct {
    sky_cursor *cursor;
} sky_eql_cursor;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_eql_cursor *sky_eql_cursor_create();

void sky_eql_cursor_free(sky_eql_cursor *cursor);

#endif
