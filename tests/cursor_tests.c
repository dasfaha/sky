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

size_t DATA_LENGTH = 50;
char DATA[] = 
    "\x0a\xda\x00\x2e\x01\x00\x03\x5d\x01\x3b\x37\xe0\x00\x06\x03\x00"
    "\x03\x5d\x02\x11\xcb\x84\x00\x07\xaa\x01\xa3\x66\x6f\x6f\x02\xa3"
    "\x62\x61\x72\x02\x00\x03\x5d\x02\xe8\x5f\x28\x00\xa5\x01\xa3\x66"
    "\x6f\x6f"
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
    mu_assert_long_equals(cursor->ptr-((void*)&DATA), 4L);
    mu_assert_bool(!cursor->eof);
    
    // Event 2
    rc = sky_cursor_next(cursor);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(cursor->path_index, 0);
    mu_assert_int_equals(cursor->event_index, 1);
    mu_assert_long_equals(cursor->ptr-((void*)&DATA), 14L);
    mu_assert_bool(!cursor->eof);

    // Event 3
    rc = sky_cursor_next(cursor);
    mu_assert_int_equals(rc, 0);
    mu_assert_int_equals(cursor->path_index, 0);
    mu_assert_int_equals(cursor->event_index, 2);
    mu_assert_long_equals(cursor->ptr-((void*)&DATA), 35L);
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