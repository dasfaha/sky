#include <stdio.h>
#include <stdlib.h>

#include <eql/eql.h>
#include <eql_path.h>

#include "minunit.h"
#include "eql_test_util.h"


//==============================================================================
//
// Fixtures
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
// Execution
//--------------------------------------

typedef void * (*sky_eql_path_func)(sky_eql_path *path);

int test_sky_eql_path_execute() {
    eql_module *module = NULL;
    COMPILE_QUERY(module, "Path", "path",
        "Cursor cursor = path.events();\n"
        "return cursor;"
    );

    // Initialize path.
    sky_eql_path *path = sky_eql_path_create();
    path->path_ptr = &DATA;

    // Execute module.
    sky_eql_path_func f = NULL;
    eql_module_get_main_function(module, (void*)(&f));
    sky_eql_cursor *cursor = f(path);

    // Validate that the cursor pointer starts at the first event.
    mu_assert_long_equals(cursor->cursor->ptr - path->path_ptr, 8L);

    // Clean up.
    sky_eql_path_free(path);
    sky_eql_cursor_free(cursor);
    eql_module_free(module);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_eql_path_execute);
    return 0;
}

RUN_TESTS()