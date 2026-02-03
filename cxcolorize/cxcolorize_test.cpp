//-----------------------------------------------------------------------------------------
// cxcolorize_test.cpp - Comprehensive colorization and escape sequence tests
//
// This test suite verifies that ANSI escape sequences are properly handled throughout
// the CxString and CxColor classes, and in colorization operations.
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <cx/base/string.h>
#include <cx/screen/color.h>

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

// Helper to check if a string contains the ESC character (0x1B)
int containsEsc(const char* str) {
    if (!str) return 0;
    while (*str) {
        if (*str == '\033') return 1;
        str++;
    }
    return 0;
}

// Helper to count ESC characters in a string
int countEsc(const char* str) {
    int count = 0;
    if (!str) return 0;
    while (*str) {
        if (*str == '\033') count++;
        str++;
    }
    return count;
}

// Helper to print hex dump of a string for debugging
void hexDump(const char* label, const char* str, int len) {
    printf("  %s (len=%d): ", label, len);
    for (int i = 0; i < len && i < 40; i++) {
        printf("%02X ", (unsigned char)str[i]);
    }
    if (len > 40) printf("...");
    printf("\n");
}

//-----------------------------------------------------------------------------------------
// Test: CxString construction with escape characters
//-----------------------------------------------------------------------------------------
void testCxStringConstructionWithEsc() {
    printf("\n== CxString Construction with ESC ==\n");

    // Test 1: Construct from literal escape sequence
    {
        CxString s("\033[38;2;200;150;255m");
        check(s.length() == 19, "escape sequence string has correct length (19)");
        check(containsEsc(s.data()), "escape sequence string contains ESC char");
        check(s.data()[0] == '\033', "first char is ESC (0x1B)");
        check(s.data()[1] == '[', "second char is '['");

        if (!containsEsc(s.data())) {
            hexDump("actual", s.data(), s.length());
        }
    }

    // Test 2: Construct from char array with escape
    {
        char buffer[30];
        sprintf(buffer, "\033[38;2;%d;%d;%dm", 200, 150, 255);
        CxString s(buffer);
        check(containsEsc(s.data()), "sprintf-built string contains ESC char");
        check(s.data()[0] == '\033', "sprintf string first char is ESC");

        if (!containsEsc(s.data())) {
            hexDump("sprintf result", buffer, strlen(buffer));
            hexDump("CxString data", s.data(), s.length());
        }
    }

    // Test 3: Copy constructor preserves ESC
    {
        CxString original("\033[31m");
        CxString copy(original);
        check(containsEsc(copy.data()), "copy constructor preserves ESC char");
        check(copy.data()[0] == '\033', "copy first char is ESC");
    }

    // Test 4: Assignment operator preserves ESC
    {
        CxString original("\033[31m");
        CxString assigned;
        assigned = original;
        check(containsEsc(assigned.data()), "assignment operator preserves ESC char");
    }
}

//-----------------------------------------------------------------------------------------
// Test: CxString concatenation with escape characters
//-----------------------------------------------------------------------------------------
void testCxStringConcatenationWithEsc() {
    printf("\n== CxString Concatenation with ESC ==\n");

    // Test 1: Simple concatenation - ESC + text
    {
        CxString color("\033[31m");
        CxString text("hello");
        CxString result = color + text;

        check(containsEsc(result.data()), "color + text preserves ESC");
        check(result.data()[0] == '\033', "result starts with ESC");
        check(result.length() == 10, "result has correct length");

        if (!containsEsc(result.data())) {
            hexDump("color", color.data(), color.length());
            hexDump("result", result.data(), result.length());
        }
    }

    // Test 2: Concatenation - text + ESC + text
    {
        CxString before("pre");
        CxString color("\033[31m");
        CxString after("post");
        CxString result = before + color + after;

        check(containsEsc(result.data()), "text + color + text preserves ESC");
        check(result.length() == 12, "result has correct length");

        if (!containsEsc(result.data())) {
            hexDump("result", result.data(), result.length());
        }
    }

    // Test 3: Multiple concatenations (simulating colorization loop)
    {
        CxString result;
        CxString color1("\033[31m");
        CxString reset("\033[0m");
        CxString word("test");

        result = result + color1;
        check(containsEsc(result.data()), "step 1: empty + color preserves ESC");

        result = result + word;
        check(containsEsc(result.data()), "step 2: color + word preserves ESC");

        result = result + reset;
        check(countEsc(result.data()) == 2, "step 3: should have 2 ESC chars");

        if (countEsc(result.data()) != 2) {
            hexDump("final result", result.data(), result.length());
        }
    }

    // Test 4: Chain of concatenations like in colorizeKeywords
    {
        CxString line("static void foo");
        CxString keywordColor("\033[38;2;200;150;255m");
        CxString typeColor("\033[38;2;100;220;220m");
        CxString reset("\033[0m");

        // Simulate: colorize "static" then "void"
        CxString result;
        result = result + keywordColor;
        result = result + CxString("static");
        result = result + reset;
        result = result + CxString(" ");
        result = result + typeColor;
        result = result + CxString("void");
        result = result + reset;
        result = result + CxString(" foo");

        int escCount = countEsc(result.data());
        check(escCount == 4, "chained colorization has 4 ESC chars");

        if (escCount != 4) {
            printf("  Expected 4 ESC chars, found %d\n", escCount);
            hexDump("result", result.data(), result.length());
        }
    }
}

