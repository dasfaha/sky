#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <block.h>

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


// Creates a block containing several paths.
Block *create_test_block0()
{
    Event *event;
    BlockInfo info;

    Database *database = Database_create(&dbpath);
    ObjectFile *object_file = ObjectFile_create(database, &objname);
    object_file->block_size = 0x10000;  // 64K

    Block *block = Block_create(object_file, info);

    // Path 2 (len=17)
    event = Event_create(946692000000000LL, 11, 0);
    Event_set_data(event, 1, &bar);
    Block_add_event(block, event);

    // Path 1 (len=46)
    event = Event_create(946684800000000LL, 10, 6);
    Block_add_event(block, event);
    event = Event_create(946688400000000LL, 10, 7);
    Event_set_data(event, 1, &foo);
    Block_add_event(block, event);
    
    return block;
}


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int test_Block_create() {
    Block *block = Block_create();
    mu_assert(block != NULL, "Unable to allocate block");
    Block_destroy(block);
    return 0;
}


//--------------------------------------
// Event management
//--------------------------------------

int test_Block_add_remove_events() {
    Block *block = create_test_block0();
    Event *event;

    // Check order of paths (based on object id).
    mu_assert(block->path_count == 2, "");
    mu_assert(block->paths[0]->object_id == 10, "");
    mu_assert(block->paths[1]->object_id == 11, "");

    // Remove event on path with single event.
    event = block->paths[1]->events[0];
    Block_remove_event(block, event);
    mu_assert(block->path_count == 1, "");
    mu_assert(block->paths[0]->object_id == 10, "");
    Event_destroy(event);
    
    // Remove event on path with multiple events.
    event = block->paths[0]->events[0];
    Block_remove_event(block, event);
    mu_assert(block->path_count == 1, "");
    mu_assert(block->paths[0]->object_id == 10, "");
    mu_assert(block->paths[0]->event_count == 1, "");
    mu_assert(block->paths[0]->events[0]->timestamp == 946688400000000LL, "");
    Event_destroy(event);
    
    // Clean up.
    Block_destroy(block);

    return 0;
}


//--------------------------------------
// Serialization length
//--------------------------------------

int test_Block_get_serialized_length() {
    Block *block = create_test_block0();
    mu_assert(Block_get_serialized_length(block) == 79, "");
    Block_destroy(block);
    return 0;
}


//--------------------------------------
// Serialization
//--------------------------------------

int test_Block_serialize() {
    FILE *file = fopen(TEMPFILE, "w");
    Block *block = create_test_block0();
    Block_serialize(block, file);
    Block_destroy(block);
    fclose(file);
    mu_assert_tempfile("tests/fixtures/serialization/block");
    return 0;
}

//--------------------------------------
// Deserialization
//--------------------------------------

int test_Block_deserialize() {
    EventData *data;
    Path *path;
    BlockInfo info;

    Database *database = Database_create(&dbpath);
    ObjectFile *object_file = ObjectFile_create(database, &objname);
    object_file->block_size = 0x10000;  // 64K

    FILE *file = fopen("tests/fixtures/serialization/block", "r");
    Block *block = Block_create(object_file, info);
    Block_deserialize(block, file);
    fclose(file);

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

    Event_get_data(path->events[1], 1, &data);
    mu_assert(biseqcstr(data->value, "foo"), "");

    // Path 2
    path = block->paths[1];
    mu_assert(path->object_id == 11, "");
    mu_assert(path->event_count == 1, "");

    mu_assert(path->events[0]->timestamp == 946692000000000LL, "");
    mu_assert(path->events[0]->object_id == 11, "");
    mu_assert(path->events[0]->action_id == 0, "");

    Event_get_data(path->events[0], 1, &data);
    mu_assert(biseqcstr(data->value, "bar"), "");

    Path_destroy(path);
    
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_Block_create);
    mu_run_test(test_Block_add_remove_events);
    mu_run_test(test_Block_get_serialized_length);
    mu_run_test(test_Block_serialize);
    mu_run_test(test_Block_deserialize);

    return 0;
}

RUN_TESTS()