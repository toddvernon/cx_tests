//-----------------------------------------------------------------------------------------
// cxscreen_test.cpp - CxScreen, CxCursor, and CxColor unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <cx/base/string.h>
#include <cx/screen/screen.h>
#include <cx/screen/cursor.h>
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

//-----------------------------------------------------------------------------------------
// CxCursor tests
//-----------------------------------------------------------------------------------------
void testCursor() {
    printf("\n== CxCursor Tests ==\n");

    // Constructor
    {
        CxCursor cursor;
        check(1, "CxCursor ctor");
    }

    // locateTerminalString - positions cursor at row,col (0-based input, 1-based output)
    {
        CxString s = CxCursor::locateTerminalString(0, 0);
        check(s == "\033[1;1H", "locateTerminalString(0,0) -> ESC[1;1H");
    }

    {
        CxString s = CxCursor::locateTerminalString(9, 19);
        check(s == "\033[10;20H", "locateTerminalString(9,19) -> ESC[10;20H");
    }

    // showTerminalString
    {
        CxString s = CxCursor::showTerminalString();
        check(s == "\033[?25h", "showTerminalString -> ESC[?25h");
    }

    // hideTerminalString
    {
        CxString s = CxCursor::hideTerminalString();
        check(s == "\033[?25l", "hideTerminalString -> ESC[?25l");
    }

    // moveLeftTerminalString
    {
        CxString s = CxCursor::moveLeftTerminalString(5);
        check(s == "\033[5D", "moveLeftTerminalString(5) -> ESC[5D");
    }

    // moveRightTerminalString
    {
        CxString s = CxCursor::moveRightTerminalString(3);
        check(s == "\033[3C", "moveRightTerminalString(3) -> ESC[3C");
    }

    // moveToColumnTerminalString (0-based input, 1-based output)
    {
        CxString s = CxCursor::moveToColumnTerminalString(0);
        check(s == "\033[1G", "moveToColumnTerminalString(0) -> ESC[1G");
    }

    {
        CxString s = CxCursor::moveToColumnTerminalString(79);
        check(s == "\033[80G", "moveToColumnTerminalString(79) -> ESC[80G");
    }

    // clearToEndOfLineTerminalString
    {
        CxString s = CxCursor::clearToEndOfLineTerminalString();
        check(s == "\033[K", "clearToEndOfLineTerminalString -> ESC[K");
    }

    // clearFromPositionDownTerminalString
    {
        CxString s = CxCursor::clearFromPositionDownTerminalString();
        check(s == "\033[J", "clearFromPositionDownTerminalString -> ESC[J");
    }

    // savePositionTerminalString
    {
        CxString s = CxCursor::savePositionTerminalString();
        check(s == "\033[s", "savePositionTerminalString -> ESC[s");
    }

    // restorePositionTerminalString
    {
        CxString s = CxCursor::restorePositionTerminalString();
        check(s == "\033[u", "restorePositionTerminalString -> ESC[u");
    }
}

//-----------------------------------------------------------------------------------------
// CxColor base class tests
//-----------------------------------------------------------------------------------------
void testColorBase() {
    printf("\n== CxColor Base Tests ==\n");

    // Constructor
    {
        CxColor color;
        check(color.getColorType() == BASE, "CxColor ctor: type is BASE");
    }

    // terminalString (base returns empty)
    {
        CxColor color;
        CxString s = color.terminalString();
        check(s.length() == 0, "CxColor terminalString is empty");
    }
}

//-----------------------------------------------------------------------------------------
// CxRGBColor tests
//-----------------------------------------------------------------------------------------
void testRGBColor() {
    printf("\n== CxRGBColor Tests ==\n");

    // Default constructor
    {
        CxRGBColor color;
        check(color.getColorType() == RGB_BASE, "CxRGBColor default: type is RGB_BASE");
    }

    // Constructor with RGB values
    {
        CxRGBColor color(255, 128, 0);
        check(color.red() == 255, "CxRGBColor(255,128,0): red is 255");
        check(color.green() == 128, "CxRGBColor(255,128,0): green is 128");
        check(color.blue() == 0, "CxRGBColor(255,128,0): blue is 0");
    }

    // Copy constructor
    {
        CxRGBColor original(100, 150, 200);
        CxRGBColor copy(original);
        check(copy.red() == 100 && copy.green() == 150 && copy.blue() == 200,
              "CxRGBColor copy ctor preserves values");
    }

    // Assignment operator
    {
        CxRGBColor original(50, 100, 150);
        CxRGBColor assigned;
        assigned = original;
        check(assigned.red() == 50 && assigned.green() == 100 && assigned.blue() == 150,
              "CxRGBColor assignment preserves values");
    }
}

