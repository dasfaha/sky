/*
 * Copyright (c) 2012 Ben Johnson, http://skylandlabs.com
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <sys/time.h>

#include "dbg.h"
#include "bstring.h"

//==============================================================================
//
// Timestamp Parsing
//
//==============================================================================

/*
 * Parses a timestamp from a C string. The return value is the number of
 * milliseconds before or after the epoch (Jan 1, 1970).
 *
 * NOTE: Parsing seems to only work back to around the first decade of the
 *       1900's. Need to investigate further why this is.
 *
 * str - The string containing an ISO 8601 formatted date.
 */
int Timestamp_parse(bstring str, long long *ret)
{
    // Validate string.
    if(str == NULL) {
        return -1;
    }
    
    // Parse date.
    struct tm tp;
    if(strptime(bdata(str), "%Y-%m-%dT%H:%M:%SZ", &tp) == NULL) {
        return -1;
    }
    
    // Set timezone information.
    tzset();

    // Convert to milliseconds since epoch in UTC.
    char buffer[100];
    strftime(buffer, 100, "%s", &tp);
    long long value = atoll(buffer);
    value -= timezone;
    value += (daylight ? 3600 : 0);
    *ret = value * 1000;
    
    return 0;
}

/*
 * Returns the number of milliseconds since the epoch.
 */
int Timestamp_now(long long *ret)
{
    time_t t = time(NULL);
    if(t != -1) {
        *ret = t * 1000;
        return 0;
    }
    else {
        return -1;
    }
}