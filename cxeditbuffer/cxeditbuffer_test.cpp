//-----------------------------------------------------------------------------------------
// cxeditbuffer_test.cpp - CxEditBuffer unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <cx/base/string.h>
#include <cx/base/slist.h>
#include <cx/editbuffer/editbuffer.h>

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
// Constructor and initialization tests
//-----------------------------------------------------------------------------------------
void testConstructors() {
    printf("\n== Constructor Tests ==\n");

    // Default constructor
    {
        CxEditBuffer buffer;
        check(buffer.numberOfLines() == 0, "default ctor creates empty buffer");
        check(buffer.cursor.row == 0, "default ctor cursor row is 0");
        check(buffer.cursor.col == 0, "default ctor cursor col is 0");
        check(!buffer.isTouched(), "default ctor buffer not touched");
        check(!buffer.isReadOnly(), "default ctor buffer not read-only");
    }

    // Constructor with tab spaces
    {
        CxEditBuffer buffer(4);
        check(buffer.numberOfLines() == 0, "tab ctor creates empty buffer");
    }

    // Reset
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.reset();
        check(buffer.numberOfLines() == 0, "reset clears buffer");
        check(buffer.cursor.row == 0, "reset resets cursor row");
        check(buffer.cursor.col == 0, "reset resets cursor col");
    }
}

//-----------------------------------------------------------------------------------------
// Character insertion tests
//-----------------------------------------------------------------------------------------
void testCharacterInsertion() {
    printf("\n== Character Insertion Tests ==\n");

    // Add single character
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        check(buffer.numberOfLines() == 1, "addCharacter creates line");
        CxString* line = buffer.line(0);
        check(line != NULL, "line(0) returns non-null");
        check(strcmp(line->data(), "a") == 0, "addCharacter stores char");
    }

    // Add multiple characters - check both row and col
    {
        CxEditBuffer buffer;
        buffer.addCharacter('h');
        buffer.addCharacter('e');
        buffer.addCharacter('l');
        buffer.addCharacter('l');
        buffer.addCharacter('o');
        CxString* line = buffer.line(0);
        check(strcmp(line->data(), "hello") == 0, "multiple addCharacter builds string");
        check(buffer.cursor.row == 0, "cursor row stays 0 after characters");
        check(buffer.cursor.col == 5, "cursor col advances with characters");
    }

    // Add character via CxString
    {
        CxEditBuffer buffer;
        buffer.addCharacter(CxString("x"));
        CxString* line = buffer.line(0);
        check(strcmp(line->data(), "x") == 0, "addCharacter(CxString) works");
    }

    // Add return creates new line - check cursor position
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addReturn();
        check(buffer.cursor.row == 1, "cursor row advances after addReturn");
        check(buffer.cursor.col == 0, "cursor col is 0 after addReturn");
        buffer.addCharacter('b');
        check(buffer.numberOfLines() == 2, "addReturn creates new line");
        check(strcmp(buffer.line(0)->data(), "a") == 0, "first line correct after return");
        check(strcmp(buffer.line(1)->data(), "b") == 0, "second line correct after return");
    }

    // Add return in middle of line splits it - check cursor position
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');
        buffer.cursorGotoRequest(0, 1);
        buffer.addReturn();
        check(buffer.cursor.row == 1, "cursor row advances after split");
        check(buffer.cursor.col == 0, "cursor col is 0 after split");
        check(buffer.numberOfLines() == 2, "return in middle creates two lines");
        check(strcmp(buffer.line(0)->data(), "a") == 0, "split line first part");
        check(strcmp(buffer.line(1)->data(), "bc") == 0, "split line second part");
    }

    // Buffer touched after edit
    {
        CxEditBuffer buffer;
        check(!buffer.isTouched(), "buffer not touched initially");
        buffer.addCharacter('x');
        check(buffer.isTouched(), "buffer touched after addCharacter");
    }
}

//-----------------------------------------------------------------------------------------
// Backspace tests
//-----------------------------------------------------------------------------------------
void testBackspace() {
    printf("\n== Backspace Tests ==\n");

    // Backspace removes character
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addBackspace();
        CxString* line = buffer.line(0);
        check(strcmp(line->data(), "a") == 0, "backspace removes last char");
        check(buffer.cursor.col == 1, "cursor moves back after backspace");
    }

    // Backspace at beginning of line joins lines - check cursor at join point
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addReturn();
        buffer.addCharacter('c');
        buffer.addCharacter('d');
        buffer.cursorGotoRequest(1, 0);
        buffer.addBackspace();
        check(buffer.numberOfLines() == 1, "backspace at line start joins lines");
        check(strcmp(buffer.line(0)->data(), "abcd") == 0, "joined line content correct");
        check(buffer.cursor.row == 0, "cursor row is 0 after join");
        check(buffer.cursor.col == 2, "cursor col at join point (end of first line)");
    }

    // Backspace at buffer start does nothing
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.cursorGotoRequest(0, 0);
        CxEditHint hint = buffer.addBackspace();
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_NONE, "backspace at start returns NONE");
        check(strcmp(buffer.line(0)->data(), "a") == 0, "content unchanged");
    }

    // Multiple backspaces
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');
        buffer.addBackspace();
        buffer.addBackspace();
        CxString* line = buffer.line(0);
        check(strcmp(line->data(), "a") == 0, "multiple backspaces work");
    }
}

