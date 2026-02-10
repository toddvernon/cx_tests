//-------------------------------------------------------------------------------------------------
//
// utfstring_test.cpp
//
// Test program for CxUTFString
//
//-------------------------------------------------------------------------------------------------
//
// Compile with:
//   g++ -D _OSX_ -g -I../.. utfstring_test.cpp utfstring.cpp utfcharacter.cpp \
//       ../base/darwin_arm64/libcx_base.a -o utfstring_test
//
// Or on Linux:
//   g++ -D _LINUX_ -g -I../.. utfstring_test.cpp utfstring.cpp utfcharacter.cpp \
//       ../base/linux_x86_64/libcx_base.a -o utfstring_test
//
//-------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cx/base/utfstring.h>

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST_ASSERT(condition, testName) \
    do { \
        if (condition) { \
            testsPassed++; \
            printf("  PASS: %s\n", testName); \
        } else { \
            testsFailed++; \
            printf("  FAIL: %s (line %d)\n", testName, __LINE__); \
        } \
    } while(0)


//-------------------------------------------------------------------------------------------------
// Test basic construction and empty strings
//-------------------------------------------------------------------------------------------------
void test_basic_construction(void)
{
    printf("\n--- Testing basic construction ---\n");

    CxUTFString empty;
    TEST_ASSERT(empty.charCount() == 0, "Empty string has 0 characters");
    TEST_ASSERT(empty.displayWidth() == 0, "Empty string has 0 display width");
    TEST_ASSERT(empty.isEmpty() == 1, "Empty string isEmpty returns true");
    TEST_ASSERT(empty.isASCII() == 1, "Empty string isASCII returns true");
}


//-------------------------------------------------------------------------------------------------
// Test ASCII string parsing
//-------------------------------------------------------------------------------------------------
void test_ascii_parsing(void)
{
    printf("\n--- Testing ASCII parsing ---\n");

    CxUTFString s;
    s.fromBytes("Hello", 5, 4);

    TEST_ASSERT(s.charCount() == 5, "ASCII string has correct character count");
    TEST_ASSERT(s.displayWidth() == 5, "ASCII string has correct display width");
    TEST_ASSERT(s.isASCII() == 1, "ASCII string isASCII returns true");
    TEST_ASSERT(s.isEmpty() == 0, "Non-empty string isEmpty returns false");

    // Check individual characters
    TEST_ASSERT(s.at(0)->codepoint() == 'H', "First character is H");
    TEST_ASSERT(s.at(4)->codepoint() == 'o', "Last character is o");
    TEST_ASSERT(s.at(5) == 0, "Out of bounds returns NULL");

    // Check display columns
    TEST_ASSERT(s.displayColumnOfChar(0) == 0, "First char at column 0");
    TEST_ASSERT(s.displayColumnOfChar(4) == 4, "Last char at column 4");
    TEST_ASSERT(s.displayColumnOfChar(5) == 5, "End of string at column 5");
}


//-------------------------------------------------------------------------------------------------
// Test UTF-8 string parsing
//-------------------------------------------------------------------------------------------------
void test_utf8_parsing(void)
{
    printf("\n--- Testing UTF-8 parsing ---\n");

    // "Héllo" with precomposed é (U+00E9)
    CxUTFString s;
    s.fromBytes("H\xC3\xA9llo", 6, 4);

    TEST_ASSERT(s.charCount() == 5, "UTF-8 string has 5 characters");
    TEST_ASSERT(s.displayWidth() == 5, "UTF-8 string has display width 5");
    TEST_ASSERT(s.isASCII() == 0, "UTF-8 string isASCII returns false");

    TEST_ASSERT(s.at(0)->codepoint() == 'H', "First char is H");
    TEST_ASSERT(s.at(1)->codepoint() == 0x00E9, "Second char is é");
    TEST_ASSERT(s.at(2)->codepoint() == 'l', "Third char is l");
}


//-------------------------------------------------------------------------------------------------
// Test CJK (wide characters)
//-------------------------------------------------------------------------------------------------
void test_cjk_parsing(void)
{
    printf("\n--- Testing CJK parsing ---\n");

    // "A中B" - ASCII, CJK (double-width), ASCII
    CxUTFString s;
    s.fromBytes("A\xE4\xB8\xAD" "B", 5, 4);

    TEST_ASSERT(s.charCount() == 3, "CJK string has 3 characters");
    TEST_ASSERT(s.displayWidth() == 4, "CJK string has display width 4 (1+2+1)");

    TEST_ASSERT(s.displayColumnOfChar(0) == 0, "A at column 0");
    TEST_ASSERT(s.displayColumnOfChar(1) == 1, "中 at column 1");
    TEST_ASSERT(s.displayColumnOfChar(2) == 3, "B at column 3");

    TEST_ASSERT(s.displayWidthAt(0) == 1, "A has width 1");
    TEST_ASSERT(s.displayWidthAt(1) == 2, "中 has width 2");
    TEST_ASSERT(s.displayWidthAt(2) == 1, "B has width 1");
}


