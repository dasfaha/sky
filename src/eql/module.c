#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "module.h"
#include "../dbg.h"
#include "../mem.h"


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring EQL_TYPE_NAME_INT   = bsStatic("Int");

struct tagbstring EQL_TYPE_NAME_FLOAT = bsStatic("Float");

struct tagbstring EQL_TYPE_NAME_VOID  = bsStatic("void");


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a module.
eql_module *eql_module_create(bstring name, eql_compiler *compiler)
{
    eql_module *module = malloc(sizeof(eql_module));
    check_mem(module);
    module->compiler = compiler;
    module->llvm_module = LLVMModuleCreateWithName(bdata(name));
    module->llvm_function = NULL;
    module->llvm_engine = NULL;
    module->llvm_pass_manager = NULL;
	module->scopes = NULL;
	module->scope_count = 0;
	module->types = NULL;
	module->type_nodes = NULL;
	module->type_count = 0;
	
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

		if(module->scopes != NULL) free(module->scopes);
		module->scopes = NULL;
		module->scope_count = 0;

		if(module->types != NULL) free(module->types);
		module->types = NULL;
		module->type_count = 0;

        free(module);
    }
}


//--------------------------------------
// Types
//--------------------------------------

// Retrieves an LLVM type definition for a given type name. Optionally returns
// the associated AST node if the type is user-defined.
//
// module - The module that contains the type.
// name   - The name of the type to search for.
// node   - A pointer to where the AST node should be returned to. This is
//          optional.
// type   - A pointer to where the LLVM type should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_module_get_type_ref(eql_module *module, bstring name,
                            eql_ast_node **node, LLVMTypeRef *type)
{
	int i;
	
    check(module != NULL, "Module is required");
    check(name != NULL, "Type name is required");
    
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);

    // Compare to built-in types.
    bool found = false;
    if(biseq(name, &EQL_TYPE_NAME_INT)) {
        if(type != NULL) *type = LLVMInt64TypeInContext(context);
		if(node != NULL) *node = NULL;
        found = true;
    }
    else if(biseq(name, &EQL_TYPE_NAME_FLOAT)) {
        if(type != NULL) *type = LLVMDoubleTypeInContext(context);
		if(node != NULL) *node = NULL;
        found = true;
    }
    else if(biseq(name, &EQL_TYPE_NAME_VOID)) {
        if(type != NULL) *type = LLVMVoidTypeInContext(context);
		if(node != NULL) *node = NULL;
        found = true;
    }
	// Find user-defined type.
	else {
		for(i=0; i<module->type_count; i++) {
			if(biseq(module->type_nodes[i]->class.name, name)) {
				if(type != NULL) *type = module->types[i];
				if(node != NULL) *node = module->type_nodes[i];
                found = true;
				break;
			}
		}
    }

    check(found, "Invalid type in module: %s", bdata(name));

    return 0;

error:
    *type = NULL;
    return -1;
}

// Adds a type for a given class to the module.
//
// module - The compilation unit that contains the class.
// node   - The AST node associated with this type.
// type   - The LLVM type.
//
// Returns 0 if successful, otherwise returns -1.
int eql_module_add_type_ref(eql_module *module, eql_ast_node *node,
							LLVMTypeRef type)
{
	check(module != NULL, "Module required");
	check(node != NULL, "Node required");
	check(node->type == EQL_AST_TYPE_CLASS, "Node type must be 'class'");
	check(type != NULL, "LLVM type required");

    module->type_count++;

	// Append to the list of types.
    module->types = realloc(module->types, sizeof(LLVMTypeRef) * module->type_count);
    check_mem(module->types);
    module->types[module->type_count-1] = type;

	// Append to the list of AST nodes.
    module->type_nodes = realloc(module->type_nodes, sizeof(eql_ast_node*) * module->type_count);
    check_mem(module->type_nodes);
    module->type_nodes[module->type_count-1] = node;
	
	return 0;
	
error:
	return -1;
}


//--------------------------------------
// Scope
//--------------------------------------

