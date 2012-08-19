#include <stdlib.h>
#include <stdbool.h>
#include "dbg.h"

#include "llvm.h"
#include "node.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int qip_ast_function_codegen_prototype(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

int qip_ast_function_generate_external_call(qip_ast_node *node,
    qip_module *module, LLVMBasicBlockRef block);


//==============================================================================
//
// Functions
//
//==============================================================================

// Creates an AST node for a function.
//
// name        - The name of the function.
// return_type - The data type that the function returns.
// args        - The arguments of the function.
// arg_count   - The number of arguments the function has.
// body        - The contents of the function.
// ret         - A pointer to where the ast node will be returned.
//
// Returns a function node.
qip_ast_node *qip_ast_function_create(bstring name, qip_ast_node *return_type,
                            struct qip_ast_node **args, unsigned int arg_count,
                            struct qip_ast_node *body)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_FUNCTION;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->function.name = bstrcpy(name);
    if(name) check_mem(node->function.name);
    
    node->function.return_type = return_type;
    if(return_type != NULL) return_type->parent = node;

    // Copy arguments.
    if(arg_count > 0) {
        size_t sz = sizeof(qip_ast_node*) * arg_count;
        node->function.args = malloc(sz);
        check_mem(node->function.args);
        
        unsigned int i;
        for(i=0; i<arg_count; i++) {
            node->function.args[i] = args[i];
            args[i]->parent = node;
        }
    }
    else {
        node->function.args = NULL;
    }
    node->function.arg_count = arg_count;
    
    // Assign function body.
    node->function.body = body;
    if(body != NULL) {
        body->parent = node;
        body->block.name = NULL;
    }

    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a function AST node from memory.
