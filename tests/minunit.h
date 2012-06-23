#include <bstring.h>

//==============================================================================
//
// Minunit
//
//==============================================================================

#define mu_fail(MSG, ...) do {\
    fprintf(stderr, "%s:%d: " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__);\
    return 1;\
} while(0)

#define mu_assert(TEST, MSG, ...) do {\
    if (!(TEST)) {\
        fprintf(stderr, "%s:%d: %s " MSG "\n", __FILE__, __LINE__, #TEST, ##__VA_ARGS__);\
        return 1;\
    }\
} while (0)

#define mu_run_test(TEST) do {\
    fprintf(stderr, "%s\n", #TEST);\
    int rc = TEST();\
    tests_run++; \
    if (rc) {\
        fprintf(stderr, "\n  Test Failure: %s()\n", #TEST);\
        return rc;\
    }\
} while (0)

#define RUN_TESTS() int main() {\
   fprintf(stderr, "== %s ==\n", __FILE__);\
   int rc = all_tests();\
   fprintf(stderr, "\n");\
   return rc;\
}

int tests_run;



//==============================================================================
//
// Constants
//
//==============================================================================

// The temporary file used for file operations in the test suite.
#define TEMPFILE "/tmp/skytemp"

// The dump file for EQL IR.
struct tagbstring DUMPFILE = bsStatic("/tmp/eqldump");


//==============================================================================
//
// Helpers
//
//==============================================================================

// Empties the tmp/db directory.
#define cleandb() \
    system("tests/cleandb.sh")
    
// Copy a database from the fixtures directory into the tmp/db directory.
#define copydb(DB) \
    char _copydb_cmd[1024]; \
    snprintf(_copydb_cmd, 1024, "tests/copydb.sh %s", DB); \
    system(_copydb_cmd)
    
