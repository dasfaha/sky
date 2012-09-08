#ifndef _qip_ast_type_ref_h
#define _qip_ast_type_ref_h

#include "bstring.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a type reference.
typedef struct {
    bstring name;
    bstring arg_name;
    qip_ast_node *return_type;
    qip_ast_node **subtypes;
    unsigned int subtype_count;
} qip_ast_type_ref;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_type_ref_create(bstring name);

qip_ast_node *qip_ast_type_ref_create_cstr(char *name);

void qip_ast_type_ref_free(qip_ast_node *node);

void qip_ast_type_ref_free_return_type(qip_ast_node *node);

void qip_ast_type_ref_free_subtypes(qip_ast_node *node);

int qip_ast_type_ref_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Subtype Management
//--------------------------------------

int qip_ast_type_ref_add_subtype(qip_ast_node *node, qip_ast_node *subtype);

int qip_ast_type_ref_add_subtypes(qip_ast_node *node, qip_ast_node **subtypes,
    unsigned int subtype_count);

//--------------------------------------
// Return Type Management
//--------------------------------------

int qip_ast_type_ref_set_return_type(qip_ast_node *node,
    qip_ast_node *return_type);

//--------------------------------------
// Arg Name
//--------------------------------------

int qip_ast_type_ref_set_arg_name(qip_ast_node *node, bstring arg_name);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_type_ref_preprocess(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Type refs
//--------------------------------------

int qip_ast_type_ref_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_type_ref_get_dependencies(qip_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Type
//--------------------------------------

int qip_ast_type_ref_get_type(qip_ast_node *node, bstring *type);

int qip_ast_type_ref_get_full_name(qip_ast_node *node, bstring *ret);

//--------------------------------------
// Misc
//--------------------------------------

bool qip_ast_type_ref_is_void(qip_ast_node *node);

int qip_ast_type_ref_flatten(qip_ast_node *type_ref);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_type_ref_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_type_ref_dump(qip_ast_node *node, bstring ret);

#endif
