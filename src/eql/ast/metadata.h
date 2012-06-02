#ifndef _eql_ast_metadata_h
#define _eql_ast_metadata_h

#include "../../bstring.h"

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

int eql_ast_metadata_create(bstring name, struct eql_ast_node **items,
    unsigned int item_count, struct eql_ast_node **ret);

void eql_ast_metadata_free(struct eql_ast_node *node);

#endif