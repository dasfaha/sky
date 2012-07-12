#include <stdlib.h>
#include <inttypes.h>

#include "dbg.h"
#include "endian.h"
#include "mem.h"
#include "bstring.h"
#include "file.h"
#include "header_file.h"


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int compare_block_info(const void *_a, const void *_b);
int compare_block_info_by_id(const void *_a, const void *_b);

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
sky_header_file *sky_header_file_create()
{
    sky_header_file *header_file = calloc(sizeof(sky_header_file), 1);
    check_mem(header_file);
    return header_file;
    
error:
    sky_header_file_free(header_file);
    return NULL;
}

// Removes a header file reference from memory.
//
// header_file - The header file to free.
void sky_header_file_free(sky_header_file *header_file)
{
    if(header_file) {
        if(header_file->path) bdestroy(header_file->path);
        header_file->path = NULL;
        sky_header_file_unload(header_file);
        free(header_file);
    }
}


//======================================
// Path
//======================================

// Sets the file path of a header file.
//
// header_file - The header file.
// path        - The file path to set.
//
// Returns 0 if successful, otherwise returns -1.
int sky_header_file_set_path(sky_header_file *header_file, bstring path)
{
    check(header_file != NULL, "Header file required");

    if(header_file->path) {
        bdestroy(header_file->path);
    }
    
    header_file->path = bstrcpy(path);
    if(path) check_mem(header_file->path);

    return 0;

error:
    header_file->path = NULL;
    return -1;
}


//======================================
// Persistence
//======================================

// Loads header information.
//
// header_file - The header file to load.
//
// Returns 0 if successful, otherwise returns -1.
int sky_header_file_load(sky_header_file *header_file)
{
    int rc;
    FILE *file;
    sky_block_info **infos = NULL;
    uint32_t version = 1;
    uint32_t block_count = 0;
    uint8_t buffer[SKY_BLOCK_INFO_SIZE];

    // Read in header file if it exists.
    if(sky_file_exists(header_file->path)) {
        file = fopen(bdata(header_file->path), "r");
        check(file, "Failed to open header file for reading: %s",  bdata(header_file->path));

        // Read database format version & block size.
        check(fread(&version, sizeof(version), 1, file) == 1, "Unable to read version");
        version = ntohl(version);
        check(fread(&header_file->block_size, sizeof(header_file->block_size), 1, file) == 1, "Unable to read block size");
        header_file->block_size = ntohl(header_file->block_size);

        // Read block count.
        check(fread(&block_count, sizeof(block_count), 1, file) == 1, "Unable to read block count");
        block_count = ntohl(block_count);
        infos = malloc(sizeof(sky_block_info*) * block_count);
        if(block_count > 0) check_mem(infos);

        // Read block info items until end of file.
        uint32_t i;
        for(i=0; i<block_count && !feof(file); i++) {
            // Allocate info.
            sky_block_info *info = sky_block_info_create();
            check_mem(info);

            // Set index.
            info->index = i;
            info->spanned = false;

            // Read into buffer.
            rc = fread(buffer, SKY_BLOCK_INFO_SIZE, 1, file);
            check(rc == 1, "Unable to read block info #%d", info->index);
            
            // Unpack from buffer.
            size_t sz;
            rc = sky_block_info_unpack(info, buffer, &sz);
            check(rc == 0, "Unable to unpack block info #%d", info->index);
            
            infos[i] = info;
        }

        // Close the file.
        fclose(file);

        // Sort ranges by starting object id.
        qsort(infos, block_count, sizeof(sky_block_info*), compare_block_info);

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

    // Store version and block information.
    header_file->version = version;
    header_file->block_count = block_count;
    header_file->infos = infos;

    return 0;

error:
    if(file) fclose(file);
    return -1;
}

// Saves block infos to file.
//
// header_file - The header file to save.
//
// Returns 0 if successful, otherwise returns -1.
int sky_header_file_save(sky_header_file *header_file)
{
    int rc;
    uint8_t buffer[SKY_BLOCK_INFO_SIZE];
    
    check(header_file != NULL, "Header file required");
    check(header_file->path != NULL, "Header file path required");

    // Copy infos to a new array to re-sort.
    sky_block_info **infos = malloc(sizeof(sky_block_info*) * header_file->block_count); check_mem(infos);
    memcpy(infos, header_file->infos, sizeof(sky_block_info*) * header_file->block_count);
    qsort(infos, header_file->block_count, sizeof(sky_block_info*), compare_block_info_by_id);

    // Open the header file.
    FILE *file = fopen(bdata(header_file->path), "w");
    check(file, "Failed to open header file for writing: %s", bdata(header_file->path));

    // Write database format version.
    uint32_t version = htonl(header_file->version);
    rc = fwrite(&version, sizeof(version), 1, file);
    check(rc == 1, "Unable to write version");

    // Write database block size.
    uint32_t block_size = htonl(header_file->block_size);
    rc = fwrite(&block_size, sizeof(block_size), 1, file);
    check(rc == 1, "Unable to write block size");

    // Write block count.
    uint32_t block_count = htonl(block_count);
    rc = fwrite(&block_count, sizeof(block_count), 1, file);
    check(rc == 1, "Unable to write block count");

    // Read block info items until end of file.
    uint32_t i;
    for(i=0; i<block_count; i++) {
        sky_block_info *info = infos[i];
            
        // Pack into buffer.
        size_t sz;
        rc = sky_block_info_pack(info, buffer, &sz);
        check(rc == 0, "Unable to pack block info #%d", info->index);

        // Write buffer.
        rc = fwrite(buffer, SKY_BLOCK_INFO_SIZE, 1, file);
        check(rc == 1, "Unable to write block info #%d", info->index);
    }

    // Close the file.
    fclose(file);

    // Clean up.
    free(infos);

    return 0;

error:
    if(file) fclose(file);
    if(infos) free(infos);
    return -1;
}

// Unloads the block infos in the header file from memory.
//
// header_file - The header file to save.
//
// Returns 0 if successful, otherwise returns -1.
int sky_header_file_unload(sky_header_file *header_file)
{
    if(header_file) {
        // Release block info.
        if(header_file->infos) {
            uint32_t i=0;
            for(i=0; i<header_file->block_count; i++) {
                sky_block_info *info = header_file->infos[i];
                sky_block_info_free(info);
                header_file->infos[i] = NULL;
            }
            free(header_file->infos);
            header_file->infos = NULL;
        }
        
        header_file->version = 0;
        header_file->block_size = SKY_DEFAULT_BLOCK_SIZE;
        header_file->block_count = 0;
    }
    
    return 0;
}



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

// Compares two blocks and sorts them by id. This is used when serializing block
// info to the header file.
int compare_block_info_by_id(const void *_a, const void *_b)
{
    sky_block_info **a = (sky_block_info **)_a;
    sky_block_info **b = (sky_block_info **)_b;

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

