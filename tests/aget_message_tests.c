#include <stdio.h>
#include <stdlib.h>

#include <aget_message.h>
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

int test_sky_aget_message_pack() {
    cleantmp();
    sky_aget_message *message = sky_aget_message_create();
    message->action_id = 20;
    
    FILE *file = fopen("tmp/message", "w");
    mu_assert_bool(sky_aget_message_pack(message, file) == 0);
    fclose(file);
    mu_assert_file("tmp/message", "tests/fixtures/aget_message/0/message");
    sky_aget_message_free(message);
    return 0;
}

int test_sky_aget_message_unpack() {
    FILE *file = fopen("tests/fixtures/aget_message/0/message", "r");
    sky_aget_message *message = sky_aget_message_create();
    mu_assert_bool(sky_aget_message_unpack(message, file) == 0);
    fclose(file);

    mu_assert_int_equals(message->action_id, 20);
    sky_aget_message_free(message);
    return 0;
}


//--------------------------------------
// Processing
//--------------------------------------

int test_sky_aget_message_process() {
    loadtmp("tests/fixtures/aget_message/1/table");
    sky_table *table = sky_table_create();
    table->path = bfromcstr("tmp");
    sky_table_open(table);
    
    sky_aget_message *message = sky_aget_message_create();
    message->action_id = 1;

    FILE *output = fopen("tmp/output", "w");
    mu_assert(sky_aget_message_process(message, table, output) == 0, "");
    fclose(output);
    mu_assert_file("tmp/actions", "tests/fixtures/aget_message/1/table/actions");
    mu_assert_file("tmp/output", "tests/fixtures/aget_message/1/output");

    sky_aget_message_free(message);
    sky_table_free(table);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_aget_message_pack);
    mu_run_test(test_sky_aget_message_unpack);
    mu_run_test(test_sky_aget_message_process);
    return 0;
}

RUN_TESTS()