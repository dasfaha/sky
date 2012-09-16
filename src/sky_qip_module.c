#include <stdlib.h>

#include "sky_qip_module.h"
#include "property.h"
#include "constants.h"
#include "minipack.h"
#include "mem.h"
#include "dbg.h"

#include "qip/qip.h"
#include "qip_path.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

typedef void (*sky_qip_path_map_func)(sky_qip_path *path, qip_map *map);
typedef void (*sky_qip_result_serialize_func)(void *result, qip_serializer *serializer);

typedef void (*sky_qip_update_dynamic_offsets_func)(void *event, qip_fixed_array *offsets);


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int sky_qip_module_process_dynamic_class_callback(qip_module *module,
    qip_ast_node *class);

int sky_qip_module_process_event_class(sky_qip_module *module,
    qip_ast_node *class);


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates a wrapped module.
//
// Returns a new module.
sky_qip_module *sky_qip_module_create()
{
    struct tagbstring core_class_path = bsStatic(SKY_LIB_CORE_PATH);
    struct tagbstring sky_class_path = bsStatic(SKY_LIB_SKY_PATH);

    sky_qip_module *module = NULL;
    module = calloc(1, sizeof(sky_qip_module)); check_mem(module);
    
    // Setup compiler.
    module->compiler = qip_compiler_create(); check_mem(module->compiler);
    module->compiler->process_dynamic_class = sky_qip_module_process_dynamic_class_callback;
    module->compiler->dependency_count = 1;
    module->compiler->dependencies = calloc(module->compiler->dependency_count, sizeof(*module->compiler->dependencies));
    module->compiler->dependencies[0] = bfromcstr("String");
    qip_compiler_add_class_path(module->compiler, &core_class_path);
    qip_compiler_add_class_path(module->compiler, &sky_class_path);

    return module;

error:
    sky_qip_module_free(module);
    return NULL;
}

// Frees a wrapped module object from memory.
//
// module - The module.
//
// Returns nothing.
void sky_qip_module_free(sky_qip_module *module)
{
    if(module) {
        sky_qip_module_free_event_info(module);
        qip_compiler_free(module->compiler);
        qip_module_free(module->_qip_module);
        free(module);
    }
}

// Frees dynamic event data from the module.
//
// module - The module.
//
// Returns nothing.
void sky_qip_module_free_event_info(sky_qip_module *module)
{
    if(module) {
        free(module->event_property_ids);
        module->event_property_ids = NULL;
        free(module->event_property_offsets);
        module->event_property_offsets = NULL;
        free(module->event_property_types);
        module->event_property_types = NULL;
        
        module->event_property_count = 0;
    }
}


//--------------------------------------
// Dynamic Event Property Processing
//--------------------------------------

// Preprocesses dynamic classes to add database properties.
//
// module   - The Qip module.
// class    - The class AST node.
//
// Returns 0 if successful, otherwise returns -1.
int sky_qip_module_process_dynamic_class_callback(qip_module *module,
                                                  qip_ast_node *class)
{
    int rc;
    check(module != NULL, "Module required");
    check(class != NULL, "Class required");

    // Extract wrapped module via context.
    sky_qip_module *wrapped_module = module->context;
    check(wrapped_module != NULL, "Module context required");
    
    // Process Event class only.
    if(biseqcstr(class->class.name, "Event")) {
        rc = sky_qip_module_process_event_class(wrapped_module, class);
    }
    
    return 0;

error:
    return -1;
}

// Preprocesses dynamic classes to add database properties.
//
// module   - The wrapped module.
// class    - The class AST node.
//
// Returns 0 if successful, otherwise returns -1.
int sky_qip_module_process_event_class(sky_qip_module *module,
                                       qip_ast_node *class)
{
    int rc;
    uint32_t i;
    check(module != NULL, "Module required");
    check(class != NULL, "Class required");
    
    struct tagbstring this_str = bsStatic("this");
    struct tagbstring event_str = bsStatic("Event");
    struct tagbstring udo_str = bsStatic("updateDynamicOffsets");
    struct tagbstring offsets_str = bsStatic("offsets");
    struct tagbstring set_item_at_str = bsStatic("setItemAt");

    // Retrieve updateDynamicOffsets() method.
    qip_ast_node *udo_method = NULL;
    rc = qip_ast_class_get_method(class, &udo_str, &udo_method);
    check(rc == 0 && udo_method != NULL, "Unable to find Event.updateDynamicOffsets() method");
    qip_ast_node *udo_block = udo_method->method.function->function.body;
    qip_ast_block_free_exprs(udo_block);
    
    // Loop over each module to find Event variable references.
    qip_array *var_refs = qip_array_create(); check_mem(var_refs);
    for(i=0; i<module->_qip_module->ast_module_count; i++) {
        qip_ast_node *ast_module = module->_qip_module->ast_modules[i];
        rc = qip_ast_node_get_var_refs_by_type(ast_module, module->_qip_module, &event_str, var_refs);
        check(rc == 0, "Unable to search for Event variable references");
    }

    // Loop over references and add properties as needed.
    module->event_property_count = 0;
    
    for(i=0; i<var_refs->length; i++) {
        qip_ast_node *var_ref = (qip_ast_node*)var_refs->elements[i];
        
        // Add property if member is a property reference.
        if(var_ref->var_ref.member && var_ref->var_ref.member->var_ref.type == QIP_AST_VAR_REF_TYPE_VALUE) {
            bstring property_name = var_ref->var_ref.member->var_ref.name;
            
            // Check if it exists on the Event class.
            qip_ast_node *property = NULL;
            rc = qip_ast_class_get_property(class, property_name, &property);
            check(rc == 0, "Unable to retrieve property from Event class");
            
            // Only add one if it doesn't exist.
            if(property == NULL) {
                // Lookup property in the database.
                sky_property *db_property = NULL;
                rc = sky_property_file_find_by_name(module->table->property_file, property_name, &db_property);
                check(rc == 0, "Unable to find property '%s' in table: %s", bdata(property_name), bdata(module->table->path));
                
                // Generate and add property to class.
                property = qip_ast_property_create(QIP_ACCESS_PUBLIC, 
                    qip_ast_var_decl_create(qip_ast_type_ref_create(db_property->data_type), property_name, NULL)
                );
                rc = qip_ast_class_add_property(class, property);
                check(rc == 0, "Unable to add property to class");
                
                // Add to db property id to updateDynamicOffsets() method:
                //
                //   offsets.setItemAt(offsetof(this.<db-prop-name>), <index>);
                qip_ast_node *args[2];
                args[0] = qip_ast_offsetof_create(
                    qip_ast_var_ref_create_property_access(&this_str, property_name)
                );
                args[1] = qip_ast_int_literal_create(module->event_property_count);
                rc = qip_ast_block_add_expr(udo_block, qip_ast_var_ref_create_method_invoke(&offsets_str, &set_item_at_str, args, 2));
                check(rc == 0, "Unable to add property id to updateDynamicOffsets()");

                // Increment counter.
                module->event_property_count++;

                // Append to property id array.
                module->event_property_ids = realloc(module->event_property_ids, sizeof(*module->event_property_ids) * module->event_property_count);
                module->event_property_ids[module->event_property_count-1] = db_property->id;

                // Append to property type array.
                bstring type_name = property->property.var_decl->var_decl.type->type_ref.name;
                module->event_property_types = realloc(module->event_property_types, sizeof(*module->event_property_types) * module->event_property_count);
                rc = sky_property_get_standard_data_type_name(type_name, &module->event_property_types[module->event_property_count-1]);
                check(rc == 0, "Unable to retrieve standard type name: '%s'", bdata(type_name));
            }
        }
    }
    
    // Append void return for dynamic array method.
    rc = qip_ast_block_add_expr(udo_block, qip_ast_freturn_create(NULL));
    check(rc == 0, "Unable to add return to updateDynamicOffsets() block");
   
    qip_array_free(var_refs);
    return 0;

error:
    qip_array_free(var_refs);
    return -1;
}