//-----------------------------------------------------------------------------------------
// Cursor movement tests
//-----------------------------------------------------------------------------------------
void testCursorMovement() {
    printf("\n== Cursor Movement Tests ==\n");

    // Cursor right
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');
        buffer.cursorGotoRequest(0, 0);
        buffer.cursorRightRequest();
        check(buffer.cursor.col == 1, "cursorRight moves right");
    }

    // Cursor left
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.cursorLeftRequest();
        check(buffer.cursor.col == 1, "cursorLeft moves left");
    }

    // Cursor up
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addReturn();
        buffer.addCharacter('b');
        buffer.cursorUpRequest();
        check(buffer.cursor.row == 0, "cursorUp moves to previous row");
    }

    // Cursor down
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addReturn();
        buffer.addCharacter('b');
        buffer.cursorGotoRequest(0, 0);
        buffer.cursorDownRequest();
        check(buffer.cursor.row == 1, "cursorDown moves to next row");
    }

    // cursorGotoRequest
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addReturn();
        buffer.addCharacter('c');
        buffer.addCharacter('d');
        CxEditBuffer::POSITION pos = buffer.cursorGotoRequest(1, 1);
        check(pos == CxEditBuffer::POS_VALID_INSERT, "cursorGotoRequest valid position");
        check(buffer.cursor.row == 1, "cursorGotoRequest sets row");
        check(buffer.cursor.col == 1, "cursorGotoRequest sets col");
    }

    // cursorGotoRequest invalid row
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        CxEditBuffer::POSITION pos = buffer.cursorGotoRequest(10, 0);
        check(pos == CxEditBuffer::POS_INVALID_ROW, "cursorGotoRequest invalid row");
    }

    // Cursor left at start wraps to previous line - check col at end
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');
        buffer.addReturn();
        buffer.addCharacter('d');
        buffer.cursorGotoRequest(1, 0);
        CxEditHint hint = buffer.cursorLeftRequest();
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_WRAP_UP, "cursorLeft at start wraps up");
        check(buffer.cursor.row == 0, "cursor row on previous line after wrap up");
        check(buffer.cursor.col == 3, "cursor col at end of previous line after wrap up");
    }

    // Cursor right at end wraps to next line
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addReturn();
        buffer.addCharacter('c');
        buffer.addCharacter('d');
        buffer.cursorGotoRequest(0, 2);  // at end of first line
        CxEditHint hint = buffer.cursorRightRequest();
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_WRAP_DOWN, "cursorRight at end wraps down");
        check(buffer.cursor.row == 1, "cursor row on next line after wrap down");
        check(buffer.cursor.col == 0, "cursor col at start of next line after wrap down");
    }
}

//-----------------------------------------------------------------------------------------
// Line access tests
//-----------------------------------------------------------------------------------------
void testLineAccess() {
    printf("\n== Line Access Tests ==\n");

    // line() returns correct line
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addReturn();
        buffer.addCharacter('b');
        buffer.addReturn();
        buffer.addCharacter('c');

        check(strcmp(buffer.line(0)->data(), "a") == 0, "line(0) correct");
        check(strcmp(buffer.line(1)->data(), "b") == 0, "line(1) correct");
        check(strcmp(buffer.line(2)->data(), "c") == 0, "line(2) correct");
    }

    // line() invalid index returns NULL
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        CxString* line = buffer.line(10);
        check(line == NULL, "line(invalid) returns NULL");
    }

    // numberOfLines
    {
        CxEditBuffer buffer;
        check(buffer.numberOfLines() == 0, "empty buffer has 0 lines");

        buffer.addCharacter('a');
        check(buffer.numberOfLines() == 1, "one char creates 1 line");

        buffer.addReturn();
        buffer.addCharacter('b');
        check(buffer.numberOfLines() == 2, "return creates 2 lines");
    }

    // characterCount - basic
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("hello");
        // "hello" = 5 chars + 1 newline = 6
        check(buffer.characterCount() == 6, "characterCount: 'hello' = 6 (5 + newline)");
    }

    // characterCount - multiple lines
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("ab\ncd\nef");
        // "ab" + newline + "cd" + newline + "ef" + newline = 2+1+2+1+2+1 = 9
        check(buffer.characterCount() == 9, "characterCount: 'ab\\ncd\\nef' = 9");
    }

    // characterCount - excludes tab extensions (0xFF)
    {
        CxEditBuffer buffer(4);  // 4-space tabs
        buffer.addCharacter('a');
        buffer.addTab();  // tab at col 1 expands to cols 1,2,3 (3 chars: 1 tab + 2 extensions)
        buffer.addCharacter('b');
        // Internal: "a" + tab + 0xFF + 0xFF + "b" = 5 raw chars
        // Logical: "a" + tab + "b" + newline = 4 chars
        check(buffer.characterCount() == 4, "characterCount excludes tab extensions");
    }

    // detab - convert tabs to spaces
    {
        CxEditBuffer buffer(4);  // 4-space tabs
        buffer.addCharacter('\t');  // tab at col 0 = 4 chars internal
        buffer.addCharacter('x');
        buffer.detab();
        CxString *line = buffer.line(0);
        // tab + extensions should become 4 spaces
        check(line->length() == 5, "detab: line length is 5 (4 spaces + x)");
        check(line->charAt(0) == ' ', "detab: char 0 is space");
        check(line->charAt(3) == ' ', "detab: char 3 is space");
        check(line->charAt(4) == 'x', "detab: char 4 is x");
    }

    // entab - convert leading spaces to tabs
    {
        CxEditBuffer buffer(4);  // 4-space tabs
        buffer.loadTextFromString("    hello");  // 4 leading spaces
        buffer.entab();
        CxString *line = buffer.line(0);
        // 4 spaces should become tab + 3 extensions (0xFF)
        check(line->charAt(0) == '\t', "entab: char 0 is tab");
        check((unsigned char)line->charAt(1) == 0xFF, "entab: char 1 is 0xFF extension");
        check((unsigned char)line->charAt(2) == 0xFF, "entab: char 2 is 0xFF extension");
        check((unsigned char)line->charAt(3) == 0xFF, "entab: char 3 is 0xFF extension");
        check(line->charAt(4) == 'h', "entab: char 4 is h");
    }

    // entab - partial leading spaces stay as spaces
    {
        CxEditBuffer buffer(4);  // 4-space tabs
        buffer.loadTextFromString("  hi");  // 2 leading spaces (not enough for tab)
        buffer.entab();
        CxString *line = buffer.line(0);
        check(line->charAt(0) == ' ', "entab partial: char 0 is space");
        check(line->charAt(1) == ' ', "entab partial: char 1 is space");
        check(line->charAt(2) == 'h', "entab partial: char 2 is h");
    }

    // entab - 8 spaces becomes 2 tabs
    {
        CxEditBuffer buffer(4);  // 4-space tabs
        buffer.loadTextFromString("        code");  // 8 leading spaces
        buffer.entab();
        CxString *line = buffer.line(0);
        check(line->charAt(0) == '\t', "entab 8 spaces: first tab");
        check(line->charAt(4) == '\t', "entab 8 spaces: second tab");
        check(line->charAt(8) == 'c', "entab 8 spaces: code starts at 8");
    }

    // detab then entab roundtrip
    {
        CxEditBuffer buffer(4);
        buffer.addCharacter('\t');
        buffer.addCharacter('x');
        buffer.detab();  // tab -> spaces
        buffer.entab();  // spaces -> tab
        CxString *line = buffer.line(0);
        check(line->charAt(0) == '\t', "roundtrip: char 0 is tab after detab+entab");
        check(line->charAt(4) == 'x', "roundtrip: x at position 4");
    }

    // characterAt
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');

        check(buffer.characterAt(0, 0) == 'a', "characterAt(0,0) correct");
        check(buffer.characterAt(0, 1) == 'b', "characterAt(0,1) correct");
        check(buffer.characterAt(0, 2) == 'c', "characterAt(0,2) correct");
    }

    // characterAt invalid position
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        char c = buffer.characterAt(0, 10);
        check(c == 0, "characterAt invalid col returns 0");
    }
}

