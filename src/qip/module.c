#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "array.h"
#include "llvm.h"
#include "module.h"
#include "util.h"
#include "dbg.h"


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring QIP_TYPE_NAME_INT     = bsStatic("Int");

struct tagbstring QIP_TYPE_NAME_FLOAT   = bsStatic("Float");

struct tagbstring QIP_TYPE_NAME_BOOLEAN = bsStatic("Boolean");

struct tagbstring QIP_TYPE_NAME_VOID    = bsStatic("void");


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a module.
qip_module *qip_module_create(bstring name, qip_compiler *compiler)
{
    // Default module name to "eql".
    struct tagbstring eql_str = bsStatic("eql");
    if(name == NULL) {
        name = &eql_str;
    }
    
    qip_module *module = malloc(sizeof(qip_module));
    check_mem(module);
    module->compiler = compiler;
    module->llvm_module = LLVMModuleCreateWithName(bdata(name));
    module->llvm_engine = NULL;
    module->llvm_pass_manager = NULL;
    module->sequence = 0;
    module->scopes = NULL;
    module->scope_count = 0;
    module->types = NULL;
    module->type_nodes = NULL;
    module->type_count = 0;
    module->errors = NULL;
    module->error_count = 0;
    module->ast_modules = NULL;
    module->ast_module_count = 0;
    module->perm_pool = qip_mempool_create(); check_mem(module->perm_pool);
    module->temp_pool = qip_mempool_create(); check_mem(module->temp_pool);

    // Initialize LLVM.
    LLVMInitializeNativeTarget();
    LLVMLinkInJIT();

    // Initialize engine.
    char *msg = NULL;
    if(LLVMCreateExecutionEngineForModule(&module->llvm_engine, module->llvm_module, &msg) == 1) {
        sentinel("Unable to initialize execution engine: %s", msg);
    }
    
    return module;
    
error:
    if(msg != NULL) LLVMDisposeMessage(msg);
    qip_module_free(module);
    return NULL;
}

// Frees a module.
//
// module - The module to free.
void qip_module_free(qip_module *module)
{
    if(module) {
        module->compiler = NULL;

        if(module->llvm_pass_manager) LLVMDisposePassManager(module->llvm_pass_manager);
        module->llvm_pass_manager = NULL;

        if(module->llvm_module) LLVMDisposeModule(module->llvm_module);
        module->llvm_module = NULL;

        if(module->scopes != NULL) free(module->scopes);
        module->scopes = NULL;
        module->scope_count = 0;

        if(module->types != NULL) free(module->types);
        module->types = NULL;
        module->type_count = 0;

        qip_module_free_errors(module);
        qip_module_free_ast_modules(module);

        qip_mempool_free(module->perm_pool);
        qip_mempool_free(module->temp_pool);

        free(module);
    }
}


//--------------------------------------
// AST Module Management
//--------------------------------------

// Adds an AST module to a compilation module.
//
// module     - The compilation module.
// ast_module - The AST module.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_add_ast_module(qip_module *module, qip_ast_node *ast_module)
{
    check(module != NULL, "Module required");
    check(ast_module != NULL, "AST Module required");
    check(ast_module->type == QIP_AST_TYPE_MODULE, "Node type must be 'module'");
    
    // Append ast module to compilation module.
    module->ast_module_count++;
    module->ast_modules = realloc(module->ast_modules, sizeof(qip_ast_node*) * module->ast_module_count);
    check_mem(module->ast_modules);
    module->ast_modules[module->ast_module_count-1] = ast_module;
    
    return 0;

error:
    return -1;
}

// Retrieves the AST node for a class that has been added to the compilation
// module.
//
// module - The compilation module.
// name   - The name of the class to search for.
// ret    - A pointer to where the class AST should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_get_ast_class(qip_module *module, bstring name,
                             qip_ast_node **ret)
{
    int rc;
    check(module != NULL, "Module required");
    check(name != NULL, "Class name required");
    check(ret != NULL, "Return pointer required");
    
    // Initialize the return value.
    *ret = NULL;
    
    // Loop over each AST module.
    uint32_t i;
    for(i=0; i<module->ast_module_count; i++) {
        qip_ast_node *ast_module = module->ast_modules[i];
        
        // Search module for class.
        rc = qip_ast_module_get_class(ast_module, name, ret);
        check(rc == 0, "Unable find class");
        
        // If class is found then break.
        if(*ret != NULL) {
            break;
        }
    }
    
    return 0;

error:
    *ret = NULL;
    return -1;
}

