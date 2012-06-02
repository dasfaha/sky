#ifndef _eql_ast_fcall_h
#define _eql_ast_fcall_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Represents a function call in the AST.
typedef struct {
    bstring name;
    struct eql_ast_node **args;
    unsigned int arg_count;
} eql_ast_fcall;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_fcall_create(bstring name, struct eql_ast_node **args,
    unsigned int arg_count, struct eql_ast_node **ret);

void eql_ast_fcall_free(struct eql_ast_node *node);

#endif