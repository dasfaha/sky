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

size_t DATA_LENGTH = 5;
char DATA[] = "\x0a\xa3\x66\x6f\x6f";


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int test_sky_event_data_create() {
    struct tagbstring value = bsStatic("foo");
    
    sky_event_data *data = sky_event_data_create(10, &value);
    mu_assert(data != NULL, "Unable to allocate event data");
    mu_assert(data->key == 10, "Event data key not assigned");
    mu_assert(biseqcstr(data->string_value, "foo"), "Event data value not assigned");

    sky_event_data_free(data);

    return 0;
}


//--------------------------------------
// Serialization
//--------------------------------------

int test_sky_event_data_sizeof() {
    struct tagbstring value = bsStatic("foo");
    sky_event_data *data;
    
    data = sky_event_data_create(10, &value);
    size_t sz = sky_event_data_sizeof(data);
    mu_assert_long_equals(sz, 5L);
    sky_event_data_free(data);

    return 0;
}

int test_sky_event_data_pack() {
    size_t sz;
    struct tagbstring value = bsStatic("foo");
    void *addr = calloc(DATA_LENGTH, 1);
    
    sky_event_data *data = sky_event_data_create(10, &value);
    sky_event_data_pack(data, addr, &sz);
    sky_event_data_free(data);
    
    mu_assert_long_equals(sz, DATA_LENGTH);
    mu_assert_mem(addr, &DATA, DATA_LENGTH);

    free(addr);
    
    return 0;
}

int test_sky_event_data_unpack() {
    size_t sz;

    sky_event_data *data = sky_event_data_create(0, NULL);
    sky_event_data_unpack(data, &DATA, &sz);

    mu_assert_long_equals(sz, DATA_LENGTH);
    mu_assert(data->key == 10, "");
    mu_assert(biseqcstr(data->string_value, "foo"), "");

    sky_event_data_free(data);

    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_event_data_create);
    mu_run_test(test_sky_event_data_sizeof);
    mu_run_test(test_sky_event_data_pack);
    mu_run_test(test_sky_event_data_unpack);
    return 0;
}

RUN_TESTS()