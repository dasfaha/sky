#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "dbg.h"
#include "endian.h"
#include "bstring.h"
#include "mem.h"
#include "block.h"
#include "path.h"
#include "path_iterator.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a block object.
//
// Returns a reference to a new block object if successful. Otherwise
// returns null.
sky_block *sky_block_create(sky_data_file *data_file)
{
    sky_block *block = calloc(sizeof(sky_block), 1);
    check_mem(block);

    block->data_file = data_file;
    
    return block;
    
error:
    sky_block_free(block);
    return NULL;
}

// Removes a block object from memory.
void sky_block_free(sky_block *block)
{
    if(block) {
        memset(block, 0, sizeof(*block));
        free(block);
    }
}


//======================================
// Persistence
//======================================

// Syncs the in-memory block back to disk.
//
// block - The block to save.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_save(sky_block *block)
{
    int rc;
    check(block != NULL, "Block required");

    // Retrieve the location of 
    void *ptr = NULL;
    rc = sky_block_get_ptr(block, &ptr);
    check(rc == 0, "Unable to retrieve block data pointer");
    
    // Sync the memory for the block.
    rc = msync(ptr, block->data_file->block_size, MS_SYNC);
    check(rc == 0, "Unable to sync block to disk");
    
    return 0;
    
error:
    return -1;
}


//======================================
// Serialization
//======================================

// Packs a block object into memory at a given pointer.
//
// block - The block object to pack.
// ptr   - The pointer to the current location.
// sz    - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_pack(sky_block *block, void *ptr, size_t *sz)
{
    // Validate.
    check(block != NULL, "Block required");
    check(ptr != NULL, "Pointer required");
    
    // Write object id range.
    *((sky_object_id_t*)ptr) = block->min_object_id;
    ptr += sizeof(sky_object_id_t);
    *((sky_object_id_t*)ptr) = block->max_object_id;
    ptr += sizeof(sky_object_id_t);

    // Write timestamp range.
    *((sky_timestamp_t*)ptr) = block->min_timestamp;
    ptr += sizeof(sky_timestamp_t);
    *((sky_timestamp_t*)ptr) = block->max_timestamp;
    ptr += sizeof(sky_timestamp_t);

    // Store number of bytes written.
    if(sz != NULL) {
        *sz = SKY_BLOCK_HEADER_SIZE;
    }
    
    return 0;

error:
    return -1;
}

// Unpacks a block object from memory at the current pointer.
//
// block - The block object to unpack into.
// ptr  - The pointer to the current location.
// sz   - The number of bytes read.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_unpack(sky_block *block, void *ptr, size_t *sz)
{
    // Validate.
    check(block != NULL, "Block required");
    check(ptr != NULL, "Pointer required");

    // Read object id range.
    block->min_object_id = *((sky_object_id_t*)ptr);
    ptr += sizeof(sky_object_id_t);
    block->max_object_id = *((sky_object_id_t*)ptr);
    ptr += sizeof(sky_object_id_t);

    // Read timestamp range.
    block->min_timestamp = *((sky_timestamp_t*)ptr);
    ptr += sizeof(sky_timestamp_t);
    block->max_timestamp = *((sky_timestamp_t*)ptr);
    ptr += sizeof(sky_timestamp_t);

    // Store number of bytes read.
    if(sz != NULL) {
        *sz = SKY_BLOCK_HEADER_SIZE;
    }
    
    return 0;

error:
    *sz = 0;
    return -1;
}


//======================================
// Header Management
//======================================

