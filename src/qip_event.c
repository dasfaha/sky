#include <stdlib.h>

#include "qip_event.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an event.
sky_qip_event *sky_qip_event_create()
{
    sky_qip_event *event = malloc(sizeof(sky_qip_event));
    event->action_id = 0LL;
    return event;
}

// Frees an event.
//
// event - The event to free.
void sky_qip_event_free(sky_qip_event *event)
{
    if(event) {
        free(event);
    }
}

