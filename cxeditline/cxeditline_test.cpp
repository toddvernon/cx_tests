//-----------------------------------------------------------------------------------------
// cxeditline_test.cpp - CxEditLine unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <cx/base/string.h>
#include <cx/editbuffer/editline.h>
#include <cx/editbuffer/edithint.h>

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
void testConstructor() {
    printf("\n== Constructor Tests ==\n");

    // Default constructor
    {
        CxEditLine line(80);
        check(line.cursorCol() == 0, "ctor: cursor col is 0");
        check(line.cursorRow() == 0, "ctor: cursor row is 0");
        check(line.text().length() == 0, "ctor: text is empty");
    }

    // Different edit widths
    {
        CxEditLine line1(40);
        CxEditLine line2(120);
        check(line1.text().length() == 0, "ctor with width 40: text empty");
        check(line2.text().length() == 0, "ctor with width 120: text empty");
    }
}

//-----------------------------------------------------------------------------------------
// Text manipulation tests
//-----------------------------------------------------------------------------------------
void testTextManipulation() {
    printf("\n== Text Manipulation Tests ==\n");

    // text() returns empty initially
    {
        CxEditLine line(80);
        CxString t = line.text();
        check(t.length() == 0, "text() returns empty string initially");
    }

    // appendText adds characters
    {
        CxEditLine line(80);
        line.appendText("hello");
        check(strcmp(line.text().data(), "hello") == 0, "appendText adds text");
        check(line.cursorCol() == 5, "appendText advances cursor");
    }

    // appendText multiple times
    {
        CxEditLine line(80);
        line.appendText("hello");
        line.appendText(" world");
        check(strcmp(line.text().data(), "hello world") == 0, "appendText concatenates");
        check(line.cursorCol() == 11, "cursor at end after multiple appendText");
    }

    // setText clears the line
    {
        CxEditLine line(80);
        line.appendText("hello");
        line.setText("ignored");  // Note: setText ignores parameter, just clears
        check(line.text().length() == 0, "setText clears text");
        check(line.cursorCol() == 0, "setText resets cursor to 0");
    }
}

//-----------------------------------------------------------------------------------------
// Character insertion tests
//-----------------------------------------------------------------------------------------
void testCharacterInsertion() {
    printf("\n== Character Insertion Tests ==\n");

    // Add single character
    {
        CxEditLine line(80);
        CxEditHint hint = line.addCharacter('a');
        check(strcmp(line.text().data(), "a") == 0, "addCharacter(char) adds char");
        check(line.cursorCol() == 1, "cursor advances after addCharacter");
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_RIGHT, "hint indicates cursor right");
    }

    // Add multiple characters
    {
        CxEditLine line(80);
        line.addCharacter('h');
        line.addCharacter('e');
        line.addCharacter('l');
        line.addCharacter('l');
        line.addCharacter('o');
        check(strcmp(line.text().data(), "hello") == 0, "multiple addCharacter builds string");
        check(line.cursorCol() == 5, "cursor col is 5 after 5 chars");
    }

    // Add character via CxString
    {
        CxEditLine line(80);
        CxEditHint hint = line.addCharacter(CxString("x"));
        check(strcmp(line.text().data(), "x") == 0, "addCharacter(CxString) works");
        check(line.cursorCol() == 1, "cursor advances with CxString version");
    }

    // Insert character in middle
    {
        CxEditLine line(80);
        line.appendText("hllo");
        // Move cursor left to position 1
        line.cursorLeftRequest();
        line.cursorLeftRequest();
        line.cursorLeftRequest();
        check(line.cursorCol() == 1, "cursor at position 1");
        line.addCharacter('e');
        check(strcmp(line.text().data(), "hello") == 0, "insert in middle works");
        check(line.cursorCol() == 2, "cursor advances after insert");
    }
}

