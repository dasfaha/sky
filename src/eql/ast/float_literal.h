#ifndef _eql_ast_float_literal_h
#define _eql_ast_float_literal_h

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Represents a literal floating point number in the AST.
typedef struct {
    double value;
} eql_ast_float_literal;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_float_literal_create(double value, struct eql_ast_node **node);

#endif