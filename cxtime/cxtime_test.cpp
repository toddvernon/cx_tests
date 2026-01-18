//-----------------------------------------------------------------------------------------
// cxtime_test.cpp - CxTime unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <cx/base/string.h>
#include <cx/base/time.h>

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
// CxTime constructor tests
//-----------------------------------------------------------------------------------------
void testTimeConstructors() {
    printf("\n== CxTime Constructor Tests ==\n");

    // Default constructor - current time
    {
        CxTime before = CxTime::now();
        CxTime t;
        CxTime after = CxTime::now();

        check(t.epochSeconds() >= before.epochSeconds(), "default ctor >= before");
        check(t.epochSeconds() <= after.epochSeconds(), "default ctor <= after");
    }

    // Constructor with epoch seconds
    {
        time_t epoch = 1000000000;  // Sep 9, 2001
        CxTime t(epoch);
        check(t.epochSeconds() == epoch, "epoch ctor stores correct value");
    }

    // Constructor with components
    {
        CxTime t(2020, 6, 15, 12, 30, 45);
        check(t.year() == 2020, "component ctor year");
        check(t.month() == 6, "component ctor month");
        check(t.day() == 15, "component ctor day");
        check(t.hour() == 12, "component ctor hour");
        check(t.minute() == 30, "component ctor minute");
        check(t.second() == 45, "component ctor second");
    }

    // Copy constructor
    {
        CxTime t1(2021, 3, 20, 8, 15, 30);
        CxTime t2(t1);
        check(t2.epochSeconds() == t1.epochSeconds(), "copy ctor copies value");
        check(t2.year() == 2021, "copy ctor year matches");
    }

    // Epoch 0 (Jan 1, 1970 00:00:00 UTC)
    {
        CxTime t((time_t)0);
        check(t.epochSeconds() == 0, "epoch 0 stored correctly");
    }
}

//-----------------------------------------------------------------------------------------
// CxTime component accessor tests
//-----------------------------------------------------------------------------------------
void testTimeAccessors() {
    printf("\n== CxTime Accessor Tests ==\n");

    // Known date/time - verify round-trip through constructor
    {
        CxTime t(2023, 7, 15, 14, 30, 45);  // Use mid-year to avoid DST edge cases
        check(t.year() == 2023, "year accessor");
        check(t.month() == 7, "month accessor");
        check(t.day() == 15, "day accessor");
        check(t.hour() == 14, "hour accessor");
        check(t.minute() == 30, "minute accessor");
        check(t.second() == 45, "second accessor");
    }

    // Edge case: midnight
    {
        CxTime t(2020, 1, 1, 0, 0, 0);
        check(t.hour() == 0, "midnight hour");
        check(t.minute() == 0, "midnight minute");
        check(t.second() == 0, "midnight second");
    }

    // Edge case: end of day
    {
        CxTime t(2020, 1, 1, 23, 59, 59);
        check(t.hour() == 23, "end of day hour");
        check(t.minute() == 59, "end of day minute");
        check(t.second() == 59, "end of day second");
    }

    // Month boundaries
    {
        CxTime jan(2020, 1, 15, 12, 0, 0);
        CxTime dec(2020, 12, 15, 12, 0, 0);
        check(jan.month() == 1, "January is month 1");
        check(dec.month() == 12, "December is month 12");
    }
}

