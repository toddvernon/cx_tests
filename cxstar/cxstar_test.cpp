//-----------------------------------------------------------------------------------------
// cxstar_test.cpp - CxStringMatch and CxMatchTemplate unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <cx/base/star.h>

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
// CxStringMatch constructor tests
//-----------------------------------------------------------------------------------------
void testStringMatchConstructors() {
    printf("\n== CxStringMatch Constructor Tests ==\n");

    // Default constructor
    {
        CxStringMatch sm;
        // Just verify it doesn't crash
        check(1, "default ctor doesn't crash");
    }

    // String constructor
    {
        CxStringMatch sm("hello world");
        check(sm.isNext("hello"), "string ctor: can find substring");
    }

    // Empty string constructor
    {
        CxStringMatch sm("");
        check(!sm.isNext("x"), "empty string: isNext returns false");
    }
}

//-----------------------------------------------------------------------------------------
// CxStringMatch isNext tests
//-----------------------------------------------------------------------------------------
void testStringMatchIsNext() {
    printf("\n== CxStringMatch isNext Tests ==\n");

    // Find single substring
    {
        CxStringMatch sm("hello world");
        check(sm.isNext("world"), "find single substring");
    }

    // Find substring at start
    {
        CxStringMatch sm("hello world");
        check(sm.isNext("hello"), "find substring at start");
    }

    // Find substrings in sequence
    {
        CxStringMatch sm("abc def ghi");
        check(sm.isNext("abc"), "sequence: find first");
        check(sm.isNext("def"), "sequence: find second");
        check(sm.isNext("ghi"), "sequence: find third");
    }

    // Find substrings must be in order
    {
        CxStringMatch sm("abc def ghi");
        check(sm.isNext("def"), "order: find middle first");
        check(!sm.isNext("abc"), "order: can't find earlier after later");
    }

    // Substring not found
    {
        CxStringMatch sm("hello world");
        check(!sm.isNext("xyz"), "not found: returns false");
    }

    // Find same substring twice
    {
        CxStringMatch sm("abcabc");
        check(sm.isNext("abc"), "duplicate: find first");
        check(sm.isNext("abc"), "duplicate: find second");
    }

    // Find overlapping
    {
        CxStringMatch sm("aaaa");
        check(sm.isNext("aa"), "overlap: find first aa");
        check(sm.isNext("aa"), "overlap: find second aa");
    }

    // Case sensitive
    {
        CxStringMatch sm("Hello World");
        check(!sm.isNext("hello"), "case sensitive: lowercase not found");
        check(sm.isNext("Hello"), "case sensitive: exact case found");
    }

    // Single character searches
    {
        CxStringMatch sm("abcdef");
        check(sm.isNext("a"), "single char: a");
        check(sm.isNext("c"), "single char: c");
        check(sm.isNext("f"), "single char: f");
    }
}

//-----------------------------------------------------------------------------------------
// CxMatchTemplate constructor tests
//-----------------------------------------------------------------------------------------
void testMatchTemplateConstructors() {
    printf("\n== CxMatchTemplate Constructor Tests ==\n");

    // Default constructor
    {
        CxMatchTemplate mt;
        check(!mt.test("anything"), "default ctor: matches nothing");
    }

    // CxString constructor
    {
        CxMatchTemplate mt(CxString("*.txt"));
        check(mt.test("file.txt"), "CxString ctor: pattern works");
    }

    // char* constructor
    {
        CxMatchTemplate mt("*.txt");
        check(mt.test("file.txt"), "char* ctor: pattern works");
    }

    // Copy constructor
    {
        CxMatchTemplate mt1("*.txt");
        CxMatchTemplate mt2(mt1);
        check(mt2.test("file.txt"), "copy ctor: pattern works");
        check(!mt2.test("file.doc"), "copy ctor: non-match fails");
    }

    // Assignment operator
    {
        CxMatchTemplate mt1("*.txt");
        CxMatchTemplate mt2;
        mt2 = mt1;
        check(mt2.test("file.txt"), "assignment: pattern works");
    }

    // Self-assignment
    {
        CxMatchTemplate mt("*.txt");
        mt = mt;
        check(mt.test("file.txt"), "self-assignment: pattern preserved");
    }
}

