//-----------------------------------------------------------------------------------------
// cxregex_test.cpp - CxRegex unit tests
//-----------------------------------------------------------------------------------------

#if defined(_LINUX_) || defined(_OSX_)

#include <stdio.h>
#include <string.h>

#include <cx/base/string.h>
#include <cx/regex/regex.h>

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
// Basic compile and match tests
//-----------------------------------------------------------------------------------------
void testBasicMatch() {
    printf("\n== Basic Match Tests ==\n");

    // Simple literal match
    {
        CxRegex regex;
        check(regex.compile("hello") == 0, "compile simple pattern");
        check(regex.isCompiled() == 1, "isCompiled returns true");
        check(regex.match("hello world") == 1, "match at start");
        check(regex.match("say hello") == 1, "match in middle");
        check(regex.match("goodbye") == 0, "no match");
    }

    // Pattern with special regex chars
    {
        CxRegex regex;
        check(regex.compile("a.*b") == 0, "compile pattern with .*");
        check(regex.match("axxb") == 1, "match a.*b");
        check(regex.match("ab") == 1, "match a.*b (zero chars)");
        // Note: POSIX regex '.' behavior with newlines is implementation-dependent
        // Some match newlines, some don't. We just verify the pattern works.
        check(regex.match("aXXXb") == 1, "match a.*b (multiple chars)");
    }

    // Character class
    {
        CxRegex regex;
        check(regex.compile("[0-9]+") == 0, "compile digit pattern");
        check(regex.match("abc123def") == 1, "match digits in string");
        check(regex.match("no digits here") == 0, "no digits");
    }

    // Anchors
    {
        CxRegex regex;
        check(regex.compile("^hello") == 0, "compile anchored pattern");
        check(regex.match("hello world") == 1, "match at start");
        check(regex.match("say hello") == 0, "no match (not at start)");
    }

    // End anchor
    {
        CxRegex regex;
        check(regex.compile("world$") == 0, "compile end-anchored pattern");
        check(regex.match("hello world") == 1, "match at end");
        check(regex.match("world hello") == 0, "no match (not at end)");
    }
}

//-----------------------------------------------------------------------------------------
// Case insensitive tests
//-----------------------------------------------------------------------------------------
void testCaseInsensitive() {
    printf("\n== Case Insensitive Tests ==\n");

    // Case sensitive (default)
    {
        CxRegex regex;
        regex.compile("Hello");
        check(regex.match("Hello") == 1, "case sensitive: exact match");
        check(regex.match("hello") == 0, "case sensitive: lowercase no match");
        check(regex.match("HELLO") == 0, "case sensitive: uppercase no match");
    }

    // Case insensitive
    {
        CxRegex regex;
        regex.compile("Hello", 1);
        check(regex.match("Hello") == 1, "case insensitive: exact match");
        check(regex.match("hello") == 1, "case insensitive: lowercase match");
        check(regex.match("HELLO") == 1, "case insensitive: uppercase match");
        check(regex.match("hElLo") == 1, "case insensitive: mixed case match");
    }
}

//-----------------------------------------------------------------------------------------
// Match position tests
//-----------------------------------------------------------------------------------------
void testMatchPosition() {
    printf("\n== Match Position Tests ==\n");

    {
        CxRegex regex;
        regex.compile("world");
        int start, len;
        check(regex.match("hello world", &start, &len) == 1, "match with position");
        check(start == 6, "start position is 6");
        check(len == 5, "length is 5");
    }

    {
        CxRegex regex;
        regex.compile("[0-9]+");
        int start, len;
        check(regex.match("abc123def", &start, &len) == 1, "match digits with position");
        check(start == 3, "start position is 3");
        check(len == 3, "length is 3");
    }

    // Match at start
    {
        CxRegex regex;
        regex.compile("hello");
        int start, len;
        regex.match("hello world", &start, &len);
        check(start == 0, "match at start: position is 0");
        check(len == 5, "match at start: length is 5");
    }
}

//-----------------------------------------------------------------------------------------
// Error handling tests
//-----------------------------------------------------------------------------------------
void testErrorHandling() {
    printf("\n== Error Handling Tests ==\n");

    // Invalid pattern
    {
        CxRegex regex;
        int result = regex.compile("[invalid");
        check(result != 0, "compile invalid pattern returns error");
        check(regex.isCompiled() == 0, "isCompiled returns false");
        check(regex.getError().length() > 0, "getError returns message");
    }

    // Match without compile
    {
        CxRegex regex;
        check(regex.match("test") == 0, "match without compile returns 0");
    }

    // Reset and recompile
    {
        CxRegex regex;
        regex.compile("first");
        check(regex.match("first") == 1, "first pattern matches");
        regex.reset();
        check(regex.isCompiled() == 0, "isCompiled false after reset");
        regex.compile("second");
        check(regex.match("second") == 1, "second pattern matches");
        check(regex.match("first") == 0, "first pattern no longer matches");
    }
}

