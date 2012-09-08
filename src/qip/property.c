#include <stdlib.h>
#include "dbg.h"

#include "node.h"
#include "util.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a property.
//
// var_decl - The variable declaration.
//
// Returns a property node.
qip_ast_node *qip_ast_property_create(qip_ast_access_e access,
                                      qip_ast_node *var_decl)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_PROPERTY;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->property.access   = access;
    node->property.var_decl = var_decl;
    node->property.metadata_count = 0;
    node->property.metadatas = NULL;

    if(var_decl != NULL) {
        var_decl->parent = node;
    }
    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a property AST node from memory.
//
// node - The AST node to free.
void qip_ast_property_free(struct qip_ast_node *node)
{
    if(node->property.var_decl) qip_ast_node_free(node->property.var_decl);
    node->property.var_decl = NULL;

    unsigned int i;
    for(i=0; i<node->property.metadata_count; i++) {
        qip_ast_node_free(node->property.metadatas[i]);
        node->property.metadatas[i] = NULL;
    }
    free(node->property.metadatas);
    node->property.metadata_count = 0;
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_property_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_property_create(node->property.access, NULL);
    check_mem(clone);

    rc = qip_ast_node_copy(node->property.var_decl, &clone->property.var_decl);
    check(rc == 0, "Unable to copy var decl");
    if(clone->property.var_decl) clone->property.var_decl->parent = clone;
    
    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Metadata Management
//--------------------------------------

// Adds a metadata tag to a property.
//
// node     - The node to add the metadata to.
// metadata - The metadata to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_property_add_metadata(qip_ast_node *node, qip_ast_node *metadata)
{
    // Validate.
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_PROPERTY, "Node type must be 'property'");
    check(metadata != NULL, "Metadata required");
    
    // Append metadata.
    node->property.metadata_count++;
    node->property.metadatas = realloc(node->property.metadatas, sizeof(qip_ast_node*) * node->property.metadata_count);
    check_mem(node->property.metadatas);
    node->property.metadatas[node->property.metadata_count-1] = metadata;
    
    // Link metadata.
    metadata->parent = node;
    
    return 0;

error:
    return -1;
}

// Adds a list of metatdata tags to a property.
//
// node            - The node.
// metadatas      - A list of metatdatas to add.
// metadata_count - The number of metatdatas to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_property_add_metadatas(qip_ast_node *node,
                                   qip_ast_node **metadatas,
                                   unsigned int metadata_count)
{
    // Validate.
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_PROPERTY, "Node type must be 'property'");
    check(metadatas != NULL || metadata_count == 0, "Metadata tags are required");

    // Add each metadata.
    unsigned int i;
    for(i=0; i<metadata_count; i++) {
        int rc = qip_ast_property_add_metadata(node, metadatas[i]);
        check(rc == 0, "Unable to add metadata to property");
    }
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Preprocesses the node.
//
// node   - The node.
// module - The module that the node is a part of.
// stage  - The processing stage.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_property_preprocess(qip_ast_node *node, qip_module *module,
                                qip_ast_processing_stage_e stage)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Preprocess metadata.
    unsigned int i;
    for(i=0; i<node->property.metadata_count; i++) {
        rc = qip_ast_node_preprocess(node->property.metadatas[i], module, stage);
        check(rc == 0, "Unable to preprocess property metadata");
    }

    // Preprocess variable declaration
    rc = qip_ast_node_preprocess(node->property.var_decl, module, stage);
    check(rc == 0, "Unable to preprocess property variable declaration");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Dependencies
//--------------------------------------

// Computes a list of class names that this AST node depends on.
//
// node         - The node to compute dependencies for.
// dependencies - A pointer to an array of dependencies.
// count        - A pointer to where the number of dependencies is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_property_get_dependencies(qip_ast_node *node,
                                      bstring **dependencies,
                                      uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    if(node->property.var_decl != NULL) {
        rc = qip_ast_node_get_dependencies(node->property.var_decl, dependencies, count);
        check(rc == 0, "Unable to add property variable declaration dependencies");
    }

    return 0;
    
error:
    qip_ast_node_dependencies_free(dependencies, count);
    return -1;
}


