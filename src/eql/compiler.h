#ifndef _eql_compiler_h
#define _eql_compiler_h

#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include "../bstring.h"


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

typedef struct eql_compiler eql_compiler;

#include "module.h"


//==============================================================================
//
// Typedefs
//
//==============================================================================

struct eql_compiler {
    LLVMBuilderRef llvm_builder;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

eql_compiler *eql_compiler_create();

void eql_compiler_free(eql_compiler *compiler);


//======================================
// Compile
//======================================

int eql_compiler_compile(eql_compiler *compiler, bstring name, bstring text,
    eql_module **ret);


#endif
