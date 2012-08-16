#ifndef _eql_ast_template_var_h
#define _eql_ast_template_var_h

#include "bstring.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a template variable in the AST.
typedef struct {
    bstring name;
} eql_ast_template_var;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_template_var_create(bstring name);

void eql_ast_template_var_free(eql_ast_node *node);

int eql_ast_template_var_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_template_var_dump(eql_ast_node *node, bstring ret);

#endif