//-----------------------------------------------------------------------------------------
// Mark and cut/copy tests
//-----------------------------------------------------------------------------------------
void testMarkAndCut() {
    printf("\n== Mark and Cut/Copy Tests ==\n");

    // setMark
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.cursorGotoRequest(0, 1);
        buffer.setMark();
        check(buffer.mark.row == 0, "setMark captures row");
        check(buffer.mark.col == 1, "setMark captures col");
    }

    // copyText
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');
        buffer.cursorGotoRequest(0, 0);
        buffer.setMark();
        buffer.cursorGotoRequest(0, 2);
        CxString copied = buffer.copyText();
        check(strcmp(copied.data(), "ab") == 0, "copyText returns marked text");
        // Buffer should be unchanged
        check(strcmp(buffer.line(0)->data(), "abc") == 0, "copyText doesn't modify buffer");
    }

    // cutToMark - check cursor position after cut
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');
        buffer.cursorGotoRequest(0, 1);
        buffer.setMark();
        buffer.cursorGotoRequest(0, 3);
        CxString cut = buffer.cutToMark();
        check(strcmp(cut.data(), "bc") == 0, "cutToMark returns cut text");
        check(strcmp(buffer.line(0)->data(), "a") == 0, "cutToMark removes text");
        check(buffer.cursor.col == 1, "cursor at mark position after cut");
    }

    // cutTextToEndOfLine - check cursor position
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');
        buffer.addCharacter('d');
        buffer.cursorGotoRequest(0, 2);
        CxString cut = buffer.cutTextToEndOfLine();
        check(strcmp(cut.data(), "cd") == 0, "cutTextToEndOfLine returns end of line");
        check(strcmp(buffer.line(0)->data(), "ab") == 0, "cutTextToEndOfLine removes text");
        check(buffer.cursor.col == 2, "cursor stays at cut position");
    }

    // deleteText
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');
        buffer.addCharacter('d');
        buffer.cursorGotoRequest(0, 1);
        buffer.setMark();
        buffer.cursorGotoRequest(0, 3);
        buffer.deleteText();
        check(strcmp(buffer.line(0)->data(), "ad") == 0, "deleteText removes marked region");
    }
}

//-----------------------------------------------------------------------------------------
// Insert and paste tests
//-----------------------------------------------------------------------------------------
void testInsertAndPaste() {
    printf("\n== Insert and Paste Tests ==\n");

    // insertTextAtCursor - check cursor position after insert
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('d');
        buffer.cursorGotoRequest(0, 1);
        buffer.insertTextAtCursor("bc");
        check(strcmp(buffer.line(0)->data(), "abcd") == 0, "insertTextAtCursor inserts text");
        check(buffer.cursor.col == 3, "cursor advances past inserted text");
    }

    // pasteFromCutBuffer - check cursor position after paste
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('d');
        buffer.cursorGotoRequest(0, 1);
        buffer.pasteFromCutBuffer("bc");
        check(strcmp(buffer.line(0)->data(), "abcd") == 0, "pasteFromCutBuffer inserts text");
        check(buffer.cursor.col == 3, "cursor advances past pasted text");
    }

    // Insert text with newlines - check cursor position
    {
        CxEditBuffer buffer;
        buffer.addCharacter('x');
        buffer.cursorGotoRequest(0, 0);
        buffer.insertTextAtCursor("a\nb\n");
        check(buffer.numberOfLines() >= 2, "insertTextAtCursor handles newlines");
        // Cursor should be after the inserted text
        check(buffer.cursor.row >= 1, "cursor row advances with newlines");
    }
}

