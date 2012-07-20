#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <event.h>
#include <mem.h>

#include "minunit.h"

//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring foo = bsStatic("foo");
struct tagbstring bar = bsStatic("bar");
struct tagbstring baz = bsStatic("baz");

size_t ACTION_EVENT_DATA_LENGTH = 11;
char ACTION_EVENT_DATA[] = 
    "\x01\x1e\x00\x00\x00\x00\x00\x00\x00\x14\x00"
;

size_t DATA_EVENT_DATA_LENGTH = 23;
char DATA_EVENT_DATA[] = 
    "\x02\x1e\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x01\xa3\x66"
    "\x6f\x6f\x02\xa3\x62\x61\x72"
;

size_t ACTION_DATA_EVENT_DATA_LENGTH = 25;
char ACTION_DATA_EVENT_DATA[] = 
    "\x03\x1e\x00\x00\x00\x00\x00\x00\x00\x14\x00\x0a\x00\x00\x00\x01"
    "\xa3\x66\x6f\x6f\x02\xa3\x62\x61\x72"
;


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int test_sky_event_create() {
    sky_event *event = sky_event_create(10, 1325376000000LL, 200);
    mu_assert(event != NULL, "Unable to allocate event");
    mu_assert(event->timestamp == 1325376000000LL, "Event timestamp not assigned");
    mu_assert(event->object_id == 10, "Event object id not assigned");
    mu_assert(event->action_id == 200, "Event action id not assigned");
    mu_assert(event->data == NULL, "Event data non-null");
    mu_assert(event->data_count == 0, "Event data count not initialized");

    sky_event_free(event);

    return 0;
}


//--------------------------------------
// Event data
//--------------------------------------

int test_sky_event_set_data() {
    sky_event_data *data = NULL;

    sky_event *event = sky_event_create(0, 0, 0);
    sky_event_set_data(event, 10, &foo);
    sky_event_set_data(event, 20, &bar);
    sky_event_set_data(event, 10, &baz);
    mu_assert(event->data_count == 2, "Expected data count to be 2");

    sky_event_get_data(event, 10, &data);
    mu_assert(biseqcstr(data->value, "baz"), "Expected data 10 to equal 'baz'");
    sky_event_get_data(event, 20, &data);
    mu_assert(biseqcstr(data->value, "bar"), "Expected data 20 to equal 'bar'");
    sky_event_get_data(event, 30, &data);
    mu_assert(data == NULL, "Expected data 30 be NULL");

    sky_event_free(event);

    return 0;
}

int test_sky_event_unset_data() {
    sky_event_data *data = NULL;

    sky_event *event = sky_event_create(0, 0, 0);
    sky_event_set_data(event, 10, &foo);
    sky_event_set_data(event, 20, &bar);
    mu_assert(event->data_count == 2, "Expected data count to be 2");
    sky_event_unset_data(event, 10);
    mu_assert(event->data_count == 1, "Expected data count to be 1");

    sky_event_get_data(event, 10, &data);
    mu_assert(data == NULL, "Expected data 10 be NULL");
    sky_event_get_data(event, 20, &data);
    mu_assert(biseqcstr(data->value, "bar"), "Expected data 20 to equal 'bar'");

    sky_event_free(event);

    return 0;
}


//--------------------------------------
// Serialization length
//--------------------------------------

// Action-only event.
int test_sky_event_action_event_sizeof() {
    sky_event *event = sky_event_create(0, 1325376000000LL, 20);
    size_t sz = sky_event_sizeof(event);
    mu_assert_long_equals(sz, 13L);
    sky_event_free(event);
    return 0;
}

// Data-only event.
int test_sky_event_data_event_sizeof() {
    sky_event *event = sky_event_create(0, 1325376000000LL, 0);
    sky_event_set_data(event, 1, &foo);
    sky_event_set_data(event, 2, &bar);
    size_t sz = sky_event_sizeof(event);
    mu_assert_long_equals(sz, 23L);
    sky_event_free(event);
    return 0;
}

// Action + data event.
int test_sky_event_action_data_event_sizeof() {
    sky_event *event = sky_event_create(0, 1325376000000LL, 100);
    sky_event_set_data(event, 1, &foo);
    sky_event_set_data(event, 2, &bar);
    size_t sz = sky_event_sizeof(event);
    mu_assert_long_equals(sz, 27L);
    sky_event_free(event);
    return 0;
}

//--------------------------------------
// Serialization
//--------------------------------------

// Action event.
int test_sky_event_action_event_pack() {
    size_t sz;
    void *addr = calloc(ACTION_EVENT_DATA_LENGTH, 1);
    sky_event *event = sky_event_create(0, 30LL, 20);
    sky_event_pack(event, addr, &sz);
    sky_event_free(event);
    mu_assert_long_equals(sz, ACTION_EVENT_DATA_LENGTH);
    mu_assert_mem(addr, &ACTION_EVENT_DATA, ACTION_EVENT_DATA_LENGTH);
    free(addr);
    return 0;
}

