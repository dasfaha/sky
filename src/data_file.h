#ifndef _data_file_h
#define _data_file_h

#include <inttypes.h>
#include <stdbool.h>

typedef struct sky_data_file sky_data_file;

#include "bstring.h"
#include "file.h"
#include "types.h"
#include "block.h"
#include "event.h"

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

#define SKY_DATA_FILE_VERSION  1

#define SKY_HEADER_FILE_HDR_SIZE sizeof(uint32_t) + sizeof(uint32_t)

struct sky_data_file {
    bstring path;
    bstring header_path;
    uint32_t block_size;
    sky_block **blocks;
    uint32_t block_count;
    int data_fd;
    void *data;
    size_t data_length;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_data_file *sky_data_file_create();

void sky_data_file_free(sky_data_file *data_file);


//--------------------------------------
// Paths
//--------------------------------------

int sky_data_file_set_path(sky_data_file *data_file, bstring path);

int sky_data_file_set_header_path(sky_data_file *data_file, bstring path);


//--------------------------------------
// Persistence
//--------------------------------------

int sky_data_file_load(sky_data_file *data_file);

int sky_data_file_unload(sky_data_file *data_file);


//--------------------------------------
// Block Management
//--------------------------------------

int sky_data_file_create_block(sky_data_file *data_file, sky_block **ret);

int sky_data_file_find_insertion_block(sky_data_file *data_file,
    sky_event *event, sky_block **ret);

int sky_data_file_move_to_new_block(sky_data_file *data_file, void **ptr,
    size_t sz, sky_block **new_block);


//--------------------------------------
// Event Management
//--------------------------------------

int sky_data_file_add_event(sky_data_file *data_file, sky_event *event);

#endif
