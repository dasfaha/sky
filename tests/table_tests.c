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
    // TODO: Test locking.
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