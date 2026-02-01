//-------------------------------------------------------------------------------------------------
//
// utfcharacter_test.cpp
//
// Test program for CxUTFCharacter
//
//-------------------------------------------------------------------------------------------------
//
// Compile with:
//   g++ -D _OSX_ -g -I../.. utfcharacter_test.cpp utfcharacter.cpp -o utfcharacter_test
//
// Or on Linux:
//   g++ -D _LINUX_ -g -I../.. utfcharacter_test.cpp utfcharacter.cpp -o utfcharacter_test
//
//-------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cx/editbuffer/utfcharacter.h>

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
// Test ASCII characters
//-------------------------------------------------------------------------------------------------
void test_ascii(void)
{
    printf("\n--- Testing ASCII characters ---\n");

    // Test single ASCII character
    CxUTFCharacter ch;
    const char *input = "A";
    int consumed = ch.fromUTF8(input);

    TEST_ASSERT(consumed == 1, "ASCII 'A' consumes 1 byte");
    TEST_ASSERT(ch.byteCount() == 1, "ASCII 'A' has 1 byte");
    TEST_ASSERT(ch.displayWidth() == 1, "ASCII 'A' has display width 1");
    TEST_ASSERT(ch.isASCII() == 1, "ASCII 'A' isASCII returns true");
    TEST_ASSERT(ch.codepoint() == 'A', "ASCII 'A' codepoint is 0x41");

    // Test fromASCII factory
    CxUTFCharacter ch2 = CxUTFCharacter::fromASCII('Z');
    TEST_ASSERT(ch2.codepoint() == 'Z', "fromASCII('Z') creates correct character");
    TEST_ASSERT(ch2.displayWidth() == 1, "fromASCII has display width 1");

    // Test space
    CxUTFCharacter space;
    space.fromUTF8(" ");
    TEST_ASSERT(space.codepoint() == ' ', "Space character");
    TEST_ASSERT(space.displayWidth() == 1, "Space has display width 1");
}


//-------------------------------------------------------------------------------------------------
// Test 2-byte UTF-8 characters
//-------------------------------------------------------------------------------------------------
void test_2byte_utf8(void)
{
    printf("\n--- Testing 2-byte UTF-8 ---\n");

    // U+00E9 LATIN SMALL LETTER E WITH ACUTE (é)
    // UTF-8: C3 A9
    CxUTFCharacter ch;
    const char *input = "\xC3\xA9";
    int consumed = ch.fromUTF8(input);

    TEST_ASSERT(consumed == 2, "U+00E9 (é) consumes 2 bytes");
    TEST_ASSERT(ch.byteCount() == 2, "U+00E9 (é) has 2 bytes");
    TEST_ASSERT(ch.displayWidth() == 1, "U+00E9 (é) has display width 1");
    TEST_ASSERT(ch.isASCII() == 0, "U+00E9 (é) is not ASCII");
    TEST_ASSERT(ch.codepoint() == 0x00E9, "U+00E9 (é) codepoint is correct");

    // U+00F1 LATIN SMALL LETTER N WITH TILDE (ñ)
    // UTF-8: C3 B1
    CxUTFCharacter ch2;
    ch2.fromUTF8("\xC3\xB1");
    TEST_ASSERT(ch2.codepoint() == 0x00F1, "U+00F1 (ñ) codepoint is correct");

    // U+00FC LATIN SMALL LETTER U WITH DIAERESIS (ü)
    // UTF-8: C3 BC
    CxUTFCharacter ch3;
    ch3.fromUTF8("\xC3\xBC");
    TEST_ASSERT(ch3.codepoint() == 0x00FC, "U+00FC (ü) codepoint is correct");
}


