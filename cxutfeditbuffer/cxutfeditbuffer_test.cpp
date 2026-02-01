//-------------------------------------------------------------------------------------------------
//
// utfeditbuffer_test.cpp
//
// Test program for CxUTFEditBuffer
//
//-------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cx/editbuffer/utfeditbuffer.h>

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


void test_basic_construction(void)
{
    printf("\n--- Testing basic construction ---\n");

    CxUTFEditBuffer buf;
    TEST_ASSERT(buf.numberOfLines() == 0, "Empty buffer has 0 lines");
    TEST_ASSERT(buf.cursor.row == 0, "Cursor starts at row 0");
    TEST_ASSERT(buf.cursor.col == 0, "Cursor starts at col 0");
    TEST_ASSERT(buf.isTouched() == 0, "New buffer is not touched");
    TEST_ASSERT(buf.isReadOnly() == 0, "New buffer is not read-only");
}


void test_add_characters(void)
{
    printf("\n--- Testing add characters ---\n");

    CxUTFEditBuffer buf;

    buf.addCharacter('H');
    TEST_ASSERT(buf.numberOfLines() == 1, "After first char, 1 line");
    TEST_ASSERT(buf.cursor.col == 1, "Cursor at col 1");
    TEST_ASSERT(buf.isTouched() == 1, "Buffer is touched");

    buf.addCharacter('i');
    TEST_ASSERT(buf.cursor.col == 2, "Cursor at col 2");

    CxUTFString *line = buf.line(0);
    TEST_ASSERT(line != 0, "Line 0 exists");
    TEST_ASSERT(line->charCount() == 2, "Line has 2 characters");
    TEST_ASSERT(line->at(0)->codepoint() == 'H', "First char is H");
    TEST_ASSERT(line->at(1)->codepoint() == 'i', "Second char is i");
}


void test_add_return(void)
{
    printf("\n--- Testing add return ---\n");

    CxUTFEditBuffer buf;

    buf.addCharacter('A');
    buf.addCharacter('B');
    buf.addReturn();
    buf.addCharacter('C');

    TEST_ASSERT(buf.numberOfLines() == 2, "After return, 2 lines");
    TEST_ASSERT(buf.cursor.row == 1, "Cursor on row 1");
    TEST_ASSERT(buf.cursor.col == 1, "Cursor at col 1");

    TEST_ASSERT(buf.line(0)->charCount() == 2, "First line has 2 chars");
    TEST_ASSERT(buf.line(1)->charCount() == 1, "Second line has 1 char");
}


void test_cursor_navigation(void)
{
    printf("\n--- Testing cursor navigation ---\n");

    CxUTFEditBuffer buf;
    buf.loadTextFromString(CxString("ABC\nDEF\nGHI"));

    TEST_ASSERT(buf.numberOfLines() == 3, "Loaded 3 lines");
    TEST_ASSERT(buf.cursor.row == 0 && buf.cursor.col == 0, "Cursor at 0,0");

    // Right
    buf.cursorRightRequest();
    TEST_ASSERT(buf.cursor.col == 1, "After right, col 1");

    buf.cursorRightRequest();
    buf.cursorRightRequest();
    TEST_ASSERT(buf.cursor.col == 3, "At end of line, col 3");

    // Wrap to next line
    buf.cursorRightRequest();
    TEST_ASSERT(buf.cursor.row == 1 && buf.cursor.col == 0, "Wrapped to row 1, col 0");

    // Down
    buf.cursorDownRequest();
    TEST_ASSERT(buf.cursor.row == 2, "Down to row 2");

    // Up
    buf.cursorUpRequest();
    TEST_ASSERT(buf.cursor.row == 1, "Up to row 1");

    // Left wrap
    buf.cursorGotoRequest(1, 0);
    buf.cursorLeftRequest();
    TEST_ASSERT(buf.cursor.row == 0 && buf.cursor.col == 3, "Left wrapped to row 0, col 3");
}


