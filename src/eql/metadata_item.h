#ifndef _eql_ast_metadata_item_h
#define _eql_ast_metadata_item_h

#include "bstring.h"

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

eql_ast_node *eql_ast_metadata_item_create(bstring key, bstring value);

void eql_ast_metadata_item_free(struct eql_ast_node *node);

int eql_ast_metadata_item_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_metadata_item_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_metadata_item_validate(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_metadata_item_dump(eql_ast_node *node, bstring ret);

#endif