//
// node - The AST node to free.
void qip_ast_function_free(struct qip_ast_node *node)
{
    if(node->function.name) bdestroy(node->function.name);
    node->function.name = NULL;

    qip_ast_node_free(node->function.return_type);
    node->function.return_type = NULL;
    
    if(node->function.arg_count > 0) {
        unsigned int i;
        for(i=0; i<node->function.arg_count; i++) {
            qip_ast_node_free(node->function.args[i]);
            node->function.args[i] = NULL;
        }
        free(node->function.args);
        node->function.arg_count = 0;
    }

    if(node->function.body) qip_ast_node_free(node->function.body);
    node->function.body = NULL;
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_function_create(node->function.name, NULL, NULL, 0, NULL);
    check_mem(clone);

    rc = qip_ast_node_copy(node->function.return_type, &clone->function.return_type);
    check(rc == 0, "Unable to copy return type");
    if(clone->function.return_type) clone->function.return_type->parent = clone;
    
    // Copy args.
    clone->function.arg_count = node->function.arg_count;
    clone->function.args = calloc(clone->function.arg_count, sizeof(*clone->function.args));
    check_mem(clone->function.args);
    for(i=0; i<clone->function.arg_count; i++) {
        rc = qip_ast_node_copy(node->function.args[i], &clone->function.args[i]);
        check(rc == 0, "Unable to copy arg");
        if(clone->function.args[i]) clone->function.args[i]->parent = clone;
    }

    rc = qip_ast_node_copy(node->function.body, &clone->function.body);
    check(rc == 0, "Unable to copy body");
    if(clone->function.body) clone->function.body->parent = clone;
    
    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Argument Management
//--------------------------------------

// Adds an argument to a function.
//
// node - The function node.
// farg - The argument to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_add_arg(qip_ast_node *node,
                             qip_ast_node *farg)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");
    check(farg != NULL, "Argument is required");
    
    // Append argument to function.
    node->function.arg_count++;
    node->function.args = realloc(node->function.args, sizeof(qip_ast_node*) * node->function.arg_count);
    check_mem(node->function.args);
    node->function.args[node->function.arg_count-1] = farg;
    
    // Link property to function.
    farg->parent = node;
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the function AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_codegen(qip_ast_node *node, qip_module *module,
                             LLVMValueRef *value)
{
    int rc;

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Create function prototype.
    LLVMValueRef func = NULL;
    rc = qip_ast_function_codegen_prototype(node, module, &func);
    check(rc == 0, "Unable to generate function prototype");
    
    // Store the current function on the module.
    module->llvm_function = func;
    module->llvm_last_alloca = NULL;
    rc = qip_module_push_scope(module, node);
    check(rc == 0, "Unable to add function scope");

    // Grab the External metadata node if there is one.
    qip_ast_node *external_metadata_node = NULL;
    rc = qip_ast_function_get_external_metadata_node(node, &external_metadata_node);
    check(rc == 0, "Unable to retrieve external metadata node");
    
    // Generate basic block for body.
    LLVMBasicBlockRef block = LLVMAppendBasicBlock(module->llvm_function, "");

    // Generate function arguments.
    LLVMPositionBuilderAtEnd(builder, block);
    rc = qip_ast_function_codegen_args(node, module);
    check(rc == 0, "Unable to codegen function arguments");

    // If this function is marked as external then dynamically generate the
    // code to call the function and return the value.
    if(external_metadata_node != NULL) {
        rc = qip_ast_function_generate_external_call(node, module, block);
        check(rc == 0, "Unable to generate call to external function");
    }
    // Otherwise generate the body if it exists.
    else if(node->function.body != NULL) {
        rc = qip_ast_block_codegen_with_block(node->function.body, module, block);
        check(rc == 0, "Unable to generate function body statements");
    }
    // If there's no body or it's not external then we have a problem.
    else {
        sentinel("Function must be external or have a body");
    }
    
    // Dump before verification.
    // LLVMDumpValue(func);
    
    // Verify function.
    rc = LLVMVerifyFunction(func, LLVMPrintMessageAction);
    check(rc != 1, "Invalid function");

    // Unset the current function.
    rc = qip_module_pop_scope(module, node);
    check(rc == 0, "Unable to remove function scope");
    module->llvm_function = NULL;
    if(module->llvm_last_alloca != NULL) {
        LLVMInstructionEraseFromParent(module->llvm_last_alloca);
        module->llvm_last_alloca = NULL;
    }

    // Return function as a value.
    *value = func;
    
    return 0;

error:
    // Unset the current function.
    module->llvm_function = NULL;
    //if(func) LLVMDeleteFunction(func);
    *value = NULL;
    return -1;
}

// Generates a function call to an external function.
//
// node  - The function node.
// block - The LLVM block the call is being added to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_generate_external_call(qip_ast_node *node,
                                             qip_module *module,
                                             LLVMBasicBlockRef block)
{
    int rc;
    check(node != NULL, "Node required");
    check(block != NULL, "Block required");
    
    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Grab the External metadata node if there is one.
    qip_ast_node *external_metadata_node = NULL;
    rc = qip_ast_function_get_external_metadata_node(node, &external_metadata_node);
    check(rc == 0, "Unable to retrieve external metadata node");
    check(external_metadata_node != NULL, "External metadata node does not exist");
    
    // Retrieve function name from metadata.
    bstring function_name = NULL;
    rc = qip_ast_metadata_get_item_value(external_metadata_node, NULL, &function_name);
    check(rc == 0, "Unable to retrieve function name from metadata");
    
    // Make sure we have an external function name.
    check(function_name != NULL, "External function name required");
    
    // Loop over function arguments to make call arguments.
    unsigned int i;
    LLVMValueRef *args = malloc(sizeof(LLVMValueRef) * node->function.arg_count); check_mem(args);
    for(i=0; i<node->function.arg_count; i++) {
        qip_ast_node *farg = node->function.args[i];
        bstring arg_name = farg->farg.var_decl->var_decl.name;
        
        // Retrieve LLVM reference.
        qip_ast_node *farg_var_decl = NULL;
        LLVMValueRef farg_value = NULL;
        rc = qip_module_get_variable(module, arg_name, &farg_var_decl, &farg_value);
        check(rc == 0, "Unable to retrieve function argument declaration: %s", bdata(arg_name));
        check(farg_value != NULL, "No LLVM value for function argument declaration: %s", bdata(arg_name));
        
        // Load the reference and add it to the argument list.
        args[i] = LLVMBuildLoad(builder, farg_value, "");
        check(args[i] != NULL, "Unable to build load for function argument");
    }
    
    // Retrieve function.
    LLVMValueRef func = LLVMGetNamedFunction(module->llvm_module, bdata(function_name));
    check(func != NULL, "Unable to find external function: %s", bdata(function_name));
    check(LLVMCountParams(func) == node->function.arg_count, "Argument mismatch (got %d, expected %d)", node->function.arg_count, LLVMCountParams(func));
    
    // Create call instruction.
    LLVMValueRef call_value = LLVMBuildCall(builder, func, args, node->function.arg_count, "");
    check(call_value != NULL, "Unable to build external function call");
    
    // If function return void then generate a void return.
    if(qip_ast_type_ref_is_void(node->function.return_type)) {
        LLVMValueRef ret_value = LLVMBuildRetVoid(builder);
        check(ret_value != NULL, "Unable to build external void return");
    }
    // Otherwise return the value from the function.
    else {
        LLVMValueRef ret_value = LLVMBuildRet(builder, call_value);
        check(ret_value != NULL, "Unable to build external return value");
    }
    
    return 0;

error:
    return -1;
}

