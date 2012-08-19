#include <stdio.h>
#include <stdlib.h>

#include <server.h>
#include <mem.h>

#include "minunit.h"


//==============================================================================
//
// Fixtures
//
//==============================================================================

char PEACH_MESSAGE[] = 
    "\x00\x01"                          // Version
    "\x00\x02\x00\x06"                  // Type
    "\x00\x00\x00\xF7"                  // Length
    "\x02" "db"                         // Database Name
    "\x05" "users"                      // Table Name
    "\x00\x00\x00\xEA"                  // Query Length
    "[Hashable(\"id\")]\n"              // Query
    "[Serializable]\n"
    "class Result {\n"
    "  public Int id;\n"
    "  public Int count;\n"
    "}\n"
    "Cursor cursor = path.events();\n"
    "for each (Event event in cursor) {\n"
    "  Result item = data.get(event.actionId);\n"
    "  item.count = item.count + 1;\n"
    "}\n"
    "return;"
;


//==============================================================================
//
// Test Cases
//
//==============================================================================

int test_sky_server_process_peach_message() {
    int socket = 1;
    struct tagbstring root = bsStatic("tmp");
    loadtmp("tests/fixtures/server/peach/0");
    sky_server *server = sky_server_create(&root);
    mu_assert(sky_server_process_peach_message(server, socket, &PEACH_MESSAGE) == 0, "");
    sky_server_free(server);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_server_process_peach_message);
    return 0;
}

RUN_TESTS()