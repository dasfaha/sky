#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <dbg.h>
#include <table.h>
#include <bstring.h>

#include "minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Open
//--------------------------------------

int test_sky_table_open() {
    struct tagbstring lock_file_path = bsStatic("tmp/.skylock");
    struct tagbstring data_file_path = bsStatic("tmp/0/data");
    struct tagbstring header_file_path = bsStatic("tmp/0/header");
    cleantmp();
    
    int rc;
    sky_table *table = sky_table_create();
    table->path = bfromcstr("tmp");
    
    // Open table.
    rc = sky_table_open(table);
    mu_assert_int_equals(rc, 0);
    mu_assert(sky_file_exists(&lock_file_path), "");
    mu_assert(sky_file_exists(&data_file_path), "");
    mu_assert(sky_file_exists(&header_file_path), "");
    
    // Close table.
    rc = sky_table_close(table);
    mu_assert_int_equals(rc, 0);
    mu_assert(!sky_file_exists(&lock_file_path), "");
    mu_assert(sky_file_exists(&data_file_path), "");
    mu_assert(sky_file_exists(&header_file_path), "");

    sky_table_free(table);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_table_open);
    return 0;
}

RUN_TESTS()