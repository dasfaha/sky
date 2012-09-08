#include <stdlib.h>
#include "dbg.h"

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
//
// Returns a module node.
qip_ast_node *qip_ast_module_create(bstring name,
                                    struct qip_ast_node **classes,
                                    unsigned int class_count, 
                                    struct qip_ast_node *main_function)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_MODULE;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->module.name = bstrcpy(name);
    if(name) check_mem(node->module.name);
    
    node->module.main_function = main_function;
    if(main_function != NULL) {
        main_function->parent = node;
    }

    // Copy classes.
    if(class_count > 0) {
        size_t sz = sizeof(qip_ast_node*) * class_count;
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
    
    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a function call AST node from memory.
//
// node - The AST node to free.
void qip_ast_module_free(struct qip_ast_node *node)
{
    if(node->module.name) bdestroy(node->module.name);
    node->module.name = NULL;
    
    if(node->module.main_function) qip_ast_node_free(node->module.main_function);
    node->module.main_function = NULL;
    
    if(node->module.class_count > 0) {
        unsigned int i;
        for(i=0; i<node->module.class_count; i++) {
            qip_ast_node_free(node->module.classes[i]);
            node->module.classes[i] = NULL;
        }
        free(node->module.classes);
        node->module.class_count = 0;
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_module_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_module_create(node->module.name, NULL, 0, NULL);
    check_mem(clone);

    // Copy classes.
    clone->module.class_count = node->module.class_count;
    clone->module.classes = calloc(clone->module.class_count, sizeof(*clone->module.classes));
    check_mem(clone->module.classes);
    for(i=0; i<clone->module.class_count; i++) {
        rc = qip_ast_node_copy(node->module.classes[i], &clone->module.classes[i]);
        check(rc == 0, "Unable to copy class");
        if(clone->module.classes[i]) clone->module.classes[i]->parent = clone;
    }

    rc = qip_ast_node_copy(node->module.main_function, &clone->module.main_function);
    check(rc == 0, "Unable to copy main function");
    if(clone->module.main_function) clone->module.main_function->parent = clone;
    
    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
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
int qip_ast_module_codegen(qip_ast_node *node, qip_module *module)
{
    int rc;
    unsigned int i;

    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_MODULE, "Node type must be 'module'");
    check(module != NULL, "Module required");

    // Codegen classes.
    for(i=0; i<node->module.class_count; i++) {
        qip_ast_node *class_ast = node->module.classes[i];
        rc = qip_ast_node_codegen(class_ast, module, NULL);
        check(rc == 0, "Unable to codegen class: %s", bdata(class_ast->class.name));
    }
    
    // Generate the main function if a block exists.
    if(node->module.main_function != NULL) {
        rc = qip_ast_node_codegen(node->module.main_function, module, NULL);
        check(rc == 0, "Unable to generate main function");
    }
    
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
int qip_ast_module_codegen_type(qip_module *module, qip_ast_node *node)
{
    int rc;
    unsigned int i;
    
    check(module != NULL, "Module required");
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_MODULE, "Node type must be 'module'");

    // Generate class type structures.
    for(i=0; i<node->module.class_count; i++) {
        qip_ast_node *class_ast = node->module.classes[i];
        rc = qip_ast_class_codegen_type(module, class_ast);
        check(rc == 0, "Unable to generate type for class: %s", bdata(class_ast->class.name));
    }
    
    return 0;
    
error:
    return -1;
}

// Recursively generates forward declarations for functions.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_module_codegen_forward_decl(qip_ast_node *node,
                                        qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_MODULE, "Node type must be 'module'");
    check(module != NULL, "Module required");

    // Codegen classes.
    unsigned int i;
    for(i=0; i<node->module.class_count; i++) {
        rc = qip_ast_node_codegen_forward_decl(node->module.classes[i], module);
        check(rc == 0, "Unable to codegen class forward declarations");
    }
    
    // Generate the main function if a block exists.
    if(node->module.main_function != NULL) {
        rc = qip_ast_node_codegen_forward_decl(node->module.main_function, module);
        check(rc == 0, "Unable to generate main function forward declarations");
    }
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Preprocess the node.
//
// node   - The node to validate.
// module - The module that the node is a part of.
// stage  - The processing stage.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_module_preprocess(qip_ast_node *node, qip_module *module,
                              qip_ast_processing_stage_e stage)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Preprocess class.
    uint32_t i;
    for(i=0; i<node->module.class_count; i++) {
        rc = qip_ast_node_preprocess(node->module.classes[i], module, stage);
        check(rc == 0, "Unable to preprocess module class");
    }

    // Preprocess main function.
    if(node->module.main_function != NULL) {
        rc = qip_ast_node_preprocess(node->module.main_function, module, stage);
        check(rc == 0, "Unable to preprocess main function");
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
int qip_ast_module_add_class(struct qip_ast_node *module,
                             struct qip_ast_node *class)
{
    // Validate.
    check(module != NULL, "Module is required");
    check(module->type == QIP_AST_TYPE_MODULE, "Module node is invalid type: %d", module->type);
    check(class != NULL, "Class is required");
    
    // Append class to module.
    module->module.class_count++;
    module->module.classes = realloc(module->module.classes, sizeof(qip_ast_node*) * module->module.class_count);
    check_mem(module->module.classes);
    module->module.classes[module->module.class_count-1] = class;
    
    // Assign class parent.
    class->parent = module;
    
    return 0;

error:
    return -1;
}

// Retrieves the a class within a given module.
//
// node - The node
// name - The name of the class.
// ret  - A pointer to where the class should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_module_get_class(qip_ast_node *node, bstring name,
                             qip_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_MODULE, "Node type must be 'module'");
    check(name != NULL, "Name required");
    check(ret != NULL, "Return pointer required");
    
    // Initialize the return value.
    *ret = NULL;
    
    // Loop over classes and match name.
    unsigned int i;
    for(i=0; i<node->module.class_count; i++) {
        // Return class if name matches.
        if(biseq(node->module.classes[i]->class.name, name)) {
            *ret = node->module.classes[i];
            break;
        }
    }
    
    return 0;

