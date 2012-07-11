#include <stdio.h>
#include <stdlib.h>

#include <action_file.h>
#include <mem.h>
#include <bstring.h>

#include "minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Path
//--------------------------------------

int test_sky_action_file_path() {
    int rc;
    struct tagbstring path = bsStatic("/dev/null");

    sky_action_file *action_file = sky_action_file_create();
    rc = sky_action_file_set_path(action_file, &path);
    mu_assert_int_equals(rc, 0);
    mu_assert_bstring(action_file->path, "/dev/null");
    
    bstring ret;
    rc = sky_action_file_get_path(action_file, &ret);
    mu_assert_int_equals(rc, 0);
    mu_assert_bstring(ret, "/dev/null");

    sky_action_file_free(action_file);
    bdestroy(ret);
    return 0;
}


//--------------------------------------
// Save
//--------------------------------------

int test_sky_action_file_save() {
    int rc;
    struct tagbstring path = bsStatic("tmp/actions");
    
    // Initialize action file.
    sky_action_file *action_file = sky_action_file_create();
    sky_action_file_set_path(action_file, &path);
    
    // Action 1
    sky_action *action1 = sky_action_create();
    action1->name = bfromcstr("foo");
    rc = sky_action_file_add_action(action_file, action1);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(action_file->action_count, 1);

    // Action 2
    sky_action *action2 = sky_action_create();
    action2->name = bfromcstr("this_is_a_really_long_action_name_woohoo");
    rc = sky_action_file_add_action(action_file, action2);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(action_file->action_count, 2);

    // Save
    rc = sky_action_file_save(action_file);
    mu_assert_int_equals(rc, 0);
    mu_assert_file("tmp/actions", "tests/fixtures/action_files/0");

    sky_action_file_free(action_file);
    return 0;
}


//--------------------------------------
// Load
//--------------------------------------

int test_sky_action_file_load() {
    int rc;
    struct tagbstring path = bsStatic("tests/fixtures/action_files/0");
    
    // Initialize and load action file.
    sky_action_file *action_file = sky_action_file_create();
    sky_action_file_set_path(action_file, &path);
    rc = sky_action_file_load(action_file);
    mu_assert_int_equals(rc, 0);

    // Assert actions.
    mu_assert_int_equals(action_file->action_count, 2);

    sky_action_file_free(action_file);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_action_file_path);
    mu_run_test(test_sky_action_file_save);
    mu_run_test(test_sky_action_file_load);
    return 0;
}

RUN_TESTS()