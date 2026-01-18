//-----------------------------------------------------------------------------------------
// cxdatetime_test.cpp - CxDateTime unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
// CxDateTime constructor tests
//-----------------------------------------------------------------------------------------
void testDateTimeConstructors() {
    printf("\n== CxDateTime Constructor Tests ==\n");

    // Default constructor - current time
    {
        int before = CxDateTime::now_epochSeconds();
        CxDateTime dt;
        int after = CxDateTime::now_epochSeconds();

        check(dt.epochSeconds() >= before, "default ctor >= before");
        check(dt.epochSeconds() <= after, "default ctor <= after");
    }

    // Constructor with epoch seconds
    {
        time_t epoch = 1000000000;  // Sep 9, 2001
        CxDateTime dt(epoch);
        check(dt.epochSeconds() == epoch, "epoch ctor stores correct value");
    }

    // Constructor with time components (local timezone)
    {
        CxDateTime dt(2020, 6, 15, 12, 30, 45);
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second);
        check(year == 2020, "component ctor year");
        check(month == 6, "component ctor month");
        check(day == 15, "component ctor day");
        check(hour == 12, "component ctor hour");
        check(minute == 30, "component ctor minute");
        check(second == 45, "component ctor second");
    }

    // Constructor with time string
    {
        CxDateTime dt("2020-06-15 12:30:45");
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second);
        check(year == 2020, "string ctor year");
        check(month == 6, "string ctor month");
        check(day == 15, "string ctor day");
        check(hour == 12, "string ctor hour");
        check(minute == 30, "string ctor minute");
        check(second == 45, "string ctor second");
    }

    // Copy constructor
    {
        CxDateTime dt1(2021, 3, 20, 8, 15, 30);
        CxDateTime dt2(dt1);
        check(dt2.epochSeconds() == dt1.epochSeconds(), "copy ctor copies value");
    }
}

//-----------------------------------------------------------------------------------------
// CxDateTime timezone tests
//-----------------------------------------------------------------------------------------
void testDateTimeTimezones() {
    printf("\n== CxDateTime Timezone Tests ==\n");

    // zoneNameToZone for valid timezone
    {
        cctz::time_zone nyZone = CxDateTime::zoneNameToZone("America/New_York");
        CxDateTime dt(2020, 6, 15, 12, 0, 0, nyZone);
        CxString tzName = dt.timeZoneName();
        check(tzName.index("New_York") != -1, "NY timezone name");
    }

    // zoneNameToZone for LA timezone
    {
        cctz::time_zone laZone = CxDateTime::zoneNameToZone("America/Los_Angeles");
        CxDateTime dt(2020, 6, 15, 12, 0, 0, laZone);
        CxString tzName = dt.timeZoneName();
        check(tzName.index("Los_Angeles") != -1, "LA timezone name");
    }

    // Same instant in different timezones has same epoch seconds
    {
        cctz::time_zone nyZone = CxDateTime::zoneNameToZone("America/New_York");
        cctz::time_zone laZone = CxDateTime::zoneNameToZone("America/Los_Angeles");

        // Create same wall clock time in different zones
        CxDateTime dtNY(2020, 6, 15, 15, 0, 0, nyZone);  // 3 PM in NY
        CxDateTime dtLA(2020, 6, 15, 12, 0, 0, laZone);  // 12 PM in LA

        // These should be the same instant (NY is 3 hours ahead of LA in summer)
        check(dtNY.epochSeconds() == dtLA.epochSeconds(), "same instant different zones");
    }

    // breakdown with different timezone
    {
        cctz::time_zone utcZone = CxDateTime::zoneNameToZone("UTC");
        cctz::time_zone nyZone = CxDateTime::zoneNameToZone("America/New_York");

        CxDateTime dt(2020, 6, 15, 12, 0, 0, utcZone);  // Noon UTC

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, nyZone);

        // In summer, NY is UTC-4, so noon UTC = 8 AM NY
        check(hour == 8, "breakdown to different timezone hour");
    }
}

