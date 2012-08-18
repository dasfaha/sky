#ifndef _qip_ast_metadata_item_h
#define _qip_ast_metadata_item_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct qip_ast_node;

// Represents a key/value pair in a metadata tag in the AST.
typedef struct {
    bstring key;
    bstring value;
} qip_ast_metadata_item;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_metadata_item_create(bstring key, bstring value);

void qip_ast_metadata_item_free(struct qip_ast_node *node);

int qip_ast_metadata_item_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_metadata_item_preprocess(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_metadata_item_validate(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_metadata_item_dump(qip_ast_node *node, bstring ret);

#endif