#include <stdlib.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "dbg.h"
#include "mem.h"
#include "bstring.h"
#include "file.h"
#include "data_file.h"

//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int sky_data_file_unmap(sky_data_file *data_file);

int sky_data_file_load_header(sky_data_file *data_file);
int sky_data_file_unload_header(sky_data_file *data_file);
int sky_data_file_create_header(sky_data_file *data_file);

int sky_data_file_normalize(sky_data_file *data_file);

int compare_blocks(const void *_a, const void *_b);


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
    data_file->block_size = SKY_DEFAULT_BLOCK_SIZE;
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
        sky_data_file_unload_header(data_file);
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
    check(data_file != NULL, "Data file required");
    check(data_file->path != NULL, "Data file path required");

    // Load header if not loaded yet.
    if(data_file->blocks == NULL) {
        rc = sky_data_file_load_header(data_file);
        check(rc == 0, "Unable to load header");
    }

    // There should always be at least one block.
    check(data_file->block_size > 0, "Data file should have at least one block");
    
    // Calculate the data length.
    size_t data_length = data_file->block_count * data_file->block_size;

    // Close mapping if remapping isn't supported.
    if(!MREMAP_AVAILABLE) {
        rc = sky_data_file_unmap(data_file);
        check(rc == 0, "Unable to unmap data file");
    }

    // Open the data file and map it if it is not currently open.
    if(data_file->data_fd == 0) {
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

    // Update the data file.
    data_file->data = ptr;
    data_file->data_length = data_length;

    return 0;

error:
    sky_data_file_unload(data_file);
    return -1;
}

// Unloads the data file from memory.
//
// data_file - The data file to save.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_unload(sky_data_file *data_file)
{
    // Unload header.
    sky_data_file_unload_header(data_file);
    
    // Unmap the data file.
    sky_data_file_unmap(data_file);
    
    return 0;
}

// Unmaps the data file but does not unload the header.
//
// data_file - The data file to save.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_unmap(sky_data_file *data_file)
{
    // Unmap file.
    if(data_file->data != NULL) {
        munmap(data_file->data, data_file->data_length);
    }
    
    // Close file descriptor.
    if(data_file->data_fd != 0) {
        close(data_file->data_fd);
    }
    
    data_file->data_fd = 0;
    data_file->data = NULL;
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

    // Unload existing header information.
    rc = sky_data_file_unload_header(data_file);
    check(rc == 0, "Unable to unload header");

    // If header doesn't exist then create a new one.
    if(!sky_file_exists(data_file->header_path)) {
        rc = sky_data_file_create_header(data_file);
        check(rc == 0, "Unable to create header file");
    }
    
    file = fopen(bdata(data_file->header_path), "r");
    check(file, "Failed to open header file for reading: %s",  bdata(data_file->header_path));

    // Read database format version.
    rc = fread(&version, sizeof(version), 1, file);
    check(rc == 1, "Unable to read version");

    // Read block size.
    rc = fread(&data_file->block_size, sizeof(data_file->block_size), 1, file);
    check(rc == 1, "Unable to read block size");

    // Read blocks until end of file.
    off_t file_length = sky_file_get_size(data_file->header_path);
    while(ftell(file) < file_length && !feof(file)) {
        sky_block *block = sky_block_create(data_file); check_mem(block);
        block->index = data_file->block_count;

        // Read into buffer.
        rc = fread(buffer, SKY_BLOCK_HEADER_SIZE, 1, file);
        check(rc == 1, "Unable to read block #%d", block->index);
        
        // Unpack from buffer.
        rc = sky_block_unpack(block, buffer, &sz);
        check(rc == 0, "Unable to unpack block #%d", block->index);
        
        // Append to blocks
        data_file->block_count++;
        data_file->blocks = realloc(data_file->blocks, sizeof(sky_block*) * data_file->block_count);
        check_mem(data_file->blocks);
        data_file->blocks[data_file->block_count-1] = block;
    }

    // Close the file.
    fclose(file);

    rc = sky_data_file_normalize(data_file);
    check(rc == 0, "Unable to normalize data file");
    

    return 0;

error:
    if(file) fclose(file);
    sky_data_file_unload_header(data_file);
    return -1;
}

// Unloads header information from memory.
//
// data_file - The data file object associated with the header file.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_unload_header(sky_data_file *data_file)
{
    check(data_file != NULL, "Data file required");
    
    if(data_file->blocks) {
        uint32_t i;
        for(i=0; i<data_file->block_count; i++) {
            sky_block *block = data_file->blocks[i];
            if(block) {
                sky_block_free(block);
                data_file->blocks[i] = NULL;
            }
        }
        free(data_file->blocks);
    }
    data_file->blocks = NULL;
    data_file->block_count = 0;
    
    return 0;
    
error:
    return -1;
}

