#include <stdlib.h>
#include "dbg.h"
#include "util.h"
#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Node Lifecycle
//--------------------------------------

// Recursively frees an AST node.
//
// node - The node to free.
void qip_ast_node_free(qip_ast_node *node)
{
    if(!node) return;
    
    // Recursively free dependent data.
    switch(node->type) {
        case QIP_AST_TYPE_INT_LITERAL: qip_ast_int_literal_free(node); break;
        case QIP_AST_TYPE_FLOAT_LITERAL: qip_ast_float_literal_free(node); break;
        case QIP_AST_TYPE_BOOLEAN_LITERAL: qip_ast_boolean_literal_free(node); break;
        case QIP_AST_TYPE_STRING_LITERAL: qip_ast_string_literal_free(node); break;
        case QIP_AST_TYPE_NULL_LITERAL: qip_ast_null_literal_free(node); break;
        case QIP_AST_TYPE_ARRAY_LITERAL: qip_ast_array_literal_free(node); break;
        case QIP_AST_TYPE_BINARY_EXPR: qip_ast_binary_expr_free(node); break;
        case QIP_AST_TYPE_VAR_REF: qip_ast_var_ref_free(node); break;
        case QIP_AST_TYPE_VAR_DECL: qip_ast_var_decl_free(node); break;
        case QIP_AST_TYPE_TYPE_REF: qip_ast_type_ref_free(node); break;
        case QIP_AST_TYPE_VAR_ASSIGN: qip_ast_var_assign_free(node); break;
        case QIP_AST_TYPE_FARG: qip_ast_farg_free(node); break;
        case QIP_AST_TYPE_FRETURN: qip_ast_freturn_free(node); break;
        case QIP_AST_TYPE_FUNCTION: qip_ast_function_free(node); break;
        case QIP_AST_TYPE_BLOCK: qip_ast_block_free(node); break;
        case QIP_AST_TYPE_IF_STMT: qip_ast_if_stmt_free(node); break;
        case QIP_AST_TYPE_FOR_EACH_STMT: qip_ast_for_each_stmt_free(node); break;
        case QIP_AST_TYPE_METHOD: qip_ast_method_free(node); break;
        case QIP_AST_TYPE_PROPERTY: qip_ast_property_free(node); break;
        case QIP_AST_TYPE_CLASS: qip_ast_class_free(node); break;
        case QIP_AST_TYPE_TEMPLATE_VAR: qip_ast_template_var_free(node); break;
        case QIP_AST_TYPE_MODULE: qip_ast_module_free(node); break;
        case QIP_AST_TYPE_METADATA: qip_ast_metadata_free(node); break;
        case QIP_AST_TYPE_METADATA_ITEM: qip_ast_metadata_item_free(node); break;
        case QIP_AST_TYPE_SIZEOF: qip_ast_sizeof_free(node); break;
        case QIP_AST_TYPE_ALLOCA: qip_ast_alloca_free(node); break;
    }
    node->parent = NULL;
    
    free(node);
}


//--------------------------------------
// Hierarchy
//--------------------------------------

// Retrieves the node depth level in the AST tree.
//
// node  - The node to retrieve the depth for.
// depth - A pointer to where the depth should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_get_depth(qip_ast_node *node, int32_t *depth)
{
    check(node != NULL, "Node required");

    // Count how many parents the node has.
    int32_t count = 0;
    qip_ast_node *parent = node->parent;
    while(parent != NULL) {
        count++;
        parent = parent->parent;
    }
    
    // Return the value to the caller.
    *depth = count;
    return 0;
    
error:
    *depth = 0;
    return -1;
}