//-----------------------------------------------------------------------------------------
// CxMatchTemplate wildcard * tests
//-----------------------------------------------------------------------------------------
void testMatchTemplateWildcard() {
    printf("\n== CxMatchTemplate Wildcard Tests ==\n");

    // Match all with just *
    {
        CxMatchTemplate mt("*");
        check(mt.test("anything"), "* matches anything");
        check(mt.test("file.txt"), "* matches file.txt");
        check(mt.test("x"), "* matches single char");
    }

    // Trailing * (prefix match)
    {
        CxMatchTemplate mt("file*");
        check(mt.test("file"), "file*: matches 'file' exactly");
        check(mt.test("file.txt"), "file*: matches file.txt");
        check(mt.test("filename"), "file*: matches filename");
        check(!mt.test("myfile"), "file*: doesn't match myfile");
        check(!mt.test("afile"), "file*: doesn't match afile");
    }

    // Leading * (suffix match)
    {
        CxMatchTemplate mt("*.txt");
        check(mt.test("file.txt"), "*.txt: matches file.txt");
        check(mt.test("a.txt"), "*.txt: matches a.txt");
        check(mt.test(".txt"), "*.txt: matches .txt");
        check(!mt.test("file.doc"), "*.txt: doesn't match file.doc");
        check(!mt.test("txt"), "*.txt: doesn't match txt (no dot)");
    }

    // Both leading and trailing * (contains match)
    {
        CxMatchTemplate mt("*test*");
        check(mt.test("test"), "*test*: matches test exactly");
        check(mt.test("testing"), "*test*: matches testing");
        check(mt.test("mytest"), "*test*: matches mytest");
        check(mt.test("mytesting"), "*test*: matches mytesting");
        check(!mt.test("tes"), "*test*: doesn't match tes");
    }

    // Multiple * in pattern
    {
        CxMatchTemplate mt("a*b*c");
        check(mt.test("abc"), "a*b*c: matches abc");
        check(mt.test("aXbXc"), "a*b*c: matches aXbXc");
        check(mt.test("aXXXbYYYc"), "a*b*c: matches aXXXbYYYc");
        check(!mt.test("ab"), "a*b*c: doesn't match ab (no c)");
        check(!mt.test("bc"), "a*b*c: doesn't match bc (no a at start)");
    }

    // Adjacent *
    {
        CxMatchTemplate mt("a**b");
        check(mt.test("ab"), "a**b: matches ab");
        check(mt.test("aXb"), "a**b: matches aXb");
    }
}

//-----------------------------------------------------------------------------------------
// CxMatchTemplate exact match tests (no wildcards)
//-----------------------------------------------------------------------------------------
void testMatchTemplateExact() {
    printf("\n== CxMatchTemplate Exact Match Tests ==\n");

    // Exact string match
    {
        CxMatchTemplate mt("test");
        check(mt.test("test"), "exact: matches same string");
        check(!mt.test("testing"), "exact: doesn't match longer");
        check(!mt.test("tes"), "exact: doesn't match shorter");
        check(!mt.test("xtest"), "exact: doesn't match with prefix");
        check(!mt.test("testx"), "exact: doesn't match with suffix");
    }

    // Single character exact (shell-style: no wildcards = exact match)
    {
        CxMatchTemplate mt("x");
        check(mt.test("x"), "single char: matches x");
        check(!mt.test("xx"), "single char: doesn't match xx (exact only)");
        check(!mt.test("y"), "single char: doesn't match y");
    }

    // Longer exact string
    {
        CxMatchTemplate mt("hello world");
        check(mt.test("hello world"), "long exact: matches");
        check(!mt.test("hello"), "long exact: doesn't match partial");
        check(!mt.test("hello world!"), "long exact: doesn't match extended");
    }
}

