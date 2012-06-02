#ifndef _eql_ast_property_h
#define _eql_ast_property_h

#include "../../bstring.h"
#include "access.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declarations.
struct eql_ast_node;

// Represents a property in the AST.
typedef struct {
    eql_ast_access_e access;
    struct eql_ast_node *var_decl;
} eql_ast_property;


//==============================================================================
//
// Functions
//
//==============================================================================

int eql_ast_property_create(eql_ast_access_e access,
    struct eql_ast_node *var_decl, struct eql_ast_node **ret);

void eql_ast_property_free(struct eql_ast_node *node);

#endif