#include <stdlib.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <math.h>

#include "dbg.h"
#include "mem.h"
#include "bstring.h"
#include "file.h"
#include "database.h"
#include "block.h"
#include "table.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Block Sorting
//======================================

// Compares two blocks and sorts them based on starting min object identifier
// and then by id.
int compare_block_info(const void *_a, const void *_b)
{
    sky_block_info **a = (sky_block_info **)_a;
    sky_block_info **b = (sky_block_info **)_b;

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
            if((*a)->id > (*b)->id) {
                return 1;
            }
            else if((*a)->id < (*b)->id) {
                return -1;
            }
            else {
                return 0;
            }
        }
    }
}

// Compares two blocks and sorts them by id. This is used when serializing block
// info to the header file.
int compare_block_info_by_id(const void *_a, const void *_b)
{
    sky_block_info **a = (sky_block_info **)_a;
    sky_block_info **b = (sky_block_info **)_b;

    if((*a)->id > (*b)->id) {
        return 1;
    }
    else if((*a)->id < (*b)->id) {
        return -1;
    }
    else {
        return 0;
    }
}

// Sorts blocks by object id and block id.
//
// infos - The array of block info objects.
// count - The number of blocks.
void sort_blocks(sky_block_info **infos, uint32_t count)
{
    qsort(infos, count, sizeof(sky_block_info*), compare_block_info);
}



//======================================
// Header Management
//======================================

// Retrieves the file path of an table's header file.
//
// table - The table who owns the header file.
bstring get_header_file_path(sky_table *table)
{
    return bformat("%s/header", bdata(table->path)); 
}

// Saves header information to file.
//
// table - The table containing the header information.
//
// Returns 0 if successful, otherwise returns -1.
int save_header(sky_table *table)
{
    int rc;

    // Copy infos to a new array to re-sort.
    uint32_t block_count = table->block_count;
    sky_block_info **infos = malloc(sizeof(sky_block_info*) * block_count); check_mem(infos);
    memcpy(infos, table->infos, sizeof(sky_block_info*) * block_count);
    qsort(infos, block_count, sizeof(sky_block_info*), compare_block_info_by_id);

    // Open the header file.
    bstring path = get_header_file_path(table); check_mem(path);
    FILE *file = fopen(bdata(path), "w");
    check(file, "Failed to open header file for writing: %s",  bdata(path));
    
    // Write database format version & block size.
    rc = fwrite(&table->version, sizeof(table->version), 1, file);
    check(rc == 1, "Unable to write version");
    rc = fwrite(&table->block_size, sizeof(table->block_size), 1, file);
    check(rc == 1, "Unable to write block size");

    // Write block count.
    rc = fwrite(&block_count, sizeof(block_count), 1, file);
    check(rc == 1, "Unable to write block count");

    // Read block info items until end of file.
    uint32_t i;
    for(i=0; i<block_count; i++) {
        sky_block_info *info = infos[i];

        // Write object id range.
        rc = fwrite(&info->min_object_id, sizeof(info->min_object_id), 1, file);
        check(rc == 1, "Unable to write min object id : blk%d", i);
        rc = fwrite(&info->max_object_id, sizeof(info->max_object_id), 1, file);
        check(rc == 1, "Unable to read max object id : blk%d", i);
        
        // Read timestamp range.
        rc = fwrite(&info->min_timestamp, sizeof(info->min_timestamp), 1, file);
        check(rc == 1, "Unable to read min timestamp : blk%d", i);
        rc = fwrite(&info->max_timestamp, sizeof(info->max_timestamp), 1, file);
        check(rc == 1, "Unable to read max timestamp : blk%d", i);
    }

    // Close the file.
    fclose(file);

    // Clean up.
    bdestroy(path);
    free(infos);

    return 0;
    
error:
    bdestroy(path);
    if(file) fclose(file);
    if(infos) free(infos);
    return -1;
}