error:
    *ret = NULL;
    return -1;
}

// Retrieves a template class matching a type reference within a given module.
//
// node     - The node
// type_ref - The type reference to match.
// ret      - A pointer to where the class should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_module_get_template_class(qip_ast_node *node,
                                      qip_ast_node *type_ref,
                                      qip_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_MODULE, "Node type must be 'module'");
    check(type_ref != NULL, "Type reference required");
    check(type_ref->type == QIP_AST_TYPE_TYPE_REF, "Node type must be 'type ref'");
    check(type_ref->type_ref.subtype_count > 0, "Type reference must have subtypes");
    check(ret != NULL, "Return pointer required");
    
    // Initialize the return value.
    *ret = NULL;
    
    // Loop over classes and match name and the number of template variables.
    unsigned int i;
    for(i=0; i<node->module.class_count; i++) {
        qip_ast_node *class = node->module.classes[i];
        
        // Return class if match.
        if(biseq(class->class.name, type_ref->type_ref.name) &&
           class->class.template_var_count == type_ref->type_ref.subtype_count)
        {
            *ret = class;
            break;
        }
    }
    
    return 0;

error:
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Find
//--------------------------------------

// Computes a list of type refs within this node.
//
// node      - The node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_module_get_type_refs(qip_ast_node *node,
                                    qip_ast_node ***type_refs,
                                    uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Class type refs.
    uint32_t i;
    for(i=0; i<node->module.class_count; i++) {
        rc = qip_ast_node_get_type_refs(node->module.classes[i], type_refs, count);
        check(rc == 0, "Unable to add module class type refs");
    }

    // Main function type refs.
    if(node->module.main_function != NULL) {
        rc = qip_ast_node_get_type_refs(node->module.main_function, type_refs, count);
        check(rc == 0, "Unable to add main function type refs");
    }

    return 0;
    
error:
    qip_ast_node_type_refs_free(type_refs, count);
    return -1;
}

// Retrieves all variable reference of a given name within this node.
//
// node  - The node.
// name  - The variable name.
// array - The array to add the references to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_module_get_var_refs(qip_ast_node *node, bstring name,
                                qip_array *array)
{
    int rc;
    check(node != NULL, "Node required");
    check(name != NULL, "Variable name required");
    check(array != NULL, "Array required");

    uint32_t i;
    for(i=0; i<node->module.class_count; i++) {
        rc = qip_ast_node_get_var_refs(node->module.classes[i], name, array);
        check(rc == 0, "Unable to add module class var refs");
    }

    if(node->module.main_function != NULL) {
        rc = qip_ast_node_get_var_refs(node->module.main_function, name, array);
        check(rc == 0, "Unable to add main function var refs");
    }

    return 0;
    
error:
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
int qip_ast_module_get_dependencies(qip_ast_node *node,
                                    bstring **dependencies,
                                    uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Class dependencies.
    uint32_t i;
    for(i=0; i<node->module.class_count; i++) {
        rc = qip_ast_node_get_dependencies(node->module.classes[i], dependencies, count);
        check(rc == 0, "Unable to add module class dependency");
    }

    // Main function dependencies.
    if(node->module.main_function != NULL) {
        rc = qip_ast_node_get_dependencies(node->module.main_function, dependencies, count);
        check(rc == 0, "Unable to add main function dependencies");
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
int qip_ast_module_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Validate class.
    uint32_t i;
    for(i=0; i<node->module.class_count; i++) {
        rc = qip_ast_node_validate(node->module.classes[i], module);
        check(rc == 0, "Unable to validate module class");
    }

    // Validate main function.
    if(node->module.main_function != NULL) {
        rc = qip_ast_node_validate(node->module.main_function, module);
        check(rc == 0, "Unable to validate main function");
    }

    return 0;

error:
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
int qip_ast_module_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    bstring str = bformat("<module name='%s'>\n", bdatae(node->module.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    unsigned int i;
    for(i=0; i<node->module.class_count; i++) {
        rc = qip_ast_node_dump(node->module.classes[i], ret);
        check(rc == 0, "Unable to dump class");
    }
    if(node->module.main_function != NULL) {
        rc = qip_ast_node_dump(node->module.main_function, ret);
        check(rc == 0, "Unable to dump main function");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
