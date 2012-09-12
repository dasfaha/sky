#include <stdio.h>
#include <stdlib.h>

#include <eadd_message.h>
#include <mem.h>

#include "minunit.h"


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring SERVER_ROOT = bsStatic("tmp");
struct tagbstring OBJECT_TYPE = bsStatic("users");

char EADD_MESSAGE[] = 
    "\x00\x01"                          // Version
    "\x00\x01\x00\x01"                  // Type
    "\x00\x00\x00\x25"                  // Length
    "\x02" "db"                         // Database Name
    "\x05" "users"                      // Table Name
    "\x00\x00\x00\x14"                  // Object ID
    "\x00\x00\x01\x34\x96\x90\xD0\x00"  // Timestamp
    "\x00\x01"                          // Action ID #1
;


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Serialization
//--------------------------------------

int test_sky_eadd_message_pack() {
    cleantmp();
    sky_eadd_message *message = sky_eadd_message_create();
    message->object_id = 10;
    message->timestamp = 1000;
    message->action_id = 20;
    
    FILE *file = fopen("tmp/message", "w");
    mu_assert_bool(sky_eadd_message_pack(message, file) == 0);
    fclose(file);
    mu_assert_file("tmp/message", "tests/fixtures/eadd_message/0/message");
    sky_eadd_message_free(message);
    return 0;
}

int test_sky_eadd_message_unpack() {
    FILE *file = fopen("tests/fixtures/eadd_message/0/message", "r");
    sky_eadd_message *message = sky_eadd_message_create();
    mu_assert_bool(sky_eadd_message_unpack(message, file) == 0);
    fclose(file);

    mu_assert_int_equals(message->object_id, 10);
    mu_assert_int64_equals(message->timestamp, 1000LL);
    mu_assert_int_equals(message->action_id, 20);
    sky_eadd_message_free(message);
    return 0;
}


//--------------------------------------
// Processing
//--------------------------------------

int test_sky_eadd_message_process() {
    cleantmp();
    sky_table *table = sky_table_create();
    table->path = bfromcstr("tmp");
    sky_table_open(table);
    FILE *output = fopen("tmp/output", "w");
    
    sky_eadd_message *message = sky_eadd_message_create();
    message->object_id = 10;
    message->timestamp = 1000LL;
    message->action_id = 20;

    mu_assert(sky_eadd_message_process(message, table, output) == 0, "");
    fclose(output);
    mu_assert_file("tmp/0/header", "tests/fixtures/eadd_message/1/table/0/header");
    mu_assert_file("tmp/0/data", "tests/fixtures/eadd_message/1/table/0/data");

    sky_eadd_message_free(message);
    sky_table_free(table);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_eadd_message_pack);
    mu_run_test(test_sky_eadd_message_unpack);
    mu_run_test(test_sky_eadd_message_process);
    return 0;
}

RUN_TESTS()