// Asserts that a block has a specific block id and object id range.
#define mu_assert_block_info(INDEX, ID, MIN_OBJECT_ID, MAX_OBJECT_ID, MIN_TIMESTAMP, MAX_TIMESTAMP, SPANNED) \
    mu_assert(table->infos[INDEX]->id == ID, "Block " #INDEX " id expected to be " #ID); \
    mu_assert(table->infos[INDEX]->min_object_id == MIN_OBJECT_ID, "Block " #INDEX " min object id expected to be " #MIN_OBJECT_ID); \
    mu_assert(table->infos[INDEX]->max_object_id == MAX_OBJECT_ID, "Block " #INDEX " max object id expected to be " #MAX_OBJECT_ID); \
    mu_assert(table->infos[INDEX]->min_timestamp == MIN_TIMESTAMP, "Block " #INDEX " min timestamp expected to be " #MIN_TIMESTAMP); \
    mu_assert(table->infos[INDEX]->max_timestamp == MAX_TIMESTAMP, "Block " #INDEX " max timestamp expected to be " #MAX_TIMESTAMP); \
    mu_assert(table->infos[INDEX]->spanned == SPANNED, "Block " #INDEX " spanned expected to be " #SPANNED);

// Asserts action data.
#define mu_assert_action(INDEX, ID, NAME) \
    mu_assert(table->actions[INDEX]->id == ID, "Expected action #" #INDEX " id to be: " #ID); \
    mu_assert(biseqcstr(table->actions[INDEX]->name, NAME) == 1, "Expected action #" #INDEX " name to be: " #NAME);

// Asserts property data.
#define mu_assert_property(INDEX, ID, NAME) \
    mu_assert(table->properties[INDEX]->id == ID, "Expected property #" #INDEX " id to be: " #ID); \
    mu_assert(biseqcstr(table->properties[INDEX]->name, NAME) == 1, "Expected property #" #INDEX " name to be: " #NAME);

// Asserts the contents of the temp file.
#define mu_assert_file(FILENAME1, FILENAME2) do {\
    unsigned char ch1, ch2; \
    FILE *file1 = fopen(FILENAME1, "r"); \
    FILE *file2 = fopen(FILENAME2, "r"); \
    if(file1 == NULL) mu_fail("Cannot open file 1: %s", FILENAME1); \
    if(file2 == NULL) mu_fail("Cannot open file 2: %s", FILENAME2); \
    while(1) { \
        fread(&ch2, 1, 1, file2); \
        fread(&ch1, 1, 1, file1); \
        if(feof(file2) || feof(file1)) break; \
        if(ch1 != ch2) { \
            mu_fail("Expected 0x%02x (%s), received 0x%02x (%s) at location %ld", ch2, FILENAME2, ch1, FILENAME1, (ftell(file2)-1)); \
        } \
    } \
    if(!feof(file1)) mu_fail("Expected file length longer than expected: %s", FILENAME2); \
    if(!feof(file2)) mu_fail("Expected file length shorter than expected: %s", FILENAME2); \
    fclose(file1); \
    fclose(file2); \
} while(0)

// Asserts the contents of the temp file.
#define mu_assert_tempfile(EXP_FILENAME) \
    mu_assert_file(TEMPFILE, EXP_FILENAME)

// Asserts the contents of the eql dump file.
#define mu_assert_eql_compile(QUERY, EXP_FILENAME) \
    eql_module *module; \
    bstring module_name = bfromcstr("foo"); \
    bstring text = bfromcstr(QUERY); \
    eql_compiler *compiler = eql_compiler_create(); \
    int rc = eql_compiler_compile(compiler, module_name, text, &module); \
    mu_assert(rc == 0, "Unable to compile"); \
    eql_module_dump_to_file(module, &DUMPFILE); \
    mu_assert_file(bdata(&DUMPFILE), EXP_FILENAME); \
    eql_module_free(module); \
    eql_compiler_free(compiler); \
    bdestroy(module_name); \
    bdestroy(text);

// Executes an EQL query that returns an Int.
#define mu_assert_eql_execute_int(QUERY, EXPECTED) \
    int rc; \
    int64_t ret; \
    eql_module *module; \
    bstring module_name = bfromcstr("eql"); \
    bstring text = bfromcstr(QUERY); \
    eql_compiler *compiler = eql_compiler_create(); \
    rc = eql_compiler_compile(compiler, module_name, text, &module); \
    mu_assert(rc == 0, "Unable to compile"); \
    rc = eql_module_execute_int(module, &ret); \
    mu_assert(rc == 0, "Unable to execute"); \
    mu_assert(ret == EXPECTED, "Unexpected return value: %lld", ret); \
    eql_module_free(module); \
    eql_compiler_free(compiler); \
    bdestroy(module_name); \
    bdestroy(text);

// Executes an EQL query that returns a Float.
#define mu_assert_eql_execute_float(QUERY, EXPECTED) \
    int rc; \
    double ret; \
    eql_module *module; \
    bstring module_name = bfromcstr("eql"); \
    bstring text = bfromcstr(QUERY); \
    eql_compiler *compiler = eql_compiler_create(); \
    rc = eql_compiler_compile(compiler, module_name, text, &module); \
    mu_assert(rc == 0, "Unable to compile"); \
    rc = eql_module_execute_float(module, &ret); \
    mu_assert(rc == 0, "Unable to execute"); \
    mu_assert(ret == EXPECTED, "Unexpected return value: %f", ret); \
    eql_module_free(module); \
    eql_compiler_free(compiler); \
    bdestroy(module_name); \
    bdestroy(text);

// Asserts the contents of an EQL AST dump.
#define mu_assert_eql_node_dump(NODE, EXPECTED) \
    bstring actual = bfromcstr(""); \
    bstring expected = bfromcstr(EXPECTED); \
    int rc = eql_ast_node_dump(node, actual); \
    mu_assert(rc == 0, "Unable to dump"); \
    mu_assert(biseq(actual, expected), "Unexpected dump.\n\nExpected:\n%s\nActual:\n%s", bdata(expected), bdata(actual)); \
    bdestroy(actual); \
    bdestroy(expected);

