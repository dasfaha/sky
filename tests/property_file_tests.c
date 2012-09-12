#include <stdio.h>
#include <stdlib.h>

#include <property_file.h>
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

int test_sky_property_file_path() {
    int rc;
    struct tagbstring path = bsStatic("/dev/null");

    sky_property_file *property_file = sky_property_file_create();
    rc = sky_property_file_set_path(property_file, &path);
    mu_assert_int_equals(rc, 0);
    mu_assert_bstring(property_file->path, "/dev/null");
    
    bstring ret;
    rc = sky_property_file_get_path(property_file, &ret);
    mu_assert_int_equals(rc, 0);
    mu_assert_bstring(ret, "/dev/null");

    sky_property_file_free(property_file);
    bdestroy(ret);
    return 0;
}


//--------------------------------------
// Save
//--------------------------------------

int test_sky_property_file_save() {
    int rc;
    struct tagbstring path = bsStatic("tmp/properties");
    
    // Initialize property file.
    sky_property_file *property_file = sky_property_file_create();
    sky_property_file_set_path(property_file, &path);
    
    // Action 1
    sky_property *property1 = sky_property_create();
    property1->type = SKY_PROPERTY_TYPE_OBJECT;
    property1->data_type = bfromcstr("Int");
    property1->name = bfromcstr("foo");
    rc = sky_property_file_add_property(property_file, property1);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(property_file->property_count, 1);

    // Action 2
    sky_property *property2 = sky_property_create();
    property2->type = SKY_PROPERTY_TYPE_ACTION;
    property2->data_type = bfromcstr("String");
    property2->name = bfromcstr("this_is_a_really_long_property_name_woohoo");
    rc = sky_property_file_add_property(property_file, property2);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(property_file->property_count, 2);

    // Save
    rc = sky_property_file_save(property_file);
    mu_assert_int_equals(rc, 0);
    mu_assert_file("tmp/properties", "tests/fixtures/property_files/0/properties");

    sky_property_file_free(property_file);
    return 0;
}


//--------------------------------------
// Load
//--------------------------------------

int test_sky_property_file_load() {
    int rc;
    struct tagbstring path = bsStatic("tests/fixtures/property_files/0/properties");
    
    // Initialize and load property file.
    sky_property_file *property_file = sky_property_file_create();
    sky_property_file_set_path(property_file, &path);
    rc = sky_property_file_load(property_file);
    mu_assert_int_equals(rc, 0);

    // Assert properties.
    mu_assert_int_equals(property_file->property_count, 2);

    sky_property_file_free(property_file);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_property_file_path);
    mu_run_test(test_sky_property_file_save);
    mu_run_test(test_sky_property_file_load);
    return 0;
}

RUN_TESTS()