//-----------------------------------------------------------------------------------------
// CxRGBForegroundColor tests
//-----------------------------------------------------------------------------------------
void testRGBForegroundColor() {
    printf("\n== CxRGBForegroundColor Tests ==\n");

    // Default constructor
    {
        CxRGBForegroundColor color;
        check(color.getColorType() == RGB_FOREGROUND, "type is RGB_FOREGROUND");
    }

    // Constructor with RGB values
    {
        CxRGBForegroundColor color(255, 0, 0);
        check(color.red() == 255 && color.green() == 0 && color.blue() == 0,
              "CxRGBForegroundColor(255,0,0): RGB values correct");
    }

    // terminalString format: ESC[38;2;R;G;Bm
    {
        CxRGBForegroundColor color(255, 128, 64);
        CxString s = color.terminalString();
        check(s == "\033[38;2;255;128;64m", "terminalString -> ESC[38;2;255;128;64m");
    }

    // resetTerminalString
    {
        CxRGBForegroundColor color(255, 128, 64);
        CxString s = color.resetTerminalString();
        check(s == "\033[39m", "resetTerminalString -> ESC[39m");
    }
}

//-----------------------------------------------------------------------------------------
// CxRGBBackgroundColor tests
//-----------------------------------------------------------------------------------------
void testRGBBackgroundColor() {
    printf("\n== CxRGBBackgroundColor Tests ==\n");

    // Default constructor
    {
        CxRGBBackgroundColor color;
        check(color.getColorType() == RGB_BACKGROUND, "type is RGB_BACKGROUND");
    }

    // terminalString format: ESC[48;2;R;G;Bm
    {
        CxRGBBackgroundColor color(0, 255, 128);
        CxString s = color.terminalString();
        check(s == "\033[48;2;0;255;128m", "terminalString -> ESC[48;2;0;255;128m");
    }

    // resetTerminalString
    {
        CxRGBBackgroundColor color(0, 255, 128);
        CxString s = color.resetTerminalString();
        check(s == "\033[49m", "resetTerminalString -> ESC[49m");
    }
}

//-----------------------------------------------------------------------------------------
// CxAnsiColor tests
//-----------------------------------------------------------------------------------------
void testAnsiColor() {
    printf("\n== CxAnsiColor Tests ==\n");

    // Base class
    {
        CxAnsiColor color;
        check(color.getColorType() == ANSI_BASE, "CxAnsiColor: type is ANSI_BASE");
    }
}

//-----------------------------------------------------------------------------------------
// CxAnsiForegroundColor tests
//-----------------------------------------------------------------------------------------
void testAnsiForegroundColor() {
    printf("\n== CxAnsiForegroundColor Tests ==\n");

    // Default constructor
    {
        CxAnsiForegroundColor color;
        check(color.getColorType() == ANSI_FOREGROUND, "type is ANSI_FOREGROUND");
    }

    // Constructor with color name
    {
        CxAnsiForegroundColor color("red");
        check(color.getColorType() == ANSI_FOREGROUND, "CxAnsiForegroundColor(red): type correct");
    }

    // Copy constructor
    {
        CxAnsiForegroundColor original("blue");
        CxAnsiForegroundColor copy(original);
        check(copy.getColorType() == ANSI_FOREGROUND, "copy ctor: type preserved");
    }

    // terminalString returns non-empty for valid color
    {
        CxAnsiForegroundColor color("red");
        CxString s = color.terminalString();
        check(s.length() > 0, "terminalString for 'red' is non-empty");
    }

    // resetTerminalString
    {
        CxAnsiForegroundColor color("red");
        CxString s = color.resetTerminalString();
        check(s == "\033[39m", "resetTerminalString -> ESC[39m");
    }
}

