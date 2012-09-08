#ifndef _qip_fixed_array_h
#define _qip_fixed_array_h

#include <inttypes.h>

//==============================================================================
//
// Definitions
//
//==============================================================================

// The fixed array stores a fixed-size array of data.
typedef struct {
    int64_t elemsz;
    int64_t length;
    void *elements;
} qip_fixed_array;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

qip_fixed_array *qip_fixed_array_create(int64_t elemsz, int64_t length);

qip_fixed_array *qip_fixed_array_alloc();

void qip_fixed_array_init(qip_fixed_array *array, int64_t elemsz,
    int64_t length);

void qip_fixed_array_free(qip_fixed_array *array);


//======================================
// Element Retrieval
//======================================

void *qip_fixed_array_get_item_at(qip_fixed_array *array, int64_t index);

int64_t qip_fixed_array_get_int_item_at(qip_fixed_array *array, int64_t index);


//======================================
// Element Assignment
//======================================

void qip_fixed_array_set_item_at(qip_fixed_array *array, void *item,
    int64_t index);

void qip_fixed_array_set_int_item_at(qip_fixed_array *array, int64_t item,
    int64_t index);

#endif