//-------------------------------------------------------------------------------------------------
// Test 3-byte UTF-8 characters
//-------------------------------------------------------------------------------------------------
void test_3byte_utf8(void)
{
    printf("\n--- Testing 3-byte UTF-8 ---\n");

    // U+4E2D CHINESE CHARACTER (中)
    // UTF-8: E4 B8 AD
    CxUTFCharacter ch;
    const char *input = "\xE4\xB8\xAD";
    int consumed = ch.fromUTF8(input);

    TEST_ASSERT(consumed == 3, "U+4E2D (中) consumes 3 bytes");
    TEST_ASSERT(ch.byteCount() == 3, "U+4E2D (中) has 3 bytes");
    TEST_ASSERT(ch.displayWidth() == 2, "U+4E2D (中) has display width 2 (CJK)");
    TEST_ASSERT(ch.codepoint() == 0x4E2D, "U+4E2D (中) codepoint is correct");

    // U+2500 BOX DRAWINGS LIGHT HORIZONTAL (─)
    // UTF-8: E2 94 80
    CxUTFCharacter box;
    box.fromUTF8("\xE2\x94\x80");
    TEST_ASSERT(box.byteCount() == 3, "U+2500 (─) has 3 bytes");
    TEST_ASSERT(box.displayWidth() == 1, "U+2500 (─) has display width 1");
    TEST_ASSERT(box.codepoint() == 0x2500, "U+2500 (─) codepoint is correct");

    // U+2502 BOX DRAWINGS LIGHT VERTICAL (│)
    // UTF-8: E2 94 82
    CxUTFCharacter vbox;
    vbox.fromUTF8("\xE2\x94\x82");
    TEST_ASSERT(vbox.codepoint() == 0x2502, "U+2502 (│) codepoint is correct");

    // U+3042 HIRAGANA LETTER A (あ)
    // UTF-8: E3 81 82
    CxUTFCharacter hiragana;
    hiragana.fromUTF8("\xE3\x81\x82");
    TEST_ASSERT(hiragana.codepoint() == 0x3042, "U+3042 (あ) codepoint is correct");
    TEST_ASSERT(hiragana.displayWidth() == 2, "U+3042 (あ) has display width 2 (Japanese)");
}


//-------------------------------------------------------------------------------------------------
// Test 4-byte UTF-8 characters (emoji, etc.)
//-------------------------------------------------------------------------------------------------
void test_4byte_utf8(void)
{
    printf("\n--- Testing 4-byte UTF-8 ---\n");

    // U+1F44B WAVING HAND SIGN (👋)
    // UTF-8: F0 9F 91 8B
    CxUTFCharacter ch;
    const char *input = "\xF0\x9F\x91\x8B";
    int consumed = ch.fromUTF8(input);

    TEST_ASSERT(consumed == 4, "U+1F44B (👋) consumes 4 bytes");
    TEST_ASSERT(ch.byteCount() == 4, "U+1F44B (👋) has 4 bytes");
    TEST_ASSERT(ch.displayWidth() == 2, "U+1F44B (👋) has display width 2 (emoji)");
    TEST_ASSERT(ch.codepoint() == 0x1F44B, "U+1F44B (👋) codepoint is correct");

    // U+1F600 GRINNING FACE (😀)
    // UTF-8: F0 9F 98 80
    CxUTFCharacter grin;
    grin.fromUTF8("\xF0\x9F\x98\x80");
    TEST_ASSERT(grin.codepoint() == 0x1F600, "U+1F600 (😀) codepoint is correct");
    TEST_ASSERT(grin.displayWidth() == 2, "U+1F600 (😀) has display width 2");
}