//-----------------------------------------------------------------------------------------
// Find and replace tests
//-----------------------------------------------------------------------------------------
void testFindAndReplace() {
    printf("\n== Find and Replace Tests ==\n");

    // findString - found
    {
        CxEditBuffer buffer;
        buffer.addCharacter('h');
        buffer.addCharacter('e');
        buffer.addCharacter('l');
        buffer.addCharacter('l');
        buffer.addCharacter('o');
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString("llo");
        check(found, "findString returns true when found");
        check(buffer.cursor.col == 2, "findString moves cursor to match");
    }

    // findString - not found
    {
        CxEditBuffer buffer;
        buffer.addCharacter('h');
        buffer.addCharacter('e');
        buffer.addCharacter('l');
        buffer.addCharacter('l');
        buffer.addCharacter('o');
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString("xyz");
        check(!found, "findString returns false when not found");
    }

    // findString across lines
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addReturn();
        buffer.addCharacter('c');
        buffer.addCharacter('d');
        buffer.addReturn();
        buffer.addCharacter('e');
        buffer.addCharacter('f');
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString("ef");
        check(found, "findString finds text on later line");
        check(buffer.cursor.row == 2, "findString cursor on correct row");
    }

    // replaceString (cursor must be at find position first)
    // Note: replaceString returns whether NEXT occurrence was found, not if replacement worked
    {
        CxEditBuffer buffer;
        buffer.addCharacter('h');
        buffer.addCharacter('e');
        buffer.addCharacter('l');
        buffer.addCharacter('l');
        buffer.addCharacter('o');
        buffer.cursorGotoRequest(0, 0);
        // findString positions cursor at the match
        buffer.findString("ll");
        check(buffer.cursor.col == 2, "findString positions cursor at match");
        int foundNext = buffer.replaceString("ll", "LL");
        // Returns false because no more "ll" after replacement
        check(!foundNext, "replaceString returns false when no more matches");
        check(strcmp(buffer.line(0)->data(), "heLLo") == 0, "replaceString replaces text");
    }

    // replaceString with multiple occurrences
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("hello hello");
        buffer.cursorGotoRequest(0, 0);
        buffer.findString("ll");
        int foundNext = buffer.replaceString("ll", "LL");
        // Returns true because there's another "ll" in the second "hello"
        check(foundNext, "replaceString returns true when more matches exist");
        check(strcmp(buffer.line(0)->data(), "heLLo hello") == 0, "first replaceString works");
    }

    // replace-all where replacement contains find string (regression test for infinite loop)
    // This tests that cursor advances past replacement to avoid re-finding within it
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("// comment\n// another");
        buffer.cursorGotoRequest(0, 0);

        // Simulate replace-all: loop until no more matches
        // Note: replaceAgain returns TRUE if NEXT match found, FALSE otherwise
        // So for N occurrences, we get N-1 TRUE returns + 1 FALSE (loop exit)
        int count = 0;
        int maxIterations = 100;  // safety limit
        while (buffer.replaceAgain("//", "///") && count < maxIterations) {
            count++;
        }

        // 2 occurrences: 1st replace returns TRUE (found 2nd), 2nd returns FALSE → count=1
        check(count == 1, "replace-all with overlapping pattern: 1 TRUE return for 2 occurrences");
        check(count < maxIterations, "replace-all terminates (no infinite loop)");
        check(strcmp(buffer.line(0)->data(), "/// comment") == 0,
              "replace-all line 0: '/// comment'");
        check(strcmp(buffer.line(1)->data(), "/// another") == 0,
              "replace-all line 1: '/// another'");
    }

    // replace-all on single line with multiple adjacent matches
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("////");  // two adjacent "//" patterns
        buffer.cursorGotoRequest(0, 0);

        int count = 0;
        int maxIterations = 100;
        while (buffer.replaceAgain("//", "///") && count < maxIterations) {
            count++;
        }

        // 2 occurrences: 1st replace returns TRUE (found 2nd), 2nd returns FALSE → count=1
        check(count == 1, "adjacent patterns: 1 TRUE return for 2 occurrences");
        check(count < maxIterations, "adjacent patterns: terminates");
        // "////" -> "/////" (first //) -> "//////" (second //)
        check(strcmp(buffer.line(0)->data(), "//////") == 0,
              "adjacent replace result: '//////'");
    }
}

//-----------------------------------------------------------------------------------------
// Load text from string tests
//-----------------------------------------------------------------------------------------
void testLoadFromString() {
    printf("\n== Load From String Tests ==\n");

    // loadTextFromString single line - check cursor position
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("hello world");
        check(buffer.numberOfLines() == 1, "loadTextFromString creates line");
        check(strcmp(buffer.line(0)->data(), "hello world") == 0, "loadTextFromString content correct");
        check(buffer.cursor.row == 0, "cursor row is 0 after loadTextFromString");
        check(buffer.cursor.col == 0, "cursor col is 0 after loadTextFromString");
    }

    // loadTextFromString multiple lines - check cursor position
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("line1\nline2\nline3");
        check(buffer.numberOfLines() == 3, "loadTextFromString creates multiple lines");
        check(strcmp(buffer.line(0)->data(), "line1") == 0, "loadTextFromString line 1");
        check(strcmp(buffer.line(1)->data(), "line2") == 0, "loadTextFromString line 2");
        check(strcmp(buffer.line(2)->data(), "line3") == 0, "loadTextFromString line 3");
        check(buffer.cursor.row == 0, "cursor row is 0 after multi-line load");
        check(buffer.cursor.col == 0, "cursor col is 0 after multi-line load");
    }

    // flattenBuffer
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addReturn();
        buffer.addCharacter('b');
        CxString flat = buffer.flattenBuffer();
        check(flat.index("\n") != -1, "flattenBuffer contains newline");
    }
}

//-----------------------------------------------------------------------------------------
// Status and property tests
//-----------------------------------------------------------------------------------------
void testStatusMethods() {
    printf("\n== Status Methods Tests ==\n");

    // isReadOnly
    {
        CxEditBuffer buffer;
        check(!buffer.isReadOnly(), "default not read-only");
        buffer.setReadOnly(1);
        check(buffer.isReadOnly(), "setReadOnly(1) makes read-only");
        buffer.setReadOnly(0);
        check(!buffer.isReadOnly(), "setReadOnly(0) makes writable");
    }

    // isTouched
    {
        CxEditBuffer buffer;
        check(!buffer.isTouched(), "new buffer not touched");
        buffer.addCharacter('a');
        check(buffer.isTouched(), "buffer touched after edit");
    }

    // filePath
    {
        CxEditBuffer buffer;
        buffer.setFilePath("/path/to/file.txt");
        check(strcmp(buffer.getFilePath().data(), "/path/to/file.txt") == 0, "getFilePath returns set path");
    }

    // name
    {
        CxEditBuffer buffer;
        buffer.name = "TestBuffer";
        check(strcmp(buffer.name.data(), "TestBuffer") == 0, "name can be set and read");
    }

    // visual screen position
    {
        CxEditBuffer buffer;
        buffer.setVisualFirstScreenLine(10);
        check(buffer.getVisualFirstScreenLine() == 10, "visual screen line persists");

        buffer.setVisualFirstScreenCol(5);
        check(buffer.getVisualFirstScreenCol() == 5, "visual screen col persists");
    }
}

