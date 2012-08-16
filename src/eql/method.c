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

// Creates an AST node for a method.
//
// function - The function.
//
// Returns a method node.
eql_ast_node *eql_ast_method_create(eql_ast_access_e access,
                                    eql_ast_node *function)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_METHOD;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->method.access = access;
    node->method.function = function;
    if(function) function->parent = node;
    node->method.metadata_count = 0;
    node->method.metadatas = NULL;
    
    return node;

error:
    eql_ast_node_free(node);
    return NULL;
}

// Frees a method AST node from memory.
//
// node - The AST node to free.
void eql_ast_method_free(eql_ast_node *node)
{
    if(node->method.function) eql_ast_node_free(node->method.function);
    node->method.function = NULL;

    unsigned int i;
    for(i=0; i<node->method.metadata_count; i++) {
        eql_ast_node_free(node->method.metadatas[i]);
        node->method.metadatas[i] = NULL;
    }
    free(node->method.metadatas);
    node->method.metadata_count = 0;
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_copy(eql_ast_node *node, eql_ast_node **ret)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    eql_ast_node *clone = eql_ast_method_create(node->method.access, NULL);
    check_mem(clone);

    rc = eql_ast_node_copy(node->method.function, &clone->method.function);
    check(rc == 0, "Unable to copy function");
    if(clone->method.function) clone->method.function->parent = clone;
    
    // Copy metadatas.
    clone->method.metadata_count = node->method.metadata_count;
    clone->method.metadatas = calloc(clone->method.metadata_count, sizeof(*clone->method.metadatas));
    check_mem(clone->method.metadatas);
    for(i=0; i<clone->method.metadata_count; i++) {
        rc = eql_ast_node_copy(node->method.metadatas[i], &clone->method.metadatas[i]);
        check(rc == 0, "Unable to copy metadata");
        if(clone->method.metadatas[i]) clone->method.metadatas[i]->parent = clone;
    }

    *ret = clone;
    return 0;

error:
    eql_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Metadata Management
//--------------------------------------

// Adds a metadata tag to a method.
//
// node     - The node to add the metadata to.
// metadata - The metadata to add.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_add_metadata(eql_ast_node *node, eql_ast_node *metadata)
{
    // Validate.
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_METHOD, "Node type must be 'method'");
    check(metadata != NULL, "Metadata required");
    
    // Append metadata.
    node->method.metadata_count++;
    node->method.metadatas = realloc(node->method.metadatas, sizeof(eql_ast_node*) * node->method.metadata_count);
    check_mem(node->method.metadatas);
    node->method.metadatas[node->method.metadata_count-1] = metadata;
    
    // Link metadata.
    metadata->parent = node;
    
    return 0;

error:
    return -1;
}

// Adds a list of metatdata tags to a method.
//
// node            - The node.
// metadatas      - A list of metatdatas to add.
// metadata_count - The number of metatdatas to add.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_add_metadatas(eql_ast_node *node,
                                 eql_ast_node **metadatas,
                                 unsigned int metadata_count)
{
    // Validate.
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_METHOD, "Node type must be 'method'");
    check(metadatas != NULL || metadata_count == 0, "Metadata tags are required");

    // Add each metadata.
    unsigned int i;
    for(i=0; i<metadata_count; i++) {
        int rc = eql_ast_method_add_metadata(node, metadatas[i]);
        check(rc == 0, "Unable to add metadata to method");
    }
    
    return 0;

error:
    return -1;
}

// Retrieves the first metadata node with the given name. If no nodes are
// found then NULL is returned.
//
// node - The method node.
// name - The name of the metadata node to search for.
// ret  - A pointer to where the matching metadata node should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_get_metadata_node(eql_ast_node *node, bstring name,
                                     eql_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(name != NULL, "Name required");
    check(ret != NULL, "Return pointer required");

    // Initialize return value.
    *ret = NULL;

    // Loop over metadata and search for node.
    unsigned int i;
    for(i=0; i<node->method.metadata_count; i++) {
        if(biseq(node->method.metadatas[i]->metadata.name, name)) {
            *ret = node->method.metadatas[i];
            break;
        }
    }
    
    return 0;

error:
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the method AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_codegen(eql_ast_node *node, eql_module *module,
                           LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_METHOD, "Node type must be 'method'");
    check(node->method.function != NULL, "Method function required");
    check(module != NULL, "Module required");

    // Codegen metadata.
    unsigned int i;
    for(i=0; i<node->method.metadata_count; i++) {
        rc = eql_ast_node_codegen(node->method.metadatas[i], module, NULL);
        check(rc == 0, "Unable to codegen method metadata");
    }

    // Codegen function.
    if(node->method.function != NULL) {
        rc = eql_ast_node_codegen(node->method.function, module, value);
        check(rc == 0, "Unable to codegen method function");
    }
    
    return 0;

error:
    return -1;
}

