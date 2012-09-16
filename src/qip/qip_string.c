#include <stdlib.h>

#include "qip_string.h"
#include "dbg.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a fixed-length string.
//
// length - The number of characters in the string.
// data   - A pointer to the character data.
//
// Returns a fixed-length string
qip_string qip_string_create(int64_t length, char *data)
{
    qip_string string;
    string.length = length;
    string.data = data;
    return string;
}


//======================================
// Equality
//======================================

// Evaluates whether the string length and string contents are equal between
// two strings.
//
// a - The first string.
// b - The second string.
//
// Returns true if the strings are equal. Otherwise returns false.
bool qip_string_equals(qip_module *module, qip_string a, qip_string b)
{
    check(module != NULL, "Module required");
    
    // Check length first. We could have two strings pointing at the same data
    // but one string only points to a subset of the data.
    if(a.length == b.length) {
        // If both strings point at the same data it must be the same string.
        if(a.data == b.data) {
            return true;
        }
        // If these are different data pointers then check the contents of
        // the data pointers.
        else {
            return (memcmp(a.data, b.data, a.length)) == 0;
        }
    }
    
    return false;

error:
    return false;
}