void test_utf8_navigation(void)
{
    printf("\n--- Testing UTF-8 navigation ---\n");

    CxUTFEditBuffer buf;
    // "A中B" - ASCII, CJK, ASCII
    buf.loadTextFromString(CxString("A\xE4\xB8\xAD" "B"));

    TEST_ASSERT(buf.numberOfLines() == 1, "1 line");
    TEST_ASSERT(buf.line(0)->charCount() == 3, "3 characters");

    // Move right through all characters
    TEST_ASSERT(buf.cursor.col == 0, "Start at col 0");

    buf.cursorRightRequest();
    TEST_ASSERT(buf.cursor.col == 1, "After right, col 1 (中)");

    buf.cursorRightRequest();
    TEST_ASSERT(buf.cursor.col == 2, "After right, col 2 (B)");

    buf.cursorRightRequest();
    TEST_ASSERT(buf.cursor.col == 3, "After right, col 3 (end)");

    // Display column check
    TEST_ASSERT(buf.cursorDisplayColumn() == 4, "Display column is 4 (1+2+1)");

    // Left back
    buf.cursorLeftRequest();
    TEST_ASSERT(buf.cursor.col == 2, "After left, col 2");
}


void test_backspace(void)
{
    printf("\n--- Testing backspace ---\n");

    CxUTFEditBuffer buf;
    buf.loadTextFromString(CxString("ABC"));
    buf.cursorGotoRequest(0, 2);

    buf.addBackspace();
    TEST_ASSERT(buf.line(0)->charCount() == 2, "After backspace, 2 chars");
    TEST_ASSERT(buf.cursor.col == 1, "Cursor at col 1");
    TEST_ASSERT(buf.line(0)->at(0)->codepoint() == 'A', "First is A");
    TEST_ASSERT(buf.line(0)->at(1)->codepoint() == 'C', "Second is C");
}


void test_join_lines(void)
{
    printf("\n--- Testing join lines (backspace at start) ---\n");

    CxUTFEditBuffer buf;
    buf.loadTextFromString(CxString("AB\nCD"));

    TEST_ASSERT(buf.numberOfLines() == 2, "2 lines");

    buf.cursorGotoRequest(1, 0);
    buf.addBackspace();

    TEST_ASSERT(buf.numberOfLines() == 1, "After join, 1 line");
    TEST_ASSERT(buf.line(0)->charCount() == 4, "Line has 4 chars");
    TEST_ASSERT(buf.cursor.col == 2, "Cursor at col 2 (join point)");
}


void test_tabs(void)
{
    printf("\n--- Testing tabs ---\n");

    CxUTFEditBuffer buf(4);  // tabWidth = 4

    buf.addCharacter('A');
    buf.addTab();
    buf.addCharacter('B');

    TEST_ASSERT(buf.line(0)->charCount() == 3, "3 characters (A, tab, B)");
    TEST_ASSERT(buf.line(0)->at(1)->isTab() == 1, "Second char is tab");

    // Check display width
    // A at col 0, tab fills to col 4, B at col 4
    TEST_ASSERT(buf.cursorDisplayColumn() == 5, "Cursor at display col 5");
}


void test_copy_paste(void)
{
    printf("\n--- Testing copy/paste ---\n");

    CxUTFEditBuffer buf;
    buf.loadTextFromString(CxString("Hello World"));

    // Select "ello"
    buf.cursorGotoRequest(0, 1);
    buf.setMark();
    buf.cursorGotoRequest(0, 5);

    CxString copied = buf.copyText();
    TEST_ASSERT(strcmp(copied.data(), "ello") == 0, "Copied 'ello'");

    // Paste at end
    buf.cursorGotoRequest(0, 11);
    buf.pasteFromCutBuffer(copied);

    CxString result = buf.line(0)->toBytes();
    TEST_ASSERT(strcmp(result.data(), "Hello Worldello") == 0, "Pasted at end");
}


void test_delete_text(void)
{
    printf("\n--- Testing delete text ---\n");

    CxUTFEditBuffer buf;
    buf.loadTextFromString(CxString("ABCDEF"));

    buf.cursorGotoRequest(0, 1);
    buf.setMark();
    buf.cursorGotoRequest(0, 4);

    buf.deleteText();

    CxString result = buf.line(0)->toBytes();
    TEST_ASSERT(strcmp(result.data(), "AEF") == 0, "Deleted BCD");
    TEST_ASSERT(buf.cursor.col == 1, "Cursor at delete start");
}


