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
// EADD
//--------------------------------------

int test_sky_server_process_eadd_message() {
    cleantmp();
    int socket = 1;
    sky_server *server = sky_server_create(&SERVER_ROOT);
    mu_assert(sky_server_process_eadd_message(server, socket, &EADD_MESSAGE) == 0, "");
    mu_assert_file("tmp/db/users/0/header", "tests/fixtures/server/eadd/0/db/users/0/header");
    mu_assert_file("tmp/db/users/0/data", "tests/fixtures/server/eadd/0/db/users/0/data");
    sky_server_free(server);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_server_process_eadd_message);
    return 0;
}

RUN_TESTS()