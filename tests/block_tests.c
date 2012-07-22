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

char DATA[] = 
    "\x0a\x00\x00\x00\x14\x00\x00\x00\x1e\x00\x00\x00\x00\x00\x00\x00"
    "\x28\x00\x00\x00\x00\x00\x00\x00"
;


//==============================================================================
//
// Helpers
//
//==============================================================================

#define INIT_DATA_FILE(PATH) \
    loadtmp(PATH); \
    data_file = sky_data_file_create(); \
    data_file->path = bfromcstr("tmp/data"); \
    data_file->header_path = bfromcstr("tmp/header"); \
    sky_data_file_load(data_file);

#define ASSERT_PATH_STAT(PATH_STAT, OBJECT_ID, START_POS, END_POS, SZ) do { \
    mu_assert(PATH_STAT.block == NULL, ""); \
    mu_assert_int_equals(PATH_STAT.object_id, OBJECT_ID); \
    if(START_POS != -1) mu_assert_long_equals(PATH_STAT.start_ptr - data_file->data, START_POS); \
    if(END_POS != -1) mu_assert_long_equals(PATH_STAT.end_ptr - data_file->data, END_POS); \
    mu_assert_long_equals(PATH_STAT.sz, SZ); \
} while(0)


sky_block *create_block(sky_data_file *data_file, uint32_t index,
                        sky_object_id_t min_object_id,
                        sky_object_id_t max_object_id,
                        bool spanned)
{
    sky_block *block = sky_block_create(data_file);
    block->index = index;
    block->min_object_id = min_object_id;
    block->max_object_id = max_object_id;
    block->spanned = spanned;
    return block;
}

//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Serialization
//--------------------------------------

int test_sky_block_pack() {
    sky_block *block = sky_block_create(NULL);
    block->min_object_id = 10;
    block->max_object_id = 20;
    block->min_timestamp = 30LL;
    block->max_timestamp = 40LL;
    
    size_t sz;
    uint8_t *buffer[SKY_BLOCK_HEADER_SIZE];
    int rc = sky_block_pack(block, buffer, &sz);
    mu_assert_int_equals(rc, 0);
    mu_assert_long_equals(sz, SKY_BLOCK_HEADER_SIZE);
    mu_assert_mem(buffer, &DATA, SKY_BLOCK_HEADER_SIZE);
    sky_block_free(block);
    return 0;
}

int test_sky_block_unpack() {
    sky_block *block = sky_block_create(NULL);
    size_t sz;
    int rc = sky_block_unpack(block, &DATA, &sz);
    mu_assert_int_equals(rc, 0);
    mu_assert_long_equals(sz, SKY_BLOCK_HEADER_SIZE);
    mu_assert_int_equals(block->min_object_id, 10);
    mu_assert_int_equals(block->max_object_id, 20);
    mu_assert_int64_equals(block->min_timestamp, 30LL);
    mu_assert_int64_equals(block->max_timestamp, 40LL);
    sky_block_free(block);
    return 0;
}


//--------------------------------------
// Block Position
//--------------------------------------

int test_sky_block_get_offset() {
    sky_data_file *data_file = sky_data_file_create();
    data_file->block_size = 128;
    sky_block *block = sky_block_create(data_file);
    block->index = 3;
    
    size_t offset;
    int rc = sky_block_get_offset(block, &offset);
    mu_assert_int_equals(rc, 0);
    mu_assert_long_equals(offset, 384L);
    sky_data_file_free(data_file);
    sky_block_free(block);
    return 0;
}

int test_sky_block_get_ptr() {
    uint8_t x;
    sky_data_file *data_file = sky_data_file_create();
    data_file->block_size = 128;
    data_file->data = (void*)&x;
    sky_block *block = sky_block_create(data_file);
    block->index = 3;
    
    void *ptr;
    int rc = sky_block_get_ptr(block, &ptr);
    mu_assert_int_equals(rc, 0);
    mu_assert_long_equals(ptr-((void*)&x), 384L);
    sky_data_file_free(data_file);
    sky_block_free(block);
    return 0;
}


//--------------------------------------
// Spanning
//--------------------------------------

int test_sky_block_get_span_count() {
    int rc;
    uint32_t count;
    sky_data_file *data_file = sky_data_file_create();
    data_file->block_count = 5;
    data_file->blocks = malloc(sizeof(sky_block*) * data_file->block_count);
    data_file->blocks[0] = create_block(data_file, 0, 10LL, 11LL, false);
    data_file->blocks[1] = create_block(data_file, 1, 20LL, 20LL, true);
    data_file->blocks[2] = create_block(data_file, 2, 20LL, 20LL, true);
    data_file->blocks[3] = create_block(data_file, 3, 30LL, 30LL, true);
    data_file->blocks[4] = create_block(data_file, 4, 30LL, 30LL, true);

    // Unspanned blocks should return 1.
    rc = sky_block_get_span_count(data_file->blocks[0], &count);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(count, 1);

    // Test that spanned blocks return correct number.
    rc = sky_block_get_span_count(data_file->blocks[1], &count);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(count, 2);

    // Test that blocks at the end are returned correctly.
    rc = sky_block_get_span_count(data_file->blocks[3], &count);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(count, 2);

    sky_data_file_free(data_file);
    return 0;
}


