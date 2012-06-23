#include <stdlib.h>
#include <stdbool.h>
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

// Creates an AST node for a literal integer.
//
// value - The integer value.
// ret   - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_int_literal_create(int64_t value, eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_INT_LITERAL;
    node->parent = NULL;
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
int eql_ast_int_literal_codegen(eql_ast_node *node,
                                eql_module *module,
                                LLVMValueRef *value)
{
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);
    *value = LLVMConstInt(LLVMInt64TypeInContext(context), node->int_literal.value, true);
    return 0;
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node - The AST node to determine the type for.
// type - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_int_literal_get_type(eql_ast_node *node, bstring *type)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_INT_LITERAL, "Node type must be 'int literal'");
    
    *type = bfromcstr("Int");
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
int eql_ast_int_literal_dump(eql_ast_node *node, bstring ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bformat("<int-literal value='%lld'>\n", node->int_literal.value);
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
