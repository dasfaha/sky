#ifndef _eql_array_h
#define _eql_array_h

#include <inttypes.h>


//==============================================================================
//
// Overview
//
//==============================================================================

// The EQL array provides a dynamic array functionality within the EQL language
// and within the EQL processing code.


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct {
    uint32_t length;
    void **elements;
} eql_array;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

eql_array *eql_array_create();

void eql_array_free(eql_array *array);


//======================================
// Element Management
//======================================

int eql_array_push(eql_array *array, void *item);

int eql_array_pop(eql_array *array, void **item);



#endif
