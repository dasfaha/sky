#include <stdio.h>
#include <stdlib.h>

#include <peach_message.h>
#include <mem.h>
#include <dbg.h>

#include "minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Serialization
//--------------------------------------

int test_sky_peach_message_pack() {
    cleantmp();
    sky_peach_message *message = sky_peach_message_create();
    message->query = bfromcstr("class Foo{ public Int x; }");
    
    FILE *file = fopen("tmp/message", "w");
    mu_assert_bool(sky_peach_message_pack(message, file) == 0);
    fclose(file);
    mu_assert_file("tmp/message", "tests/fixtures/peach_message/0/message");
    sky_peach_message_free(message);
    return 0;
}

int test_sky_peach_message_unpack() {
    FILE *file = fopen("tests/fixtures/peach_message/0/message", "r");
    sky_peach_message *message = sky_peach_message_create();
    mu_assert_bool(sky_peach_message_unpack(message, file) == 0);
    fclose(file);

    mu_assert_bstring(message->query, "class Foo{ public Int x; }");
    sky_peach_message_free(message);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_peach_message_pack);
    mu_run_test(test_sky_peach_message_unpack);
    return 0;
}

RUN_TESTS()