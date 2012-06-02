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
    struct eql_ast_node *block;
} eql_ast_module;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_module_create(bstring name, struct eql_ast_node **methods,
    unsigned int method_count, struct eql_ast_node *block,
    struct eql_ast_node **ret);

void eql_ast_module_free(struct eql_ast_node *node);

#endif