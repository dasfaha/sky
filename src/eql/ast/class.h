#ifndef _eql_ast_class_h
#define _eql_ast_class_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declaration of node.
struct eql_ast_node;

// Represents a class in the AST.
typedef struct {
    bstring name;
    struct eql_ast_node **methods;
    unsigned int method_count;
    struct eql_ast_node **properties;
    unsigned int property_count;
} eql_ast_class;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_class_create(bstring name, struct eql_ast_node **methods,
    unsigned int method_count, struct eql_ast_node **properties,
    unsigned int property_count, struct eql_ast_node **ret);

void eql_ast_class_free(struct eql_ast_node *node);

#endif