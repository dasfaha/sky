#include <stdlib.h>
#include <inttypes.h>

#include "dbg.h"
#include "bstring.h"
#include "action.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a reference to an action.
// 
// Returns a reference to the new action if successful.
sky_action *sky_action_create()
{
    sky_action *action = calloc(sizeof(sky_action), 1);
    check_mem(action);
    return action;
    
error:
    sky_action_free(action);
    return NULL;
}

// Removes an action reference from memory.
//
// action - The action to free.
void sky_action_free(sky_action *action)
{
    if(action) {
        action->action_file = NULL;
        action->id = 0;
        if(action->name) bdestroy(action->name);
        action->name = NULL;
        free(action);
    }
}


