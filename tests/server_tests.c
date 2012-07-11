#include <stdio.h>
#include <stdlib.h>

#include <server.h>
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
    "\x00\x00\x00\x3C"                  // Length (70 bytes)
    "\x02" "db"                         // Database Name
    "\x05" "users"                      // Table Name
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
// EADD
//--------------------------------------

int test_sky_server_process_eadd_message() {
    cleandb();
    int socket = 0;
    sky_server *server = sky_server_create(&SERVER_ROOT);
    mu_assert(sky_server_process_eadd_message(server, socket, &EADD_MESSAGE) == 0, "");
    mu_assert_file("tmp/db/users/actions", "tests/fixtures/db/server0/users/actions");
    mu_assert_file("tmp/db/users/properties", "tests/fixtures/db/server0/users/properties");
    mu_assert_file("tmp/db/users/header", "tests/fixtures/db/server0/users/header");
    mu_assert_file("tmp/db/users/data", "tests/fixtures/db/server0/users/data");
    sky_server_free(server);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    //mu_run_test(test_sky_server_process_eadd_message);
    return 0;
}

RUN_TESTS()