// Data event.
int test_sky_event_data_event_pack() {
    size_t sz;
    void *addr = calloc(DATA_EVENT_DATA_LENGTH, 1);
    sky_event *event = sky_event_create(0, 30LL, 0);
    sky_event_set_data(event, 1, &foo);
    sky_event_set_data(event, 2, &bar);
    sky_event_pack(event, addr, &sz);
    sky_event_free(event);
    mu_assert_long_equals(sz, DATA_EVENT_DATA_LENGTH);
    mu_assert_mem(addr, &DATA_EVENT_DATA, DATA_EVENT_DATA_LENGTH);
    free(addr);
    return 0;
}

// Action+Data event.
int test_sky_event_action_data_event_pack() {
    size_t sz;
    void *addr = calloc(ACTION_DATA_EVENT_DATA_LENGTH, 1);
    sky_event *event = sky_event_create(0, 30LL, 20);
    sky_event_set_data(event, 1, &foo);
    sky_event_set_data(event, 2, &bar);
    sky_event_pack(event, addr, &sz);
    sky_event_free(event);
    mu_assert_long_equals(sz, ACTION_DATA_EVENT_DATA_LENGTH);
    mu_assert_mem(addr, &ACTION_DATA_EVENT_DATA, ACTION_DATA_EVENT_DATA_LENGTH);
    free(addr);
    return 0;
}

//--------------------------------------
// Deserialization
//--------------------------------------

// Action event.
int test_sky_event_action_event_unpack() {
    size_t sz;
    sky_event *event = sky_event_create(0, 0, 0);
    sky_event_unpack(event, &ACTION_EVENT_DATA, &sz);

    mu_assert_long_equals(sz, ACTION_EVENT_DATA_LENGTH);
    mu_assert_int64_equals(event->timestamp, 30LL);
    mu_assert(event->action_id == 20, "Expected action id to equal 20");
    mu_assert(event->object_id == 0, "Expected object id to equal 0");
    mu_assert(event->data == NULL, "Expected data to be NULL");
    mu_assert(event->data_count == 0, "Expected data count to be 0");

    sky_event_free(event);
    
    return 0;
}

// Data event.
int test_sky_event_data_event_unpack() {
    size_t sz;
    sky_event *event = sky_event_create(0, 0, 0);
    sky_event_unpack(event, &DATA_EVENT_DATA, &sz);

    mu_assert_long_equals(sz, DATA_EVENT_DATA_LENGTH);
    mu_assert_int64_equals(event->timestamp, 30LL);
    mu_assert(event->action_id == 0, "Expected action id to equal 0");
    mu_assert(event->object_id == 0, "Expected object id to equal 0");
    mu_assert(event->data != NULL, "Expected data to not be NULL");
    mu_assert(event->data_count == 2, "Expected data count to be 2");

    sky_event_data *data = NULL;
    sky_event_get_data(event, 1, &data);
    mu_assert(biseqcstr(data->value, "foo"), "Expected data 1 to equal 'foo'");
    sky_event_get_data(event, 2, &data);
    mu_assert(biseqcstr(data->value, "bar"), "Expected data 1 to equal 'bar'");

    sky_event_free(event);
    
    return 0;
}

// Action+Data event.
int test_sky_event_action_data_event_unpack() {
    size_t sz;
    sky_event *event = sky_event_create(0, 0, 0);
    sky_event_unpack(event, &ACTION_DATA_EVENT_DATA, &sz);

    mu_assert_long_equals(sz, ACTION_DATA_EVENT_DATA_LENGTH);
    mu_assert_int64_equals(event->timestamp, 30LL);
    mu_assert(event->action_id == 20, "Expected action id to equal 20");
    mu_assert(event->object_id == 0, "Expected object id to equal 0");
    mu_assert(event->data != NULL, "Expected data to not be NULL");
    mu_assert(event->data_count == 2, "Expected data count to be 2");

    sky_event_data *data = NULL;
    sky_event_get_data(event, 1, &data);
    mu_assert(biseqcstr(data->value, "foo"), "Expected data 1 to equal 'foo'");
    sky_event_get_data(event, 2, &data);
    mu_assert(biseqcstr(data->value, "bar"), "Expected data 1 to equal 'bar'");

    sky_event_free(event);
    
    return 0;
}



//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_event_create);

    mu_run_test(test_sky_event_set_data);
    mu_run_test(test_sky_event_unset_data);
    
    /*
    mu_run_test(test_sky_event_action_event_sizeof);
    mu_run_test(test_sky_event_data_event_sizeof);
    mu_run_test(test_sky_event_action_data_event_sizeof);
    */
    
    mu_run_test(test_sky_event_action_event_pack);
    mu_run_test(test_sky_event_data_event_pack);
    mu_run_test(test_sky_event_action_data_event_pack);

    mu_run_test(test_sky_event_action_event_unpack);
    mu_run_test(test_sky_event_data_event_unpack);
    mu_run_test(test_sky_event_action_data_event_unpack);

    return 0;
}

RUN_TESTS()