// Adds a scope associated with an AST node to the scope stack of the module.
//
// module - The module to add the scope to.
// node   - The AST node to associate the scope with.
//
// Returns 0 if successful, otherwise returns -1.
int eql_module_push_scope(eql_module *module, eql_ast_node *node)
{
	check(module != NULL, "Module is required");
	check(node != NULL, "Node is required");
	
	// Create scope.
	eql_module_scope *scope = malloc(sizeof(eql_module_scope));
	check_mem(scope);
	scope->node = node;
	scope->var_values = NULL;
	scope->var_decls = NULL;
	scope->var_count = 0;
	
	// Resize scope stack and append.
    module->scope_count++;
    module->scopes = realloc(module->scopes, sizeof(eql_module_scope*) * module->scope_count);
    check_mem(module->scopes);
    module->scopes[module->scope_count-1] = scope;

	return 0;
	
error:
	if(scope) free(scope);
	return -1;
}

// Removes the current scope from the stack of the module. If the current
// scope is not associated with the given AST node then an error is returned.
//
// module - The module to remove the scope from.
// node   - The AST node associated with the scope being removed.
//
// Returns 0 if successful, otherwise returns -1.
int eql_module_pop_scope(eql_module *module, eql_ast_node *node)
{
	check(module != NULL, "Module is required");
	check(module->scope_count > 0, "Module has no more scopes");
	check(node != NULL, "Node is required");
	check(module->scopes[module->scope_count-1]->node == node, "Current scope does not match node");

	// Destroy current scope.
	eql_module_scope *scope = module->scopes[module->scope_count-1];
	scope->node = NULL;
	if(scope->var_values) free(scope->var_values);
	scope->var_values = NULL;
	if(scope->var_decls) free(scope->var_decls);
	scope->var_decls = NULL;
	scope->var_count = 0;
	free(scope);
	
	// Resize scope stack.
    module->scope_count--;

	return 0;
	
error:
	return -1;
	
}

// Searches the module scope stack for variables declared with the given name.
//
// module    - The module containing the variable declaration.
// name      - The name of the variable to search for.
// var_decl  - A pointer to where the variable declaration should be returned
//             to.
// value     - A pointer to where the LLVM value should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_module_get_variable(eql_module *module, bstring name,
    eql_ast_node **var_decl, LLVMValueRef *value)
{
	check(module != NULL, "Module is required");
	check(module->scope_count > 0, "Module has no scope");
	check(name != NULL, "Variable name is required");
	check(var_decl != NULL, "Variable declaration return pointer is required");
	check(value != NULL, "LLVM value return pointer is required");

	// Loop over scopes from the top down.
	int32_t i, j;
	for(i=module->scope_count-1; i>=0; i--) {
		eql_module_scope *scope = module->scopes[i];
		
		// Search scope for a variable with the given name.
		for(j=0; j<scope->var_count; j++) {
			if(biseq(scope->var_decls[j]->var_decl.name, name)) {
				*var_decl = scope->var_decls[j];
				*value    = scope->var_values[j];
				return 0;
			}
		}
	}

	// If we reach here then we couldn't find the variable in any scope.
	*var_decl = NULL;
	*value    = NULL;
	return 0;

error:
	*var_decl = NULL;
	*value    = NULL;
	return -1;
}

// Adds a variable declaration to the current scope of the module.
//
// module    - The module containing the variable declaration.
// var_decl  - The AST variable declaration to add.
// value     - The LLVM value associated with the declaration.
//
// Returns 0 if successful, otherwise returns -1.
int eql_module_add_variable(eql_module *module, eql_ast_node *var_decl,
	LLVMValueRef value)
{
	check(module != NULL, "Module is required");
	check(module->scope_count > 0, "Module has no scope");
	check(var_decl != NULL, "Variable declaration is required");
	check(var_decl->var_decl.name != NULL, "Variable declaration name is required");
	check(value != NULL, "LLVM value is required");

	// Find current scope.
	eql_module_scope *scope = module->scopes[module->scope_count-1];

	// Search for existing variable in scope.
	int32_t i;
	for(i=0; i<scope->var_count; i++) {
		if(biseq(scope->var_decls[i]->var_decl.name, var_decl->var_decl.name)) {
			sentinel("Variable already exists in scope: %s", bdata(var_decl->var_decl.name));
		}
	}

	// Append variable declaration & LLVM value to scope.
    scope->var_count++;
    scope->var_decls = realloc(scope->var_decls, sizeof(eql_ast_node*) * scope->var_count);
    check_mem(scope->var_decls);
    scope->var_decls[scope->var_count-1] = var_decl;

    scope->var_values = realloc(scope->var_values, sizeof(LLVMValueRef) * scope->var_count);
    check_mem(scope->var_values);
    scope->var_values[scope->var_count-1] = value;

	return 0;

error:
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