//-----------------------------------------------------------------------------------------
// CxTime offset tests
//-----------------------------------------------------------------------------------------
void testTimeOffsets() {
    printf("\n== CxTime Offset Tests ==\n");

    // offsetSeconds
    {
        CxTime t(2020, 1, 1, 12, 0, 0);
        time_t original = t.epochSeconds();
        t.offsetSeconds(30);
        check(t.epochSeconds() == original + 30, "offsetSeconds adds 30");
        check(t.second() == 30, "offsetSeconds result");
    }

    // offsetSeconds negative
    {
        CxTime t(2020, 1, 1, 12, 0, 30);
        t.offsetSeconds(-15);
        check(t.second() == 15, "offsetSeconds negative");
    }

    // offsetMinutes
    {
        CxTime t(2020, 1, 1, 12, 0, 0);
        time_t original = t.epochSeconds();
        t.offsetMinutes(5);
        check(t.epochSeconds() == original + 300, "offsetMinutes adds 5 min");
        check(t.minute() == 5, "offsetMinutes result");
    }

    // offsetHours
    {
        CxTime t(2020, 1, 1, 12, 0, 0);
        time_t original = t.epochSeconds();
        t.offsetHours(3);
        check(t.epochSeconds() == original + 10800, "offsetHours adds 3 hr");
        check(t.hour() == 15, "offsetHours result");
    }

    // offsetDays
    {
        CxTime t(2020, 1, 1, 12, 0, 0);
        time_t original = t.epochSeconds();
        t.offsetDays(1);
        check(t.epochSeconds() == original + 86400, "offsetDays adds 1 day");
        check(t.day() == 2, "offsetDays result");
    }

    // offsetDays crosses month boundary
    {
        CxTime t(2020, 1, 31, 12, 0, 0);
        t.offsetDays(1);
        check(t.month() == 2, "offsetDays crosses to February");
        check(t.day() == 1, "offsetDays day is 1");
    }

    // offsetHours negative crosses day
    {
        CxTime t(2020, 1, 2, 1, 0, 0);  // 1 AM on Jan 2
        t.offsetHours(-3);
        check(t.day() == 1, "offsetHours negative crosses day");
        check(t.hour() == 22, "offsetHours negative hour");
    }
}

//-----------------------------------------------------------------------------------------
// CxTime comparison and assignment tests
//-----------------------------------------------------------------------------------------
void testTimeCompareAssign() {
    printf("\n== CxTime Compare/Assign Tests ==\n");

    // operator==
    {
        CxTime t1(2020, 6, 15, 12, 0, 0);
        CxTime t2(2020, 6, 15, 12, 0, 0);
        check(t1 == t2, "equal times compare equal");
    }

    // operator== with different times
    {
        CxTime t1(2020, 6, 15, 12, 0, 0);
        CxTime t2(2020, 6, 15, 12, 0, 1);
        check(!(t1 == t2), "different times not equal");
    }

    // Assignment operator
    {
        CxTime t1(2020, 6, 15, 12, 30, 45);
        CxTime t2;
        t2 = t1;
        check(t2 == t1, "assignment copies value");
        check(t2.year() == 2020, "assignment year");
    }

    // Self-assignment
    {
        CxTime t(2020, 6, 15, 12, 0, 0);
        time_t original = t.epochSeconds();
        t = t;
        check(t.epochSeconds() == original, "self-assignment preserves value");
    }
}

//-----------------------------------------------------------------------------------------
// CxTime::now tests
//-----------------------------------------------------------------------------------------
void testTimeNow() {
    printf("\n== CxTime::now Tests ==\n");

    // now() returns current time
    {
        time_t before = ::time(NULL);
        CxTime t = CxTime::now();
        time_t after = ::time(NULL);

        check(t.epochSeconds() >= before, "now() >= system time before");
        check(t.epochSeconds() <= after, "now() <= system time after");
    }

    // Consecutive calls to now()
    {
        CxTime t1 = CxTime::now();
        CxTime t2 = CxTime::now();
        check(t2.epochSeconds() >= t1.epochSeconds(), "consecutive now() non-decreasing");
    }
}