//-----------------------------------------------------------------------------------------
// CxDateTime offset tests
//-----------------------------------------------------------------------------------------
void testDateTimeOffsets() {
    printf("\n== CxDateTime Offset Tests ==\n");

    // offsetSeconds
    {
        CxDateTime dt(2020, 1, 1, 12, 0, 0);
        int original = dt.epochSeconds();
        dt.offsetSeconds(30);
        check(dt.epochSeconds() == original + 30, "offsetSeconds adds 30");
    }

    // offsetMinutes
    {
        CxDateTime dt(2020, 1, 1, 12, 0, 0);
        int original = dt.epochSeconds();
        dt.offsetMinutes(5);
        check(dt.epochSeconds() == original + 300, "offsetMinutes adds 5 min");
    }

    // offsetHours
    {
        CxDateTime dt(2020, 1, 1, 12, 0, 0);
        int original = dt.epochSeconds();
        dt.offsetHours(3);
        check(dt.epochSeconds() == original + 10800, "offsetHours adds 3 hr");
    }

    // offsetDays
    {
        CxDateTime dt(2020, 1, 1, 12, 0, 0);
        int original = dt.epochSeconds();
        dt.offsetDays(1);
        check(dt.epochSeconds() == original + 86400, "offsetDays adds 1 day");
    }

    // offsetMonths
    {
        CxDateTime dt(2020, 1, 15, 12, 0, 0);
        dt.offsetMonths(1);
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second);
        check(month == 2, "offsetMonths moves to February");
    }

    // offsetYears
    {
        CxDateTime dt(2020, 6, 15, 12, 0, 0);
        dt.offsetYears(1);
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second);
        check(year == 2021, "offsetYears moves to 2021");
    }

    // Negative offset
    {
        CxDateTime dt(2020, 6, 15, 12, 0, 30);
        dt.offsetSeconds(-15);
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second);
        check(second == 15, "negative offsetSeconds");
    }
}

//-----------------------------------------------------------------------------------------
// CxDateTime format tests
//-----------------------------------------------------------------------------------------
void testDateTimeFormat() {
    printf("\n== CxDateTime Format Tests ==\n");

    // Default format
    {
        CxDateTime dt(2020, 6, 15, 12, 30, 45);
        CxString s = dt.format();
        check(s.length() > 0, "default format non-empty");
        check(s.index("2020") != -1, "default format contains year");
    }

    // Custom format YYYY-MM-DD
    {
        CxDateTime dt(2020, 6, 15, 12, 30, 45);
        CxString s = dt.format("%Y-%m-%d");
        check(strcmp(s.data(), "2020-06-15") == 0, "format YYYY-MM-DD");
    }

    // Custom format HH:MM:SS
    {
        CxDateTime dt(2020, 6, 15, 12, 30, 45);
        CxString s = dt.format("%H:%M:%S");
        check(strcmp(s.data(), "12:30:45") == 0, "format HH:MM:SS");
    }

    // Format with timezone
    {
        cctz::time_zone utcZone = CxDateTime::zoneNameToZone("UTC");
        CxDateTime dt(2020, 6, 15, 12, 0, 0, utcZone);
        CxString s = dt.format(utcZone, "%Y-%m-%d %H:%M:%S");
        check(strcmp(s.data(), "2020-06-15 12:00:00") == 0, "format with UTC zone");
    }

    // Format same time in different timezone
    {
        cctz::time_zone utcZone = CxDateTime::zoneNameToZone("UTC");
        cctz::time_zone nyZone = CxDateTime::zoneNameToZone("America/New_York");

        CxDateTime dt(2020, 6, 15, 12, 0, 0, utcZone);  // Noon UTC

        CxString utcStr = dt.format(utcZone, "%H:%M");
        CxString nyStr = dt.format(nyZone, "%H:%M");

        check(strcmp(utcStr.data(), "12:00") == 0, "format UTC hour");
        check(strcmp(nyStr.data(), "08:00") == 0, "format NY hour (UTC-4 in summer)");
    }
}