//-------------------------------------------------------------------------------------------------
// Test combining characters
//-------------------------------------------------------------------------------------------------
void test_combining_characters(void)
{
    printf("\n--- Testing combining characters ---\n");

    // e + combining acute accent = é (two codepoints, one grapheme)
    // U+0065 U+0301
    // UTF-8: 65 CC 81
    CxUTFCharacter ch;
    const char *input = "\x65\xCC\x81";
    int consumed = ch.fromUTF8(input);

    TEST_ASSERT(consumed == 3, "e + combining acute consumes 3 bytes");
    TEST_ASSERT(ch.byteCount() == 3, "e + combining acute has 3 bytes total");
    TEST_ASSERT(ch.displayWidth() == 1, "e + combining acute has display width 1");
    TEST_ASSERT(ch.codepoint() == 'e', "Base codepoint is 'e'");

    // a + combining ring above = å
    // U+0061 U+030A
    // UTF-8: 61 CC 8A
    CxUTFCharacter ch2;
    int consumed2 = ch2.fromUTF8("\x61\xCC\x8A");
    TEST_ASSERT(consumed2 == 3, "a + combining ring consumes 3 bytes");
    TEST_ASSERT(ch2.displayWidth() == 1, "a + combining ring has display width 1");

    // o + combining diaeresis + combining macron (multiple combiners)
    // U+006F U+0308 U+0304
    // UTF-8: 6F CC 88 CC 84
    CxUTFCharacter ch3;
    int consumed3 = ch3.fromUTF8("\x6F\xCC\x88\xCC\x84");
    TEST_ASSERT(consumed3 == 5, "o + two combiners consumes 5 bytes");
    TEST_ASSERT(ch3.byteCount() == 5, "o + two combiners has 5 bytes");
    TEST_ASSERT(ch3.displayWidth() == 1, "o + two combiners has display width 1");
}


//-------------------------------------------------------------------------------------------------
// Test emoji with skin tone modifiers
//-------------------------------------------------------------------------------------------------
void test_emoji_skin_tone(void)
{
    printf("\n--- Testing emoji with skin tone ---\n");

    // 👋🏽 = Waving hand + medium skin tone
    // U+1F44B U+1F3FD
    // UTF-8: F0 9F 91 8B F0 9F 8F BD
    CxUTFCharacter ch;
    const char *input = "\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBD";
    int consumed = ch.fromUTF8(input);

    TEST_ASSERT(consumed == 8, "Waving hand + skin tone consumes 8 bytes");
    TEST_ASSERT(ch.byteCount() == 8, "Waving hand + skin tone has 8 bytes");
    TEST_ASSERT(ch.displayWidth() == 2, "Emoji with skin tone has display width 2");
    TEST_ASSERT(ch.codepoint() == 0x1F44B, "Base codepoint is waving hand");
}


//-------------------------------------------------------------------------------------------------
// Test tabs
//-------------------------------------------------------------------------------------------------
void test_tabs(void)
{
    printf("\n--- Testing tabs ---\n");

    CxUTFCharacter tab4 = CxUTFCharacter::makeTab(4);
    TEST_ASSERT(tab4.isTab() == 1, "makeTab creates a tab");
    TEST_ASSERT(tab4.displayWidth() == 4, "Tab with width 4");
    TEST_ASSERT(tab4.bytes()[0] == '\t', "Tab byte is \\t");

    CxUTFCharacter tab8 = CxUTFCharacter::makeTab(8);
    TEST_ASSERT(tab8.displayWidth() == 8, "Tab with width 8");

    CxUTFCharacter tab1 = CxUTFCharacter::makeTab(1);
    TEST_ASSERT(tab1.displayWidth() == 1, "Tab with width 1");
}


//-------------------------------------------------------------------------------------------------
// Test fromCodepoint
//-------------------------------------------------------------------------------------------------
void test_from_codepoint(void)
{
    printf("\n--- Testing fromCodepoint ---\n");

    // ASCII
    CxUTFCharacter ascii = CxUTFCharacter::fromCodepoint('A');
    TEST_ASSERT(ascii.codepoint() == 'A', "fromCodepoint ASCII");
    TEST_ASSERT(ascii.byteCount() == 1, "ASCII is 1 byte");

    // 2-byte
    CxUTFCharacter latin = CxUTFCharacter::fromCodepoint(0x00E9);
    TEST_ASSERT(latin.codepoint() == 0x00E9, "fromCodepoint 2-byte");
    TEST_ASSERT(latin.byteCount() == 2, "2-byte char has 2 bytes");

    // 3-byte
    CxUTFCharacter cjk = CxUTFCharacter::fromCodepoint(0x4E2D);
    TEST_ASSERT(cjk.codepoint() == 0x4E2D, "fromCodepoint 3-byte CJK");
    TEST_ASSERT(cjk.byteCount() == 3, "CJK char has 3 bytes");
    TEST_ASSERT(cjk.displayWidth() == 2, "CJK has display width 2");

    // 4-byte
    CxUTFCharacter emoji = CxUTFCharacter::fromCodepoint(0x1F44B);
    TEST_ASSERT(emoji.codepoint() == 0x1F44B, "fromCodepoint 4-byte emoji");
    TEST_ASSERT(emoji.byteCount() == 4, "Emoji has 4 bytes");
}


