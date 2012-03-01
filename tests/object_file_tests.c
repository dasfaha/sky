#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    mu_assert(biseqcstr(object_file->name, "users") == 1, "Invalid name");

    ObjectFile_destroy(object_file);
    Database_destroy(database);

    return NULL;
}

char *test_ObjectFile_open() {
    int rc = 0;
    
    char *cwd = getcwd(NULL, 0);
    bstring path = bformat("%s/tests/fixtures/db/simple", cwd);
    struct tagbstring object_type = bsStatic("users");

    Database *database = Database_create(path);
    ObjectFile *object_file = ObjectFile_create(database, &object_type);
    rc = ObjectFile_open(object_file);

    mu_assert(rc == 0, "Object file could not be opened");
    mu_assert(object_file->action_count == 3, "Expected 3 actions");

    mu_assert(object_file->actions[0].id == 1, "Expected action[0].id == 1");
    mu_assert(biseqcstr(object_file->actions[0].name, "home_page"), "Expected action[0].name == 'home_page'");

    mu_assert(object_file->actions[1].id == 2, "Expected action[1].id == 2");
    mu_assert(biseqcstr(object_file->actions[0].name, "sign_up"), "Expected action[0].name == 'home_page'");

    mu_assert(object_file->actions[2].id == 3, "Expected action[2].id == 3");
    mu_assert(biseqcstr(object_file->actions[0].name, "sign_in"), "Expected action[0].name == 'home_page'");

    ObjectFile_destroy(object_file);
    Database_destroy(database);

    bdestroy(path);
    free(cwd);

    return NULL;
}


//==============================================================================
//
// Setup
//
//==============================================================================

char *all_tests() {
    mu_run_test(test_ObjectFile_create_destroy);
    mu_run_test(test_ObjectFile_open);
    return NULL;
}

RUN_TESTS(all_tests)