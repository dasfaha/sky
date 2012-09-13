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


//--------------------------------------
// Processing
//--------------------------------------

int test_sky_peach_message_process() {
    loadtmp("tests/fixtures/peach_message/1/table");
    sky_table *table = sky_table_create();
    table->path = bfromcstr("tmp");
    sky_table_open(table);
    
    // NOTE: The table contains two properties: foo (String) and this_is_a_really...(Int)
    sky_peach_message *message = sky_peach_message_create();
    message->query = bfromcstr(
        "[Hashable(\"id\")]\n"
        "[Serializable]\n"
        "class Result {\n"
        "  public Int id;\n"
        "  public Int count;\n"
        "}\n"
        "Cursor cursor = path.events();\n"
        "for each (Event event in cursor) {\n"
        "  Int dynamic_prop1 = event.foo;\n"
        "  String dynamic_prop2 = event.this_is_a_really_long_property_name_woohoo;\n"
        "  Result item = data.get(event.actionId);\n"
        "  item.count = item.count + 1;\n"
        "}\n"
        "return;"
    );

    FILE *output = fopen("tmp/output", "w");
    mu_assert(sky_peach_message_process(message, table, output) == 0, "");
    fclose(output);
    mu_assert_file("tmp/output", "tests/fixtures/peach_message/1/output");

    sky_peach_message_free(message);
    sky_table_free(table);
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
    mu_run_test(test_sky_peach_message_process);
    return 0;
}

RUN_TESTS()