//-----------------------------------------------------------------------------------------
// Position evaluation tests
//-----------------------------------------------------------------------------------------
void testPositionEvaluation() {
    printf("\n== Position Evaluation Tests ==\n");

    // evaluatePosition - valid insert
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');
        CxEditBuffer::POSITION pos = buffer.evaluatePosition(0, 1);
        check(pos == CxEditBuffer::POS_VALID_INSERT, "evaluatePosition valid insert");
    }

    // evaluatePosition - append column
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        CxEditBuffer::POSITION pos = buffer.evaluatePosition(0, 2);
        check(pos == CxEditBuffer::POS_VALID_APPEND_COL, "evaluatePosition append col");
    }

    // evaluatePosition - invalid row
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        CxEditBuffer::POSITION pos = buffer.evaluatePosition(10, 0);
        check(pos == CxEditBuffer::POS_INVALID_ROW, "evaluatePosition invalid row");
    }

    // evaluatePosition - invalid col
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        CxEditBuffer::POSITION pos = buffer.evaluatePosition(0, 10);
        check(pos == CxEditBuffer::POS_INVALID_COL, "evaluatePosition invalid col");
    }
}

//-----------------------------------------------------------------------------------------
// Helper to check if cursor is on a tab extension character
//-----------------------------------------------------------------------------------------
int cursorOnTabExtension(CxEditBuffer& buffer) {
    CxString* line = buffer.line(buffer.cursor.row);
    if (line == NULL) return 0;
    if (buffer.cursor.col >= line->length()) return 0;
    return (line->data()[buffer.cursor.col] == '\377');
}

//-----------------------------------------------------------------------------------------
// Tab handling tests - cursor should never end up on tab extension characters
//-----------------------------------------------------------------------------------------
void testTabHandling() {
    printf("\n== Tab Handling Tests ==\n");

    // addTab places cursor after the tab (not on extension)
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addTab();
        check(!cursorOnTabExtension(buffer), "cursor not on tab extension after addTab");
        // With default tab of 4, 'a' at col 0, tab at col 1 extends to col 4
        // cursor should be at col 4 (past the tab extensions)
    }

    // addTab at start of line
    {
        CxEditBuffer buffer;
        buffer.addTab();
        check(!cursorOnTabExtension(buffer), "cursor not on tab ext after addTab at start");
        check(buffer.cursor.col == 4, "cursor at tab stop after addTab at col 0");
    }

    // addTab after multiple characters
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');  // cursor now at col 3
        buffer.addTab();
        check(!cursorOnTabExtension(buffer), "cursor not on tab ext after addTab at col 3");
        check(buffer.cursor.col == 4, "cursor at next tab stop after addTab");
    }

    // cursorRight through a tab skips extensions
    {
        CxEditBuffer buffer;
        buffer.addTab();
        buffer.addCharacter('x');
        buffer.cursorGotoRequest(0, 0);  // position at start (on the tab character)
        buffer.cursorRightRequest();
        check(!cursorOnTabExtension(buffer), "cursorRight skips tab extensions");
        check(buffer.cursor.col == 4, "cursorRight jumps past tab extensions");
    }

    // cursorLeft through a tab skips extensions
    {
        CxEditBuffer buffer;
        buffer.addTab();
        buffer.addCharacter('x');
        buffer.cursorGotoRequest(0, 4);  // position after tab, before 'x'
        buffer.cursorLeftRequest();
        check(!cursorOnTabExtension(buffer), "cursorLeft skips tab extensions");
        check(buffer.cursor.col == 0, "cursorLeft jumps back to tab start");
    }

    // cursorUp to line with tab - cursor should not land on extension
    {
        CxEditBuffer buffer;
        buffer.addTab();           // line 0: tab (cols 0-3)
        buffer.addCharacter('x');  // line 0: x at col 4
        buffer.addReturn();
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');  // line 1: "abc", cursor at col 3
        // Now move up - col 3 would be on tab extension
        buffer.cursorUpRequest();
        check(!cursorOnTabExtension(buffer), "cursorUp avoids tab extension");
        check(buffer.cursor.row == 0, "cursorUp moved to row 0");
    }

    // cursorDown to line with tab - cursor should not land on extension
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');
        buffer.addCharacter('c');  // line 0: "abc", cursor at col 3
        buffer.addReturn();
        buffer.addTab();           // line 1: tab (cols 0-3)
        buffer.addCharacter('x');  // line 1: x at col 4
        buffer.cursorGotoRequest(0, 2);  // on line 0, col 2
        buffer.cursorDownRequest();
        check(!cursorOnTabExtension(buffer), "cursorDown avoids tab extension");
        check(buffer.cursor.row == 1, "cursorDown moved to row 1");
    }

    // evaluatePosition returns POS_INVALID_MID_TAB for tab extension position
    {
        CxEditBuffer buffer;
        buffer.addTab();
        buffer.addCharacter('x');
        // Tab at col 0, extensions at cols 1-3, 'x' at col 4
        CxEditBuffer::POSITION pos = buffer.evaluatePosition(0, 1);
        check(pos == CxEditBuffer::POS_INVALID_MID_TAB, "evaluatePosition detects mid-tab at col 1");
        pos = buffer.evaluatePosition(0, 2);
        check(pos == CxEditBuffer::POS_INVALID_MID_TAB, "evaluatePosition detects mid-tab at col 2");
        pos = buffer.evaluatePosition(0, 3);
        check(pos == CxEditBuffer::POS_INVALID_MID_TAB, "evaluatePosition detects mid-tab at col 3");
        pos = buffer.evaluatePosition(0, 0);
        check(pos == CxEditBuffer::POS_VALID_INSERT, "evaluatePosition valid at tab char");
        pos = buffer.evaluatePosition(0, 4);
        check(pos == CxEditBuffer::POS_VALID_INSERT, "evaluatePosition valid after tab");
    }

    // cursorGotoRequest to mid-tab position should fail
    {
        CxEditBuffer buffer;
        buffer.addTab();
        buffer.addCharacter('x');
        CxEditBuffer::POSITION pos = buffer.cursorGotoRequest(0, 2);
        check(pos == CxEditBuffer::POS_INVALID_MID_TAB, "cursorGotoRequest rejects mid-tab");
        // Cursor should have been moved to col 0 (safe position)
        check(!cursorOnTabExtension(buffer), "cursor not on tab ext after rejected goto");
    }

    // backspace through tab removes entire tab
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addTab();
        buffer.addCharacter('b');
        // Line is now: a[tab][ext][ext]b, cursor at col 5
        buffer.addBackspace();  // removes 'b'
        check(!cursorOnTabExtension(buffer), "cursor not on ext after backspace removes char");
        buffer.addBackspace();  // should remove tab
        check(!cursorOnTabExtension(buffer), "cursor not on ext after backspace removes tab");
        check(buffer.cursor.col == 1, "cursor at col 1 after tab backspace");
    }

    // Multiple tabs
    {
        CxEditBuffer buffer;
        buffer.addTab();
        buffer.addTab();
        buffer.addCharacter('x');
        check(!cursorOnTabExtension(buffer), "cursor not on ext after multiple tabs");
        check(buffer.cursor.col == 9, "cursor after two tabs and char");

        // Navigate back through both tabs
        buffer.cursorLeftRequest();  // to col 8
        check(!cursorOnTabExtension(buffer), "cursorLeft past second tab valid");
        buffer.cursorLeftRequest();  // should jump to col 4
        check(!cursorOnTabExtension(buffer), "cursorLeft through second tab valid");
        buffer.cursorLeftRequest();  // should jump to col 0
        check(!cursorOnTabExtension(buffer), "cursorLeft through first tab valid");
        check(buffer.cursor.col == 0, "cursor at start after navigating back");
    }

    // Insert character before tab
    {
        CxEditBuffer buffer;
        buffer.addTab();
        buffer.addCharacter('x');
        buffer.cursorGotoRequest(0, 0);
        buffer.addCharacter('a');  // insert 'a' before tab
        check(!cursorOnTabExtension(buffer), "cursor valid after insert before tab");
    }

    // Tab after partial column (tab should align to next tab stop)
    {
        CxEditBuffer buffer;
        buffer.addCharacter('a');
        buffer.addCharacter('b');  // cursor at col 2
        buffer.addTab();  // should extend to col 4
        check(!cursorOnTabExtension(buffer), "cursor valid after tab from col 2");
        check(buffer.cursor.col == 4, "tab aligns to tab stop from col 2");
    }

    // Line with text and tab - cursor movement
    {
        CxEditBuffer buffer;
        buffer.addCharacter('h');
        buffer.addCharacter('i');
        buffer.addTab();  // tab from col 2 to col 4
        buffer.addCharacter('!');

        // Move cursor through the line
        buffer.cursorGotoRequest(0, 0);
        for (int i = 0; i < 10; i++) {
            check(!cursorOnTabExtension(buffer), "cursor never on tab ext during traversal");
            buffer.cursorRightRequest();
        }
    }
}