// Creates a new header file. The header file will only be created if one
// does not already exist.
//
// data_file - The data file object associated with the header file.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_create_header(sky_data_file *data_file)
{
    int rc;
    check(data_file != NULL, "Data file required");
    check(data_file->header_path != NULL, "Data file header path required");
    check(!sky_file_exists(data_file->header_path), "Header file already exists");

    // Open the file for writing.
    FILE *file = fopen(bdata(data_file->header_path), "w");
    check(file, "Failed to open header file for writing: %s",  bdata(data_file->header_path));

    // Write database format version.
    uint32_t version = SKY_DATA_FILE_VERSION;
    rc = fwrite(&version, sizeof(version), 1, file);
    check(rc == 1, "Unable to write version");

    // Write block size.
    rc = fwrite(&data_file->block_size, sizeof(data_file->block_size), 1, file);
    check(rc == 1, "Unable to write block size");
    
    // Write a single empty block.
    uint8_t *buffer[SKY_BLOCK_HEADER_SIZE];
    memset(buffer, 0, SKY_BLOCK_HEADER_SIZE);
    rc = fwrite(buffer, SKY_BLOCK_HEADER_SIZE, 1, file);
    check(rc == 1, "Unable to write initial block header");
    
    // Close file.
    fclose(file);
    
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
// ret       - A pointer to where the new block should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_create_block(sky_data_file *data_file, sky_block **ret)
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
    void *ptr;
    rc = sky_block_get_ptr(block, &ptr);
    check(rc == 0, "Unable to retrieve block pointer");
    memset(ptr, 0, data_file->block_size);

    // Re-sort blocks.
    qsort(data_file->blocks, data_file->block_count, sizeof(sky_block*), compare_blocks);

    // Return the new block.
    *ret = block;

    return 0;

error:
    return -1;
}

// Finds the correct block to add an event to.
//
// Insertion has several rules to ensure consistency. There are two types of
// blocks: single object blocks and multi-object blocks. Single object blocks
// are spans of two or more blocks that have a single object. Multi-object
// blocks are single blocks that have multiple object ids. An object that is
// stored in a single block (i.e. doesn't span blocks) is considered a
// multi-object block since there is probably room to add more objects.
// 
// 1. An object id range is unique to a block. If a block has object ids 5 - 8
//    then no other block can overlap into those ids.
//
// 2. Only multi-object blocks can have object ids outside its range added.
//
// 3. Objects are added to the first block where the object id is included in
//    the object id range or is less than the minimum object id of the block.
//
// 4. If no block is found by the end then the last block is used for insertion.
//
// data_file - The data file that the event is being added to.
// event     - The event to add to the table.
// ret       - A pointer to where the insertion block is returned to.
//
// Returns 0 if successfully finds a block. Otherwise returns -1.
int sky_data_file_find_insertion_block(sky_data_file *data_file,
                                       sky_event *event,
                                       sky_block **ret)
{
    int rc;
    check(data_file != NULL, "Data file required");
    check(event != NULL, "Event required");
    check(event->object_id != 0, "Event object id required");
    
    // Initialize return value to NULL.
    *ret = NULL;
    
    // Extract object id and timestamp from event.
    sky_object_id_t object_id = event->object_id;
    sky_timestamp_t timestamp = event->timestamp;

    // Loop over sorted blocks to find the appropriate insertion point.
    uint32_t i;
    sky_block *block = NULL;
    for(i=0; i<data_file->block_count; i++) {
        block = data_file->blocks[i];
        
        // If block is within range then use the block.
        if(object_id >= block->min_object_id && object_id <= block->max_object_id) {
            // If this is a single object block then find the appropriate block
            // based on timestamp.
            if(block->spanned) {
                // Find first block where timestamp is before the max.
                while(i<data_file->block_count && data_file->blocks[i]->min_object_id == object_id) {
                    if(timestamp <= data_file->blocks[i]->max_timestamp) {
                        *ret = block;
                        break;
                    }
                    i++;
                }
                
                // If this event is being appended to the object path then use
                // the last block.
                if(*ret == NULL) {
                    *ret = data_file->blocks[i-1];
                }

                break;
            }
            // If this is a multi-object block then simply use it.
            else {
                *ret = block;
                break;
            }
        }
        // If block is before this object id range, then use the block if it
        // is a multi-object block.
        else if(object_id < block->min_object_id && !block->spanned) {
            *ret = block;
            break;
        }
    }
    
    // If we haven't found a block then it means that the object id is after all
    // other object ids or that we are inserting before a single object block or
    // that we have no blocks.
    if(*ret == NULL) {
        // Find the last block if one exists.
        sky_block *last_block = (data_file->block_count > 0 ? data_file->blocks[data_file->block_count-1] : NULL);
        
        // If the last block available is unspanned then use it.
        if(last_block != NULL && !last_block->spanned) {
            *ret = last_block;
        }
        // Otherwise just create a new block.
        else {
            rc = sky_data_file_create_block(data_file, ret);
            check(rc == 0, "Unable to create block");
        }
    }
    
    return 0;

error:
    return -1;
}