// Replaces a node with a different node.
//
// node - The node to swap out.
// new_node - The new node to replace the original with.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_replace(qip_ast_node *node, qip_ast_node *new_node)
{
    int rc;
    check(node != NULL, "Node required");
    check(new_node != NULL, "New node required");
    
    // Exit if there is no parent node.
    if(node->parent == NULL) {
        return 0;
    }
    
    // Delegate copy to child nodes.
    switch(node->parent->type) {
        case QIP_AST_TYPE_VAR_DECL: rc = qip_ast_var_decl_replace(node->parent, node, new_node); break;
        default: rc = 0;
    }
    check(rc == 0, "Unable to replace node");
    
    return 0;

error:
    return -1;
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    check(ret != NULL, "Return pointer required");
    
    // Initialize return value.
    *ret = NULL;
    
    // Exit if there is no node.
    if(node == NULL) {
        return 0;
    }

    // Delegate copy to child nodes.
    switch(node->type) {
        case QIP_AST_TYPE_INT_LITERAL: rc = qip_ast_int_literal_copy(node, ret); break;
        case QIP_AST_TYPE_FLOAT_LITERAL: rc = qip_ast_float_literal_copy(node, ret); break;
        case QIP_AST_TYPE_BOOLEAN_LITERAL: rc = qip_ast_boolean_literal_copy(node, ret); break;
        case QIP_AST_TYPE_STRING_LITERAL: rc = qip_ast_string_literal_copy(node, ret); break;
        case QIP_AST_TYPE_NULL_LITERAL: rc = qip_ast_null_literal_copy(node, ret); break;
        case QIP_AST_TYPE_ARRAY_LITERAL: rc = qip_ast_array_literal_copy(node, ret); break;
        case QIP_AST_TYPE_BINARY_EXPR: rc = qip_ast_binary_expr_copy(node, ret); break;
        case QIP_AST_TYPE_VAR_DECL: rc = qip_ast_var_decl_copy(node, ret); break;
        case QIP_AST_TYPE_VAR_REF: rc = qip_ast_var_ref_copy(node, ret); break;
        case QIP_AST_TYPE_TYPE_REF: rc = qip_ast_type_ref_copy(node, ret); break;
        case QIP_AST_TYPE_VAR_ASSIGN: rc = qip_ast_var_assign_copy(node, ret); break;
        case QIP_AST_TYPE_FRETURN: rc = qip_ast_freturn_copy(node, ret); break;
        case QIP_AST_TYPE_FARG: rc = qip_ast_farg_copy(node, ret); break;
        case QIP_AST_TYPE_MODULE: rc = qip_ast_module_copy(node, ret); break;
        case QIP_AST_TYPE_FUNCTION: rc = qip_ast_function_copy(node, ret); break;
        case QIP_AST_TYPE_BLOCK: rc = qip_ast_block_copy(node, ret); break;
        case QIP_AST_TYPE_IF_STMT: rc = qip_ast_if_stmt_copy(node, ret); break;
        case QIP_AST_TYPE_FOR_EACH_STMT: rc = qip_ast_for_each_stmt_copy(node, ret); break;
        case QIP_AST_TYPE_CLASS: rc = qip_ast_class_copy(node, ret); break;
        case QIP_AST_TYPE_METHOD: rc = qip_ast_method_copy(node, ret); break;
        case QIP_AST_TYPE_PROPERTY: rc = qip_ast_property_copy(node, ret); break;
        case QIP_AST_TYPE_TEMPLATE_VAR: rc = qip_ast_template_var_copy(node, ret); break;
        case QIP_AST_TYPE_METADATA: rc = qip_ast_metadata_copy(node, ret); break;
        case QIP_AST_TYPE_METADATA_ITEM: rc = qip_ast_metadata_item_copy(node, ret); break;
        case QIP_AST_TYPE_SIZEOF: rc = qip_ast_sizeof_copy(node, ret); break;
        case QIP_AST_TYPE_ALLOCA: rc = qip_ast_alloca_copy(node, ret); break;
    }
    check(rc == 0, "Unable to copy node");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for an AST node.
//
// node    - The node to generate LLVM values for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_codegen(qip_ast_node *node, qip_module *module,
                         LLVMValueRef *value)
{
    int rc;
    
    check(node != NULL, "Node is required");
    check(module != NULL, "Module is required");
    check(module->llvm_module != NULL, "LLVM Module is required");
    check(module->compiler != NULL, "Module compiler is required");
    check(module->compiler->llvm_builder != NULL, "LLVM Builder is required");

    // Create an intermediate return value so that callers can optionally
    // pass in NULL for the return value.
    LLVMValueRef ret_value = NULL;

    // Delegate codegen to AST nodes.
    switch(node->type) {
        case QIP_AST_TYPE_INT_LITERAL: rc = qip_ast_int_literal_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_FLOAT_LITERAL: rc = qip_ast_float_literal_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_BOOLEAN_LITERAL: rc = qip_ast_boolean_literal_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_STRING_LITERAL: rc = qip_ast_string_literal_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_NULL_LITERAL: rc = qip_ast_null_literal_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_ARRAY_LITERAL: rc = qip_ast_array_literal_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_BINARY_EXPR: rc = qip_ast_binary_expr_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_VAR_DECL: rc = qip_ast_var_decl_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_VAR_REF: rc = qip_ast_var_ref_codegen(node, module, false, &ret_value); break;
        case QIP_AST_TYPE_VAR_ASSIGN: rc = qip_ast_var_assign_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_FRETURN: rc = qip_ast_freturn_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_FARG: rc = qip_ast_farg_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_FUNCTION: rc = qip_ast_function_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_BLOCK: rc = qip_ast_block_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_IF_STMT: rc = qip_ast_if_stmt_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_FOR_EACH_STMT: rc = qip_ast_for_each_stmt_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_CLASS: rc = qip_ast_class_codegen(node, module); break;
        case QIP_AST_TYPE_METHOD: rc = qip_ast_method_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_SIZEOF: rc = qip_ast_sizeof_codegen(node, module, &ret_value); break;
        case QIP_AST_TYPE_ALLOCA: rc = qip_ast_alloca_codegen(node, module, &ret_value); break;
        default: rc = 0;
    }
    check(rc == 0, "Unable to codegen node");
    
    // If a return value is requested then set it.
    if(value != NULL) {
        *value = ret_value;
    }
    
    return 0;

error:
    if(value != NULL) *value = NULL;
    return -1;
}

