#include <stdio.h>
#include <timestamp.h>
#include <bstring.h>

#include "minunit.h"

//==============================================================================
//
// Helpers
//
//==============================================================================

#define mu_timestamp_assert(STR, VALUE) \
timestamp = 0; \
str = bfromcstr(STR); \
rc = sky_timestamp_parse(str, &timestamp); \
mu_assert(rc == 0, "Invalid return code for timestamp: " #STR); \
mu_assert(timestamp == VALUE, "Expected " #STR " to convert to " #VALUE); \
bdestroy(str);


//==============================================================================
//
// Test Cases
//
//==============================================================================

int test_sky_timestamp_parse()
{
    sky_timestamp_t timestamp = 0;
    int rc;
    bstring str;

    // Parse ISO 8601 date.
    mu_timestamp_assert("2010-01-02T10:30:20Z", 1262428220000000);

    // Parse ISO 8601 date past 2036.
    mu_timestamp_assert("2080-01-01T00:00:00Z", 3471292800000000);

    // Parse ISO 8601 date on the epoch.
    mu_timestamp_assert("1970-01-01T00:00:00Z", 0);

    // Parse ISO 8601 date just before the epoch.
    mu_timestamp_assert("1969-12-31T23:59:59Z", -1000000);

    // Parse ISO 8601 date a year before the epoch.
    mu_timestamp_assert("1969-01-01T00:00:00Z", -31536000000000);

    // Parse ISO 8601 date a decade before the epoch.
    mu_timestamp_assert("1960-01-01T00:00:00Z", -315619200000000);

    // Parse ISO 8601 date a long time before the epoch.
    mu_timestamp_assert("1910-01-01T00:00:00Z", -1893456000000000);

    return 0;
}

int test_sky_timestamp_parse_invalid()
{
    sky_timestamp_t timestamp = 0;
    int rc;

    // Parse invalid date.
    rc = sky_timestamp_parse(bfromcstr("foo"), &timestamp);
    mu_assert(rc == -1, "Timestamp should not have been parsed");
    
    return 0;
}

//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_timestamp_parse);
    mu_run_test(test_sky_timestamp_parse_invalid);
    return 0;
}

RUN_TESTS()