//-----------------------------------------------------------------------------------------
// Test: CxString subString with escape characters
//-----------------------------------------------------------------------------------------
void testCxStringSubStringWithEsc() {
    printf("\n== CxString subString with ESC ==\n");

    // Test 1: subString that includes ESC at start
    {
        CxString s("\033[31mhello");
        CxString sub = s.subString(0, 5);  // Should get "\033[31m"
        check(containsEsc(sub.data()), "subString from start preserves ESC");
        check(sub.data()[0] == '\033', "subString first char is ESC");
    }

    // Test 2: subString after ESC sequence
    {
        CxString s("\033[31mhello");
        CxString sub = s.subString(5, 5);  // Should get "hello"
        check(!containsEsc(sub.data()), "subString after ESC has no ESC");
        check(strcmp(sub.data(), "hello") == 0, "subString content is correct");
    }

    // Test 3: subString with ESC in middle
    {
        CxString s("pre\033[31mpost");
        CxString sub = s.subString(0, 8);  // Should include ESC
        check(containsEsc(sub.data()), "subString including middle ESC preserves it");
    }
}

//-----------------------------------------------------------------------------------------
// Test: CxString append with escape characters
//-----------------------------------------------------------------------------------------
void testCxStringAppendWithEsc() {
    printf("\n== CxString append with ESC ==\n");

    // Test 1: Append ESC string to empty
    {
        CxString s;
        CxString color("\033[31m");
        s.append(color);
        check(containsEsc(s.data()), "append ESC to empty preserves ESC");
    }

    // Test 2: Append text to ESC string
    {
        CxString s("\033[31m");
        s.append(CxString("hello"));
        check(containsEsc(s.data()), "append text to ESC string preserves ESC");
        check(s.data()[0] == '\033', "first char still ESC after append");
    }

    // Test 3: Multiple appends with ESC
    {
        CxString s;
        s.append(CxString("\033[31m"));
        s.append(CxString("word"));
        s.append(CxString("\033[0m"));

        int escCount = countEsc(s.data());
        check(escCount == 2, "multiple appends preserve all ESC chars");

        if (escCount != 2) {
            hexDump("result", s.data(), s.length());
        }
    }
}

//-----------------------------------------------------------------------------------------
// Test: CxColor terminal string generation
//-----------------------------------------------------------------------------------------
void testCxColorTerminalString() {
    printf("\n== CxColor Terminal String Generation ==\n");

    // Test 1: RGB foreground color
    {
        CxRGBForegroundColor color(200, 150, 255);
        CxString ts = color.terminalString();

        check(ts.length() > 0, "RGB color generates non-empty string");
        check(containsEsc(ts.data()), "RGB color string contains ESC");
        check(ts.data()[0] == '\033', "RGB color string starts with ESC");
        check(ts.data()[1] == '[', "RGB color string has '[' after ESC");

        if (!containsEsc(ts.data())) {
            hexDump("terminalString", ts.data(), ts.length());
        }
    }

    // Test 2: RGB color reset string
    {
        CxRGBForegroundColor color(200, 150, 255);
        CxString rs = color.resetTerminalString();

        check(containsEsc(rs.data()), "RGB reset string contains ESC");
    }

    // Test 3: ANSI foreground color
    {
        CxAnsiForegroundColor color("BRIGHT_YELLOW");
        CxString ts = color.terminalString();

        if (ts.length() > 0) {
            check(containsEsc(ts.data()), "ANSI color string contains ESC");
        }
    }

    // Test 4: Copy RGB color and get terminal string
    {
        CxRGBForegroundColor original(100, 200, 150);
        CxRGBForegroundColor copy(original);
        CxString ts = copy.terminalString();

        check(containsEsc(ts.data()), "copied color terminal string has ESC");
    }
}