// Retrieves a variable pointer to a given node. This only works with variable
// references and struct member accesses.
//
// node    - The variable node to retrieve the pointer for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_get_var_pointer(qip_ast_node *node, qip_module *module,
                                 LLVMValueRef *value)
{
    int rc;
    
    check(node != NULL, "Node is required");
    check(module != NULL, "Module is required");
    check(module->llvm_module != NULL, "LLVM Module is required");
    check(module->compiler != NULL, "Module compiler is required");
    check(module->compiler->llvm_builder != NULL, "LLVM Builder is required");

    // Delegate codegen to AST nodes.
    switch(node->type) {
        case QIP_AST_TYPE_VAR_REF: {
            rc = qip_ast_var_ref_get_pointer(node, module, NULL, value);
            check(rc == 0, "Unable to retrieve pointer to variable reference");
            break;
        }
        default:
        {
            sentinel("Unable to retrieve pointer for AST node");
            break;
        }
    }
    
    return 0;
    
error:
    *value = NULL;
    return -1;
}

// Generates forward declarations for a node.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_codegen_forward_decl(qip_ast_node *node, qip_module *module)
{
    int rc;
    
    check(node != NULL, "Node is required");
    check(module != NULL, "Module is required");

    // Delegate codegen to AST nodes.
    switch(node->type) {
        case QIP_AST_TYPE_MODULE: rc = qip_ast_module_codegen_forward_decl(node, module); break;
        case QIP_AST_TYPE_CLASS: rc = qip_ast_class_codegen_forward_decl(node, module); break;
        case QIP_AST_TYPE_METHOD: rc = qip_ast_method_codegen_forward_decl(node, module); break;
        case QIP_AST_TYPE_FUNCTION: rc = qip_ast_function_codegen_forward_decl(node, module); break;
        case QIP_AST_TYPE_METADATA: rc = qip_ast_metadata_codegen_forward_decl(node, module); break;
        default: rc = 0;
    }
    check(rc == 0, "Unable to codegen node");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Recursively preprocesses an AST tree.
//
// node    - The node to preprocess.
// module  - The compilation unit this node is a part of.
// stage   - The processing stage.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_preprocess(qip_ast_node *node, qip_module *module,
                            qip_ast_processing_stage_e stage)
{
    int rc = 0;
    check(node != NULL, "Node is required");
    check(module != NULL, "Module is required");

    // Delegate validation to AST node types.
    switch(node->type) {
        case QIP_AST_TYPE_ARRAY_LITERAL: rc = qip_ast_array_literal_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_BINARY_EXPR: rc = qip_ast_binary_expr_preprocess(node, module); break;
        case QIP_AST_TYPE_FUNCTION: rc = qip_ast_function_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_BLOCK: rc = qip_ast_block_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_MODULE: rc = qip_ast_module_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_FRETURN: rc = qip_ast_freturn_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_VAR_DECL: rc = qip_ast_var_decl_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_TYPE_REF: rc = qip_ast_type_ref_preprocess(node, module); break;
        case QIP_AST_TYPE_VAR_REF: rc = qip_ast_var_ref_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_VAR_ASSIGN: rc = qip_ast_var_assign_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_FARG: rc = qip_ast_farg_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_IF_STMT: rc = qip_ast_if_stmt_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_FOR_EACH_STMT: rc = qip_ast_for_each_stmt_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_CLASS: rc = qip_ast_class_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_METHOD: rc = qip_ast_method_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_PROPERTY: rc = qip_ast_property_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_METADATA: rc = qip_ast_metadata_preprocess(node, module, stage); break;
        case QIP_AST_TYPE_METADATA_ITEM: rc = qip_ast_metadata_item_preprocess(node, module); break;
        case QIP_AST_TYPE_SIZEOF: rc = qip_ast_sizeof_preprocess(node, module); break;
        case QIP_AST_TYPE_ALLOCA: rc = qip_ast_alloca_preprocess(node, module); break;
        default: rc = 0;
    }
    check(rc == 0, "Unable to preprocess node");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Types
//--------------------------------------

// Recursively determines the type of a node.
//
// node   - The node to determine the type for.
// module - The compilation unit this node is a part of.
// type   - A pointer to where the type should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_get_type(qip_ast_node *node, qip_module *module,
                          qip_ast_node **type)
{
    int rc;
    check(node != NULL, "Node required");

    // Delegate to each type.
    switch(node->type) {
        case QIP_AST_TYPE_INT_LITERAL: rc = qip_ast_int_literal_get_type(node, type); break;
        case QIP_AST_TYPE_FLOAT_LITERAL: rc = qip_ast_float_literal_get_type(node, type); break;
        case QIP_AST_TYPE_BOOLEAN_LITERAL: rc = qip_ast_boolean_literal_get_type(node, type); break;
        case QIP_AST_TYPE_STRING_LITERAL: rc = qip_ast_string_literal_get_type(node, type); break;
        case QIP_AST_TYPE_NULL_LITERAL: rc = qip_ast_null_literal_get_type(node, type); break;
        case QIP_AST_TYPE_ARRAY_LITERAL: rc = qip_ast_array_literal_get_type(node, type); break;
        case QIP_AST_TYPE_BINARY_EXPR: rc = qip_ast_binary_expr_get_type(node, module, type); break;
        case QIP_AST_TYPE_VAR_REF: rc = qip_ast_var_ref_get_type(node, module, type); break;
        case QIP_AST_TYPE_SIZEOF: rc = qip_ast_sizeof_get_type(node, type); break;
        case QIP_AST_TYPE_ALLOCA: rc = qip_ast_sizeof_get_type(node, type); break;
        default: {
            sentinel("AST node does not have a type");
            break;
        }
    }
    check(rc == 0, "Unable to retrieve type name");
    
    return 0;

error:
    *type = NULL;
    return -1;
}

