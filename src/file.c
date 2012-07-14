#include <sys/stat.h>

#include "dbg.h"
#include "file.h"

//==============================================================================
//
// Functions
//
//==============================================================================

// Checks if a file exists.
//
// path - The path of the file.
//
// Returns true if the file at the given path exists. Otherwise returns false.
bool sky_file_exists(bstring path)
{
    struct stat buffer;
    int rc = stat(bdata(path), &buffer);
    return (rc == 0);
}

// Retrieves the size of a file, in bytes.
//
// path - The path of the file.
//
// Returns the size of the file at the given path in bytes.
off_t sky_file_get_size(bstring path)
{
    struct stat buffer;
    if(stat(bdata(path), &buffer) == 0) {
        return buffer.st_size;
    }
    else {
        return 0;
    }
}

// Copies a single file from the source path to the destination path.
//
// src  - The path of the file to copy.
// dest - The path where the copy should be placed.
//
// Returns 0 if successful, otherwise returns -1.
int sky_file_cp(bstring src, bstring dest)
{
    check(src != NULL, "Source path required");
    check(dest != NULL, "Destination path required");
    check(sky_file_exists(src), "Source file does not exist");

    // Open source files.
    FILE *src_file = fopen(bdata(src), "r");
    check(src_file != NULL, "Unable to open source file for reading");

    // Open destination file.
    FILE *dest_file = fopen(bdata(dest), "w");
    check(dest_file != NULL, "Unable to open destination file for writing");
    
    // Read from source and write to destination until done.
    while(!feof(src_file)) {
        int buffer_size = 1024;
        char *buffer[buffer_size];
        
        size_t sz = fread(buffer, sizeof(char), buffer_size, src_file);
        fwrite(buffer, sizeof(char), sz, dest_file);
    }
    
    // Close files.
    fclose(src_file);
    fclose(dest_file);
    
    return 0;

error:
    return -1;
}