//-------------------------------------------------------------------------------------------------
// Test charIndexAtDisplayColumn
//-------------------------------------------------------------------------------------------------
void test_char_index_at_display_column(void)
{
    printf("\n--- Testing charIndexAtDisplayColumn ---\n");

    // "A中B" - columns: A=0, 中=1-2, B=3
    CxUTFString s;
    s.fromBytes("A\xE4\xB8\xAD" "B", 5, 4);

    TEST_ASSERT(s.charIndexAtDisplayColumn(0) == 0, "Column 0 -> char 0 (A)");
    TEST_ASSERT(s.charIndexAtDisplayColumn(1) == 1, "Column 1 -> char 1 (中)");
    TEST_ASSERT(s.charIndexAtDisplayColumn(2) == 1, "Column 2 -> char 1 (中) - middle of wide char");
    TEST_ASSERT(s.charIndexAtDisplayColumn(3) == 2, "Column 3 -> char 2 (B)");
    TEST_ASSERT(s.charIndexAtDisplayColumn(4) == 3, "Column 4 -> char 3 (end)");
    TEST_ASSERT(s.charIndexAtDisplayColumn(10) == 3, "Column 10 -> char 3 (end)");
}


//-------------------------------------------------------------------------------------------------
// Test combining characters
//-------------------------------------------------------------------------------------------------
void test_combining_characters(void)
{
    printf("\n--- Testing combining characters ---\n");

    // "e" + combining acute + "x" = 2 graphemes
    CxUTFString s;
    s.fromBytes("\x65\xCC\x81x", 4, 4);

    TEST_ASSERT(s.charCount() == 2, "String with combining has 2 characters");
    TEST_ASSERT(s.displayWidth() == 2, "String with combining has display width 2");

    TEST_ASSERT(s.at(0)->codepoint() == 'e', "First grapheme base is 'e'");
    TEST_ASSERT(s.at(0)->byteCount() == 3, "First grapheme has 3 bytes");
    TEST_ASSERT(s.at(1)->codepoint() == 'x', "Second grapheme is 'x'");
}


//-------------------------------------------------------------------------------------------------
// Test tabs
//-------------------------------------------------------------------------------------------------
void test_tabs(void)
{
    printf("\n--- Testing tabs ---\n");

    // "A\tB" with tabWidth 4
    // A at col 0, tab expands to columns 1-3, B at col 4
    CxUTFString s;
    s.fromBytes("A\tB", 3, 4);

    TEST_ASSERT(s.charCount() == 3, "String with tab has 3 characters");
    TEST_ASSERT(s.at(1)->isTab() == 1, "Second character is tab");
    TEST_ASSERT(s.at(1)->displayWidth() == 3, "Tab width is 3 (from col 1 to col 4)");
    TEST_ASSERT(s.displayColumnOfChar(2) == 4, "B at column 4");
    TEST_ASSERT(s.displayWidth() == 5, "Total display width is 5");

    // Tab at start: "\tX" with tabWidth 4
    CxUTFString s2;
    s2.fromBytes("\tX", 2, 4);
    TEST_ASSERT(s2.at(0)->displayWidth() == 4, "Tab at start has width 4");
    TEST_ASSERT(s2.displayColumnOfChar(1) == 4, "X at column 4");

    // Multiple tabs
    CxUTFString s3;
    s3.fromBytes("\t\t", 2, 4);
    TEST_ASSERT(s3.at(0)->displayWidth() == 4, "First tab has width 4");
    TEST_ASSERT(s3.at(1)->displayWidth() == 4, "Second tab has width 4");
    TEST_ASSERT(s3.displayWidth() == 8, "Two tabs have total width 8");
}


