#include <stdlib.h>

#include "map.h"
#include "dbg.h"


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int eql_map_elem_cmp(const void *_a, const void *_b);


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a map.
eql_map *eql_map_create()
{
    eql_map *map = malloc(sizeof(eql_map));
    check_mem(map);
    map->elemsz = 0LL;
    map->count = 0LL;
    map->elements = NULL;
    
    return map;
    
error:
    eql_map_free(map);
    return NULL;
}

// Frees a map.
//
// map - The map to free.
void eql_map_free(eql_map *map)
{
    if(map) {
        uint32_t i;
        for(i=0; i<map->count; i++) {
            free(map->elements[i]);
            map->elements[i] = NULL;
        }
        
        if(map->elements) free(map->elements);
        map->elements = NULL;

        free(map);
    }
}


//======================================
// Element Management
//======================================

// Allocates memory for a single element in the map.
//
// map - The map to allocate for.
//
// Returns a pointer to the new element.
void *eql_map_elalloc(eql_map *map)
{
    void *elem = calloc(map->elemsz, 1);
    
    map->count++;
    map->elements = realloc(map->elements, sizeof(*map->elements) * map->count);
    map->elements[map->count-1] = elem;
    
    return elem;
}

// Finds an element in the map with a given key.
//
// map - The map.
// key - The key to search for.
//
// Returns a pointer to the new element if found. Otherwise returns null.
void *eql_map_find(eql_map *map, int64_t key)
{
    // Exit if there are no elements.
    if(map->count == 0) {
        return NULL;
    }
    
    // Perform a binary search to find the element.
    void *key_ptr = &key;
    void *ret = bsearch(&key_ptr, map->elements, map->count, sizeof(*map->elements), eql_map_elem_cmp);
    
    if(ret != NULL) {
        return *((void**)ret);
    }
    else {
        return NULL;
    }
}

// Internally refreshes the map. This must be performed whenever a new element
// is added and initialized.
//
// map - The map.
//
// Returns nothing.
void eql_map_refresh(eql_map *map)
{
    // Sort the elements by hash code for faster lookup.
    if(map->elements) {
        qsort(map->elements, map->count, sizeof(*map->elements), eql_map_elem_cmp);
    }
}


//======================================
// Element Sorting
//======================================

// Compares two elements to determine the order in which they are sorted.
// The first 8-bytes of every element is a hash code that can be compared.
int eql_map_elem_cmp(const void *_a, const void *_b)
{
    int64_t a = *((int64_t*)(*((void**)(_a))));
    int64_t b = *((int64_t*)(*((void**)(_b))));
    
    if(a > b) {
        return 1;
    }
    else if(a < b) {
        return -1;
    }
    else {
        return 0;
    }
}