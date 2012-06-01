#ifndef _eql_ast_var_decl_h
#define _eql_ast_var_decl_h

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
    bstring type;
    bstring name;
} eql_ast_var_decl;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_var_decl_create(bstring type, bstring name, struct eql_ast_node **node);

void eql_ast_var_decl_free(struct eql_ast_node *node);

#endif