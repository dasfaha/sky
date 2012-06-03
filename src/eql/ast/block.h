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

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_block_create(struct eql_ast_node **exprs, unsigned int expr_count,
    struct eql_ast_node **ret);

void eql_ast_block_free(struct eql_ast_node *node);


//--------------------------------------
// Expression Management
//--------------------------------------

int eql_ast_block_add_expr(struct eql_ast_node *block, struct eql_ast_node *expr);

int eql_ast_block_add_exprs(struct eql_ast_node *block,
    struct eql_ast_node **exprs, unsigned int expr_count);

#endif