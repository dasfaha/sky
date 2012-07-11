#include <stdlib.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <math.h>

#include "dbg.h"
#include "mem.h"
#include "endian.h"
#include "bstring.h"
#include "file.h"
#include "database.h"
#include "block.h"
#include "table.h"

//==============================================================================
//
// Forward Declarations
//
//==============================================================================

//--------------------------------------
// Header file
//--------------------------------------

int sky_table_load_header_file(sky_table *table);

int sky_table_unload_header_file(sky_table *table);


//--------------------------------------
// Action file
//--------------------------------------

int sky_table_load_action_file(sky_table *table);

int sky_table_unload_action_file(sky_table *table);


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Block Management
//======================================

// Serializes a block in memory and writes it to disk.
// 
// block - The in-memory block to write to disk.
//
// Returns 0 if successful. Otherwise returns -1.
int save_block(sky_block *block)
{
    int rc;

    sky_table *table = block->table;
    sky_block_info *info = block->info;

    // Move pointer to starting position of block.
    size_t offset = get_block_offset(table, info);
    void *addr = table->data + offset;

    // Pack block.
    rc = sky_block_pack(block, addr, NULL);
    check(rc == 0, "Failed to pack block: %d", info->id);

    // Update block info.
    sky_block_update_info(block);

    return 0;

error:
    return -1;
}