//-----------------------------------------------------------------------------------------
// Test: Simulated colorization operations
//-----------------------------------------------------------------------------------------
void testSimulatedColorization() {
    printf("\n== Simulated Colorization ==\n");

    // This simulates what injectTextConstantEntryExitText does

    CxString line("static void foo(int x)");
    CxString keywordColor("\033[38;2;200;150;255m");
    CxString typeColor("\033[38;2;100;220;220m");
    CxString reset("\033[0m");

    // Test 1: Colorize a single keyword
    {
        // Simulate finding "static" at position 0-6
        CxString result;
        int start = 0;
        int end = 6;  // "static"

        // Build: color + "static" + reset + rest_of_line
        result = result + keywordColor;
        result = result + line.subString(start, end - start);
        result = result + reset;
        result = result + line.subString(end, line.length() - end);

        int escCount = countEsc(result.data());
        check(escCount == 2, "single keyword colorization has 2 ESC");

        if (escCount != 2) {
            printf("  Found %d ESC chars instead of 2\n", escCount);
            hexDump("keywordColor", keywordColor.data(), keywordColor.length());
            hexDump("reset", reset.data(), reset.length());
            hexDump("result", result.data(), result.length());
        }
    }

    // Test 2: Colorize two keywords in same line (like static + void)
    {
        // First pass: colorize "static"
        CxString pass1;
        pass1 = pass1 + keywordColor;
        pass1 = pass1 + CxString("static");
        pass1 = pass1 + reset;
        pass1 = pass1 + CxString(" void foo(int x)");

        int pass1Esc = countEsc(pass1.data());
        check(pass1Esc == 2, "pass 1 has 2 ESC chars");

        // Second pass: colorize "void" in the already-colorized line
        // This is where the bug might be - when we process a line that
        // already contains escape sequences

        // Find "void" - it should be at a position that accounts for
        // the escape sequences we already added
        // In pass1: "\033[38;2;200;150;255mstatic\033[0m void foo(int x)"
        // The actual text positions are shifted due to escape sequences

        // For this test, let's just verify the escape chars are preserved
        // when we do further string operations on pass1

        CxString pass2;
        pass2 = pass2 + pass1.subString(0, 10);  // Some portion
        pass2 = pass2 + pass1.subString(10, pass1.length() - 10);  // Rest

        int pass2Esc = countEsc(pass2.data());
        check(pass2Esc == pass1Esc, "reconstructing line preserves ESC count");

        if (pass2Esc != pass1Esc) {
            printf("  pass1 ESC: %d, pass2 ESC: %d\n", pass1Esc, pass2Esc);
            hexDump("pass1", pass1.data(), pass1.length());
            hexDump("pass2", pass2.data(), pass2.length());
        }
    }
}

//-----------------------------------------------------------------------------------------
// Test: Edge cases
//-----------------------------------------------------------------------------------------
void testEdgeCases() {
    printf("\n== Edge Cases ==\n");

    // Test 1: Empty string concatenation
    {
        CxString empty;
        CxString color("\033[31m");
        CxString result = empty + color;
        check(containsEsc(result.data()), "empty + color preserves ESC");
    }

    // Test 2: ESC at various positions
    {
        CxString s1("a\033b");  // ESC in middle
        check(containsEsc(s1.data()), "ESC in middle preserved");

        CxString s2("\033ab");  // ESC at start
        check(s2.data()[0] == '\033', "ESC at start preserved");

        CxString s3("ab\033");  // ESC at end
        check(containsEsc(s3.data()), "ESC at end preserved");
    }

    // Test 3: Multiple ESC characters
    {
        CxString s("\033[31m\033[0m\033[32m");
        int count = countEsc(s.data());
        check(count == 3, "string with 3 ESC chars has count 3");
    }

    // Test 4: Long string with ESC
    {
        CxString color("\033[38;2;200;150;255m");
        CxString text("This is a much longer string that we want to colorize in the terminal");
        CxString reset("\033[0m");

        CxString result = color + text + reset;
        check(countEsc(result.data()) == 2, "long colorized string has 2 ESC");
    }
}

//-----------------------------------------------------------------------------------------
// Test: Byte-level verification
//-----------------------------------------------------------------------------------------
void testByteLevelVerification() {
    printf("\n== Byte-Level Verification ==\n");

    // Test 1: Verify exact bytes of escape sequence
    {
        CxString s("\033[31m");
        const char* d = s.data();

        check((unsigned char)d[0] == 0x1B, "byte 0 is 0x1B (ESC)");
        check(d[1] == '[', "byte 1 is '['");
        check(d[2] == '3', "byte 2 is '3'");
        check(d[3] == '1', "byte 3 is '1'");
        check(d[4] == 'm', "byte 4 is 'm'");

        printf("  Actual bytes: ");
        for (int i = 0; i < s.length(); i++) {
            printf("%02X ", (unsigned char)d[i]);
        }
        printf("\n");
    }

    // Test 2: Verify bytes after concatenation
    {
        CxString a("\033[");
        CxString b("31m");
        CxString result = a + b;

        const char* d = result.data();
        check((unsigned char)d[0] == 0x1B, "concat byte 0 is 0x1B (ESC)");
        check(d[1] == '[', "concat byte 1 is '['");

        printf("  Concat result bytes: ");
        for (int i = 0; i < result.length(); i++) {
            printf("%02X ", (unsigned char)d[i]);
        }
        printf("\n");
    }

    // Test 3: Verify CxColor output bytes
    {
        CxRGBForegroundColor color(255, 0, 0);  // Red
        CxString ts = color.terminalString();

        if (ts.length() > 0) {
            const char* d = ts.data();
            check((unsigned char)d[0] == 0x1B, "CxColor byte 0 is 0x1B (ESC)");
            check(d[1] == '[', "CxColor byte 1 is '['");

            printf("  CxColor bytes: ");
            for (int i = 0; i < ts.length() && i < 25; i++) {
                printf("%02X ", (unsigned char)d[i]);
            }
            printf("\n");
        }
    }
}

//-----------------------------------------------------------------------------------------
// Test: Exact simulation of injectTextConstantEntryExitText
//-----------------------------------------------------------------------------------------

