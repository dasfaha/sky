#include <stdio.h>
#include <stdlib.h>

#include <dbg.h>
#include <mem.h>
#include <data_file.h>

#include "minunit.h"


//==============================================================================
//
// Helpers
//
//==============================================================================

#define INIT_DATA_FILE(PATH, BLOCK_SIZE) \
    loadtmp(PATH); \
    data_file = sky_data_file_create(); \
    data_file->block_size = 64; \
    data_file->path = bfromcstr("tmp/data"); \
    data_file->header_path = bfromcstr("tmp/header"); \
    sky_data_file_load(data_file);

#define ADD_EVENT(TIMESTAMP, OBJECT_ID, ACTION_ID) do { \
    sky_event *event = sky_event_create(TIMESTAMP, OBJECT_ID, ACTION_ID); \
    mu_assert_int_equals(sky_data_file_add_event(data_file, event), 0); \
    sky_event_free(event); \
} while (0)


#define ASSERT_DATA_FILE(FIXTURE) \
    mu_assert_file("tmp/data", FIXTURE "/data"); \
    mu_assert_file("tmp/header", FIXTURE "/header");


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Paths
//--------------------------------------

int test_sky_data_file_set_path() {
    int rc;
    struct tagbstring path = bsStatic("/dev/null");
    sky_data_file *data_file = sky_data_file_create();
    rc = sky_data_file_set_path(data_file, &path);
    mu_assert_int_equals(rc, 0);
    mu_assert_bstring(data_file->path, "/dev/null");
    sky_data_file_free(data_file);
    return 0;
}

int test_sky_data_file_set_header_path() {
    int rc;
    struct tagbstring path = bsStatic("/dev/null");
    sky_data_file *data_file = sky_data_file_create();
    rc = sky_data_file_set_path(data_file, &path);
    mu_assert_int_equals(rc, 0);
    mu_assert_bstring(data_file->path, "/dev/null");
    sky_data_file_free(data_file);
    return 0;
}


//--------------------------------------
// Empty File
//--------------------------------------

int test_sky_data_file_load_empty() {
    cleantmp();
    
    int rc;
    sky_data_file *data_file = sky_data_file_create();
    data_file->block_size = 128;
    data_file->path = bfromcstr("tmp/data");
    data_file->header_path = bfromcstr("tmp/header");
    
    rc = sky_data_file_load(data_file);
    mu_assert_int_equals(rc, 0);

    mu_assert_bool(data_file->data != NULL);
    mu_assert_bool(data_file->data_fd != 0);
    mu_assert_long_equals(data_file->data_length, 128L);
    mu_assert_bool(data_file->blocks != NULL);
    mu_assert_int_equals(data_file->block_count, 1);
    mu_assert_file("tmp/data", "tests/fixtures/data_files/0/data");
    mu_assert_file("tmp/header", "tests/fixtures/data_files/0/header");

    rc = sky_data_file_unload(data_file);
    mu_assert_int_equals(rc, 0);
    
    mu_assert_bool(data_file->data == NULL);
    mu_assert_bool(data_file->data_fd == 0);
    mu_assert_long_equals(data_file->data_length, 0L);
    mu_assert_bool(data_file->blocks == NULL);
    mu_assert_int_equals(data_file->block_count, 0);

    sky_data_file_free(data_file);
    return 0;
}


//--------------------------------------
// Add Event
//--------------------------------------

int test_sky_data_file_add_event_to_new_block() {
    sky_data_file *data_file;
    INIT_DATA_FILE("", 64);
    ADD_EVENT(10LL, 1LL, 20);
    ASSERT_DATA_FILE("tests/fixtures/data_files/1/a");
    sky_data_file_free(data_file);
    return 0;
}

int test_sky_data_file_prepend_event_to_existing_path() {
    sky_data_file *data_file;
    INIT_DATA_FILE("tests/fixtures/data_files/1/a", 0);
    ADD_EVENT(8LL, 1LL, 21);
    ASSERT_DATA_FILE("tests/fixtures/data_files/1/b");
    sky_data_file_free(data_file);
    return 0;
}

int test_sky_data_file_append_event_to_existing_path() {
    sky_data_file *data_file;
    INIT_DATA_FILE("tests/fixtures/data_files/1/b", 0);
    ADD_EVENT(11LL, 1LL, 22);
    ASSERT_DATA_FILE("tests/fixtures/data_files/1/c");
    sky_data_file_free(data_file);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_data_file_set_path);
    mu_run_test(test_sky_data_file_set_header_path);
    mu_run_test(test_sky_data_file_load_empty);
    mu_run_test(test_sky_data_file_add_event_to_new_block);
    mu_run_test(test_sky_data_file_prepend_event_to_existing_path);
    return 0;
}

RUN_TESTS()