#include <stdio.h>
#include <stdlib.h>

#include <pall_message.h>
#include <mem.h>

#include "minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Serialization
//--------------------------------------

int test_sky_pall_message_pack() {
    cleantmp();
    sky_pall_message *message = sky_pall_message_create();
    
    FILE *file = fopen("tmp/message", "w");
    mu_assert_bool(sky_pall_message_pack(message, file) == 0);
    fclose(file);
    mu_assert_file("tmp/message", "tests/fixtures/pall_message/0/message");
    sky_pall_message_free(message);
    return 0;
}

int test_sky_pall_message_unpack() {
    FILE *file = fopen("tests/fixtures/pall_message/0/message", "r");
    sky_pall_message *message = sky_pall_message_create();
    mu_assert_bool(sky_pall_message_unpack(message, file) == 0);
    fclose(file);

    sky_pall_message_free(message);
    return 0;
}


//--------------------------------------
// Processing
//--------------------------------------

int test_sky_pall_message_process() {
    loadtmp("tests/fixtures/pall_message/1/table");
    sky_table *table = sky_table_create();
    table->path = bfromcstr("tmp");
    sky_table_open(table);
    
    sky_pall_message *message = sky_pall_message_create();

    FILE *output = fopen("tmp/output", "w");
    mu_assert(sky_pall_message_process(message, table, output) == 0, "");
    fclose(output);
    mu_assert_file("tmp/output", "tests/fixtures/pall_message/1/output");

    sky_pall_message_free(message);
    sky_table_free(table);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_pall_message_pack);
    mu_run_test(test_sky_pall_message_unpack);
    mu_run_test(test_sky_pall_message_process);
    return 0;
}

RUN_TESTS()