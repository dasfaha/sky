#include <stdlib.h>
#include "serializer.h"
#include "dbg.h"

//==============================================================================
//
// Functions
//
//==============================================================================

// The number of bytes to allocate each type the serializer requests more
// memory.
#define QIP_SERIALIZER_ALLOC_SIZE 0x10000

// The maximum number of bytes needed to store a msgpack element.
#define QIP_SERIALIZER_MAX_ELEMENT_SIZE   9


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates a serializer.
qip_serializer *qip_serializer_create()
{
    qip_serializer *serializer = malloc(sizeof(qip_serializer));
    check_mem(serializer);
    serializer->ptr = NULL;
    serializer->data = NULL;
    serializer->length = 0LL;
    serializer->blength = 0LL;
    return serializer;
    
error:
    qip_serializer_free(serializer);
    return NULL;
}

// Frees a serializer.
//
// serializer - The serializer to free.
void qip_serializer_free(qip_serializer *serializer)
{
    if(serializer) {
        serializer->ptr = NULL;
        serializer->data = NULL;
        serializer->length = 0LL;
        serializer->blength = 0LL;
        free(serializer);
    }
}


//--------------------------------------
// Memory Management
//--------------------------------------

// Ensures that at least the specified number of bytes is available in the
// buffer.
//
// serializer - The serializer.
// n          - The number of bytes to allocate in the buffer.
//
// Returns nothing.
void qip_serializer_alloc(qip_serializer *serializer, int64_t n)
{
    // Check if there are enough remaining bytes in buffer.
    if(n > serializer->blength - serializer->length) {
        // If there aren't then allocate a large chunk.
        size_t sz = (n > QIP_SERIALIZER_ALLOC_SIZE ? n : QIP_SERIALIZER_ALLOC_SIZE);
        serializer->blength = sz;
        serializer->data = realloc(serializer->data, serializer->blength);
        serializer->ptr = serializer->data + serializer->length;
    }
}


//--------------------------------------
// Packing
//--------------------------------------

// Packs a 64-bit signed integer.
//
// serializer - The serializer.
// value      - The 64-bit int to pack.
//
// Returns nothing.
void qip_serializer_pack_int(qip_serializer *serializer, int64_t value)
{
    size_t sz;
    qip_serializer_alloc(serializer, QIP_SERIALIZER_MAX_ELEMENT_SIZE);
    minipack_pack_int(serializer->ptr, value, &sz);
    serializer->length += sz;
    serializer->ptr = serializer->data + serializer->length;
}

void qip_serializer_pack_float(qip_serializer *serializer, double value)
{
    size_t sz;
    qip_serializer_alloc(serializer, QIP_SERIALIZER_MAX_ELEMENT_SIZE);
    minipack_pack_double(serializer->ptr, value, &sz);
    serializer->length += sz;
    serializer->ptr = serializer->data + serializer->length;
}

void qip_serializer_pack_string(qip_serializer *serializer, qip_string *value)
{
    qip_serializer_pack_raw(serializer, value->data, value->length-1);
}

void qip_serializer_pack_raw(qip_serializer *serializer, void *value,
                             uint64_t length)
{
    size_t sz;
    
    // Allocate memory.
    qip_serializer_alloc(serializer, QIP_SERIALIZER_MAX_ELEMENT_SIZE + length);

    // Pack raw header.
    minipack_pack_raw(serializer->ptr, (uint32_t)length, &sz);
    serializer->length += sz;
    serializer->ptr = serializer->data + serializer->length;

    // Pack data.
    memmove(serializer->ptr, value, (uint32_t)length);
    serializer->length += (uint32_t)length;
    serializer->ptr = serializer->data + serializer->length;
}

void qip_serializer_pack_map(qip_serializer *serializer, int64_t count)
{
    size_t sz;
    qip_serializer_alloc(serializer, QIP_SERIALIZER_MAX_ELEMENT_SIZE);
    minipack_pack_map(serializer->ptr, (uint32_t)count, &sz);
    serializer->length += sz;
    serializer->ptr = serializer->data + serializer->length;
}
