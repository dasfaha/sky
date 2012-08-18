#ifndef _qip_array_h
#define _qip_array_h

#include <inttypes.h>


//==============================================================================
//
// Overview
//
//==============================================================================

// The QIP array provides a dynamic array functionality within the QIP language
// and within the QIP processing code.


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct {
    uint32_t length;
    void **elements;
} qip_array;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

qip_array *qip_array_create();

void qip_array_free(qip_array *array);


//======================================
// Element Management
//======================================

int qip_array_push(qip_array *array, void *item);

int qip_array_pop(qip_array *array, void **item);



#endif