//-------------------------------------------------------------------------------------------------
// Test toBytes roundtrip
//-------------------------------------------------------------------------------------------------
void test_to_bytes_roundtrip(void)
{
    printf("\n--- Testing toBytes roundtrip ---\n");

    // ASCII roundtrip
    const char *ascii = "Hello World";
    CxUTFString s1;
    s1.fromBytes(ascii, strlen(ascii), 4);
    CxString back1 = s1.toBytes();
    TEST_ASSERT(strcmp(back1.data(), ascii) == 0, "ASCII roundtrip");

    // UTF-8 roundtrip
    const char *utf8 = "H\xC3\xA9llo \xE4\xB8\xAD";  // "Héllo 中"
    CxUTFString s2;
    s2.fromBytes(utf8, strlen(utf8), 4);
    CxString back2 = s2.toBytes();
    TEST_ASSERT(strcmp(back2.data(), utf8) == 0, "UTF-8 roundtrip");

    // Tab roundtrip (tabs stored as single \t)
    const char *tabs = "A\tB\tC";
    CxUTFString s3;
    s3.fromBytes(tabs, strlen(tabs), 4);
    CxString back3 = s3.toBytes();
    TEST_ASSERT(strcmp(back3.data(), tabs) == 0, "Tab roundtrip");
}


//-------------------------------------------------------------------------------------------------
// Test insert single character
//-------------------------------------------------------------------------------------------------
void test_insert_single(void)
{
    printf("\n--- Testing insert single character ---\n");

    CxUTFString s;
    s.fromBytes("AC", 2, 4);

    CxUTFCharacter b = CxUTFCharacter::fromASCII('B');
    s.insert(1, b);

    TEST_ASSERT(s.charCount() == 3, "After insert, 3 characters");
    TEST_ASSERT(s.at(0)->codepoint() == 'A', "First is A");
    TEST_ASSERT(s.at(1)->codepoint() == 'B', "Second is B (inserted)");
    TEST_ASSERT(s.at(2)->codepoint() == 'C', "Third is C");

    // Insert at start
    CxUTFCharacter z = CxUTFCharacter::fromASCII('Z');
    s.insert(0, z);
    TEST_ASSERT(s.charCount() == 4, "After insert at start, 4 characters");
    TEST_ASSERT(s.at(0)->codepoint() == 'Z', "First is Z");

    // Insert at end
    CxUTFCharacter y = CxUTFCharacter::fromASCII('Y');
    s.insert(4, y);
    TEST_ASSERT(s.charCount() == 5, "After insert at end, 5 characters");
    TEST_ASSERT(s.at(4)->codepoint() == 'Y', "Last is Y");
}


//-------------------------------------------------------------------------------------------------
// Test insert string
//-------------------------------------------------------------------------------------------------
void test_insert_string(void)
{
    printf("\n--- Testing insert string ---\n");

    CxUTFString s;
    s.fromBytes("AD", 2, 4);

    CxUTFString mid;
    mid.fromBytes("BC", 2, 4);

    s.insert(1, mid);

    TEST_ASSERT(s.charCount() == 4, "After insert string, 4 characters");
    TEST_ASSERT(s.at(0)->codepoint() == 'A', "First is A");
    TEST_ASSERT(s.at(1)->codepoint() == 'B', "Second is B");
    TEST_ASSERT(s.at(2)->codepoint() == 'C', "Third is C");
    TEST_ASSERT(s.at(3)->codepoint() == 'D', "Fourth is D");
}


//-------------------------------------------------------------------------------------------------
// Test remove
//-------------------------------------------------------------------------------------------------
void test_remove(void)
{
    printf("\n--- Testing remove ---\n");

    CxUTFString s;
    s.fromBytes("ABCDE", 5, 4);

    s.remove(1, 2);  // Remove "BC"

    TEST_ASSERT(s.charCount() == 3, "After remove, 3 characters");
    TEST_ASSERT(s.at(0)->codepoint() == 'A', "First is A");
    TEST_ASSERT(s.at(1)->codepoint() == 'D', "Second is D");
    TEST_ASSERT(s.at(2)->codepoint() == 'E', "Third is E");

    // Remove at start
    s.remove(0, 1);
    TEST_ASSERT(s.charCount() == 2, "After remove at start, 2 characters");
    TEST_ASSERT(s.at(0)->codepoint() == 'D', "First is D");

    // Remove at end
    s.remove(1, 1);
    TEST_ASSERT(s.charCount() == 1, "After remove at end, 1 character");
    TEST_ASSERT(s.at(0)->codepoint() == 'D', "Only D remains");

    // Remove all
    s.remove(0, 1);
    TEST_ASSERT(s.charCount() == 0, "After remove all, 0 characters");
}