// Compiles a Qip query against the module. This can only be performed once
// on a module. Modules cannot be reused.
//
// module     - The wrapped module.
// query_text - The text content of the query to compile.
//
// Returns 0 if successful, otherwise returns -1.
int sky_qip_module_compile(sky_qip_module *module, bstring query_text)
{
    int rc;
    check(module != NULL, "Module required");
    check(module->_qip_module == NULL, "Module cannot be reused");
    check(query_text != NULL, "Query text required");

    struct tagbstring event_str = bsStatic("Event");
    struct tagbstring udo_str = bsStatic("updateDynamicOffsets");

    // Compile query and execute.
    qip_ast_node *type_ref, *var_decl;
    qip_ast_node *args[2];
    
    // Path argument.
    struct tagbstring path_str = bsStatic("path");
    type_ref = qip_ast_type_ref_create_cstr("Path");
    var_decl = qip_ast_var_decl_create(type_ref, &path_str, NULL);
    args[0] = qip_ast_farg_create(var_decl);
    
    // Data argument.
    struct tagbstring data_str = bsStatic("data");
    type_ref = qip_ast_type_ref_create_cstr("Map");
    qip_ast_type_ref_add_subtype(type_ref, qip_ast_type_ref_create_cstr("Int"));
    qip_ast_type_ref_add_subtype(type_ref, qip_ast_type_ref_create_cstr("Result"));
    var_decl = qip_ast_var_decl_create(type_ref, &data_str, NULL);
    args[1] = qip_ast_farg_create(var_decl);

    // Compile.
    struct tagbstring module_name = bsStatic("sky");
    module->_qip_module = qip_module_create(&module_name, module->compiler);
    module->_qip_module->context = module;
    rc = qip_compiler_compile(module->compiler, module->_qip_module, query_text, args, 2);
    check(rc == 0, "Unable to compile");
    if(module->_qip_module->error_count > 0) {
        debug("Parse error [line %d] %s", module->_qip_module->errors[0]->line_no, bdata(module->_qip_module->errors[0]->message));
    }
    check(module->_qip_module->error_count == 0, "Parse errors found");

    // Retrieve Event functions for dynamic property info.
    sky_qip_update_dynamic_offsets_func update_dynamic_offsets = NULL;
    rc = qip_module_get_class_method(module->_qip_module, &event_str, &udo_str, (void*)&update_dynamic_offsets);
    check(rc == 0 && update_dynamic_offsets != NULL, "Unable to find Event.updateDynamicOffsets() function");

    // Retrieve dynamic property info.
    qip_fixed_array *offsets = qip_fixed_array_create(sizeof(int64_t), module->event_property_count);
    update_dynamic_offsets(NULL, offsets);

    // Simplify arrays.
    module->event_property_offsets = (int64_t*)offsets->elements;
    offsets->elements = NULL;
    qip_fixed_array_free(offsets);

    // Print dynamic info.
    debug("dynamic property count: %lld", module->event_property_count);
    int64_t j;
    for(j=0; j<module->event_property_count; j++) {
        debug("  [%lld] id: %d, offset: %lld, type: %s [%p]", j, module->event_property_ids[j], module->event_property_offsets[j], bdata(module->event_property_types[j]), module->event_property_types[j]);
    }
    
    // Retrieve main function.
    rc = qip_module_get_main_function(module->_qip_module, &module->main_function);
    check(rc == 0, "Unable to retrieve main function");

    return 0;

error:
    return -1;
}
 