// Loads header information from file.
//
// table - The table where the header is stored.
//
// Returns 0 if successful, otherwise returns -1.
int load_header(sky_table *table)
{
    FILE *file;
    sky_block_info **infos = NULL;
    uint32_t version = 1;
    uint32_t block_count = 0;

    // Retrieve file stats on header file
    bstring path = get_header_file_path(table); check_mem(path);
    
    // Read in header file if it exists.
    if(sky_file_exists(path)) {
        file = fopen(bdata(path), "r");
        check(file, "Failed to open header file for reading: %s",  bdata(path));

        // Read database format version & block size.
        check(fread(&version, sizeof(version), 1, file) == 1, "Unable to read version");
        check(fread(&table->block_size, sizeof(table->block_size), 1, file) == 1, "Unable to read block size");

        // Read block count.
        check(fread(&block_count, sizeof(block_count), 1, file) == 1, "Unable to read block count");
        infos = malloc(sizeof(sky_block_info*) * block_count);
        if(block_count > 0) check_mem(infos);

        // Read block info items until end of file.
        uint32_t i;
        for(i=0; i<block_count && !feof(file); i++) {
            // Allocate info.
            sky_block_info *info = malloc(sizeof(sky_block_info));
            
            // Set index.
            info->id = i;
            info->spanned = false;
            
            // Read object id range.
            check(fread(&info->min_object_id, sizeof(info->min_object_id), 1, file) == 1, "Unable to read min object id : block #%d", i);
            check(fread(&info->max_object_id, sizeof(info->max_object_id), 1, file) == 1, "Unable to read max object id : block #%d", i);
            
            // Read timestamp range.
            check(fread(&info->min_timestamp, sizeof(info->min_timestamp), 1, file) == 1, "Unable to read min timestamp : block #%d", i);
            check(fread(&info->max_timestamp, sizeof(info->max_timestamp), 1, file) == 1, "Unable to read max timestamp : block #%d", i);
            
            infos[i] = info;
        }

        // Close the file.
        fclose(file);

        // Sort ranges by starting object id.
        sort_blocks(infos, block_count);
        
        // Determine spanned blocks.
        sky_object_id_t last_object_id = -1;
        for(i=0; i<block_count; i++) {
            // If this is a single object block then track the object id to
            // possibly mark it as spanned.
            if(infos[i]->min_object_id == infos[i]->max_object_id && infos[i]->min_object_id > 0) {
                // If it has spanned since the last block then mark it and the
                // previous block.
                if(infos[i]->min_object_id == last_object_id) {
                    infos[i]->spanned = true;
                    infos[i-1]->spanned = true;
                }
                // If this is the first block with one object then store the id.
                else {
                    last_object_id = infos[i]->min_object_id;
                }
            }
            // Clear out last object id for multi-object blocks.
            else {
                last_object_id = -1;
            }
        }
    }

    // Store version and block information on table.
    table->version = version;
    table->block_count = block_count;
    table->infos = infos;

    // Clean up.
    bdestroy(path);

    return 0;

error:
    if(file) fclose(file);
    bdestroy(path);
    return -1;
}

// Unloads the header data.
//
// table - The table where the header data is stored.
//
// Returns 0 if successful, otherwise returns -1.
int unload_header(sky_table *table)
{
    if(table) {
        // Free block infos.
        uint32_t i;
        if(table->infos) {
            for(i=0; i<table->block_count; i++) {
                free(table->infos[i]);
                table->infos[i] = NULL;
            }

            free(table->infos);
            table->infos = NULL;
        }

        table->version = 0;
        table->block_size = SKY_DEFAULT_BLOCK_SIZE;
        table->block_count = 0;
    }
    
    return 0;
}




//======================================
// Property Management
//======================================

// Retrieves the file path of an table's actions file.
//
// table - The table who owns the header file.
bstring get_properties_file_path(sky_table *table)
{
    return bformat("%s/properties", bdata(table->path)); 
}


// Saves property information to file.
//
// table - The table where the property information is stored.
//
// Returns 0 if successful, otherwise returns -1.
int save_properties(sky_table *table)
{
    int rc;

    // Open the properties file.
    bstring path = get_properties_file_path(table); check_mem(path);
    FILE *file = fopen(bdata(path), "w");
    check(file, "Failed to open properties file for writing: %s", bdata(path));

    // Write property count.
    rc = fwrite(&table->property_count, sizeof(table->property_count), 1, file);
    check(rc == 1, "Unable to write property count");

    // Write properties to file.
    sky_table_property_count_t i;
    for(i=0; i<table->property_count; i++) {
        sky_property *property = table->properties[i];
        
        // Write property id.
        rc = fwrite(&property->id, sizeof(sky_property_id_t), 1, file);
        check(rc == 1, "Unable to write property id");

        // Write property name length.
        uint16_t property_name_length = blength(property->name);
        rc = fwrite(&property_name_length, sizeof(property_name_length), 1, file);
        check(rc == 1, "Unable to write property name length");

        // Write property name.
        char *name = bdata(property->name);
        rc = fwrite(name, property_name_length, 1, file);
        check(rc == 1, "Unable to write property name");
    }

    // Close the file.
    fclose(file);

    // Clean up.
    bdestroy(path);

    return 0;

error:
    bdestroy(path);
    if(file) fclose(file);
    return -1;
}

