#include <stdlib.h>
#include "dbg.h"

#include "node.h"


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int eql_ast_metadata_codegen_method_external(eql_ast_node *node,
    eql_module *module);

int eql_ast_metadata_validate_external(eql_ast_node *node, eql_module *module);

int eql_ast_metadata_validate_enumerable(eql_ast_node *node, eql_module *module);

int eql_ast_metadata_validate_dynamic(eql_ast_node *node, eql_module *module);


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a metadata tag.
//
// name       - The name of the metadata tag.
// items      - The key/value pairs attached to the metadata.
// item_count - The number of key/value pairs attached to the metadata.
//
// Returns 0 if successful, otherwise returns -1.
eql_ast_node *eql_ast_metadata_create(bstring name,
                            struct eql_ast_node **items,
                            unsigned int item_count)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_METADATA;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->metadata.name = bstrcpy(name); check_mem(node->metadata.name);

    // Copy items.
    if(item_count > 0) {
        size_t sz = sizeof(eql_ast_node*) * item_count;
        node->metadata.items = malloc(sz);
        check_mem(node->metadata.items);
        
        unsigned int i;
        for(i=0; i<item_count; i++) {
            node->metadata.items[i] = items[i];
            items[i]->parent = node;
        }
    }
    else {
        node->metadata.items = NULL;
    }
    node->metadata.item_count = item_count;
    
    return node;

error:
    eql_ast_node_free(node);
    return NULL;
}

// Frees a metadata AST node from memory.
//
// node - The AST node to free.
void eql_ast_metadata_free(struct eql_ast_node *node)
{
    if(node->metadata.name) bdestroy(node->metadata.name);
    node->metadata.name = NULL;

    if(node->metadata.item_count > 0) {
        unsigned int i;
        for(i=0; i<node->metadata.item_count; i++) {
            eql_ast_node_free(node->metadata.items[i]);
            node->metadata.items[i] = NULL;
        }
        free(node->metadata.items);
        node->metadata.item_count = 0;
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_metadata_copy(eql_ast_node *node, eql_ast_node **ret)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    eql_ast_node *clone = eql_ast_metadata_create(node->metadata.name, NULL, 0);
    check_mem(clone);

    // Copy items.
    clone->metadata.item_count = node->metadata.item_count;
    clone->metadata.items = calloc(clone->metadata.item_count, sizeof(*clone->metadata.items));
    check_mem(clone->metadata.items);
    for(i=0; i<clone->metadata.item_count; i++) {
        rc = eql_ast_node_copy(node->metadata.items[i], &clone->metadata.items[i]);
        check(rc == 0, "Unable to copy item");
        if(clone->metadata.items[i]) clone->metadata.items[i]->parent = clone;
    }
    
    *ret = clone;
    return 0;

error:
    eql_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Metadata items
//--------------------------------------

// Retrieves the value of a metadata item with a given key.
//
// node  - The metadata node.
// key   - The name of the metadata item key.
// value - A pointer to where the metadata item's key should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_metadata_get_item_value(eql_ast_node *node, bstring key,
                                    bstring *value)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_METADATA, "Node type must be 'metadata'");
    check(value != NULL, "Value return pointer required");

    // Initialize the return value.
    *value = NULL;

    // Loop over metadata items until we find the right key.
    unsigned int i;
    for(i=0; i<node->metadata.item_count; i++) {
        eql_ast_node *item = node->metadata.items[i];
        if(biseq(key, item->metadata_item.key)) {
            *value = bstrcpy(item->metadata_item.value); check_mem(*value);
            break;
        }
    }
    
    return 0;

error:
    if(value) bdestroy(*value);
    if(value) *value = NULL;
    return -1;
}


//--------------------------------------
// Codegen (Forward Declarations)
//--------------------------------------

// Recursively generates forward declarations.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_metadata_codegen_forward_decl(eql_ast_node *node,
                                          eql_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_METADATA, "Node type must be 'metadata'");
    check(module != NULL, "Module required");

    // Method metadata.
    if(node->parent->type == EQL_AST_TYPE_METHOD) {
        // Generate forward declarations for external functions.
        if(biseqcstr(node->metadata.name, "External")) {
            rc = eql_ast_metadata_codegen_method_external(node, module);
            check(rc == 0, "Unable to codegen external method");
        }
    }
    
    return 0;

error:
    return -1;
}

// Generates a forward declaration for an external function.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_metadata_codegen_method_external(eql_ast_node *node,
                                             eql_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_METADATA, "Node type must be 'metadata'");
    check(module != NULL, "Module required");
    check(node->parent != NULL, "Node parent required");

    // Retrieve related method and function.
    eql_ast_node *method = node->parent;
    check(method->type == EQL_AST_TYPE_METHOD, "Parent node must be a method");
    eql_ast_node *function = method->method.function;
    check(function->type == EQL_AST_TYPE_FUNCTION, "Method function required");
    
    // Retrieve function name from metadata.
    bstring function_name = NULL;
    rc = eql_ast_metadata_get_item_value(node, NULL, &function_name);
    check(rc == 0, "Unable to retrieve function name from metadata");
    
    // Make sure we have an external function name.
    check(function_name != NULL, "External function name required");
    
    // Generate the forward declaration.
    LLVMValueRef func = NULL;
    rc = eql_ast_function_codegen_prototype_with_name(function, module, function_name, &func);
    check(rc == 0, "Unable to generate external forward declaration");

    bdestroy(function_name);
    return 0;