// Creates an empty block in the table and returns it.
//
// table - The table that will own the new block.
// ret         - The block object returned to the caller.
//
// Returns 0 if successful, otherwise returns -1.
int create_block(sky_table *table, sky_block **ret)
{
    int rc;
    
    // Increment block count and resize block info memory.
    table->block_count++;
    table->infos = realloc(table->infos, sizeof(sky_block_info*) * table->block_count);
    if(table->block_count > 0) check_mem(table->infos);

    // Create new block.
    sky_block_info *info = malloc(sizeof(sky_block_info)); check_mem(info);
    info->id = table->block_count-1;
    info->min_object_id = 0LL;
    info->max_object_id = 0LL;
    info->spanned = false;
    table->infos[table->block_count-1] = info;

    // Remap data file.
    rc = map_data_file(table);
    check(rc == 0, "Unable to remap data file");

    // Save empty block to file.
    sky_block *block = sky_block_create(table, info);
    rc = save_block(block);
    check(rc == 0, "Unable to initialize new block");

    // Sort blocks.
    sort_blocks(table->infos, table->block_count);

    // Return the block.
    *ret = block;

    return 0;
    
error:
    if(block) sky_block_free(block);
    *ret = block = NULL;

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
// table - The table that the event is being added to.
// event       - The event to add to the table.
// ret         - A reference to the correct block info returned to the caller.
//
// Returns 0 if successfully finds a block. Otherwise returns -1.
int find_insertion_block(sky_table *table, sky_event *event, sky_block_info **ret)
{
    int i, n, rc;
    sky_block_info *info = NULL;
    info = NULL;
    
    // Initialize return value to NULL.
    *ret = NULL;
    
    // Extract object id and timestamp from event.
    sky_object_id_t object_id = event->object_id;
    sky_timestamp_t timestamp = event->timestamp;

    // Loop over sorted blocks to find the appropriate insertion point.
    n = table->block_count;
    for(i=0; i<n; i++) {
        info = table->infos[i];
        
        // If block is within range then use the block.
        if(object_id >= info->min_object_id && object_id <= info->max_object_id) {
            // If this is a single object block then find the appropriate block
            // based on timestamp.
            if(info->spanned) {
                // Find first block where timestamp is before the max.
                while(i<n && table->infos[i]->min_object_id == object_id) {
                    if(timestamp <= table->infos[i]->max_timestamp) {
                        *ret = info;
                        break;
                    }
                    i++;
                }
                
                // If this event is being appended to the object path then use
                // the last block.
                if(*ret == NULL) {
                    *ret = table->infos[i-1];
                }

                break;
            }
            // If this is a multi-object block then simply use it.
            else {
                *ret = info;
                break;
            }
        }
        // If block is before this object id range, then use the block if it
        // is a multi-object block.
        else if(object_id < info->min_object_id && !info->spanned) {
            *ret = info;
            break;
        }
    }
    
    // If we haven't found a block then it means that the object id is after all
    // other object ids or that we are inserting before a single object block or
    // that we have no blocks.
    if(*ret == NULL) {
        // Find the last block if one exists.
        sky_block_info *last_info = (table->block_count > 0 ? table->infos[table->block_count-1] : NULL);
        
        // If the last block available is unspanned then use it.
        if(last_info != NULL && !last_info->spanned) {
            *ret = last_info;
        }
        // Otherwise just create a new block.
        else {
            sky_block *block;
            rc = create_block(table, &block);
            check(rc == 0, "Unable to create block");
            *ret = block->info;
            sky_block_free(block);
        }
    }
    
    return 0;

error:
    return -1;
}

// Reads a block in from disk and unpacks it.
// 
// table - The table that contains the block.
// info        - A reference to the block position.
// ret         - The reference to the block returned to the caller.
//
// Returns 0 if successful. Otherwise returns -1.
int load_block(sky_table *table, sky_block_info *info, sky_block **ret)
{
    int rc;
    
    // Create pointer at starting position of block.
    size_t offset = get_block_offset(table, info);
    void *addr = table->data + offset;

    // Unpack block.
    sky_block *block = sky_block_create(table, info);
    rc = sky_block_unpack(block, addr, NULL);
    check(rc == 0, "Failed to unpack block: %d", info->id);
    
    // Assign block to return value.
    *ret = block;
    
    return 0;

error:
    return -1;
}

// Creates a target block and appends it to a list of blocks.
//
// source - The block that is the source of paths/events for the target block.
// target - The target block that will be returned.
// blocks - An array of blocks.
// count  - The number of blocks in the array.
//
// Returns 0 if successful, otherwise returns -1.
int create_target_block(sky_block *source, sky_block **target, sky_block ***blocks, int *count)
{
    int rc;
    
    // Create new block.
    rc = create_block(source->table, target);
    check(rc == 0, "Unable to create target block");
    
    // Increment block count.
    (*count)++;
    *blocks = realloc(*blocks, sizeof(sky_block*) * (*count));
    check_mem(*blocks);
    
    // Append target block.
    (*blocks)[*count-1] = *target;

    return 0;

error:
    if(*target) sky_block_free(*target);
    *target = NULL;

    return -1;    
}

// Attempts to split a given block into multiple smaller blocks if the block
// exceeds the maximum block size allowed for an table.
//
// block           - The block to attempt to split.
// affected_blocks - An array of blocks. This includes the original block passed
//                   in as well as any blocks split off the original.
// affected_block_count - The number of blocks in the affected_blocks array.
//
// Returns 0 if successful, otherwise returns -1.
int split_block(sky_block *block, sky_block ***affected_blocks, int *affected_block_count)
{
    int rc;
    sky_block *target_block;
    sky_event **events;
    uint32_t event_count;

    // Add original block to list of affected blocks.
    *affected_block_count = 1;
    *affected_blocks = malloc(sizeof(sky_block*)); check_mem(*affected_blocks);
    (*affected_blocks)[0] = block;

    // If block size has not been exceeded then exit this function immediately.
    uint32_t block_packed_length = sky_block_sizeof(block);
    if(block_packed_length <= block->table->block_size) {
        return 0;
    }
    
    // Extract paths and info from original block.
    bool spanned = block->info->spanned;
    sky_path **paths = block->paths;
    uint32_t path_count = block->path_count;
    block->paths = NULL;
    block->path_count = 0;
    
    // Assign original block as first target block.
    target_block = block;
    
    // Calculate target block size if we were to spread paths evenly across blocks.
    uint32_t max_block_size = block->table->block_size - MAX_BLOCK_HEADER_LENGTH;
    uint32_t target_block_count = (uint32_t)ceil((double)block_packed_length / (double)max_block_size);
    uint32_t target_block_size = block_packed_length / target_block_count;
    
    // Loop over paths and spread them across blocks.
    uint32_t i, j;
    for(i=0; i<path_count; i++) {
        sky_path *path = paths[i];
        uint32_t path_packed_length = sky_path_sizeof(path);
        
        // If path is already spanned or the path is larger than max block size
        // then spread its events across multiple blocks.
        if(spanned || path_packed_length > max_block_size) {
            // Extract events from path.
            events = path->events;
            event_count = path->event_count;
            path->events = NULL;
            path->event_count = 0;

            // Mark target block as spanned.
            target_block->info->spanned = true;

            // Split path into spanned blocks.
            sky_path *target_path = NULL;
            for(j=0; j<event_count; j++) {
                sky_event *event = events[j];
                uint32_t event_packed_length = sky_event_sizeof(event);
                
                // Create new target path if adding event will make path exceed block size.
                if(target_path != NULL) {
                    if(sky_path_sizeof(target_path) + event_packed_length > max_block_size) {
                        rc = sky_block_add_path(target_block, target_path);
                        check(rc == 0, "Unable to add path to block[1]: %d", target_block->info->id);
                        target_path = NULL;

                        rc = create_target_block(block, &target_block, affected_blocks, affected_block_count);
                        check(rc == 0, "Unable to create target block for block split[1]");

                        // Mark new block as spanned.
                        target_block->info->spanned = true;
                    }
                }
                
                // Create a path if we don't have one available.
                if(target_path == NULL) {
                    target_path = sky_path_create(event->object_id);
                    check_mem(target_path);
                }
                
                // Add event to new target path.
                rc = sky_path_add_event(target_path, event);
                check(rc == 0, "Unable to add event to new target path");
                
                // Add target path to new block if we're at the end.
                if(j == event_count-1) {
                    rc = sky_block_add_path(target_block, target_path);
                    check(rc == 0, "Unable to add path to block[2]: %d", target_block->info->id);
                    target_path = NULL;
                }
            }
            
            // Remove path since it's now been split into smaller subpaths.
            sky_path_free(path);
            
            // Clean up event list.
            free(events);
            events = NULL;
            event_count = 0;
        }
        // Otherwise add path to the target block.
        else {
            block_packed_length = sky_block_sizeof(target_block);

            // If target block will be larger than target block size then create
            // a new block. Only do this if a path exists on the block though.
            if(target_block->path_count > 0 &&
               block_packed_length + path_packed_length > target_block_size)
            {
                rc = create_target_block(block, &target_block, affected_blocks, affected_block_count);
                check(rc == 0, "Unable to create target block for block split[2]");
            }
            
            // Add path to target block.
            rc = sky_block_add_path(target_block, path);
            check(rc == 0, "Unable to add path to block[3]: %d", target_block->info->id);
        }
    }

    return 0;

error:
    // Return an empty blockset on error.
    if(*affected_blocks) free(*affected_blocks);
    *affected_blocks = NULL;
    *affected_block_count = 0;
    
    // Clean up paths extracted from block.
    if(paths) {
        for(i=0; i<path_count; i++) {
            sky_path_free(paths[i]);
        }
        free(paths);
    }
    
    // Clean up events if an error occurred during a path split.
    if(events) {
        for(i=0; i<event_count; i++) {
            sky_event_free(events[i]);
        }
        free(events);
    }
    
    return -1;
}



//======================================
// Lifecycle
//======================================


// Creates a reference to an table.
// 
// Returns a reference to the new table if successful. Otherwise returns
// null.
sky_table *sky_table_create()
{
    sky_table *table = calloc(sizeof(sky_table), 1); check_mem(table);
    return table;
    
error:
    sky_table_free(table);
    return NULL;
}

// Removes an table reference from memory.
//
// table - The table to free.
void sky_table_free(sky_table *table)
{
    if(table) {
        bdestroy(table->name);
        table->name = NULL;
        bdestroy(table->path);
        table->path = NULL;
        sky_table_unload_action_file(table);
        free(table);
    }
}


//======================================
// Action file management
//======================================

// Initializes and opens the action file on the table.
//
// table - The table to initialize the action file for.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_load_action_file(sky_table *table)
{
    int rc;
    check(table != NULL, "Table required");
    check(table->path != NULL, "Table path required");
    
    // Unload any existing action file.
    sky_table_unload_action_file(table);
    
    // Initialize action file.
    table->action_file = sky_action_file_create();
    check_mem(table->action_file);
    table->action_file->path = bformat("%s/actions", bdata(table->path));
    check_mem(table->action_file->path);
    
    // Load data
    rc = sky_action_file_load(table->action_file);
    check(rc == 0, "Unable to load actions");

    return 0;
error:
    return -1;
}

// Initializes and opens the action file on the table.
//
// table - The table to initialize the action file for.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_unload_action_file(sky_table *table)
{
    check(table != NULL, "Table required");

    if(table->action_file) {
        sky_action_file_free(table->action_file);
        table->action_file = NULL;
    }

    return 0;
error:
    return -1;
}


//======================================
// State
//======================================

// Opens the table for reading and writing events.
// 
// table - The table to open.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_open(sky_table *table)
{
    int rc;
    check(table != NULL, "Table required");
    check(table->path != NULL, "Table path is required");
    check(!table->opened, "Table is already open");

    // Create directory if it doesn't exist.
    if(!sky_file_exists(table->path)) {
        rc = mkdir(bdata(table->path), S_IRWXU);
        check(rc == 0, "Unable to create table directory: %s", bdata(table->path));
    }

    // Load header file.
    rc = sky_table_load_header_file(table);
    check(rc == 0, "Unable to load header file");
    
    // Load action file.
    rc = sky_table_load_action_file(table);
    check(rc == 0, "Unable to load action file");
    
    // Load properties.
    check(load_properties(table) == 0, "Unable to load property data");

    // Map the data file.
    rc = map_data_file(table);
    check(rc == 0, "Unable to map data file");
    
    // Flag the table as open.
    table->opened = true;

    return 0;

error:
    sky_table_close(table);
    return -1;
}

// Closes an table.
//
// table - The table to close.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_close(sky_table *table)
{
    int rc;
    check(table != NULL, "Table required to close");

    // Unload header file.
    rc = sky_table_unload_header_file(table);
    check(rc == 0, "Unable to unload header file");

    // Unload action data.
    rc = sky_table_unload_action_file(table);
    check(rc == 0, "Unable to unload action file");

    check(unload_properties(table) == 0, "Unable to unload property data");

    // Unmap data file.
    unmap_data_file(table);

    // Update state to closed.
    table->opened = false;

    return 0;
    
error:
    return -1;
}


//======================================
// Locking
//======================================

// Creates a lock file for the table.
// 
// table - The table to lock.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_create_lock(sky_table *table)
{
    FILE *file;
    check(table != NULL, "Table required to lock");
    check(table->state == SKY_OBJECT_FILE_STATE_OPEN, "Table must be open to lock")

    // Construct path to lock.
    bstring path = bformat("%s/%s", bdata(table->path), SKY_LOCK_NAME); check_mem(path);

    // Raise error if table is already locked.
    check(!sky_file_exists(path), "Cannot obtain lock: %s", bdata(path));

    // Write pid to lock file.
    file = fopen(bdata(path), "w");
    check(file, "Failed to open lock file: %s",  bdata(path));
    check(fprintf(file, "%d", getpid()) > 0, "Error writing lock file: %s",  bdata(path));
    fclose(file);

    // Clean up.
    bdestroy(path);

    return 0;

error:
    if(file) fclose(file);
    bdestroy(path);
    return -1;
}

// Unlocks an table.
// 
// table - The table to unlock.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_remove_lock(sky_table *table)
{
    FILE *file;

    // Validate arguments.
    check(table != NULL, "Table required to unlock");
    check(table->state == SKY_OBJECT_FILE_STATE_LOCKED, "Table must be locked to unlock")

    // Construct path to lock.
    bstring path = bformat("%s/%s", bdata(table->path), SKY_LOCK_NAME); check_mem(path);

    // If file exists, check its PID and then attempt to remove it.
    if(sky_file_exists(path)) {
        // Read PID from lock.
        pid_t pid = 0;
        file = fopen(bdata(path), "r");
        check(file, "Failed to open lock file: %s",  bdata(path));
        check(fscanf(file, "%d", &pid) > 0, "Error reading lock file: %s", bdata(path));
        fclose(file);

        // Make sure we are removing a lock we created.
        check(pid == getpid(), "Cannot remove lock from another process (PID #%d): %s", pid, bdata(path));

        // Remove lock.
        check(unlink(bdata(path)) == 0, "Unable to remove lock: %s", bdata(path));
    }

    // Clean up.
    bdestroy(path);

    return 0;

error:
    if(file) fclose(file);
    bdestroy(path);
    return -1;
}
    

//======================================
// Block Management
//======================================

// Determines the number of blocks that a given block spans. This function only
// works on the starting block of a span of blocks.
//
// table - The table containing the blocks.
// block_index - The index of the block inside the table's block list.
// span_count  - A pointer to where the return value should be stored.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_get_block_span_count(sky_table *table, uint32_t block_index, uint32_t *span_count)
{
    check(table != NULL, "Table required");
    check(block_index < table->block_count, "Block index out of range");
    check(span_count != NULL, "Span count address required");
    
    // Loop until the ending block of the span is found.
    uint32_t index = block_index;
    sky_object_id_t object_id = table->infos[index]->min_object_id;
    while(true) {
        index++;

        // If we've reached the end of the blocks or if the object id no longer
        // matches the starting object id then break out of the loop.
        if(index > table->block_count-1 ||
           object_id != table->infos[index]->min_object_id)
        {
            break;
        }
    }

    // Assign count back to caller's provided address.
    *span_count = (index - block_index);
    
    return 0;

error:
    *span_count = 0;
    return -1;
}


//======================================
// Event Management
//======================================

int sky_table_add_event(sky_table *table, sky_event *event)
{
    int rc = 0;
    sky_block_info *info = NULL;
    sky_block *block = NULL;
    
    sky_block **affected_blocks = NULL;
    int affected_block_count = 0;
    
    // Verify arguments.
    check(table != NULL, "Table is required");
    check(event != NULL, "Event is required");
    check(table->state == SKY_OBJECT_FILE_STATE_LOCKED, "Table must be locked to add events");
    
    // Make a copy of the event.
    sky_event *tmp = NULL;
    rc = sky_event_copy(event, &tmp);
    check(rc == 0, "Unable to copy event before insertion");
    event = tmp;
    
    // Find the appropriate block to insert into.
    rc = find_insertion_block(table, event, &info);
    check(rc == 0, "Unable to find an insertion block");

    // Load block to memory and unpack it.
    rc = load_block(table, info, &block);
    check(rc == 0, "Unable to load block %d", info->id);
    
    // Add event to block.
    rc = sky_block_add_event(block, event);
    check(rc == 0, "Unable to add event to block %d", info->id);
    
    // Attempt to split block into multiple blocks if necessary.
    rc = split_block(block, &affected_blocks, &affected_block_count);
    check(rc == 0, "Unable to split block %d", info->id);

    // Save all affected blocks.
    int i;
    for(i=0; i<affected_block_count; i++) {
        rc = save_block(affected_blocks[i]);
        check(rc == 0, "Unable to save block %d", affected_blocks[i]->info->id);
    }

    // Re-sort blocks.
    sort_blocks(table->infos, table->block_count);

    // Save header.
    save_header(table);

    // Clean up.
    sky_block_free(block);
    
    return 0;

error:
    // TODO: Restore state of database if an error occurred.
    sky_block_free(block);
    return -1;
}



//======================================
// Property Management
//======================================

// Retrieves the id for a property with a given name.
//
// table - The table that the property belongs to.
// name        - The name of the property.
// property_id - A pointer to where the property id should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_find_or_create_property_id_by_name(sky_table *table,
                                                       bstring name,
                                                       sky_property_id_t *property_id)
{
    check(table != NULL, "Table required");
    check(table->state == SKY_OBJECT_FILE_STATE_LOCKED, "Table must be open to retrieve property");
    check(name != NULL, "Property name required");
    check(blength(name) > 0, "Property name cannot be blank");
    
    // Initialize property id to zero.
    *property_id = 0;
    
    // Loop over properties to find matching name.
    sky_table_property_count_t i;
    for(i=0; i<table->property_count; i++) {
        if(biseq(table->properties[i]->name, name) == 1) {
            *property_id = table->properties[i]->id;
            break;
        }
    }
    
    // If no property was found then create one.
    if(*property_id == 0) {
        // Create property.
        sky_property *property = malloc(sizeof(sky_property)); check_mem(property);
        property->id = table->property_count+1;
        property->name = bstrcpy(name);
        
        // Append to properties.
        table->property_count++;
        table->properties = realloc(table->properties, sizeof(sky_property*) * table->property_count);
        table->properties[table->property_count-1] = property;
        
        // Save properties file.
        save_properties(table);
        
        // Return property id to caller.
        *property_id = property->id;
    }
    
    return 0;

error:
    *property_id = 0;
    return -1;
}