// Loads property information from file.
//
// table - The table from which to load property definitions.
//
// Returns 0 if sucessfuly, otherwise returns -1.
int load_properties(sky_table *table)
{
    FILE *file;
    sky_property **properties = NULL;
    char *buffer;
    sky_table_property_count_t count = 0;
    
    // Retrieve file stats on properties file
    bstring path = get_properties_file_path(table); check_mem(path);
    
    // Read in properties file if it exists.
    if(sky_file_exists(path)) {
        file = fopen(bdata(path), "r");
        check(file, "Failed to open properties file: %s",  bdata(path));
        
        // Read properties count.
        fread(&count, sizeof(count), 1, file);
        properties = malloc(sizeof(sky_property*) * count);
        
        // Read properties until end of file.
        uint32_t i;
        uint16_t length;
        for(i=0; i<count && !feof(file); i++) {
            sky_property *property = malloc(sizeof(sky_property)); check_mem(property);
            
            // Read property id and name length.
            check(fread(&property->id, sizeof(int16_t), 1, file) == 1, "Corrupt properties file");
            check(fread(&length, sizeof(length), 1, file) == 1, "Corrupt properties file");

            // Read property name.
            buffer = calloc(1, length+1); check_mem(buffer);
            check(fread(buffer, length, 1, file) == 1, "Corrupt properties file");
            property->name = bfromcstr(buffer); check_mem(property->name);
            free(buffer);
            
            // Append property to array.
            properties[i] = property;
        }
        
        // Close the file.
        fclose(file);
    }

    // Store property list on table.
    table->properties = properties;
    table->property_count = count;
    
    // Clean up.
    bdestroy(path);
    
    return 0;

error:
    if(file) fclose(file);
    if(buffer) free(buffer);
    bdestroy(path);
    return -1;
}

// Unloads the property data.
//
// table - The table where the property data is stored.
//
// Returns 0 if successful, otherwise returns -1.
int unload_properties(sky_table *table)
{
    if(table) {
        // Release properties.
        if(table->properties) {
            uint32_t i=0;
            for(i=0; i<table->property_count; i++) {
                bdestroy(table->properties[i]->name);
                free(table->properties[i]);
                table->properties[i] = NULL;
            }
            free(table->properties);
            table->properties = NULL;
        }
        
        table->property_count = 0;
    }
    
    return 0;
}


//======================================
// Block Management
//======================================

// Retrieves the file path of an table's data file.
//
// table - The table who owns the data file.
bstring get_data_file_path(sky_table *table)
{
    return bformat("%s/data", bdata(table->path)); 
}


// Calculates the byte offset in a data file for a block. This is simply the
// block index multiplied by the block size.
//
// table - The table that contains the block.
// info        - The header info about the block.
//
// Returns the number of bytes from the start of the data file where the block
// begins.
ptrdiff_t get_block_offset(sky_table *table, sky_block_info *info)
{
    return (table->block_size * info->id);
}

// Unmaps a data file for an object.
//
// table - The table that owns the data file.
//
// Returns 0 if successful, otherwise returns -1.
int unmap_data_file(sky_table *table)
{
    // Unmap file.
    if(table->data != NULL) {
        munmap(table->data, table->data_length);
        table->data = NULL;
    }
    
    // Close file descriptor.
    if(table->data_fd != 0) {
        close(table->data_fd);
        table->data_fd = 0;
    }
    
    table->data_length = 0;
    
    return 0;
}

