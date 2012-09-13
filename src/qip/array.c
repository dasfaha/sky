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
qip_array *qip_array_create()
{
    qip_array *array = malloc(sizeof(qip_array));
    check_mem(array);
    array->elements = NULL;
    array->length = 0;
    
    return array;
    
error:
    qip_array_free(array);
    return NULL;
}

// Frees an array.
//
// array - The array to free.
void qip_array_free(qip_array *array)
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
int qip_array_push(qip_array *array, void *item)
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
int qip_array_pop(qip_array *array, void **ret)
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


//======================================
// Qip Interface
//======================================

// Retrieves an item from the array at a given index.
//
// array - The array.
// index - The index to lookup
//
// Returns the item at the given index if found. If the index is out of
// bounds then null is returned.
void *qipx_array_get_item_at(qip_array *array, int64_t index)
{
    check(array != NULL, "Array required");

    // Check if it is in-bounds.
    if(index >= 0 && index < array->length) {
        return array->elements[index];
    }
    else {
        return NULL;
    }

error:
    return NULL;
}

// Sets the element at a given index to a new value.
//
// array - The array.
// index - The index to lookup
//
// Returns nothing.
void qipx_array_set_item_at(qip_array *array, void *item, int64_t index)
{
    check(array != NULL, "Array required");

    // Check if it is in-bounds.
    if(index >= 0 && index < array->length) {
        array->elements[index] = item;
    }

    return;
    
error:
    return;
}
