#ifndef _eql_ast_metadata_h
#define _eql_ast_metadata_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Represents a metadata tag in the AST.
typedef struct {
    bstring name;
    struct eql_ast_node **items;
    unsigned int item_count;
} eql_ast_metadata;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_metadata_create(bstring name, struct eql_ast_node **items,
    unsigned int item_count);

void eql_ast_metadata_free(struct eql_ast_node *node);

int eql_ast_metadata_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Metadata items
//--------------------------------------

int eql_ast_metadata_get_item_value(eql_ast_node *node, bstring key,
    bstring *value);

//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_metadata_codegen_forward_decl(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_metadata_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_metadata_validate(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_metadata_dump(eql_ast_node *node, bstring ret);

#endif