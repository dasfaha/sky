#include <stdlib.h>
#include "dbg.h"

#include "llvm.h"
#include "util.h"
#include "node.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a variable declaration.
//
// type  - The type of variable that is being defined.
// name  - The name of the variable being defined.
// ret   - A pointer to where the ast node will be returned.
//
// Returns a variable declaration node.
qip_ast_node *qip_ast_var_decl_create(qip_ast_node *type, bstring name,
                                      qip_ast_node *initial_value)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_VAR_DECL;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;

    node->var_decl.type = type;
    if(type != NULL) type->parent = node;

    node->var_decl.name = bstrcpy(name);
    if(name != NULL) check_mem(node->var_decl.name);

    node->var_decl.initial_value = initial_value;
    if(initial_value != NULL) {
        initial_value->parent = node;
    }

    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a variable declaration AST node from memory.
//
// node - The AST node to free.
void qip_ast_var_decl_free(qip_ast_node *node)
{
    if(node != NULL) {
        qip_ast_node_free(node->var_decl.type);
        node->var_decl.type = NULL;

        if(node->var_decl.name) bdestroy(node->var_decl.name);
        node->var_decl.name = NULL;

        if(node->var_decl.initial_value) qip_ast_node_free(node->var_decl.initial_value);
        node->var_decl.initial_value = NULL;
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_decl_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_var_decl_create(NULL, node->var_decl.name, NULL);
    check_mem(clone);

    rc = qip_ast_node_copy(node->var_decl.type, &clone->var_decl.type);
    check(rc == 0, "Unable to copy type");
    if(clone->var_decl.type) clone->var_decl.type->parent = clone;
    
    rc = qip_ast_node_copy(node->var_decl.initial_value, &clone->var_decl.initial_value);
    check(rc == 0, "Unable to copy initial value");
    if(clone->var_decl.initial_value) clone->var_decl.initial_value->parent = clone;
    
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

// Recursively generates LLVM code for the variable declaration AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_decl_codegen(qip_ast_node *node, qip_module *module,
                             LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_DECL, "Node type expected to be 'variable declaration'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");
    check(node->var_decl.type != NULL, "Variable declaration type required");
    check(node->var_decl.name != NULL, "Variable declaration name required");
    
    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Store farg or property parents if they exist.
    qip_ast_node *farg = (node->parent != NULL && node->parent->type == QIP_AST_TYPE_FARG ? node->parent : NULL);
    qip_ast_node *property = (node->parent != NULL && node->parent->type == QIP_AST_TYPE_PROPERTY ? node->parent : NULL);

    // Save position;
    LLVMBasicBlockRef originalBlock = LLVMGetInsertBlock(builder);

    // If no allocas exist yet, position builder at the beginning of function.
    LLVMBasicBlockRef entryBlock = LLVMGetEntryBasicBlock(module->llvm_function);
    if(module->llvm_last_alloca == NULL) {
        LLVMPositionBuilder(builder, entryBlock, LLVMGetFirstInstruction(entryBlock));
    }
    // Otherwise position it after the last alloca in the function.
    else {
        LLVMPositionBuilder(builder, entryBlock, module->llvm_last_alloca);
    }
    
    // Retrieve type name.
    bstring type_name = NULL;
    rc = qip_ast_type_ref_get_full_name(node->var_decl.type, &type_name);
    check(rc == 0, "Unable to retrieve full type name");
    
    // Find LLVM type.
    LLVMTypeRef type;
    rc = qip_module_get_type_ref(module, type_name, NULL, &type);
    check(rc == 0 && type != NULL, "Unable to find LLVM type ref: %s", bdata(type_name));
    bool is_complex_type = qip_llvm_is_complex_type(type);

    // Create a function argument allocation.
    LLVMValueRef value_alloca = NULL;
    if(farg != NULL) {
        // If the argument is complex then create a pointer allocation.
        if(is_complex_type) {
            *value = LLVMBuildAlloca(builder, LLVMPointerType(type, 0), "");
        }
        // If the argument is simple then pass it by value.
        else {
            *value = LLVMBuildAlloca(builder, type, "");
        }
    }
    // Create a stack variable allocation.
    else {
        // Allocate space for the value of the variable.
        *value = LLVMBuildAlloca(builder, type, bdata(node->var_decl.name));

        // If this is a complex type then create an allocation for the
        // pointer to the allocation. All objects are pointers!
        if(is_complex_type) {
            value_alloca = *value;
            *value = LLVMBuildAlloca(builder, LLVMPointerType(type, 0), "");
        }
    }

    // Store variable location in the current scope.
    rc = qip_module_add_variable(module, node, *value);
    check(rc == 0, "Unable to add variable to scope: %s", bdata(node->var_decl.name));
    
    // Reposition builder at end of original block.
    LLVMPositionBuilderAtEnd(builder, originalBlock);

    // Copy the address of the complex stack var to its pointer alloca now
    // that we have moved to the end of the block.
    if(farg == NULL && is_complex_type) {
        LLVMBuildStore(builder, value_alloca, *value);
    }

    // Generate call to constructor if this is not a built-in.
    if(property == NULL && farg == NULL && !qip_is_builtin_type_name(type_name)) {
        bstring constructor_name = bformat("%s.init", bdata(type_name), bdata(type_name));
        check_mem(constructor_name);
        
        // Invoke constructor.
        LLVMValueRef args[1];
        args[0] = LLVMBuildLoad(builder, *value, "");
        LLVMValueRef func = LLVMGetNamedFunction(module->llvm_module, bdata(constructor_name));
        LLVMBuildCall(builder, func, args, 1, "");
    }

    // Generate initial value.
    LLVMValueRef initial_value = NULL;
    if(node->var_decl.initial_value != NULL) {
        rc = qip_ast_node_codegen(node->var_decl.initial_value, module, &initial_value);
        check(rc == 0, "Unable to codegen variable declaration initial value");
    }

    // Create a store instruction if there is an initial value.
    if(initial_value != NULL) {
        LLVMBuildStore(builder, initial_value, *value);
    }

    bdestroy(type_name);
    return 0;

error:
    bdestroy(type_name);
    *value = NULL;
    return -1;
}