// Retrieves the AST node for a class with the matching set of template
// variables.
//
// module   - The compilation module.
// type_ref - A reference to the type.
// ret      - A pointer to where the class AST should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_get_ast_template_class(qip_module *module,
                                      qip_ast_node *type_ref,
                                      qip_ast_node **ret)
{
    int rc;
    check(module != NULL, "Module required");
    check(type_ref != NULL, "Type reference required");
    check(ret != NULL, "Return pointer required");
    
    // Initialize the return value.
    *ret = NULL;
    
    // Loop over each AST module.
    uint32_t i;
    for(i=0; i<module->ast_module_count; i++) {
        qip_ast_node *ast_module = module->ast_modules[i];
        
        // Search module for template class.
        rc = qip_ast_module_get_template_class(ast_module, type_ref, ret);
        check(rc == 0, "Unable find template class");
        
        // If class is found then break.
        if(*ret != NULL) {
            break;
        }
    }
    
    return 0;

error:
    *ret = NULL;
    return -1;
}

// Searches all the AST modules to find the main function.
//
// module - The module.
// ret    - A pointer to where the main function should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_get_ast_main_function(qip_module *module, qip_ast_node **ret)
{
    check(module != NULL, "Module required");
    check(ret != NULL, "Return pointer required");
    
    // Initialize return pointer.
    *ret = NULL;
    
    // Search AST modules for a main function.
    unsigned int i;
    for(i=0; i<module->ast_module_count; i++) {
        qip_ast_node *ast_module = module->ast_modules[i];

        if(ast_module->module.main_function != NULL) {
            *ret = ast_module->module.main_function;
            break;
        }
    }
    
    return 0;

error:
    *ret = NULL;
    return -1;
}

// Processes all dynamic classes through the module's compiler's delegate.
//
// module - The compilation module.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_process_dynamic_classes(qip_module *module)
{
    int rc;
    check(module != NULL, "Module required");
    check(module->compiler != NULL, "Module compiler required");

    // Loop over AST modules.
    uint32_t i;
    unsigned int j;
    for(i=0; i<module->ast_module_count; i++) {
        qip_ast_node *ast_module = module->ast_modules[i];
        
        // Loop over classes in each module.
        for(j=0; j<ast_module->module.class_count; j++) {
            qip_ast_node *class = ast_module->module.classes[j];
            
            // Retrieve 'Dynamic' metatag from class.
            struct tagbstring dynamic_metadata_name = bsStatic("Dynamic");
            qip_ast_node *dynamic_metadata = NULL;
            rc = qip_ast_class_get_metadata_node(class, &dynamic_metadata_name, &dynamic_metadata);
            check(rc == 0, "Unable to retrieve dynamic metadata from class");
            
            // If dynamic metatag exists then process it.
            if(dynamic_metadata != NULL) {
                rc = qip_compiler_process_dynamic_class(module->compiler, class);
                check(rc == 0, "Unable to process dynamic class");
            }
        }
    }

    return 0;
    
error:
    return -1;
}

