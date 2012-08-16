#ifndef _eql_error_h
#define _eql_error_h

#include "bstring.h"


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

typedef struct eql_error eql_error;


//==============================================================================
//
// Typedefs
//
//==============================================================================

struct eql_error {
    bstring source;
    bstring message;
    int32_t line_no;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

eql_error *eql_error_create();

void eql_error_free(eql_error *err);


#endif