// Generates a call to the deconstructor for a variable declaration. This is
// called by the containing block to destroy all variable that are instances
// of classes with a deconstructor.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_decl_codegen_destroy(qip_ast_node *node, qip_module *module,
                                     LLVMValueRef *value)
{
    int rc;
    bstring type_name = NULL;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_DECL, "Node type expected to be 'variable declaration'");
    check(module != NULL, "Module required");
    
    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Only try to generate if this not a built-in type.
    bstring deconstructor_qualified_name = NULL;
    if(!qip_is_builtin_type(node->var_decl.type)) {
        // Find the class.
        qip_ast_node *class = NULL;
        rc = qip_module_get_ast_class(module, node->var_decl.type->type_ref.name, &class);
        check(rc == 0, "Unable to retrieve class");
        check(class != NULL, "Unable to find class: %s", bdata(type_name));
    
        // Retrieve deconstructor.
        qip_ast_node *method = NULL;
        struct tagbstring deconstructor_name = bsStatic("destroy");
        rc = qip_ast_class_get_method(class, &deconstructor_name, &method);
        check(rc == 0, "Unable to retrieve deconstructor");
        
        // If there is a deconstructor then call it.
        if(method != NULL) {
            // Retrieve alloca.
            LLVMValueRef ptr;
            rc = qip_module_get_variable(module, node->var_decl.name, NULL, &ptr);
            check(rc == 0, "Unable to retrieve variable pointer");
            check(ptr != NULL, "No LLVM value for variable declaration");

            // Retrieve fully qualified name.
            deconstructor_qualified_name = bformat("%s.destroy", bdata(class->class.name));
            
            // Invoke deconstructor.
            LLVMValueRef args[1];
            args[0] = LLVMBuildLoad(builder, ptr, "");
            LLVMValueRef func = LLVMGetNamedFunction(module->llvm_module, bdata(deconstructor_qualified_name));
            check(func, "Deconstructor not found");
            LLVMBuildCall(builder, func, args, 1, "");
        }
    }

    bdestroy(deconstructor_qualified_name);
    return 0;

error:
    bdestroy(deconstructor_qualified_name);
    *value = NULL;
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Preprocess the node.
//
// node   - The node.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_decl_preprocess(qip_ast_node *node, qip_module *module)
{
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    return 0;

error:
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
int qip_ast_var_decl_get_type_refs(qip_ast_node *node,
                                   qip_ast_node ***type_refs,
                                   uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Add type.
    rc = qip_ast_node_get_type_refs(node->var_decl.type, type_refs, count);
    check(rc == 0, "Unable to add variable declaration type refs");

    return 0;
    
error:
    qip_ast_node_type_refs_free(type_refs, count);
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
int qip_ast_var_decl_get_dependencies(qip_ast_node *node,
                                      bstring **dependencies,
                                      uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Type ref.
    rc = qip_ast_node_get_dependencies(node->var_decl.type, dependencies, count);
    check(rc == 0, "Unable to add variable declaration dependencies");

    return 0;
    
error:
    qip_ast_node_dependencies_free(dependencies, count);
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
int qip_ast_var_decl_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    bstring msg = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    qip_ast_node *property = (node->parent != NULL && node->parent->type == QIP_AST_TYPE_PROPERTY ? node->parent : NULL);
    qip_ast_node *farg = (node->parent != NULL && node->parent->type == QIP_AST_TYPE_FARG ? node->parent : NULL);

    // Validate that the type is not a Ref unless this is a property or function argument.
    if(property == NULL && farg == NULL && biseqcstr(node->var_decl.type->type_ref.name, "Ref")) {
        msg = bformat("Ref type is only allowed on properties and function arguments");
    }
    
    // If we have an error message then add it.
    if(msg != NULL) {
        rc = qip_module_add_error(module, node, msg);
        check(rc == 0, "Unable to add module error");
    }

    bdestroy(msg);
    return 0;

error:
    bdestroy(msg);
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
int qip_ast_var_decl_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bformat("<var-decl name='%s'>\n", bdatae(node->var_decl.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    if(node->var_decl.type != NULL) {
        rc = qip_ast_node_dump(node->var_decl.type, ret);
        check(rc == 0, "Unable to dump type");
    }

    if(node->var_decl.initial_value != NULL) {
        rc = qip_ast_node_dump(node->var_decl.initial_value, ret);
        check(rc == 0, "Unable to dump initial value");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