//-----------------------------------------------------------------------------------------
// Multi-line find and replace tests
//-----------------------------------------------------------------------------------------
void testMultiLineFind() {
    printf("\n== Multi-Line Find Tests ==\n");

    // find "dat\n" - segment at end of line with trailing newline
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("file.dat\nnextline");
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString(CxString("dat\n"));
        check(found, "find 'dat\\n' found in buffer");
        check(buffer.cursor.row == 0, "find 'dat\\n' cursor on row 0");
        check(buffer.cursor.col == 5, "find 'dat\\n' cursor at col 5 (start of 'dat')");
    }

    // find "\ntime" - empty first segment, match at start of next line
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("prev\ntimestamp");
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString(CxString("\ntime"));
        check(found, "find '\\ntime' found in buffer");
        check(buffer.cursor.row == 0, "find '\\ntime' cursor on row 0");
        check(buffer.cursor.col == 4, "find '\\ntime' cursor at col 4 (EOL of 'prev')");
    }

    // find "\n\n" - matches a blank line (EOL + empty line + next line boundary)
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("foo\n\nbar");
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString(CxString("\n\n"));
        check(found, "find '\\n\\n' found in buffer with blank line");
        check(buffer.cursor.row == 0, "find '\\n\\n' cursor on row 0");
        check(buffer.cursor.col == 3, "find '\\n\\n' cursor at col 3 (EOL of 'foo')");
    }

    // find "line1\nline2" - multi-word across two lines
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("aaa line1\nline2 bbb\nother");
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString(CxString("line1\nline2"));
        check(found, "find 'line1\\nline2' found across lines");
        check(buffer.cursor.row == 0, "find cross-line cursor on row 0");
        check(buffer.cursor.col == 4, "find cross-line cursor at start of 'line1'");
    }

    // find across three lines
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("header\nmiddle\nfooter\nend");
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString(CxString("header\nmiddle\nfooter"));
        check(found, "find across 3 lines found");
        check(buffer.cursor.row == 0, "find 3-line cursor on row 0");
        check(buffer.cursor.col == 0, "find 3-line cursor at col 0");
    }

    // find not found - segment doesn't match end of line
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("file.txt\nnextline");
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString(CxString("dat\n"));
        check(!found, "find 'dat\\n' not found when line ends with .txt");
    }

    // find not found - not enough lines in buffer
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("onlyline");
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString(CxString("only\n"));
        check(!found, "find with trailing newline not found in single-line buffer");
    }

    // find not found - middle segment doesn't match entire line
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("header\nmiddle extra\nfooter");
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString(CxString("header\nmiddle\nfooter"));
        check(!found, "find fails when middle segment doesn't match entire line");
    }

    // find not found - last segment doesn't match start of last line
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("header\nXfooter");
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString(CxString("header\nfooter"));
        check(!found, "find fails when last segment doesn't match start");
    }

    // findAgain skips current match
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("aa\nbb\naa\nbb");
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString(CxString("aa\nbb"));
        check(found, "first find 'aa\\nbb' found");
        check(buffer.cursor.row == 0, "first find at row 0");

        found = buffer.findAgain(CxString("aa\nbb"), TRUE);
        check(found, "findAgain 'aa\\nbb' finds second occurrence");
        check(buffer.cursor.row == 2, "findAgain at row 2");
    }

    // find with empty last segment at end of buffer - should not match
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("lastline");
        buffer.cursorGotoRequest(0, 0);
        int found = buffer.findString(CxString("lastline\n"));
        check(!found, "find 'lastline\\n' not found at end of buffer (no trailing newline)");
    }
}


