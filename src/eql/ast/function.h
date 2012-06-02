#ifndef _eql_ast_function_h
#define _eql_ast_function_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Represents a function in the AST.
typedef struct {
    bstring name;
    bstring return_type;
    struct eql_ast_node **args;
    unsigned int arg_count;
    struct eql_ast_node *body;
} eql_ast_function;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_function_create(bstring name, bstring return_type,
    struct eql_ast_node **args, unsigned int arg_count,
    struct eql_ast_node *body, struct eql_ast_node **ret);

void eql_ast_function_free(struct eql_ast_node *node);

#endif