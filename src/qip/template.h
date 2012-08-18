#ifndef _qip_ast_template_h
#define _qip_ast_template_h

#include "bstring.h"
#include "node.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a template in the AST.
typedef struct {
    qip_ast_node *class;
    qip_ast_node *type_ref;
} qip_ast_template;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_template *qip_ast_template_create(qip_ast_node *class,
    qip_ast_node *type_ref);

void qip_ast_template_free(qip_ast_template *template);


//--------------------------------------
// Templating
//--------------------------------------

int qip_ast_template_apply(qip_ast_template *template, qip_ast_node *node);


#endif
