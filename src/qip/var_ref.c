#include <stdlib.h>
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

// Creates an AST node for a variable reference.
//
// name - The name of the variable value.
// ret  - A pointer to where the ast node will be returned.
//
// Returns a variable reference node.
qip_ast_node *qip_ast_var_ref_create(bstring name)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_VAR_REF;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->var_ref.name = bstrcpy(name);
    check_mem(node->var_ref.name);
    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a variable reference AST node from memory.
//
// node - The AST node to free.
void qip_ast_var_ref_free(qip_ast_node *node)
{
    if(node->var_ref.name) {
        bdestroy(node->var_ref.name);
    }
    node->var_ref.name = NULL;
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_copy(qip_ast_node *node, qip_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_var_ref_create(node->var_ref.name);
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

// Recursively generates LLVM code for the variable reference AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_codegen(qip_ast_node *node, qip_module *module,
                            LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_REF, "Node type expected to be 'variable reference'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Find the variable declaration.
    LLVMValueRef ptr = NULL;
    rc = qip_ast_node_get_var_pointer(node, module, &ptr);
    check(rc == 0 && ptr != NULL, "Unable to retrieve variable pointer");

    // Create load instruction.
    *value = LLVMBuildLoad(builder, ptr, "");
    check(*value != NULL, "Unable to create load instruction");

    return 0;

error:
    *value = NULL;
    return -1;
}

// Retrieves the pointer to the variable reference.
//
// node    - The node to retrieve the pointer for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM pointer value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_get_pointer(qip_ast_node *node, qip_module *module,
                                LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_REF, "Node type expected to be 'variable reference'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");

    // Find the variable declaration.
    qip_ast_node *var_decl;
    LLVMValueRef var_decl_value;
    rc = qip_module_get_variable(module, node->var_ref.name, &var_decl, &var_decl_value);
    check(rc == 0, "Unable to retrieve variable declaration: %s", bdata(node->var_ref.name));
    check(var_decl != NULL, "No variable declaration found: %s", bdata(node->var_ref.name));
    check(var_decl_value != NULL, "No LLVM value for variable declaration: %s", bdata(node->var_ref.name));

    // Return the pointer.
    *value = var_decl_value;

    return 0;

error:
    *value = NULL;
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Preprocesses the node.
//
// node   - The node.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_preprocess(qip_ast_node *node, qip_module *module)
{
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node     - The AST node to determine the type for.
// type_ref - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_get_type(qip_ast_node *node, qip_ast_node **type_ref)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_REF, "Node type must be 'var ref'");

    // Search up the parent hierarchy to find variable declaration.
    qip_ast_node *var_decl = NULL;
    qip_ast_node *parent = node->parent;
    while(parent != NULL) {
        int rc = qip_ast_node_get_var_decl(parent, node->var_ref.name, &var_decl);
        check(rc == 0, "Unable to search node for variable declarations");
        
        // If a declaration was found then return its type.
        if(var_decl != NULL) {
            *type_ref = var_decl->var_decl.type;
            return 0;
        }
        
        parent = parent->parent;
    }

    sentinel("Unable to find variable declaration: %s", bdata(node->var_ref.name));

error:
    *type_ref = NULL;
    return -1;
}



//--------------------------------------
// Validation
//--------------------------------------

// Validates the AST node.
//
// node - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_validate(qip_ast_node *node, qip_module *module)
{
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    return 0;

error:
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
int qip_ast_var_ref_dump(qip_ast_node *node, bstring ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bformat("<var-ref name='%s'>\n", bdatae(node->var_ref.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
