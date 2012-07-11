#include <stdlib.h>
#include <inttypes.h>

#include "dbg.h"
#include "mem.h"
#include "bstring.h"
#include "file.h"
#include "action_file.h"
#include "minipack.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a reference to an action file.
// 
// Returns a reference to the new action file if successful. Otherwise returns
// null.
sky_action_file *sky_action_file_create()
{
    sky_action_file *action_file = calloc(sizeof(sky_action_file), 1);
    check_mem(action_file);
    return action_file;
    
error:
    sky_action_file_free(action_file);
    return NULL;
}

// Removes an action file reference from memory.
//
// action_file - The action file to free.
void sky_action_file_free(sky_action_file *action_file)
{
    if(action_file) {
        if(action_file->path) bdestroy(action_file->path);
        action_file->path = NULL;
        sky_action_file_unload(action_file);
        free(action_file);
    }
}


//======================================
// Path
//======================================

// Retrieves the file path of an action file.
//
// action_file - The action file.
// path        - A pointer to where the path should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int sky_action_file_get_path(sky_action_file *action_file, bstring *path)
{
    check(action_file != NULL, "Action file required");

    *path = bstrcpy(action_file->path);
    if(action_file->path) check_mem(*path);

    return 0;

error:
    *path = NULL;
    return -1;
}

// Sets the file path of an action file.
//
// action_file - The action file.
// path        - The file path to set.
//
// Returns 0 if successful, otherwise returns -1.
int sky_action_file_set_path(sky_action_file *action_file, bstring path)
{
    check(action_file != NULL, "Action file required");

    if(action_file->path) {
        bdestroy(action_file->path);
    }
    
    action_file->path = bstrcpy(path);
    if(path) check_mem(action_file->path);

    return 0;

error:
    action_file->path = NULL;
    return -1;
}


//======================================
// Persistence
//======================================

// Loads actions from file.
//
// action_file - The action file to load.
//
// Returns 0 if successful, otherwise returns -1.
int sky_action_file_load(sky_action_file *action_file)
{
    FILE *file;
    sky_action **actions = NULL;
    uint32_t count = 0;
    size_t sz;

    int rc;
    check(action_file != NULL, "Action file required");
    check(action_file->path != NULL, "Action file path required");

    // Unload any actions currently in memory.
    rc = sky_action_file_unload(action_file);
    check(rc == 0, "Unable to unload action file");

    // Read in actions file if it exists.
    if(sky_file_exists(action_file->path)) {
        file = fopen(bdata(action_file->path), "r");
        check(file, "Failed to open action file: %s",  bdata(action_file->path));

        // Read action count.
        count = minipack_fread_array(file, &sz);
        check(sz != 0, "Unable to read actions array at byte: %ld", ftell(file));

        // Allocate actions.
        actions = malloc(sizeof(sky_action*) * count);
        if(count > 0) check_mem(actions);

        // Read actions.
        uint32_t i;
        for(i=0; i<count; i++) {
            sky_action *action = sky_action_create();
            check_mem(action);

            // Read action id.
            action->id = minipack_fread_int(file, &sz);
            check(sz != 0, "Unable to read action identifier at byte: %ld", ftell(file));

            // Read action name.
            rc = sky_minipack_fread_bstring(file, &action->name);
            check(rc == 0, "Unable to read action name at byte: %ld", ftell(file));

            // Append to array.
            action->action_file = action_file;
            actions[i] = action;
        }

        // Close the file.
        fclose(file);
    }

    // Store action list on action file.
    action_file->actions = actions;
    action_file->action_count = count;

    return 0;

error:
    if(file) fclose(file);
    return -1;
}

// Saves actions to file.
//
// action_file - The action file to save.
//
// Returns 0 if successful, otherwise returns -1.
int sky_action_file_save(sky_action_file *action_file)
{
    FILE *file;
    size_t sz;

    int rc;
    check(action_file != NULL, "Action file required");
    check(action_file->path != NULL, "Action file path required");

    // Open file.
    file = fopen(bdata(action_file->path), "w");
    check(file, "Failed to open action file: %s", bdata(action_file->path));

    // Write action count.
    rc = minipack_fwrite_array(file, action_file->action_count, &sz);
    check(rc == 0, "Unable to write actions array at byte: %ld", ftell(file));

    // Write actions.
    uint32_t i;
    for(i=0; i<action_file->action_count; i++) {
        sky_action *action = action_file->actions[i];

        // Write action id.
        minipack_fwrite_int(file, action->id, &sz);
        check(sz != 0, "Unable to write action identifier at byte: %ld", ftell(file));

        // Write action name.
        rc = sky_minipack_fwrite_bstring(file, action->name);
        check(rc == 0, "Unable to write action name at byte: %ld", ftell(file));
    }

    // Close the file.
    fclose(file);

    return 0;

error:
    if(file) fclose(file);
    return -1;
}

// Unloads the actions in the action file from memory.
//
// action_file - The action file to save.
//
// Returns 0 if successful, otherwise returns -1.
int sky_action_file_unload(sky_action_file *action_file)
{
    if(action_file) {
        // Release actions.
        if(action_file->actions) {
            uint32_t i=0;
            for(i=0; i<action_file->action_count; i++) {
                sky_action *action = action_file->actions[i];
                sky_action_free(action);
                action_file->actions[i] = NULL;
            }
            free(action_file->actions);
            action_file->actions = NULL;
        }
        
        action_file->action_count = 0;
    }
    
    return 0;
}


//======================================
// Action Management
//======================================

// Retrieves the id for an action with a given name.
//
// action_file - The action file to search.
// name        - The name of the action.
// ret         - A pointer to where the action should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_action_file_find_action_by_name(sky_action_file *action_file,
                                        bstring name, sky_action **ret)
{
    check(action_file != NULL, "Action file required");
    check(name != NULL, "Action name required");
    
    // Initialize action id to zero.
    *ret = NULL;
    
    // Loop over actions to find matching name.
    uint32_t i;
    for(i=0; i<action_file->action_count; i++) {
        if(biseq(action_file->actions[i]->name, name) == 1) {
            *ret = action_file->actions[i];
            break;
        }
    }
    
    return 0;

error:
    *ret = NULL;
    return -1;
}


// Adds an action to an action file.
//
// action_file - The action file to add the action to.
// action      - The action to add to the action file.
//
// Returns 0 if successful, otherwise returns -1.
int sky_action_file_add_action(sky_action_file *action_file,
                               sky_action *action)
{
    check(action_file != NULL, "Action file required");
    check(action != NULL, "Action required");
    check(action->id == 0, "Action ID must be zero");
    check(action->action_file == NULL, "Action ID must be zero");
    
    // Make sure an action with that name doesn't already exist.
    sky_action *_action;
    int rc = sky_action_file_find_action_by_name(action_file, action->name, &_action);
    check(rc == 0 && _action == NULL, "Action already exists with the same name");
    
    // Link to action file.
    action->action_file = action_file;

    // Increment action identifier.
    if(action_file->action_count == 0) {
        action->id = 1;
    }
    else {
        action->id = action_file->actions[action_file->action_count-1]->id + 1;
    }
    
    // Append action to list.
    action_file->action_count++;
    action_file->actions = realloc(action_file->actions, sizeof(sky_action*) * action_file->action_count);
    check_mem(action_file->actions);
    action_file->actions[action_file->action_count-1] = action;
    
    return 0;

error:
    return -1;
}