//-------------------------------------------------------------------------------------------------
// Test append
//-------------------------------------------------------------------------------------------------
void test_append(void)
{
    printf("\n--- Testing append ---\n");

    CxUTFString s;

    // Append single characters
    s.append(CxUTFCharacter::fromASCII('A'));
    TEST_ASSERT(s.charCount() == 1, "After first append, 1 character");

    s.append(CxUTFCharacter::fromASCII('B'));
    TEST_ASSERT(s.charCount() == 2, "After second append, 2 characters");

    // Append string
    CxUTFString more;
    more.fromBytes("CD", 2, 4);
    s.append(more);

    TEST_ASSERT(s.charCount() == 4, "After append string, 4 characters");
    TEST_ASSERT(s.at(2)->codepoint() == 'C', "Third is C");
    TEST_ASSERT(s.at(3)->codepoint() == 'D', "Fourth is D");
}


//-------------------------------------------------------------------------------------------------
// Test substring
//-------------------------------------------------------------------------------------------------
void test_substring(void)
{
    printf("\n--- Testing substring ---\n");

    CxUTFString s;
    s.fromBytes("ABCDE", 5, 4);

    CxUTFString sub1 = s.subString(1, 3);  // "BCD"
    TEST_ASSERT(sub1.charCount() == 3, "Substring has 3 characters");
    TEST_ASSERT(sub1.at(0)->codepoint() == 'B', "First is B");
    TEST_ASSERT(sub1.at(2)->codepoint() == 'D', "Third is D");

    // Substring at start
    CxUTFString sub2 = s.subString(0, 2);
    TEST_ASSERT(sub2.charCount() == 2, "Substring from start has 2 chars");
    TEST_ASSERT(sub2.at(0)->codepoint() == 'A', "First is A");

    // Substring at end
    CxUTFString sub3 = s.subString(3, 2);
    TEST_ASSERT(sub3.charCount() == 2, "Substring at end has 2 chars");
    TEST_ASSERT(sub3.at(1)->codepoint() == 'E', "Last is E");

    // Substring beyond bounds (should clamp)
    CxUTFString sub4 = s.subString(3, 10);
    TEST_ASSERT(sub4.charCount() == 2, "Substring clamped to 2 chars");

    // Empty substring
    CxUTFString sub5 = s.subString(1, 0);
    TEST_ASSERT(sub5.charCount() == 0, "Zero-length substring is empty");
}


//-------------------------------------------------------------------------------------------------
// Test copy and assignment
//-------------------------------------------------------------------------------------------------
void test_copy_assignment(void)
{
    printf("\n--- Testing copy and assignment ---\n");

    CxUTFString orig;
    orig.fromBytes("Hello", 5, 4);

    // Copy constructor
    CxUTFString copy(orig);
    TEST_ASSERT(copy.charCount() == 5, "Copy has same char count");
    TEST_ASSERT(copy.at(0)->codepoint() == 'H', "Copy has same content");
    TEST_ASSERT(copy == orig, "Copy equals original");

    // Assignment
    CxUTFString assigned;
    assigned = orig;
    TEST_ASSERT(assigned.charCount() == 5, "Assigned has same char count");
    TEST_ASSERT(assigned == orig, "Assigned equals original");

    // Modify copy doesn't affect original
    copy.remove(0, 1);
    TEST_ASSERT(copy.charCount() == 4, "Copy modified");
    TEST_ASSERT(orig.charCount() == 5, "Original unchanged");
}


//-------------------------------------------------------------------------------------------------
// Test clear
//-------------------------------------------------------------------------------------------------
void test_clear(void)
{
    printf("\n--- Testing clear ---\n");

    CxUTFString s;
    s.fromBytes("Hello", 5, 4);

    TEST_ASSERT(s.charCount() == 5, "Before clear, 5 characters");

    s.clear();
    TEST_ASSERT(s.charCount() == 0, "After clear, 0 characters");
    TEST_ASSERT(s.isEmpty() == 1, "After clear, isEmpty returns true");

    // Can still append after clear
    s.append(CxUTFCharacter::fromASCII('X'));
    TEST_ASSERT(s.charCount() == 1, "After clear and append, 1 character");
}


//-------------------------------------------------------------------------------------------------
// Test recalculateTabWidths
//-------------------------------------------------------------------------------------------------
void test_recalculate_tab_widths(void)
{
    printf("\n--- Testing recalculateTabWidths ---\n");

    CxUTFString s;
    s.fromBytes("A\tB", 3, 4);

    // Initial: A at 0, tab width 3, B at 4
    TEST_ASSERT(s.at(1)->displayWidth() == 3, "Initial tab width is 3");

    // Insert character before tab
    s.insert(0, CxUTFCharacter::fromASCII('X'));
    // Now: X A \t B - tab at position 2, but display column now 2
    // Need to recalculate

    s.recalculateTabWidths(4);
    // X(col 0) A(col 1) \t(col 2, should expand to col 4) B(col 4)
    TEST_ASSERT(s.at(2)->displayWidth() == 2, "Tab width after insert is 2");
    TEST_ASSERT(s.displayColumnOfChar(3) == 4, "B at column 4");
}


