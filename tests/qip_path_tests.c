#include <stdio.h>
#include <stdlib.h>

#include <qip/qip.h>
#include <qip_path.h>

#include "minunit.h"
#include "qip_test_util.h"


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

typedef void * (*sky_qip_path_func)(sky_qip_path *path);

int test_sky_qip_path_execute() {
    qip_module *module = qip_module_create(NULL, NULL);
    COMPILE_QUERY_1ARG(module, "Path", "path",
        "Cursor cursor = path.events();\n"
        "return cursor;"
    );

    // Initialize path.
    sky_qip_path *path = sky_qip_path_create();
    path->path_ptr = &DATA;

    // Execute module.
    sky_qip_path_func f = NULL;
    qip_module_get_main_function(module, (void*)(&f));
    sky_qip_cursor *cursor = f(path);

    // Validate that the cursor pointer starts at the first event.
    mu_assert_long_equals(cursor->cursor->ptr - path->path_ptr, 8L);

    // Clean up.
    sky_qip_path_free(path);
    sky_qip_cursor_free(cursor);
    qip_module_free(module);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_qip_path_execute);
    return 0;
}

RUN_TESTS()