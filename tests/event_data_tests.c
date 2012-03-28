#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <event_data.h>

#include "minunit.h"

//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

char *test_EventData_create() {
    struct tagbstring value = bsStatic("foo");
    
    EventData *data = EventData_create(10, &value);
    mu_assert(data != NULL, "Unable to allocate event data");
    mu_assert(data->key == 10, "Event data key not assigned");
    mu_assert(biseqcstr(data->value, "foo"), "Event data value not assigned");

    EventData_destroy(data);

    return NULL;
}


//--------------------------------------
// Serialization
//--------------------------------------

char *test_EventData_get_serialized_length() {
    struct tagbstring value = bsStatic("foo");
    EventData *data;
    
    data = EventData_create(10, &value);
    uint32_t length = EventData_get_serialized_length(data);
    mu_assert(length == 6, "Expected length of 6 for 'foo'");
    EventData_destroy(data);

    return NULL;
}

char *test_EventData_serialize() {
    struct tagbstring value = bsStatic("foo");

    FILE *file = fopen(TEMPFILE, "w");
    EventData *data = EventData_create(10, &value);
    EventData_serialize(data, file);
    EventData_destroy(data);
    fclose(file);

    mu_assert_tempfile("tests/fixtures/serialization/event_data", "Serialize Event Data");

    return NULL;
}

char *test_EventData_deserialize() {
    FILE *file = fopen("tests/fixtures/serialization/event_data", "r");
    EventData *data = EventData_create(0, NULL);
    EventData_deserialize(data, file);
    fclose(file);

    mu_assert(data->key == 10, "Expected data key to equal 10");
    mu_assert(biseqcstr(data->value, "foo"), "Expected data value to equal 'foo'");

    EventData_destroy(data);

    return NULL;
}


//==============================================================================
//
// Setup
//
//==============================================================================

char *all_tests() {
    mu_run_test(test_EventData_create);
    mu_run_test(test_EventData_get_serialized_length);
    mu_run_test(test_EventData_serialize);
    mu_run_test(test_EventData_deserialize);
    return NULL;
}

RUN_TESTS(all_tests)