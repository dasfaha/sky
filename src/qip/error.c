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
qip_error *qip_error_create()
{
    qip_error *err = calloc(sizeof(qip_error), 1);
    check_mem(err);
    return err;
    
error:
    qip_error_free(err);
    return NULL;
}

// Frees an error object.
//
// err - The error object to free.
void qip_error_free(qip_error *err)
{
    if(err) {
        bdestroy(err->message);
        err->message = NULL;
        bdestroy(err->source);
        err->source = NULL;
        free(err);
    }
}


