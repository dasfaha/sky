#include <stdlib.h>

#include "map.h"
#include "dbg.h"


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int qip_map_elem_cmp(const void *_a, const void *_b);


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a map.
qip_map *qip_map_create()
{
    qip_map *map = malloc(sizeof(qip_map));
    check_mem(map);
    map->elemsz = 0LL;
    map->count = 0LL;
    map->elements = NULL;
    
    return map;
    
error:
    qip_map_free(map);
    return NULL;
}

// Frees a map.
//
// map - The map to free.
void qip_map_free(qip_map *map)
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
void *qip_map_elalloc(qip_module *module, qip_map *map)
{
    check(module != NULL, "Module required");
    
    void *elem = calloc(map->elemsz, 1);
    
    map->count++;
    map->elements = realloc(map->elements, sizeof(*map->elements) * map->count);
    map->elements[map->count-1] = elem;
    
    return elem;

error:
    return NULL;
}

// Finds an element in the map with a given key.
//
// map - The map.
// key - The key to search for.
//
// Returns a pointer to the new element if found. Otherwise returns null.
void *qip_map_find(qip_module *module, qip_map *map, int64_t key)
{
    check(module != NULL, "Module required");

    // Exit if there are no elements.
    if(map->count == 0) {
        return NULL;
    }
    
    // Perform a binary search to find the element.
    void *key_ptr = &key;
    void *ret = bsearch(&key_ptr, map->elements, map->count, sizeof(*map->elements), qip_map_elem_cmp);
    
    if(ret != NULL) {
        return *((void**)ret);
    }
    else {
        return NULL;
    }

error:
    return NULL;
}

// Internally refreshes the map. This must be performed whenever a new element
// is added and initialized.
//
// map - The map.
//
// Returns nothing.
void qip_map_refresh(qip_module *module, qip_map *map)
{
    check(module != NULL, "Module required");

    // Sort the elements by hash code for faster lookup.
    if(map->elements) {
        qsort(map->elements, map->count, sizeof(*map->elements), qip_map_elem_cmp);
    }

    return;
    
error:
    return;
}


//======================================
// Element Sorting
//======================================

// Compares two elements to determine the order in which they are sorted.
// The first 8-bytes of every element is a hash code that can be compared.
int qip_map_elem_cmp(const void *_a, const void *_b)
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