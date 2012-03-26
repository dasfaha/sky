#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <event_data.h>

#include "minunit.h"

//==============================================================================
//
// Test Cases
//
//==============================================================================

char *test_EventData_create() {
    struct tagbstring value = bsStatic("foo");
    
    EventData *data = EventData_create(10, &value);
    mu_assert(data != NULL, "Unable to allocate event data");
    mu_assert(data->key == 10, "Event data key not assigned");
    mu_assert(biseqcstr(data->value, "foo"), "Event data value not assigned");

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
    return NULL;
}

RUN_TESTS(all_tests)