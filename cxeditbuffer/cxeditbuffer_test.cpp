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
    testLoadFromString();
    testStatusMethods();
    testPositionEvaluation();
    testTabHandling();

    printf("\n=======================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
