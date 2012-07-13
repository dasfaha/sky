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
    "\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\x14"
    "\x00\x00\x01\x34\x96\x90\xd0\x00\x00\x03\x5d\x01\x3b\x37\xe0\x00"
;


//==============================================================================
//
// Helpers
//
//==============================================================================

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
    block->min_timestamp = 1325376000000LL;
    block->max_timestamp = 946684800000000LL;
    
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
    mu_assert_int64_equals(block->min_object_id, 10LL);
    mu_assert_int64_equals(block->max_object_id, 20LL);
    mu_assert_int64_equals(block->min_timestamp, 1325376000000LL);
    mu_assert_int64_equals(block->max_timestamp, 946684800000000LL);
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
    return 0;
}

RUN_TESTS()