// Recursively determines the type name of a node.
//
// node   - The node to determine the type for.
// module - The compilation unit this node is a part of.
// type   - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_get_type_name(qip_ast_node *node, qip_module *module,
                               bstring *ret)
{
    int rc;
    check(node != NULL, "Node required");

    // Retrieve type.
    qip_ast_node *type_ref = NULL;
    rc = qip_ast_node_get_type(node, module, &type_ref);
    check(rc == 0, "Unable to retrieve type ref");

    // If a type was found then generate the full name.
    if(type_ref != NULL) {
        rc = qip_ast_type_ref_get_full_name(type_ref, ret);
        check(rc == 0, "Unable to retrieve type ref full name");
    }
    // Otherwise return NULL if no type was found.
    else {
        *ret = NULL;
    }
    
    return 0;

error:
    *ret = NULL;
    return -1;
}


// Performs a non-recursive search for variable declarations of a given name.
//
// node     - The node to search within.
// name     - The name of the variable to search for.
// var_decl - A pointer to where the variable declaration should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_get_var_decl(qip_ast_node *node, bstring name,
                             qip_ast_node **var_decl)
{
    int rc;

    check(node != NULL, "Node required");

    // Delegate to each type.
    switch(node->type) {
        case QIP_AST_TYPE_FUNCTION: {
            rc = qip_ast_function_get_var_decl(node, name, var_decl);
            check(rc == 0, "Unable to retrieve variable declaration for function");
            break;
        }
        case QIP_AST_TYPE_BLOCK: {
            rc = qip_ast_block_get_var_decl(node, name, var_decl);
            check(rc == 0, "Unable to retrieve variable declaration for block");
            break;
        }
        case QIP_AST_TYPE_FOR_EACH_STMT: {
            rc = qip_ast_for_each_stmt_get_var_decl(node, name, var_decl);
            check(rc == 0, "Unable to retrieve variable declaration for for each statement");
            break;
        }
        default:
        {
            *var_decl = NULL;
            break;
        }
    }
    
    return 0;

error:
    *var_decl = NULL;
    return -1;
}


