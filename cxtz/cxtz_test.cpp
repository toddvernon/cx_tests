//-----------------------------------------------------------------------------------------
// cxtz_test.cpp - CxDateTime and timezone unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <cx/base/string.h>
#include <cx/tz/date_time.h>

//-----------------------------------------------------------------------------------------
// Test harness
//-----------------------------------------------------------------------------------------
static int testsPassed = 0;
static int testsFailed = 0;

void check(int condition, const char* testName) {
    if (condition) {
        testsPassed++;
        printf("  PASS: %s\n", testName);
    } else {
        testsFailed++;
        printf("  FAIL: %s\n", testName);
    }
}

//-----------------------------------------------------------------------------------------
// Helper: check if string contains substring
//-----------------------------------------------------------------------------------------
int contains(const CxString& haystack, const char* needle) {
    return strstr(haystack.data(), needle) != NULL;
}

//-----------------------------------------------------------------------------------------
// Constructor tests
//-----------------------------------------------------------------------------------------
void testConstructor() {
    printf("\n== CxDateTime Constructor Tests ==\n");

    // Default constructor (current time in local timezone)
    {
        CxDateTime dt;
        int epoch = dt.epochSeconds();
        check(epoch > 0, "default constructor: epochSeconds is positive");

        CxString tzName = dt.timeZoneName();
        check(tzName.length() > 0, "default constructor: timezone name is set");
    }

    // Constructor from epoch seconds
    {
        time_t knownTime = 1609459200; // 2021-01-01 00:00:00 UTC
        CxDateTime dt(knownTime);
        int epoch = dt.epochSeconds();
        check(epoch == 1609459200, "epoch constructor: preserves epoch seconds");
    }

    // Constructor from time parts (local zone)
    {
        CxDateTime dt(2020, 6, 15, 12, 30, 45);
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second);
        check(year == 2020, "parts constructor: year correct");
        check(month == 6, "parts constructor: month correct");
        check(day == 15, "parts constructor: day correct");
        check(hour == 12, "parts constructor: hour correct");
        check(minute == 30, "parts constructor: minute correct");
        check(second == 45, "parts constructor: second correct");
    }

    // Constructor from time string
    {
        CxDateTime dt("2019-07-20 15:30:00");
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second);
        check(year == 2019, "string constructor: year correct");
        check(month == 7, "string constructor: month correct");
        check(day == 20, "string constructor: day correct");
        check(hour == 15, "string constructor: hour correct");
    }
}

//-----------------------------------------------------------------------------------------
// Copy and assignment tests
//-----------------------------------------------------------------------------------------
void testCopyAndAssignment() {
    printf("\n== CxDateTime Copy/Assignment Tests ==\n");

    // Copy constructor
    {
        CxDateTime dt1(2021, 3, 15, 10, 20, 30);
        CxDateTime dt2(dt1);

        check(dt1.epochSeconds() == dt2.epochSeconds(), "copy constructor: epoch matches");
        check(dt1 == dt2, "copy constructor: equality operator works");
    }

    // Assignment operator
    {
        CxDateTime dt1(2021, 4, 20, 8, 0, 0);
        CxDateTime dt2;
        dt2 = dt1;

        check(dt1.epochSeconds() == dt2.epochSeconds(), "assignment: epoch matches");
        check(dt1 == dt2, "assignment: equality operator works");
    }

    // Self-assignment
    {
        CxDateTime dt1(2021, 5, 1, 0, 0, 0);
        int originalEpoch = dt1.epochSeconds();
        dt1 = dt1;
        check(dt1.epochSeconds() == originalEpoch, "self-assignment: preserved");
    }
}

