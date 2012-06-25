#ifndef _eql_ast_metadata_item_h
#define _eql_ast_metadata_item_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Represents a key/value pair in a metadata tag in the AST.
typedef struct {
    bstring key;
    bstring value;
} eql_ast_metadata_item;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_metadata_item_create(bstring key, bstring value, struct eql_ast_node **node);

void eql_ast_metadata_item_free(struct eql_ast_node *node);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_metadata_item_dump(eql_ast_node *node, bstring ret);

#endif