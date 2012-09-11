#include <stdlib.h>

#include "qip_string.h"
#include "dbg.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates a fixed-length string.
//
// length - The number of characters in the string.
// data   - A pointer to the character data.
//
// Returns a fixed-length string
qip_string *qip_string_create(int64_t length, char *data)
{
    qip_string *string = qip_string_alloc();
    check_mem(string);
    qip_string_init(string, length, data);
    return string;
    
error:
    qip_string_free(string);
    return NULL;
}

// Allocates a fixed-length string in memory.
// 
// Returns a fixed-length string.
qip_string *qip_string_alloc()
{
    return calloc(1, sizeof(qip_string));
}

// Initializes a fixed-length string.
//
// string - The string.
// length - The number of characters in the string.
// data   - A pointer to the character data.
//
// Returns nothing.
void qip_string_init(qip_string *string, int64_t length, char *data)
{
    string->length = length;
    string->data = data;
}

// Frees a fixed string.
//
// string - The string to free.
void qip_string_free(qip_string *string)
{
    if(string) {
        string->length = 0;
        string->data = NULL;
        free(string);
    }
}


//--------------------------------------
// Equality
//--------------------------------------

// Evaluates whether the string length and string contents are equal between
// two strings.
//
// a - The first string.
// b - The second string.
//
// Returns true if the strings are equal. Otherwise returns false.
bool qip_string_equals(qip_string *a, qip_string *b)
{
    if(a == b) {
        return true;
    }
    else if(a != NULL && b != NULL) {
        if(a->length == b->length) {
            return (memcmp(a->data, b->data, a->length)) == 0;
        }
    }
    
    return false;
}
