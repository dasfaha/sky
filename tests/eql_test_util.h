#include <dbg.h>

// Initializes and compiles a query into a module.
#define COMPILE_QUERY(MODULE, ARG_TYPE, ARG_NAME, QUERY) do {\
    uint32_t arg_count = 0; \
    eql_ast_node *args[1]; \
    if(ARG_TYPE) { \
        struct tagbstring arg_name = bsStatic(ARG_NAME); \
        eql_ast_node *type_ref = eql_ast_type_ref_create_cstr(ARG_TYPE); \
        eql_ast_node *var_decl = eql_ast_var_decl_create(type_ref, &arg_name, NULL); \
        args[0] = eql_ast_farg_create(var_decl); \
        arg_count++; \
    } \
    struct tagbstring query = bsStatic(QUERY); \
    struct tagbstring module_name = bsStatic("eql"); \
    struct tagbstring core_class_path = bsStatic("lib/core"); \
    struct tagbstring sky_class_path = bsStatic("lib/sky"); \
    eql_compiler *compiler = eql_compiler_create(); \
    eql_compiler_add_class_path(compiler, &core_class_path); \
    eql_compiler_add_class_path(compiler, &sky_class_path); \
    int _rc = eql_compiler_compile(compiler, &module_name, &query, args, arg_count, &module); \
    mu_assert(_rc == 0, "Unable to compile"); \
    if(module->error_count > 0) fprintf(stderr, "Parse error: %s\n", bdata(module->errors[0]->message)); \
    mu_assert_int_equals(module->error_count, 0); \
    eql_compiler_free(compiler); \
} while(0)