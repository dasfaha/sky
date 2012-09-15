#include <dbg.h>

// Required for Dynamic Event class.
int dynamic_class_callback(qip_module *module, qip_ast_node *class)
{
    check(module != NULL, "Module required");
    check(class != NULL, "Class required");
    
    return 0;

error:
    return -1;
}

// Initializes and compiles a query into a module.
#define COMPILE_QUERY_RAW(MODULE, ARGS, ARG_COUNT, QUERY) do {\
    struct tagbstring query = bsStatic(QUERY); \
    struct tagbstring core_class_path = bsStatic("lib/core"); \
    struct tagbstring sky_class_path = bsStatic("lib/sky"); \
    qip_compiler *compiler = qip_compiler_create(); \
    compiler->process_dynamic_class = dynamic_class_callback; \
    qip_compiler_add_class_path(compiler, &core_class_path); \
    qip_compiler_add_class_path(compiler, &sky_class_path); \
    MODULE->compiler = compiler; \
    int _rc = qip_compiler_compile(compiler, module, &query, ARGS, ARG_COUNT); \
    mu_assert(_rc == 0, "Unable to compile"); \
    if(module->error_count > 0) fprintf(stderr, "Parse error [line %d] %s\n", module->errors[0]->line_no, bdata(module->errors[0]->message)); \
    mu_assert_int_equals(module->error_count, 0); \
    qip_compiler_free(compiler); \
} while(0)

// Initializes and compiles a query into a module.
#define COMPILE_QUERY_2ARG(MODULE, ARG_TYPE1, ARG_NAME1, ARG_TYPE2, ARG_NAME2, QUERY) do {\
    uint32_t arg_count = 0; \
    qip_ast_node *args[2]; \
    if(strlen(ARG_TYPE1) > 0) { \
        struct tagbstring arg_name = bsStatic(ARG_NAME1); \
        qip_ast_node *type_ref = qip_ast_type_ref_create_cstr(ARG_TYPE1); \
        qip_ast_node *var_decl = qip_ast_var_decl_create(type_ref, &arg_name, NULL); \
        args[0] = qip_ast_farg_create(var_decl); \
        arg_count++; \
    } \
    if(strlen(ARG_TYPE2) > 0) { \
        struct tagbstring arg_name = bsStatic(ARG_NAME2); \
        qip_ast_node *type_ref = qip_ast_type_ref_create_cstr(ARG_TYPE2); \
        qip_ast_node *var_decl = qip_ast_var_decl_create(type_ref, &arg_name, NULL); \
        args[1] = qip_ast_farg_create(var_decl); \
        arg_count++; \
    } \
    COMPILE_QUERY_RAW(MODULE, args, arg_count, QUERY); \
} while(0)

#define COMPILE_QUERY_1ARG(MODULE, ARG_TYPE, ARG_NAME, QUERY) COMPILE_QUERY_2ARG(MODULE, ARG_TYPE, ARG_NAME, "", "", QUERY)
#define COMPILE_QUERY(MODULE, QUERY) COMPILE_QUERY_1ARG(MODULE, "", "", QUERY)
