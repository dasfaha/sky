#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

#include <event_data.h>

#include "minunit.h"

//==============================================================================
//
// Test Cases
//
//==============================================================================

size_t INT_DATA_LENGTH = 4;
char INT_DATA[] = "\x0a\xD1\x03\xE8";

size_t FLOAT_DATA_LENGTH = 10;
char FLOAT_DATA[] = "\x0a\xCB\x40\x59\x0C\xCC\xCC\xCC\xCC\xCD";

size_t BOOLEAN_DATA_LENGTH = 2;
char BOOLEAN_DATA[] = "\x0a\xC3";

size_t STRING_DATA_LENGTH = 5;
char STRING_DATA[] = "\x0a\xa3\x66\x6f\x6f";


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int test_sky_event_data_create_int() {
    sky_event_data *data = sky_event_data_create_int(10, 20);
    mu_assert_bool(data != NULL);
    mu_assert_int_equals(data->key, 10);
    mu_assert_int64_equals(data->int_value, 20LL);
    sky_event_data_free(data);
    return 0;
}

int test_sky_event_data_create_float() {
    sky_event_data *data = sky_event_data_create_float(10, 100.1);
    mu_assert_bool(data != NULL);
    mu_assert_int_equals(data->key, 10);
    mu_assert_bool(fabs(data->float_value - 100.1) < 0.1);
    sky_event_data_free(data);
    return 0;
}

int test_sky_event_data_create_boolean() {
    sky_event_data *data = sky_event_data_create_boolean(10, true);
    mu_assert_bool(data != NULL);
    mu_assert_int_equals(data->key, 10);
    mu_assert_bool(data->boolean_value == true);
    sky_event_data_free(data);
    return 0;
}

int test_sky_event_data_create_string() {
    struct tagbstring value = bsStatic("foo");
    sky_event_data *data = sky_event_data_create_string(10, &value);
    mu_assert_bool(data != NULL);
    mu_assert_int_equals(data->key, 10);
    mu_assert_bstring(data->string_value, "foo");
    sky_event_data_free(data);
    return 0;
}


//--------------------------------------
// Sizeof
//--------------------------------------

int test_sky_event_data_sizeof_int() {
    sky_event_data *data = sky_event_data_create_int(10, 20);
    size_t sz = sky_event_data_sizeof(data);
    mu_assert_long_equals(sz, 2L);
    sky_event_data_free(data);
    return 0;
}

int test_sky_event_data_sizeof_float() {
    sky_event_data *data = sky_event_data_create_float(10, 100.1);
    size_t sz = sky_event_data_sizeof(data);
    mu_assert_long_equals(sz, 10L);
    sky_event_data_free(data);
    return 0;
}

int test_sky_event_data_sizeof_boolean() {
    sky_event_data *data = sky_event_data_create_boolean(10, true);
    size_t sz = sky_event_data_sizeof(data);
    mu_assert_long_equals(sz, 2L);
    sky_event_data_free(data);
    return 0;
}

int test_sky_event_data_sizeof_string() {
    struct tagbstring value = bsStatic("foo");
    sky_event_data *data = sky_event_data_create_string(10, &value);
    size_t sz = sky_event_data_sizeof(data);
    mu_assert_long_equals(sz, 5L);
    sky_event_data_free(data);
    return 0;
}

//--------------------------------------
// Pack
//--------------------------------------

int test_sky_event_data_pack_int() {
    size_t sz;
    void *ptr = calloc(INT_DATA_LENGTH, 1);
    sky_event_data *data = sky_event_data_create_int(10, 1000);
    sky_event_data_pack(data, ptr, &sz);
    sky_event_data_free(data);
    mu_assert_long_equals(sz, INT_DATA_LENGTH);
    mu_assert_mem(ptr, &INT_DATA, INT_DATA_LENGTH);
    free(ptr);
    return 0;
}

int test_sky_event_data_pack_float() {
    size_t sz;
    void *ptr = calloc(FLOAT_DATA_LENGTH, 1);
    sky_event_data *data = sky_event_data_create_float(10, 100.2);
    sky_event_data_pack(data, ptr, &sz);
    sky_event_data_free(data);
    mu_assert_long_equals(sz, FLOAT_DATA_LENGTH);
    mu_assert_mem(ptr, &FLOAT_DATA, FLOAT_DATA_LENGTH);
    free(ptr);
    return 0;
}

