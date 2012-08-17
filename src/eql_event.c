#include <stdlib.h>

#include "eql_event.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates an event.
sky_eql_event *sky_eql_event_create()
{
    sky_eql_event *event = malloc(sizeof(sky_eql_event));
    event->action_id = 0LL;
    return event;
}

// Frees an event.
//
// event - The event to free.
void sky_eql_event_free(sky_eql_event *event)
{
    if(event) {
        free(event);
    }
}

