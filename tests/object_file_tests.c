#include <stdio.h>
#include <database.h>
#include <object_file.h>
#include <bstring.h>

#include "minunit.h"

//==============================================================================
//
// Test Cases
//
//==============================================================================

char *test_ObjectFile_create_destroy() {
    Database *database = Database_create(bfromcstr("/etc/sky/data"));
    ObjectFile *object_file = ObjectFile_create(
        database,
        bfromcstr("users")
    );

    mu_assert(object_file != NULL, "Could not create object file");
    mu_assert(biseqcstr(object_file->name, "users"), "Invalid name");

    ObjectFile_destroy(object_file);
    Database_destroy(database);

    return NULL;
}



//==============================================================================
//
// Setup
//
//==============================================================================

char *all_tests() {
    mu_run_test(test_ObjectFile_create_destroy);
    return NULL;
}

RUN_TESTS(all_tests)