//-----------------------------------------------------------------------------------------
// CxTime string output tests
//-----------------------------------------------------------------------------------------
void testTimeStrings() {
    printf("\n== CxTime String Tests ==\n");

    // asString returns non-empty
    {
        CxTime t(2020, 6, 15, 12, 30, 45);
        CxString s = t.asString();
        check(s.length() > 0, "asString returns non-empty");
    }

    // asString contains year
    {
        CxTime t(2020, 6, 15, 12, 30, 45);
        CxString s = t.asString();
        check(s.index("2020") != -1, "asString contains year");
    }

    // asFormattedString with %Y-%m-%d
    {
        CxTime t(2020, 6, 15, 12, 30, 45);
        CxString s = t.asFormattedString("%Y-%m-%d");
        check(strcmp(s.data(), "2020-06-15") == 0, "formatted string YYYY-MM-DD");
    }

    // asFormattedString with %H:%M:%S
    {
        CxTime t(2020, 6, 15, 12, 30, 45);
        CxString s = t.asFormattedString("%H:%M:%S");
        check(strcmp(s.data(), "12:30:45") == 0, "formatted string HH:MM:SS");
    }

    // asFormattedString with full datetime
    {
        CxTime t(2020, 6, 15, 12, 30, 45);
        CxString s = t.asFormattedString("%Y-%m-%d %H:%M:%S");
        check(strcmp(s.data(), "2020-06-15 12:30:45") == 0, "formatted full datetime");
    }

    // asFormattedString with weekday
    {
        CxTime t(2020, 6, 15, 12, 0, 0);  // Monday
        CxString s = t.asFormattedString("%A");
        check(s.length() > 0, "formatted weekday non-empty");
    }
}

//-----------------------------------------------------------------------------------------
// CxTime::monthNumber tests
//-----------------------------------------------------------------------------------------
void testMonthNumber() {
    printf("\n== CxTime::monthNumber Tests ==\n");

    // Standard month names
    check(CxTime::monthNumber("jan") == 1, "jan = 1");
    check(CxTime::monthNumber("feb") == 2, "feb = 2");
    check(CxTime::monthNumber("mar") == 3, "mar = 3");
    check(CxTime::monthNumber("apr") == 4, "apr = 4");
    check(CxTime::monthNumber("may") == 5, "may = 5");
    check(CxTime::monthNumber("jun") == 6, "jun = 6");
    check(CxTime::monthNumber("jul") == 7, "jul = 7");
    check(CxTime::monthNumber("aug") == 8, "aug = 8");
    check(CxTime::monthNumber("sep") == 9, "sep = 9");
    check(CxTime::monthNumber("oct") == 10, "oct = 10");
    check(CxTime::monthNumber("nov") == 11, "nov = 11");
    check(CxTime::monthNumber("dec") == 12, "dec = 12");

    // Case insensitive
    check(CxTime::monthNumber("JAN") == 1, "JAN uppercase = 1");
    check(CxTime::monthNumber("Jan") == 1, "Jan mixed case = 1");
    check(CxTime::monthNumber("DEC") == 12, "DEC uppercase = 12");

    // Invalid month
    check(CxTime::monthNumber("xxx") == -1, "invalid month = -1");
    check(CxTime::monthNumber("") == -1, "empty string = -1");
}

//-----------------------------------------------------------------------------------------
// CxTime::firstSecondOfTheDay / lastSecondOfTheDay tests
//-----------------------------------------------------------------------------------------
void testDayBoundaries() {
    printf("\n== CxTime Day Boundary Tests ==\n");

    // firstSecondOfTheDay
    {
        CxTime t(2020, 6, 15, 14, 30, 45);
        CxTime first = CxTime::firstSecondOfTheDay(t);
        check(first.year() == 2020, "first second year");
        check(first.month() == 6, "first second month");
        check(first.day() == 15, "first second day");
        check(first.hour() == 0, "first second hour");
        check(first.minute() == 0, "first second minute");
        check(first.second() == 0, "first second second");
    }

    // lastSecondOfTheDay
    {
        CxTime t(2020, 6, 15, 14, 30, 45);
        CxTime last = CxTime::lastSecondOfTheDay(t);
        check(last.year() == 2020, "last second year");
        check(last.month() == 6, "last second month");
        check(last.day() == 15, "last second day");
        check(last.hour() == 23, "last second hour");
        check(last.minute() == 59, "last second minute");
        check(last.second() == 59, "last second second");
    }

    // First and last are on same day
    {
        CxTime t(2020, 6, 15, 12, 0, 0);
        CxTime first = CxTime::firstSecondOfTheDay(t);
        CxTime last = CxTime::lastSecondOfTheDay(t);
        check(first.day() == last.day(), "first and last same day");
        check(last.epochSeconds() > first.epochSeconds(), "last > first");
        check(last.epochSeconds() - first.epochSeconds() == 86399, "difference is 86399 sec");
    }
}