// Simulated parseTextConstant - finds a keyword as a whole word
int simParseTextConstant(CxString s, CxString item, unsigned long initialPos,
                          unsigned long *start, unsigned long *end)
{
    int c = s.index(item, initialPos);
    if (c == -1) return 0;

    unsigned long startPos = c;
    unsigned long endPos = c + item.length();

    // Check char before (word boundary)
    if (startPos > 0) {
        char charBefore = s.data()[startPos-1];
        if (charBefore != ' ' && charBefore != '\t' && charBefore != '(' &&
            charBefore != ')' && charBefore != '{' && charBefore != '}' &&
            charBefore != ',' && charBefore != ';' && charBefore != '\033') {
            return 0;
        }
    }

    // Check char after (word boundary)
    if (endPos < (unsigned long)s.length()) {
        char charAfter = s.data()[endPos];
        if (charAfter != ' ' && charAfter != '\t' && charAfter != '(' &&
            charAfter != ')' && charAfter != '{' && charAfter != '}' &&
            charAfter != ',' && charAfter != ';' && charAfter != '\0') {
            return 0;
        }
    }

    *start = startPos;
    *end = endPos;
    return 1;
}

// Exact simulation of the fixed injectTextConstantEntryExitText
CxString simInjectTextConstantEntryExitText(CxString line, CxString item,
                                             CxString entryString, CxString exitString)
{
    unsigned long initial = 0;
    unsigned long start = 0;
    unsigned long end = 0;
    CxString result;
    unsigned long lastPos = 0;

    while (simParseTextConstant(line, item, initial, &start, &end)) {
        if (start > lastPos) {
            result = result + line.subString(lastPos, start - lastPos);
        }
        result = result + entryString;
        result = result + line.subString(start, end - start);
        result = result + exitString;
        lastPos = end;
        initial = end;
    }

    if (lastPos < (unsigned long)line.length()) {
        result = result + line.subString(lastPos, line.length() - lastPos);
    }

    return (lastPos > 0) ? result : line;
}

void testExactInjectSimulation() {
    printf("\n== Exact injectTextConstantEntryExitText Simulation ==\n");

    CxString keywordColor("\033[38;2;200;150;255m");
    CxString typeColor("\033[38;2;100;220;220m");
    CxString reset("\033[0m");

    // Test 1: Single keyword colorization
    {
        CxString line("static void foo");
        CxString result = simInjectTextConstantEntryExitText(line, CxString("static"),
                                                              keywordColor, reset);
        int escCount = countEsc(result.data());
        check(escCount == 2, "single inject: 2 ESC chars");

        if (escCount != 2) {
            printf("  Line: '%s'\n", line.data());
            printf("  Result length: %d\n", result.length());
            hexDump("result", result.data(), result.length());
        }
    }

    // Test 2: Two keywords on same line - sequential processing
    {
        CxString line("static void foo");

        // First pass: colorize "static"
        CxString pass1 = simInjectTextConstantEntryExitText(line, CxString("static"),
                                                             keywordColor, reset);
        int pass1Esc = countEsc(pass1.data());
        check(pass1Esc == 2, "pass1: colorize 'static' has 2 ESC");

        printf("  Pass1 result: ");
        for (int i = 0; i < pass1.length() && i < 60; i++) {
            unsigned char c = pass1.data()[i];
            if (c == 0x1B) printf("\\033");
            else if (c >= 32 && c < 127) printf("%c", c);
            else printf("\\x%02X", c);
        }
        printf("\n");

        // Second pass: colorize "void" in the already-colorized line
        CxString pass2 = simInjectTextConstantEntryExitText(pass1, CxString("void"),
                                                             typeColor, reset);
        int pass2Esc = countEsc(pass2.data());
        check(pass2Esc == 4, "pass2: colorize 'void' has 4 ESC");

        printf("  Pass2 result: ");
        for (int i = 0; i < pass2.length() && i < 80; i++) {
            unsigned char c = pass2.data()[i];
            if (c == 0x1B) printf("\\033");
            else if (c >= 32 && c < 127) printf("%c", c);
            else printf("\\x%02X", c);
        }
        printf("\n");

        if (pass2Esc != 4) {
            printf("  Expected 4 ESC chars, found %d\n", pass2Esc);
            hexDump("pass2", pass2.data(), pass2.length());
        }
    }

    // Test 3: Multiple keywords via colorizeKeywords-like loop
    {
        CxString line("if (x == 0) return;");
        const char* keywords = "if,return";

        // Process each keyword
        const char* p = keywords;
        while (*p) {
            const char* start = p;
            while (*p && *p != ',') p++;
            int keywordLen = p - start;
            if (keywordLen > 0 && keywordLen < 64) {
                char keyword[64];
                strncpy(keyword, start, keywordLen);
                keyword[keywordLen] = '\0';
                line = simInjectTextConstantEntryExitText(line, CxString(keyword),
                                                          keywordColor, reset);
            }
            if (*p == ',') p++;
        }

        int escCount = countEsc(line.data());
        check(escCount == 4, "multi-keyword loop: 4 ESC chars");

        printf("  Multi-keyword result: ");
        for (int i = 0; i < line.length() && i < 80; i++) {
            unsigned char c = line.data()[i];
            if (c == 0x1B) printf("\\033");
            else if (c >= 32 && c < 127) printf("%c", c);
            else printf("\\x%02X", c);
        }
        printf("\n");
    }
}

