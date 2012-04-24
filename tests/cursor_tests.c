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

int test_Cursor_next() {
    copydb("cursor0");

    // Open object file and create iterator.
    Database *database = Database_create(&ROOT);
    ObjectFile *object_file = ObjectFile_create(database, &OBJECT_TYPE);
    ObjectFile_open(object_file);

    Cursor *cursor = Cursor_create();
    PathIterator *iterator = PathIterator_create(object_file);
    
    void *data = object_file->data;

    // First path (OID#10).
    PathIterator_next(iterator, cursor);
    mu_assert(cursor->ptr-data == 16, "");
    mu_assert(Cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr-data == 40, "");
    mu_assert(Cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr-data == 272, "");
    mu_assert(Cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");
    
    // Second path (OID#11)
    PathIterator_next(iterator, cursor);
    mu_assert(cursor->ptr-data == 144, "");
    mu_assert(Cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr-data == 172, "");
    mu_assert(Cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");

    // Third path (OID#12)
    PathIterator_next(iterator, cursor);
    mu_assert(cursor->ptr-data == 400, "");
    mu_assert(Cursor_next_event(cursor) == 0, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");

    // No more paths so iterator should be EOF.
    PathIterator_next(iterator, cursor);
    mu_assert(iterator->eof == true, "");
    mu_assert(cursor->ptr == NULL, "");
    mu_assert(cursor->eof, "");
    
    // Clean up.
    PathIterator_destroy(iterator);
    Cursor_destroy(cursor);

    ObjectFile_close(object_file);
    ObjectFile_destroy(object_file);
    Database_destroy(database);
    
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_Cursor_next);
    return 0;
}

RUN_TESTS()