// Maps the data file for an table. If the data file is already mapped
// then it is remapped.
//
// table - The table that owns the data file.
//
// Returns 0 if successful, otherwise returns -1.
int map_data_file(sky_table *table)
{
    int rc;
    void *ptr;
    
    // Calculate the data length.
    size_t data_length;
    if(table->block_count == 0) {
        data_length = table->block_size;
    }
    else {
        data_length = table->block_count * table->block_size;
    }
    
    // Close mapping if remapping isn't supported.
    if(!MREMAP_AVAILABLE) {
        unmap_data_file(table);
    }
    
    // Find the path to the data file.
    bstring path = get_data_file_path(table); check_mem(path);

    // Open the data file and map it if it is not currently open.
    if(table->data_fd == 0) {
        table->data_fd = open(bdata(path), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        check(table->data_fd != -1, "Failed to open data file descriptor: %s",  bdata(path));

        // Truncate the file to the appropriate size (larger or smaller).
        rc = ftruncate(table->data_fd, data_length);
        check(rc == 0, "Unable to truncate data file");
        
        // Memory map the data file.
        ptr = mmap(0, data_length, PROT_READ | PROT_WRITE, MAP_SHARED, table->data_fd, 0);
        check(ptr != MAP_FAILED, "Unable to memory map data file");
    }
    // If we already have the data mapped then simply remap it to the
    // appropriate size.
    else {
#if MREMAP_AVAILABLE
        ptr = mremap(table->data, table->data_length, data_length, MREMAP_MAYMOVE);
        check(ptr != MAP_FAILED, "Unable to remap data file");
#endif
    }

    // Update the table.
    table->data = ptr;
    table->data_length = data_length;

    bdestroy(path);
    
    return 0;

error:
    table->data = NULL;
    bdestroy(path);
    return -1;
}

// Serializes a block in memory and writes it to disk.
// 
// block       - The in-memory block to write to disk.
//
// Returns 0 if successful. Otherwise returns -1.
int save_block(sky_block *block)
{
    int rc;

    sky_table *table = block->table;
    sky_block_info *info = block->info;

    // Move pointer to starting position of block.
    ptrdiff_t offset = get_block_offset(table, info);
    void *addr = table->data + offset;

    // Serialize block.
    ptrdiff_t ptrdiff;
    rc = sky_block_serialize(block, addr, &ptrdiff);
    check(rc == 0, "Failed to serialize block: %d", info->id);

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

// Reads a block in from disk and deserializes it.
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
    ptrdiff_t offset = get_block_offset(table, info);
    void *addr = table->data + offset;

    // Deserialize block.
    ptrdiff_t ptrdiff;
    sky_block *block = sky_block_create(table, info);
    rc = sky_block_deserialize(block, addr, &ptrdiff);
    check(rc == 0, "Failed to deserialize block: %d", info->id);
    
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
    uint32_t block_serialized_length = sky_block_get_serialized_length(block);
    if(block_serialized_length <= block->table->block_size) {
        return 0;
    }
    
    // Extract paths and info from original block.
    bool spanned = block->info->spanned;
    sky_path **paths = block->paths;
    sky_path_count_t path_count = block->path_count;
    block->paths = NULL;
    block->path_count = 0;
    
    // Assign original block as first target block.
    target_block = block;
    
    // Calculate target block size if we were to spread paths evenly across blocks.
    uint32_t max_block_size = block->table->block_size - BLOCK_HEADER_LENGTH;
    uint32_t target_block_count = (uint32_t)ceil((double)block_serialized_length / (double)max_block_size);
    uint32_t target_block_size = block_serialized_length / target_block_count;
    
    // Loop over paths and spread them across blocks.
    uint32_t i, j;
    for(i=0; i<path_count; i++) {
        sky_path *path = paths[i];
        uint32_t path_serialized_length = sky_path_get_serialized_length(path);
        
        // If path is already spanned or the path is larger than max block size
        // then spread its events across multiple blocks.
        if(spanned || path_serialized_length > max_block_size) {
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
                uint32_t event_serialized_length = sky_event_get_serialized_length(event);
                
                // Create new target path if adding event will make path exceed block size.
                if(target_path != NULL) {
                    if(sky_path_get_serialized_length(target_path) + event_serialized_length > max_block_size) {
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
            block_serialized_length = sky_block_get_serialized_length(target_block);

            // If target block will be larger than target block size then create
            // a new block. Only do this if a path exists on the block though.
            if(target_block->path_count > 0 &&
               block_serialized_length + path_serialized_length > target_block_size)
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
// database - A reference to the database that the table belongs to.
// name - The name of the table.
//
// Returns a reference to the new table if successful. Otherwise returns
// null.
sky_table *sky_table_create(sky_database *database, bstring name)
{
    sky_table *table;
    
    check(database != NULL, "Cannot create table without a database");
    check(name != NULL, "Cannot create unnamed table");
    
    table = malloc(sizeof(sky_table)); check_mem(table);
    table->state = SKY_OBJECT_FILE_STATE_CLOSED;
    table->name = bstrcpy(name); check_mem(table->name);
    table->path = bformat("%s/%s", bdata(database->path), bdata(table->name));
    check_mem(table->path);

    table->block_size = SKY_DEFAULT_BLOCK_SIZE;  // Default to 64K blocks.
    table->infos = NULL;
    table->block_count = 0;

    table->action_file = sky_action_file_create(table);
    check_mem(table->action_file);

    table->properties = NULL;
    table->property_count = 0;

    table->data = NULL;
    table->data_length = 0;

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
        bdestroy(table->path);
        sky_action_file_free(table->action_file);
        table->action_file = NULL;
        free(table);
    }
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
    
    // Validate arguments.
    check(table != NULL, "Table required to open");
    check(table->state == SKY_OBJECT_FILE_STATE_CLOSED, "Table must be closed to open")

    // Create directory if it doesn't exist.
    if(!sky_file_exists(table->path)) {
        rc = mkdir(bdata(table->path), S_IRWXU);
        check(rc == 0, "Unable to create table directory: %s", bdata(table->path));
    }

    // Load header.
    check(load_header(table) == 0, "Unable to load header data");
    
    // Load action data.
    rc = sky_action_file_load(table->action_file);
    check(rc == 0, "Unable to load actions");
    
    // Load properties.
    check(load_properties(table) == 0, "Unable to load property data");

    // Map the data file.
    rc = map_data_file(table);
    check(rc == 0, "Unable to map data file");
    
    // Flag the table as locked.
    table->state = SKY_OBJECT_FILE_STATE_OPEN;

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
    // Validate arguments.
    check(table != NULL, "Table required to close");

    // Unload header, action and properties data.
    check(unload_header(table) == 0, "Unable to unload header data");

    // Unload action data.
    sky_action_file_unload(table->action_file);

    check(unload_properties(table) == 0, "Unable to unload property data");

    // Unmap data file.
    unmap_data_file(table);

    // Update state.
    table->state = SKY_OBJECT_FILE_STATE_CLOSED;

    return 0;
    
error:
    return -1;
}


//======================================
// Locking
//======================================


// Locks an table for writing.
// 
// table - The table to lock.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_lock(sky_table *table)
{
    FILE *file;

    // Validate arguments.
    check(table != NULL, "Table required to lock");
    check(table->state == SKY_OBJECT_FILE_STATE_OPEN, "Table must be open to lock")

    // Construct path to lock.
    bstring path = bformat("%s/%s", bdata(table->path), SKY_OBJECT_FILE_LOCK_NAME); check_mem(path);

    // Raise error if table is already locked.
    check(!sky_file_exists(path), "Cannot obtain lock: %s", bdata(path));

    // Write pid to lock file.
    file = fopen(bdata(path), "w");
    check(file, "Failed to open lock file: %s",  bdata(path));
    check(fprintf(file, "%d", getpid()) > 0, "Error writing lock file: %s",  bdata(path));
    fclose(file);

    // Flag the table as locked.
    table->state = SKY_OBJECT_FILE_STATE_LOCKED;

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
int sky_table_unlock(sky_table *table)
{
    FILE *file;
    pid_t pid = 0;

    // Validate arguments.
    check(table != NULL, "Table required to unlock");
    check(table->state == SKY_OBJECT_FILE_STATE_LOCKED, "Table must be locked to unlock")

    // Construct path to lock.
    bstring path = bformat("%s/%s", bdata(table->path), SKY_OBJECT_FILE_LOCK_NAME); check_mem(path);

    // If file exists, check its PID and then attempt to remove it.
    if(sky_file_exists(path)) {
        // Read PID from lock.
        file = fopen(bdata(path), "r");
        check(file, "Failed to open lock file: %s",  bdata(path));
        check(fscanf(file, "%d", &pid) > 0, "Error reading lock file: %s", bdata(path));
        fclose(file);

        // Make sure we are removing a lock we created.
        check(pid == getpid(), "Cannot remove lock from another process (PID #%d): %s", pid, bdata(path));

        // Remove lock.
        check(unlink(bdata(path)) == 0, "Unable to remove lock: %s", bdata(path));
    }

    // Flag the table as open.
    table->state = SKY_OBJECT_FILE_STATE_OPEN;

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

    // Load block to memory and deserialize.
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
