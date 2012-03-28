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

char *test_EventData_create() {
    struct tagbstring value = bsStatic("foo");
    
    EventData *data = EventData_create(10, &value);
    mu_assert(data != NULL, "Unable to allocate event data");
    mu_assert(data->key == 10, "Event data key not assigned");
    mu_assert(biseqcstr(data->value, "foo"), "Event data value not assigned");

    EventData_destroy(data);

    return NULL;
}

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
    char exp[] = {0x0A, 0x00, 0x03, 'f', 'o', 'o'};

    int fd = open(TEMPFILE, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    EventData *data = EventData_create(10, &value);
    EventData_serialize(data, fd);
    mu_assert_tempfile(6, exp);
    EventData_destroy(data);
    close(fd);

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
    return NULL;
}

RUN_TESTS(all_tests)