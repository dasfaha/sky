#include <stdio.h>
#include <stdlib.h>

#include <dbg.h>
#include <mem.h>
#include <path_iterator.h>

#include "minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Single Block
//--------------------------------------

int test_sky_path_iterator_single_block_next() {
    loadtmp("tests/fixtures/path_iterator/0");
    int rc;
    void *block_ptr, *ptr;
    sky_data_file *data_file = sky_data_file_create();
    data_file->path = bfromcstr("tmp/data");
    data_file->header_path = bfromcstr("tmp/header");
    sky_data_file_load(data_file);

    sky_block_get_ptr(data_file->blocks[0], &block_ptr);
    
    sky_path_iterator *iterator = sky_path_iterator_create();
    sky_path_iterator_set_block(iterator, data_file->blocks[0]);
    mu_assert(iterator->block == data_file->blocks[0], "");
    mu_assert(iterator->data_file == NULL, "");
    mu_assert_int_equals(iterator->block_index, 0);

    // Path 1
    rc = sky_path_iterator_get_ptr(iterator, &ptr);
    mu_assert_int_equals(rc, 0);
    mu_assert_long_equals(ptr-block_ptr, 0L);
    mu_assert_bool(!iterator->eof);

    // Path 2
    rc = sky_path_iterator_next(iterator);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(iterator->byte_index, 12);
    mu_assert_bool(!iterator->eof);
    rc = sky_path_iterator_get_ptr(iterator, &ptr);
    mu_assert_int_equals(rc, 0);
    mu_assert_long_equals(ptr-block_ptr, 12L);
    
    // Path 3
    rc = sky_path_iterator_next(iterator);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(iterator->byte_index, 58);
    mu_assert_bool(!iterator->eof);
    rc = sky_path_iterator_get_ptr(iterator, &ptr);
    mu_assert_int_equals(rc, 0);
    mu_assert_long_equals(ptr-block_ptr, 58L);
    
    // EOF
    rc = sky_path_iterator_next(iterator);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(iterator->byte_index, 0);
    mu_assert_bool(iterator->eof);

    sky_path_iterator_free(iterator);
    sky_data_file_free(data_file);
    return 0;
}


//--------------------------------------
// Data File
//--------------------------------------

int test_sky_path_iterator_data_file_next() {
    loadtmp("tests/fixtures/path_iterator/1");
    int rc;
    void *block_ptr, *ptr;
    sky_data_file *data_file = sky_data_file_create();
    data_file->path = bfromcstr("tmp/data");
    data_file->header_path = bfromcstr("tmp/header");
    sky_data_file_load(data_file);

    sky_path_iterator *iterator = sky_path_iterator_create();
    sky_path_iterator_set_data_file(iterator, data_file);
    mu_assert(iterator->block == NULL, "");
    mu_assert(iterator->data_file == data_file, "");
    mu_assert_int_equals(iterator->block_index, 0);

    // Path 1
    rc = sky_path_iterator_get_ptr(iterator, &ptr);
    mu_assert_int_equals(rc, 0);
    mu_assert_long_equals(ptr-data_file->data, 0L);
    mu_assert_bool(!iterator->eof);

    // Path 2
    rc = sky_path_iterator_next(iterator);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(iterator->byte_index, 12);
    mu_assert_bool(!iterator->eof);
    rc = sky_path_iterator_get_ptr(iterator, &ptr);
    mu_assert_int_equals(rc, 0);
    mu_assert_long_equals(ptr-data_file->data, 12L);
    
    // Path 3 (Spanned)
    rc = sky_path_iterator_next(iterator);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(iterator->byte_index, 0);
    mu_assert_bool(!iterator->eof);
    rc = sky_path_iterator_get_ptr(iterator, &ptr);
    mu_assert_int_equals(rc, 0);
    mu_assert_long_equals(ptr-data_file->data, 64L);
    
    // Path 4
    rc = sky_path_iterator_next(iterator);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(iterator->byte_index, 0);
    mu_assert_bool(!iterator->eof);
    rc = sky_path_iterator_get_ptr(iterator, &ptr);
    mu_assert_int_equals(rc, 0);
    mu_assert_long_equals(ptr-data_file->data, 192L);
    
    // EOF
    rc = sky_path_iterator_next(iterator);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(iterator->byte_index, 0);
    mu_assert_bool(iterator->eof);

    sky_path_iterator_free(iterator);
    sky_data_file_free(data_file);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_path_iterator_single_block_next);
    mu_run_test(test_sky_path_iterator_data_file_next);
    return 0;
}

RUN_TESTS()