//-----------------------------------------------------------------------------------------
// CxDateTime comparison and assignment tests
//-----------------------------------------------------------------------------------------
void testDateTimeCompareAssign() {
    printf("\n== CxDateTime Compare/Assign Tests ==\n");

    // operator==
    {
        CxDateTime dt1(2020, 6, 15, 12, 0, 0);
        CxDateTime dt2(2020, 6, 15, 12, 0, 0);
        check(dt1 == dt2, "equal times compare equal");
    }

    // operator== with different times
    {
        CxDateTime dt1(2020, 6, 15, 12, 0, 0);
        CxDateTime dt2(2020, 6, 15, 12, 0, 1);
        check(!(dt1 == dt2), "different times not equal");
    }

    // Assignment operator
    {
        CxDateTime dt1(2020, 6, 15, 12, 30, 45);
        CxDateTime dt2;
        dt2 = dt1;
        check(dt2 == dt1, "assignment copies value");
    }

    // Self-assignment
    {
        CxDateTime dt(2020, 6, 15, 12, 0, 0);
        int original = dt.epochSeconds();
        dt = dt;
        check(dt.epochSeconds() == original, "self-assignment preserves value");
    }
}

//-----------------------------------------------------------------------------------------
// CxDateTime static method tests
//-----------------------------------------------------------------------------------------
void testDateTimeStaticMethods() {
    printf("\n== CxDateTime Static Method Tests ==\n");

    // now_epochSeconds
    {
        time_t before = ::time(NULL);
        int now = CxDateTime::now_epochSeconds();
        time_t after = ::time(NULL);

        check(now >= before, "now_epochSeconds >= system before");
        check(now <= after, "now_epochSeconds <= system after");
    }

    // now_tp
    {
        auto tp = CxDateTime::now_tp();
        CxDateTime dt(tp);
        int systemNow = CxDateTime::now_epochSeconds();
        // Should be within 1 second
        check(dt.epochSeconds() >= systemNow - 1, "now_tp creates valid time");
        check(dt.epochSeconds() <= systemNow + 1, "now_tp within 1 sec of now");
    }

    // firstSecondOfTheDay
    {
        CxDateTime dt(2020, 6, 15, 14, 30, 45);
        CxDateTime first = CxDateTime::firstSecondOfTheDay(dt);

        long year, month, day, hour, minute, second;
        first.breakdown(&year, &month, &day, &hour, &minute, &second);

        check(year == 2020, "firstSecondOfTheDay year");
        check(month == 6, "firstSecondOfTheDay month");
        check(day == 15, "firstSecondOfTheDay day");
        check(hour == 0, "firstSecondOfTheDay hour");
        check(minute == 0, "firstSecondOfTheDay minute");
        check(second == 0, "firstSecondOfTheDay second");
    }

    // lastSecondOfTheDay
    {
        CxDateTime dt(2020, 6, 15, 14, 30, 45);
        CxDateTime last = CxDateTime::lastSecondOfTheDay(dt);

        long year, month, day, hour, minute, second;
        last.breakdown(&year, &month, &day, &hour, &minute, &second);

        check(year == 2020, "lastSecondOfTheDay year");
        check(month == 6, "lastSecondOfTheDay month");
        check(day == 15, "lastSecondOfTheDay day");
        check(hour == 23, "lastSecondOfTheDay hour");
        check(minute == 59, "lastSecondOfTheDay minute");
        check(second == 59, "lastSecondOfTheDay second");
    }

    // formatTimeLength
    {
        CxDateTime start(2020, 1, 1, 12, 0, 0);
        CxDateTime end(2020, 1, 1, 14, 30, 45);
        CxString s = CxDateTime::formatTimeLength(start, end);
        check(s.index("hour") != -1, "formatTimeLength contains hours");
        check(s.index("minute") != -1, "formatTimeLength contains minutes");
    }

    // formatTimeLength zero duration
    {
        CxDateTime dt(2020, 1, 1, 12, 0, 0);
        CxString s = CxDateTime::formatTimeLength(dt, dt);
        check(s.index("0 seconds") != -1, "formatTimeLength zero duration");
    }
}

