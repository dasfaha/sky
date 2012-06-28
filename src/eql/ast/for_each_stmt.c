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

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for an "for each" statement.
//
// var_decl   - The variable declaration for the loop variable.
// enumerator - The source of the enumeration.
// block      - The block to execute for each item in the enumerator.
// ret        - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_for_each_stmt_create(eql_ast_node *var_decl,
                                 eql_ast_node *enumerator,
                                 eql_ast_node *block,
                                 eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_FOR_EACH_STMT;
    node->parent = NULL;

    // Assign variable declaration
    node->for_each_stmt.var_decl = var_decl;
    if(var_decl != NULL) {
        var_decl->parent = node;
    }

    // Assign enumerator.
    node->for_each_stmt.enumerator = enumerator;
    if(enumerator != NULL) {
        enumerator->parent = node;
    }

    // Assign block
    node->for_each_stmt.block = block;
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

// Frees an "for each" statement AST node from memory.
//
// node - The AST node to free.
void eql_ast_for_each_stmt_free(struct eql_ast_node *node)
{
    if(node->for_each_stmt.var_decl) eql_ast_node_free(node->for_each_stmt.var_decl);
    node->for_each_stmt.var_decl = NULL;

    if(node->for_each_stmt.enumerator) eql_ast_node_free(node->for_each_stmt.enumerator);
    node->for_each_stmt.enumerator = NULL;

    if(node->for_each_stmt.block) eql_ast_node_free(node->for_each_stmt.block);
    node->for_each_stmt.block = NULL;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the "for each" statement AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_for_each_stmt_codegen(eql_ast_node *node, eql_module *module,
                                  LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    check(node->for_each_stmt.var_decl != NULL, "For Each statement variable declaration required");
    check(node->for_each_stmt.enumerator != NULL, "For Each statement enumerator required");
    check(node->for_each_stmt.block != NULL, "For Each statement block required");

    // TODO

    *value = NULL;
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
int eql_ast_for_each_stmt_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    check(bcatcstr(ret, "<for-each-stmt>\n") == BSTR_OK, "Unable to append dump");

    // Recursively dump children
    if(node->for_each_stmt.var_decl != NULL) {
        rc = eql_ast_node_dump(node->for_each_stmt.var_decl, ret);
        check(rc == 0, "Unable to dump for each statement variable declaration");
    }

    if(node->for_each_stmt.enumerator != NULL) {
        rc = eql_ast_node_dump(node->for_each_stmt.enumerator, ret);
        check(rc == 0, "Unable to dump for each statement enumerator");
    }

    if(node->for_each_stmt.block != NULL) {
        rc = eql_ast_node_dump(node->for_each_stmt.block, ret);
        check(rc == 0, "Unable to dump for each statement block");
    }

    return 0;

error:
    return -1;
}