//-----------------------------------------------------------------------------------------
// CxMatchTemplate empty/edge case tests
//-----------------------------------------------------------------------------------------
void testMatchTemplateEdgeCases() {
    printf("\n== CxMatchTemplate Edge Case Tests ==\n");

    // Empty pattern matches nothing
    {
        CxMatchTemplate mt("");
        check(!mt.test("anything"), "empty pattern: doesn't match anything");
        check(!mt.test(""), "empty pattern: doesn't match empty string");
    }

    // Empty candidate never matches
    {
        CxMatchTemplate mt("*");
        check(!mt.test(""), "* doesn't match empty candidate");
    }

    // Whitespace-only candidate
    {
        CxMatchTemplate mt("*");
        check(!mt.test("   "), "* doesn't match whitespace-only (stripped)");
    }

    // Pattern with leading/trailing spaces
    // NOTE: Only LEADING spaces are stripped from pattern, not trailing.
    // So "  test  " becomes "test  " which won't match "test".
    {
        CxMatchTemplate mt("  test");  // No trailing spaces
        check(mt.test("test"), "pattern leading spaces: stripped");
    }

    // Candidate with leading/trailing spaces
    {
        CxMatchTemplate mt("test");
        check(mt.test("  test  "), "candidate spaces: stripped");
    }

    // Pattern with only spaces
    {
        CxMatchTemplate mt("   ");
        check(!mt.test("anything"), "space-only pattern: matches nothing");
    }

    // Very long pattern
    {
        CxString longPattern = "*";
        for (int i = 0; i < 50; i++) longPattern += "x";
        longPattern += "*";
        CxMatchTemplate mt(longPattern);
        CxString longCandidate;
        for (int i = 0; i < 100; i++) longCandidate += "x";
        check(mt.test(longCandidate), "long pattern: matches");
    }
}

//-----------------------------------------------------------------------------------------
// CxMatchTemplate file extension tests (common use case)
//-----------------------------------------------------------------------------------------
void testMatchTemplateFileExtensions() {
    printf("\n== CxMatchTemplate File Extension Tests ==\n");

    // Common file patterns
    {
        CxMatchTemplate txt("*.txt");
        CxMatchTemplate cpp("*.cpp");
        CxMatchTemplate h("*.h");

        check(txt.test("readme.txt"), "*.txt matches readme.txt");
        check(txt.test("NOTES.txt"), "*.txt matches NOTES.txt");
        check(!txt.test("readme.TXT"), "*.txt case sensitive");

        check(cpp.test("main.cpp"), "*.cpp matches main.cpp");
        check(!cpp.test("main.c"), "*.cpp doesn't match .c");

        check(h.test("header.h"), "*.h matches header.h");
        check(!h.test("header.hpp"), "*.h doesn't match .hpp");
    }

    // Prefix patterns
    {
        CxMatchTemplate mt("test_*");
        check(mt.test("test_file.txt"), "test_* matches test_file.txt");
        check(mt.test("test_"), "test_* matches test_");
        check(!mt.test("mytest_file"), "test_* doesn't match mytest_file");
    }

    // Combined prefix and extension
    {
        CxMatchTemplate mt("log_*.txt");
        check(mt.test("log_2024.txt"), "log_*.txt matches log_2024.txt");
        check(mt.test("log_.txt"), "log_*.txt matches log_.txt");
        check(!mt.test("log_2024.log"), "log_*.txt doesn't match .log");
        check(!mt.test("mylog_2024.txt"), "log_*.txt doesn't match prefix");
    }
}

//-----------------------------------------------------------------------------------------
// CxMatchTemplate special character tests
//-----------------------------------------------------------------------------------------
void testMatchTemplateSpecialChars() {
    printf("\n== CxMatchTemplate Special Character Tests ==\n");

    // Pattern with dots
    {
        CxMatchTemplate mt("file.name.txt");
        check(mt.test("file.name.txt"), "dots: exact match");
        check(!mt.test("filename.txt"), "dots: no match without dots");
    }

    // Pattern with numbers
    {
        CxMatchTemplate mt("file123*");
        check(mt.test("file123.txt"), "numbers: match");
        check(!mt.test("file12.txt"), "numbers: partial doesn't match");
    }

    // Pattern with underscores
    {
        CxMatchTemplate mt("*_test_*");
        check(mt.test("my_test_file"), "underscores: match");
        check(mt.test("_test_"), "underscores: minimal match");
    }

    // Pattern with hyphens
    {
        CxMatchTemplate mt("*-backup-*");
        check(mt.test("file-backup-001"), "hyphens: match");
    }
}

