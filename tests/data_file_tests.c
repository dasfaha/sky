#include <stdio.h>
#include <stdlib.h>

#include <dbg.h>
#include <mem.h>
#include <data_file.h>

#include "minunit.h"


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
    cleandb();
    
    int rc;
    sky_data_file *data_file = sky_data_file_create();
    data_file->block_size = 128;
    data_file->path = bfromcstr("tmp/db/data");
    data_file->header_path = bfromcstr("tmp/db/header");
    
    rc = sky_data_file_load(data_file);
    mu_assert_int_equals(rc, 0);

    mu_assert_bool(data_file->data != NULL);
    mu_assert_bool(data_file->data_fd != 0);
    mu_assert_long_equals(data_file->data_length, 128L);
    mu_assert_bool(data_file->blocks != NULL);
    mu_assert_int_equals(data_file->block_count, 1);
    mu_assert_file("tmp/db/data", "tests/fixtures/data_files/0/data");
    mu_assert_file("tmp/db/header", "tests/fixtures/data_files/0/header");

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

//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_data_file_set_path);
    mu_run_test(test_sky_data_file_set_header_path);
    mu_run_test(test_sky_data_file_load_empty);
    return 0;
}

RUN_TESTS()