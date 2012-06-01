#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Node Lifecycle
//--------------------------------------

// Recursively frees an AST node.
//
// node - The node to free.
void eql_ast_node_free(eql_ast_node *node)
{
    unsigned int i;
    if(!node) return;
    
    // Recursively free dependent data.
    switch(node->type) {
        case EQL_AST_TYPE_INT_LITERAL: break;
        case EQL_AST_TYPE_FLOAT_LITERAL: break;
        case EQL_AST_TYPE_BINARY_EXPR: {
            if(node->binary_expr.lhs) eql_ast_node_free(node->binary_expr.lhs);
            if(node->binary_expr.rhs) eql_ast_node_free(node->binary_expr.rhs);
            break;
        }
    }
    
    free(node);
}