//-----------------------------------------------------------------------------------------
// Cursor movement tests
//-----------------------------------------------------------------------------------------
void testCursorMovement() {
    printf("\n== Cursor Movement Tests ==\n");

    // Cursor right
    {
        CxEditLine line(80);
        line.appendText("hello");
        // Cursor is at end (col 5), move to beginning
        line.cursorLeftRequest();
        line.cursorLeftRequest();
        line.cursorLeftRequest();
        line.cursorLeftRequest();
        line.cursorLeftRequest();
        check(line.cursorCol() == 0, "cursor at beginning after 5 lefts");

        CxEditHint hint = line.cursorRightRequest();
        check(line.cursorCol() == 1, "cursorRightRequest moves right");
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_RIGHT, "hint is CURSOR_HINT_RIGHT");
    }

    // Cursor right at end of line (should stop)
    {
        CxEditLine line(80);
        line.appendText("abc");
        check(line.cursorCol() == 3, "cursor at end");
        CxEditHint hint = line.cursorRightRequest();
        check(line.cursorCol() == 3, "cursor stays at end");
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_NONE, "hint is NONE at end");
    }

    // Cursor left
    {
        CxEditLine line(80);
        line.appendText("hello");
        CxEditHint hint = line.cursorLeftRequest();
        check(line.cursorCol() == 4, "cursorLeftRequest moves left");
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_LEFT, "hint is CURSOR_HINT_LEFT");
    }

    // Cursor left at beginning (should stop)
    {
        CxEditLine line(80);
        CxEditHint hint = line.cursorLeftRequest();
        check(line.cursorCol() == 0, "cursor stays at 0");
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_NONE, "hint is NONE at beginning");
    }

    // Cursor up (no-op for single line)
    {
        CxEditLine line(80);
        line.appendText("hello");
        unsigned long colBefore = line.cursorCol();
        CxEditHint hint = line.cursorUpRequest();
        check(line.cursorCol() == colBefore, "cursorUpRequest is no-op");
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_NONE, "cursorUp hint is NONE");
    }

    // Cursor down (no-op for single line)
    {
        CxEditLine line(80);
        line.appendText("hello");
        unsigned long colBefore = line.cursorCol();
        CxEditHint hint = line.cursorDownRequest();
        check(line.cursorCol() == colBefore, "cursorDownRequest is no-op");
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_NONE, "cursorDown hint is NONE");
    }
}

//-----------------------------------------------------------------------------------------
// Backspace tests
//-----------------------------------------------------------------------------------------
void testBackspace() {
    printf("\n== Backspace Tests ==\n");

    // Backspace at end of text
    {
        CxEditLine line(80);
        line.appendText("hello");
        CxEditHint hint = line.addBackspace();
        check(strcmp(line.text().data(), "hell") == 0, "backspace removes last char");
        check(line.cursorCol() == 4, "cursor moves back after backspace");
        check(hint.updateHint() == CxEditHint::UPDATE_HINT_LINE_PAST_POINT, "hint indicates line update");
    }

    // Multiple backspaces
    {
        CxEditLine line(80);
        line.appendText("hello");
        line.addBackspace();
        line.addBackspace();
        line.addBackspace();
        check(strcmp(line.text().data(), "he") == 0, "multiple backspaces work");
        check(line.cursorCol() == 2, "cursor at correct position");
    }

    // Backspace at beginning (no-op)
    {
        CxEditLine line(80);
        CxEditHint hint = line.addBackspace();
        check(line.cursorCol() == 0, "backspace at beginning doesn't move cursor");
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_NONE, "hint is NONE for backspace at start");
    }

    // Backspace in middle of text
    {
        CxEditLine line(80);
        line.appendText("hello");
        line.cursorLeftRequest();
        line.cursorLeftRequest();  // cursor at position 3 (between 'l' and 'l')
        line.addBackspace();       // should delete the first 'l'
        check(strcmp(line.text().data(), "helo") == 0, "backspace in middle works");
        check(line.cursorCol() == 2, "cursor at correct position after middle backspace");
    }
}

//-----------------------------------------------------------------------------------------
// Position evaluation tests
//-----------------------------------------------------------------------------------------
void testPositionEvaluation() {
    printf("\n== Position Evaluation Tests ==\n");

    // Empty buffer - position 0 is append
    {
        CxEditLine line(80);
        CxEditLine::POSITION pos = line.evaluatePosition(0);
        check(pos == CxEditLine::POS_VALID_APPEND_COL, "empty buffer pos 0 is APPEND");
    }

    // With text - position at start is insert
    {
        CxEditLine line(80);
        line.appendText("hello");
        CxEditLine::POSITION pos = line.evaluatePosition(0);
        check(pos == CxEditLine::POS_VALID_INSERT, "pos 0 with text is INSERT");
    }

    // With text - position in middle is insert
    {
        CxEditLine line(80);
        line.appendText("hello");
        CxEditLine::POSITION pos = line.evaluatePosition(2);
        check(pos == CxEditLine::POS_VALID_INSERT, "pos 2 with text is INSERT");
    }

    // With text - position at end is append
    {
        CxEditLine line(80);
        line.appendText("hello");
        CxEditLine::POSITION pos = line.evaluatePosition(5);
        check(pos == CxEditLine::POS_VALID_APPEND_COL, "pos at length is APPEND");
    }

    // With text - position past end is invalid
    {
        CxEditLine line(80);
        line.appendText("hello");
        CxEditLine::POSITION pos = line.evaluatePosition(10);
        check(pos == CxEditLine::POS_INVALID_COL, "pos past length is INVALID");
    }
}

