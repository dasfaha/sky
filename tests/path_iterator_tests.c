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
    mu_assert(memcmp(&num, PTR, sizeof(num)) == 0, "Expected: " #VALUE);\
} while(0)

//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Iteration
//--------------------------------------

int test_PathIterator_next() {
    copydb("path_iterator0");

    // Open object file and create iterator.
    sky_database *database = sky_database_create(&ROOT);
    sky_object_file *object_file = sky_object_file_create(database, &OBJECT_TYPE);
    sky_object_file_open(object_file);

    sky_cursor *cursor = sky_cursor_create();
    PathIterator *iterator = PathIterator_create(object_file);
    
    void *data = object_file->data;

    // First path should span across the first and third blocks.
    mu_assert(PathIterator_next(iterator, cursor) == 0, "");
    mu_assert(cursor->path_count == 2, "");
    mu_assert(cursor->paths[0] - data == 4, "");
    mu_assert(cursor->paths[1] - data == 260, "");
    
    // Second path should be in the second block.
    mu_assert(PathIterator_next(iterator, cursor) == 0, "");
    mu_assert(cursor->path_count == 1, "");
    mu_assert(cursor->paths[0] - data == 132, "");

    // Third path should be in the fourth block.
    mu_assert(PathIterator_next(iterator, cursor) == 0, "");
    mu_assert(cursor->path_count == 1, "");
    mu_assert(cursor->paths[0] - data == 388, "");

    // No more paths so iterator should be EOF.
    mu_assert(PathIterator_next(iterator, cursor) == 0, "");
    mu_assert(cursor->path_count == 0, "");
    mu_assert(cursor->paths == NULL, "");
    mu_assert(iterator->eof == true, "");
    
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
    mu_run_test(test_PathIterator_next);
    return 0;
}

RUN_TESTS()