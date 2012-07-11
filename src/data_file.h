#ifndef _data_file_h
#define _data_file_h

#include <inttypes.h>
#include <stdbool.h>

typedef struct sky_data_file sky_data_file;

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

struct sky_data_file {
    sky_header_file *header_file;
    bstring path;
    int data_fd;
    void *data;
    size_t data_length;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_data_file *sky_data_file_create();

void sky_data_file_free(sky_data_file *data_file);


//======================================
// Path
//======================================

int sky_data_file_set_path(sky_data_file *data_file, bstring path);


//======================================
// Persistence
//======================================

int sky_data_file_load(sky_data_file *data_file);

int sky_data_file_unload(sky_data_file *data_file);

#endif