//-----------------------------------------------------------------------------------------
// Tab and Return tests (currently no-ops)
//-----------------------------------------------------------------------------------------
void testTabAndReturn() {
    printf("\n== Tab and Return Tests ==\n");

    // addTab is currently a no-op
    {
        CxEditLine line(80);
        line.appendText("hello");
        unsigned long colBefore = line.cursorCol();
        CxString textBefore = line.text();
        CxEditHint hint = line.addTab();
        check(line.cursorCol() == colBefore, "addTab doesn't change cursor (no-op)");
        check(strcmp(line.text().data(), textBefore.data()) == 0, "addTab doesn't change text (no-op)");
        check(hint.updateHint() == CxEditHint::UPDATE_HINT_NONE, "addTab hint is NONE");
    }

    // addReturn is currently a no-op
    {
        CxEditLine line(80);
        line.appendText("hello");
        unsigned long colBefore = line.cursorCol();
        CxString textBefore = line.text();
        CxEditHint hint = line.addReturn();
        check(line.cursorCol() == colBefore, "addReturn doesn't change cursor (no-op)");
        check(strcmp(line.text().data(), textBefore.data()) == 0, "addReturn doesn't change text (no-op)");
        check(hint.updateHint() == CxEditHint::UPDATE_HINT_NONE, "addReturn hint is NONE");
    }
}

//-----------------------------------------------------------------------------------------
// Edit hint tests
//-----------------------------------------------------------------------------------------
void testEditHints() {
    printf("\n== Edit Hint Tests ==\n");

    // Character insertion hint
    {
        CxEditLine line(80);
        CxEditHint hint = line.addCharacter('a');
        check(hint.updateHint() == CxEditHint::UPDATE_HINT_LINE_PAST_POINT, "addChar returns LINE_PAST_POINT");
        check(hint.cursorHint() == CxEditHint::CURSOR_HINT_RIGHT, "addChar returns CURSOR_RIGHT");
    }

    // Backspace hint
    {
        CxEditLine line(80);
        line.appendText("ab");
        CxEditHint hint = line.addBackspace();
        check(hint.updateHint() == CxEditHint::UPDATE_HINT_LINE_PAST_POINT, "backspace returns LINE_PAST_POINT");
    }

    // Cursor movement hints
    {
        CxEditLine line(80);
        line.appendText("hello");

        CxEditHint leftHint = line.cursorLeftRequest();
        check(leftHint.updateHint() == CxEditHint::UPDATE_HINT_NONE, "cursorLeft returns UPDATE_NONE");

        CxEditHint rightHint = line.cursorRightRequest();
        check(rightHint.updateHint() == CxEditHint::UPDATE_HINT_NONE, "cursorRight returns UPDATE_NONE");
    }
}

//-----------------------------------------------------------------------------------------
// Position string tests
//-----------------------------------------------------------------------------------------
void testPositionString() {
    printf("\n== Position String Tests ==\n");

    CxEditLine line(80);

    CxString insertStr = line.positionString(CxEditLine::POS_VALID_INSERT);
    check(strcmp(insertStr.data(), "POS_VALID_INSERT") == 0, "positionString INSERT");

    CxString appendStr = line.positionString(CxEditLine::POS_VALID_APPEND_COL);
    check(strcmp(appendStr.data(), "POS_VALID_APPEND_COL") == 0, "positionString APPEND");

    CxString invalidStr = line.positionString(CxEditLine::POS_INVALID_COL);
    check(strcmp(invalidStr.data(), "POS_INVALID_COL") == 0, "positionString INVALID");
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    printf("CxEditLine Unit Tests\n");
    printf("=====================\n");

    testConstructor();
    testTextManipulation();
    testCharacterInsertion();
    testCursorMovement();
    testBackspace();
    testPositionEvaluation();
    testTabAndReturn();
    testEditHints();
    testPositionString();

    printf("\n=====================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);
    printf("=====================\n");

    return testsFailed > 0 ? 1 : 0;
}
