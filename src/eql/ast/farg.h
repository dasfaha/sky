#ifndef _eql_ast_farg_h
#define _eql_ast_farg_h

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
    struct eql_ast_node *var_decl;
} eql_ast_farg;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_farg_create(struct eql_ast_node *var_decl, struct eql_ast_node **node);

void eql_ast_farg_free(struct eql_ast_node *node);

#endif