//-----------------------------------------------------------------------------------------
// Timezone tests
//-----------------------------------------------------------------------------------------
void testTimezone() {
    printf("\n== CxDateTime Timezone Tests ==\n");

    // UTC timezone
    {
        cctz::time_zone utc = cctz::utc_time_zone();
        CxDateTime dt(2021, 1, 1, 12, 0, 0, utc);
        CxString name = dt.timeZoneName();
        check(contains(name, "UTC"), "UTC timezone: name contains UTC");
    }

    // Named timezone loading
    {
        cctz::time_zone tz = CxDateTime::zoneNameToZone("America/New_York");
        CxDateTime dt(2021, 6, 1, 12, 0, 0, tz);
        // In June, NYC is on EDT (-4)
        CxString formatted = dt.format();
        check(contains(formatted, "2021-06-01"), "named timezone: date preserved");
    }

    // Local timezone
    {
        cctz::time_zone local = cctz::local_time_zone();
        CxDateTime dt(local);
        CxString name = dt.timeZoneName();
        check(name.length() > 0, "local timezone: has a name");
    }

    // Get timezone
    {
        cctz::time_zone utc = cctz::utc_time_zone();
        CxDateTime dt(2021, 1, 1, 0, 0, 0, utc);
        cctz::time_zone retrieved = dt.timeZone();
        check(retrieved == utc, "timeZone(): returns correct zone");
    }
}

//-----------------------------------------------------------------------------------------
// Breakdown tests
//-----------------------------------------------------------------------------------------
void testBreakdown() {
    printf("\n== CxDateTime Breakdown Tests ==\n");

    // Basic breakdown
    {
        cctz::time_zone utc = cctz::utc_time_zone();
        CxDateTime dt(2022, 8, 15, 14, 30, 45, utc);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);

        check(year == 2022, "breakdown: year");
        check(month == 8, "breakdown: month");
        check(day == 15, "breakdown: day");
        check(hour == 14, "breakdown: hour");
        check(minute == 30, "breakdown: minute");
        check(second == 45, "breakdown: second");
    }

    // Breakdown with different timezone
    {
        cctz::time_zone utc = cctz::utc_time_zone();
        cctz::time_zone nyc = CxDateTime::zoneNameToZone("America/New_York");

        // Create time at UTC noon
        CxDateTime dt(2022, 1, 15, 12, 0, 0, utc);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, nyc);

        // NYC is UTC-5 in January
        check(hour == 7, "breakdown with timezone: hour offset correctly");
    }
}

//-----------------------------------------------------------------------------------------
// Offset tests
//-----------------------------------------------------------------------------------------
void testOffset() {
    printf("\n== CxDateTime Offset Tests ==\n");

    cctz::time_zone utc = cctz::utc_time_zone();

    // Offset years
    {
        CxDateTime dt(2020, 6, 15, 12, 0, 0, utc);
        dt.offsetYears(2, utc);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        check(year == 2022, "offsetYears: adds years correctly");
    }

    // Offset months
    {
        CxDateTime dt(2020, 6, 15, 12, 0, 0, utc);
        dt.offsetMonths(3, utc);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        check(month == 9, "offsetMonths: adds months correctly");
    }

    // Offset days
    {
        CxDateTime dt(2020, 6, 15, 12, 0, 0, utc);
        dt.offsetDays(10, utc);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        check(day == 25, "offsetDays: adds days correctly");
    }

    // Offset hours
    {
        CxDateTime dt(2020, 6, 15, 12, 0, 0, utc);
        dt.offsetHours(5, utc);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        check(hour == 17, "offsetHours: adds hours correctly");
    }

    // Offset minutes
    {
        CxDateTime dt(2020, 6, 15, 12, 0, 0, utc);
        dt.offsetMinutes(45, utc);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        check(minute == 45, "offsetMinutes: adds minutes correctly");
    }

    // Offset seconds
    {
        CxDateTime dt(2020, 6, 15, 12, 0, 0, utc);
        dt.offsetSeconds(30, utc);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        check(second == 30, "offsetSeconds: adds seconds correctly");
    }

    // Negative offset
    {
        CxDateTime dt(2020, 6, 15, 12, 0, 0, utc);
        dt.offsetDays(-5, utc);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        check(day == 10, "offsetDays negative: subtracts correctly");
    }

    // Month rollover
    {
        CxDateTime dt(2020, 1, 31, 12, 0, 0, utc);
        dt.offsetDays(1, utc);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        check(month == 2 && day == 1, "offsetDays: handles month rollover");
    }
}