//--------------------------------------
// Path Stats
//--------------------------------------

int test_sky_data_file_get_path_stats_with_no_event() {
    sky_data_file *data_file;
    INIT_DATA_FILE("tests/fixtures/blocks/path_stats/a");
    sky_block_path_stat *paths = NULL;
    uint32_t path_count = 0;
    int rc = sky_block_get_path_stats(data_file->blocks[0], NULL, &paths, &path_count);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(path_count, 2);
    ASSERT_PATH_STAT(paths[0], 3, 0L, 41L, 41L);
    ASSERT_PATH_STAT(paths[1], 10, 41L, 60L, 19L);
    sky_data_file_free(data_file);
    return 0;
}

int test_sky_data_file_get_path_stats_with_event_in_existing_path() {
    sky_data_file *data_file;
    INIT_DATA_FILE("tests/fixtures/blocks/path_stats/a");
    sky_block_path_stat *paths = NULL;
    uint32_t path_count = 0;
    sky_event *event = sky_event_create(3, 7LL, 20);
    int rc = sky_block_get_path_stats(data_file->blocks[0], event, &paths, &path_count);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(path_count, 2);
    ASSERT_PATH_STAT(paths[0], 3, 0L, 41L, 52L);
    ASSERT_PATH_STAT(paths[1], 10, 41L, 60L, 19L);
    sky_event_free(event);
    sky_data_file_free(data_file);
    return 0;
}

int test_sky_data_file_get_path_stats_with_event_in_new_starting_path() {
    sky_data_file *data_file;
    INIT_DATA_FILE("tests/fixtures/blocks/path_stats/a");
    sky_block_path_stat *paths = NULL;
    uint32_t path_count = 0;
    sky_event *event = sky_event_create(2, 7LL, 20);
    int rc = sky_block_get_path_stats(data_file->blocks[0], event, &paths, &path_count);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(path_count, 3);
    ASSERT_PATH_STAT(paths[0], 2, -1L, -1L, 19L);
    ASSERT_PATH_STAT(paths[1], 3, 0L, 41L, 41L);
    ASSERT_PATH_STAT(paths[2], 10, 41L, 60L, 19L);
    sky_event_free(event);
    sky_data_file_free(data_file);
    return 0;
}

int test_sky_data_file_get_path_stats_with_event_in_new_middle_path() {
    sky_data_file *data_file;
    INIT_DATA_FILE("tests/fixtures/blocks/path_stats/a");
    sky_block_path_stat *paths = NULL;
    uint32_t path_count = 0;
    sky_event *event = sky_event_create(4, 7LL, 20);
    int rc = sky_block_get_path_stats(data_file->blocks[0], event, &paths, &path_count);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(path_count, 3);
    ASSERT_PATH_STAT(paths[0], 3, 0L, 41L, 41L);
    ASSERT_PATH_STAT(paths[1], 4, -1L, -1L, 19L);
    ASSERT_PATH_STAT(paths[2], 10, 41L, 60L, 19L);
    sky_event_free(event);
    sky_data_file_free(data_file);
    return 0;
}

int test_sky_data_file_get_path_stats_with_event_in_new_ending_path() {
    sky_data_file *data_file;
    INIT_DATA_FILE("tests/fixtures/blocks/path_stats/a");
    sky_block_path_stat *paths = NULL;
    uint32_t path_count = 0;
    sky_event *event = sky_event_create(11, 7LL, 20);
    int rc = sky_block_get_path_stats(data_file->blocks[0], event, &paths, &path_count);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(path_count, 3);
    ASSERT_PATH_STAT(paths[0], 3, 0L, 41L, 41L);
    ASSERT_PATH_STAT(paths[1], 10, 41L, 60L, 19L);
    ASSERT_PATH_STAT(paths[2], 11, -1L, -1L, 19L);
    sky_event_free(event);
    sky_data_file_free(data_file);
    return 0;
}



//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_block_pack);
    mu_run_test(test_sky_block_unpack);
    mu_run_test(test_sky_block_get_offset);
    mu_run_test(test_sky_block_get_ptr);
    mu_run_test(test_sky_block_get_span_count);

    mu_run_test(test_sky_data_file_get_path_stats_with_no_event);
    mu_run_test(test_sky_data_file_get_path_stats_with_event_in_existing_path);
    mu_run_test(test_sky_data_file_get_path_stats_with_event_in_new_starting_path);
    mu_run_test(test_sky_data_file_get_path_stats_with_event_in_new_middle_path);
    mu_run_test(test_sky_data_file_get_path_stats_with_event_in_new_ending_path);

    return 0;
}

RUN_TESTS()