// Generates the function prototype. This is used for forward declarations
// and for the function codegen itself.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_codegen_prototype(qip_ast_node *node, qip_module *module,
                                       LLVMValueRef *value)
{
    int rc;
    
    // Retrieve the fully qualified function name.
    bstring qualified_name = NULL;
    rc = qip_ast_function_get_qualified_name(node, &qualified_name);
    check(rc == 0, "Unable to determine fully qualified function name");
    
    // Generate the prototype with the given name.
    rc = qip_ast_function_codegen_prototype_with_name(node, module, qualified_name, value);
    check(rc == 0, "Unable to generate named prototype");
    
    bdestroy(qualified_name);
    return 0;

error:
    bdestroy(qualified_name);
    return -1;
}

// Generates the function prototype but overrides the name. This is used by
// the metadata to create external APIs to C.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_codegen_prototype_with_name(qip_ast_node *node,
                                                 qip_module *module,
                                                 bstring function_name,
                                                 LLVMValueRef *value)
{
    int rc;
    unsigned int i;
    LLVMValueRef func = NULL;
    bstring arg_type_name = NULL;
    bstring return_type_name = NULL;

    // Check for an existing prototype.
    func = LLVMGetNamedFunction(module->llvm_module, bdata(function_name));
    
    // If a prototype exists then simply verify it matches and return it.
    if(func != NULL) {
        check(LLVMCountBasicBlocks(func) == 0, "Illegal function redefinition");
        check(LLVMCountParams(func) == node->function.arg_count, "Function prototype already exists with different arguments");
    }
    // If there is no prototype then create one.
    else {
        // Dynamically generate the return type of the function if it is missing.
        if(node->function.return_type == NULL) {
            rc = qip_ast_function_generate_return_type(node, module);
            check(rc == 0, "Unable to generate return type for function");
        }

        // Create a list of function argument types.
        qip_ast_node *arg;
        unsigned int arg_count = node->function.arg_count;
        LLVMTypeRef *params = malloc(sizeof(LLVMTypeRef) * arg_count);

        // Create arguments.
        for(i=0; i<arg_count; i++) {
            arg = node->function.args[i];
            rc = qip_ast_type_ref_get_full_name(arg->farg.var_decl->var_decl.type, &arg_type_name);
            check(rc == 0, "Unable to retrieve function arg type name");
            
            LLVMTypeRef param = NULL;
            rc = qip_module_get_type_ref(module, arg_type_name, NULL, &param);
            check(rc == 0, "Unable to determine function argument type");
        
            // Pass argument as reference if this is a complex type.
            if(qip_llvm_is_complex_type(param)) {
                params[i] = LLVMPointerType(param, 0);
            }
            // Otherwise pass it by value.
            else {
                params[i] = param;
            }
        }

        // Retrieve type name.
        rc = qip_ast_type_ref_get_full_name(node->function.return_type, &return_type_name);
        check(rc == 0, "Unable to retrieve full return type name");
        
        // Determine return type.
        LLVMTypeRef return_type;
        rc = qip_module_get_type_ref(module, return_type_name, NULL, &return_type);
        check(rc == 0, "Unable to determine function return type");

        if(qip_llvm_is_complex_type(return_type)) {
            return_type = LLVMPointerType(return_type, 0);
        }

        // Create function type.
        LLVMTypeRef funcType = LLVMFunctionType(return_type, params, arg_count, false);
        check(funcType != NULL, "Unable to create function type");

        // Create function.
        func = LLVMAddFunction(module->llvm_module, bdata(function_name), funcType);
        check(func != NULL, "Unable to create function");
    
        // Assign names to function arguments.
        for(i=0; i<arg_count; i++) {
            arg = node->function.args[i];
            LLVMValueRef param = LLVMGetParam(func, i);
            LLVMSetValueName(param, bdata(arg->farg.var_decl->var_decl.name));
        }
    }
    
    // Return function prototype;
    *value = func;
    
    bdestroy(arg_type_name);
    bdestroy(return_type_name);
    return 0;

error:
    bdestroy(arg_type_name);
    bdestroy(return_type_name);
    if(func) LLVMDeleteFunction(func);
    *value = NULL;
    return -1;
}