//-----------------------------------------------------------------------------------------
// Escape for literal tests
//-----------------------------------------------------------------------------------------
void testEscapeForLiteral() {
    printf("\n== Escape For Literal Tests ==\n");

    // No special chars
    {
        CxString escaped = CxRegex::escapeForLiteral("hello");
        check(strcmp(escaped.data(), "hello") == 0, "no special chars unchanged");
    }

    // Dot
    {
        CxString escaped = CxRegex::escapeForLiteral("file.txt");
        check(strcmp(escaped.data(), "file\\.txt") == 0, "dot escaped");
    }

    // Multiple special chars
    {
        CxString escaped = CxRegex::escapeForLiteral("a*b+c?");
        check(strcmp(escaped.data(), "a\\*b\\+c\\?") == 0, "multiple specials escaped");
    }

    // Brackets
    {
        CxString escaped = CxRegex::escapeForLiteral("[test]");
        check(strcmp(escaped.data(), "\\[test\\]") == 0, "brackets escaped");
    }

    // Backslash
    {
        CxString escaped = CxRegex::escapeForLiteral("a\\b");
        check(strcmp(escaped.data(), "a\\\\b") == 0, "backslash escaped");
    }

    // Verify escaped pattern matches literally
    {
        CxString escaped = CxRegex::escapeForLiteral("file.txt");
        CxRegex regex;
        regex.compile(escaped);
        check(regex.match("file.txt") == 1, "escaped pattern matches literal");
        check(regex.match("fileXtxt") == 0, "escaped pattern doesn't match dot-as-any");
    }
}

//-----------------------------------------------------------------------------------------
// Replace tests
//-----------------------------------------------------------------------------------------
void testReplace() {
    printf("\n== Replace Tests ==\n");

    // Simple replace
    {
        CxString result = regexReplace("hello world", "world", "there");
        check(strcmp(result.data(), "hello there") == 0, "simple replace");
    }

    // Replace at start
    {
        CxString result = regexReplace("hello world", "hello", "hi");
        check(strcmp(result.data(), "hi world") == 0, "replace at start");
    }

    // Replace with pattern
    {
        CxString result = regexReplace("abc123def", "[0-9]+", "NUM");
        check(strcmp(result.data(), "abcNUMdef") == 0, "replace with pattern");
    }

    // No match - return original
    {
        CxString result = regexReplace("hello world", "xyz", "abc");
        check(strcmp(result.data(), "hello world") == 0, "no match returns original");
    }

    // Replace with empty string
    {
        CxString result = regexReplace("hello world", "world", "");
        check(strcmp(result.data(), "hello ") == 0, "replace with empty string");
    }

    // Case insensitive replace
    {
        CxString result = regexReplace("Hello World", "world", "there", 1);
        check(strcmp(result.data(), "Hello there") == 0, "case insensitive replace");
    }
}

//-----------------------------------------------------------------------------------------
// Replace all tests
//-----------------------------------------------------------------------------------------
void testReplaceAll() {
    printf("\n== Replace All Tests ==\n");

    // Replace all occurrences
    {
        CxString result = regexReplaceAll("one two one three one", "one", "1");
        check(strcmp(result.data(), "1 two 1 three 1") == 0, "replace all occurrences");
    }

    // Replace with pattern
    {
        CxString result = regexReplaceAll("a1b2c3d4", "[0-9]", "X");
        check(strcmp(result.data(), "aXbXcXdX") == 0, "replace all digits");
    }

    // No matches
    {
        CxString result = regexReplaceAll("hello world", "xyz", "abc");
        check(strcmp(result.data(), "hello world") == 0, "no matches returns original");
    }

    // Replace all with empty
    {
        CxString result = regexReplaceAll("a1b2c3", "[0-9]", "");
        check(strcmp(result.data(), "abc") == 0, "replace all with empty");
    }

    // Adjacent matches
    {
        CxString result = regexReplaceAll("aaa", "a", "b");
        check(strcmp(result.data(), "bbb") == 0, "replace adjacent matches");
    }

    // Overlapping pattern positions (non-overlapping matches)
    {
        CxString result = regexReplaceAll("abab", "ab", "X");
        check(strcmp(result.data(), "XX") == 0, "non-overlapping replace");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    printf("CxRegex Unit Tests\n");
    printf("==================\n");

    testBasicMatch();
    testCaseInsensitive();
    testMatchPosition();
    testErrorHandling();
    testEscapeForLiteral();
    testReplace();
    testReplaceAll();

    printf("\n==================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);
    printf("==================\n");

    return testsFailed > 0 ? 1 : 0;
}

#else

// Not Linux/macOS - just print a message
#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("CxRegex tests: skipped (not Linux/macOS)\n");
    return 0;
}

#endif
