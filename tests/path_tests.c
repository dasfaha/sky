#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <path.h>

#include "minunit.h"


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring foo = bsStatic("foo");
struct tagbstring bar = bsStatic("bar");
struct tagbstring baz = bsStatic("baz");


// Creates a path containing action-only, data-only and action+data events.
Path *create_test_path0()
{
    Event *event;
    Path *path = Path_create(10);

    // Action-only event
    event = Event_create(946684800000LL, 10, 6);
    Path_add_event(path, event);

    // Action+data event
    event = Event_create(946688400000LL, 10, 7);
    Event_set_data(event, 1, &foo);
    Event_set_data(event, 2, &bar);
    Path_add_event(path, event);

    // Data-only event
    event = Event_create(946692000000LL, 10, 0);
    Event_set_data(event, 1, &foo);
    Path_add_event(path, event);

    return path;
}


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


//--------------------------------------
// Serialization length
//--------------------------------------

char *test_Path_get_serialized_length() {
    // Create path.
    Path *path = create_test_path0();
    
    // Verify
    mu_assert(Path_get_serialized_length(path) == 69, "");

    Path_destroy(path);

    return NULL;
}


//--------------------------------------
// Serialization
//--------------------------------------

char *test_Path_serialize() {
    FILE *file = fopen(TEMPFILE, "w");
    Path *path = create_test_path0();
    Path_serialize(path, file);
    Path_destroy(path);
    fclose(file);
    mu_assert_tempfile("tests/fixtures/serialization/path", "Serialize Path");
    return NULL;
}


//--------------------------------------
// Deserialization
//--------------------------------------

char *test_Path_deserialize() {
    EventData *data;
    FILE *file = fopen("tests/fixtures/serialization/path", "r");
    Path *path = Path_create(0);
    Path_deserialize(path, file);
    fclose(file);

    mu_assert(path->object_id == 10, "");
    mu_assert(path->events != NULL, "");
    mu_assert(path->event_count == 3, "");

    mu_assert(path->events[0]->timestamp == 946684800000LL, "");
    mu_assert(path->events[0]->object_id == 10, "");
    mu_assert(path->events[0]->action_id == 6, "");

    mu_assert(path->events[1]->timestamp == 946688400000LL, "");
    mu_assert(path->events[1]->object_id == 10, "");
    mu_assert(path->events[1]->action_id == 7, "");

    Event_get_data(path->events[1], 1, &data);
    mu_assert(biseqcstr(data->value, "foo"), "");
    Event_get_data(path->events[1], 2, &data);
    mu_assert(biseqcstr(data->value, "bar"), "");

    mu_assert(path->events[2]->timestamp == 946692000000LL, "");
    mu_assert(path->events[2]->object_id == 10, "");
    mu_assert(path->events[2]->action_id == 0, "");

    Event_get_data(path->events[2], 1, &data);
    mu_assert(biseqcstr(data->value, "foo"), "");

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
    mu_run_test(test_Path_get_serialized_length);
    mu_run_test(test_Path_serialize);
    mu_run_test(test_Path_deserialize);

    return NULL;
}

RUN_TESTS(all_tests)