#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <event.h>

#include "minunit.h"

//==============================================================================
//
// Test Cases
//
//==============================================================================

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

char *test_Event_set_data() {
    struct tagbstring foo = bsStatic("foo");
    struct tagbstring bar = bsStatic("bar");
    struct tagbstring baz = bsStatic("baz");

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
    struct tagbstring foo = bsStatic("foo");
    struct tagbstring bar = bsStatic("bar");

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


//==============================================================================
//
// Setup
//
//==============================================================================

char *all_tests() {
    mu_run_test(test_Event_create);
    mu_run_test(test_Event_set_data);
    mu_run_test(test_Event_unset_data);
    return NULL;
}

RUN_TESTS(all_tests)