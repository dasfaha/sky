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

size_t DATA_LENGTH = 57;
char DATA[] = 
    "\x0a\x00\x00\x00\x31\x00\x00\x00\x01\xa0\x00\x00\x00\x00\x00\x00"
    "\x00\x0b\x00\x02\xa1\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00"
    "\x01\xa3\x66\x6f\x6f\x03\xa2\x00\x00\x00\x00\x00\x00\x00\x0d\x00"
    "\x05\x00\x00\x00\x01\xa3\x62\x61\x72"
;


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Iteration
//--------------------------------------

int test_sky_cursor_next() {
    sky_cursor *cursor = sky_cursor_create();
    
    // Event 1
    int rc = sky_cursor_set_path(cursor, &DATA);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(cursor->path_index, 0);
    mu_assert_int_equals(cursor->event_index, 0);
    mu_assert_long_equals(cursor->ptr-((void*)&DATA), 8L);
    mu_assert_bool(!cursor->eof);
    
    // Event 2
    rc = sky_cursor_next(cursor);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(cursor->path_index, 0);
    mu_assert_int_equals(cursor->event_index, 1);
    mu_assert_long_equals(cursor->ptr-((void*)&DATA), 19L);
    mu_assert_bool(!cursor->eof);

    // Event 3
    rc = sky_cursor_next(cursor);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(cursor->path_index, 0);
    mu_assert_int_equals(cursor->event_index, 2);
    mu_assert_long_equals(cursor->ptr-((void*)&DATA), 37L);
    mu_assert_bool(!cursor->eof);
    
    // EOF
    rc = sky_cursor_next(cursor);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(cursor->path_index, 0);
    mu_assert_int_equals(cursor->event_index, 0);
    mu_assert_bool(cursor->eof);

    sky_cursor_free(cursor);
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