void testMultiLineReplace() {
    printf("\n== Multi-Line Replace Tests ==\n");

    // Example A: "find: .dat\n" replace: "todd" - join lines
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("file.dat\nnextline");
        buffer.cursorGotoRequest(0, 0);
        buffer.findString(CxString(".dat\n"));
        buffer.replaceString(CxString(".dat\n"), CxString("todd"));
        check(buffer.numberOfLines() == 1, "replace joins lines: 1 line");
        check(strcmp(buffer.line(0)->data(), "filetoddnextline") == 0,
              "replace join result: 'filetoddnextline'");
    }

    // Example B: "find: \ntime" replace: "\ntodd" - preserve lines, replace start
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("prev\ntimestamp");
        buffer.cursorGotoRequest(0, 0);
        buffer.findString(CxString("\ntime"));
        buffer.replaceString(CxString("\ntime"), CxString("\ntodd"));
        check(buffer.numberOfLines() == 2, "replace preserves 2 lines");
        check(strcmp(buffer.line(0)->data(), "prev") == 0,
              "replace preserve first line: 'prev'");
        check(strcmp(buffer.line(1)->data(), "toddstamp") == 0,
              "replace preserve second line: 'toddstamp'");
    }

    // Example C: "find: \n\n" replace: "\n" - remove blank line
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("foo\n\nbar");
        buffer.cursorGotoRequest(0, 0);
        buffer.findString(CxString("\n\n"));
        buffer.replaceString(CxString("\n\n"), CxString("\n"));
        check(buffer.numberOfLines() == 2, "replace removes blank line: 2 lines");
        check(strcmp(buffer.line(0)->data(), "foo") == 0,
              "remove blank line first: 'foo'");
        check(strcmp(buffer.line(1)->data(), "bar") == 0,
              "remove blank line second: 'bar'");
    }

    // Replace with empty string - delete matched region (join lines)
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("hello\nworld");
        buffer.cursorGotoRequest(0, 0);
        buffer.findString(CxString("lo\nwor"));
        buffer.replaceString(CxString("lo\nwor"), CxString(""));
        check(buffer.numberOfLines() == 1, "empty replace joins: 1 line");
        check(strcmp(buffer.line(0)->data(), "helld") == 0,
              "empty replace result: 'helld'");
    }

    // Replace multi-line with longer multi-line
    // "aaa\nbbb" matches lines 0-1 entirely; "ccc" on line 2 is not part of the match
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("aaa\nbbb\nccc");
        buffer.cursorGotoRequest(0, 0);
        buffer.findString(CxString("aaa\nbbb"));
        buffer.replaceString(CxString("aaa\nbbb"), CxString("xxx\nyyy\nzzz"));
        check(buffer.numberOfLines() == 4, "replace expands: 4 lines");
        check(strcmp(buffer.line(0)->data(), "xxx") == 0, "expand first: 'xxx'");
        check(strcmp(buffer.line(1)->data(), "yyy") == 0, "expand middle: 'yyy'");
        check(strcmp(buffer.line(2)->data(), "zzz") == 0, "expand last: 'zzz'");
        check(strcmp(buffer.line(3)->data(), "ccc") == 0, "expand preserved: 'ccc'");
    }

    // Replace single-segment (join) preserving prefix and suffix
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("prefix_end\nstart_suffix");
        buffer.cursorGotoRequest(0, 0);
        buffer.findString(CxString("end\nstart"));
        buffer.replaceString(CxString("end\nstart"), CxString("JOINED"));
        check(buffer.numberOfLines() == 1, "join with prefix/suffix: 1 line");
        check(strcmp(buffer.line(0)->data(), "prefix_JOINED_suffix") == 0,
              "join result: 'prefix_JOINED_suffix'");
    }

    // replaceAgain - cursor not at match just does findAgain
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("aa\nbb\naa\nbb");
        buffer.cursorGotoRequest(0, 0);
        // cursor is at (0,0) which is not at a match for "aa\nbb" (would need to be at col 0 but
        // seg0="aa" must be at end of line, which it is since line is "aa")
        // Actually for "aa\nbb", seg0="aa" must match at end of line 0, and seg1="bb" at start of line 1
        // Line 0 is "aa" (length 2), so endPos = 2-2 = 0. matchCol = 0.
        // So cursor at (0,0) IS at the match. Let's adjust:
        buffer.cursorGotoRequest(0, 0);
        buffer.findString(CxString("aa\nbb"));
        // Now replace
        buffer.replaceString(CxString("aa\nbb"), CxString("XX\nYY"));
        check(strcmp(buffer.line(0)->data(), "XX") == 0, "replaceAgain first: 'XX'");
        check(strcmp(buffer.line(1)->data(), "YY") == 0, "replaceAgain second: 'YY'");
    }

    // Replace at various positions in the buffer (not at start)
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("keep\nfind_end\nfind_start\nkeep");
        buffer.cursorGotoRequest(0, 0);
        buffer.findString(CxString("end\nfind"));
        // cursor should be on row 1, at the 'e' of "end"
        check(buffer.cursor.row == 1, "find in middle: cursor row 1");
        buffer.replaceString(CxString("end\nfind"), CxString("REPLACED"));
        check(buffer.numberOfLines() == 3, "replace in middle: 3 lines");
        check(strcmp(buffer.line(0)->data(), "keep") == 0, "middle replace line 0 unchanged");
        check(strcmp(buffer.line(1)->data(), "find_REPLACED_start") == 0,
              "middle replace line 1: 'find_REPLACED_start'");
        check(strcmp(buffer.line(2)->data(), "keep") == 0, "middle replace line 2 unchanged");
    }

    // Replace multiple occurrences sequentially
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("a\nb\na\nb\na\nb");
        buffer.cursorGotoRequest(0, 0);
        buffer.findString(CxString("a\nb"));
        buffer.replaceString(CxString("a\nb"), CxString("X"));
        // After first replace: "X\na\nb\na\nb" -> cursor finds next match
        check(strcmp(buffer.line(0)->data(), "X") == 0, "sequential replace 1st line");
        // The findAgain should have found the next occurrence
    }

    // Trailing newline in find: "line\n" with single-line buffer after
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("first\nsecond\nthird");
        buffer.cursorGotoRequest(0, 0);
        buffer.findString(CxString("first\n"));
        buffer.replaceString(CxString("first\n"), CxString(""));
        check(strcmp(buffer.line(0)->data(), "second") == 0,
              "trailing newline replace removes first line");
        check(buffer.numberOfLines() == 2, "trailing newline replace: 2 lines remain");
    }
}


