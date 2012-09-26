#include <sys/stat.h>
#include <stdbool.h>
#include <bstring.h>
#include <file.h>

#import <importer.h>

//==============================================================================
//
// Minunit
//
//==============================================================================

//--------------------------------------
// Core
//--------------------------------------

#define mu_fail(MSG, ...) do {\
    fprintf(stderr, "%s:%d: " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__);\
    return 1;\
} while(0)

#define mu_assert_with_msg(TEST, MSG, ...) do {\
    if (!(TEST)) {\
        fprintf(stderr, "%s:%d: %s " MSG "\n", __FILE__, __LINE__, #TEST, ##__VA_ARGS__);\
        return 1;\
    }\
} while (0)

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


//--------------------------------------
// Typed asserts
//--------------------------------------

#define mu_assert_bool(ACTUAL) mu_assert_with_msg(ACTUAL, "Expected value to be true but was %d", ACTUAL)
#define mu_assert_int_equals(ACTUAL, EXPECTED) mu_assert_with_msg(ACTUAL == EXPECTED, "Expected: %d; Received: %d", EXPECTED, ACTUAL)
#define mu_assert_long_equals(ACTUAL, EXPECTED) mu_assert_with_msg(ACTUAL == EXPECTED, "Expected: %ld; Received: %ld", EXPECTED, ACTUAL)
#define mu_assert_int64_equals(ACTUAL, EXPECTED) mu_assert_with_msg(ACTUAL == EXPECTED, "Expected: %lld; Received: %lld", EXPECTED, ACTUAL)
#define mu_assert_bstring(ACTUAL, EXPECTED) mu_assert_with_msg(biseqcstr(ACTUAL, EXPECTED), "Expected: %s; Received: %s", EXPECTED, bdata(ACTUAL))



//==============================================================================
//
// Constants
//
//==============================================================================

// The temporary directory.
#define TMPDIR "tmp"
struct tagbstring BSTMPDIR = bsStatic(TMPDIR);

// The temporary file used for file operations in the test suite.
#define TEMPFILE "/tmp/skytemp"

// The path where memory dumps are directed to.
#define MEMDUMPFILE "/tmp/memdump"


//==============================================================================
//
// Helpers
//
//==============================================================================

// Empties the tmp directory.
#define cleantmp() do {\
    mu_assert_with_msg(sky_file_rm_r(&BSTMPDIR) == 0, "Unable to clean tmp directory"); \
    mu_assert_with_msg(mkdir(bdata(&BSTMPDIR), S_IRWXU | S_IRWXG | S_IRWXO) == 0, "Unable to create tmp directory"); \
} while(0)
    
// Loads the tmp directory with the contents of another directory.
#define loadtmp(PATH) do {\
    if(strcmp(PATH, "") == 0) { \
        cleantmp(); \
    } \
    else { \
        struct tagbstring _srcpath = bsStatic(PATH); \
        cleantmp(); \
        mu_assert_with_msg(sky_file_cp_r(&_srcpath, &BSTMPDIR) == 0, "Unable to copy to tmp directory"); \
    } \
} while(0)

// Uses an import file to create a table in the given directory.
#define importtmp(PATH) do {\
    cleantmp(); \
    sky_importer *importer = sky_importer_create(); \
    importer->path = bfromcstr(TMPDIR); \
    FILE *file = fopen(PATH, "r"); \
    mu_assert_with_msg(file != NULL, "Unable to open import file: " PATH); \
    int rc = sky_importer_import(importer, file); \
    mu_assert_int_equals(rc, 0); \
    fclose(file); \
} while(0)
    
// Asserts that a block has a specific block id and object id range.
#define mu_assert_block_info(INDEX, ID, MIN_OBJECT_ID, MAX_OBJECT_ID, MIN_TIMESTAMP, MAX_TIMESTAMP, SPANNED) \
    mu_assert(table->infos[INDEX]->id == ID, "Block " #INDEX " id expected to be " #ID); \
    mu_assert(table->infos[INDEX]->min_object_id == MIN_OBJECT_ID, "Block " #INDEX " min object id expected to be " #MIN_OBJECT_ID); \
    mu_assert(table->infos[INDEX]->max_object_id == MAX_OBJECT_ID, "Block " #INDEX " max object id expected to be " #MAX_OBJECT_ID); \
    mu_assert(table->infos[INDEX]->min_timestamp == MIN_TIMESTAMP, "Block " #INDEX " min timestamp expected to be " #MIN_TIMESTAMP); \
    mu_assert(table->infos[INDEX]->max_timestamp == MAX_TIMESTAMP, "Block " #INDEX " max timestamp expected to be " #MAX_TIMESTAMP); \
    mu_assert(table->infos[INDEX]->spanned == SPANNED, "Block " #INDEX " spanned expected to be " #SPANNED);

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
            mu_fail("Expected 0x%02x (%s), received 0x%02x (%s) at location %ld (0x%x)", ch2, FILENAME2, ch1, FILENAME1, (ftell(file2)-1), (int)(ftell(file2)-1)); \
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

// Asserts the contents of memory. If an error occurs then the memory is
// dumped to file.
#define mu_assert_mem(ACTUAL, EXPECTED, N) do {\
    if(memcmp(ACTUAL, EXPECTED, N) != 0) { \
        FILE *file = fopen(MEMDUMPFILE, "w"); \
        fwrite(ACTUAL, N, sizeof(char), file); \
        fclose(file); \
        mu_fail("Memory contents do not match. Memory dumped to: " MEMDUMPFILE); \
    } \
} while(0)