//-----------------------------------------------------------------------------------------
// CxTime::formatTimeLength tests
//-----------------------------------------------------------------------------------------
void testFormatTimeLength() {
    printf("\n== CxTime::formatTimeLength Tests ==\n");

    // Zero duration
    {
        CxTime t(2020, 1, 1, 12, 0, 0);
        CxString s = CxTime::formatTimeLength(t, t);
        check(s.index("0 seconds") != -1, "zero duration");
    }

    // Seconds only
    {
        CxTime start(2020, 1, 1, 12, 0, 0);
        CxTime end(2020, 1, 1, 12, 0, 30);
        CxString s = CxTime::formatTimeLength(start, end);
        check(s.index("30 seconds") != -1, "30 seconds");
    }

    // Minutes and seconds
    {
        CxTime start(2020, 1, 1, 12, 0, 0);
        CxTime end(2020, 1, 1, 12, 5, 30);
        CxString s = CxTime::formatTimeLength(start, end);
        check(s.index("5 minutes") != -1, "5 minutes");
        check(s.index("30 seconds") != -1, "30 seconds in duration");
    }

    // Hours, minutes, seconds
    {
        CxTime start(2020, 1, 1, 12, 0, 0);
        CxTime end(2020, 1, 1, 14, 30, 45);
        CxString s = CxTime::formatTimeLength(start, end);
        // Check for hours (may be singular or plural)
        check(s.index("hour") != -1, "hours in duration");
        check(s.index("30 minutes") != -1, "30 minutes in duration");
        check(s.index("45 seconds") != -1, "45 seconds in duration");
    }
}

//-----------------------------------------------------------------------------------------
// CxTime edge cases and special dates
//-----------------------------------------------------------------------------------------
void testTimeEdgeCases() {
    printf("\n== CxTime Edge Cases ==\n");

    // Leap year Feb 29
    {
        CxTime t(2020, 2, 29, 12, 0, 0);
        check(t.year() == 2020, "leap year 2020");
        check(t.month() == 2, "February");
        check(t.day() == 29, "29th day");
    }

    // Year 2000 (Y2K)
    {
        CxTime t(2000, 1, 1, 0, 0, 0);
        check(t.year() == 2000, "Y2K year");
    }

    // Far future date
    {
        CxTime t(2035, 12, 31, 23, 59, 59);
        check(t.year() == 2035, "future year 2035");
    }

    // End of month days
    {
        CxTime jan31(2020, 1, 31, 12, 0, 0);
        CxTime apr30(2020, 4, 30, 12, 0, 0);
        check(jan31.day() == 31, "January 31");
        check(apr30.day() == 30, "April 30");
    }

    // epochSeconds round-trip
    {
        CxTime t1(2020, 6, 15, 12, 30, 45);
        time_t epoch = t1.epochSeconds();
        CxTime t2(epoch);
        check(t2 == t1, "epoch seconds round-trip");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxTime Test Suite\n");
    printf("=================\n");

    testTimeConstructors();
    testTimeAccessors();
    testTimeOffsets();
    testTimeCompareAssign();
    testTimeNow();
    testTimeStrings();
    testMonthNumber();
    testDayBoundaries();
    testFormatTimeLength();
    testTimeEdgeCases();

    printf("\n=================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