// Updates the block information to mark blocks as spanned or not.
//
// data_file - The data file.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_normalize(sky_data_file *data_file)
{
    // Sort ranges by starting object id.
    qsort(data_file->blocks, data_file->block_count, sizeof(*data_file->blocks), compare_blocks);

    // Determine spanned blocks.
    uint32_t i;
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

    return 0;
}

// Moves a specific number of bytes from a given pointer to a newly created
// block. The pointer passed in may change if the data file is remapped to a
// different memory location.
//
// data_file - The data file that contains the pointer.
// ptr       - A pointer to a location on the data file.
// sz        - The number of bytes to move to a new block.
// new_block - A pointer to where the new block should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_move_to_new_block(sky_data_file *data_file, void **ptr,
                                    size_t sz, sky_block **new_block)
{
    int rc;
    check(data_file != NULL, "Data file required");

    // Store offset before data file remap.
    off_t ptr_off = (*ptr) - data_file->data;

    // Create block.
    rc = sky_data_file_create_block(data_file, new_block);
    check(rc == 0, "Unable to create new block");
    
    // Restore pointers in case remap relocated data.
    *ptr = data_file->data + ptr_off;

    // Retrieve new block's pointer.
    void *new_block_ptr = NULL;
    rc = sky_block_get_ptr(*new_block, &new_block_ptr);
    check(rc == 0, "Unable to determine new block pointer");
    
    // Copy over everything since the checkpoint.
    memmove(new_block_ptr, (*ptr), sz);

    // Clear out data from the source block.
    memset((*ptr), 0, sz);
    
    // Perform a full update on the block header. This is probably not
    // needed but we're going to be safe. The amortized cost of this
    // should be small considering splits shouldn't happen often.
    rc = sky_block_full_update(*new_block);
    check(rc == 0, "Unable to update block ranges on new block");

    return 0;
    
error:
    return -1;
}


//======================================
// Event Management
//======================================

// Adds an event to the data file.
//
// data_file - The data file to add the event to..
// event     - The event to add.
//
// Returns 0 if successful, otherwise returns -1.
int sky_data_file_add_event(sky_data_file *data_file, sky_event *event)
{
    int rc;
    check(data_file != NULL, "Data file required");
    check(event != NULL, "Event required");
    
    // Find insertion block.
    sky_block *block;
    rc = sky_data_file_find_insertion_block(data_file, event, &block);
    check(rc == 0, "Unable to find insertion block");
    
    // Add the event to the block.
    rc = sky_block_add_event(block, event);
    check(rc == 0, "Unable to add event to block");

    // Re-sort blocks.
    qsort(data_file->blocks, data_file->block_count, sizeof(sky_block*), compare_blocks);
    
    return 0;

error:
    return -1;
}


//======================================
// Block Sorting
//======================================

// Compares two blocks and sorts them based on starting min object identifier
// and then by id.
int compare_blocks(const void *_a, const void *_b)
{
    sky_block **a = (sky_block **)_a;
    sky_block **b = (sky_block **)_b;

    // Sort by min object id first.
    if((*a)->min_object_id > (*b)->min_object_id) {
        return 1;
    }
    else if((*a)->min_object_id < (*b)->min_object_id) {
        return -1;
    }
    else {
        // If min object ids are the same then sort by min timestamp.
        if((*a)->min_timestamp > (*b)->min_timestamp) {
            return 1;
        }
        else if((*a)->min_timestamp < (*b)->min_timestamp) {
            return -1;
        }
        else {
            // If min timestamps are the same then sort by block id.
            if((*a)->index > (*b)->index) {
                return 1;
            }
            else if((*a)->index < (*b)->index) {
                return -1;
            }
            else {
                return 0;
            }
        }
    }
}