// Resolves all implementations of generic classes within the module. It works
// by starting with non-template classes and resolving generic references by
// creating new classes. The new classes are then processed for references
// within them.
//
// module - The module.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_process_templates(qip_module *module)
{
    int rc;
    unsigned int i, j;
    check(module != NULL, "Module required");

    // Initialize an array to hold sources of type refs.
    qip_array *sources = qip_array_create(); check_mem(sources);

    // If there is a main function then add it at the beginning.
    qip_ast_node *main_function = NULL;
    rc = qip_module_get_ast_main_function(module, &main_function);
    check(rc == 0, "Unable to retrieve main function");
    
    if(main_function != NULL) {
        rc = qip_array_push(sources, main_function);
        check(rc == 0, "Unable to add main function to template sources")
    }

    // Collect all non-template classes.
    for(i=0; i<module->ast_module_count; i++) {
        qip_ast_node *ast_module = module->ast_modules[i];
        
        for(j=0; j<ast_module->module.class_count; j++) {
            qip_ast_node *class = ast_module->module.classes[j];
            if(class->class.template_var_count == 0) {
                rc = qip_array_push(sources, class);
                check(rc == 0, "Unable to append class to template sources");
            }
        }
    }
    
    // Loop over array and generate new classes as necessary.
    qip_ast_node **type_refs = NULL;
    uint32_t type_ref_count = 0;
    for(i=0; i<sources->length; i++) {
        qip_ast_node *source = sources->elements[i];
        
        // Retrieve list of type references.
        type_refs = NULL;
        type_ref_count = 0;
        rc = qip_ast_node_get_type_refs(source, &type_refs, &type_ref_count);
        check(rc == 0, "Unable to retrieve type refs for node");
        
        // Loop over type refs and generate templates.
        for(j=0; j<type_ref_count; j++) {
            qip_ast_node *type_ref = type_refs[j];
            qip_ast_node *class = NULL;
            rc = qip_module_generate_template_type(module, type_ref, &class);
            check(rc == 0, "Unable to generate type");

            // If a new type was generated then append it to the source list.
            if(class != NULL) {
                rc = qip_array_push(sources, class);
                check(rc == 0, "Unable to append generated class to array");
            }
            
            // Flatten the type reference's type down.
            rc = qip_ast_type_ref_flatten(type_ref);
            check(rc == 0, "Unable to flatten type ref");
        }
        
        // Clean up.
        qip_ast_node_type_refs_free(&type_refs, &type_ref_count);
    }

    qip_array_free(sources);
    return 0;
    
error:
    qip_array_free(sources);
    return -1;
}

// Generates a type for a template class based on a type reference. If a type
// is generated then it will be returned. If the type already exists then
// a NULL is returned.
//
// module   - The module.
// type_ref - The type reference that the generation is based off of.
// class    - A pointer to where the generated class should be returned to.
int qip_module_generate_template_type(qip_module *module,
                                      qip_ast_node *type_ref,
                                      qip_ast_node **class)
{
    int rc;
    qip_ast_template *template = NULL;
    check(module != NULL, "Module required");
    check(type_ref != NULL, "Type reference required");
    check(type_ref->type == QIP_AST_TYPE_TYPE_REF, "Node type must be 'type ref'");
    check(class != NULL, "Class return pointer required");
    
    // Initialize the return value.
    *class = NULL;
    
    // Exit if the type ref is not a template reference or it's a Function.
    if(qip_is_function_type(type_ref) || type_ref->type_ref.subtype_count == 0) {
        return 0;
    }

    // Retrieve generated class name.
    bstring generated_class_name = NULL;
    rc = qip_ast_type_ref_get_full_name(type_ref, &generated_class_name);
    check(rc == 0, "Unable to calcualte generated class name");
    
    // Only generate the class if it doesn't exist.
    qip_ast_node *generated_class = NULL;
    rc = qip_module_get_ast_class(module, generated_class_name, &generated_class);
    check(rc == 0, "Unable to search for generated class");
    
    if(generated_class == NULL) {
        // Find the type in the module.
        qip_ast_node *template_class = NULL;
        rc = qip_module_get_ast_template_class(module, type_ref, &template_class);
        check(rc == 0, "Unable to retrieve class");
        check(template_class != NULL, "Unable to retrieve class for generation");
    
        // Copy the class.
        rc = qip_ast_node_copy(template_class, &generated_class);
        check(rc == 0, "Unable to copy class template");
        
        // Update name and remove template variables.
        bdestroy(generated_class->class.name);
        generated_class->class.name = bstrcpy(generated_class_name);
        check_mem(generated_class->class.name);
        qip_ast_class_free_template_vars(generated_class);
    
        // Create template & apply the template.
        template = qip_ast_template_create(template_class, type_ref);
        check_mem(template);
        rc = qip_ast_template_apply(template, generated_class);
        check(rc == 0, "Unable to apply template to generated class");
    
        // Add the generated class to the same module as the template.
        rc = qip_ast_module_add_class(template_class->parent, generated_class);
        check(rc == 0, "Unable to add generated class to module");
    
        qip_ast_template_free(template);
        
        // Return generated class.
        *class = generated_class;
    }

    bdestroy(generated_class_name);
    return 0;

error:
    bdestroy(generated_class_name);
    qip_ast_template_free(template);
    *class = NULL;
    return -1;
}

