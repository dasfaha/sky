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

    // Open object file and create iterator.
    sky_database *database = sky_database_create(&ROOT);
    sky_object_file *object_file = sky_object_file_create(database, &OBJECT_TYPE);
    sky_object_file_open(object_file);

    sky_cursor *cursor = sky_cursor_create();
    PathIterator *iterator = PathIterator_create(object_file);
    
    void *data = object_file->data;

    // First path (OID#10).
    PathIterator_next(iterator, cursor);
    mu_assert(cursor->ptr-data == 16, "");
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr-data == 40, "");
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr-data == 272, "");
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");
    
    // Second path (OID#11)
    PathIterator_next(iterator, cursor);
    mu_assert(cursor->ptr-data == 144, "");
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr-data == 172, "");
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");

    // Third path (OID#12)
    PathIterator_next(iterator, cursor);
    mu_assert(cursor->ptr-data == 400, "");
    mu_assert(sky_cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");

    // No more paths so iterator should be EOF.
    PathIterator_next(iterator, cursor);
    mu_assert(iterator->eof == true, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");
    
    // Clean up.
    PathIterator_destroy(iterator);
    sky_cursor_free(cursor);

    sky_object_file_close(object_file);
    sky_object_file_free(object_file);
    sky_database_free(database);
    
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_cursor_next);
    return 0;
}

RUN_TESTS()