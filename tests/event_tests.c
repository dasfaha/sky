#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <event.h>

#include "minunit.h"

//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring foo = bsStatic("foo");
struct tagbstring bar = bsStatic("bar");
struct tagbstring baz = bsStatic("baz");


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

char *test_Event_create() {
    Event *event = Event_create(1325376000000LL, 10, 200);
    mu_assert(event != NULL, "Unable to allocate event");
    mu_assert(event->timestamp == 1325376000000LL, "Event timestamp not assigned");
    mu_assert(event->object_id == 10, "Event object id not assigned");
    mu_assert(event->action_id == 200, "Event action id not assigned");
    mu_assert(event->data == NULL, "Event data non-null");
    mu_assert(event->data_count == 0, "Event data count not initialized");

    Event_destroy(event);

    return NULL;
}


//--------------------------------------
// Event data
//--------------------------------------

char *test_Event_set_data() {
    EventData *data = NULL;

    Event *event = Event_create(0, 0, 0);
    Event_set_data(event, 10, &foo);
    Event_set_data(event, 20, &bar);
    Event_set_data(event, 10, &baz);
    mu_assert(event->data_count == 2, "Expected data count to be 2");

    Event_get_data(event, 10, &data);
    mu_assert(biseqcstr(data->value, "baz"), "Expected data 10 to equal 'baz'");
    Event_get_data(event, 20, &data);
    mu_assert(biseqcstr(data->value, "bar"), "Expected data 20 to equal 'bar'");
    Event_get_data(event, 30, &data);
    mu_assert(data == NULL, "Expected data 30 be NULL");

    Event_destroy(event);

    return NULL;
}

char *test_Event_unset_data() {
    EventData *data = NULL;

    Event *event = Event_create(0, 0, 0);
    Event_set_data(event, 10, &foo);
    Event_set_data(event, 20, &bar);
    mu_assert(event->data_count == 2, "Expected data count to be 2");
    Event_unset_data(event, 10);
    mu_assert(event->data_count == 1, "Expected data count to be 1");

    Event_get_data(event, 10, &data);
    mu_assert(data == NULL, "Expected data 10 be NULL");
    Event_get_data(event, 20, &data);
    mu_assert(biseqcstr(data->value, "bar"), "Expected data 20 to equal 'bar'");

    Event_destroy(event);

    return NULL;
}


//--------------------------------------
// Serialization length
//--------------------------------------

// Action-only event.
char *test_Event_action_event_get_serialized_length() {
    Event *event = Event_create(1325376000000LL, 0, 20);
    mu_assert(Event_get_serialized_length(event) == 13, "Unexpected length for action-only event.");
    Event_destroy(event);
    return NULL;
}

// Data-only event.
char *test_Event_data_event_get_serialized_length() {
    Event *event = Event_create(1325376000000LL, 0, 0);
    Event_set_data(event, 1, &foo);
    Event_set_data(event, 2, &bar);
    mu_assert(Event_get_serialized_length(event) == 23, "Unexpected length for data-only event.");
    Event_destroy(event);
    return NULL;
}

// Action + data event.
char *test_Event_action_data_event_get_serialized_length() {
    Event *event = Event_create(1325376000000LL, 0, 100);
    Event_set_data(event, 1, &foo);
    Event_set_data(event, 2, &bar);
    mu_assert(Event_get_serialized_length(event) == 27, "Unexpected length for action+data event.");
    Event_destroy(event);
    return NULL;
}

//--------------------------------------
// Serialization
//--------------------------------------

// Action event.
char *test_Event_action_event_serialize() {
    FILE *file = fopen(TEMPFILE, "w");
    Event *event = Event_create(1325376000000LL, 0, 20);
    Event_serialize(event, file);
    Event_destroy(event);
    fclose(file);
    mu_assert_tempfile("tests/fixtures/serialization/action_event", "Serialize Action Event");
    return NULL;
}

// Data event.
char *test_Event_data_event_serialize() {
    FILE *file = fopen(TEMPFILE, "w");
    Event *event = Event_create(1325376000000LL, 0, 0);
    Event_set_data(event, 1, &foo);
    Event_set_data(event, 2, &bar);
    Event_serialize(event, file);
    Event_destroy(event);
    fclose(file);
    mu_assert_tempfile("tests/fixtures/serialization/data_event", "Serialize Data Event");
    return NULL;
}

// Action+Data event.
char *test_Event_action_data_event_serialize() {
    FILE *file = fopen(TEMPFILE, "w");
    Event *event = Event_create(1325376000000LL, 0, 20);
    Event_set_data(event, 1, &foo);
    Event_set_data(event, 2, &bar);
    Event_serialize(event, file);
    Event_destroy(event);
    fclose(file);
    mu_assert_tempfile("tests/fixtures/serialization/action_data_event", "Serialize Action+Data Event");
    return NULL;
}

/*
char *test_Event_deserialize() {
    FILE *file = fopen("tests/fixtures/serialization/event_data", "r");
    EventData *data = EventData_create(0, NULL);
    EventData_deserialize(data, file);
    fclose(file);

    mu_assert(data->key == 10, "Expected data key to equal 10");
    mu_assert(biseqcstr(data->value, "foo"), "Expected data value to equal 'foo'");

    EventData_destroy(data);

    return NULL;
}
*/


//==============================================================================
//
// Setup
//
//==============================================================================

char *all_tests() {
    mu_run_test(test_Event_create);

    mu_run_test(test_Event_set_data);
    mu_run_test(test_Event_unset_data);
    
    mu_run_test(test_Event_action_event_get_serialized_length);
    mu_run_test(test_Event_data_event_get_serialized_length);
    mu_run_test(test_Event_action_data_event_get_serialized_length);
    
    mu_run_test(test_Event_action_event_serialize);
    mu_run_test(test_Event_data_event_serialize);
    mu_run_test(test_Event_action_data_event_serialize);

    //mu_run_test(test_Event_deserialize);

    return NULL;
}

RUN_TESTS(all_tests)