void test_find(void)
{
    printf("\n--- Testing find ---\n");

    CxUTFEditBuffer buf;
    buf.loadTextFromString(CxString("Hello World Hello"));

    int found = buf.findString(CxString("World"));
    TEST_ASSERT(found == 1, "Found 'World'");
    TEST_ASSERT(buf.cursor.col == 6, "Cursor at col 6");

    found = buf.findAgain(CxString("Hello"), 1);
    TEST_ASSERT(found == 1, "Found second 'Hello'");
    TEST_ASSERT(buf.cursor.col == 12, "Cursor at col 12");
}


void test_save_load_roundtrip(void)
{
    printf("\n--- Testing save/load roundtrip ---\n");

    // Create buffer with UTF-8 content
    CxUTFEditBuffer buf1;
    buf1.loadTextFromString(CxString("Hello\n\xE4\xB8\xAD\xE6\x96\x87\nWorld"));  // "Hello\n中文\nWorld"

    buf1.saveText(CxString("/tmp/utfeditbuffer_test.txt"));

    CxUTFEditBuffer buf2;
    int loaded = buf2.loadText(CxString("/tmp/utfeditbuffer_test.txt"), 1);

    TEST_ASSERT(loaded == 1, "File loaded");
    TEST_ASSERT(buf2.numberOfLines() == 3, "3 lines loaded");

    CxString line1 = buf2.line(1)->toBytes();
    TEST_ASSERT(strcmp(line1.data(), "\xE4\xB8\xAD\xE6\x96\x87") == 0, "UTF-8 preserved");
}


void test_box_drawing(void)
{
    printf("\n--- Testing box drawing characters ---\n");

    CxUTFEditBuffer buf;
    // "│cell│"
    buf.loadTextFromString(CxString("\xE2\x94\x82" "cell" "\xE2\x94\x82"));

    TEST_ASSERT(buf.numberOfLines() == 1, "1 line");
    TEST_ASSERT(buf.line(0)->charCount() == 6, "6 characters");

    // Navigate across box drawing chars
    buf.cursorRightRequest();
    TEST_ASSERT(buf.cursor.col == 1, "After right, col 1");

    buf.cursorGotoRequest(0, 5);
    TEST_ASSERT(buf.line(0)->at(5)->codepoint() == 0x2502, "Last char is │");
}


void test_emoji(void)
{
    printf("\n--- Testing emoji ---\n");

    CxUTFEditBuffer buf;
    // "Hi👋" - ASCII + emoji
    buf.loadTextFromString(CxString("Hi\xF0\x9F\x91\x8B"));

    TEST_ASSERT(buf.line(0)->charCount() == 3, "3 characters");

    buf.cursorGotoRequest(0, 2);
    TEST_ASSERT(buf.line(0)->at(2)->codepoint() == 0x1F44B, "Third is waving hand");

    // Display width should account for emoji being double-width
    buf.cursorGotoRequest(0, 3);
    TEST_ASSERT(buf.cursorDisplayColumn() == 4, "Display column is 4 (1+1+2)");
}


void test_flatten_buffer(void)
{
    printf("\n--- Testing flatten buffer ---\n");

    CxUTFEditBuffer buf;
    buf.loadTextFromString(CxString("Line1\nLine2\nLine3"));

    CxString flat = buf.flattenBuffer();
    TEST_ASSERT(strcmp(flat.data(), "Line1\nLine2\nLine3\n") == 0, "Flattened correctly");
}


int main(int argc, char **argv)
{
    printf("=== CxUTFEditBuffer Test Suite ===\n");

    test_basic_construction();
    test_add_characters();
    test_add_return();
    test_cursor_navigation();
    test_utf8_navigation();
    test_backspace();
    test_join_lines();
    test_tabs();
    test_copy_paste();
    test_delete_text();
    test_find();
    test_save_load_roundtrip();
    test_box_drawing();
    test_emoji();
    test_flatten_buffer();

    printf("\n=================================\n");
    printf("Tests passed: %d\n", testsPassed);
    printf("Tests failed: %d\n", testsFailed);
    printf("=================================\n");

    return testsFailed > 0 ? 1 : 0;
}
