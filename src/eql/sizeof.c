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

// Creates an AST node for a sizeof invocation.
//
// name - The name of the variable value.
// ret  - A pointer to where the ast node will be returned.
//
// Returns a variable reference node.
eql_ast_node *eql_ast_sizeof_create(eql_ast_node *type_ref)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_SIZEOF;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;

    node->szof.type_ref = type_ref;
    if(type_ref != NULL) type_ref->parent = node;

    node->szof.return_type_ref = eql_ast_type_ref_create_cstr("Int");
    check_mem(node->szof.return_type_ref);
    node->szof.return_type_ref->parent = node;

    return node;

error:
    eql_ast_node_free(node);
    return NULL;
}

// Frees a variable reference AST node from memory.
//
// node - The AST node to free.
void eql_ast_sizeof_free(eql_ast_node *node)
{
    if(node != NULL) {
        eql_ast_node_free(node->szof.type_ref);
        node->szof.type_ref = NULL;
        
        eql_ast_node_free(node->szof.return_type_ref);
        node->szof.return_type_ref = NULL;
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_sizeof_copy(eql_ast_node *node, eql_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    eql_ast_node *clone = eql_ast_sizeof_create(NULL);
    check_mem(clone);

    rc = eql_ast_node_copy(node->szof.type_ref, &clone->szof.type_ref);
    check(rc == 0, "Unable to copy type");
    if(clone->szof.type_ref) clone->szof.type_ref->parent = clone;
    
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

// Recursively generates LLVM code for the variable reference AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_sizeof_codegen(eql_ast_node *node, eql_module *module,
                           LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_SIZEOF, "Node type expected to be 'sizeof'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");

    LLVMBuilderRef builder = module->compiler->llvm_builder;
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);

    // Retrieve LLVM type.
    LLVMTypeRef type;
    bstring type_name = node->szof.type_ref->type_ref.name;
    rc = eql_module_get_type_ref(module, type_name, NULL, &type);
    check(rc == 0 && type != NULL, "Unable to find LLVM type ref: %s", bdata(type_name));

    // Create a null pointer.
    LLVMValueRef null_ptr = LLVMConstPointerNull(LLVMPointerType(type, 0));

    // Create a pointer to element 1 of a null array.
    LLVMValueRef indices[1];
    indices[0] = LLVMConstInt(LLVMInt64TypeInContext(context), 1, true);
    LLVMValueRef ptr = LLVMBuildGEP(builder, null_ptr, indices, 1, "");
    check(ptr != NULL, "Unable to create null pointer offset");
    
    // Cast pointer to an int and return.
    *value = LLVMBuildPtrToInt(builder, ptr, LLVMInt64TypeInContext(context), "");
    check((*value) != NULL, "Unable cast pointer to int");
    
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
int eql_ast_sizeof_preprocess(eql_ast_node *node, eql_module *module)
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
// node - The AST node to determine the type for.
// ret  - A pointer to where the type should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_sizeof_get_type(eql_ast_node *node, eql_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_SIZEOF, "Node type must be 'sizeof'");

    *ret = node->szof.return_type_ref;
    return 0;

error:
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Type refs
//--------------------------------------

// Computes a list of type references used by a node.
//
// node      - The node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_sizeof_get_type_refs(eql_ast_node *node,
                                 eql_ast_node ***type_refs,
                                 uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Add type.
    rc = eql_ast_node_get_type_refs(node->szof.type_ref, type_refs, count);
    check(rc == 0, "Unable to add sizeof type ref");

    return 0;
    
error:
    eql_ast_node_type_refs_free(type_refs, count);
    return -1;
}


//--------------------------------------
// Dependencies
//--------------------------------------

// Computes a list of class names that this AST node depends on.
//
// node         - The node to compute dependencies for.
// dependencies - A pointer to an array of dependencies.
// count        - A pointer to where the number of dependencies is stored.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_sizeof_get_dependencies(eql_ast_node *node,
                                    bstring **dependencies,
                                    uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Type ref.
    rc = eql_ast_node_get_dependencies(node->szof.type_ref, dependencies, count);
    check(rc == 0, "Unable to add sizeof dependencies");

    return 0;
    
error:
    eql_ast_node_dependencies_free(dependencies, count);
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
int eql_ast_sizeof_validate(eql_ast_node *node, eql_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Type ref.
    rc = eql_ast_node_validate(node->szof.type_ref, module);
    check(rc == 0, "Unable to validate sizeof type ref");
    
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
int eql_ast_sizeof_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bfromcstr("<sizeof>\n"); check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    if(node->szof.type_ref != NULL) {
        rc = eql_ast_node_dump(node->szof.type_ref, ret);
        check(rc == 0, "Unable to dump type ref");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
