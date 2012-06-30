#include <stdlib.h>
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

// Creates an AST node for a variable declaration.
//
// type  - The type of variable that is being defined.
// name  - The name of the variable being defined.
// ret   - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_var_decl_create(bstring type, bstring name,
                            struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_VAR_DECL;
    node->parent = NULL;
    node->var_decl.type = bstrcpy(type);
    check_mem(node->var_decl.type);
    node->var_decl.name = bstrcpy(name);
    check_mem(node->var_decl.name);
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a variable declaration AST node from memory.
//
// node - The AST node to free.
void eql_ast_var_decl_free(struct eql_ast_node *node)
{
    if(node->var_decl.type) {
        bdestroy(node->var_decl.type);
    }
    node->var_decl.type = NULL;

    if(node->var_decl.name) {
        bdestroy(node->var_decl.name);
    }
    node->var_decl.name = NULL;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the variable declaration AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_var_decl_codegen(eql_ast_node *node, eql_module *module,
                             LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_VAR_DECL, "Node type expected to be 'variable declaration'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");
    check(node->var_decl.type != NULL, "Variable declaration type required");
    check(node->var_decl.name != NULL, "Variable declaration name required");
    
    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Save position;
    LLVMBasicBlockRef originalBlock = LLVMGetInsertBlock(builder);

    // If no allocas exist yet, position builder at the beginning of function.
    LLVMBasicBlockRef entryBlock = LLVMGetEntryBasicBlock(module->llvm_function);
    if(module->llvm_last_alloca == NULL) {
        LLVMPositionBuilder(builder, entryBlock, LLVMGetFirstInstruction(entryBlock));
    }
    // Otherwise position it after the last alloca in the function.
    else {
        LLVMPositionBuilder(builder, entryBlock, module->llvm_last_alloca);
    }
    
    // Find LLVM type.
    LLVMTypeRef type;
    rc = eql_module_get_type_ref(module, node->var_decl.type, NULL, &type);
    check(rc == 0 && type != NULL, "Unable to find LLVM type ref: %s", bdata(node->var_decl.type));

    // If this is a complex type and a function argument then wrap the type.
    if(node->parent != NULL &&
       node->parent->type == EQL_AST_TYPE_FARG &&
       eql_llvm_is_complex_type(type))
    {
        type = LLVMPointerType(type, 0);
    }

    // Add alloca.
    char *name = (node->parent->type == EQL_AST_TYPE_FARG ? "" : bdata(node->var_decl.name));
    *value = LLVMBuildAlloca(builder, type, name);
    module->llvm_last_alloca = *value;
    
    // Store variable location in the current scope.
    rc = eql_module_add_variable(module, node, *value);
    check(rc == 0, "Unable to add variable to scope: %s", bdata(node->var_decl.name));
    
    // Reposition builder at end of original block.
    LLVMPositionBuilderAtEnd(builder, originalBlock);

    return 0;

error:
    *value = NULL;
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
int eql_ast_var_decl_dump(eql_ast_node *node, bstring ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bformat("<var-decl type='%s' name='%s'>\n", bdatae(node->var_decl.type, ""), bdatae(node->var_decl.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
