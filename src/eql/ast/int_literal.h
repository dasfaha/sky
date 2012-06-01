#ifndef _eql_ast_int_literal_h
#define _eql_ast_int_literal_h

#include <inttypes.h>


//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Represents a literal integer in the AST.
typedef struct {
    int64_t value;
} eql_ast_int_literal;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_int_literal_create(int64_t value, struct eql_ast_node **node);

#endif