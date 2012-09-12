#include <stdio.h>
#include <stdlib.h>

#include <padd_message.h>
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

int test_sky_padd_message_pack() {
    cleantmp();
    sky_padd_message *message = sky_padd_message_create();
    message->property->type = SKY_PROPERTY_TYPE_OBJECT;
    message->property->data_type = bfromcstr("Int");
    message->property->name = bfromcstr("foo");
    
    FILE *file = fopen("tmp/message", "w");
    mu_assert_bool(sky_padd_message_pack(message, file) == 0);
    fclose(file);
    mu_assert_file("tmp/message", "tests/fixtures/padd_message/0/message");
    sky_padd_message_free(message);
    return 0;
}

int test_sky_padd_message_unpack() {
    FILE *file = fopen("tests/fixtures/padd_message/0/message", "r");
    sky_padd_message *message = sky_padd_message_create();
    mu_assert_bool(sky_padd_message_unpack(message, file) == 0);
    fclose(file);

    mu_assert_bool(message->property->type == SKY_PROPERTY_TYPE_OBJECT);
    mu_assert_bstring(message->property->data_type, "Int");
    mu_assert_bstring(message->property->name, "foo");
    sky_padd_message_free(message);
    return 0;
}


//--------------------------------------
// Processing
//--------------------------------------

int test_sky_padd_message_process() {
    cleantmp();
    sky_table *table = sky_table_create();
    table->path = bfromcstr("tmp");
    sky_table_open(table);
    
    sky_padd_message *message = sky_padd_message_create();
    message->property->type = SKY_PROPERTY_TYPE_OBJECT;
    message->property->data_type = bfromcstr("Int");
    message->property->name = bfromcstr("foo");

    FILE *output = fopen("tmp/output", "w");
    mu_assert(sky_padd_message_process(message, table, output) == 0, "");
    fclose(output);
    mu_assert_file("tmp/properties", "tests/fixtures/padd_message/1/table/properties");
    mu_assert_file("tmp/output", "tests/fixtures/padd_message/1/output");

    sky_padd_message_free(message);
    sky_table_free(table);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_padd_message_pack);
    mu_run_test(test_sky_padd_message_unpack);
    mu_run_test(test_sky_padd_message_process);
    return 0;
}

RUN_TESTS()