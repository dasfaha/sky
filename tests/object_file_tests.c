#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <database.h>
#include <object_file.h>
#include <bstring.h>

#include "minunit.h"

//==============================================================================
//
// Helpers
//
//==============================================================================

#define mu_block_info_assert(INDEX, ID, MIN_OBJECT_ID, MAX_OBJECT_ID) \
mu_assert(object_file->infos[INDEX].id == ID, "Block " #INDEX " id expected to be " #ID); \
mu_assert(object_file->infos[INDEX].min_object_id == MIN_OBJECT_ID, "Block " #INDEX " min object id expected to be " #MIN_OBJECT_ID); \
mu_assert(object_file->infos[INDEX].max_object_id == MAX_OBJECT_ID, "Block " #INDEX " max object id expected to be " #MAX_OBJECT_ID);



//==============================================================================
//
// Test Cases
//
//==============================================================================

char *test_ObjectFile_create_destroy() {
    struct tagbstring root = bsStatic("/etc/sky/data");
    struct tagbstring object_type = bsStatic("users");
    Database *database = Database_create(&root);
    ObjectFile *object_file = ObjectFile_create(database, &object_type);

    mu_assert(object_file != NULL, "Could not create object file");
    mu_assert(biseqcstr(object_file->name, "users") == 1, "Invalid name");

    ObjectFile_destroy(object_file);
    Database_destroy(database);

    return NULL;
}

char *test_ObjectFile_open_header() {
    int rc = 0;
    
    char *cwd = getcwd(NULL, 0);
    bstring path = bformat("%s/tests/fixtures/db/simple", cwd);
    struct tagbstring object_type = bsStatic("users");

    Database *database = Database_create(path);
    ObjectFile *object_file = ObjectFile_create(database, &object_type);
    rc = ObjectFile_open(object_file);

    mu_assert(rc == 0, "Object file could not be opened");
    mu_assert(object_file->block_count == 4, "Expected 4 blocks");

    mu_block_info_assert(0, 0, 1, 3);
    mu_block_info_assert(1, 1, 4, 5);
    mu_block_info_assert(2, 2, 7, 10);
    mu_block_info_assert(3, 3, 11, 15);

    ObjectFile_close(object_file);

    ObjectFile_destroy(object_file);
    Database_destroy(database);

    bdestroy(path);
    free(cwd);

    return NULL;
}

char *test_ObjectFile_open_actions() {
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
    mu_assert(biseqcstr(object_file->actions[0].name, "home_page") == 1, "action[0].name");

    mu_assert(object_file->actions[1].id == 2, "Expected action[1].id == 2");
    mu_assert(biseqcstr(object_file->actions[1].name, "sign_up") == 1, "action[1].name");

    mu_assert(object_file->actions[2].id == 3, "Expected action[2].id == 3");
    mu_assert(biseqcstr(object_file->actions[2].name, "sign_in") == 1, "action[2].name");

    ObjectFile_close(object_file);

    ObjectFile_destroy(object_file);
    Database_destroy(database);

    bdestroy(path);
    free(cwd);

    return NULL;
}

char *test_ObjectFile_open_properties() {
    int rc = 0;
    
    char *cwd = getcwd(NULL, 0);
    bstring path = bformat("%s/tests/fixtures/db/simple", cwd);
    struct tagbstring object_type = bsStatic("users");

    Database *database = Database_create(path);
    ObjectFile *object_file = ObjectFile_create(database, &object_type);
    rc = ObjectFile_open(object_file);

    mu_assert(rc == 0, "Object file could not be opened");
    mu_assert(object_file->action_count == 3, "Expected 3 properties");

    mu_assert(object_file->properties[0].id == 1, "Expected properties[0].id == 1");
    mu_assert(biseqcstr(object_file->properties[0].name, "first_name") == 1, "properties[0].name");

    mu_assert(object_file->properties[1].id == 2, "Expected properties[1].id == 2");
    mu_assert(biseqcstr(object_file->properties[1].name, "last_name") == 1, "properties[0].name");

    mu_assert(object_file->properties[2].id == 3, "Expected properties[2].id == 3");
    mu_assert(biseqcstr(object_file->properties[2].name, "salary") == 1, "properties[0].name");

    ObjectFile_close(object_file);

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
    mu_run_test(test_ObjectFile_open_header);
    mu_run_test(test_ObjectFile_open_actions);
    mu_run_test(test_ObjectFile_open_properties);
    return NULL;
}

RUN_TESTS(all_tests)