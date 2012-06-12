#include <stdlib.h>
#include "../../dbg.h"

#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a function return.
//
// value - The value to be returned.
// ret   - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_freturn_create(struct eql_ast_node *value, struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_FRETURN;
    node->freturn.value = value;
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
void eql_ast_freturn_free(struct eql_ast_node *node)
{
    if(node->freturn.value) eql_ast_node_free(node->freturn.value);
    node->freturn.value = NULL;
}


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_freturn_codegen(eql_ast_node *node, eql_module *module,
                            LLVMValueRef *value)
{
    check(node != NULL, "Node is required");
    check(node->type == EQL_AST_TYPE_FRETURN, "Node must be a function return");
    
    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Return value if specified.
    if(node->freturn.value) {
        LLVMValueRef return_value;
        int rc = eql_ast_node_codegen(node->freturn.value, module, &return_value);
        check(rc == 0, "Unable to codegen function return value");
        *value = LLVMBuildRet(builder, return_value);
        check(*value != NULL, "Unable to generate function return");
    }
    // Otherwise return void.
    else {
        *value = LLVMBuildRetVoid(builder);
        check(*value != NULL, "Unable to generate function return void");
    }
    
    return 0;

error:
    *value = NULL;
    return -1;
}