int test_sky_event_data_pack_boolean() {
    size_t sz;
    void *ptr = calloc(BOOLEAN_DATA_LENGTH, 1);
    sky_event_data *data = sky_event_data_create_boolean(10, true);
    sky_event_data_pack(data, ptr, &sz);
    sky_event_data_free(data);
    mu_assert_long_equals(sz, BOOLEAN_DATA_LENGTH);
    mu_assert_mem(ptr, &BOOLEAN_DATA, BOOLEAN_DATA_LENGTH);
    free(ptr);
    return 0;
}

int test_sky_event_data_pack_string() {
    size_t sz;
    struct tagbstring value = bsStatic("foo");
    void *ptr = calloc(STRING_DATA_LENGTH, 1);
    sky_event_data *data = sky_event_data_create_string(10, &value);
    sky_event_data_pack(data, ptr, &sz);
    sky_event_data_free(data);
    mu_assert_long_equals(sz, STRING_DATA_LENGTH);
    mu_assert_mem(ptr, &STRING_DATA, STRING_DATA_LENGTH);
    free(ptr);
    return 0;
}

//--------------------------------------
// Unpack
//--------------------------------------

int test_sky_event_data_unpack_int() {
    size_t sz;
    sky_event_data *data = sky_event_data_create(0);
    sky_event_data_unpack(data, &INT_DATA, &sz);
    mu_assert_long_equals(sz, INT_DATA_LENGTH);
    mu_assert_bstring(data->data_type, "Int");
    mu_assert_int_equals(data->key, 10);
    mu_assert_int64_equals(data->int_value, 1000LL);
    sky_event_data_free(data);
    return 0;
}

int test_sky_event_data_unpack_float() {
    size_t sz;
    sky_event_data *data = sky_event_data_create(0);
    sky_event_data_unpack(data, &FLOAT_DATA, &sz);
    mu_assert_long_equals(sz, FLOAT_DATA_LENGTH);
    mu_assert_bstring(data->data_type, "Float");
    mu_assert_int_equals(data->key, 10);
    mu_assert_bool(fabs(data->float_value - 100.2) < 0.1);
    sky_event_data_free(data);
    return 0;
}

int test_sky_event_data_unpack_boolean() {
    size_t sz;
    sky_event_data *data = sky_event_data_create(0);
    sky_event_data_unpack(data, &BOOLEAN_DATA, &sz);
    mu_assert_long_equals(sz, BOOLEAN_DATA_LENGTH);
    mu_assert_bstring(data->data_type, "Boolean");
    mu_assert_int_equals(data->key, 10);
    mu_assert_bool(data->boolean_value == true);
    sky_event_data_free(data);
    return 0;
}

int test_sky_event_data_unpack_string() {
    size_t sz;
    sky_event_data *data = sky_event_data_create(0);
    sky_event_data_unpack(data, &STRING_DATA, &sz);
    mu_assert_long_equals(sz, STRING_DATA_LENGTH);
    mu_assert_bstring(data->data_type, "String");
    mu_assert_int_equals(data->key, 10);
    mu_assert_bstring(data->string_value, "foo");
    sky_event_data_free(data);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_event_data_create_int);
    mu_run_test(test_sky_event_data_create_float);
    mu_run_test(test_sky_event_data_create_boolean);
    mu_run_test(test_sky_event_data_create_string);

    mu_run_test(test_sky_event_data_sizeof_int);
    mu_run_test(test_sky_event_data_sizeof_float);
    mu_run_test(test_sky_event_data_sizeof_boolean);
    mu_run_test(test_sky_event_data_sizeof_string);

    mu_run_test(test_sky_event_data_pack_int);
    mu_run_test(test_sky_event_data_pack_float);
    mu_run_test(test_sky_event_data_pack_boolean);
    mu_run_test(test_sky_event_data_pack_string);

    mu_run_test(test_sky_event_data_unpack_int);
    mu_run_test(test_sky_event_data_unpack_float);
    mu_run_test(test_sky_event_data_unpack_boolean);
    mu_run_test(test_sky_event_data_unpack_string);
    return 0;
}

RUN_TESTS()