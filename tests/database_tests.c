#include <stdio.h>
#include <database.h>
#include <bstring.h>

#include "minunit.h"

//==============================================================================
//
// Test Cases
//
//==============================================================================

int test_sky_database_create_destroy() {
    struct tagbstring root = bsStatic("/etc/sky/data");
    sky_database *database = sky_database_create(&root);
    mu_assert(database != NULL, "Could not create database");
    mu_assert(biseqcstr(database->path, "/etc/sky/data"), "Invalid path");
    sky_database_free(database);
    return 0;
}



//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_database_create_destroy);
    return 0;
}

RUN_TESTS()