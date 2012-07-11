#include <stdlib.h>
#include <inttypes.h>

#include "dbg.h"
#include "bstring.h"
#include "file.h"
#include "data_file.h"

//==============================================================================
//
// Forward Declarations
//
//==============================================================================


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a reference to a header file.
// 
// Returns a reference to the new header file if successful. Otherwise returns
// null.
sky_data_file *sky_data_file_create()
{
    sky_data_file *data_file = calloc(sizeof(sky_data_file), 1);
    check_mem(data_file);
    return data_file;
    
error:
    sky_data_file_free(data_file);
    return NULL;
}

// Removes a header file reference from memory.
//
// data_file - The header file to free.
void sky_data_file_free(sky_data_file *data_file)
{
    if(data_file) {
        if(data_file->path) bdestroy(data_file->path);
        data_file->path = NULL;
        sky_data_file_unload(data_file);
        free(data_file);
    }
}


//======================================
// Path
//======================================

// Sets the file path of a header file.
//
// data_file - The header file.
// path      - The file path to set.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_set_path(sky_data_file *data_file, bstring path)
{
    check(data_file != NULL, "Header file required");

    if(data_file->path) {
        bdestroy(data_file->path);
    }
    
    data_file->path = bstrcpy(path);
    if(path) check_mem(data_file->path);

    return 0;

error:
    data_file->path = NULL;
    return -1;
}


//======================================
// Persistence
//======================================

// Loads the data file into memory as a memory-mapped file.
//
// data_file - The header file to load.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_load(sky_data_file *data_file)
{
    int rc;
    void *ptr;

    // Calculate the data length.
    size_t data_length;
    if(data_file->header_file->block_count == 0) {
        data_length = data_file->header_file->block_size;
    }
    else {
        data_length = table->header_file->block_count * table->header_file->block_size;
    }

    // Close mapping if remapping isn't supported.
    if(!MREMAP_AVAILABLE) {
        sky_data_file_unload(table);
    }

    // Open the data file and map it if it is not currently open.
    if(table->data_fd == 0) {
        data_file->data_fd = open(bdata(data_file->path), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        check(data_file->data_fd != -1, "Failed to open data file descriptor: %s",  bdata(data_file->path));

        // Truncate the file to the appropriate size (larger or smaller).
        rc = ftruncate(data_file->data_fd, data_length);
        check(rc == 0, "Unable to truncate data file");

        // Memory map the data file.
        ptr = mmap(0, data_length, PROT_READ | PROT_WRITE, MAP_SHARED, data_file->data_fd, 0);
        check(ptr != MAP_FAILED, "Unable to memory map data file");
    }
    // If we already have the data mapped then simply remap it to the
    // appropriate size.
    else {
#if MREMAP_AVAILABLE
        ptr = mremap(data_file->data, data_file->data_length, data_length, MREMAP_MAYMOVE);
        check(ptr != MAP_FAILED, "Unable to remap data file");
#endif
    }

    // Update the table.
    data_file->data = ptr;
    data_file->data_length = data_length;

    return 0;

error:
    sky_data_file_unload(table);
    return -1;
}

// Unloads the data file from memory.
//
// data_file - The header file to save.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_unload(sky_data_file *data_file)
{
    // Unmap file.
    if(data_file->data != NULL) {
        munmap(data_file->data, data_file->data_length);
        data_file->data = NULL;
    }
    
    // Close file descriptor.
    if(data_file->data_fd != 0) {
        close(data_file->data_fd);
        data_file->data_fd = 0;
    }
    
    data_file->data_length = 0;
    
    return 0;
}


