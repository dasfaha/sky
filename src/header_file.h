#ifndef _header_file_h
#define _header_file_h

#include <inttypes.h>
#include <stdbool.h>

typedef struct sky_header_file sky_header_file;

#include "bstring.h"
#include "file.h"
#include "types.h"
#include "block_info.h"

//==============================================================================
//
// Overview
//
//==============================================================================

// The header file stores information about how the table's data file is
// structured. The beginning of the file lists the database format version
// (4-bytes), block size (4-bytes) and block count (4-bytes). From there the
// blocks are listed out in 


//==============================================================================
//
// Typedefs
//
//==============================================================================

#define SKY_DEFAULT_BLOCK_SIZE 0x10000

struct sky_header_file {
    bstring path;
    uint32_t version;
    uint32_t block_size;
    uint32_t block_count;
    sky_block_info **infos;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_header_file *sky_header_file_create();

void sky_header_file_free(sky_header_file *header_file);


//======================================
// Path
//======================================

int sky_header_file_set_path(sky_header_file *header_file, bstring path);


//======================================
// Persistence
//======================================

int sky_header_file_load(sky_header_file *header_file);

int sky_header_file_unload(sky_header_file *header_file);

int sky_header_file_save(sky_header_file *header_file);

//======================================
// Block Management
//======================================

size_t sky_header_file_get_block_offset(sky_header_file *header_file,
    uint32_t index);

#endif