//-----------------------------------------------------------------------------------------
// Kill accumulation tests - consecutive Ctrl-K operations accumulate into kill buffer
//-----------------------------------------------------------------------------------------
void testKillAccumulation() {
    printf("\n== Kill Accumulation Tests ==\n");

    // Single kill - just cuts to end of line
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("hello world");
        buffer.cursorGotoRequest(0, 6);  // position after "hello "
        CxString cut = buffer.cutTextToEndOfLine();
        check(strcmp(cut.data(), "world") == 0, "single kill returns 'world'");
        check(strcmp(buffer.line(0)->data(), "hello ") == 0, "line after single kill");
    }

    // Two consecutive kills - first cuts text, second joins line
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("first\nsecond");
        buffer.cursorGotoRequest(0, 0);
        CxString cut1 = buffer.cutTextToEndOfLine();  // cuts "first"
        check(strcmp(cut1.data(), "first") == 0, "first kill: 'first'");
        CxString cut2 = buffer.cutTextToEndOfLine();  // joins with next line (kills newline)
        check(strcmp(cut2.data(), "first\n") == 0, "second kill accumulates: 'first\\n'");
        check(buffer.numberOfLines() == 1, "after two kills: 1 line");
    }

    // Three consecutive kills - full accumulation
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("AAA\nBBB\nCCC");
        buffer.cursorGotoRequest(0, 0);
        CxString cut1 = buffer.cutTextToEndOfLine();  // "AAA"
        CxString cut2 = buffer.cutTextToEndOfLine();  // newline (join)
        CxString cut3 = buffer.cutTextToEndOfLine();  // "BBB"
        check(strcmp(cut3.data(), "AAA\nBBB") == 0, "three kills accumulate: 'AAA\\nBBB'");
    }

    // Kill accumulation resets after cursor movement
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("first\nsecond\nthird");
        buffer.cursorGotoRequest(0, 0);
        CxString cut1 = buffer.cutTextToEndOfLine();  // "first"
        buffer.cursorRightRequest();  // move cursor - resets accumulation
        buffer.cursorGotoRequest(1, 0);  // move to second line
        CxString cut2 = buffer.cutTextToEndOfLine();  // "second" - fresh start
        check(strcmp(cut2.data(), "second") == 0, "kill after cursor move: 'second' (not accumulated)");
    }

    // Kill accumulation resets after character insertion
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("hello\nworld");
        buffer.cursorGotoRequest(0, 0);
        CxString cut1 = buffer.cutTextToEndOfLine();  // "hello"
        buffer.addCharacter('X');  // insert char - resets accumulation
        buffer.cursorGotoRequest(1, 0);
        CxString cut2 = buffer.cutTextToEndOfLine();  // "world" - fresh start
        check(strcmp(cut2.data(), "world") == 0, "kill after insert: 'world' (not accumulated)");
    }

    // Kill on empty line joins with next
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("first\n\nthird");  // empty line in middle
        buffer.cursorGotoRequest(1, 0);  // on empty line
        CxString cut = buffer.cutTextToEndOfLine();  // should join
        check(strcmp(cut.data(), "\n") == 0, "kill empty line: newline");
        check(buffer.numberOfLines() == 2, "after kill empty: 2 lines");
        check(strcmp(buffer.line(1)->data(), "third") == 0, "line 1 is 'third'");
    }

    // Multiple kills then paste restores original
    {
        CxEditBuffer buffer;
        buffer.loadTextFromString("line1\nline2\nline3");
        buffer.cursorGotoRequest(0, 0);
        buffer.cutTextToEndOfLine();  // "line1"
        buffer.cutTextToEndOfLine();  // "\n"
        buffer.cutTextToEndOfLine();  // "line2"
        CxString accumulated = buffer.cutTextToEndOfLine();  // "\n"
        // accumulated should be "line1\nline2\n"
        check(strcmp(accumulated.data(), "line1\nline2\n") == 0, "four kills: 'line1\\nline2\\n'");
        // Now paste it back
        buffer.pasteFromCutBuffer(accumulated);
        check(buffer.numberOfLines() == 3, "after paste: 3 lines");
    }
}


//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxEditBuffer Test Suite\n");
    printf("=======================\n");

    testConstructors();
    testCharacterInsertion();
    testBackspace();
    testCursorMovement();
    testLineAccess();
    testMarkAndCut();
    testInsertAndPaste();
    testFindAndReplace();
    testMultiLineFind();
    testMultiLineReplace();
    testLoadFromString();
    testStatusMethods();
    testPositionEvaluation();
    testTabHandling();
    testKillAccumulation();

    printf("\n=======================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
