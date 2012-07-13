#include <sys/stat.h>

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
off_t sky_get_file_size(bstring path)
{
    struct stat buffer;
    if(stat(bdata(path), &buffer) == 0) {
        return buffer.st_size;
    }
    else {
        return 0;
    }
}

