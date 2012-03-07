#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <database.h>
#include <object_file.h>
#include <bstring.h>

#include "minunit.h"

//==============================================================================
//
// Constants
//
//==============================================================================

struct tagbstring ROOT = bsStatic("tmp/db");
struct tagbstring OBJECT_TYPE = bsStatic("users");


//==============================================================================
//
// Test Cases
//
//==============================================================================

char *test_ObjectFile_open() {
    struct stat buffer;
    int rc;
    
    copydb("simple");
    
    Database *database = Database_create(&ROOT);
    ObjectFile *object_file = ObjectFile_create(database, &OBJECT_TYPE);
    mu_assert(object_file->state == OBJECT_FILE_STATE_CLOSED, "Expected state initialize as closed");

    rc = ObjectFile_open(object_file);
    mu_assert(rc == 0, "Object file could not be opened");

    mu_assert(object_file->state == OBJECT_FILE_STATE_OPEN, "Expected state to be open");
    mu_assert(object_file->block_count == 9, "Expected 9 blocks");

    rc = ObjectFile_lock(object_file);
    mu_assert(rc == 0, "Object file could not be locked");
    mu_assert(object_file->state == OBJECT_FILE_STATE_LOCKED, "Expected state to be locked");

    // Verify lock file.
    rc = stat("tmp/db/users/.lock", &buffer);
    mu_assert(rc == 0, "Expected lock file to exist");

    // Verify block info.
    mu_assert_block_info(0, 1, 1, 3);
    mu_assert_block_info(1, 8, 4, 5);
    mu_assert_block_info(2, 0, 6, 6);
    mu_assert_block_info(3, 3, 6, 6);
    mu_assert_block_info(4, 5, 6, 6);
    mu_assert_block_info(5, 2, 7, 9);
    mu_assert_block_info(6, 4, 10, 10);
    mu_assert_block_info(7, 6, 10, 10);
    mu_assert_block_info(8, 7, 10, 10);

    // Verify actions.
    mu_assert(object_file->action_count == 3, "Expected 3 actions");
    mu_assert_action(0, 1, "home_page");
    mu_assert_action(1, 2, "sign_up");
    mu_assert_action(2, 3, "sign_in");

    // Verify properties.
    mu_assert(object_file->property_count == 3, "Expected 3 properties");
    mu_assert_property(0, 1, "first_name");
    mu_assert_property(1, 2, "last_name");
    mu_assert_property(2, 3, "salary");

    rc = ObjectFile_unlock(object_file);
    mu_assert(rc == 0, "Object file could not be unlocked");
    mu_assert(object_file->state == OBJECT_FILE_STATE_OPEN, "Expected state to be open after unlock");

    // Verify lock is gone.
    rc = stat("tmp/db/users/.lock", &buffer);
    mu_assert(rc == -1, "Expected lock file to not exist");
    mu_assert(errno == ENOENT, "Expected stat error on lock file to be ENOENT");

    rc = ObjectFile_close(object_file);
    mu_assert(rc == 0, "Object file could not be closed");
    mu_assert(object_file->state == OBJECT_FILE_STATE_CLOSED, "Expected state to be closed");

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
    mu_run_test(test_ObjectFile_open);
    return NULL;
}

RUN_TESTS(all_tests)