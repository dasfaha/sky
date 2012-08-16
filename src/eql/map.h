#ifndef _eql_map_h
#define _eql_map_h

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
} eql_map;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

eql_map *eql_map_create();

void eql_map_free(eql_map *map);


//======================================
// Element Management
//======================================

void *eql_map_elalloc(eql_map *map);

void *eql_map_find(eql_map *map, int64_t key);

void eql_map_refresh(eql_map *map);

#endif
