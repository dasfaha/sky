#include <stdlib.h>
#include <sys/stat.h>

#include "node.h"
#include "compiler.h"
#include "parser.h"
#include "dbg.h"


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

void qip_compiler_free_class_paths(qip_compiler *compiler);

void qip_compiler_free_dependencies(qip_compiler *compiler);


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a compiler.
qip_compiler *qip_compiler_create()
{
    qip_compiler *compiler = calloc(1, sizeof(qip_compiler));
    check_mem(compiler);
    compiler->llvm_builder = LLVMCreateBuilder();
    
    return compiler;
    
error:
    qip_compiler_free(compiler);
    return NULL;
}

// Frees an compiler.
//
// compiler - The compiler to free.
void qip_compiler_free(qip_compiler *compiler)
{
    if(compiler) {
        if(compiler->llvm_builder) LLVMDisposeBuilder(compiler->llvm_builder);
        compiler->llvm_builder = NULL;

        qip_compiler_free_class_paths(compiler);
        qip_compiler_free_dependencies(compiler);
        
        free(compiler);
    }
}

// Frees the class paths on the compiler.
//
// compiler - The compiler.
void qip_compiler_free_class_paths(qip_compiler *compiler)
{
    if(compiler) {
        uint32_t i;
        for(i=0; i<compiler->class_path_count; i++) {
            bdestroy(compiler->class_paths[i]);
            compiler->class_paths[i] = NULL;
        }
        free(compiler->class_paths);
        compiler->class_path_count = 0;
    }
}

// Frees the dependencies on the compiler.
//
// compiler - The compiler.
void qip_compiler_free_dependencies(qip_compiler *compiler)
{
    if(compiler) {
        uint32_t i;
        for(i=0; i<compiler->dependency_count; i++) {
            bdestroy(compiler->dependencies[i]);
            compiler->dependencies[i] = NULL;
        }
        free(compiler->dependencies);
        compiler->dependency_count = 0;
    }
}



//======================================
// Compile
//======================================

// Compiles QIP program text into a module.
//
// compiler  - The compiler object stores compiler settings such as include paths.
// name      - The name of the QIP module.
// source    - The QIP program source code.
// args      - A list of arguments to be passed into the main function.
// arg_count - The number of arguments.
// data      - A pointer to data passed to callbacks for reference.
// ret       - A pointer to where the compiled QIP module will return to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_compiler_compile(qip_compiler *compiler, bstring name,
                         bstring source, qip_ast_node **args, 
                         uint32_t arg_count, void *data, qip_module **ret)
{
    int rc;
    uint32_t i;
    check(compiler != NULL, "Compiler is required");
    
    // Initialize return value.
    *ret = NULL;
    
    // Create the module & parser.
    qip_parser *parser = NULL;
    qip_module *module = qip_module_create(name, compiler); check_mem(module);

    // Maintain a list of dependencies for all modules.
    bstring *dependencies = NULL;
    uint32_t dependency_count = 0;

    // Add standard dependencies.
    for(i=0; i<compiler->dependency_count; i++) {
        rc = qip_ast_node_add_dependency(compiler->dependencies[i], &dependencies, &dependency_count);
        check(rc == 0, "Unable to copy compiler dependency");
    }

    // Clear errors.
    qip_module_free_errors(module);

    // Continuously loop and parse QIP files until there are no more dependencies.
    bstring current_module_name = name;
    bstring current_module_source = source;
    do {
        // Throw error if no source can be found.
        check(current_module_source != NULL, "Source not found for '%s'", bdata(current_module_name));
        
        // Parse the text into a module AST.
        qip_ast_node *ast_module = NULL;
        parser = qip_parser_create(); check_mem(parser);
        rc = qip_parser_parse(parser, current_module_name, current_module_source, &ast_module);
        check(rc == 0, "Error occurred while parsing QIP query: %s", bdata(current_module_name));

        // If we have a syntax error then return the errors and exit.
        if(parser->error_count > 0) {
            module->errors = parser->errors;
            module->error_count = parser->error_count;
            parser->errors = NULL;
            parser->error_count = 0;
            qip_parser_free(parser);
            parser = NULL;
            break;
        }

        // If there is a main function then inject the arguments passed in.
        qip_ast_node *main_function = ast_module->module.main_function;
        if(main_function != NULL && arg_count > 0) {
            if(main_function->function.args) free(main_function->function.args);
            main_function->function.args = malloc(sizeof(*args) * arg_count);
            check_mem(main_function->function.args);
            main_function->function.arg_count = arg_count;
            
            for(i=0; i<arg_count; i++) {
                main_function->function.args[i] = args[i];
                args[i]->parent = main_function;
            }
        }

        // Append AST to list of AST modules.
        rc = qip_module_add_ast_module(module, ast_module);
        check(rc == 0, "Unable to add AST module to compilation module");
        
        // Perform any preprocessing that needs to be done on the AST.
        rc = qip_ast_node_preprocess(ast_module, module, QIP_AST_PROCESSING_STAGE_LOADING);
        check(rc == 0, "Unable to preprocess module (loading)");

        // Retrieve dependencies in module.
        rc = qip_ast_node_get_dependencies(ast_module, &dependencies, &dependency_count);
        check(rc == 0, "Unable to calculate dependencies for module: %s", bdata(current_module_name));

        // Clear out current module while we search for the next one.
        current_module_name   = NULL;
        current_module_source = NULL;
        
        // Loop over dependencies and find missing ones.
        for(i=0; i<dependency_count; i++) {
            bstring dependency = dependencies[i];
            
            // Find class among all loaded so far.
            qip_ast_node *class = NULL;
            rc = qip_module_get_ast_class(module, dependency, &class);
            check(rc == 0, "Unable to find class in compilation module");
            
            // If we couldn't find the dependency then load it.
            if(class == NULL) {
                current_module_name = dependency;
                rc = qip_compiler_load_module_source(compiler, dependency, &current_module_source);
                check(rc == 0, "Unable to load module source: %s", bdata(dependency));
                break;
            }
        }
        
        // Clean up for next pass.
        qip_parser_free(parser);
        parser = NULL;
        
    } while(current_module_name != NULL);
    
    // Process dynamic classes before proceeding to code generation.
    if(module->error_count == 0) {
        rc = qip_module_process_dynamic_classes(module, data);
        check(rc == 0, "Unable to process dynamic classes for module");
    }

    // Process templates.
    if(module->error_count == 0) {
        rc = qip_module_process_templates(module);
        check(rc == 0, "Unable to process module templates");
    }
    
    // qip_module_ast_dump_stderr(module);

    // Generate module types.
    if(module->error_count == 0) {
        for(i=0; i<module->ast_module_count; i++) {
            rc = qip_ast_module_codegen_type(module, module->ast_modules[i]);
            check(rc == 0, "Unable to generate types for module");
        }
    }

    // Perform any preprocessing that needs to be done once all the modules
    // are loaded.
    rc = qip_module_preprocess(module, QIP_AST_PROCESSING_STAGE_INITIALIZED);
    check(rc == 0, "Unable to preprocess module (initialized)");

    // Validate all AST nodes if we don't have any syntax errors.
    if(module->error_count == 0) {
        for(i=0; i<module->ast_module_count; i++) {
            rc = qip_ast_node_validate(module->ast_modules[i], module);
            check(rc == 0, "Unable to validate module");
        }
    }

    // Generate forward declarations.
    if(module->error_count == 0) {
        for(i=0; i<module->ast_module_count; i++) {
            rc = qip_ast_node_codegen_forward_decl(module->ast_modules[i], module);
            check(rc == 0, "Unable to generate forward declarations for module");
        }
    }

    // Generate module if there are no errors.
    if(module->error_count == 0) {
        // Generate module code.
        for(i=0; i<module->ast_module_count; i++) {
            rc = qip_ast_module_codegen(module->ast_modules[i], module);
            check(rc == 0, "Unable to codegen module");
        }
    }

    // qip_module_dump(module);
    
    *ret = module;
    return 0;

error:
    qip_parser_free(parser);
    qip_module_free(module);
    *ret = NULL;
    return -1;
}


