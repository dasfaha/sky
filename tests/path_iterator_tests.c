#include <stdio.h>
#include <stdlib.h>

#include <dbg.h>
#include <mem.h>
#include <path_iterator.h>

#include "minunit.h"


//==============================================================================
//
// Constants
//
//==============================================================================

struct tagbstring ROOT = bsStatic("tmp/db");
struct tagbstring OBJECT_TYPE = bsStatic("users");


//==============================================================================
//
// Macros
//
//==============================================================================

#define mu_assert_int64(PTR, VALUE) do {\
    int64_t num = VALUE;\
    mu_assert_mem(&num, PTR, sizeof(num));\
} while(0)

//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Iteration
//--------------------------------------

int test_sky_path_iterator_next() {
    copydb("path_iterator0");

    // Open table and create iterator.
    sky_database *database = sky_database_create(&ROOT);
    sky_table *table = sky_table_create(database, &OBJECT_TYPE);
    sky_table_open(table);

    sky_cursor *cursor = sky_cursor_create();
    sky_path_iterator *iterator = sky_path_iterator_create(table);
    
    void *data = table->data;

    // First path should span across the first and third blocks.
    mu_assert(sky_path_iterator_next(iterator, cursor) == 0, "");
    mu_assert(cursor->path_count == 2, "");
    mu_assert(cursor->paths[0] - data == 4, "");
    mu_assert(cursor->paths[1] - data == 260, "");
    
    // Second path should be in the second block.
    mu_assert(sky_path_iterator_next(iterator, cursor) == 0, "");
    mu_assert(cursor->path_count == 1, "");
    mu_assert(cursor->paths[0] - data == 132, "");

    // Third path should be in the fourth block.
    mu_assert(sky_path_iterator_next(iterator, cursor) == 0, "");
    mu_assert(cursor->path_count == 1, "");
    mu_assert(cursor->paths[0] - data == 388, "");

    // No more paths so iterator should be EOF.
    mu_assert(sky_path_iterator_next(iterator, cursor) == 0, "");
    mu_assert(cursor->path_count == 0, "");
    mu_assert(cursor->paths == NULL, "");
    mu_assert(iterator->eof == true, "");
    
    // Clean up.
    sky_path_iterator_free(iterator);
    sky_cursor_free(cursor);

    sky_table_close(table);
    sky_table_free(table);
    sky_database_free(database);
    
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_path_iterator_next);
    return 0;
}

RUN_TESTS()