//-----------------------------------------------------------------------------------------
// Test: File output verification
//-----------------------------------------------------------------------------------------
void testFileOutputVerification() {
    printf("\n== File Output Verification ==\n");

    CxString color("\033[38;2;200;150;255m");
    CxString text("hello");
    CxString reset("\033[0m");
    CxString result = color + text + reset;

    // Write to file and read back
    FILE* f = fopen("/tmp/colorize_test.bin", "wb");
    if (f) {
        fwrite(result.data(), 1, result.length(), f);
        fclose(f);

        // Read back
        f = fopen("/tmp/colorize_test.bin", "rb");
        if (f) {
            char buffer[256];
            int bytesRead = fread(buffer, 1, 255, f);
            buffer[bytesRead] = '\0';
            fclose(f);

            check(containsEsc(buffer), "file contains ESC after write/read");
            check((unsigned char)buffer[0] == 0x1B, "file first byte is ESC");

            printf("  File bytes: ");
            for (int i = 0; i < bytesRead && i < 40; i++) {
                printf("%02X ", (unsigned char)buffer[i]);
            }
            printf("\n");
        }
    }
}

//-----------------------------------------------------------------------------------------
// Test: Terminal output demo
//-----------------------------------------------------------------------------------------
void testTerminalOutputDemo() {
    printf("\n== Terminal Output Demo ==\n");
    printf("  The following line should be colored if terminal supports it:\n");

    CxString color("\033[38;2;200;150;255m");
    CxString text("This text should be purple/pink");
    CxString reset("\033[0m");
    CxString result = color + text + reset;

    printf("  ");
    fwrite(result.data(), 1, result.length(), stdout);
    printf("\n");

    // Also test via puts-style
    printf("  Direct fwrite of raw bytes: ");
    const char* raw = "\033[38;2;100;220;220mCyan text\033[0m";
    fwrite(raw, 1, strlen(raw), stdout);
    printf("\n");
}

//-----------------------------------------------------------------------------------------
// Test: Diagnose ESC filtering
//-----------------------------------------------------------------------------------------
void testDiagnoseEscFiltering() {
    printf("\n== Diagnose ESC Filtering ==\n");

    // Test 1: Write ESC byte directly via different methods
    printf("  Test 1 - putchar ESC then '[': ");
    putchar('\033');
    putchar('[');
    printf("31mRED");
    putchar('\033');
    putchar('[');
    printf("0m\n");

    // Test 2: Write using write() syscall
    printf("  Test 2 - write() syscall: ");
    fflush(stdout);
    const char* escSeq = "\033[32mGREEN\033[0m\n";
    write(1, escSeq, strlen(escSeq));

    // Test 3: fprintf with ESC
    printf("  Test 3 - fprintf: ");
    fprintf(stdout, "\033[33mYELLOW\033[0m\n");

    // Test 4: printf with %c for ESC
    printf("  Test 4 - printf %%c: ");
    printf("%c[34mBLUE%c[0m\n", '\033', '\033');

    // Test 5: Byte-by-byte comparison
    printf("  Test 5 - Byte-by-byte to stdout:\n");
    CxString s("\033[35mMAGENTA\033[0m");
    printf("    CxString bytes: ");
    for (int i = 0; i < s.length(); i++) {
        printf("%02X ", (unsigned char)s.data()[i]);
    }
    printf("\n");
    printf("    Writing byte-by-byte: ");
    fflush(stdout);
    for (int i = 0; i < s.length(); i++) {
        write(1, &s.data()[i], 1);
    }
    write(1, "\n", 1);

    // Test 6: Check if stdout is a tty
    printf("  Test 6 - isatty(1) = %d\n", isatty(1));

    // Test 7: Check what happens with explicit hex byte
    printf("  Test 7 - Explicit hex 0x1B: ");
    char buf[20];
    buf[0] = 0x1B;  // ESC
    buf[1] = '[';
    buf[2] = '3';
    buf[3] = '6';
    buf[4] = 'm';
    buf[5] = 'C';
    buf[6] = 'Y';
    buf[7] = 'A';
    buf[8] = 'N';
    buf[9] = 0x1B;  // ESC
    buf[10] = '[';
    buf[11] = '0';
    buf[12] = 'm';
    buf[13] = '\n';
    buf[14] = '\0';
    fflush(stdout);
    write(1, buf, 14);
}

//-----------------------------------------------------------------------------------------
// Exclusion Region Support - simulates MarkUp's exclusion region logic
//-----------------------------------------------------------------------------------------
#define MAX_COLOR_REGIONS 32

struct ColorRegion {
    int start;      // starting position (inclusive)
    int end;        // ending position (exclusive)
};

struct ColorRegions {
    ColorRegion regions[MAX_COLOR_REGIONS];
    int count;
};

