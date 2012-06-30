#include <stdlib.h>
#include <stdbool.h>
#include "../../dbg.h"

#include "../llvm.h"
#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a struct member access.
//
// var_name    - The name of the parent variable.
// member_name - The name of the member of the parent to access.
// ret         - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_staccess_create(eql_ast_node *var_ref, bstring member_name,
                            eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_STACCESS;
    node->parent = NULL;

    node->staccess.member_name = bstrcpy(member_name);
    if(member_name != NULL) check_mem(node->staccess.member_name);

    node->staccess.var_ref = var_ref;
    if(var_ref) {
        var_ref->parent = node;
    }

    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a struct member access AST node from memory.
//
// node - The AST node to free.
void eql_ast_staccess_free(eql_ast_node *node)
{
    eql_ast_node_free(node->staccess.var_ref);
    
    if(node->staccess.member_name) bdestroy(node->staccess.member_name);
    node->staccess.member_name = NULL;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the struct member access AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_staccess_codegen(eql_ast_node *node, eql_module *module,
                             LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_STACCESS, "Node type expected to be 'struct member access'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Retrieve a pointer to the member.
    LLVMValueRef ptr = NULL;
    rc = eql_ast_node_get_var_pointer(node, module, &ptr);
    check(rc == 0 && ptr != NULL, "Unable to retrieve struct member pointer");

    // Create load instruction.
    *value = LLVMBuildLoad(builder, ptr, "");
    check(*value != NULL, "Unable to create load instruction");

    return 0;

error:
    *value = NULL;
    return -1;
}

// Retrieves the pointer to the member of a struct.
//
// node    - The node to retrieve the pointer for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM pointer value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_staccess_get_pointer(eql_ast_node *node, eql_module *module,
                                 LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_STACCESS, "Node type expected to be 'struct member access'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");
    check(node->staccess.var_ref != NULL, "Variable reference required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Find the struct reference.
    LLVMValueRef var_ref_pointer;
    rc = eql_ast_node_get_var_pointer(node->staccess.var_ref, module, &var_ref_pointer);
    check(rc == 0 && var_ref_pointer != NULL, "Unable to retrieve pointer to struct");

    // Retrieve the type of the variable reference.
    bstring var_ref_type_name = NULL;
    rc = eql_ast_node_get_type(node->staccess.var_ref, module, &var_ref_type_name);
    check(rc == 0 && var_ref_type_name != NULL, "Unable to determine struct type");

    // Retrieve the class AST for the variable reference.
    eql_ast_node *class_ast = NULL;
    rc = eql_module_get_type_ref(module, var_ref_type_name, &class_ast, NULL);
    check(rc == 0 && class_ast != NULL, "Unable to find class: %s", bdata(var_ref_type_name));

    // Determine the property index for the member.
    unsigned int property_index = 0;
    rc = eql_ast_class_get_property_index(class_ast, node->staccess.member_name, &property_index);
    check(rc == 0, "Unable to find property '%s' on class '%s'", bdata(node->staccess.member_name), bdata(var_ref_type_name));

    // If this is a complex type from a function argument then dereference first.
    if(eql_llvm_is_double_pointer_type(LLVMTypeOf(var_ref_pointer))) {
        var_ref_pointer = LLVMBuildLoad(builder, var_ref_pointer, "");
    }

    // Build GEP instruction.
    *value = LLVMBuildStructGEP(builder, var_ref_pointer, property_index, "");
    check(*value != NULL, "Unable to build GEP instruction");

    return 0;

error:
    *value = NULL;
    return -1;
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node   - The AST node to determine the type for.
// module - The compilation unit this node is a part of.
// type   - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_staccess_get_type(eql_ast_node *node, eql_module *module,
                              bstring *type)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_STACCESS, "Node type must be 'struct member access'");

    // Retrieve the type of the variable reference.
    bstring var_ref_type_name = NULL;
    rc = eql_ast_node_get_type(node->staccess.var_ref, module, &var_ref_type_name);
    check(rc == 0 && var_ref_type_name != NULL, "Unable to determine struct type");

    // Retrieve the class AST for the variable reference.
    eql_ast_node *class_ast = NULL;
    rc = eql_module_get_type_ref(module, var_ref_type_name, &class_ast, NULL);
    check(rc == 0 && class_ast != NULL, "Unable to find class: %s", bdata(var_ref_type_name));

    // Find the property for the member.
    eql_ast_node *property = NULL;
    rc = eql_ast_class_get_property(class_ast, node->staccess.member_name, &property);
    check(rc == 0 || property == NULL, "Unable to find property '%s' on class '%s'", bdata(node->staccess.member_name), bdata(var_ref_type_name));

    // Return the property type name.
    *type = property->property.var_decl->var_decl.type;

    return 0;
    
error:
    *type = NULL;
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
int eql_ast_staccess_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    bstring str = bformat("<staccess member-name='%s'>\n", bdatae(node->staccess.member_name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    if(node->staccess.var_ref != NULL) {
        rc = eql_ast_node_dump(node->staccess.var_ref, ret);
        check(rc == 0, "Unable to dump variable reference");
    }

    return 0;

error:
    if(str) bdestroy(str);
    return -1;
}