//-------------------------------------------------------------------------------------------------
// Test equality
//-------------------------------------------------------------------------------------------------
void test_equality(void)
{
    printf("\n--- Testing equality ---\n");

    CxUTFString s1;
    s1.fromBytes("Hello", 5, 4);

    CxUTFString s2;
    s2.fromBytes("Hello", 5, 4);

    CxUTFString s3;
    s3.fromBytes("World", 5, 4);

    TEST_ASSERT(s1 == s2, "Same strings are equal");
    TEST_ASSERT(!(s1 == s3), "Different strings are not equal");
    TEST_ASSERT(s1 != s3, "Different strings != returns true");
    TEST_ASSERT(!(s1 != s2), "Same strings != returns false");
}


//-------------------------------------------------------------------------------------------------
// Test box drawing characters (the original use case!)
//-------------------------------------------------------------------------------------------------
void test_box_drawing(void)
{
    printf("\n--- Testing box drawing characters ---\n");

    // Table row: "│cell│"
    const char *input = "\xE2\x94\x82" "cell" "\xE2\x94\x82";
    CxUTFString s;
    s.fromBytes(input, strlen(input), 4);

    TEST_ASSERT(s.charCount() == 6, "Box drawing string has 6 characters");
    TEST_ASSERT(s.displayWidth() == 6, "Box drawing string has display width 6");
    TEST_ASSERT(s.at(0)->codepoint() == 0x2502, "First char is │");
    TEST_ASSERT(s.at(5)->codepoint() == 0x2502, "Last char is │");

    // Verify roundtrip
    CxString back = s.toBytes();
    TEST_ASSERT(strcmp(back.data(), input) == 0, "Box drawing roundtrip");
}


//-------------------------------------------------------------------------------------------------
// Test emoji
//-------------------------------------------------------------------------------------------------
void test_emoji(void)
{
    printf("\n--- Testing emoji ---\n");

    // "Hi👋" - ASCII + emoji
    const char *input = "Hi\xF0\x9F\x91\x8B";
    CxUTFString s;
    s.fromBytes(input, strlen(input), 4);

    TEST_ASSERT(s.charCount() == 3, "Emoji string has 3 characters");
    TEST_ASSERT(s.displayWidth() == 4, "Emoji string has display width 4 (1+1+2)");
    TEST_ASSERT(s.at(2)->codepoint() == 0x1F44B, "Third char is waving hand");

    // Emoji with skin tone
    const char *skin = "\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBD";  // 👋🏽
    CxUTFString s2;
    s2.fromBytes(skin, strlen(skin), 4);

    TEST_ASSERT(s2.charCount() == 1, "Emoji + skin tone is 1 character");
    TEST_ASSERT(s2.displayWidth() == 2, "Emoji + skin tone has display width 2");
}


//-------------------------------------------------------------------------------------------------
// Test fromCxString
//-------------------------------------------------------------------------------------------------
void test_from_cxstring(void)
{
    printf("\n--- Testing fromCxString ---\n");

    CxString src("Hello");
    CxUTFString s;
    s.fromCxString(src, 4);

    TEST_ASSERT(s.charCount() == 5, "fromCxString parses correctly");
    TEST_ASSERT(s.at(0)->codepoint() == 'H', "First char is H");
}


