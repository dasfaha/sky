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
    Database *database = Database_create(&ROOT);
    ObjectFile *object_file = ObjectFile_create(database, &OBJECT_TYPE);
    ObjectFile_open(object_file);

    Cursor *cursor = Cursor_create();
    PathIterator *iterator = PathIterator_create(object_file);
    
    // Validate order of paths.
    mu_assert(PathIterator_next(iterator, cursor) == 0, "");
    mu_assert_int64(cursor->ptr, 10);
    
    mu_assert(PathIterator_next(iterator, cursor) == 0, "");
    mu_assert_int64(cursor->ptr, 11);

    mu_assert(PathIterator_next(iterator, cursor) == 0, "");
    mu_assert(iterator->eof == true, "");
    mu_assert(cursor->ptr == NULL, "");
    
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
    mu_run_test(test_PathIterator_next);
    return 0;
}

RUN_TESTS()