//--------------------------------------
// Type Refs
//--------------------------------------

// Computes a list of type references used in the node and its children. The
// type refs are not copies so they should not be freed.
//
// node      - The node.
// type_refs - A pointer to an array of type ref nodes.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_get_type_refs(qip_ast_node *node, qip_ast_node ***type_refs,
                               uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Recursively retrieve type refs from children.
    switch(node->type) {
        case QIP_AST_TYPE_MODULE: rc = qip_ast_module_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_CLASS: rc = qip_ast_class_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_METHOD: rc = qip_ast_method_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_FUNCTION: rc = qip_ast_function_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_FARG: rc = qip_ast_farg_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_BLOCK: rc = qip_ast_block_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_PROPERTY: rc = qip_ast_property_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_VAR_DECL: rc = qip_ast_var_decl_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_TYPE_REF: rc = qip_ast_type_ref_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_IF_STMT: rc = qip_ast_if_stmt_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_FOR_EACH_STMT: rc = qip_ast_for_each_stmt_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_BINARY_EXPR: rc = qip_ast_binary_expr_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_VAR_ASSIGN: rc = qip_ast_var_assign_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_SIZEOF: rc = qip_ast_sizeof_get_type_refs(node, type_refs, count); break;
        case QIP_AST_TYPE_ALLOCA: rc = qip_ast_alloca_get_type_refs(node, type_refs, count); break;
        default: rc = 0; break;
    }
    check(rc == 0, "Unable to retrieve type refs");

    return 0;

error:
    qip_ast_node_type_refs_free(type_refs, count);
    return -1;
}

// Adds a type ref onto the list of type refs.
//
// type_ref  - The type ref node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_add_type_ref(qip_ast_node *type_ref, qip_ast_node ***type_refs, uint32_t *count)
{
    check(type_refs != NULL, "Type ref array pointer required");
    check(count != NULL, "Type ref count pointer required");
    
    // If dependency is blank or null or it is a built-in type then ignore it.
    if(type_ref == NULL) {
        return 0;
    }
    
    // Increment the count and append.
    (*count)++;
    *type_refs = realloc(*type_refs, sizeof(**type_refs) * (*count));
    check_mem(*type_refs);
    (*type_refs)[(*count)-1] = type_ref;
    check_mem((*type_refs)[(*count)-1]);
    
    return 0;

error:
    qip_ast_node_type_refs_free(type_refs, count);
    return -1;
}

// Frees memory used by a list of type refs.
//
// type_refs - An array of type ref.
// count     - The number of dependencies.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_type_refs_free(qip_ast_node ***type_refs, uint32_t *count)
{
    if(*type_refs != NULL) free(*type_refs);
    *type_refs = NULL;
    *count = 0;
    return 0;
}
    

//--------------------------------------
// Var Refs
//--------------------------------------

// Retrieves an array of variable references within 
//
// node  - The node.
// name  - The name of the variable.
// array - The array to add variable references to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_get_var_refs(qip_ast_node *node, bstring name,
                              qip_array *array)
{
    int rc;
    check(node != NULL, "Node required");
    check(name != NULL, "Variable name required");
    check(array != NULL, "Array required");

    // Recursively retrieve var refs from children.
    switch(node->type) {
        case QIP_AST_TYPE_MODULE: rc = qip_ast_module_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_CLASS: rc = qip_ast_class_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_METHOD: rc = qip_ast_method_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_FUNCTION: rc = qip_ast_function_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_FARG: rc = qip_ast_farg_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_BLOCK: rc = qip_ast_block_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_PROPERTY: rc = qip_ast_property_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_VAR_DECL: rc = qip_ast_var_decl_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_IF_STMT: rc = qip_ast_if_stmt_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_FOR_EACH_STMT: rc = qip_ast_for_each_stmt_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_BINARY_EXPR: rc = qip_ast_binary_expr_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_VAR_ASSIGN: rc = qip_ast_var_assign_get_var_refs(node, name, array); break;
        case QIP_AST_TYPE_VAR_REF: rc = qip_ast_var_ref_get_var_refs(node, name, array); break;
        default: rc = 0; break;
    }
    check(rc == 0, "Unable to retrieve var refs");

    return 0;

error:
    return -1;
}