//--------------------------------------
// Find
//--------------------------------------

// Computes a list of type references used by a node.
//
// node      - The node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_property_get_type_refs(qip_ast_node *node,
                                   qip_ast_node ***type_refs,
                                   uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    if(node->property.var_decl != NULL) {
        rc = qip_ast_node_get_type_refs(node->property.var_decl, type_refs, count);
        check(rc == 0, "Unable to add property variable declaration type refs");
    }

    return 0;
    
error:
    qip_ast_node_type_refs_free(type_refs, count);
    return -1;
}

// Retrieves all variable reference of a given name within this node.
//
// node  - The node.
// name  - The variable name.
// array - The array to add the references to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_property_get_var_refs(qip_ast_node *node, bstring name,
                                  qip_array *array)
{
    int rc;
    check(node != NULL, "Node required");
    check(name != NULL, "Variable name required");
    check(array != NULL, "Array required");

    if(node->property.var_decl != NULL) {
        rc = qip_ast_node_get_var_refs(node->property.var_decl, name, array);
        check(rc == 0, "Unable to add property variable declaration var refs");
    }

    return 0;
    
error:
    return -1;
}


//--------------------------------------
// Validation
//--------------------------------------

// Validates the AST node.
//
// node   - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_property_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Validate metadata.
    unsigned int i;
    for(i=0; i<node->property.metadata_count; i++) {
        rc = qip_ast_node_validate(node->property.metadatas[i], module);
        check(rc == 0, "Unable to validate property metadata");
    }

    // Validate variable declaration
    rc = qip_ast_node_validate(node->property.var_decl, module);
    check(rc == 0, "Unable to validate property variable declaration");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Generation
//--------------------------------------

// Generates AST nodes for initializing a property. It will initialize numeric
// properties to zero and pointers to NULL. Initializes are automatically
// generated for class constructors.
//
// node - The property node.
// ret  - A pointer to where the initializing expression is returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_property_generate_initializer(qip_ast_node *node,
                                          qip_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");
    
    struct tagbstring this_str = bsStatic("this");

    // Generate access to the property.
    qip_ast_node *var_ref = qip_ast_var_ref_create_property_access(&this_str, node->property.var_decl->var_decl.name);
    check_mem(var_ref);
    var_ref->generated = true;
    
    // Generate value depending on the type.
    qip_ast_node *value = NULL;
    if(qip_is_builtin_type(node->property.var_decl->var_decl.type)) {
        bstring type_name = node->property.var_decl->var_decl.type->type_ref.name;
        if(biseqcstr(type_name, "Int")) {
            value = qip_ast_int_literal_create(0); check_mem(value);
        }
        else if(biseqcstr(type_name, "Float")) {
            value = qip_ast_float_literal_create(0); check_mem(value);
        }
        else if(biseqcstr(type_name, "Boolean")) {
            value = qip_ast_boolean_literal_create(false); check_mem(value);
        }
    }

    // If built-in type initializer is not found then set it to null.
    if(value == NULL) {
        value = qip_ast_null_literal_create(); check_mem(value);
    }
    value->generated = true;

    // Generate assignment.
    qip_ast_node *var_assign = qip_ast_var_assign_create(var_ref, value);
    check_mem(var_assign);
    var_assign->generated = true;
    
    // Return initializer.
    *ret = var_assign;
    
    return 0;
    
error:
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Debugging
//--------------------------------------

// Append the contents of the AST node to the string.
// 
// node - The node to dump.
// ret  - A pointer to the bstring to concatenate to.
//
// Return 0 if successful, otherwise returns -1.s
int qip_ast_property_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    check(bcatcstr(ret, "<property>\n") == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    unsigned int i;
    for(i=0; i<node->method.metadata_count; i++) {
        rc = qip_ast_node_dump(node->method.metadatas[i], ret);
        check(rc == 0, "Unable to dump method metadata");
    }
    if(node->property.var_decl != NULL) {
        rc = qip_ast_node_dump(node->property.var_decl, ret);
        check(rc == 0, "Unable to dump property variable declaration");
    }

    return 0;

error:
    return -1;
}
