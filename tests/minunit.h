//==============================================================================
//
// Minunit
//
//==============================================================================

/* file: minunit.h */
#define mu_assert(test, message) do {\
if (!(test)) \
    return __FILE__ ": Expected: " #test;\
} while (0)

#define mu_run_test(test) do { char *message = test(); tests_run++; \
                               if (message) return message; } while (0)

#define RUN_TESTS(name) int main() {\
   char *result = all_tests();\
   if (result != 0) {\
       printf("%s\n", result);\
   }\
   else {\
       printf("ALL TESTS PASSED\n");\
   }\
   printf("Tests run: %d\n", tests_run);\
   return result != 0;\
}

int tests_run;



//==============================================================================
//
// Constants
//
//==============================================================================

// The temporary file used for file operations in the test suite.
#define TEMPFILE "/tmp/skytemp"


//==============================================================================
//
// Helpers
//
//==============================================================================

// Copy a database from the fixtures directory into the tmp/db directory.
#define copydb(DB) \
    char _copydb_cmd[1024]; \
    snprintf(_copydb_cmd, 1024, "tests/copydb.sh %s", DB); \
    system(_copydb_cmd)
    
// Asserts that a block has a specific block id and object id range.
#define mu_assert_block_info(INDEX, ID, MIN_OBJECT_ID, MAX_OBJECT_ID, MIN_TIMESTAMP, MAX_TIMESTAMP, SPANNED) \
    mu_assert(object_file->infos[INDEX].id == ID, "Block " #INDEX " id expected to be " #ID); \
    mu_assert(object_file->infos[INDEX].min_object_id == MIN_OBJECT_ID, "Block " #INDEX " min object id expected to be " #MIN_OBJECT_ID); \
    mu_assert(object_file->infos[INDEX].max_object_id == MAX_OBJECT_ID, "Block " #INDEX " max object id expected to be " #MAX_OBJECT_ID); \
    mu_assert(object_file->infos[INDEX].min_timestamp == MIN_TIMESTAMP, "Block " #INDEX " min timestamp expected to be " #MIN_TIMESTAMP); \
    mu_assert(object_file->infos[INDEX].max_timestamp == MAX_TIMESTAMP, "Block " #INDEX " max timestamp expected to be " #MAX_TIMESTAMP); \
    mu_assert(object_file->infos[INDEX].spanned == SPANNED, "Block " #INDEX " spanned expected to be " #SPANNED);

// Asserts action data.
#define mu_assert_action(INDEX, ID, NAME) \
    mu_assert(object_file->actions[INDEX].id == ID, "Expected action #" #INDEX " id to be: " #ID); \
    mu_assert(biseqcstr(object_file->actions[INDEX].name, NAME) == 1, "Expected action #" #INDEX " name to be: " #NAME);

// Asserts property data.
#define mu_assert_property(INDEX, ID, NAME) \
    mu_assert(object_file->properties[INDEX].id == ID, "Expected property #" #INDEX " id to be: " #ID); \
    mu_assert(biseqcstr(object_file->properties[INDEX].name, NAME) == 1, "Expected property #" #INDEX " name to be: " #NAME);

// Asserts the contents of the temp file.
#define mu_assert_tempfile(EXP_FILENAME, MESSAGE) \
    unsigned char tempch; \
    unsigned char expch; \
    FILE *tempfile = fopen(TEMPFILE, "r"); \
    FILE *expfile = fopen(EXP_FILENAME, "r"); \
    if(tempfile == NULL) return MESSAGE ": Cannot open tempfile"; \
    if(expfile == NULL) return MESSAGE ": Cannot open expectation file"; \
    while(1) { \
        fread(&expch, 1, 1, expfile); \
        fread(&tempch, 1, 1, tempfile); \
        if(feof(expfile) || feof(tempfile)) break; \
        if(tempch != expch) { \
            fprintf(stderr, "%s:%d: Expected 0x%02x, received 0x%02x at location %ld in tempfile.\n", __FILE__, __LINE__, expch, tempch, (ftell(expfile)-1)); \
            return MESSAGE ": Tempfile does not match expected value"; \
        } \
    } \
    if(!feof(tempfile)) return MESSAGE ": Tempfile length longer than expected"; \
    if(!feof(expfile)) return MESSAGE ": Tempfile length shorter than expected"; \
    fclose(tempfile); \
    fclose(expfile);