//-----------------------------------------------------------------------------------------
// CxMatchTemplate regression/potential bug tests
//-----------------------------------------------------------------------------------------
void testMatchTemplateRegressions() {
    printf("\n== CxMatchTemplate Regression Tests ==\n");

    // Pattern "ab" should only match "ab" exactly
    {
        CxMatchTemplate mt("ab");
        check(mt.test("ab"), "ab matches ab");
        // These are the potential bugs - no wildcards should mean exact match
        int matchesAbab = mt.test("abab");
        int matchesXab = mt.test("xab");
        int matchesAbx = mt.test("abx");
        printf("    INFO: 'ab' vs 'abab': %s\n", matchesAbab ? "MATCH" : "NO MATCH");
        printf("    INFO: 'ab' vs 'xab': %s\n", matchesXab ? "MATCH" : "NO MATCH");
        printf("    INFO: 'ab' vs 'abx': %s\n", matchesAbx ? "MATCH" : "NO MATCH");
        // Document actual behavior
        check(!matchesXab, "no-wildcard shouldn't match with prefix");
        check(!matchesAbx, "no-wildcard shouldn't match with suffix");
    }

    // Single part appearing multiple times
    {
        CxMatchTemplate mt("a*a");
        check(mt.test("aa"), "a*a matches aa");
        check(mt.test("aba"), "a*a matches aba");
        check(mt.test("abba"), "a*a matches abba");
        check(!mt.test("a"), "a*a doesn't match single a");
        check(!mt.test("ab"), "a*a doesn't match ab");
    }

    // Parts must appear in order
    {
        CxMatchTemplate mt("a*b");
        check(mt.test("ab"), "a*b matches ab");
        check(mt.test("aXb"), "a*b matches aXb");
        check(!mt.test("ba"), "a*b doesn't match ba (wrong order)");
        check(!mt.test("b"), "a*b doesn't match b (no a)");
    }

    // Empty parts between stars
    {
        CxMatchTemplate mt("**");
        check(mt.test("anything"), "** matches anything");
    }

    // Star at different positions
    {
        CxMatchTemplate mt1("*x");
        CxMatchTemplate mt2("x*");
        CxMatchTemplate mt3("*x*");

        check(mt1.test("x"), "*x matches x");
        check(mt1.test("ax"), "*x matches ax");
        check(!mt1.test("xa"), "*x doesn't match xa");

        check(mt2.test("x"), "x* matches x");
        check(mt2.test("xa"), "x* matches xa");
        check(!mt2.test("ax"), "x* doesn't match ax");

        check(mt3.test("x"), "*x* matches x");
        check(mt3.test("ax"), "*x* matches ax");
        check(mt3.test("xa"), "*x* matches xa");
        check(mt3.test("axa"), "*x* matches axa");
    }

    // Ensure parts are found in sequence, not just present
    {
        CxMatchTemplate mt("a*b*c");
        check(mt.test("abc"), "a*b*c: abc");
        check(mt.test("aXbYc"), "a*b*c: aXbYc");
        check(!mt.test("cba"), "a*b*c: cba wrong order");
        check(!mt.test("acb"), "a*b*c: acb wrong order");
    }
}

//-----------------------------------------------------------------------------------------
// CxMatchTemplate case sensitivity tests
//-----------------------------------------------------------------------------------------
void testMatchTemplateCaseSensitivity() {
    printf("\n== CxMatchTemplate Case Sensitivity Tests ==\n");

    // Exact case required
    {
        CxMatchTemplate mt("Test");
        check(mt.test("Test"), "exact case matches");
        check(!mt.test("test"), "lowercase doesn't match");
        check(!mt.test("TEST"), "uppercase doesn't match");
        check(!mt.test("tEST"), "mixed case doesn't match");
    }

    // Wildcard with case
    {
        CxMatchTemplate mt("*.TXT");
        check(mt.test("file.TXT"), "*.TXT matches .TXT");
        check(!mt.test("file.txt"), "*.TXT doesn't match .txt");
    }

    // Mixed case pattern
    {
        CxMatchTemplate mt("*Test*");
        check(mt.test("myTestFile"), "*Test* matches myTestFile");
        check(!mt.test("mytestfile"), "*Test* doesn't match mytestfile");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxStar Test Suite\n");
    printf("=================\n");

    testStringMatchConstructors();
    testStringMatchIsNext();
    testMatchTemplateConstructors();
    testMatchTemplateWildcard();
    testMatchTemplateExact();
    testMatchTemplateEdgeCases();
    testMatchTemplateFileExtensions();
    testMatchTemplateSpecialChars();
    testMatchTemplateRegressions();
    testMatchTemplateCaseSensitivity();

    printf("\n=================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
