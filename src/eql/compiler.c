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
	
    check(compiler != NULL, "Compiler is required");
    
    // Create the module.
    *module = eql_module_create(name, compiler);
    check_mem(*module);

    // TODO: Convert to use parser.
    // TODO: Continuously loop and parse EQL files until there are no more dependencies.
    
    // Parse the text into an module AST.
    eql_ast_node *module_ast;
    rc = eql_parse(name, text, &module_ast);
    check(rc == 0, "Unable to parse EQL query");

    // TODO: Append each AST to module.

    // TODO: Validate all ASTs.
    
    // Generate module types.
    rc = eql_ast_module_codegen_type(*module, module_ast);
    check(rc == 0, "Unable to generate types for module");
    
    // Generate module code.
    rc = eql_ast_module_codegen(module_ast, *module);
    check(rc == 0, "Unable to codegen module");
	
    return 0;

error:
    eql_module_free(*module);
    return -1;
}
