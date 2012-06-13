#include <stdlib.h>
#include "../../dbg.h"
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
void eql_ast_node_free(eql_ast_node *node)
{
    if(!node) return;
    
    // Recursively free dependent data.
    switch(node->type) {
        case EQL_AST_TYPE_INT_LITERAL: break;
        case EQL_AST_TYPE_FLOAT_LITERAL: break;
        case EQL_AST_TYPE_BINARY_EXPR: {
            eql_ast_binary_expr_free(node);
            break;
        }
        case EQL_AST_TYPE_VAR_REF: {
            eql_ast_var_ref_free(node);
            break;
        }
        case EQL_AST_TYPE_VAR_DECL: {
            eql_ast_var_decl_free(node);
            break;
        }
        case EQL_AST_TYPE_FARG: {
            eql_ast_farg_free(node);
            break;
        }
        case EQL_AST_TYPE_FRETURN: {
            eql_ast_freturn_free(node);
            break;
        }
        case EQL_AST_TYPE_FUNCTION: {
            eql_ast_function_free(node);
            break;
        }
        case EQL_AST_TYPE_FCALL: {
            eql_ast_fcall_free(node);
            break;
        }
        case EQL_AST_TYPE_BLOCK: {
            eql_ast_block_free(node);
            break;
        }
        case EQL_AST_TYPE_METHOD: {
            eql_ast_method_free(node);
            break;
        }
        case EQL_AST_TYPE_PROPERTY: {
            eql_ast_property_free(node);
            break;
        }
        case EQL_AST_TYPE_CLASS: {
            eql_ast_class_free(node);
            break;
        }
        case EQL_AST_TYPE_MODULE: {
            eql_ast_module_free(node);
            break;
        }
        case EQL_AST_TYPE_METADATA: {
            eql_ast_metadata_free(node);
            break;
        }
        case EQL_AST_TYPE_METADATA_ITEM: {
            eql_ast_metadata_item_free(node);
            break;
        }
    }
    node->parent = NULL;
    
    free(node);
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
int eql_ast_node_codegen(eql_ast_node *node, eql_module *module,
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
        case EQL_AST_TYPE_INT_LITERAL: {
            rc = eql_ast_int_literal_codegen(node, module, value);
            check(rc == 0, "Unable to codegen literal integer");
            break;
        }
        case EQL_AST_TYPE_FLOAT_LITERAL: {
            rc = eql_ast_float_literal_codegen(node, module, value);
            check(rc == 0, "Unable to codegen literal float");
            break;
        }
        case EQL_AST_TYPE_BINARY_EXPR: {
            rc = eql_ast_binary_expr_codegen(node, module, value);
            check(rc == 0, "Unable to codegen binary expression");
            break;
        }
        case EQL_AST_TYPE_FRETURN: {
            rc = eql_ast_freturn_codegen(node, module, value);
            check(rc == 0, "Unable to codegen function return");
            break;
        }
        case EQL_AST_TYPE_FUNCTION: {
            rc = eql_ast_function_codegen(node, module, value);
            check(rc == 0, "Unable to codegen function");
            break;
        }
        case EQL_AST_TYPE_BLOCK: {
            rc = eql_ast_block_codegen(node, module, value);
            check(rc == 0, "Unable to codegen block");
            break;
        }
    }
    
    return 0;

error:
    *value = NULL;
    return -1;
}



//--------------------------------------
// types
//--------------------------------------

// Recursively determines the type name of a node.
//
// node - The node to determine the type for.
// type - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_node_get_type(eql_ast_node *node, bstring *type)
{
    check(node != NULL, "Node required");

    // Delegate to each type.
    switch(node->type) {
        case EQL_AST_TYPE_INT_LITERAL: {
            eql_ast_int_literal_get_type(node, type);
            break;
        }
        case EQL_AST_TYPE_FLOAT_LITERAL: {
            eql_ast_float_literal_get_type(node, type);
            break;
        }
        case EQL_AST_TYPE_BINARY_EXPR: {
            eql_ast_binary_expr_get_type(node, type);
            break;
        }
        /*
        case EQL_AST_TYPE_VAR_REF: {
            eql_ast_var_ref_get_type(node, type);
            break;
        }
        case EQL_AST_TYPE_FCALL: {
            eql_ast_fcall_get_type(node, type);
            break;
        }
        */
        default:
        {
            sentinel("AST node does not have a type");
            break;
        }
    }
    
    return 0;

error:
    *type = NULL;
    return -1;
}
