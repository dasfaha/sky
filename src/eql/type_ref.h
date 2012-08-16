#ifndef _eql_ast_type_ref_h
#define _eql_ast_type_ref_h

#include "bstring.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a type reference.
typedef struct {
    bstring name;
    eql_ast_node **subtypes;
    unsigned int subtype_count;
} eql_ast_type_ref;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_type_ref_create(bstring name);

eql_ast_node *eql_ast_type_ref_create_cstr(char *name);

void eql_ast_type_ref_free(eql_ast_node *node);

void eql_ast_type_ref_free_subtypes(eql_ast_node *node);

int eql_ast_type_ref_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Subtype Management
//--------------------------------------

int eql_ast_type_ref_add_subtype(eql_ast_node *node, eql_ast_node *subtype);

int eql_ast_type_ref_add_subtypes(eql_ast_node *node, eql_ast_node **subtypes,
    unsigned int subtype_count);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_type_ref_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_type_ref_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_type_ref_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Type
//--------------------------------------

int eql_ast_type_ref_get_type(eql_ast_node *node, bstring *type);

int eql_ast_type_ref_get_full_name(eql_ast_node *node, bstring *ret);

//--------------------------------------
// Misc
//--------------------------------------

bool eql_ast_type_ref_is_void(eql_ast_node *node);

int eql_ast_type_ref_flatten(eql_ast_node *type_ref);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_type_ref_validate(eql_ast_node *node, eql_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_type_ref_dump(eql_ast_node *node, bstring ret);

#endif
