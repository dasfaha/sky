#ifndef _eql_ast_binary_expr_h
#define _eql_ast_binary_expr_h


//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Defines the types of binary expressions.
typedef enum {
    EQL_BINOP_PLUS,
    EQL_BINOP_MINUS,
    EQL_BINOP_MUL,
    EQL_BINOP_DIV,
} eql_ast_binop_e;

// Represents a binary expression in the AST.
typedef struct {
    eql_ast_binop_e operator;
    struct eql_ast_node *lhs;
    struct eql_ast_node *rhs;
} eql_ast_binary_expr;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_binary_expr_create(eql_ast_binop_e operator,
    struct eql_ast_node *lhs, struct eql_ast_node *rhs,
    struct eql_ast_node **ret);

#endif