// Runs preprocessing on all AST modules within a compilation module.
//
// module - The module.
// stage  - The processing stage.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_preprocess(qip_module *module,
                          qip_ast_processing_stage_e stage)
{
    int rc;
    check(module != NULL, "Module required");
    
    uint32_t i;
    for(i=0; i<module->ast_module_count; i++) {
        rc = qip_ast_node_preprocess(module->ast_modules[i], module, stage);
        check(rc == 0, "Unable to preprocess module");
    }
    
    return 0;

error:
    return -1;
}


// Clears all AST modules on the compilation module.
//
// module  - The module.
void qip_module_free_ast_modules(qip_module *module)
{
    if(module != NULL) {
        uint32_t i;
        for(i=0; i<module->ast_module_count; i++) {
            qip_ast_node_free(module->ast_modules[i]);
            module->ast_modules[i] = NULL;
        }
        free(module->ast_modules);
        module->ast_modules = NULL;
        module->ast_module_count = 0;
    }
}


//--------------------------------------
// Types
//--------------------------------------

// Retrieves an LLVM type definition for a given type name. Optionally returns
// the associated AST node if the type is user-defined. For templated classes,
// this only works after the namespace has been flattened.
//
// module   - The module that contains the type.
// type_ref - The type reference.
// node     - A pointer to where the AST node should be returned to. This is
//          optional.
// type     - A pointer to where the LLVM type should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_get_type_ref(qip_module *module, qip_ast_node *type_ref,
                            qip_ast_node **node, LLVMTypeRef *type)
{
    unsigned int i;
    int rc;
    
    check(module != NULL, "Module is required");
    check(type_ref != NULL, "Type ref is required");
    
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);

    // Compare to built-in types.
    bool found = false;
    if(biseqcstr(type_ref->type_ref.name, "Int")) {
        if(type != NULL) *type = LLVMInt64TypeInContext(context);
        if(node != NULL) *node = NULL;
        found = true;
    }
    else if(biseqcstr(type_ref->type_ref.name, "Float")) {
        if(type != NULL) *type = LLVMDoubleTypeInContext(context);
        if(node != NULL) *node = NULL;
        found = true;
    }
    else if(biseqcstr(type_ref->type_ref.name, "Boolean")) {
        if(type != NULL) *type = LLVMInt1TypeInContext(context);
        if(node != NULL) *node = NULL;
        found = true;
    }
    else if(biseqcstr(type_ref->type_ref.name, "Ref")) {
        if(type != NULL) *type = LLVMPointerType(LLVMInt8TypeInContext(context), 0);
        if(node != NULL) *node = NULL;
        found = true;
    }
    else if(biseqcstr(type_ref->type_ref.name, "void")) {
        if(type != NULL) *type = LLVMVoidTypeInContext(context);
        if(node != NULL) *node = NULL;
        found = true;
    }
    // Generate function type.
    else if(qip_is_function_type(type_ref)) {
        // Determine LLVM return type.
        LLVMTypeRef llvm_return_type = NULL;
        if(type_ref->type_ref.return_type) {
            rc = qip_module_get_type_ref(module, type_ref->type_ref.return_type, NULL, &llvm_return_type);
            check(rc == 0, "Unable to determine function return type");
        }
        else {
            llvm_return_type = LLVMVoidTypeInContext(context);
        }
        
        // Determine param types.
        LLVMTypeRef params[type_ref->type_ref.subtype_count];
        for(i=0; i<type_ref->type_ref.subtype_count; i++) {
            rc = qip_module_get_type_ref(module, type_ref->type_ref.subtypes[i], NULL, &params[i]);
            check(rc == 0, "Unable to determine function param type: %d", i);
            
            // If the param is a complex type then wrap it in a pointer.
            if(qip_llvm_is_complex_type(params[i])) {
                params[i] = LLVMPointerType(params[i], 0);
            }
        }
        
        // Generate function type.
        if(type != NULL) *type = LLVMPointerType(LLVMFunctionType(llvm_return_type, params, type_ref->type_ref.subtype_count, false), 0);
        if(node != NULL) *node = NULL;
        found = true;
    }
    // Find user-defined type.
    else {
        // If we have generated types already then look at the type list.
        for(i=0; i<(unsigned int)module->type_count; i++) {
            if(biseq(module->type_nodes[i]->class.name, type_ref->type_ref.name)) {
                if(type != NULL) *type = module->types[i];
                if(node != NULL) *node = module->type_nodes[i];
                found = true;
                break;
            }
        }
        
        // Otherwise check the AST modules for classes.
        if(!found) {
            for(i=0; i<module->ast_module_count; i++) {
                qip_ast_node *ast_module = module->ast_modules[i];
                qip_ast_node *class = NULL;
                rc = qip_ast_module_get_class(ast_module, type_ref->type_ref.name, &class);
                check(rc == 0, "Unable to search AST module for class");
                
                if(class != NULL) {
                    if(type != NULL) *type = NULL;
                    if(node != NULL) *node = class;
                    found = true;
                    break;
                }
            }
        }
    }
    
    check(found, "Invalid type in module: %s", bdata(type_ref->type_ref.name));

    return 0;