// Updates the block object id and timestamp ranges and saves the changes to
// the header file as required.
//
// block     - The block to update ranges for.
// object_id - The object id being added to the block.
// timestamp - The timestamp being added to the block.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_update(sky_block *block, sky_object_id_t object_id,
                     sky_timestamp_t timestamp)
{
    int rc, fd;
    FILE *file;
    check(block != NULL, "Block required");
    check(object_id != 0, "Object id cannot be zero");

    // Check if the object id or timestamp is out of range.
    bool is_empty = (block->min_object_id == 0 && block->max_object_id == 0);
    bool object_id_changed = (object_id < block->min_object_id || object_id > block->max_object_id);
    bool timestamp_changed = (timestamp < block->min_timestamp || timestamp > block->max_timestamp);
    
    // If anything is out of range then update block and save.
    if(is_empty || object_id_changed || timestamp_changed) {
        // Update block ranges.
        if(is_empty || object_id < block->min_object_id) {
            block->min_object_id = object_id;
        }
        if(is_empty || object_id > block->max_object_id) {
            block->max_object_id = object_id;
        }
        if(is_empty || timestamp < block->min_timestamp) {
            block->min_timestamp = timestamp;
        }
        if(is_empty || timestamp > block->max_timestamp) {
            block->max_timestamp = timestamp;
        }
        
        // Open header file.
        fd = open(bdata(block->data_file->header_path), O_WRONLY);
        check(fd != 0, "Unable to open header file for block update");
        file = fdopen(fd, "w");
        check(file != NULL, "Unable to open header file stream for block update");
        
        // Write to buffer.
        size_t sz;
        uint8_t *buffer[SKY_BLOCK_HEADER_SIZE];
        rc = sky_block_pack(block, buffer, &sz);
        check(rc == 0, "Unable to pack block header data");
        
        // Determine header file position.
        off_t offset;
        rc = sky_block_get_header_offset(block, &offset);
        check(rc == 0, "Unable to determine block offset in header file");
        rc = fseek(file, offset, SEEK_SET);
        check(rc == 0, "Unable to position file at block position: %lld", offset);
        
        // Write to file.
        rc = fwrite(buffer, SKY_BLOCK_HEADER_SIZE, 1, file);
        check(rc == 1, "Unable to write block to header file");
        
        // Close header file.
        fclose(file);
        close(fd);
    }

    return 0;

error:
    if(file) fclose(file);
    if(fd) close(fd);
    return -1;
}    

// Calculates the byte position of the block inside the header file using the
// block's index.
//
// block  - The block to determine the position for.
// offset - The offset, in bytes, from the beginning of the header file where
//          the block data begins.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_get_header_offset(sky_block *block, off_t *offset)
{
    check(block != NULL, "Block required");
    check(offset != NULL, "Offset pointer required");

    *offset = ((uint32_t)SKY_HEADER_FILE_HDR_SIZE) + (block->index * ((uint32_t)SKY_BLOCK_HEADER_SIZE));
    return 0;
    
error:
    *offset = 0;
    return -1;
}

//======================================
// Block Position
//======================================

// Calculates the byte offset for the beginning on the block in the
// data file based on the data file block size and the block index.
//
// block  - The block to calculate the byte offset of.
// offset - A pointer to where the offset will be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_get_offset(sky_block *block, size_t *offset)
{
    check(block != NULL, "Block required");
    check(block->data_file != NULL, "Data file required");
    check(block->data_file->block_size != 0, "Data file must have a nonzero block size");

    *offset = (block->data_file->block_size * block->index);
    return 0;

error:
    *offset = 0;
    return -1;
}

// Calculates the pointer position for the beginning on the block in the
// data file based on the data file block size and the block index.
//
// block - The block to calculate the byte offset of.
// ptr   - A pointer to where the blocks starting address will be set.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_get_ptr(sky_block *block, void **ptr)
{
    check(block != NULL, "Block required");
    check(block->data_file != NULL, "Data file required");
    check(block->data_file->data != NULL, "Data file must be mapped");

    // Retrieve the offset.
    size_t offset;
    int rc = sky_block_get_offset(block, &offset);
    check(rc == 0, "Unable to determine block offset");

    // Calculate pointer based on data pointer.
    *ptr = block->data_file->data + offset;
    
    return 0;

error:
    *ptr = NULL;
    return -1;
}


//======================================
// Spanning
//======================================

