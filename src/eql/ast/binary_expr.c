#include <stdlib.h>
#include "../../dbg.h"

#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a binary expression.
//
// operator - The operator used in the expression.
// lhs      - The node on the left-hand side.
// rhs      - The node on the right-hand side.
// ret      - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_binary_expr_create(eql_ast_binop_e operator,
                               eql_ast_node *lhs,
                               eql_ast_node *rhs,
                               eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_BINARY_EXPR;
    node->binary_expr.operator = operator;
    node->binary_expr.lhs = lhs;
    node->binary_expr.rhs = rhs;
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a binary expression AST node from memory.
//
// node - The AST node to free.
void eql_ast_binary_expr_free(eql_ast_node *node)
{
    if(node->binary_expr.lhs) {
        eql_ast_node_free(node->binary_expr.lhs);
    }
    node->binary_expr.lhs = NULL;

    if(node->binary_expr.rhs) {
        eql_ast_node_free(node->binary_expr.rhs);
    }
    node->binary_expr.rhs = NULL;
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node - The AST node to determine the type for.
// type - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_binary_expr_get_type(eql_ast_node *node, bstring *type)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_BINARY_EXPR, "Node type must be 'binary expr'");
    check(node->binary_expr.lhs != NULL, "Binary expression LHS is required");
    
    // Return the type of the left side.
    int rc = eql_ast_node_get_type(node->binary_expr.lhs, type);
    check(rc == 0, "Unable to determine the binary expression type");
    
    return 0;

error:
    *type = NULL;
    return -1;
}