//-----------------------------------------------------------------------------------------
// CxAnsiBackgroundColor tests
//-----------------------------------------------------------------------------------------
void testAnsiBackgroundColor() {
    printf("\n== CxAnsiBackgroundColor Tests ==\n");

    // Default constructor
    {
        CxAnsiBackgroundColor color;
        check(color.getColorType() == ANSI_BACKGROUND, "type is ANSI_BACKGROUND");
    }

    // Constructor with color name
    {
        CxAnsiBackgroundColor color("blue");
        check(color.getColorType() == ANSI_BACKGROUND, "CxAnsiBackgroundColor(blue): type correct");
    }

    // resetTerminalString
    {
        CxAnsiBackgroundColor color("blue");
        CxString s = color.resetTerminalString();
        check(s == "\033[49m", "resetTerminalString -> ESC[49m");
    }
}

//-----------------------------------------------------------------------------------------
// CxXterm256Color tests
//-----------------------------------------------------------------------------------------
void testXterm256Color() {
    printf("\n== CxXterm256Color Tests ==\n");

    // Base class
    {
        CxXterm256Color color;
        check(color.getColorType() == XTERM256_BASE, "CxXterm256Color: type is XTERM256_BASE");
    }
}

//-----------------------------------------------------------------------------------------
// CxXterm256ForegroundColor tests
//-----------------------------------------------------------------------------------------
void testXterm256ForegroundColor() {
    printf("\n== CxXterm256ForegroundColor Tests ==\n");

    // Default constructor
    {
        CxXterm256ForegroundColor color;
        check(color.getColorType() == XTERM256_FOREGROUND, "type is XTERM256_FOREGROUND");
    }

    // Constructor with color name
    {
        CxXterm256ForegroundColor color("Red");
        check(color.getColorType() == XTERM256_FOREGROUND, "CxXterm256ForegroundColor(Red): type correct");
    }

    // Copy constructor
    {
        CxXterm256ForegroundColor original("Blue");
        CxXterm256ForegroundColor copy(original);
        check(copy.getColorType() == XTERM256_FOREGROUND, "copy ctor: type preserved");
    }

    // terminalString format: ESC[38;5;Nm
    {
        CxXterm256ForegroundColor color("Red");
        CxString s = color.terminalString();
        // Red is index 9 in xterm-256
        check(s.length() > 0, "terminalString for 'Red' is non-empty");
    }

    // resetTerminalString
    {
        CxXterm256ForegroundColor color("Red");
        CxString s = color.resetTerminalString();
        check(s == "\033[39m", "resetTerminalString -> ESC[39m");
    }
}

//-----------------------------------------------------------------------------------------
// CxXterm256BackgroundColor tests
//-----------------------------------------------------------------------------------------
void testXterm256BackgroundColor() {
    printf("\n== CxXterm256BackgroundColor Tests ==\n");

    // Default constructor
    {
        CxXterm256BackgroundColor color;
        check(color.getColorType() == XTERM256_BACKGROUND, "type is XTERM256_BACKGROUND");
    }

    // Constructor with color name
    {
        CxXterm256BackgroundColor color("Green");
        check(color.getColorType() == XTERM256_BACKGROUND, "CxXterm256BackgroundColor(Green): type correct");
    }

    // resetTerminalString
    {
        CxXterm256BackgroundColor color("Green");
        CxString s = color.resetTerminalString();
        check(s == "\033[49m", "resetTerminalString -> ESC[49m");
    }
}

//-----------------------------------------------------------------------------------------
// CxScreen tests (static methods that don't require terminal)
//-----------------------------------------------------------------------------------------
void testScreen() {
    printf("\n== CxScreen Tests ==\n");

    // Constructor - just verify it doesn't crash
    // Note: CxScreen constructor sets up signal handlers, so we test carefully
    {
        // We can't easily test CxScreen without a terminal, so just verify
        // the static row/col methods work (they read from static winsize)
        int r = CxScreen::rows();
        int c = CxScreen::cols();
        check(r >= 0, "CxScreen::rows() returns non-negative");
        check(c >= 0, "CxScreen::cols() returns non-negative");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxScreen Test Suite\n");
    printf("===================\n");

    // Cursor tests
    testCursor();

    // Color base tests
    testColorBase();

    // RGB color tests
    testRGBColor();
    testRGBForegroundColor();
    testRGBBackgroundColor();

    // ANSI color tests
    testAnsiColor();
    testAnsiForegroundColor();
    testAnsiBackgroundColor();

    // Xterm256 color tests
    testXterm256Color();
    testXterm256ForegroundColor();
    testXterm256BackgroundColor();

    // Screen tests
    testScreen();

    printf("\n===================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
