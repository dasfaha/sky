#ifndef _eql_ast_block_h
#define _eql_ast_block_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Represents a block in the AST.
typedef struct {
    struct eql_ast_node **exprs;
    unsigned int expr_count;
} eql_ast_block;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_block_create(struct eql_ast_node **exprs, unsigned int expr_count,
    struct eql_ast_node **ret);

void eql_ast_block_free(struct eql_ast_node *node);

#endif