error:
    if(type) *type = NULL;
    return -1;
}

// Adds a type for a given class to the module.
//
// module - The compilation unit that contains the class.
// node   - The AST node associated with this type.
// type   - The LLVM type.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_add_type_ref(qip_module *module, qip_ast_node *node,
                            LLVMTypeRef type)
{
    check(module != NULL, "Module required");
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_CLASS, "Node type must be 'class'");
    check(type != NULL, "LLVM type required");

    module->type_count++;

    // Append to the list of types.
    module->types = realloc(module->types, sizeof(LLVMTypeRef) * module->type_count);
    check_mem(module->types);
    module->types[module->type_count-1] = type;

    // Append to the list of AST nodes.
    module->type_nodes = realloc(module->type_nodes, sizeof(qip_ast_node*) * module->type_count);
    check_mem(module->type_nodes);
    module->type_nodes[module->type_count-1] = node;
    
    return 0;
    
error:
    return -1;
}

// Casts an LLVM value from one type to another.
//
// module         - The compilation unit that value belongs to.
// value          - The original value to cast.
// from_type_name - The name of the original type.
// to_type_name   - The name of the type being cast to.
// ret            - A pointer to where the new value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_cast_value(qip_module *module, LLVMValueRef value,
                          bstring from_type_name, bstring to_type_name,
                          LLVMValueRef *ret)
{
    check(module != NULL, "Module required");
    check(value != NULL, "Value required");
    check(from_type_name != NULL, "Source type required");
    check(to_type_name != NULL, "Destination type required");
    check(ret != NULL, "Return pointer required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);

    *ret = NULL;
    
    // If the types are equal then just return the original value.
    if(biseq(to_type_name, from_type_name)) {
        *ret = value;
    }
    // Otherwise maps types.
    else {
        // Cast to Int
        if(biseqcstr(to_type_name, "Int")) {
            if(biseqcstr(from_type_name, "Float")) {
                *ret = LLVMBuildFPToSI(builder, value, LLVMInt64TypeInContext(context), "");
            }
            else if(biseqcstr(from_type_name, "Boolean")) {
                *ret = LLVMBuildZExt(builder, value, LLVMInt64TypeInContext(context), "");
            }
        }
        // Cast to Float
        else if(biseqcstr(to_type_name, "Float")) {
            if(biseqcstr(from_type_name, "Int")) {
                *ret = LLVMBuildSIToFP(builder, value, LLVMDoubleTypeInContext(context), "");
            }
            else if(biseqcstr(from_type_name, "Boolean")) {
                *ret = LLVMBuildUIToFP(builder, value, LLVMDoubleTypeInContext(context), "");
            }
        }
        // Cast to Boolean
        else if(biseqcstr(to_type_name, "Boolean")) {
            if(biseqcstr(from_type_name, "Int")) {
                *ret = LLVMBuildTrunc(builder, value, LLVMInt1TypeInContext(context), "");
            }
            else if(biseqcstr(from_type_name, "Float")) {
                *ret = LLVMBuildFPToUI(builder, value, LLVMInt1TypeInContext(context), "");
            }
        }
    }

    // Throw an error if the types cannot be cast.
    check(*ret != NULL, "Unable to cast '%s' to '%s'", bdatae(from_type_name, ""), bdatae(to_type_name, ""))
    
    return 0;

error:
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Scope
//--------------------------------------

// Adds a scope to the module stack.
//
// module - The module.
// scope  - The scope to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_push_scope(qip_module *module, qip_scope *scope)
{
    check(module != NULL, "Module required");
    check(scope != NULL, "Scope required");
    
    // Resize scope stack and append.
    module->scope_count++;
    module->scopes = realloc(module->scopes, sizeof(*module->scopes) * module->scope_count);
    check_mem(module->scopes);
    module->scopes[module->scope_count-1] = scope;

    return 0;
    
error:
    return -1;
}

// Removes the current scope from the stack of the module.
//
// module - The module to remove the scope from.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_pop_scope(qip_module *module)
{
    check(module != NULL, "Module required");
    check(module->scope_count > 0, "Module has no more scopes");

    // Destroy current scope.
    qip_scope *scope = module->scopes[module->scope_count-1];
    qip_scope_free(scope);
    module->scopes[module->scope_count-1] = NULL;
    
    // Resize scope stack.
    module->scope_count--;

    return 0;
    
error:
    return -1;
    
}

// Searches the module scope stack for variables declared with the given name.
//
// module - The module containing the variable declaration.
// name   - The name of the variable to search for.
// node   - A pointer the AST node that created the value.
// value  - A pointer to where the LLVM value should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_get_variable(qip_module *module, bstring name,
                            qip_ast_node **node, LLVMValueRef *value)
{
    int rc;
    check(module != NULL, "Module required");
    check(name != NULL, "Variable name required");
    check(node != NULL || value != NULL, "Node return pointer or value return pointer required");

    // Loop over scopes from the top down.
    if(module->scope_count > 0) {
        int32_t i;
        for(i=module->scope_count-1; i>=0; i--) {
            // Search scope for a variable with the given name.
            qip_scope *scope = module->scopes[i];
            rc = qip_scope_get_variable(scope, name, node, value);
            check(rc == 0, "Unable to search scope");

            // If we found something then exit.
            if((node != NULL && *node != NULL) || (value != NULL && *value != NULL)) {
                return 0;
            }

            // Stop one we get to the function boundry.
            if(scope->type == QIP_SCOPE_TYPE_FUNCTION) {
                break;
            }
        }
    }

    // If we reach here then we couldn't find the variable in any scope.
    if(node) *node   = NULL;
    if(value) *value = NULL;

    return 0;

error:
    if(node) *node   = NULL;
    if(value) *value = NULL;
    return -1;
}

// Adds a variable declaration to the current scope of the module.
//
// module    - The module containing the variable declaration.
// var_decl  - The AST variable declaration to add.
// value     - The LLVM value associated with the declaration.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_add_variable(qip_module *module, qip_ast_node *var_decl,
                            LLVMValueRef value)
{
    int rc;
    check(module != NULL, "Module is required");
    check(module->scope_count > 0, "Module has no scope");
    check(var_decl != NULL, "Variable declaration is required");
    check(var_decl->var_decl.name != NULL, "Variable declaration name is required");
    check(value != NULL, "LLVM value is required");

    // Find current scope and add variable.
    qip_scope *scope = module->scopes[module->scope_count-1];
    rc = qip_scope_add_variable(scope, var_decl, value);
    check(rc == 0, "Unable to add variable to scope");

    return 0;

error:
    return -1;
}

