#include <stdio.h>
#include <database.h>
#include <bstring.h>

#include "minunit.h"

//==============================================================================
//
// Test Cases
//
//==============================================================================

int test_sky_database_set_path() {
    struct tagbstring root = bsStatic("/etc/sky/data");
    sky_database *database = sky_database_create();
    int rc = sky_database_set_path(database, &root);
    mu_assert_int_equals(rc, 0);
    mu_assert_bstring(database->path, "/etc/sky/data");
    sky_database_free(database);
    return 0;
}



//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_database_set_path);
    return 0;
}

RUN_TESTS()