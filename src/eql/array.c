#include <stdlib.h>

#include "array.h"
#include "dbg.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates an array.
eql_array *eql_array_create()
{
    eql_array *array = malloc(sizeof(eql_array));
    check_mem(array);
    array->elements = NULL;
    array->length = 0;
    
    return array;
    
error:
    eql_array_free(array);
    return NULL;
}

// Frees an array.
//
// array - The array to free.
void eql_array_free(eql_array *array)
{
    if(array) {
        if(array->elements) free(array->elements);
        array->elements = NULL;
        free(array);
    }
}


//======================================
// Element Management
//======================================

// Adds an item to the end of an array.
//
// array - The array to add to.
// item  - The item to add to the array.
//
// Returns 0 if successful, otherwise returns -1.
int eql_array_push(eql_array *array, void *item)
{
    check(array != NULL, "Array required");

    // Resize array.
    array->length++;
    array->elements = realloc(array->elements, sizeof(void*) * array->length);
    check_mem(array->elements);

    // Append item to the array.
    array->elements[array->length-1] = item;

    return 0;

error:
    return -1;
}


// Removes an item from the end of an array and returns it.
//
// array - The array to remove from.
// ret   - A pointer to where the item should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_array_pop(eql_array *array, void **ret)
{
    check(array != NULL, "Array required");
    check(ret != NULL, "Return pointer required");

    // If there are no elements then return null.
    if(array->length == 0) {
        *ret = NULL;
    }
    // Otherwise return the last element and resize.
    else {
        *ret = array->elements[array->length-1];
        array->elements[array->length-1] = NULL;
        array->length--;
    }
    
    return 0;

error:
    return -1;
}


