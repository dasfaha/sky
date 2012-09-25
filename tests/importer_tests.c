#include <stdio.h>
#include <stdlib.h>

#include <importer.h>
#include "minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Import
//--------------------------------------

int test_sky_importer_import() {
    cleantmp();
    sky_importer *importer = sky_importer_create();
    importer->path = bfromcstr("tmp");
    
    FILE *file = fopen("tests/fixtures/importer/0/data.json", "r");
    int rc = sky_importer_import(importer, file);
    mu_assert_int_equals(rc, 0);
    fclose(file);
    
    // Validate.
    mu_assert_bool(importer->table != NULL);
    mu_assert_int_equals(importer->table->default_block_size, 128);

    mu_assert_int_equals(importer->table->action_file->action_count, 2);
    mu_assert_int_equals(importer->table->action_file->actions[0]->id, 1);
    mu_assert_bstring(importer->table->action_file->actions[0]->name, "hello");
    mu_assert_int_equals(importer->table->action_file->actions[1]->id, 2);
    mu_assert_bstring(importer->table->action_file->actions[1]->name, "goodbye");
    
    mu_assert_int_equals(importer->table->property_file->property_count, 3);
    mu_assert_int_equals(importer->table->property_file->properties[0]->id, 1);
    mu_assert_bool(importer->table->property_file->properties[0]->type == SKY_PROPERTY_TYPE_OBJECT);
    mu_assert_bstring(importer->table->property_file->properties[0]->data_type, "Int");
    mu_assert_bstring(importer->table->property_file->properties[0]->name, "myInt");
    mu_assert_int_equals(importer->table->property_file->properties[1]->id, -1);
    mu_assert_bool(importer->table->property_file->properties[1]->type == SKY_PROPERTY_TYPE_ACTION);
    mu_assert_bstring(importer->table->property_file->properties[1]->data_type, "String");
    mu_assert_bstring(importer->table->property_file->properties[1]->name, "myString");
    mu_assert_int_equals(importer->table->property_file->properties[2]->id, 2);
    mu_assert_bool(importer->table->property_file->properties[2]->type == SKY_PROPERTY_TYPE_OBJECT);
    mu_assert_bstring(importer->table->property_file->properties[2]->data_type, "Boolean");
    mu_assert_bstring(importer->table->property_file->properties[2]->name, "myBoolean");

    sky_importer_free(importer);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_importer_import);
    return 0;
}

RUN_TESTS()