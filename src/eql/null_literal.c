#include <stdlib.h>
#include <stdbool.h>
#include "dbg.h"

#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a literal null.
//
// ret   - A pointer to where the ast node will be returned.
//
// Returns an integer null node.
eql_ast_node *eql_ast_null_literal_create()
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_NULL_LITERAL;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->null_literal.type_ref = eql_ast_type_ref_create_cstr("Ref");
    check_mem(node->null_literal.type_ref);
    return node;

error:
    eql_ast_node_free(node);
    return NULL;
}

// Frees the node.
//
// node - The node.
//
// Returns nothing.
void eql_ast_null_literal_free(eql_ast_node *node)
{
    if(node != NULL) {
        eql_ast_type_ref_free(node->null_literal.type_ref);
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_null_literal_copy(eql_ast_node *node, eql_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    eql_ast_node *clone = eql_ast_null_literal_create();
    check_mem(clone);
    
    *ret = clone;
    return 0;

error:
    eql_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the literal null AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_null_literal_codegen(eql_ast_node *node,
                                 eql_module *module,
                                 LLVMValueRef *value)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_NULL_LITERAL, "Node type must be 'null literal'");
    check(module != NULL, "Module required");
    
    // We need to generate nulls with a type so this is all contained within
    // the var_assign codegen.
    sentinel("Illegal null literal codegen");

error:
    *value = NULL;
    return -1;
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node - The AST node to determine the type for.
// type - A pointer to where the type should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_null_literal_get_type(eql_ast_node *node,
                                  eql_ast_node **type_ref)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_NULL_LITERAL, "Node type must be 'null literal'");
    
    *type_ref = node->null_literal.type_ref;
    return 0;

error:
    *type_ref = NULL;
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
int eql_ast_null_literal_dump(eql_ast_node *node, bstring ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bfromcstr("<null-literal>\n");
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    bdestroy(str);
    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
