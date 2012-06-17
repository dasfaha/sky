#include <stdlib.h>
#include "../../dbg.h"

#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a module.
//
// name          - The name of the module.
// classes       - An array of classes attached to the module.
// class_count   - The number of classes.
// main_function - A function containing the AST nodes outside any class.
// ret           - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_module_create(bstring name,
                         struct eql_ast_node **classes,
                         unsigned int class_count, 
                         struct eql_ast_node *main_function,
                         struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_MODULE;
    node->parent = NULL;
    node->module.name = bstrcpy(name);
    if(name) check_mem(node->module.name);
    
    node->module.main_function = main_function;
    if(main_function != NULL) {
        main_function->parent = node;
    }

    // Copy classes.
    if(class_count > 0) {
        size_t sz = sizeof(eql_ast_node*) * class_count;
        node->module.classes = malloc(sz);
        check_mem(node->module.classes);
        
        unsigned int i;
        for(i=0; i<class_count; i++) {
            node->module.classes[i] = classes[i];
            classes[i]->parent = node;
        }
    }
    else {
        node->module.classes = NULL;
    }
    node->module.class_count = class_count;
    
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a function call AST node from memory.
//
// node - The AST node to free.
void eql_ast_module_free(struct eql_ast_node *node)
{
    if(node->module.name) bdestroy(node->module.name);
    node->module.name = NULL;
    
    if(node->module.main_function) eql_ast_node_free(node->module.main_function);
    node->module.main_function = NULL;
    
    if(node->module.class_count > 0) {
        unsigned int i;
        for(i=0; i<node->module.class_count; i++) {
            eql_ast_node_free(node->module.classes[i]);
            node->module.classes[i] = NULL;
        }
        free(node->module.classes);
        node->module.class_count = 0;
    }
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the module AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_module_codegen(eql_ast_node *node, eql_module *module)
{
	int rc;
    unsigned int i;

	check(node != NULL, "Node required");
	check(node->type == EQL_AST_TYPE_MODULE, "Node type must be 'module'");
	check(module != NULL, "Module required");

    // Codegen classes.
	for(i=0; i<node->module.class_count; i++) {
		eql_ast_node *class_ast = node->module.classes[i];
		rc = eql_ast_node_codegen(class_ast, module, NULL);
		check(rc == 0, "Unable to codegen class: %s", bdata(class_ast->class.name));
	}
    
    // Generate the main function if a block exists.
    rc = eql_ast_node_codegen(node->module.main_function, module, NULL);
    check(rc == 0, "Unable to generate main function");
    
    return 0;

error:
    return -1;
}

// Generates and registers an LLVM struct for a given AST class. This struct
// can then be referenced within the module as the class' type.
//
// module - The compilation unit that contains the class.
// node   - The class AST node.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_module_codegen_type(eql_module *module, eql_ast_node *node)
{
	int rc;
	unsigned int i;
	
	check(module != NULL, "Module required");
	check(node != NULL, "Node required");
	check(node->type == EQL_AST_TYPE_MODULE, "Node type must be 'module'");

 	// Generate class type structures.
	for(i=0; i<node->module.class_count; i++) {
		eql_ast_node *class_ast = node->module.classes[i];
		rc = eql_ast_class_codegen_type(module, class_ast);
		check(rc == 0, "Unable to generate type for class: %s", bdata(class_ast->class.name));
	}
	
	return 0;
	
error:
	return -1;
}


//--------------------------------------
// Class Management
//--------------------------------------

// Adds a class to the module.
//
// module - The module to add the class to.
// class  - The class to add.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_module_add_class(struct eql_ast_node *module,
                             struct eql_ast_node *class)
{
    // Validate.
    check(module != NULL, "Module is required");
    check(module->type == EQL_AST_TYPE_MODULE, "Module node is invalid type: %d", module->type);
    check(class != NULL, "Class is required");
    
    // Append class to module.
    module->module.class_count++;
    module->module.classes = realloc(module->module.classes, sizeof(eql_ast_node*) * module->module.class_count);
    check_mem(module->module.classes);
    module->module.classes[module->module.class_count-1] = class;
    
    // Assign class parent.
    class->parent = module;
    
    return 0;

error:
    return -1;
}
