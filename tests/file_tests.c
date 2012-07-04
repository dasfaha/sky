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


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_file_exists);
    return 0;
}

RUN_TESTS()