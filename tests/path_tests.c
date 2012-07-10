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

size_t DATA_LENGTH = 69;
char DATA[] = {
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x39, 0x00, 0x00, 0x00, 0x01, 0x00, 0xE0, 0x37,
    0x3B, 0x01, 0x5D, 0x03, 0x00, 0x06, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x84, 0xCB, 0x11, 0x02, 0x5D,
    0x03, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0C, 0x00,
    0x01, 0x00, 0x03, 0x66, 0x6F, 0x6F, 0x02, 0x00,
    0x03, 0x62, 0x61, 0x72, 0x02, 0x00, 0x28, 0x5F,
    0xE8, 0x02, 0x5D, 0x03, 0x00, 0x06, 0x00, 0x01,
    0x00, 0x03, 0x66, 0x6F, 0x6F
};


// Creates a path containing action-only, data-only and action+data events.
sky_path *create_test_path0()
{
    sky_event *event;
    sky_path *path = sky_path_create(10);

    // Action-only event
    event = sky_event_create(946684800000000LL, 10, 6);
    sky_path_add_event(path, event);

    // Action+data event
    event = sky_event_create(946688400000000LL, 10, 7);
    sky_event_set_data(event, 1, &foo);
    sky_event_set_data(event, 2, &bar);
    sky_path_add_event(path, event);

    // Data-only event
    event = sky_event_create(946692000000000LL, 10, 0);
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
    sky_path *path = sky_path_create(10);
    mu_assert(path != NULL, "Unable to allocate path");
    mu_assert(path->object_id == 10, "Path object id not assigned");
    sky_path_free(path);

    return 0;
}


//--------------------------------------
// Event management
//--------------------------------------

int test_sky_path_add_remove_event() {
    sky_path *path = sky_path_create(10);

    sky_event *event0 = sky_event_create(1325377000000LL, 10, 200);
    sky_path_add_event(path, event0);
    sky_event *event1 = sky_event_create(1325376000000LL, 10, 200);
    sky_path_add_event(path, event1);
    
    // Check order of events (e1 should be after e0 based on timestamp).
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
    mu_assert(sky_path_sizeof(path) == 69, "");
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
    mu_assert(sz == DATA_LENGTH, "");
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

    mu_assert(sz == DATA_LENGTH, "");

    mu_assert(path->object_id == 10, "");
    mu_assert(path->events != NULL, "");
    mu_assert(path->event_count == 3, "");

    mu_assert(path->events[0]->timestamp == 946684800000000LL, "");
    mu_assert(path->events[0]->object_id == 10, "");
    mu_assert(path->events[0]->action_id == 6, "");

    mu_assert(path->events[1]->timestamp == 946688400000000LL, "");
    mu_assert(path->events[1]->object_id == 10, "");
    mu_assert(path->events[1]->action_id == 7, "");

    sky_event_get_data(path->events[1], 1, &data);
    mu_assert(biseqcstr(data->value, "foo"), "");
    sky_event_get_data(path->events[1], 2, &data);
    mu_assert(biseqcstr(data->value, "bar"), "");

    mu_assert(path->events[2]->timestamp == 946692000000000LL, "");
    mu_assert(path->events[2]->object_id == 10, "");
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