#include <stdlib.h>

#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "../dbg.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a compiler.
eql_compiler *eql_compiler_create()
{
    eql_compiler *compiler = malloc(sizeof(eql_compiler));
    check_mem(compiler);
    compiler->llvm_builder = LLVMCreateBuilder();
    
    return compiler;
    
error:
    eql_compiler_free(compiler);
    return NULL;
}

// Frees an compiler.
//
// compiler - The compiler to free.
void eql_compiler_free(eql_compiler *compiler)
{
    if(compiler) {
        if(compiler->llvm_builder) LLVMDisposeBuilder(compiler->llvm_builder);
        compiler->llvm_builder = NULL;

        free(compiler);
    }
}


//======================================
// Compile
//======================================

// Compiles EQL program text into a module.
//
// compiler - The compiler object stores compiler settings such as include paths.
// name     - The name of the EQL module.
// text     - The EQL program text.
// module   - A pointer to where the compiled EQL module will return to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_compiler_compile(eql_compiler *compiler, bstring name,
                         bstring text, eql_module **module)
{
    int rc;
	unsigned int i;
	
    LLVMValueRef value;
    check(compiler != NULL, "Compiler is required");
    
    // Create the module.
    *module = eql_module_create(name, compiler);
    check_mem(*module);
    
    // Parse the text into an module AST.
    eql_ast_node *module_ast;
    rc = eql_parse(name, text, &module_ast);
    check(rc == 0, "Unable to parse EQL query");
    
	// Generate class type structures.
	for(i=0; i<module_ast->module.class_count; i++) {
		eql_ast_node *class_ast = module_ast->module.classes[i];
		rc = eql_ast_class_codegen_type(*module, class_ast);
		check(rc == 0, "Unable to generate type for class: %s", bdata(class_ast->class.name));
	}
	
    // TODO: Codegen classes.
    
    // Generate the main function if a block exists.
    rc = eql_ast_node_codegen(module_ast->module.main_function, *module, &value);
    check(rc == 0, "Unable to generate main function");
    
    return 0;

error:
    eql_module_free(*module);
    return -1;
}
