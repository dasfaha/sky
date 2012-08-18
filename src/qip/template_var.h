#ifndef _qip_ast_template_var_h
#define _qip_ast_template_var_h

#include "bstring.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a template variable in the AST.
typedef struct {
    bstring name;
} qip_ast_template_var;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_template_var_create(bstring name);

void qip_ast_template_var_free(qip_ast_node *node);

int qip_ast_template_var_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_template_var_dump(qip_ast_node *node, bstring ret);

#endif
