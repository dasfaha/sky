#ifndef _sky_eql_event_h
#define _sky_eql_event_h

#include <inttypes.h>


//==============================================================================
//
// Definitions
//
//==============================================================================

// The event represents a state change or action at a specific point in time.
typedef struct {
    int64_t action_id;
} sky_eql_event;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_eql_event *sky_eql_event_create();

void sky_eql_event_free(sky_eql_event *event);

#endif
