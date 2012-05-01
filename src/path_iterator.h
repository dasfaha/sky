#ifndef _path_iterator_h
#define _path_iterator_h

#include <inttypes.h>
#include <stdbool.h>

#include "bstring.h"
#include "object_file.h"
#include "cursor.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// The path iterator is used to sequentially loop over a set of paths in a set
// of blocks. The next path can be requested from the iterator by calling the
// `sky_path_iterator_next()` function. When calling the `next()` function, a cursor
// is returned instead of a reference to a deserialized path. The cursor can be
// used to iterate over the raw path data.
//
// To perform the relational database equivalent of a full table scan, the
// `sky_object_file_create_iterator()` function can be used to create a path
// iterator. That contains all the paths in the object file.
//
// The path iterator operates as a forward-only iterator. Jumping to the
// previous path or jumping to a path by index is not allowed.
//
// The path iterator does not currently support full consistency if events are
// added or removed after the iterator has been created and before the iteration
// is complete. The biggest issue is that a block split can cause paths to not
// be counted. This will be fixed in a future version.


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct sky_path_iterator {
    sky_object_file *object_file;
    uint32_t block_index;
    uint32_t byte_index;
    bool eof;
} sky_path_iterator;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_path_iterator *sky_path_iterator_create(sky_object_file *object_file);

void sky_path_iterator_free(sky_path_iterator *iterator);


//======================================
// Iteration
//======================================

int sky_path_iterator_next(sky_path_iterator *iterator, sky_cursor *cursor);

int sky_path_iterator_eof(sky_path_iterator *iterator);


#endif
