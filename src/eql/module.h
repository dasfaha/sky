#ifndef _eql_module_h
#define _eql_module_h

#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

typedef struct eql_module eql_module;

#include "compiler.h"



//==============================================================================
//
// Typedefs
//
//==============================================================================

struct eql_module {
    eql_compiler *compiler;
    LLVMModuleRef llvm_module;
    LLVMValueRef llvm_function;
    LLVMExecutionEngineRef llvm_engine;
    LLVMPassManagerRef llvm_pass_manager;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

struct eql_module *eql_module_create(bstring name,
    struct eql_compiler *compiler);

void eql_module_free(struct eql_module *module);


//--------------------------------------
// Types
//--------------------------------------

int eql_module_get_type_ref(eql_module *module, bstring name,
    LLVMTypeRef *type);


//======================================
// Debugging
//======================================

int eql_module_dump(struct eql_module *module);

int eql_module_dump_to_file(eql_module *module, bstring filename);

#endif