//--------------------------------------
// Blocks
//--------------------------------------

// Retrieves the block and expression index of a given node. This node may
// not be directly attached to the block so the expression index represents
// the index of the ancestor of the node that is directly attached to the
// block.
//
// node           - The node.
// ret_block      - A pointer to where the block should be returned.
// ret_expr_index - A pointer to where the expression index should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_get_block_expr_index(qip_ast_node *node,
                                      qip_ast_node **ret_block,
                                      int32_t *ret_expr_index)
{
    int rc;
    check(node != NULL, "Node required");
    check(node != NULL, "Return block pointer required");
    check(node != NULL, "Return expression index pointer required");
    
    // Initialize return values.
    *ret_block = NULL;
    *ret_expr_index = -1;
    
    // Loop up the hierarchy until we reach a parent that is a block.
    qip_ast_node *current = node;
    while(current != NULL) {
        if(current->parent->type == QIP_AST_TYPE_BLOCK) {
            int32_t expr_index;
            rc = qip_ast_block_get_expr_index(current->parent, current, &expr_index);
            check(rc == 0, "Unable to retrieve expression index");
            
            if(expr_index >= 0) {
                *ret_block = current->parent;
                *ret_expr_index = expr_index;
            }
            break;
        }
        
        current = current->parent;
    }
    
    return 0;

error:
    *ret_block = NULL;
    *ret_expr_index = -1;
    return -1;
}


//--------------------------------------
// Dependencies
//--------------------------------------

// Computes a list of class names that this AST tree depends on.
//
// node         - The node to compute dependencies for.
// dependencies - A pointer to an array of dependencies.
// count        - A pointer to where the number of dependencies is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_get_dependencies(qip_ast_node *node, bstring **dependencies,
                                  uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Recursively retrieve dependencies from children.
    switch(node->type) {
        case QIP_AST_TYPE_VAR_DECL: rc = qip_ast_var_decl_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_TYPE_REF: rc = qip_ast_type_ref_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_FARG: rc = qip_ast_farg_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_FUNCTION: rc = qip_ast_function_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_BLOCK: rc = qip_ast_block_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_IF_STMT: rc = qip_ast_if_stmt_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_FOR_EACH_STMT: rc = qip_ast_for_each_stmt_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_CLASS: rc = qip_ast_class_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_METHOD: rc = qip_ast_method_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_PROPERTY: rc = qip_ast_property_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_MODULE: rc = qip_ast_module_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_SIZEOF: rc = qip_ast_sizeof_get_dependencies(node, dependencies, count); break;
        case QIP_AST_TYPE_ALLOCA: rc = qip_ast_sizeof_get_dependencies(node, dependencies, count); break;
        default: rc = 0; break;
    }
    check(rc == 0, "Unable to retrieve dependencies");

    return 0;

error:
    qip_ast_node_dependencies_free(dependencies, count);
    return -1;
}

// Adds a dependency onto the list of dependencies.
//
// dependency   - The name of the dependency.
// dependencies - A pointer to an array of dependencies.
// count        - A pointer to where the number of dependencies is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_add_dependency(bstring dependency, bstring **dependencies, uint32_t *count)
{
    check(dependencies != NULL, "Dependency array pointer required");
    check(count != NULL, "Dependency count pointer required");
    
    // If dependency is blank or null or it is a built-in type then ignore it.
    if(dependency == NULL || biseqcstr(dependency, "") || qip_is_builtin_type_name(dependency)) {
        return 0;
    }
    
    // If dependency exists then exit.
    uint32_t i;
    for(i=0; i<*count; i++) {
        if(biseq(dependency, (*dependencies)[i])) {
            return 0;
        }
    }
    
    // Increment the count and append.
    (*count)++;
    *dependencies = realloc(*dependencies, sizeof(bstring) * (*count));
    check_mem(*dependencies);
    (*dependencies)[(*count)-1] = bstrcpy(dependency);
    check_mem((*dependencies)[(*count)-1]);
    
    return 0;

error:
    qip_ast_node_dependencies_free(dependencies, count);
    return -1;
}
    