//-------------------------------------------------------------------------------------------------
// Test equality operators
//-------------------------------------------------------------------------------------------------
void test_equality(void)
{
    printf("\n--- Testing equality operators ---\n");

    CxUTFCharacter a1 = CxUTFCharacter::fromASCII('A');
    CxUTFCharacter a2 = CxUTFCharacter::fromASCII('A');
    CxUTFCharacter b = CxUTFCharacter::fromASCII('B');

    TEST_ASSERT(a1 == a2, "Same character equals");
    TEST_ASSERT(!(a1 == b), "Different characters not equal");
    TEST_ASSERT(a1 != b, "Different characters != true");
    TEST_ASSERT(!(a1 != a2), "Same characters != false");

    // UTF-8 equality
    CxUTFCharacter u1;
    CxUTFCharacter u2;
    u1.fromUTF8("\xC3\xA9");
    u2.fromUTF8("\xC3\xA9");
    TEST_ASSERT(u1 == u2, "Same UTF-8 character equals");
}


//-------------------------------------------------------------------------------------------------
// Test copy constructor and assignment
//-------------------------------------------------------------------------------------------------
void test_copy_assignment(void)
{
    printf("\n--- Testing copy and assignment ---\n");

    CxUTFCharacter orig;
    orig.fromUTF8("\xE4\xB8\xAD");  // 中

    // Copy constructor
    CxUTFCharacter copy(orig);
    TEST_ASSERT(copy.codepoint() == orig.codepoint(), "Copy constructor preserves codepoint");
    TEST_ASSERT(copy.byteCount() == orig.byteCount(), "Copy constructor preserves byteCount");
    TEST_ASSERT(copy.displayWidth() == orig.displayWidth(), "Copy constructor preserves displayWidth");

    // Assignment
    CxUTFCharacter assigned;
    assigned = orig;
    TEST_ASSERT(assigned.codepoint() == orig.codepoint(), "Assignment preserves codepoint");
    TEST_ASSERT(assigned == orig, "Assignment creates equal character");
}


//-------------------------------------------------------------------------------------------------
// Test UTF-8 utility functions
//-------------------------------------------------------------------------------------------------
void test_utility_functions(void)
{
    printf("\n--- Testing utility functions ---\n");

    // cxUTF8LeadByteLength
    TEST_ASSERT(cxUTF8LeadByteLength(0x41) == 1, "ASCII lead byte length is 1");
    TEST_ASSERT(cxUTF8LeadByteLength(0xC3) == 2, "2-byte lead byte length");
    TEST_ASSERT(cxUTF8LeadByteLength(0xE4) == 3, "3-byte lead byte length");
    TEST_ASSERT(cxUTF8LeadByteLength(0xF0) == 4, "4-byte lead byte length");

    // cxUTF8Encode and decode roundtrip
    unsigned char buf[4];
    int len;

    len = cxUTF8Encode(0x41, buf);
    TEST_ASSERT(len == 1 && buf[0] == 0x41, "Encode ASCII");

    len = cxUTF8Encode(0x00E9, buf);
    TEST_ASSERT(len == 2, "Encode 2-byte returns 2");
    TEST_ASSERT(cxUTF8Decode(buf, len) == 0x00E9, "Decode 2-byte roundtrip");

    len = cxUTF8Encode(0x4E2D, buf);
    TEST_ASSERT(len == 3, "Encode 3-byte returns 3");
    TEST_ASSERT(cxUTF8Decode(buf, len) == 0x4E2D, "Decode 3-byte roundtrip");

    len = cxUTF8Encode(0x1F44B, buf);
    TEST_ASSERT(len == 4, "Encode 4-byte returns 4");
    TEST_ASSERT(cxUTF8Decode(buf, len) == 0x1F44B, "Decode 4-byte roundtrip");

    // cxUTF8IsCombiningMark
    TEST_ASSERT(cxUTF8IsCombiningMark(0x0301) == 1, "U+0301 is combining mark");
    TEST_ASSERT(cxUTF8IsCombiningMark(0x0041) == 0, "U+0041 (A) is not combining mark");
    TEST_ASSERT(cxUTF8IsCombiningMark(0x1F3FD) == 1, "Skin tone modifier is combining");

    // cxUTF8CodepointDisplayWidth
    TEST_ASSERT(cxUTF8CodepointDisplayWidth(0x0041) == 1, "ASCII has width 1");
    TEST_ASSERT(cxUTF8CodepointDisplayWidth(0x4E2D) == 2, "CJK has width 2");
    TEST_ASSERT(cxUTF8CodepointDisplayWidth(0x0301) == 0, "Combining mark has width 0");
}


