#ifndef _sky_qip_cursor_h
#define _sky_qip_cursor_h

#include <inttypes.h>

#include "cursor.h"
#include "qip_event.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// The cursor iterates over events in a path.
typedef struct {
    sky_cursor *cursor;
} sky_qip_cursor;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_qip_cursor *sky_qip_cursor_create();

void sky_qip_cursor_free(sky_qip_cursor *cursor);


//======================================
// Iteration
//======================================

void sky_qip_cursor_next(sky_qip_cursor *cursor, sky_qip_event *event);

bool sky_qip_cursor_eof(sky_qip_cursor *cursor);


#endif