//-------------------------------------------------------------------------------------------------
// Test fromUTF8Bytes
//-------------------------------------------------------------------------------------------------
void test_from_utf8_bytes(void)
{
    printf("\n--- Testing fromUTF8Bytes ---\n");

    //---------------------------------------------------------------------------------------------
    // Tabs are stored as width-1 (no tab-stop expansion)
    //---------------------------------------------------------------------------------------------
    {
        CxUTFString s;
        s.fromUTF8Bytes("A\tB", 3);

        TEST_ASSERT(s.charCount() == 3, "fromUTF8Bytes: A\\tB has 3 characters");
        TEST_ASSERT(s.at(0)->codepoint() == 'A', "fromUTF8Bytes: first char is A");
        TEST_ASSERT(s.at(1)->isTab() == 1, "fromUTF8Bytes: second char is tab");
        TEST_ASSERT(s.at(1)->displayWidth() == 1, "fromUTF8Bytes: tab has width 1 (not expanded)");
        TEST_ASSERT(s.at(2)->codepoint() == 'B', "fromUTF8Bytes: third char is B");
        TEST_ASSERT(s.displayWidth() == 3, "fromUTF8Bytes: total width is 3 (1+1+1)");

        // Compare with fromBytes which expands tabs
        CxUTFString s2;
        s2.fromBytes("A\tB", 3, 4);
        TEST_ASSERT(s2.at(1)->displayWidth() == 3, "fromBytes: tab has width 3 (expanded)");
        TEST_ASSERT(s2.displayWidth() == 5, "fromBytes: total width is 5 (1+3+1)");
    }

    //---------------------------------------------------------------------------------------------
    // Tab at start has width 1
    //---------------------------------------------------------------------------------------------
    {
        CxUTFString s;
        s.fromUTF8Bytes("\tX", 2);

        TEST_ASSERT(s.at(0)->isTab() == 1, "fromUTF8Bytes: tab at start is tab");
        TEST_ASSERT(s.at(0)->displayWidth() == 1, "fromUTF8Bytes: tab at start has width 1");
    }

    //---------------------------------------------------------------------------------------------
    // Multiple consecutive tabs - each has width 1
    //---------------------------------------------------------------------------------------------
    {
        CxUTFString s;
        s.fromUTF8Bytes("\t\t\t", 3);

        TEST_ASSERT(s.charCount() == 3, "fromUTF8Bytes: 3 tabs has 3 characters");
        TEST_ASSERT(s.at(0)->displayWidth() == 1, "fromUTF8Bytes: first tab width 1");
        TEST_ASSERT(s.at(1)->displayWidth() == 1, "fromUTF8Bytes: second tab width 1");
        TEST_ASSERT(s.at(2)->displayWidth() == 1, "fromUTF8Bytes: third tab width 1");
        TEST_ASSERT(s.displayWidth() == 3, "fromUTF8Bytes: 3 tabs total width 3");
    }

    //---------------------------------------------------------------------------------------------
    // Newlines preserved as characters
    //---------------------------------------------------------------------------------------------
    {
        CxUTFString s;
        s.fromUTF8Bytes("AB\nCD", 5);

        TEST_ASSERT(s.charCount() == 5, "fromUTF8Bytes: AB\\nCD has 5 characters");
        TEST_ASSERT(s.at(0)->codepoint() == 'A', "fromUTF8Bytes: first is A");
        TEST_ASSERT(s.at(1)->codepoint() == 'B', "fromUTF8Bytes: second is B");
        TEST_ASSERT(s.at(2)->codepoint() == '\n', "fromUTF8Bytes: third is newline");
        TEST_ASSERT(s.at(3)->codepoint() == 'C', "fromUTF8Bytes: fourth is C");
        TEST_ASSERT(s.at(4)->codepoint() == 'D', "fromUTF8Bytes: fifth is D");
    }

    //---------------------------------------------------------------------------------------------
    // Carriage return preserved
    //---------------------------------------------------------------------------------------------
    {
        CxUTFString s;
        s.fromUTF8Bytes("A\rB", 3);

        TEST_ASSERT(s.charCount() == 3, "fromUTF8Bytes: A\\rB has 3 characters");
        TEST_ASSERT(s.at(1)->codepoint() == '\r', "fromUTF8Bytes: middle is CR");
    }

    //---------------------------------------------------------------------------------------------
    // CRLF as two separate characters
    //---------------------------------------------------------------------------------------------
    {
        CxUTFString s;
        s.fromUTF8Bytes("A\r\nB", 4);

        TEST_ASSERT(s.charCount() == 4, "fromUTF8Bytes: A\\r\\nB has 4 characters");
        TEST_ASSERT(s.at(1)->codepoint() == '\r', "fromUTF8Bytes: second is CR");
        TEST_ASSERT(s.at(2)->codepoint() == '\n', "fromUTF8Bytes: third is LF");
        TEST_ASSERT(s.at(3)->codepoint() == 'B', "fromUTF8Bytes: fourth is B");
    }

    //---------------------------------------------------------------------------------------------
    // UTF-8 multi-byte characters
    //---------------------------------------------------------------------------------------------
    {
        // "Héllo" with precomposed é (U+00E9)
        CxUTFString s;
        s.fromUTF8Bytes("H\xC3\xA9llo", 6);

        TEST_ASSERT(s.charCount() == 5, "fromUTF8Bytes: Héllo has 5 characters");
        TEST_ASSERT(s.at(1)->codepoint() == 0x00E9, "fromUTF8Bytes: second is é");
        TEST_ASSERT(s.displayWidth() == 5, "fromUTF8Bytes: Héllo display width 5");
    }

    //---------------------------------------------------------------------------------------------
    // Box-drawing characters
    //---------------------------------------------------------------------------------------------
    {
        // "│cell│" - box drawing (3 bytes each) + ASCII
        const char *input = "\xE2\x94\x82" "cell" "\xE2\x94\x82";
        CxUTFString s;
        s.fromUTF8Bytes(input, 10);

        TEST_ASSERT(s.charCount() == 6, "fromUTF8Bytes: box drawing has 6 characters");
        TEST_ASSERT(s.at(0)->codepoint() == 0x2502, "fromUTF8Bytes: first is │");
        TEST_ASSERT(s.at(5)->codepoint() == 0x2502, "fromUTF8Bytes: last is │");
        TEST_ASSERT(s.displayWidth() == 6, "fromUTF8Bytes: box drawing display width 6");
    }

    //---------------------------------------------------------------------------------------------
    // Mixed content: tabs + box-drawing + newlines
    //---------------------------------------------------------------------------------------------
    {
        const char *input = "A\t\xE2\x94\x82\nB";  // A, tab, │, newline, B
        CxUTFString s;
        s.fromUTF8Bytes(input, 7);

        TEST_ASSERT(s.charCount() == 5, "fromUTF8Bytes: mixed content has 5 characters");
        TEST_ASSERT(s.at(0)->codepoint() == 'A', "fromUTF8Bytes: mixed first is A");
        TEST_ASSERT(s.at(1)->isTab() == 1, "fromUTF8Bytes: mixed second is tab");
        TEST_ASSERT(s.at(1)->displayWidth() == 1, "fromUTF8Bytes: mixed tab width 1");
        TEST_ASSERT(s.at(2)->codepoint() == 0x2502, "fromUTF8Bytes: mixed third is │");
        TEST_ASSERT(s.at(3)->codepoint() == '\n', "fromUTF8Bytes: mixed fourth is newline");
        TEST_ASSERT(s.at(4)->codepoint() == 'B', "fromUTF8Bytes: mixed fifth is B");
    }

    //---------------------------------------------------------------------------------------------
    // Edge cases: NULL pointer, empty string, zero length
    //---------------------------------------------------------------------------------------------
    {
        CxUTFString s1;
        s1.fromUTF8Bytes((const char *)0, 0);
        TEST_ASSERT(s1.charCount() == 0, "fromUTF8Bytes: NULL creates empty string");

        CxUTFString s2;
        s2.fromUTF8Bytes("", 0);
        TEST_ASSERT(s2.charCount() == 0, "fromUTF8Bytes: empty creates empty string");

        CxUTFString s3;
        s3.fromUTF8Bytes("Hello", 0);
        TEST_ASSERT(s3.charCount() == 0, "fromUTF8Bytes: zero length creates empty string");
    }

    //---------------------------------------------------------------------------------------------
    // Roundtrip: fromUTF8Bytes -> toBytes preserves content
    //---------------------------------------------------------------------------------------------
    {
        // ASCII roundtrip
        CxUTFString s1;
        s1.fromUTF8Bytes("Hello", 5);
        CxString back1 = s1.toBytes();
        TEST_ASSERT(strcmp(back1.data(), "Hello") == 0, "fromUTF8Bytes: ASCII roundtrip");

        // UTF-8 roundtrip
        const char *utf8 = "H\xC3\xA9llo \xE4\xB8\xAD";  // "Héllo 中"
        CxUTFString s2;
        s2.fromUTF8Bytes(utf8, strlen(utf8));
        CxString back2 = s2.toBytes();
        TEST_ASSERT(strcmp(back2.data(), utf8) == 0, "fromUTF8Bytes: UTF-8 roundtrip");

        // Tab roundtrip (tabs come back as \t)
        CxUTFString s3;
        s3.fromUTF8Bytes("A\tB\tC", 5);
        CxString back3 = s3.toBytes();
        TEST_ASSERT(strcmp(back3.data(), "A\tB\tC") == 0, "fromUTF8Bytes: tab roundtrip");

        // Newline roundtrip
        CxUTFString s4;
        s4.fromUTF8Bytes("Line1\nLine2", 11);
        CxString back4 = s4.toBytes();
        TEST_ASSERT(strcmp(back4.data(), "Line1\nLine2") == 0, "fromUTF8Bytes: newline roundtrip");
    }

    //---------------------------------------------------------------------------------------------
    // Invalid UTF-8: incomplete sequences should stop parsing (not crash)
    //---------------------------------------------------------------------------------------------
    {
        // Incomplete 2-byte sequence at end
        CxUTFString s1;
        s1.fromUTF8Bytes("A\xC3", 2);
        // Should parse 'A' then stop at incomplete sequence
        TEST_ASSERT(s1.charCount() >= 1, "fromUTF8Bytes: incomplete 2-byte - at least A parsed");
        TEST_ASSERT(s1.at(0)->codepoint() == 'A', "fromUTF8Bytes: incomplete 2-byte - first is A");

        // Incomplete 3-byte sequence at end
        CxUTFString s2;
        s2.fromUTF8Bytes("AB\xE4\xB8", 4);
        TEST_ASSERT(s2.charCount() >= 2, "fromUTF8Bytes: incomplete 3-byte - at least AB parsed");
        TEST_ASSERT(s2.at(0)->codepoint() == 'A', "fromUTF8Bytes: incomplete 3-byte - first is A");
        TEST_ASSERT(s2.at(1)->codepoint() == 'B', "fromUTF8Bytes: incomplete 3-byte - second is B");
    }

    //---------------------------------------------------------------------------------------------
    // CJK characters (double-width)
    //---------------------------------------------------------------------------------------------
    {
        // "中文" = two CJK chars, each 3 bytes, display width 2
        const char *input = "\xE4\xB8\xAD\xE6\x96\x87";
        CxUTFString s;
        s.fromUTF8Bytes(input, 6);

        TEST_ASSERT(s.charCount() == 2, "fromUTF8Bytes: CJK has 2 characters");
        TEST_ASSERT(s.displayWidth() == 4, "fromUTF8Bytes: CJK display width 4 (2+2)");
        TEST_ASSERT(s.at(0)->codepoint() == 0x4E2D, "fromUTF8Bytes: first is 中");
        TEST_ASSERT(s.at(1)->codepoint() == 0x6587, "fromUTF8Bytes: second is 文");
    }

    //---------------------------------------------------------------------------------------------
    // Reuse: calling fromUTF8Bytes on a non-empty string clears first
    //---------------------------------------------------------------------------------------------
    {
        CxUTFString s;
        s.fromUTF8Bytes("Hello", 5);
        TEST_ASSERT(s.charCount() == 5, "fromUTF8Bytes: initial has 5 chars");

        s.fromUTF8Bytes("AB", 2);
        TEST_ASSERT(s.charCount() == 2, "fromUTF8Bytes: reuse clears and parses new content");
        TEST_ASSERT(s.at(0)->codepoint() == 'A', "fromUTF8Bytes: reuse first is A");
    }
}