//-----------------------------------------------------------------------------------------
// Format tests
//-----------------------------------------------------------------------------------------
void testFormat() {
    printf("\n== CxDateTime Format Tests ==\n");

    cctz::time_zone utc = cctz::utc_time_zone();

    // Default format
    {
        CxDateTime dt(2021, 3, 14, 9, 26, 53, utc);
        CxString formatted = dt.format(utc);
        check(contains(formatted, "2021-03-14"), "format default: contains date");
        check(contains(formatted, "09:26:53"), "format default: contains time");
    }

    // Custom format - date only
    {
        CxDateTime dt(2021, 12, 25, 10, 30, 0, utc);
        CxString formatted = dt.format(utc, "%Y-%m-%d");
        check(formatted == "2021-12-25", "format custom: date only");
    }

    // Custom format - time only
    {
        CxDateTime dt(2021, 12, 25, 14, 45, 30, utc);
        CxString formatted = dt.format(utc, "%H:%M:%S");
        check(formatted == "14:45:30", "format custom: time only");
    }

    // Custom format - with weekday
    {
        CxDateTime dt(2021, 12, 25, 0, 0, 0, utc);
        CxString formatted = dt.format(utc, "%A");
        check(contains(formatted, "Saturday"), "format custom: weekday");
    }
}

//-----------------------------------------------------------------------------------------
// Static method tests
//-----------------------------------------------------------------------------------------
void testStaticMethods() {
    printf("\n== CxDateTime Static Method Tests ==\n");

    // now_epochSeconds
    {
        int epoch = CxDateTime::now_epochSeconds();
        check(epoch > 1600000000, "now_epochSeconds: returns reasonable value");
    }

    // now_tp (time_point)
    {
        std::chrono::system_clock::time_point tp = CxDateTime::now_tp();
        auto duration = tp.time_since_epoch();
        check(duration.count() > 0, "now_tp: returns valid time_point");
    }

    // firstSecondOfTheDay
    {
        cctz::time_zone utc = cctz::utc_time_zone();
        CxDateTime dt(2021, 5, 15, 14, 30, 45, utc);
        CxDateTime first = CxDateTime::firstSecondOfTheDay(dt, utc);

        long year, month, day, hour, minute, second;
        first.breakdown(&year, &month, &day, &hour, &minute, &second, utc);

        check(year == 2021 && month == 5 && day == 15, "firstSecondOfTheDay: date preserved");
        check(hour == 0 && minute == 0 && second == 0, "firstSecondOfTheDay: time is 00:00:00");
    }

    // lastSecondOfTheDay
    {
        cctz::time_zone utc = cctz::utc_time_zone();
        CxDateTime dt(2021, 5, 15, 14, 30, 45, utc);
        CxDateTime last = CxDateTime::lastSecondOfTheDay(dt, utc);

        long year, month, day, hour, minute, second;
        last.breakdown(&year, &month, &day, &hour, &minute, &second, utc);

        check(year == 2021 && month == 5 && day == 15, "lastSecondOfTheDay: date preserved");
        check(hour == 23 && minute == 59 && second == 59, "lastSecondOfTheDay: time is 23:59:59");
    }

    // formatTimeLength
    {
        cctz::time_zone utc = cctz::utc_time_zone();
        CxDateTime start(2021, 5, 15, 10, 0, 0, utc);
        CxDateTime end(2021, 5, 15, 12, 30, 45, utc);

        CxString duration = CxDateTime::formatTimeLength(start, end);
        check(contains(duration, "2 hours"), "formatTimeLength: hours correct");
        check(contains(duration, "30 minutes"), "formatTimeLength: minutes correct");
        check(contains(duration, "45 seconds"), "formatTimeLength: seconds correct");
    }

    // formatTimeLength - seconds only
    {
        cctz::time_zone utc = cctz::utc_time_zone();
        CxDateTime start(2021, 5, 15, 10, 0, 0, utc);
        CxDateTime end(2021, 5, 15, 10, 0, 30, utc);

        CxString duration = CxDateTime::formatTimeLength(start, end);
        check(contains(duration, "30 seconds"), "formatTimeLength: seconds only");
    }

    // formatTimeLength - zero duration
    {
        cctz::time_zone utc = cctz::utc_time_zone();
        CxDateTime dt(2021, 5, 15, 10, 0, 0, utc);

        CxString duration = CxDateTime::formatTimeLength(dt, dt);
        check(contains(duration, "0 seconds"), "formatTimeLength: zero duration");
    }
}

