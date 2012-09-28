#include <stdio.h>
#include <stdlib.h>

#include <message_header.h>
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

int test_sky_message_header_pack() {
    cleantmp();
    sky_message_header *header = sky_message_header_create();
    header->version = 1;
    header->name = bfromcstr("eadd");
    header->length = 10;
    header->database_name = bfromcstr("foo");
    header->table_name = bfromcstr("bar");
    
    FILE *file = fopen("tmp/message", "w");
    mu_assert_bool(sky_message_header_pack(header, file) == 0);
    fclose(file);
    mu_assert_file("tmp/message", "tests/fixtures/message_header/0/message");
    sky_message_header_free(header);
    return 0;
}

int test_sky_message_header_unpack() {
    FILE *file = fopen("tests/fixtures/message_header/0/message", "r");
    sky_message_header *header = sky_message_header_create();
    mu_assert_bool(sky_message_header_unpack(header, file) == 0);
    fclose(file);

    mu_assert_int64_equals(header->version, 1LL);
    mu_assert_bstring(header->name, "eadd");
    mu_assert_int64_equals(header->length, 10LL);
    mu_assert_bstring(header->database_name, "foo");
    mu_assert_bstring(header->table_name, "bar");
    sky_message_header_free(header);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_message_header_pack);
    mu_run_test(test_sky_message_header_unpack);
    return 0;
}

RUN_TESTS()