// Determines the number of blocks that this block's object spans. This
// function only works on the starting block of a span of blocks.
//
// block - The initial block in a block span.
// count - A pointer to where the number of spanned blocks should be stored.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_get_span_count(sky_block *block, uint32_t *count)
{
    check(block != NULL, "Block required");
    check(block->data_file != NULL, "Data file required");
    check(count != NULL, "Span count address required");
    
    sky_data_file *data_file = block->data_file;
    sky_block **blocks = data_file->blocks;

    // If this block is not spanned then return 1.
    if(!block->spanned) {
        *count = 1;
    }
    // Otherwise calculate the span count.
    else {
        // Loop until the ending block of the span is found.
        uint32_t index = block->index;
        sky_object_id_t object_id = block->min_object_id;
        while(true) {
            index++;

            // If we've reached the end of the blocks or if the object id no longer
            // matches the starting object id then break out of the loop.
            if(index > data_file->block_count-1 || object_id != blocks[index]->min_object_id)
            {
                break;
            }
        }

        // Assign count back to caller's provided address.
        *count = (index - block->index);
    }
    
    return 0;

error:
    *count = 0;
    return -1;
}


//======================================
// Splitting
//======================================

// Splits a block into multiple blocks based on the the addition of an event.
// The paths are placed into multiple buckets depending on their sizes and
// order and then are evenly distributed across the blocks.
//
// block - The block to split.
// event - The event that will be added to the block.
// ret   - A reference to the block that the event will be added to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_split_with_event(sky_block *block, sky_event *event,
                               sky_block **ret)
{
    int rc;
    check(block != NULL, "Block required");
    check(event != NULL, "Event required");
    
    // Initialize the return value.
    *ret = NULL;
    
    // Create a path iterator and point it at the block.
    sky_path_iterator *iterator = sky_path_iterator_create();
    rc = sky_path_iterator_set_block(iterator, block);
    check(rc == 0, "Unable to set path iterator block");
    
    void *ptr;
    sky_object_id_t object_id = event->object_id;

    // Loop over paths in block.
    while(!iterator->eof) {
        // If the current path matches then save the ptr and exit.
        if(object_id == iterator->current_object_id) {
            rc = sky_path_iterator_get_ptr(iterator, &ptr);
            check(rc == 0, "Unable to retrieve iterator's current pointer");
            break;
        }
    }
    
    sky_path_iterator_free(iterator);
    
    return 0;

error:
    return -1;
}


//======================================
// Path Management
//======================================

// Retrieves a pointer to the start of a path with a given object id inside
// the block. If no path exists with the given object id exists then a null
// pointer is returned.
//
// block     - The block to search.
// object_id - The object id to search for.
// ret       - A pointer to where the path's pointer should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_get_path_ptr(sky_block *block, sky_object_id_t object_id,
                           void **ret)
{
    int rc;
    check(block != NULL, "Block required");
    check(object_id > 0, "Object id required");

    // Create a path iterator and point it at the block.
    sky_path_iterator iterator;
    sky_path_iterator_init(&iterator);
    rc = sky_path_iterator_set_block(&iterator, block);
    check(rc == 0, "Unable to set path iterator block");
    
    // Initialize the return value.
    *ret = NULL;
    
    // Loop over iterator until we find the path matching the object id.
    while(!iterator.eof) {
        // If the current path matches then save the ptr and exit.
        if(object_id == iterator.current_object_id) {
            rc = sky_path_iterator_get_ptr(&iterator, ret);
            check(rc == 0, "Unable to retrieve iterator's current pointer");
            break;
        }
        
        // Move to next path.
        rc = sky_path_iterator_next(&iterator);
        check(rc == 0, "Unable to move to next path");
    }
    
    return 0;

error:
    *ret = NULL;
    return -1;
}



//======================================
// Event Management
//======================================