// Simulates MarkUp::findExclusionRegions - finds strings and comments to exclude
void simFindExclusionRegions(CxString line, const char* commentMarker, ColorRegions* regions)
{
    regions->count = 0;
    const char* s = line.data();
    int len = line.length();
    int i = 0;

    while (i < len && regions->count < MAX_COLOR_REGIONS) {
        // Check for string literal
        if (s[i] == '"' || s[i] == '\'') {
            char quote = s[i];
            int start = i;
            i++;  // skip opening quote
            while (i < len) {
                if (s[i] == '\\' && i + 1 < len) {
                    i += 2;  // skip escaped char
                } else if (s[i] == quote) {
                    i++;  // skip closing quote
                    break;
                } else {
                    i++;
                }
            }
            regions->regions[regions->count].start = start;
            regions->regions[regions->count].end = i;
            regions->count++;
        }
        // Check for inline comment
        else if (commentMarker && commentMarker[0] && s[i] == commentMarker[0]) {
            int match = 1;
            for (int j = 1; commentMarker[j]; j++) {
                if (i + j >= len || s[i + j] != commentMarker[j]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                regions->regions[regions->count].start = i;
                regions->regions[regions->count].end = len;  // comment goes to end
                regions->count++;
                break;  // done, rest is comment
            } else {
                i++;
            }
        } else {
            i++;
        }
    }
}

// Simulates MarkUp::isInsideRegion
int simIsInsideRegion(int pos, ColorRegions* regions)
{
    for (int i = 0; i < regions->count; i++) {
        if (pos >= regions->regions[i].start && pos < regions->regions[i].end) {
            return 1;
        }
    }
    return 0;
}

// Simulates colorizeNumbersWithExclusions - only colorizes numbers outside exclusion regions
CxString simColorizeNumbersWithExclusions(CxString line, CxString colorStart, CxString colorEnd, ColorRegions* regions)
{
    CxString result;
    const char* s = line.data();
    int len = line.length();
    int i = 0;

    while (i < len) {
        // Check for number start (not inside identifier)
        if ((s[i] >= '0' && s[i] <= '9') &&
            (i == 0 || !(s[i-1] >= 'a' && s[i-1] <= 'z') &&
                       !(s[i-1] >= 'A' && s[i-1] <= 'Z') &&
                       s[i-1] != '_')) {

            int numStart = i;

            // Skip if inside exclusion region
            if (simIsInsideRegion(numStart, regions)) {
                // Just copy the character, don't colorize
                char buf[2] = { s[i], '\0' };
                result = result + CxString(buf);
                i++;
                continue;
            }

            // Scan full number
            while (i < len && ((s[i] >= '0' && s[i] <= '9') ||
                               s[i] == '.' || s[i] == 'x' || s[i] == 'X' ||
                               (s[i] >= 'a' && s[i] <= 'f') ||
                               (s[i] >= 'A' && s[i] <= 'F'))) {
                i++;
            }

            // Colorize the number
            result = result + colorStart;
            result = result + line.subString(numStart, i - numStart);
            result = result + colorEnd;
        } else {
            char buf[2] = { s[i], '\0' };
            result = result + CxString(buf);
            i++;
        }
    }

    return result;
}

// Simulates colorizeKeywordsWithExclusions - works on original positions
// Uses a two-pass approach: first find all keywords to colorize, then apply
CxString simColorizeKeywordsWithExclusions(CxString line, const char* keywords,
                                            CxString colorStart, CxString colorEnd,
                                            ColorRegions* regions)
{
    // First pass: find all keyword positions to colorize (on original line)
    struct KeywordPos {
        int start;
        int end;
    };
    KeywordPos positions[64];
    int posCount = 0;

    const char* p = keywords;
    while (*p) {
        const char* start = p;
        while (*p && *p != ',') p++;
        int keywordLen = p - start;
        if (keywordLen > 0 && keywordLen < 64) {
            char keyword[64];
            strncpy(keyword, start, keywordLen);
            keyword[keywordLen] = '\0';

            unsigned long initial = 0;
            unsigned long kstart = 0;
            unsigned long kend = 0;

            while (simParseTextConstant(line, CxString(keyword), initial, &kstart, &kend)) {
                // Check against original line positions (regions)
                if (!simIsInsideRegion(kstart, regions)) {
                    if (posCount < 64) {
                        positions[posCount].start = kstart;
                        positions[posCount].end = kend;
                        posCount++;
                    }
                }
                initial = kend;
            }
        }
        if (*p == ',') p++;
    }

    // Sort positions by start (simple bubble sort since count is small)
    for (int i = 0; i < posCount - 1; i++) {
        for (int j = 0; j < posCount - i - 1; j++) {
            if (positions[j].start > positions[j+1].start) {
                KeywordPos tmp = positions[j];
                positions[j] = positions[j+1];
                positions[j+1] = tmp;
            }
        }
    }

    // Second pass: build result with colorized keywords
    CxString result;
    int lastEnd = 0;
    for (int i = 0; i < posCount; i++) {
        // Add text before this keyword
        if (positions[i].start > lastEnd) {
            result = result + line.subString(lastEnd, positions[i].start - lastEnd);
        }
        // Add colorized keyword
        result = result + colorStart;
        result = result + line.subString(positions[i].start, positions[i].end - positions[i].start);
        result = result + colorEnd;
        lastEnd = positions[i].end;
    }
    // Add remaining text
    if (lastEnd < line.length()) {
        result = result + line.subString(lastEnd, line.length() - lastEnd);
    }

    return (posCount > 0) ? result : line;
}

//-----------------------------------------------------------------------------------------
// Test: Exclusion Region Finding
//-----------------------------------------------------------------------------------------
void testExclusionRegionFinding() {
    printf("\n== Exclusion Region Finding ==\n");

    // Test 1: Find double-quoted string
    {
        CxString line("printf(\"hello world\");");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        check(regions.count == 1, "finds one string region");
        check(regions.regions[0].start == 7, "string starts at position 7");
        check(regions.regions[0].end == 20, "string ends at position 20");
    }

    // Test 2: Find single-quoted string
    {
        CxString line("char c = 'x';");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        check(regions.count == 1, "finds one char literal region");
    }

    // Test 3: Find inline comment
    {
        CxString line("int x; // comment here");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        check(regions.count == 1, "finds one comment region");
        check(regions.regions[0].start == 7, "comment starts at //");
        check(regions.regions[0].end == 22, "comment extends to end");
    }

    // Test 4: Find both string and comment
    {
        CxString line("printf(\"test\"); // done");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        check(regions.count == 2, "finds string and comment regions");
    }

    // Test 5: Escaped quote in string
    {
        CxString line("char *s = \"say \\\"hi\\\"\";");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        check(regions.count == 1, "finds one region for string with escapes");
        // The string should span from first " to last "
    }

    // Test 6: Python-style comment
    {
        CxString line("x = 5  # python comment");
        ColorRegions regions;
        simFindExclusionRegions(line, "#", &regions);

        check(regions.count == 1, "finds Python comment");
        check(regions.regions[0].start == 7, "Python comment starts at #");
    }
}

//-----------------------------------------------------------------------------------------
// Test: Keywords NOT colorized inside strings
//-----------------------------------------------------------------------------------------
void testKeywordsNotColorizedInStrings() {
    printf("\n== Keywords NOT Colorized In Strings ==\n");

    CxString keywordColor("\033[38;2;200;150;255m");
    CxString reset("\033[0m");

    // Test 1: "return" inside string should NOT be colorized
    {
        CxString line("printf(\"return value is %d\", x);");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeKeywordsWithExclusions(line, "return",
                                                             keywordColor, reset, &regions);

        int escCount = countEsc(result.data());
        check(escCount == 0, "return inside string: 0 ESC chars (not colorized)");

        if (escCount != 0) {
            printf("  Line: %s\n", line.data());
            printf("  Result ESC count: %d (should be 0)\n", escCount);
        }
    }

    // Test 2: "if" inside string should NOT be colorized, but "if" outside should be
    {
        CxString line("if (strcmp(s, \"if true\") == 0)");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeKeywordsWithExclusions(line, "if",
                                                             keywordColor, reset, &regions);

        int escCount = countEsc(result.data());
        check(escCount == 2, "if: 2 ESC for outside, 0 for inside string");

        if (escCount != 2) {
            printf("  Line: %s\n", line.data());
            printf("  Result ESC count: %d (should be 2)\n", escCount);
        }
    }

    // Test 3: Multiple keywords, some inside string, some outside
    {
        CxString line("return printf(\"void return\");");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeKeywordsWithExclusions(line, "return,void",
                                                             keywordColor, reset, &regions);

        // "return" at start should be colorized (2 ESC)
        // "void" and "return" inside string should NOT be colorized
        int escCount = countEsc(result.data());
        check(escCount == 2, "only outside 'return' colorized: 2 ESC");

        if (escCount != 2) {
            printf("  Line: %s\n", line.data());
            printf("  Result ESC count: %d (should be 2)\n", escCount);
        }
    }
}

//-----------------------------------------------------------------------------------------
// Test: Keywords NOT colorized inside comments
//-----------------------------------------------------------------------------------------
void testKeywordsNotColorizedInComments() {
    printf("\n== Keywords NOT Colorized In Comments ==\n");

    CxString keywordColor("\033[38;2;200;150;255m");
    CxString reset("\033[0m");

    // Test 1: "return" in comment should NOT be colorized
    {
        CxString line("x++; // return early if needed");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeKeywordsWithExclusions(line, "return,if",
                                                             keywordColor, reset, &regions);

        int escCount = countEsc(result.data());
        check(escCount == 0, "keywords in comment: 0 ESC chars");

        if (escCount != 0) {
            printf("  Line: %s\n", line.data());
            printf("  Result ESC count: %d (should be 0)\n", escCount);
        }
    }

    // Test 2: keyword before comment, keyword in comment
    {
        CxString line("return 0; // if we get here");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeKeywordsWithExclusions(line, "return,if",
                                                             keywordColor, reset, &regions);

        int escCount = countEsc(result.data());
        check(escCount == 2, "return before comment: 2 ESC, if in comment: 0");

        if (escCount != 2) {
            printf("  Line: %s\n", line.data());
            printf("  Result ESC count: %d (should be 2)\n", escCount);
        }
    }

    // Test 3: Shell-style comment
    {
        CxString line("echo $PATH  # if PATH is set");
        ColorRegions regions;
        simFindExclusionRegions(line, "#", &regions);

        CxString result = simColorizeKeywordsWithExclusions(line, "if",
                                                             keywordColor, reset, &regions);

        int escCount = countEsc(result.data());
        check(escCount == 0, "if in # comment: 0 ESC chars");
    }
}

//-----------------------------------------------------------------------------------------
// Test: Numbers NOT colorized inside strings
//-----------------------------------------------------------------------------------------
void testNumbersNotColorizedInStrings() {
    printf("\n== Numbers NOT Colorized In Strings ==\n");

    CxString numberColor("\033[38;2;180;255;180m");
    CxString reset("\033[0m");

    // Test 1: Number inside string should NOT be colorized
    {
        CxString line("printf(\"Error code 42\");");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeNumbersWithExclusions(line, numberColor, reset, &regions);

        int escCount = countEsc(result.data());
        check(escCount == 0, "42 inside string: 0 ESC chars");

        if (escCount != 0) {
            printf("  Line: %s\n", line.data());
            printf("  Result ESC count: %d (should be 0)\n", escCount);
        }
    }

    // Test 2: Number outside string should be colorized
    {
        CxString line("int x = 42;");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeNumbersWithExclusions(line, numberColor, reset, &regions);

        int escCount = countEsc(result.data());
        check(escCount == 2, "42 outside string: 2 ESC chars");
    }

    // Test 3: Number both inside and outside string
    {
        CxString line("int x = 10; printf(\"value: 20\");");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeNumbersWithExclusions(line, numberColor, reset, &regions);

        int escCount = countEsc(result.data());
        check(escCount == 2, "10 outside (2 ESC), 20 inside (0 ESC)");

        if (escCount != 2) {
            printf("  Line: %s\n", line.data());
            printf("  Result ESC count: %d (should be 2)\n", escCount);
        }
    }
}

//-----------------------------------------------------------------------------------------
// Test: Numbers NOT colorized inside comments
//-----------------------------------------------------------------------------------------
void testNumbersNotColorizedInComments() {
    printf("\n== Numbers NOT Colorized In Comments ==\n");

    CxString numberColor("\033[38;2;180;255;180m");
    CxString reset("\033[0m");

    // Test 1: Number in comment should NOT be colorized
    {
        CxString line("x++; // increment by 1");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeNumbersWithExclusions(line, numberColor, reset, &regions);

        int escCount = countEsc(result.data());
        check(escCount == 0, "1 in comment: 0 ESC chars");
    }

    // Test 2: Number before and after comment
    {
        CxString line("int x = 42; // max is 100");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeNumbersWithExclusions(line, numberColor, reset, &regions);

        int escCount = countEsc(result.data());
        check(escCount == 2, "42 outside (2 ESC), 100 in comment (0 ESC)");

        if (escCount != 2) {
            printf("  Line: %s\n", line.data());
            printf("  Result ESC count: %d (should be 2)\n", escCount);
        }
    }

    // Test 3: Hex number in comment
    {
        CxString line("// use 0xFF for mask");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeNumbersWithExclusions(line, numberColor, reset, &regions);

        int escCount = countEsc(result.data());
        check(escCount == 0, "0xFF in comment: 0 ESC chars");
    }
}

//-----------------------------------------------------------------------------------------
// Test: Combined exclusion scenarios
//-----------------------------------------------------------------------------------------
void testCombinedExclusionScenarios() {
    printf("\n== Combined Exclusion Scenarios ==\n");

    CxString keywordColor("\033[38;2;200;150;255m");
    CxString numberColor("\033[38;2;180;255;180m");
    CxString reset("\033[0m");

    // Test 1: Real-world line with string and comment
    {
        CxString line("if (x == 0) printf(\"if zero\"); // return 0");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        // Colorize keywords
        CxString result = simColorizeKeywordsWithExclusions(line, "if,return",
                                                             keywordColor, reset, &regions);

        // Only the first "if" should be colorized
        int escCount = countEsc(result.data());
        check(escCount == 2, "only first 'if' colorized: 2 ESC");

        printf("  Original: %s\n", line.data());
        printf("  Regions found: %d\n", regions.count);
        if (escCount != 2) {
            printf("  Result ESC count: %d (should be 2)\n", escCount);
        }
    }

    // Test 2: Format string with numbers
    {
        CxString line("snprintf(buf, 256, \"value=%d\", 42);");
        ColorRegions regions;
        simFindExclusionRegions(line, "//", &regions);

        CxString result = simColorizeNumbersWithExclusions(line, numberColor, reset, &regions);

        // 256 and 42 are outside string, should be colorized (4 ESC each)
        int escCount = countEsc(result.data());
        check(escCount == 4, "256 and 42 colorized: 4 ESC, %d inside ignored");

        if (escCount != 4) {
            printf("  Line: %s\n", line.data());
            printf("  Result ESC count: %d (should be 4)\n", escCount);
        }
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    printf("==============================================\n");
    printf("CxColorize Test Suite\n");
    printf("Testing escape sequence handling throughout\n");
    printf("==============================================\n");

    testCxStringConstructionWithEsc();
    testCxStringConcatenationWithEsc();
    testCxStringSubStringWithEsc();
    testCxStringAppendWithEsc();
    testCxColorTerminalString();
    testSimulatedColorization();
    testEdgeCases();
    testByteLevelVerification();
    testExactInjectSimulation();
    testFileOutputVerification();
    testTerminalOutputDemo();
    testDiagnoseEscFiltering();

    // Exclusion region tests (strings and comments)
    testExclusionRegionFinding();
    testKeywordsNotColorizedInStrings();
    testKeywordsNotColorizedInComments();
    testNumbersNotColorizedInStrings();
    testNumbersNotColorizedInComments();
    testCombinedExclusionScenarios();

    printf("\n==============================================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);
    printf("==============================================\n");

    return (testsFailed > 0) ? 1 : 0;
}
