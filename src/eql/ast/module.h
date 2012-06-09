#ifndef _eql_ast_module_h
#define _eql_ast_module_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Represents a module in the AST.
typedef struct {
    bstring name;
    struct eql_ast_node **classes;
    unsigned int class_count;
    struct eql_ast_node *main_function;
} eql_ast_module;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_module_create(bstring name, struct eql_ast_node **classes,
    unsigned int class_count, struct eql_ast_node *main_function,
    struct eql_ast_node **ret);

void eql_ast_module_free(struct eql_ast_node *node);


//--------------------------------------
// Class Management
//--------------------------------------

int eql_ast_module_add_class(struct eql_ast_node *module, struct eql_ast_node *class);


#endif