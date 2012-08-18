#ifndef _qip_ast_metadata_h
#define _qip_ast_metadata_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct qip_ast_node;

// Represents a metadata tag in the AST.
typedef struct {
    bstring name;
    struct qip_ast_node **items;
    unsigned int item_count;
} qip_ast_metadata;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_metadata_create(bstring name, struct qip_ast_node **items,
    unsigned int item_count);

void qip_ast_metadata_free(struct qip_ast_node *node);

int qip_ast_metadata_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Metadata items
//--------------------------------------

int qip_ast_metadata_get_item_value(qip_ast_node *node, bstring key,
    bstring *value);

//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_metadata_codegen_forward_decl(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_metadata_preprocess(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_metadata_validate(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_metadata_dump(qip_ast_node *node, bstring ret);

#endif