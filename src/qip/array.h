#ifndef _qip_array_h
#define _qip_array_h

#include <inttypes.h>


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct {
    int64_t length;
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

//======================================
// Qip Interface
//======================================

void *qipx_array_get_item_at(qip_array *array, int64_t index);

void qipx_array_set_item_at(qip_array *array, void *item, int64_t index);

#endif