// Retrieves the current function scope.
//
// module - The module.
// ret    - A pointer to where the function scope should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_get_current_function_scope(qip_module *module, qip_scope **ret)
{
    check(module != NULL, "Module is required");
    check(ret != NULL, "Return pointer required");

    // Initialize return value.
    *ret = NULL;

    // Loop over scopes from the top down.
    if(module->scope_count > 0) {
        int32_t i;
        for(i=module->scope_count-1; i>=0; i--) {
            qip_scope *scope = module->scopes[i];
            if(scope->type == QIP_SCOPE_TYPE_FUNCTION) {
                *ret = scope;
                break;
            }
        }
    }

    return 0;

error:
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Execution
//--------------------------------------

// Retrieves the function pointer for the main function of a module.
//
// module - The module that contains the main function.
// ret    - A pointer to where the function pointer should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_get_main_function(qip_module *module, void **ret)
{
    check(module != NULL, "Module required");

    // Find a reference to the main function.
    LLVMValueRef func_value = LLVMGetNamedFunction(module->llvm_module, "main");
    check(func_value != NULL, "Main function not found in module");
    
    // Generate a pointer to the main function.
    *ret = LLVMGetPointerToGlobal(module->llvm_engine, func_value);

    return 0;
    
error:
    *ret = NULL;
    return -1;
}

// Executes the main function of a QIP module that returns a pointer.
//
// module - The module to execute.
// ret    - A pointer to where the returned pointer should be sent.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_execute(qip_module *module, void **ret)
{
    int rc;
    check(module != NULL, "Module required");
    check(ret != NULL, "Return pointer required");

    // Reset temporary pool.
    rc = qip_module_reset_temp_pool(module);
    check(rc == 0, "Unable to reset temporary pool");

    // Execute main function.
    sky_qip_function_t main_function;
    rc = qip_module_get_main_function(module, (void*)(&main_function));
    check(rc == 0 && main_function != NULL, "Unable to retrieve main function");
    *ret = main_function();

    return 0;
    
error:
    *ret = 0;
    return -1;
}

// Executes the main function of a QIP module that returns an Int.
//
// module - The module to execute.
// ret    - A pointer to where the returned value should be sent.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_execute_int(qip_module *module, int64_t *ret)
{
    int rc;
    check(module != NULL, "Module required");
    check(ret != NULL, "Return pointer required");

    // Reset temporary pool.
    rc = qip_module_reset_temp_pool(module);
    check(rc == 0, "Unable to reset temporary pool");

    // Execute main function.
    sky_qip_int_function_t main_function;
    rc = qip_module_get_main_function(module, (void*)(&main_function));
    check(rc == 0 && main_function != NULL, "Unable to retrieve main function");
    *ret = main_function();

    return 0;
    
error:
    *ret = 0;
    return -1;
}

// Executes the main function of a QIP module that returns a Float
//
// module - The module to execute.
// ret    - A pointer to where the returned value should be sent.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_execute_float(qip_module *module, double *ret)
{
    int rc;
    check(module != NULL, "Module required");
    check(ret != NULL, "Return pointer required");

    // Reset temporary pool.
    rc = qip_module_reset_temp_pool(module);
    check(rc == 0, "Unable to reset temporary pool");

    // Execute main function.
    sky_qip_float_function_t main_function;
    rc = qip_module_get_main_function(module, (void*)(&main_function));
    check(rc == 0 && main_function != NULL, "Unable to retrieve main function");
    *ret = main_function();

    return 0;
    
error:
    *ret = 0;
    return -1;
}

// Executes the main function of a QIP module that returns a Boolean.
//
// module - The module to execute.
// ret    - A pointer to where the returned value should be sent.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_execute_boolean(qip_module *module, bool *ret)
{
    int rc;
    check(module != NULL, "Module required");
    check(ret != NULL, "Return pointer required");

    // Reset temporary pool.
    rc = qip_module_reset_temp_pool(module);
    check(rc == 0, "Unable to reset temporary pool");

    // Execute main function.
    sky_qip_boolean_function_t main_function;
    rc = qip_module_get_main_function(module, (void*)(&main_function));
    check(rc == 0 && main_function != NULL, "Unable to retrieve main function");
    *ret = main_function();

    return 0;
    
error:
    *ret = 0;
    return -1;
}



//--------------------------------------
// Execution
//--------------------------------------

// Retrieves the function pointer for the method of a given class.
//
// module      - The module.
// class_name  - The name of the class.
// method_name - The method name.
// ret         - A pointer to where the function pointer should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_get_class_method(qip_module *module, bstring class_name,
                                bstring method_name, void **ret)
{
    check(module != NULL, "Module required");
    check(class_name != NULL, "Class name required");
    check(method_name != NULL, "Method name required");
    check(ret != NULL, "Return pointer required");

    // Generate the function name.
    bstring function_name = bformat("%s.%s", bdata(class_name), bdata(method_name));
    check_mem(function_name);

    // Find a reference to the function.
    LLVMValueRef func_value = LLVMGetNamedFunction(module->llvm_module, bdata(function_name));
    check(func_value != NULL, "Function not found in module: %s", bdata(function_name));
    
    // Generate a pointer to the function.
    *ret = LLVMGetPointerToGlobal(module->llvm_engine, func_value);
    return 0;
    
error:
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Error Management
//--------------------------------------

// Appends an error for a node onto the module.
//
// module  - The module to add the error to.
// node    - The node that caused the error.
// message - The error message.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_add_error(qip_module *module, qip_ast_node *node,
                         bstring message)
{
    check(module != NULL, "Module required");
    check(node != NULL, "Node required");
    check(message != NULL, "Message required");

    qip_error *err = qip_error_create(); check_mem(err);
    err->line_no = node->line_no;
    err->message = bstrcpy(message); check_mem(err->message);

    module->error_count++;
    module->errors = realloc(module->errors, sizeof(*module->errors) * module->error_count);
    check_mem(module->errors);
    module->errors[module->error_count-1] = err;
    
    return 0;

error:
    qip_error_free(err);
    return -1;
}

