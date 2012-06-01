#ifndef _eql_ast_var_ref_h
#define _eql_ast_var_ref_h

#include "../../bstring.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Represents a variable reference in the AST.
typedef struct {
    bstring name;
} eql_ast_var_ref;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_var_ref_create(bstring name, struct eql_ast_node **node);

void eql_ast_var_ref_free(struct eql_ast_node *node);

#endif