// Adds an event to the block. If the size of the block exceeds the block size
// then a new empty block is allocated and half the paths in the block are
// moved to the new block.
//
// block - The block to add the event to.
// event - The event to add to the block.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_add_event(sky_block *block, sky_event *event)
{
    int rc;
    check(block != NULL, "Block required");
    check(event != NULL, "Event required");

    // Store the block pointer.
    void *block_ptr;
    rc = sky_block_get_ptr(block, &block_ptr);
    check(rc == 0, "Unable to retrieve block pointer");

    // Initialize path iterator.
    sky_path_iterator iterator;
    sky_path_iterator_init(&iterator);
    rc = sky_path_iterator_set_block(&iterator, block);
    check(rc == 0, "Unable to set path iterator block");

    // Setup pointers to the insertion path & event insertion point.
    void *path_ptr  = NULL;
    void *event_ptr = NULL;

    // Loop over iterator until we find the path or insertion point.
    while(!iterator.eof) {
        // If we reached the correct path then retrieve the path pointer
        // and then use a cursor to find the insertion point.
        if(event->object_id == iterator.current_object_id) {
            // Save path pointer.
            rc = sky_path_iterator_get_ptr(&iterator, &path_ptr);
            check(rc == 0, "Unable to retrieve iterator's current pointer");
            
            // Use cursor to find event insertion point.
            sky_cursor cursor;
            sky_cursor_init(&cursor);
            sky_cursor_set_path(&cursor, path_ptr);
            check(rc == 0, "Unable to set cursor path");
            
            // Loop over cursor until we reach the event insertion point.
            while(!cursor.eof) {
                sky_timestamp_t timestamp;
                sky_action_id_t action_id;
                sky_event_data_length_t data_length;

                // Retrieve current timestamp in cursor.
                size_t hdrsz;
                rc = sky_event_unpack_hdr(&timestamp, &action_id, &data_length, cursor.ptr, &hdrsz);
                check(rc == 0, "Unable to unpack event header");
                
                // Retrieve event insertion pointer once the timestamp is
                // reached.
                if(timestamp >= event->timestamp) {
                    event_ptr = cursor.ptr;
                    break;
                }
                
                // Move to next event.
                rc = sky_cursor_next(&cursor);
                check(rc == 0, "Unable to move to next event");
            }
            
            // If no insertion point was found then append the event to the
            // end of the path.
            if(event_ptr == NULL) {
                event_ptr = path_ptr + sky_path_sizeof_raw(path_ptr);
            }
        }
        // If we are beyond the object id then exit and use the current
        // pointer as the insertion point.
        else if(path_ptr == NULL && iterator.current_object_id > event->object_id) {
            rc = sky_path_iterator_get_ptr(&iterator, &path_ptr);
            check(rc == 0, "Unable to retrieve iterator's current pointer");
        }
        
        // Move to next path.
        rc = sky_path_iterator_next(&iterator);
        check(rc == 0, "Unable to move to next path");
    }
    
    // Save the length of the data in the block. This value can only be 
    // determined after the iterator has reached EOF.
    size_t block_data_length = iterator.block_data_length;

    // If we reached EOF and found no path insertion point then move the
    // pointer to the end of the last path.
    if(path_ptr == NULL) {
        path_ptr = block_ptr + block_data_length;
    }

    // Determine size.
    bool path_exists = (event_ptr != NULL);
    size_t event_length = sky_event_sizeof(event);
    size_t sz = event_length + (path_exists ? 0 : SKY_PATH_HEADER_LENGTH);
    
    // Shift data down in the block so we have enough room.
    void *ptr = (path_exists ? event_ptr : path_ptr);
    debug("[SHIFT] %p (%ld) | %p (%ld)", block_ptr, block_data_length, ptr, sz);
    memmove(ptr+sz, ptr, block_data_length-(ptr-block_ptr));
    
    // Pack the path first if it is missing.
    if(!path_exists) {
        rc = sky_path_pack_hdr(event->object_id, event_length, ptr, &sz);
        check(rc == 0, "Unable to pack path header");
        
        // Point event pointer at the beginning of the path event data.
        event_ptr = path_ptr + SKY_PATH_HEADER_LENGTH;
    }
    // Or update the path event length if a path exists.
    else {
        *(sky_path_event_data_length_t*)(path_ptr+sizeof(sky_object_id_t)) += event_length;
    }
    
    // Pack event.
    size_t event_sz;
    rc = sky_event_pack(event, event_ptr, &event_sz);
    check(rc == 0, "Unable to pack event");
    
    // Save block to disk.
    rc = sky_block_save(block);
    check(rc == 0, "Unable to save block");
    
    // Update header.
    rc = sky_block_update(block, event->object_id, event->timestamp);
    check(rc == 0, "Unable to write block to header");
    
    return 0;

error:
    return -1;
}