// Clears all errors on the module.
//
// module  - The module.
void qip_module_free_errors(qip_module *module)
{
    if(module != NULL) {
        uint32_t i;
        for(i=0; i<module->error_count; i++) {
            qip_error_free(module->errors[i]);
            module->errors[i] = NULL;
        }
        free(module->errors);
        module->errors = NULL;
        module->error_count = 0;
    }
}


//--------------------------------------
// Memory Allocation
//--------------------------------------

// Allocates a given number of bytes in the module's permanent memory pool.
// The permanent memory pool exists for the life of the module and will only 
// be freed when the module is freed.
//
// module - The module.
// size   - The number of bytes to allocate.
// ptr    - A pointer to where the allocated pointer should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_perm_malloc(qip_module *module, size_t size, void **ptr)
{
    int rc;
    check(module != NULL, "Module required");
    
    // Delegate allocation to the module's permanent pool.
    rc = qip_mempool_malloc(module->perm_pool, size, ptr);
    check(rc == 0, "Unable to allocate memory in permanent pool");
    
    return 0;

error:
    *ptr = NULL;
    return -1;
}

// Allocates a given number of bytes in the module's temporary memory pool.
// The temporary memory pool exists for the life of a single module call.
//
// module - The module.
// size   - The number of bytes to allocate.
// ptr    - A pointer to where the allocated pointer should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_temp_malloc(qip_module *module, size_t size, void **ptr)
{
    int rc;
    check(module != NULL, "Module required");
    
    // Delegate allocation to the module's temporary pool.
    rc = qip_mempool_malloc(module->temp_pool, size, ptr);
    check(rc == 0, "Unable to allocate memory in temporary pool");
    
    return 0;

error:
    *ptr = NULL;
    return -1;
}