// Generates the allocas for the function arguments. This has to be called
// from the block since that is where the entry block is created.
//
// node    - The function node.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_codegen_args(qip_ast_node *node, qip_module *module)
{
    int rc;
    unsigned int i;
    
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type expected to be 'function'");
    check(module != NULL, "Module required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);

    // Codegen allocas.
    LLVMValueRef *values = malloc(sizeof(LLVMValueRef) * node->function.arg_count);
    check_mem(values);
    
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_codegen(node->function.args[i], module, &values[i]);
        check(rc == 0, "Unable to determine function argument type");
    }
    
    module->llvm_last_alloca = LLVMBuildAlloca(builder, LLVMInt1TypeInContext(context), "nop");
    
    // Codegen store instructions.
    for(i=0; i<node->function.arg_count; i++) {
        LLVMValueRef param = LLVMGetParam(module->llvm_function, i);
        LLVMValueRef build_value = LLVMBuildStore(builder, param, values[i]);
        check(build_value != NULL, "Unable to create store instruction");
    }
    
    free(values);

    return 0;
    
error:
    if(values) free(values);
    return -1;
}

// Recursively generates forward declarations.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_codegen_forward_decl(qip_ast_node *node,
                                          qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");
    check(module != NULL, "Module required");

    // Generate forward declaration.
    LLVMValueRef func = NULL;
    rc = qip_ast_function_codegen_prototype(node, module, &func);
    check(rc == 0, "Unable to generate function prototype");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Processes the node.
//
// node   - The node.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_preprocess(qip_ast_node *node, qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Preprocess argument types.
    uint32_t i;
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_preprocess(node->function.args[i], module);
        check(rc == 0, "Unable to preprocess function argument type");
    }

    // Preprocess block.
    if(node->function.body != NULL) {
        rc = qip_ast_node_preprocess(node->function.body, module);
        check(rc == 0, "Unable to preprocess function body");
    }

    return 0;

error:
    return -1;   
}


//--------------------------------------
// Misc
//--------------------------------------

// Retrieves the class that this function belongs to (if it is a method).
// Otherwise it returns NULL as the class.
//
// node      - The function AST node.
// class_ast - A pointer to where the class AST node should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_get_class(qip_ast_node *node, qip_ast_node **class_ast)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");
    check(class_ast != NULL, "Class return pointer must not be null");
    
    *class_ast = NULL;

    // Check if there is a parent method.
    if(node->parent != NULL && node->parent->type == QIP_AST_TYPE_METHOD) {
        qip_ast_node *method = node->parent;

        // Check if the method has a class.
        if(method->parent != NULL && method->parent->type == QIP_AST_TYPE_CLASS) {
            *class_ast = method->parent;
        }
    }
    
    return 0;
    
