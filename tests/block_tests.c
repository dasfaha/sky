#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <block.h>
#include <mem.h>

#include "minunit.h"


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring dbpath = bsStatic("/tmp/sky");
struct tagbstring objname = bsStatic("users");

struct tagbstring foo = bsStatic("foo");
struct tagbstring bar = bsStatic("bar");
struct tagbstring baz = bsStatic("baz");

int DATA_LENGTH = 128;
char DATA[] = {
    0x02, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00,
    0x01, 0x00, 0xE0, 0x37, 0x3B, 0x01, 0x5D, 0x03,
    0x00, 0x06, 0x00, 0x00, 0x00, 0x03, 0x00, 0x84,
    0xCB, 0x11, 0x02, 0x5D, 0x03, 0x00, 0x07, 0x00,
    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x03, 0x66,
    0x6F, 0x6F, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x02, 0x00,
    0x28, 0x5F, 0xE8, 0x02, 0x5D, 0x03, 0x00, 0x06,
    0x00, 0x01, 0x00, 0x03, 0x62, 0x61, 0x72, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


// Creates a block containing several paths.
sky_block *create_test_block0()
{
    sky_event *event;
    sky_block_info info;

    sky_database *database = sky_database_create(&dbpath);
    sky_table *table = sky_table_create(database, &objname);
    table->block_size = 0x10000;  // 64K

    sky_block *block = sky_block_create(table, &info);

    // Path 2 (len=17)
    event = sky_event_create(946692000000000LL, 11, 0);
    sky_event_set_data(event, 1, &bar);
    sky_block_add_event(block, event);

    // Path 1 (len=46)
    event = sky_event_create(946684800000000LL, 10, 6);
    sky_block_add_event(block, event);
    event = sky_event_create(946688400000000LL, 10, 7);
    sky_event_set_data(event, 1, &foo);
    sky_block_add_event(block, event);
    
    return block;
}


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Event management
//--------------------------------------

int test_sky_block_add_remove_events() {
    sky_block *block = create_test_block0();
    sky_event *event;

    // Check order of paths (based on object id).
    mu_assert(block->path_count == 2, "");
    mu_assert(block->paths[0]->object_id == 10, "");
    mu_assert(block->paths[1]->object_id == 11, "");

    // Remove event on path with single event.
    event = block->paths[1]->events[0];
    sky_block_remove_event(block, event);
    mu_assert(block->path_count == 1, "");
    mu_assert(block->paths[0]->object_id == 10, "");
    sky_event_free(event);
    
    // Remove event on path with multiple events.
    event = block->paths[0]->events[0];
    sky_block_remove_event(block, event);
    mu_assert(block->path_count == 1, "");
    mu_assert(block->paths[0]->object_id == 10, "");
    mu_assert(block->paths[0]->event_count == 1, "");
    mu_assert(block->paths[0]->events[0]->timestamp == 946688400000000LL, "");
    sky_event_free(event);
    
    // Clean up.
    sky_block_free(block);

    return 0;
}


//--------------------------------------
// Serialization length
//--------------------------------------

int test_sky_block_get_serialized_length() {
    sky_block *block = create_test_block0();
    mu_assert(sky_block_get_serialized_length(block) == 73, "");
    sky_block_free(block);
    return 0;
}


//--------------------------------------
// Serialization
//--------------------------------------

int test_sky_block_serialize() {
    ptrdiff_t ptrdiff;
    void *addr = calloc(DATA_LENGTH, 1);
    sky_block *block = create_test_block0();
    sky_block_serialize(block, addr, &ptrdiff);
    sky_block_free(block);
    mu_assert(ptrdiff == 73, "");
    memdump(addr, DATA_LENGTH);
    mu_assert(memcmp(addr, &DATA, DATA_LENGTH) == 0, "");
    free(addr);
    return 0;
}

//--------------------------------------
// Deserialization
//--------------------------------------

int test_sky_block_deserialize() {
    ptrdiff_t ptrdiff;

    sky_event_data *data;
    sky_path *path;
    sky_block_info info;

    sky_database *database = sky_database_create(&dbpath);
    sky_table *table = sky_table_create(database, &objname);
    table->block_size = 0x10000;  // 64K

    sky_block *block = sky_block_create(table, &info);
    sky_block_deserialize(block, &DATA, &ptrdiff);

    mu_assert(ptrdiff == 79, "");

    // Block
    mu_assert(block->path_count == 2, "");

    // Path 1
    path = block->paths[0];
    mu_assert(path->object_id == 10, "");
    mu_assert(path->event_count == 2, "");

    mu_assert(path->events[0]->timestamp == 946684800000000LL, "");
    mu_assert(path->events[0]->object_id == 10, "");
    mu_assert(path->events[0]->action_id == 6, "");

    mu_assert(path->events[1]->timestamp == 946688400000000LL, "");
    mu_assert(path->events[1]->object_id == 10, "");
    mu_assert(path->events[1]->action_id == 7, "");

    sky_event_get_data(path->events[1], 1, &data);
    mu_assert(biseqcstr(data->value, "foo"), "");

    // Path 2
    path = block->paths[1];
    mu_assert(path->object_id == 11, "");
    mu_assert(path->event_count == 1, "");

    mu_assert(path->events[0]->timestamp == 946692000000000LL, "");
    mu_assert(path->events[0]->object_id == 11, "");
    mu_assert(path->events[0]->action_id == 0, "");

    sky_event_get_data(path->events[0], 1, &data);
    mu_assert(biseqcstr(data->value, "bar"), "");

    sky_path_free(path);
    
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_block_add_remove_events);
    mu_run_test(test_sky_block_get_serialized_length);
    mu_run_test(test_sky_block_serialize);
    mu_run_test(test_sky_block_deserialize);

    return 0;
}

RUN_TESTS()