//-------------------------------------------------------------------------------------------------
// Test edge cases
//-------------------------------------------------------------------------------------------------
void test_edge_cases(void)
{
    printf("\n--- Testing edge cases ---\n");

    // Empty/null input
    CxUTFString empty;
    empty.fromBytes("", 0, 4);
    TEST_ASSERT(empty.charCount() == 0, "Empty bytes creates empty string");

    CxUTFString null;
    null.fromBytes((const char*)0, 0, 4);
    TEST_ASSERT(null.charCount() == 0, "Null bytes creates empty string");

    // Insert into empty string
    CxUTFString s;
    s.insert(0, CxUTFCharacter::fromASCII('A'));
    TEST_ASSERT(s.charCount() == 1, "Insert into empty works");

    // Remove from empty (should not crash)
    CxUTFString empty2;
    empty2.remove(0, 1);
    TEST_ASSERT(empty2.charCount() == 0, "Remove from empty is safe");

    // Substring of empty
    CxUTFString sub = empty.subString(0, 5);
    TEST_ASSERT(sub.charCount() == 0, "Substring of empty is empty");
}


//-------------------------------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    printf("=== CxUTFString Test Suite ===\n");

    test_basic_construction();
    test_ascii_parsing();
    test_utf8_parsing();
    test_cjk_parsing();
    test_char_index_at_display_column();
    test_combining_characters();
    test_tabs();
    test_to_bytes_roundtrip();
    test_insert_single();
    test_insert_string();
    test_remove();
    test_append();
    test_substring();
    test_copy_assignment();
    test_clear();
    test_recalculate_tab_widths();
    test_equality();
    test_box_drawing();
    test_emoji();
    test_from_cxstring();
    test_from_utf8_bytes();
    test_edge_cases();

    printf("\n=================================\n");
    printf("Tests passed: %d\n", testsPassed);
    printf("Tests failed: %d\n", testsFailed);
    printf("=================================\n");

    return testsFailed > 0 ? 1 : 0;
}
