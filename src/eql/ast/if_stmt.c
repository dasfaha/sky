#include <stdlib.h>
#include <stdbool.h>
#include "../../dbg.h"
#include "../../mem.h"

#include "node.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>

//==============================================================================
//
// Functions
//
//==============================================================================

// Creates an AST node for an "if" statement.
//
// condition - The conditional expression to evaluate.
// block     - The block to execute if the condition is true.
// ret       - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_if_stmt_create(eql_ast_node *condition, eql_ast_node *block,
                           eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_IF_STMT;
    node->parent = NULL;

    // Assign condition.
    node->if_stmt.condition = condition;
    if(condition != NULL) {
        condition->parent = node;
    }

    // Assign block.
    node->if_stmt.block = block;
    if(block != NULL) {
        block->parent = node;
    }

    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees an "if" statement AST node from memory.
//
// node - The AST node to free.
void eql_ast_if_stmt_free(struct eql_ast_node *node)
{
    if(node->if_stmt.condition) eql_ast_node_free(node->if_stmt.condition);
    node->if_stmt.condition = NULL;

    if(node->if_stmt.block) eql_ast_node_free(node->if_stmt.block);
    node->if_stmt.block = NULL;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the "if" statement AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_if_stmt_codegen(eql_ast_node *node, eql_module *module,
                            LLVMValueRef *value)
{
    //int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

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
int eql_ast_if_stmt_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    check(bcatcstr(ret, "<if-stmt>\n") == BSTR_OK, "Unable to append dump");

    // Recursively dump children
    rc = eql_ast_node_dump(node->if_stmt.condition, ret);
    check(rc == 0, "Unable to dump if statement condition");

    rc = eql_ast_node_dump(node->if_stmt.block, ret);
    check(rc == 0, "Unable to dump if statement block");

    return 0;

error:
    return -1;
}
