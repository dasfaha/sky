#ifndef _action_h
#define _action_h

#include <inttypes.h>
#include <stdbool.h>

typedef struct sky_action sky_action;

#include "bstring.h"
#include "file.h"
#include "action_file.h"

//==============================================================================
//
// Overview
//
//==============================================================================

// An action defines a type of verb associated with an event. For example,
// viewing a certain web page (e.g. /index.html) for a user would be
// considered an action.


//==============================================================================
//
// Typedefs
//
//==============================================================================

// An action defines a verb that is performed in an event.
struct sky_action {
    sky_action_file *action_file;
    sky_action_id_t id;
    bstring name;
};



//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_action *sky_action_create();

void sky_action_free(sky_action *action);


#endif
