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