//-----------------------------------------------------------------------------------------
// CxDateTime edge cases
//-----------------------------------------------------------------------------------------
void testDateTimeEdgeCases() {
    printf("\n== CxDateTime Edge Cases ==\n");

    // Leap year Feb 29
    {
        CxDateTime dt(2020, 2, 29, 12, 0, 0);
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second);
        check(year == 2020, "leap year 2020");
        check(month == 2, "February");
        check(day == 29, "29th day");
    }

    // DST transition (spring forward) - 2 AM becomes 3 AM
    {
        cctz::time_zone nyZone = CxDateTime::zoneNameToZone("America/New_York");
        // March 8, 2020 - DST starts in NY
        CxDateTime dt(2020, 3, 8, 1, 30, 0, nyZone);
        dt.offsetHours(1);  // 1:30 AM + 1 hour
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, nyZone);
        // After spring forward, 1:30 + 1 hr = 3:30 (skipping 2:xx)
        check(hour == 3, "DST spring forward hour");
    }

    // Year boundary
    {
        CxDateTime dt(2020, 12, 31, 23, 59, 59);
        dt.offsetSeconds(1);
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second);
        check(year == 2021, "year boundary crosses to new year");
        check(month == 1, "year boundary January");
        check(day == 1, "year boundary day 1");
    }

    // Epoch 0
    {
        CxDateTime dt((time_t)0);
        cctz::time_zone utcZone = CxDateTime::zoneNameToZone("UTC");
        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utcZone);
        check(year == 1970, "epoch 0 year 1970");
        check(month == 1, "epoch 0 January");
        check(day == 1, "epoch 0 day 1");
    }
}

//-----------------------------------------------------------------------------------------
// CxDateTime with specific timezone constructors
//-----------------------------------------------------------------------------------------
void testDateTimeWithTimezone() {
    printf("\n== CxDateTime Timezone Constructor Tests ==\n");

    // Constructor with epoch seconds and timezone
    {
        time_t epoch = 1592222400;  // 2020-06-15 12:00:00 UTC
        cctz::time_zone utcZone = CxDateTime::zoneNameToZone("UTC");
        CxDateTime dt(epoch, utcZone);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, utcZone);

        check(hour == 12, "epoch with UTC zone hour");
    }

    // Constructor with string and timezone
    {
        cctz::time_zone nyZone = CxDateTime::zoneNameToZone("America/New_York");
        CxDateTime dt("2020-06-15 12:00:00", nyZone);

        long year, month, day, hour, minute, second;
        dt.breakdown(&year, &month, &day, &hour, &minute, &second, nyZone);

        check(year == 2020, "string with zone year");
        check(hour == 12, "string with zone hour");
    }

    // Constructor with time_point and timezone
    {
        auto tp = CxDateTime::now_tp();
        cctz::time_zone utcZone = CxDateTime::zoneNameToZone("UTC");
        CxDateTime dt(tp, utcZone);

        CxString tzName = dt.timeZoneName();
        check(tzName.index("UTC") != -1, "time_point with UTC zone name");
    }

    // Constructor with just timezone (now in that zone)
    {
        cctz::time_zone tokyoZone = CxDateTime::zoneNameToZone("Asia/Tokyo");
        CxDateTime dt(tokyoZone);

        CxString tzName = dt.timeZoneName();
        check(tzName.index("Tokyo") != -1, "now with Tokyo zone");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxDateTime Test Suite\n");
    printf("=====================\n");

    testDateTimeConstructors();
    testDateTimeTimezones();
    testDateTimeOffsets();
    testDateTimeFormat();
    testDateTimeCompareAssign();
    testDateTimeStaticMethods();
    testDateTimeEdgeCases();
    testDateTimeWithTimezone();

    printf("\n=====================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