//-----------------------------------------------------------------------------------------
// Edge case tests
//-----------------------------------------------------------------------------------------
void testEdgeCases() {
    printf("\n== CxDateTime Edge Case Tests ==\n");

    cctz::time_zone utc = cctz::utc_time_zone();

    // Leap year
    {
        CxDateTime dt(2020, 2, 29, 12, 0, 0, utc);
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        check(month == 2 && day == 29, "leap year: Feb 29 valid in 2020");
    }

    // Non-leap year normalization
    {
        CxDateTime dt(2021, 2, 29, 12, 0, 0, utc); // Feb 29 doesn't exist in 2021
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        // Should normalize to March 1
        check(month == 3 && day == 1, "non-leap year: Feb 29 normalizes to Mar 1");
    }

    // Year boundary
    {
        CxDateTime dt(2020, 12, 31, 23, 59, 59, utc);
        dt.offsetSeconds(1, utc);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        check(year == 2021 && month == 1 && day == 1, "year boundary: rollover works");
    }

    // Very old date
    {
        CxDateTime dt(1970, 1, 1, 0, 0, 0, utc);
        int epoch = dt.epochSeconds();
        check(epoch == 0, "epoch zero: 1970-01-01 00:00:00 UTC");
    }

    // Midnight
    {
        CxDateTime dt(2021, 6, 15, 0, 0, 0, utc);
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utc);
        check(hour == 0 && minute == 0 && second == 0, "midnight: 00:00:00 preserved");
    }
}

//-----------------------------------------------------------------------------------------
// zoneNameToZone tests
//-----------------------------------------------------------------------------------------
void testZoneNameToZone() {
    printf("\n== CxDateTime::zoneNameToZone Tests ==\n");

    // Valid timezone
    {
        cctz::time_zone tz = CxDateTime::zoneNameToZone("America/Los_Angeles");
        CxDateTime dt(2021, 7, 4, 12, 0, 0, tz);
        CxString formatted = dt.format(tz);
        check(formatted.length() > 0, "valid timezone: LA loads correctly");
    }

    // Another valid timezone
    {
        cctz::time_zone tz = CxDateTime::zoneNameToZone("Europe/London");
        CxDateTime dt(2021, 7, 4, 12, 0, 0, tz);
        CxString formatted = dt.format(tz);
        check(formatted.length() > 0, "valid timezone: London loads correctly");
    }

    // Empty string returns local
    {
        cctz::time_zone tz = CxDateTime::zoneNameToZone("");
        CxDateTime dt(2021, 7, 4, 12, 0, 0, tz);
        check(dt.epochSeconds() > 0, "empty string: returns local timezone");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxDateTime Test Suite\n");
    printf("=====================\n");

    testConstructor();
    testCopyAndAssignment();
    testTimezone();
    testBreakdown();
    testOffset();
    testFormat();
    testStaticMethods();
    testEdgeCases();
    testZoneNameToZone();

    printf("\n=====================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
