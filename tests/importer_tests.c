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
    mu_assert_file("tmp/actions", "tests/fixtures/importer/0/table/actions");
    mu_assert_file("tmp/properties", "tests/fixtures/importer/0/table/properties");
    //mu_assert_file("tmp/0/data", "tests/fixtures/importer/0/table/0/data");
    //mu_assert_file("tmp/0/header", "tests/fixtures/importer/0/table/0/header");

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