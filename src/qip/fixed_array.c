#include <stdlib.h>

#include "fixed_array.h"
#include "dbg.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a fixed array.
//
// elemsz - The size, in bytes, of each element of the array.
// length - The number of elements in the array.
//
// Returns a fixed array.
qip_fixed_array *qip_fixed_array_create(int64_t elemsz, int64_t length)
{
    qip_fixed_array *array = qip_fixed_array_alloc();
    check_mem(array);
    qip_fixed_array_init(array, elemsz, length);
    return array;
    
error:
    qip_fixed_array_free(array);
    return NULL;
}

// Allocates a fixed array in memory.
// 
// Returns a fixed array.
qip_fixed_array *qip_fixed_array_alloc()
{
    return calloc(1, sizeof(qip_fixed_array));
}

// Initializes a fixed array.
//
// array - The array.
// elemsz - The size, in bytes, of each element of the array.
// length - The number of elements in the array.
//
// Returns nothing.
void qip_fixed_array_init(qip_fixed_array *array, int64_t elemsz,
                          int64_t length)
{
    array->elemsz = elemsz;
    array->length = length;
    array->elements = calloc(length, elemsz);
}

// Frees a fixed array.
//
// array - The array to free.
void qip_fixed_array_free(qip_fixed_array *array)
{
    if(array) {
        if(array->elements) free(array->elements);
        array->elements = NULL;
        free(array);
    }
}


//======================================
// Element Pointer Management
//======================================

// Retrieves the pointer an element in an array based on element size and
// a given index.
//
// array - The array.
// index - The element index.
//
// Returns a pointer to the element.
void *qip_fixed_array_get_element_ptr(qip_fixed_array *array, int64_t index)
{
    return (void*)(array->elements + (array->elemsz * index));
}


//======================================
// Element Retrieval
//======================================

// Retrieves the element at a given index.
//
// map   - The map.
// index - The index to lookup.
//
// Returns a pointer to the object.
void *qip_fixed_array_get_item_at(qip_module *module, qip_fixed_array *array,
                                  int64_t index)
{
    check(module != NULL, "Module required");
    void *ptr = qip_fixed_array_get_element_ptr(array, index);
    return *((void**)(ptr));

error:
    return NULL;
}

int64_t qip_fixed_array_get_int_item_at(qip_module *module,
                                        qip_fixed_array *array, int64_t index)
{
    check(module != NULL, "Module required");
    void *ptr = qip_fixed_array_get_element_ptr(array, index);
    return *((int64_t*)(ptr));

error:
    return 0;
}


//======================================
// Element Assignment
//======================================

// Sets an element in the array at a given index.
//
// map   - The map.
// item  - The item to set.
// index - The index to lookup.
//
// Returns nothing.
void qip_fixed_array_set_item_at(qip_module *module, qip_fixed_array *array,
                                 void *item, int64_t index)
{
    check(module != NULL, "Module required");
    void **ptr = (void**)qip_fixed_array_get_element_ptr(array, index);
    *ptr = item;
    return;

error:
    return;
}

void qip_fixed_array_set_int_item_at(qip_module *module,
                                     qip_fixed_array *array, int64_t item,
                                     int64_t index)
{
    check(module != NULL, "Module required");
    int64_t *ptr = (int64_t*)qip_fixed_array_get_element_ptr(array, index);
    *ptr = item;
    return;

error:
    return;
}

