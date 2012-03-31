#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <block.h>

#include "minunit.h"


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring foo = bsStatic("foo");
struct tagbstring bar = bsStatic("bar");
struct tagbstring baz = bsStatic("baz");


// Creates a block containing several paths.
Block *create_test_block0()
{
    Event *event;

    Block *block = Block_create();

    // Path 1
    event = Event_create(946692000000LL, 11, 0);
    Event_set_data(event, 1, &bar);
    Block_add_event(block, event);

    // Path 2
    event = Event_create(946684800000LL, 10, 6);
    Block_add_event(block, event);
    event = Event_create(946688400000LL, 10, 7);
    Event_set_data(event, 1, &foo);
    Block_add_event(block, event);
    
    return block;
}


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

char *test_Block_create() {
    Block *block = Block_create();
    mu_assert(block != NULL, "Unable to allocate block");
    Block_destroy(block);
    return NULL;
}


//--------------------------------------
// Event management
//--------------------------------------

char *test_Block_add_remove_events() {
    Block *block = create_test_block0();
    Event *event;

    // Check order of paths (based on object id).
    mu_assert(block->path_count == 2, "");
    mu_assert(block->paths[0]->object_id == 10, "");
    mu_assert(block->paths[1]->object_id == 11, "");

    // Remove event on path with single event.
    event = block->paths[1]->events[0];
    Block_remove_event(block, event);
    mu_assert(block->path_count == 1, "");
    mu_assert(block->paths[0]->object_id == 10, "");
    Event_destroy(event);
    
    // Remove event on path with multiple events.
    event = block->paths[0]->events[0];
    Block_remove_event(block, event);
    mu_assert(block->path_count == 1, "");
    mu_assert(block->paths[0]->object_id == 10, "");
    mu_assert(block->paths[0]->event_count == 1, "");
    mu_assert(block->paths[0]->events[0]->timestamp == 946688400000LL, "");
    Event_destroy(event);
    
    // Clean up.
    Block_destroy(block);

    return NULL;
}



//==============================================================================
//
// Setup
//
//==============================================================================

char *all_tests() {
    mu_run_test(test_Block_create);
    mu_run_test(test_Block_add_remove_events);

    return NULL;
}

RUN_TESTS(all_tests)