//======================================
// Module Management
//======================================

// Adds a file path to the list of where the compiler should search for
// classes.
//
// compiler - The compiler that is loading the source.
// path     - The class path to search.
//
// Returns 0 if successful, otherwise returns -1.
int qip_compiler_add_class_path(qip_compiler *compiler, bstring path)
{
    check(compiler != NULL, "Compiler required");
    check(path != NULL, "Path required");
    
    // Append class path.
    compiler->class_path_count++;
    compiler->class_paths = realloc(compiler->class_paths, sizeof(*compiler->class_paths) * compiler->class_path_count);
    check_mem(compiler->class_paths);
    compiler->class_paths[compiler->class_path_count-1] = bstrcpy(path);
    
    return 0;
    
error:
    return -1;
}

// Loads the source code for a module with a given name. This function
// delegates to the load_module_source function pointer.
//
// compiler - The compiler that is loading the source.
// name     - The name of the QIP module.
// source   - A pointer to where the module's source text will be loaded to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_compiler_load_module_source(qip_compiler *compiler, bstring name,
                                    bstring *source)
{
    int rc;
    bstring path = NULL;
    check(compiler != NULL, "Compiler required");
    check(name != NULL, "Module name required");
    check(source != NULL, "Source return pointer required");
    
    // Initialize return value.
    *source = NULL;
    
    // Search class paths first.
    uint32_t i;
    for(i=0; i<compiler->class_path_count; i++) {
        bstring class_path = compiler->class_paths[i];
        
        // Create path to file.
        path = bformat("%s/%s.qip", bdata(class_path), bdata(name));
        check_mem(path);

        // Attempt to read file.
        FILE *fp = fopen(bdata(path), "rb");
        if(fp != NULL) {
            // Return contents to caller and clean up.
            bstring text = bread((bNread) fread, fp);
            fclose(fp);
            bdestroy(path);
            *source = text;
            return 0;
        }
        else {
            bdestroy(path);
        }
    }
    
    // Delegate to external interface.
    if(compiler->load_module_source != NULL) {
        rc = compiler->load_module_source(compiler, name, source);
        check(rc == 0, "Unable to load module source");
    }
    
    return 0;
    
error:
    bdestroy(path);
    *source = NULL;
    return -1;
}


//======================================
// Evaluate Dynamic Class
//======================================

// Processes a dynamic class by delegating it to the compiler caller through
// a callback. The caller can then add additional AST nodes to the class
// before the module is processed.
//
// compiler - The compiler.
// module   - The module.
// class    - The class AST node.
// data     - A pointer to an object that was passed into the compiler for
//            context.
//
// Returns 0 if successful, otherwise returns -1.
int qip_compiler_process_dynamic_class(qip_compiler *compiler, qip_module *module,
                                       qip_ast_node *class, void *data)
{
    int rc;
    check(compiler != NULL, "Compiler required");
    check(module != NULL, "Module required");
    check(class != NULL, "Class AST required");
    check(compiler->process_dynamic_class != NULL, "Process dynamic class function pointer must be set");
    
    // Delegate to external interface.
    rc = compiler->process_dynamic_class(module, class, data);
    check(rc == 0, "Unable to process dynamic class");
    
    return 0;
    
error:
    return -1;
}