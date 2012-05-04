#include <stdio.h>
#include <stdlib.h>

#include <message.h>
#include <mem.h>

#include "minunit.h"

//==============================================================================
//
// Globals
//
//==============================================================================

char MESSAGE_HEADER[] =
    "\x00\x01"                 // Version
    "\x00\x01\x00\x01"         // Type
    "\x00\x10\x02\x03"         // Length
;

char EADD_MESSAGE[] = 
    "\x00\x01"                          // Version
    "\x00\x01\x00\x01"                  // Type
    "\x00\x00\x00\x3D"                  // Length (71 bytes)
    "\x03" "foo"                        // Database Name
    "\x05" "users"                      // Object File Name
    "\x00\x00\x00\x00\x00\x00\x00\x14"  // Object ID
    "\x00\x00\x01\x34\x96\x90\xD0\x00"  // Timestamp
    "\x00\x07" "sign up"                // Action Name
    "\x00\x02"                          // Data Count
    "\x00\x0A" "First Name"             // Data Key 1
    "\x04" "John"                       // Data Value 1
    "\x00\x09" "Last Name"              // Data Key 2
    "\x05" "Smith"                      // Data Value 2
;


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Header
//--------------------------------------

int test_sky_message_header_parse() {
    sky_message_header *header = sky_message_header_create();
    mu_assert(header != NULL, "");
    mu_assert(sky_message_header_parse(MESSAGE_HEADER, header) == 0, "");
    mu_assert(header->version == 1, "");
    mu_assert(header->type == 0x10001, "");
    mu_assert(header->length == 1049091, "");
    sky_message_header_free(header);
    return 0;
}


//--------------------------------------
// EADD
//--------------------------------------

int test_sky_eadd_message_parse() {
    sky_eadd_message *message = sky_eadd_message_create();
    mu_assert(message != NULL, "");
    mu_assert(sky_eadd_message_parse(EADD_MESSAGE, message) == 0, "");
    mu_assert(biseqcstr(message->database_name, "foo"), "");
    mu_assert(biseqcstr(message->object_file_name, "users"), "");
    mu_assert(message->object_id == 20, "");
    mu_assert(message->timestamp == 1325376000000LL, "");
    mu_assert(biseqcstr(message->action_name, "sign up"), "");
    mu_assert(message->data_count == 2, "");
    mu_assert(biseqcstr(message->data_keys[0], "First Name"), "");
    mu_assert(biseqcstr(message->data_values[0], "John"), "");
    mu_assert(biseqcstr(message->data_keys[1], "Last Name"), "");
    mu_assert(biseqcstr(message->data_values[1], "Smith"), "");
    sky_eadd_message_free(message);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_message_header_parse);
    mu_run_test(test_sky_eadd_message_parse);
    return 0;
}

RUN_TESTS()