error:
    bdestroy(function_name);
    return -1;
}

    

//--------------------------------------
// Preprocessor
//--------------------------------------

// Preprocesses the node.
//
// node   - The node.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_metadata_preprocess(eql_ast_node *node, eql_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Preprocess arguments.
    uint32_t i;
    for(i=0; i<node->metadata.item_count; i++) {
        rc = eql_ast_node_preprocess(node->metadata.items[i], module);
        check(rc == 0, "Unable to preprocess metadata item");
    }

    return 0;

error:
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
int eql_ast_metadata_validate(eql_ast_node *node, eql_module *module)
{
    int rc;
    bstring msg = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // External.
    rc = eql_ast_metadata_validate_external(node, module);
    check(rc == 0, "Unable to validate external metadata");
    
    // Enumerable.
    rc = eql_ast_metadata_validate_enumerable(node, module);
    check(rc == 0, "Unable to validate enumerable metadata");

    // Dynamic.
    rc = eql_ast_metadata_validate_dynamic(node, module);
    check(rc == 0, "Unable to validate dynamic metadata");
    
    // Validate arguments.
    uint32_t i;
    for(i=0; i<node->metadata.item_count; i++) {
        rc = eql_ast_node_validate(node->metadata.items[i], module);
        check(rc == 0, "Unable to validate metadata item");
    }

    return 0;

error:
    bdestroy(msg);
    return -1;
}

// Validates an External metadata node.
//
// node   - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_metadata_validate_external(eql_ast_node *node, eql_module *module)
{
    int rc;
    bstring msg = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Only validate if this is an external node.
    if(biseqcstr(node->metadata.name, "External")) {
        // Retrieve null key value.
        bstring null_key_value = NULL;
        rc = eql_ast_metadata_get_item_value(node, NULL, &null_key_value);
        check(rc == 0, "Unable to retrieve null key item from metadata");

        // External metadata must have a name specified.
        if(null_key_value == NULL || blength(null_key_value) == 0) {
            msg = bfromcstr("External metadata must have a function name defined");
        }
    }
    
    // Add an error.
    if(msg != NULL) {
        rc = eql_module_add_error(module, node, msg);
        check(rc == 0, "Unable to add module error");
    }
    bdestroy(msg);
    return 0;

error:
    bdestroy(msg);
    return -1;
}

// Validates an Enumerable metadata node.
//
// node   - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_metadata_validate_enumerable(eql_ast_node *node, eql_module *module)
{
    int rc;
    bstring msg = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Validate Enumerable metatag.
    if(biseqcstr(node->metadata.name, "Enumerable")) {
        eql_ast_node *class = (node->parent && node->parent->type == EQL_AST_TYPE_CLASS ? node->parent : NULL);

        // Make sure the metatag is on a class.
        if(node->parent != NULL && class == NULL) {
            msg = bfromcstr("Enumerable metadata can only be defined on a class");
        }

        // Ensure class has a next() method.
        if(!msg) {
            struct tagbstring next_method_name = bsStatic("next");
            eql_ast_node *next_method = NULL;
            rc = eql_ast_class_get_method(class, &next_method_name, &next_method);
            check(rc == 0, "Unable to retrieve next method on class");
            
            if(next_method == NULL) {
                msg = bformat("Class '%s' must have a next() method if it is marked as enumerable", bdata(class->class.name));
            }
        }

        // Ensure class has an eof() method.
        if(!msg) {
            struct tagbstring eof_method_name = bsStatic("eof");
            eql_ast_node *eof_method = NULL;
            rc = eql_ast_class_get_method(class, &eof_method_name, &eof_method);
            check(rc == 0, "Unable to retrieve eof method on class");
            
            if(eof_method == NULL) {
                msg = bformat("Class '%s' must have an eof() method if it is marked as enumerable", bdata(class->class.name));
            }
        }
    }
    
    // Add an error.
    if(msg != NULL) {
        rc = eql_module_add_error(module, node, msg);
        check(rc == 0, "Unable to add module error");
    }
    bdestroy(msg);
    return 0;

error:
    bdestroy(msg);
    return -1;
}

// Validates a Dynamic metadata node.
//
// node   - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_metadata_validate_dynamic(eql_ast_node *node, eql_module *module)
{
    int rc;
    bstring msg = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Validate Dynamic metatag.
    if(biseqcstr(node->metadata.name, "Dynamic")) {
        eql_ast_node *class = (node->parent && node->parent->type == EQL_AST_TYPE_CLASS ? node->parent : NULL);

        // Make sure the metatag is on a class.
        if(node->parent != NULL && class == NULL) {
            msg = bfromcstr("Dynamic metadata can only be defined on a class");
        }
    }
    
    // Add an error.
    if(msg != NULL) {
        rc = eql_module_add_error(module, node, msg);
        check(rc == 0, "Unable to add module error");
    }
    bdestroy(msg);
    return 0;

error:
    bdestroy(msg);
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
int eql_ast_metadata_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    bstring str = bformat("<metadata name='%s'>\n", bdatae(node->metadata.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    unsigned int i;
    for(i=0; i<node->metadata.item_count; i++) {
        rc = eql_ast_node_dump(node->metadata.items[i], ret);
        check(rc == 0, "Unable to dump metadata item");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
