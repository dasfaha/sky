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

// Creates an AST node for a literal boolean.
//
// value - The boolean value.
// ret   - A pointer to where the ast node will be returned.
//
// Returns a literal boolean node.
qip_ast_node *qip_ast_boolean_literal_create(bool value)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_BOOLEAN_LITERAL;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->boolean_literal.value = value;
    node->boolean_literal.type_ref = qip_ast_type_ref_create_cstr("Boolean");
    check_mem(node->boolean_literal.type_ref);
    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees the node.
//
// node - The node.
//
// Returns nothing.
void qip_ast_boolean_literal_free(qip_ast_node *node)
{
    if(node != NULL) {
        qip_ast_type_ref_free(node->boolean_literal.type_ref);
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_boolean_literal_copy(qip_ast_node *node, qip_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_boolean_literal_create(node->boolean_literal.value);
    check_mem(clone);
    
    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the literal boolean AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_boolean_literal_codegen(qip_ast_node *node,
                                    qip_module *module,
                                    LLVMValueRef *value)
{
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);
    *value = LLVMConstInt(LLVMInt1TypeInContext(context), node->boolean_literal.value, true);
    return 0;
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
int qip_ast_boolean_literal_get_type(qip_ast_node *node,
                                     qip_ast_node **type_ref)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_BOOLEAN_LITERAL, "Node type must be 'boolean literal'");
    
    *type_ref = node->boolean_literal.type_ref;
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
int qip_ast_boolean_literal_dump(qip_ast_node *node, bstring ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bformat("<boolean-literal value='%s'>\n", (node->boolean_literal.value ? "true" : "false"));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
