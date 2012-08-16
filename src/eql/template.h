#ifndef _eql_ast_template_h
#define _eql_ast_template_h

#include "bstring.h"
#include "node.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a template in the AST.
typedef struct {
    eql_ast_node *class;
    eql_ast_node *type_ref;
} eql_ast_template;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_template *eql_ast_template_create(eql_ast_node *class,
    eql_ast_node *type_ref);

void eql_ast_template_free(eql_ast_template *template);


//--------------------------------------
// Templating
//--------------------------------------

int eql_ast_template_apply(eql_ast_template *template, eql_ast_node *node);


#endif
