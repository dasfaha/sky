#ifndef _qip_ast_property_h
#define _qip_ast_property_h

#include "bstring.h"
#include "access.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declarations.
struct qip_ast_node;

// Represents a property in the AST.
typedef struct {
    qip_ast_access_e access;
    struct qip_ast_node *var_decl;
    struct qip_ast_node **metadatas;
    unsigned int metadata_count;
} qip_ast_property;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_property_create(qip_ast_access_e access,
    struct qip_ast_node *var_decl);

void qip_ast_property_free(struct qip_ast_node *node);

int qip_ast_property_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Metadata Management
//--------------------------------------

int qip_ast_property_add_metadata(struct qip_ast_node *node,
    qip_ast_node *metadata);

int qip_ast_property_add_metadatas(struct qip_ast_node *node,
    qip_ast_node **metadatas, unsigned int metadata_count);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_property_preprocess(qip_ast_node *node, qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Find
//--------------------------------------

int qip_ast_property_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

int qip_ast_property_get_var_refs(qip_ast_node *node, bstring name,
    qip_array *array);

int qip_ast_property_get_var_refs_by_type(qip_ast_node *node, qip_module *module,
    bstring type_name, qip_array *array);

//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_property_get_dependencies(qip_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_property_validate(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Generation
//--------------------------------------

int qip_ast_property_generate_initializer(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_property_dump(qip_ast_node *node, bstring ret);

#endif