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

size_t DATA_LENGTH = 62;
char DATA[] = 
    "\x02\x00\x00\x00\x36\x00\x00\x00\x01\x1a\x00\x00\x00\x00\x00\x00"
    "\x00\x06\x00\x03\x1b\x00\x00\x00\x00\x00\x00\x00\x07\x00\x0a\x00"
    "\x00\x00\x01\xa3\x66\x6f\x6f\x02\xa3\x62\x61\x72\x02\x1c\x00\x00"
    "\x00\x00\x00\x00\x00\x05\x00\x00\x00\x01\xa3\x66\x6f\x6f"
;


// Creates a path containing action-only, data-only and action+data events.
sky_path *create_test_path0()
{
    sky_event *event;
    sky_path *path = sky_path_create(2);

    // Action-only event
    event = sky_event_create(26LL, 2, 6);
    sky_path_add_event(path, event);

    // Action+data event
    event = sky_event_create(27LL, 2, 7);
    sky_event_set_data(event, 1, &foo);
    sky_event_set_data(event, 2, &bar);
    sky_path_add_event(path, event);

    // Data-only event
    event = sky_event_create(28LL, 2, 0);
    sky_event_set_data(event, 1, &foo);
    sky_path_add_event(path, event);

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

int test_sky_path_create() {
    sky_path *path = sky_path_create(2);
    mu_assert(path != NULL, "Unable to allocate path");
    mu_assert(path->object_id == 2, "Path object id not assigned");
    sky_path_free(path);

    return 0;
}


//--------------------------------------
// Event management
//--------------------------------------

int test_sky_path_add_remove_event() {
    sky_path *path = sky_path_create(2);

    sky_event *event0 = sky_event_create(27LL, 2, 11);
    sky_path_add_event(path, event0);
    sky_event *event1 = sky_event_create(26LL, 2, 10);
    sky_path_add_event(path, event1);
    
    // Check order of events (e0 should be after e1 based on timestamp).
    mu_assert(path->event_count == 2, "");
    mu_assert(path->events[0] == event1, "");
    mu_assert(path->events[1] == event0, "");

    // Remove event.
    sky_path_remove_event(path, event1);
    mu_assert(path->event_count == 1, "");
    mu_assert(path->events[0] == event0, "");
    
    // Clean up.
    sky_event_free(event1);
    sky_path_free(path);

    return 0;
}


//--------------------------------------
// Serialization length
//--------------------------------------

int test_sky_path_sizeof() {
    sky_path *path = create_test_path0();
    size_t sz = sky_path_sizeof(path);
    mu_assert_long_equals(sz, DATA_LENGTH);
    sky_path_free(path);
    return 0;
}


//--------------------------------------
// Serialization
//--------------------------------------

int test_sky_path_pack() {
    size_t sz;
    void *addr = calloc(DATA_LENGTH, 1);
    sky_path *path = create_test_path0();
    sky_path_pack(path, addr, &sz);
    sky_path_free(path);
    mu_assert_long_equals(sz, DATA_LENGTH);
    mu_assert_mem(addr, &DATA, DATA_LENGTH);
    free(addr);
    return 0;
}


//--------------------------------------
// Deserialization
//--------------------------------------

int test_sky_path_unpack() {
    size_t sz;

    sky_event_data *data;
    sky_path *path = sky_path_create(0);
    sky_path_unpack(path, &DATA, &sz);

    mu_assert_long_equals(sz, DATA_LENGTH);

    mu_assert(path->object_id == 2, "");
    mu_assert(path->events != NULL, "");
    mu_assert(path->event_count == 3, "");

    mu_assert(path->events[0]->timestamp == 26LL, "");
    mu_assert(path->events[0]->object_id == 2, "");
    mu_assert(path->events[0]->action_id == 6, "");

    mu_assert(path->events[1]->timestamp == 27LL, "");
    mu_assert(path->events[1]->object_id == 2, "");
    mu_assert(path->events[1]->action_id == 7, "");

    sky_event_get_data(path->events[1], 1, &data);
    mu_assert(biseqcstr(data->value, "foo"), "");
    sky_event_get_data(path->events[1], 2, &data);
    mu_assert(biseqcstr(data->value, "bar"), "");

    mu_assert(path->events[2]->timestamp == 28LL, "");
    mu_assert(path->events[2]->object_id == 2, "");
    mu_assert(path->events[2]->action_id == 0, "");

    sky_event_get_data(path->events[2], 1, &data);
    mu_assert(biseqcstr(data->value, "foo"), "");

    sky_path_free(path);
    
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_path_create);
    mu_run_test(test_sky_path_add_remove_event);
    mu_run_test(test_sky_path_sizeof);
    mu_run_test(test_sky_path_pack);
    mu_run_test(test_sky_path_unpack);

    return 0;
}

RUN_TESTS()