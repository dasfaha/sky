#include <stdio.h>
#include <stdlib.h>

#include <file.h>
#include "minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// File exists
//--------------------------------------

int test_sky_file_exists() {
    struct tagbstring empty_file_path = bsStatic("tests/fixtures/file/empty_file.txt");
    mu_assert(sky_file_exists(&empty_file_path) == true, "");
    return 0;
}


//--------------------------------------
// File Copy
//--------------------------------------

int test_sky_file_cp() {
    struct tagbstring src_path  = bsStatic("tests/fixtures/file/lorem.txt");
    struct tagbstring dest_path = bsStatic("tmp/dest.txt");
    int rc = sky_file_cp(&src_path, &dest_path);
    mu_assert_int_equals(rc, 0);
    mu_assert_file(bdata(&src_path), bdata(&dest_path));
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_file_exists);
    mu_run_test(test_sky_file_cp);
    return 0;
}

RUN_TESTS()