// Recursively generates forward declarations.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_codegen_forward_decl(eql_ast_node *node,
                                        eql_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_METHOD, "Node type must be 'method'");
    check(node->method.function != NULL, "Method function required");
    check(module != NULL, "Module required");

    // Codegen metadata.
    unsigned int i;
    for(i=0; i<node->method.metadata_count; i++) {
        rc = eql_ast_node_codegen_forward_decl(node->method.metadatas[i], module);
        check(rc == 0, "Unable to codegen method metadata forward declaration");
    }

    // Codegen function.
    if(node->method.function) {
        rc = eql_ast_node_codegen_forward_decl(node->method.function, module);
        check(rc == 0, "Unable to codegen method forward declaration");
    }
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Preprocesses the node.
//
// node   - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_validate(eql_ast_node *node, eql_module *module)
{
    int rc;
    bstring msg = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Ease of use variables.
    eql_ast_node *class = (node->parent && node->parent->type == EQL_AST_TYPE_CLASS ? node->parent : NULL);
    bstring function_name = node->method.function->function.name;

    // Validate constructor.
    if(class != NULL && biseqcstr(function_name, "init")) {
        // Validate no return type on constructor.
        if(!biseqcstr(node->method.function->function.return_type->type_ref.name, "void")) {
            msg = bformat("Constructor on class '%s' cannot have a return type", bdata(class->class.name));
        }
        // Validate no arguments on constructor (except 'this').
        else if(node->method.function->function.arg_count != 1) {
            msg = bformat("Constructor on class '%s' cannot have any arguments", bdata(class->class.name));
        }
    }
    // Validate deconstructor.
    else if(biseqcstr(function_name, "destroy")) {
        // Validate no return type on deconstructor.
        if(!biseqcstr(node->method.function->function.return_type->type_ref.name, "void")) {
            msg = bformat("Deconstructor on class '%s' cannot have a return type", bdata(class->class.name));
        }
        // Validate no arguments on deconstructor (except 'this').
        else if(node->method.function->function.arg_count != 1) {
            msg = bformat("Deconstructor on class '%s' cannot have any arguments", bdata(class->class.name));
        }
    }

    // Validate metadata.
    unsigned int i;
    for(i=0; i<node->method.metadata_count; i++) {
        rc = eql_ast_node_validate(node->method.metadatas[i], module);
        check(rc == 0, "Unable to validate method metadata");
    }

    // Validate variable declaration
    rc = eql_ast_node_validate(node->method.function, module);
    check(rc == 0, "Unable to validate method function");
    
    // Add an error.
    if(msg != NULL) {
        rc = eql_module_add_error(module, node, msg);
        check(rc == 0, "Unable to add module error");
    }
    bdestroy(msg);
    return 0;

error:
    bdestroy(msg);
    return -1;
}


//--------------------------------------
// Misc
//--------------------------------------

// Generates the 'this' argument for the function the method is attached to.
// This is called after the method is attached to a class.
//
// node - The method AST node.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_generate_this_farg(eql_ast_node *node)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_METHOD, "Node type must be 'method'");
    check(node->parent != NULL, "Method parent required");
    check(node->parent->type == EQL_AST_TYPE_CLASS, "Node parent type must be 'class'");

    eql_ast_node *function = node->method.function;

    // Only generate 'this' arg if the function exists.
    if(function != NULL) {
        // Generate type ref from class.
        eql_ast_node *type_ref = NULL;
        rc = eql_ast_class_generate_type_ref(node->parent, &type_ref);
        check(rc == 0, "Unable to generate class type ref");
    
        // Create 'this' variable declaration.
        struct tagbstring THIS = bsStatic("this");
        eql_ast_node *var_decl = eql_ast_var_decl_create(type_ref, &THIS, NULL);
        var_decl->line_no = function->line_no;
        check_mem(var_decl);

        // Link to function argument.
        eql_ast_node *farg = eql_ast_farg_create(var_decl); check_mem(farg);
        farg->line_no = function->line_no;
    
        // Prepend argument to function.
        function->function.arg_count++;
        function->function.args = realloc(function->function.args, sizeof(eql_ast_node*) * function->function.arg_count);
        check_mem(function->function.args);
        memmove(function->function.args+1, function->function.args, sizeof(eql_ast_node*) * (function->function.arg_count-1));
        function->function.args[0] = farg;
        farg->parent = function;
    }

    return 0;
    
error:
    return -1;
}


//--------------------------------------
// Type refs
//--------------------------------------

// Computes a list of type refs used in the node.
//
// node      - The node to compute type refs for.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_get_type_refs(eql_ast_node *node,
                                 eql_ast_node ***type_refs,
                                 uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    if(node->method.function != NULL) {
        rc = eql_ast_node_get_type_refs(node->method.function, type_refs, count);
        check(rc == 0, "Unable to add method function type refs");
    }

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
int eql_ast_method_get_dependencies(eql_ast_node *node,
                                    bstring **dependencies,
                                    uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    if(node->method.function != NULL) {
        rc = eql_ast_node_get_dependencies(node->method.function, dependencies, count);
        check(rc == 0, "Unable to add method function dependencies");
    }

    return 0;
    
error:
    eql_ast_node_dependencies_free(dependencies, count);
    return -1;
}


//--------------------------------------
// Preprocess
//--------------------------------------

// Validates the AST node.
//
// node   - The node.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_preprocess(eql_ast_node *node, eql_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Preprocess metadata.
    unsigned int i;
    for(i=0; i<node->method.metadata_count; i++) {
        rc = eql_ast_node_preprocess(node->method.metadatas[i], module);
        check(rc == 0, "Unable to preprocess method metadata");
    }

    // Preprocess variable declaration
    rc = eql_ast_node_preprocess(node->method.function, module);
    check(rc == 0, "Unable to preprocess method function");

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
int eql_ast_method_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    check(bcatcstr(ret, "<method>\n") == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    unsigned int i;
    for(i=0; i<node->method.metadata_count; i++) {
        rc = eql_ast_node_dump(node->method.metadatas[i], ret);
        check(rc == 0, "Unable to dump method metadata");
    }
    if(node->method.function != NULL) {
        rc = eql_ast_node_dump(node->method.function, ret);
        check(rc == 0, "Unable to dump method function");
    }

    return 0;

error:
    return -1;
}