//-------------------------------------------------------------------------------------------------
// Test string parsing (multiple characters)
//-------------------------------------------------------------------------------------------------
void test_string_parsing(void)
{
    printf("\n--- Testing string parsing ---\n");

    // Parse "Aé中" - ASCII, 2-byte, 3-byte CJK
    const char *input = "A\xC3\xA9\xE4\xB8\xAD";
    const char *p = input;

    CxUTFCharacter ch1;
    int consumed1 = ch1.fromUTF8(p);
    TEST_ASSERT(ch1.codepoint() == 'A', "First char is A");
    p += consumed1;

    CxUTFCharacter ch2;
    int consumed2 = ch2.fromUTF8(p);
    TEST_ASSERT(ch2.codepoint() == 0x00E9, "Second char is é");
    p += consumed2;

    CxUTFCharacter ch3;
    int consumed3 = ch3.fromUTF8(p);
    TEST_ASSERT(ch3.codepoint() == 0x4E2D, "Third char is 中");

    TEST_ASSERT(consumed1 + consumed2 + consumed3 == 6, "Total bytes consumed is 6");
}


//-------------------------------------------------------------------------------------------------
// Test edge cases
//-------------------------------------------------------------------------------------------------
void test_edge_cases(void)
{
    printf("\n--- Testing edge cases ---\n");

    // Empty/null input
    CxUTFCharacter empty;
    int consumed = empty.fromUTF8("");
    TEST_ASSERT(consumed == 0, "Empty string consumes 0 bytes");
    TEST_ASSERT(empty.byteCount() == 0, "Empty character has 0 bytes");

    CxUTFCharacter nullCh;
    consumed = nullCh.fromUTF8((const char*)0);
    TEST_ASSERT(consumed == 0, "Null pointer consumes 0 bytes");

    // Non-combining character after base (should not be consumed)
    CxUTFCharacter notCombined;
    const char *input = "AB";
    consumed = notCombined.fromUTF8(input);
    TEST_ASSERT(consumed == 1, "Non-combiner not consumed");
    TEST_ASSERT(notCombined.codepoint() == 'A', "Only first char consumed");
}


//-------------------------------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    printf("=== CxUTFCharacter Test Suite ===\n");

    test_ascii();
    test_2byte_utf8();
    test_3byte_utf8();
    test_4byte_utf8();
    test_combining_characters();
    test_emoji_skin_tone();
    test_tabs();
    test_from_codepoint();
    test_equality();
    test_copy_assignment();
    test_utility_functions();
    test_string_parsing();
    test_edge_cases();

    printf("\n=================================\n");
    printf("Tests passed: %d\n", testsPassed);
    printf("Tests failed: %d\n", testsFailed);
    printf("=================================\n");

    return testsFailed > 0 ? 1 : 0;
}
