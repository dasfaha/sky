#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "module.h"
#include "../dbg.h"


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring TYPE_NAME_INT   = bsStatic("Int");
struct tagbstring TYPE_NAME_FLOAT = bsStatic("Float");
struct tagbstring TYPE_NAME_VOID  = bsStatic("void");


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a module.
eql_module *eql_module_create(bstring name, struct eql_compiler *compiler)
{
    eql_module *module = malloc(sizeof(eql_module));
    check_mem(module);
    module->compiler = compiler;
    module->llvm_module = LLVMModuleCreateWithName(bdata(name));
    module->llvm_function = NULL;
    module->llvm_engine = NULL;
    module->llvm_pass_manager = NULL;
    
    return module;
    
error:
    eql_module_free(module);
    return NULL;
}

// Frees a module.
//
// module - The module to free.
void eql_module_free(eql_module *module)
{
    if(module) {
        module->compiler = NULL;

    	if(module->llvm_pass_manager) LLVMDisposePassManager(module->llvm_pass_manager);
        module->llvm_pass_manager = NULL;

        if(module->llvm_module) LLVMDisposeModule(module->llvm_module);
        module->llvm_module = NULL;

        module->llvm_function = NULL;

        free(module);
    }
}


//--------------------------------------
// Types
//--------------------------------------

int eql_module_get_type_ref(eql_module *module, bstring name,
                            LLVMTypeRef *type)
{
    check(module != NULL, "Module is required");
    check(name != NULL, "Type name is required");
    
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);
    
    // Compare to built-in types.
    if(biseq(name, &TYPE_NAME_INT)) {
        *type = LLVMInt64TypeInContext(context);
    }
    else if(biseq(name, &TYPE_NAME_FLOAT)) {
        *type = LLVMDoubleTypeInContext(context);
    }
    else if(biseq(name, &TYPE_NAME_VOID)) {
        *type = LLVMVoidTypeInContext(context);
    }
    else {
        sentinel("Invalid type in module: %s", bdata(name));
    }

    return 0;

error:
    *type = NULL;
    return -1;
}


//======================================
// Debugging
//======================================

int eql_module_dump(eql_module *module)
{
    check(module != NULL, "Module is required");
    check(module->llvm_module != NULL, "Module must be compiled");

    LLVMDumpModule(module->llvm_module);
    return 0;
    
error:
    return -1;
}

int eql_module_dump_to_file(eql_module *module, bstring filename)
{
    check(filename != NULL, "Filename is required");
    
    // Redirect dump to file.
    bool opened = freopen(bdata(filename), "w", stderr);
    check(opened, "Unable to open file for dump");
    
    // Dump module.
    int rc = eql_module_dump(module);
    check(rc == 0, "Unable to dump module");

    // Close file and reassign stderr.
    dup2(1, 2);
    
    return 0;
    
error:
    // Clean up stderr switch if there was an error.
    dup2(1, 2);

    return -1;
}
