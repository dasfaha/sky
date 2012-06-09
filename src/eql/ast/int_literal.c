#include <stdlib.h>
#include <stdbool.h>
#include "../../dbg.h"

#include "int_literal.h"
#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a literal integer.
//
// value - The integer value.
// ret   - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_int_literal_create(int64_t value, struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_INT_LITERAL;
    node->int_literal.value = value;
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the literal integer AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_int_literal_codegen(struct eql_ast_node *node,
                                struct eql_module *module,
                                LLVMValueRef *value)
{
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);
    *value = LLVMConstInt(LLVMInt64TypeInContext(context), node->int_literal.value, true);
    return 0;
}
