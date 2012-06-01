#ifndef _eql_ast_fproto_h
#define _eql_ast_fproto_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Represents a variable declaration in the AST.
typedef struct {
    bstring name;
    bstring return_type;
    struct eql_ast_node **args;
    unsigned int arg_count;
} eql_ast_fproto;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_fproto_create(bstring name, bstring return_type,
    struct eql_ast_node **args, unsigned int arg_count,
    struct eql_ast_node **ret);

void eql_ast_fproto_free(struct eql_ast_node *node);

#endif