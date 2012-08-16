#include <stdlib.h>

#include "dbg.h"
#include "error.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates an error object.
eql_error *eql_error_create()
{
    eql_error *err = calloc(sizeof(eql_error), 1);
    check_mem(err);
    return err;
    
error:
    eql_error_free(err);
    return NULL;
}

// Frees an error object.
//
// err - The error object to free.
void eql_error_free(eql_error *err)
{
    if(err) {
        bdestroy(err->message);
        err->message = NULL;
        bdestroy(err->source);
        err->source = NULL;
        free(err);
    }
}


