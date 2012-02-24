#include <stdio.h>
#include "minunit.h"
#include <database.h>
#include <bstring.h>

//==============================================================================
//
// Test Cases
//
//==============================================================================

char *test_Database_create() {
    Database *database = Database_create(
        bfromcstr("foo")
    );
    mu_assert(database != NULL, "Could not create database");
    mu_assert(biseq(database->name, bfromcstr("foo")), "Invalid name");
    return NULL;
}



//==============================================================================
//
// Setup
//
//==============================================================================

char *all_tests() {
    mu_run_test(test_Database_create);
    return NULL;
}

RUN_TESTS(all_tests)