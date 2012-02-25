#include <stdio.h>
#include <timestamp.h>
#include <bstring.h>

#include "minunit.h"

//==============================================================================
//
// Test Cases
//
//==============================================================================

char *test_Timestamp_parse()
{
    long long timestamp = 0;
    int rc;

    // Parse ISO 8601 date.
    rc = Timestamp_parse(bfromcstr("2010-01-02T10:30:20Z"), &timestamp);
    mu_assert(timestamp == 1262428220000, "Invalid timestamp - 2010-01-02T10:30:20Z");

    // Parse ISO 8601 date past 2036.
    rc = Timestamp_parse(bfromcstr("2080-01-01T00:00:00Z"), &timestamp);
    mu_assert(timestamp == 3471292800000, "Invalid timestamp - 2080-01-01T00:00:00Z");

    // Parse ISO 8601 date on the epoch.
    rc = Timestamp_parse(bfromcstr("1970-01-01T00:00:00Z"), &timestamp);
    mu_assert(timestamp == 0, "Invalid timestamp - 1970-01-01T00:00:00Z");

    // Parse ISO 8601 date just before the epoch.
    rc = Timestamp_parse(bfromcstr("1969-12-31T23:59:59Z"), &timestamp);
    mu_assert(timestamp == -1000, "Invalid timestamp - 1969-12-31T23:59:59Z");

    // Parse ISO 8601 date a year before the epoch.
    rc = Timestamp_parse(bfromcstr("1969-01-01T00:00:00Z"), &timestamp);
    mu_assert(timestamp == -31536000000, "Invalid timestamp - 1969-01-01T00:00:00Z");

    // Parse ISO 8601 date a decade before the epoch.
    rc = Timestamp_parse(bfromcstr("1960-01-01T00:00:00Z"), &timestamp);
    mu_assert(timestamp == -315619200000, "Invalid timestamp - 1960-01-01T00:00:00Z");

    // Parse ISO 8601 date a long time before the epoch.
    rc = Timestamp_parse(bfromcstr("1910-01-01T00:00:00Z"), &timestamp);
    mu_assert(timestamp == -1893456000000, "Invalid timestamp - 1910-01-01T00:00:00Z");

    return NULL;
}

char *test_Timestamp_parse_invalid()
{
    long long timestamp = 0;
    int rc;

    // Parse invalid date.
    rc = Timestamp_parse(bfromcstr("foo"), &timestamp);
    mu_assert(rc == -1, "Timestamp should not have been parsed");
    
    return NULL;
}

//==============================================================================
//
// Setup
//
//==============================================================================

char *all_tests() {
    mu_run_test(test_Timestamp_parse);
    mu_run_test(test_Timestamp_parse_invalid);
    return NULL;
}

RUN_TESTS(all_tests)