// Resets the temporary pool so that the allocation pointer moves to the
// beginning of the first block. This is called before every execution of the
// module.
//
// Note that the temporary pool is not actually freed. The pool is simply
// reused. The pool will automatically be freed when the module is freed.
//
// module - The module.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_reset_temp_pool(qip_module *module)
{
    int rc;
    check(module != NULL, "Module required");
    
    // Delegate reset to the module's temporary pool.
    rc = qip_mempool_reset(module->temp_pool);
    check(rc == 0, "Unable to reset temporary memory pool");
    
    return 0;

error:
    return -1;
}


//======================================
// Debugging
//======================================

int qip_module_dump(qip_module *module)
{
    check(module != NULL, "Module is required");
    check(module->llvm_module != NULL, "Module must be compiled");

    LLVMDumpModule(module->llvm_module);
    return 0;
    
error:
    return -1;
}

int qip_module_dump_to_file(qip_module *module, bstring filename)
{
    check(filename != NULL, "Filename is required");
    
    // Redirect dump to file.
    bool opened = freopen(bdata(filename), "w", stderr);
    check(opened, "Unable to open file for dump");
    
    // Dump module.
    int rc = qip_module_dump(module);
    check(rc == 0, "Unable to dump module");

    // Close file and reassign stderr.
    dup2(1, 2);
    
    return 0;
    
error:
    // Clean up stderr switch if there was an error.
    dup2(1, 2);

    return -1;
}

// Dumps all AST modules to standard error.
//
// module - The module.
//
// Returns 0 if successful, otherwise returns -1.
int qip_module_ast_dump_stderr(qip_module *module)
{
    int rc;
    unsigned int i;
    check(module != NULL, "Module required");
    
    for(i=0; i<module->ast_module_count; i++) {
        rc = qip_ast_node_dump_stderr(module->ast_modules[i]);
        check(rc == 0, "Unable to dump AST module");
    }
    
    return 0;

error:
    return -1;
}