error:
    *class_ast = NULL;
    return -1;
}


// Updates the return type of the function based on the last return statement
// of the function. This is used for implicit functions like the main function
// of a module.
//
// node   - The function ast node to generate a type for.
// module - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_generate_return_type(qip_ast_node *node,
                                          qip_module *module)
{
    int rc;
    qip_ast_node *type = NULL;
    
    check(node != NULL, "Function required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");

    // If function has no body then its return type is void.
    qip_ast_node *body = node->function.body;
    if(body == NULL) {
        node->function.return_type = qip_ast_type_ref_create_cstr("void");
        check_mem(node->function.return_type);
        node->function.return_type->parent = node;
    }
    // Otherwise find the last return statement and determine its type.
    else {
        qip_ast_node *freturn = NULL;
        
        // Loop over all returns and save the last one.
        unsigned int i;
        for(i=0; i<body->block.expr_count; i++) {
            if(body->block.exprs[i]->type == QIP_AST_TYPE_FRETURN) {
                freturn = body->block.exprs[i];
            }
        }
        
        // If there is no return statement or it's a void return then the type
        // is void.
        if(freturn == NULL || freturn->freturn.value == NULL) {
            type = qip_ast_type_ref_create_cstr("void"); check_mem(type);
        }
        // Otherwise check the last return value to determine its type.
        else {
            rc = qip_ast_node_get_type(freturn->freturn.value, module, &type);
            check(rc == 0, "Unable to determine return type");
        }

        // Copy return type.
        rc = qip_ast_node_copy(type, &node->function.return_type);
        check(rc == 0, "Unable to copy type");
        node->function.return_type->parent = node;
    }

    return 0;
    
error:
    return -1;
}

// Searches for variable declarations within the function's argument list.
//
// node     - The node to search within.
// name     - The name of the variable to search for.
// var_decl - A pointer to where the variable declaration should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_get_var_decl(qip_ast_node *node, bstring name,
                                  qip_ast_node **var_decl)
{
    unsigned int i;
    
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");

    // Search argument list for variable declaration.
    *var_decl = NULL;
    for(i=0; i<node->function.arg_count; i++) {
        if(biseq(node->function.args[i]->farg.var_decl->var_decl.name, name) == 1) {
            *var_decl = node->function.args[i]->farg.var_decl;
            break;
        }
    }

    return 0;
    
error:
    *var_decl = NULL;
    return -1;    
}

// Retrieves the fully qualified name for a function.
//
// node - The node.
// name - A pointer to where the fully qualified name should be set.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_get_qualified_name(qip_ast_node *node, bstring *name)
{
    int rc;
    check(node != NULL, "Node required");
    check(name != NULL, "Name return pointer required");
    
    // Find the class this function belongs to, if any.
    qip_ast_node *class_ast = NULL;
    rc = qip_ast_function_get_class(node, &class_ast);
    check(rc == 0, "Unable to retrieve parent class for function");

    // Function name should be prepended with the class name if this is a method.
    bool is_method = (class_ast != NULL);
    if(is_method) {
        check(blength(class_ast->class.name) > 0, "Class name required for method");
        *name = bformat("%s.%s", bdata(class_ast->class.name), bdata(node->function.name));
        check_mem(*name);
    }
    else {
        *name = bstrcpy(node->function.name);
        check_mem(*name);
    }

    return 0;

error:
    if(name) *name = NULL;
    return -1;
}

//--------------------------------------
// Metadata
//--------------------------------------