// Frees memory used by a list of dependencies.
//
// dependencies - An array of dependency names.
// count        - The number of dependencies.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_dependencies_free(bstring **dependencies, uint32_t *count)
{
    if(*dependencies != NULL) {
        uint32_t i;
        for(i=0; i<*count; i++) {
            bdestroy((*dependencies)[i]);
            (*dependencies)[i] = NULL;
        }
        free(*dependencies);
    }
    
    *dependencies = NULL;
    *count = 0;

    return 0;
}
    
//--------------------------------------
// Position
//--------------------------------------

// Assigns the line number and character number for an AST node. This function
// is used by the parser.
//
// node - The node.
// line_no - The line number that this node is parsed from.
// char_no - The character position on the given line number.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_set_pos(qip_ast_node *node, int32_t line_no, int32_t char_no)
{
    check(node != NULL, "Node required");
    node->line_no = line_no;
    node->char_no = char_no;
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Validation
//--------------------------------------

// Recursively validates an AST tree. All validation errors are appended to
// the module.
//
// node    - The node to validate.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_validate(qip_ast_node *node, qip_module *module)
{
    int rc = 0;
    
    check(node != NULL, "Node is required");
    check(module != NULL, "Module is required");

    // Delegate validation to AST node types.
    switch(node->type) {
        case QIP_AST_TYPE_BINARY_EXPR: rc = qip_ast_binary_expr_validate(node, module); break;
        case QIP_AST_TYPE_FUNCTION: rc = qip_ast_function_validate(node, module); break;
        case QIP_AST_TYPE_BLOCK: rc = qip_ast_block_validate(node, module); break;
        case QIP_AST_TYPE_MODULE: rc = qip_ast_module_validate(node, module); break;
        case QIP_AST_TYPE_FRETURN: rc = qip_ast_freturn_validate(node, module); break;
        case QIP_AST_TYPE_VAR_DECL: rc = qip_ast_var_decl_validate(node, module); break;
        case QIP_AST_TYPE_TYPE_REF: rc = qip_ast_type_ref_validate(node, module); break;
        case QIP_AST_TYPE_VAR_REF: rc = qip_ast_var_ref_validate(node, module); break;
        case QIP_AST_TYPE_VAR_ASSIGN: rc = qip_ast_var_assign_validate(node, module); break;
        case QIP_AST_TYPE_FARG: rc = qip_ast_farg_validate(node, module); break;
        case QIP_AST_TYPE_IF_STMT: rc = qip_ast_if_stmt_validate(node, module); break;
        case QIP_AST_TYPE_FOR_EACH_STMT: rc = qip_ast_for_each_stmt_validate(node, module); break;
        case QIP_AST_TYPE_CLASS: rc = qip_ast_class_validate(node, module); break;
        case QIP_AST_TYPE_METHOD: rc = qip_ast_method_validate(node, module); break;
        case QIP_AST_TYPE_PROPERTY: rc = qip_ast_property_validate(node, module); break;
        case QIP_AST_TYPE_METADATA: rc = qip_ast_metadata_validate(node, module); break;
        case QIP_AST_TYPE_METADATA_ITEM: rc = qip_ast_metadata_item_validate(node, module); break;
        case QIP_AST_TYPE_SIZEOF: rc = qip_ast_sizeof_validate(node, module); break;
        case QIP_AST_TYPE_ALLOCA: rc = qip_ast_alloca_validate(node, module); break;
        default: rc = 0;
    }
    check(rc == 0, "Unable to validate node");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Debugging
//--------------------------------------

// Recursively dumps the abstract syntax tree to a string.
//
// node - The AST node to dump.
// ret  - A pointer to a bstring to concatenate to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node is required");
    check(ret != NULL, "String required");

    // Retrieve the depth of the node.
    int32_t depth;
    rc = qip_ast_node_get_depth(node, &depth);
    check(rc == 0, "Unable to retrieve depth for node");
    
    // Print line number.
    char line_no[7];
    snprintf(line_no, 7, "%05d ", (node->line_no >= 0 ? node->line_no : 0));
    check(bcatcstr(ret, line_no) == BSTR_OK, "Unable to insert line number");
    
    // Indent current node based on depth.
    int i;
    for(i=0; i<depth; i++) {
        check(bcatcstr(ret, "  ") == BSTR_OK, "Unable to indent dump");
    }

    // Delegate dump to AST nodes.
    switch(node->type) {
        case QIP_AST_TYPE_INT_LITERAL: rc = qip_ast_int_literal_dump(node, ret); break;
        case QIP_AST_TYPE_FLOAT_LITERAL: rc = qip_ast_float_literal_dump(node, ret); break;
        case QIP_AST_TYPE_BOOLEAN_LITERAL: rc = qip_ast_boolean_literal_dump(node, ret); break;
        case QIP_AST_TYPE_STRING_LITERAL: rc = qip_ast_string_literal_dump(node, ret); break;
        case QIP_AST_TYPE_NULL_LITERAL: rc = qip_ast_null_literal_dump(node, ret); break;
        case QIP_AST_TYPE_ARRAY_LITERAL: rc = qip_ast_array_literal_dump(node, ret); break;
        case QIP_AST_TYPE_BINARY_EXPR: rc = qip_ast_binary_expr_dump(node, ret); break;
        case QIP_AST_TYPE_VAR_DECL: rc = qip_ast_var_decl_dump(node, ret); break;
        case QIP_AST_TYPE_TYPE_REF: rc = qip_ast_type_ref_dump(node, ret); break;
        case QIP_AST_TYPE_VAR_REF: rc = qip_ast_var_ref_dump(node, ret); break;
        case QIP_AST_TYPE_VAR_ASSIGN: rc = qip_ast_var_assign_dump(node, ret); break;
        case QIP_AST_TYPE_FRETURN: rc = qip_ast_freturn_dump(node, ret); break;
        case QIP_AST_TYPE_FARG: rc = qip_ast_farg_dump(node, ret); break;
        case QIP_AST_TYPE_FUNCTION: rc = qip_ast_function_dump(node, ret); break;
        case QIP_AST_TYPE_BLOCK: rc = qip_ast_block_dump(node, ret); break;
        case QIP_AST_TYPE_IF_STMT: rc = qip_ast_if_stmt_dump(node, ret); break;
        case QIP_AST_TYPE_FOR_EACH_STMT: rc = qip_ast_for_each_stmt_dump(node, ret); break;
        case QIP_AST_TYPE_CLASS: rc = qip_ast_class_dump(node, ret); break;
        case QIP_AST_TYPE_TEMPLATE_VAR: rc = qip_ast_template_var_dump(node, ret); break;
        case QIP_AST_TYPE_METHOD: rc = qip_ast_method_dump(node, ret); break;
        case QIP_AST_TYPE_PROPERTY: rc = qip_ast_property_dump(node, ret); break;
        case QIP_AST_TYPE_MODULE: rc = qip_ast_module_dump(node, ret); break;
        case QIP_AST_TYPE_METADATA: rc = qip_ast_metadata_dump(node, ret); break;
        case QIP_AST_TYPE_METADATA_ITEM: rc = qip_ast_metadata_item_dump(node, ret); break;
        case QIP_AST_TYPE_SIZEOF: rc = qip_ast_sizeof_dump(node, ret); break;
        case QIP_AST_TYPE_ALLOCA: rc = qip_ast_alloca_dump(node, ret); break;
        default: {
            sentinel("Unable to dump AST node");
            break;
        }
    }
    check(rc == 0, "Unable to dump node");

    return 0;

error:
    return -1;
}

// Recursively dumps the abstract syntax tree to standard error.
//
// node - The AST node to dump.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_node_dump_stderr(qip_ast_node *node)
{
    int rc;
    check(node != NULL, "Node is required");

    // Dump to string.
    bstring str = bfromcstr("");
    rc = qip_ast_node_dump(node, str);
    check(rc == 0, "Unable to dump node");
    
    // Print to STDERR.
    fprintf(stderr, "%s\n", bdata(str));

    bdestroy(str);
    return 0;
    
error:
    bdestroy(str);
    return -1;
}