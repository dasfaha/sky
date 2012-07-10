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
// Test Cases
//
//==============================================================================

//--------------------------------------
// Iteration
//--------------------------------------

int test_sky_cursor_next() {
    copydb("cursor0");

    // Open table and create iterator.
    sky_database *database = sky_database_create(&ROOT);
    sky_table *table = sky_table_create(database, &OBJECT_TYPE);
    sky_table_open(table);

    sky_cursor *cursor = sky_cursor_create();
    sky_path_iterator *iterator = sky_path_iterator_create(table);
    
    void *data = table->data;

    // First path (OID#10).
    sky_path_iterator_next(iterator, cursor);
    mu_assert_long_equals(cursor->ptr-data, 16L);
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr-data == 40, "");
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr-data == 272, "");
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");
    
    // Second path (OID#11)
    sky_path_iterator_next(iterator, cursor);
    mu_assert(cursor->ptr-data == 144, "");
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr-data == 172, "");
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");

    // Third path (OID#12)
    sky_path_iterator_next(iterator, cursor);
    mu_assert(cursor->ptr-data == 400, "");
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");

    // No more paths so iterator should be EOF.
    sky_path_iterator_next(iterator, cursor);
    mu_assert(iterator->eof == true, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");
    
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
    //mu_run_test(test_sky_cursor_next);
    return 0;
}

RUN_TESTS()