// Retrieves the external metadata node for a function.
//
// node - The function node.
// external_metadata_node - A pointer to where the metadata node should be
//                          returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_get_external_metadata_node(qip_ast_node *node,
                                                qip_ast_node **external_metadata_node)
{
    int rc;
    check(node != NULL, "Node required");
    check(external_metadata_node != NULL, "External Node return pointer required");
    
    struct tagbstring metadata_name = bsStatic("External");

    // Initialize return value.
    *external_metadata_node = NULL;
    
    // Check if the function is attached to a method.
    if(node->parent != NULL && node->parent->type == QIP_AST_TYPE_METHOD) {
        // Retrieve metadata from method.
        rc = qip_ast_method_get_metadata_node(node->parent, &metadata_name, external_metadata_node);
        check(rc == 0, "Unable to retrieve metadata from method");
    }
    
    return 0;

error:
    *external_metadata_node = NULL;
    return -1;
}


//--------------------------------------
// Type refs
//--------------------------------------

// Computes a list of type refs used by the node.
//
// node      - The node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_get_type_refs(qip_ast_node *node,
                                   qip_ast_node ***type_refs,
                                   uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Add return type.
    if(node->function.return_type != NULL) {
        rc = qip_ast_node_get_type_refs(node->function.return_type, type_refs, count);
        check(rc == 0, "Unable to add function return type refs");
    }

    // Add argument types.
    uint32_t i;
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_get_type_refs(node->function.args[i], type_refs, count);
        check(rc == 0, "Unable to add function argument type type refs");
    }

    // Add block type refs.
    if(node->function.body != NULL) {
        rc = qip_ast_node_get_type_refs(node->function.body, type_refs, count);
        check(rc == 0, "Unable to add function body type refs");
    }

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
int qip_ast_function_get_dependencies(qip_ast_node *node,
                                      bstring **dependencies,
                                      uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Add return type.
    if(node->function.return_type != NULL) {
        rc = qip_ast_node_get_dependencies(node->function.return_type, dependencies, count);
        check(rc == 0, "Unable to add function return type dependencies");
    }

    // Add argument types.
    uint32_t i;
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_get_dependencies(node->function.args[i], dependencies, count);
        check(rc == 0, "Unable to add function argument type dependency");
    }

    // Add block dependencies.
    if(node->function.body != NULL) {
        rc = qip_ast_node_get_dependencies(node->function.body, dependencies, count);
        check(rc == 0, "Unable to add function body dependencies");
    }

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
// node   - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    bstring msg = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Retrieve parent.
    qip_ast_node *method = (node->parent && node->parent->type == QIP_AST_TYPE_METHOD ? node->parent : NULL);

    // If there is no body then the function must have an External metatag.
    if(method != NULL && node->function.body == NULL) {
        // Grab the External metadata node if there is one.
        qip_ast_node *external_metadata_node = NULL;
        rc = qip_ast_function_get_external_metadata_node(node, &external_metadata_node);
        check(rc == 0, "Unable to retrieve external metadata node");
        
        if(external_metadata_node == NULL) {
            msg = bformat("Function '%s' must have a body unless it is marked as external", bdata(node->function.name));
            rc = qip_module_add_error(module, node, msg);
            check(rc == 0, "Unable to add module error");
            bdestroy(msg);
            msg = NULL;
        }
    }

    // Validate argument types.
    uint32_t i;
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_validate(node->function.args[i], module);
        check(rc == 0, "Unable to validate function argument type");
    }

    // Validate block.
    if(node->function.body != NULL) {
        rc = qip_ast_node_validate(node->function.body, module);
        check(rc == 0, "Unable to validate function body");
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
int qip_ast_function_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    bstring str = bformat("<function name='%s'>\n", bdatae(node->function.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children
    if(node->function.return_type != NULL) {
        rc = qip_ast_node_dump(node->function.return_type, ret);
        check(rc == 0, "Unable to dump function return type");
    }

    unsigned int i;
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_dump(node->function.args[i], ret);
        check(rc == 0, "Unable to dump function argument");
    }
    if(node->function.body != NULL) {
        rc = qip_ast_node_dump(node->function.body, ret);
        check(rc == 0, "Unable to dump function body");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
