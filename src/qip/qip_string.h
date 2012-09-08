#ifndef _qip_string_h
#define _qip_string_h

#include <inttypes.h>
#include <stdbool.h>

//==============================================================================
//
// Definitions
//
//==============================================================================

// The qip string stores information about a fixed length string.
typedef struct {
    int64_t length;
    char *data;
} qip_string;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

qip_string *qip_string_create(int64_t length, char *data);

qip_string *qip_string_alloc();

void qip_string_init(qip_string *string, int64_t length,
    char *data);

void qip_string_free(qip_string *string);


//======================================
// Equality
//======================================

bool qip_string_equals(qip_string *a, qip_string *b);

#endif
