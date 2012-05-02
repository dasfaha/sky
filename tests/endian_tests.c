#include <stdio.h>
#include <stdlib.h>

#include <endian.h>

#include "minunit.h"

//==============================================================================
//
// Test Cases
//
//==============================================================================

int test_bswap16() {
    uint16_t value = 0x0201;
    mu_assert(bswap16(value) == 0x0102, "");
    return 0;
}

int test_bswap32() {
    uint32_t value = 0x04030201;
    mu_assert(bswap32(value) == 0x01020304, "");
    return 0;
}

int test_bswap64() {
    uint64_t value = 0x0807060504030201;
    mu_assert(bswap64(value) == 0x0102030405060708, "");
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_bswap16);
    mu_run_test(test_bswap32);
    mu_run_test(test_bswap64);
    
    return 0;
}

RUN_TESTS()