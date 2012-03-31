#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <path.h>

#include "minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

char *test_Path_create() {
    Path *path = Path_create(10);
    mu_assert(path != NULL, "Unable to allocate path");
    mu_assert(path->object_id == 10, "Path object id not assigned");
    Path_destroy(path);

    return NULL;
}


//--------------------------------------
// Event management
//--------------------------------------

char *test_Path_add_remove_event() {
    Path *path = Path_create(10);

    Event *event0 = Event_create(1325377000000LL, 10, 200);
    Path_add_event(path, event0);
    Event *event1 = Event_create(1325376000000LL, 10, 200);
    Path_add_event(path, event1);
    
    // Check order of events (e1 should be after e0 based on timestamp).
    mu_assert(path->event_count == 2, "");
    mu_assert(path->events[0] == event1, "");
    mu_assert(path->events[1] == event0, "");

    // Remove event.
    Path_remove_event(path, event1);
    mu_assert(path->event_count == 1, "");
    mu_assert(path->events[0] == event0, "");
    
    // Clean up.
    Event_destroy(event1);
    Path_destroy(path);

    return NULL;
}



//==============================================================================
//
// Setup
//
//==============================================================================

char *all_tests() {
    mu_run_test(test_Path_create);
    mu_run_test(test_Path_add_remove_event);

    return NULL;
}

RUN_TESTS(all_tests)