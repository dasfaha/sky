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

char DATA[] = {0x0A, 0x00, 0x03, 0x66, 0x6F, 0x6F};
int DATA_LENGTH = 6;


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int test_EventData_create() {
    struct tagbstring value = bsStatic("foo");
    
    EventData *data = EventData_create(10, &value);
    mu_assert(data != NULL, "Unable to allocate event data");
    mu_assert(data->key == 10, "Event data key not assigned");
    mu_assert(biseqcstr(data->value, "foo"), "Event data value not assigned");

    EventData_destroy(data);

    return 0;
}


//--------------------------------------
// Serialization
//--------------------------------------

int test_EventData_get_serialized_length() {
    struct tagbstring value = bsStatic("foo");
    EventData *data;
    
    data = EventData_create(10, &value);
    uint32_t length = EventData_get_serialized_length(data);
    mu_assert(length == 6, "Expected length of 6 for 'foo'");
    EventData_destroy(data);

    return 0;
}

int test_EventData_serialize() {
    struct tagbstring value = bsStatic("foo");
    void *addr = calloc(DATA_LENGTH, 1);

    EventData *data = EventData_create(10, &value);
    EventData_serialize(data, addr);
    EventData_destroy(data);
    
    mu_assert(memcmp(addr, &DATA, DATA_LENGTH) == 0, "");

    free(addr);
    
    return 0;
}

int test_EventData_deserialize() {
    EventData *data = EventData_create(0, NULL);
    EventData_deserialize(data, &DATA);

    mu_assert(data->key == 10, "");
    mu_assert(biseqcstr(data->value, "foo"), "");

    EventData_destroy(data);

    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_EventData_create);
    mu_run_test(test_EventData_get_serialized_length);
    mu_run_test(test_EventData_serialize);
    mu_run_test(test_EventData_deserialize);
    return 0;
}

RUN_TESTS()