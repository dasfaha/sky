#include <stdio.h>
#include <stdlib.h>

#include <eql/array.h>

#include "../minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

int test_eql_array_push_pop() {
    int a, b, c;
    void *ret;
    eql_array *array = eql_array_create();
    mu_assert(array != NULL, "");
    
    // Push
    eql_array_push(array, &a);
    eql_array_push(array, &b);
    eql_array_push(array, &c);
    mu_assert(array->length == 3, "");
    mu_assert(array->elements[0] == &a, "");
    mu_assert(array->elements[1] == &b, "");
    mu_assert(array->elements[2] == &c, "");

    // Pop
    eql_array_pop(array, &ret);
    mu_assert(ret == &c, "");
    mu_assert(array->length == 2, "");
    eql_array_pop(array, &ret);
    mu_assert(ret == &b, "");
    mu_assert(array->length == 1, "");
    eql_array_pop(array, &ret);
    mu_assert(ret == &a, "");
    mu_assert(array->length == 0, "");

    eql_array_free(array);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_array_push_pop);
    return 0;
}

RUN_TESTS()