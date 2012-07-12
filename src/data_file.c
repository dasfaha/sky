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

// Creates a reference to a data file.
// 
// Returns a reference to the new data file if successful. Otherwise returns
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

// Removes a data file reference from memory.
//
// data_file - The data file to free.
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
// Paths
//======================================

// Sets the file path of a data file.
//
// data_file - The data file.
// path      - The file path to set.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_set_path(sky_data_file *data_file, bstring path)
{
    check(data_file != NULL, "Data file required");

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

// Sets the file path for the header file.
//
// data_file - The data file object.
// path      - The file path to set.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_set_header_path(sky_data_file *data_file, bstring path)
{
    check(data_file != NULL, "Data file required");

    if(data_file->header_path) {
        bdestroy(data_file->header_path);
    }
    
    data_file->header_path = bstrcpy(path);
    if(path) check_mem(data_file->header_path);

    return 0;

error:
    data_file->header_path = NULL;
    return -1;
}


//======================================
// Persistence
//======================================

// Loads the data file into memory as a memory-mapped file. If the data file
// is already loaded into memory then it is remapped.
//
// data_file - The data file to load.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_load(sky_data_file *data_file)
{
    int rc;
    void *ptr;

    // Calculate the data length.
    size_t data_length;
    if(data_file->block_count == 0) {
        data_length = data_file->block_size;
    }
    else {
        data_length = data_file->block_count * data_file->block_size;
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
// data_file - The data file to save.
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


//======================================
// Header File Management
//======================================

// Loads header information for the data file.
//
// data_file - The data file object associated with the header file.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_load_header(sky_data_file *data_file)
{
    int rc;
    size_t sz;
    FILE *file;
    uint32_t version = 1;
    uint8_t buffer[SKY_BLOCK_HEADER_SIZE];

    // Read in header file if it exists.
    if(sky_file_exists(data_file->header_path)) {
        file = fopen(bdata(data_file->header_path), "r");
        check(file, "Failed to open header file for reading: %s",  bdata(data_file->header_path));

        // Read database format version.
        rc = fread(&version, sizeof(version), 1, file);
        check(rc == 1, "Unable to read version");
        version = ntohl(version);

        // Read block size.
        rc = fread(&data_file->block_size, sizeof(data_file->block_size), 1, file);
        check(rc == 1, "Unable to read block size");
        data_file->block_size = ntohl(data_file->block_size);

        // Read block count.
        rc = fread(&data_file->block_count, sizeof(data_file->block_count), 1, file);
        check(rc == 1, "Unable to read block count");
        data_file->block_count = ntohl(data_file->block_count);
        data_file->blocks = malloc(sizeof(sky_block*) * data_file->block_count);
        if(data_file->block_count > 0) check_mem(data_file->blocks);

        // Read blocks until end of file.
        uint32_t i;
        for(i=0; i<data_file->block_count && !feof(file); i++) {
            sky_block *block = sky_block_create(data_file);
            check_mem(block);
            block->index = i;

            // Read into buffer.
            rc = fread(buffer, SKY_BLOCK_INFO_SIZE, 1, file);
            check(rc == 1, "Unable to read block #%d", block->index);
            
            // Unpack from buffer.
            rc = sky_block_unpack(block, buffer, &sz);
            check(rc == 0, "Unable to unpack block #%d", block->index);
            
            data_file->blocks[i] = block;
        }

        // Close the file.
        fclose(file);

        // Sort ranges by starting object id.
        qsort(data_file->blocks, data_file->block_count, sizeof(sky_block*), compare_blocks);

        // Determine spanned blocks.
        sky_object_id_t last_object_id = -1;
        for(i=0; i<data_file->block_count; i++) {
            sky_block *block = data_file->blocks[i];
            
            // If this is a single object block then track the object id to
            // possibly mark it as spanned.
            if(block->min_object_id == block->max_object_id && block->min_object_id > 0) {
                // If it has spanned since the last block then mark it and the
                // previous block.
                if(block->min_object_id == last_object_id) {
                    block->spanned = true;
                    data_file->blocks[i-1]->spanned = true;
                }
                // If this is the first block with one object then store the id.
                else {
                    last_object_id = block->min_object_id;
                }
            }
            // Clear out last object id for multi-object blocks.
            else {
                last_object_id = -1;
            }
        }
    }

    return 0;

error:
    if(file) fclose(file);
    return -1;
}


//======================================
// Block Management
//======================================

// Appends an empty block at the end of the data file.
//
// data_file - The data file.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_create_block(sky_data_file *data_file)
{
    int rc;
    
    // Increment block count and resize block memory.
    data_file->block_count++;
    data_file->blocks = realloc(data_file->blocks, sizeof(sky_block*) * data_file->block_count);
    if(data_file->block_count > 0) check_mem(data_file->blocks);

    // Create new block.
    sky_block *block = sky_block_create(data_file); check_mem(block);
    block->index = data_file->block_count-1;
    data_file->blocks[data_file->block_count-1] = block;

    // Remap data file.
    rc = sky_data_file_load(data_file);
    check(rc == 0, "Unable to reload data file");

    // Clear block.
    sky_block *block = sky_block_create(data_file);
    rc = save_block(block);
    check(rc == 0, "Unable to initialize new block");

    // Sort blocks.
    qsort(data_file->blocks, data_file->block_count, sizeof(sky_block*), compare_blocks);

    // Return the block.
    *ret = block;

    return 0;

error:
    return -1;
}