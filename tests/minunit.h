/* file: minunit.h */
#define mu_assert(test, message) do {\
if (!(test)) \
    return message;\
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
