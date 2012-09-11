#ifndef _qip_map_h
#define _qip_map_h

#include <inttypes.h>

//==============================================================================
//
// Definitions
//
//==============================================================================

// The map struct holds the size of each element in bytes (elemsz), the number
// of elements in the map (count), the size of the buffer (bcount), and a
// pointer to where the elements are stored.
typedef struct {
    int64_t elemsz;
    int64_t count;
    void **elements;
} qip_map;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_map *qip_map_create();

void qip_map_free(qip_map *map);


//--------------------------------------
// Element Management
//--------------------------------------

void *qip_map_elalloc(qip_map *map);

void *qip_map_find(qip_map *map, int64_t key);

void qip_map_refresh(qip_map *map);

#endif
