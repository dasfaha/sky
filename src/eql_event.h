#ifndef _sky_qip_event_h
#define _sky_qip_event_h

#include <inttypes.h>


//==============================================================================
//
// Definitions
//
//==============================================================================

// The event represents a state change or action at a specific point in time.
typedef struct {
    int64_t action_id;
} sky_qip_event;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_qip_event *sky_qip_event_create();

void sky_qip_event_free(sky_qip_event *event);

#endif
