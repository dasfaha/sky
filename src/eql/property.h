#ifndef _eql_ast_property_h
#define _eql_ast_property_h

#include "bstring.h"
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
    struct eql_ast_node **metadatas;
    unsigned int metadata_count;
} eql_ast_property;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_property_create(eql_ast_access_e access,
    struct eql_ast_node *var_decl);

void eql_ast_property_free(struct eql_ast_node *node);

int eql_ast_property_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Metadata Management
//--------------------------------------

int eql_ast_property_add_metadata(struct eql_ast_node *node,
    eql_ast_node *metadata);

int eql_ast_property_add_metadatas(struct eql_ast_node *node,
    eql_ast_node **metadatas, unsigned int metadata_count);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_property_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_property_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_property_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_property_validate(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Generation
//--------------------------------------

int eql_ast_property_generate_initializer(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_property_dump(eql_ast_node *node, bstring ret);

#endif