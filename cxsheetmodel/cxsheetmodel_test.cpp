//-----------------------------------------------------------------------------------------
// cxsheetmodel_test.cpp - CxSheetModel and related classes unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <cx/base/string.h>
#include <cx/base/double.h>
#include <cx/sheetModel/sheetCellCoordinate.h>
#include <cx/sheetModel/sheetCellRange.h>
#include <cx/sheetModel/sheetCell.h>
#include <cx/sheetModel/sheetModel.h>
#include <cx/sheetModel/sheetInputParser.h>
#include <cx/json/json_utf_object.h>
#include <cx/json/json_utf_member.h>
#include <cx/json/json_utf_number.h>

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

// Helper to compare doubles with tolerance
int doubleEqual(double a, double b, double tolerance = 0.0000001) {
    if (a == b) return 1;
    double diff = a - b;
    if (diff < 0) diff = -diff;
    return diff < tolerance;
}

//-----------------------------------------------------------------------------------------
// CxSheetCellCoordinate tests
//-----------------------------------------------------------------------------------------
void testCoordinateConstructors() {
    printf("\n== CxSheetCellCoordinate Constructor Tests ==\n");

    // Default constructor
    {
        CxSheetCellCoordinate coord;
        check(coord.getRow() == 0, "default ctor: row is 0");
        check(coord.getCol() == 0, "default ctor: col is 0");
        check(coord.isRowAbsolute() == 0, "default ctor: row not absolute");
        check(coord.isColAbsolute() == 0, "default ctor: col not absolute");
    }

    // Numeric constructor
    {
        CxSheetCellCoordinate coord(5, 10);
        check(coord.getRow() == 5, "numeric ctor: row is 5");
        check(coord.getCol() == 10, "numeric ctor: col is 10");
    }

    // Copy constructor
    {
        CxSheetCellCoordinate c1(3, 7);
        c1.setRowAbsolute(1);
        CxSheetCellCoordinate c2(c1);
        check(c2.getRow() == 3, "copy ctor: row");
        check(c2.getCol() == 7, "copy ctor: col");
        check(c2.isRowAbsolute() == 1, "copy ctor: row absolute");
    }

    // Assignment operator
    {
        CxSheetCellCoordinate c1(2, 4);
        CxSheetCellCoordinate c2;
        c2 = c1;
        check(c2.getRow() == 2, "assignment: row");
        check(c2.getCol() == 4, "assignment: col");
    }
}

void testCoordinateColumnLetters() {
    printf("\n== CxSheetCellCoordinate Column Letter Conversion Tests ==\n");

    CxSheetCellCoordinate coord;

    // colToLetters tests
    check(coord.colToLetters(0) == "A", "colToLetters: 0 = A");
    check(coord.colToLetters(1) == "B", "colToLetters: 1 = B");
    check(coord.colToLetters(25) == "Z", "colToLetters: 25 = Z");
    check(coord.colToLetters(26) == "AA", "colToLetters: 26 = AA");
    check(coord.colToLetters(27) == "AB", "colToLetters: 27 = AB");
    check(coord.colToLetters(51) == "AZ", "colToLetters: 51 = AZ");
    check(coord.colToLetters(52) == "BA", "colToLetters: 52 = BA");
    check(coord.colToLetters(701) == "ZZ", "colToLetters: 701 = ZZ");
    check(coord.colToLetters(702) == "AAA", "colToLetters: 702 = AAA");

    // lettersToCol tests
    check(coord.lettersToCol("A") == 0, "lettersToCol: A = 0");
    check(coord.lettersToCol("B") == 1, "lettersToCol: B = 1");
    check(coord.lettersToCol("Z") == 25, "lettersToCol: Z = 25");
    check(coord.lettersToCol("AA") == 26, "lettersToCol: AA = 26");
    check(coord.lettersToCol("AB") == 27, "lettersToCol: AB = 27");
    check(coord.lettersToCol("AZ") == 51, "lettersToCol: AZ = 51");
    check(coord.lettersToCol("BA") == 52, "lettersToCol: BA = 52");
    check(coord.lettersToCol("ZZ") == 701, "lettersToCol: ZZ = 701");
    check(coord.lettersToCol("AAA") == 702, "lettersToCol: AAA = 702");

    // Lowercase should work too
    check(coord.lettersToCol("a") == 0, "lettersToCol: a = 0 (lowercase)");
    check(coord.lettersToCol("aa") == 26, "lettersToCol: aa = 26 (lowercase)");
}

void testCoordinateAddressParsing() {
    printf("\n== CxSheetCellCoordinate Address Parsing Tests ==\n");

    // Simple address
    {
        CxSheetCellCoordinate coord("A1");
        check(coord.getCol() == 0, "parse A1 - col is 0");
        check(coord.getRow() == 0, "parse A1 - row is 0");
        check(coord.isColAbsolute() == 0, "parse A1 - col not absolute");
        check(coord.isRowAbsolute() == 0, "parse A1 - row not absolute");
    }

    // Larger address
    {
        CxSheetCellCoordinate coord("C6");
        check(coord.getCol() == 2, "parse C6 - col is 2");
        check(coord.getRow() == 5, "parse C6 - row is 5");
    }

    // Double letter column
    {
        CxSheetCellCoordinate coord("AA100");
        check(coord.getCol() == 26, "parse AA100 - col is 26");
        check(coord.getRow() == 99, "parse AA100 - row is 99");
    }

    // Absolute column
    {
        CxSheetCellCoordinate coord("$B5");
        check(coord.getCol() == 1, "parse $B5 - col is 1");
        check(coord.getRow() == 4, "parse $B5 - row is 4");
        check(coord.isColAbsolute() == 1, "parse $B5 - col is absolute");
        check(coord.isRowAbsolute() == 0, "parse $B5 - row not absolute");
    }

    // Absolute row
    {
        CxSheetCellCoordinate coord("B$5");
        check(coord.getCol() == 1, "parse B$5 - col is 1");
        check(coord.getRow() == 4, "parse B$5 - row is 4");
        check(coord.isColAbsolute() == 0, "parse B$5 - col not absolute");
        check(coord.isRowAbsolute() == 1, "parse B$5 - row is absolute");
    }

    // Both absolute
    {
        CxSheetCellCoordinate coord("$C$6");
        check(coord.getCol() == 2, "parse $C$6 - col is 2");
        check(coord.getRow() == 5, "parse $C$6 - row is 5");
        check(coord.isColAbsolute() == 1, "parse $C$6 - col is absolute");
        check(coord.isRowAbsolute() == 1, "parse $C$6 - row is absolute");
    }
}

void testCoordinateToAddress() {
    printf("\n== CxSheetCellCoordinate toAddress Tests ==\n");

    // Simple address
    {
        CxSheetCellCoordinate coord(0, 0);
        check(coord.toAddress() == "A1", "toAddress: (0,0) = A1");
    }

    // Larger row/col
    {
        CxSheetCellCoordinate coord(99, 26);
        check(coord.toAddress() == "AA100", "toAddress: (99,26) = AA100");
    }

    // Absolute address
    {
        CxSheetCellCoordinate coord(5, 2);
        coord.setColAbsolute(1);
        coord.setRowAbsolute(1);
        check(coord.toAbsoluteAddress() == "$C$6", "toAbsoluteAddress: $C$6");
    }
}

void testCoordinateHashAndEquality() {
    printf("\n== CxSheetCellCoordinate Hash and Equality Tests ==\n");

    // Equality
    {
        CxSheetCellCoordinate c1(5, 10);
        CxSheetCellCoordinate c2(5, 10);
        check(c1 == c2, "equality: same coordinates");
    }

    // Inequality - different row
    {
        CxSheetCellCoordinate c1(5, 10);
        CxSheetCellCoordinate c2(6, 10);
        check(!(c1 == c2), "inequality: different row");
    }

    // Inequality - different col
    {
        CxSheetCellCoordinate c1(5, 10);
        CxSheetCellCoordinate c2(5, 11);
        check(!(c1 == c2), "inequality: different col");
    }

    // Hash values for same coordinates should be equal
    {
        CxSheetCellCoordinate c1(5, 10);
        CxSheetCellCoordinate c2(5, 10);
        check(c1.hashValue() == c2.hashValue(), "hash: same coords same hash");
    }

    // Hash values for different coordinates should (usually) be different
    {
        CxSheetCellCoordinate c1(0, 0);
        CxSheetCellCoordinate c2(0, 1);
        CxSheetCellCoordinate c3(1, 0);
        check(c1.hashValue() != c2.hashValue(), "hash: different coords different hash (1)");
        check(c1.hashValue() != c3.hashValue(), "hash: different coords different hash (2)");
    }
}

//-----------------------------------------------------------------------------------------
// CxSheetCell tests
//-----------------------------------------------------------------------------------------
void testCellConstructors() {
    printf("\n== CxSheetCell Constructor Tests ==\n");

    // Default constructor - empty cell
    {
        CxSheetCell cell;
        check(cell.getType() == CxSheetCell::EMPTY, "default ctor: type is EMPTY");
    }

    // Text constructor
    {
        CxSheetCell cell(CxString("Hello"));
        check(cell.getType() == CxSheetCell::TEXT, "text ctor: type is TEXT");
        check(cell.getText() == "Hello", "text ctor: text value");
    }

    // Double constructor
    {
        CxSheetCell cell(CxDouble(3.14));
        check(cell.getType() == CxSheetCell::DOUBLE, "double ctor: type is DOUBLE");
        check(doubleEqual(cell.getDouble().value, 3.14), "double ctor: value");
    }

    // Copy constructor
    {
        CxSheetCell c1(CxString("Test"));
        CxSheetCell c2(c1);
        check(c2.getType() == CxSheetCell::TEXT, "copy ctor: type");
        check(c2.getText() == "Test", "copy ctor: text");
    }
}

void testCellSetters() {
    printf("\n== CxSheetCell Setter Tests ==\n");

    CxSheetCell cell;

    // Set text
    {
        cell.setText(CxString("Hello World"));
        check(cell.getType() == CxSheetCell::TEXT, "setText: type is TEXT");
        check(cell.getText() == "Hello World", "setText: value");
    }

    // Set double
    {
        cell.setDouble(CxDouble(42.5));
        check(cell.getType() == CxSheetCell::DOUBLE, "setDouble: type is DOUBLE");
        check(doubleEqual(cell.getDouble().value, 42.5), "setDouble: value");
        check(doubleEqual(cell.getEvaluatedValue().value, 42.5), "setDouble: evaluated value");
    }

    // Clear
    {
        cell.clear();
        check(cell.getType() == CxSheetCell::EMPTY, "clear: type is EMPTY");
    }

    // Set formula
    {
        cell.setFormula(CxString("5+3"));
        check(cell.getType() == CxSheetCell::FORMULA, "setFormula: type is FORMULA");
        check(cell.getFormulaText() == "5+3", "setFormula: formula text");
    }
}

//-----------------------------------------------------------------------------------------
// CxSheetModel tests
//-----------------------------------------------------------------------------------------
void testModelBasics() {
    printf("\n== CxSheetModel Basic Tests ==\n");

    CxSheetModel model;

    // Initial state
    check(model.numberOfRows() == 1, "initial: numberOfRows is 1");
    check(model.numberOfColumns() == 1, "initial: numberOfColumns is 1");
    check(model.isTouched() == 0, "initial: not touched");
    check(model.isReadOnly() == 0, "initial: not read only");

    // Current position starts at 0,0
    {
        CxSheetCellCoordinate pos = model.getCurrentPosition();
        check(pos.getRow() == 0, "initial position: row 0");
        check(pos.getCol() == 0, "initial position: col 0");
    }
}

void testModelCellStorage() {
    printf("\n== CxSheetModel Cell Storage Tests ==\n");

    CxSheetModel model;

    // Set and get a text cell
    {
        CxSheetCell textCell(CxString("Hello"));
        model.setCell(CxSheetCellCoordinate(0, 0), textCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 0));
        check(retrieved.getType() == CxSheetCell::TEXT, "storage: text cell type");
        check(retrieved.getText() == "Hello", "storage: text cell value");
    }

    // Set and get a double cell
    {
        CxSheetCell doubleCell(CxDouble(99.5));
        model.setCell(CxSheetCellCoordinate(1, 1), doubleCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(1, 1));
        check(retrieved.getType() == CxSheetCell::DOUBLE, "storage: double cell type");
        check(doubleEqual(retrieved.getDouble().value, 99.5), "storage: double cell value");
    }

    // Get empty cell (not set)
    {
        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(10, 10));
        check(retrieved.getType() == CxSheetCell::EMPTY, "storage: unset cell is EMPTY");
    }

    // Check extents updated
    check(model.numberOfRows() == 2, "extents: rows after setting (1,1)");
    check(model.numberOfColumns() == 2, "extents: cols after setting (1,1)");

    // Check touched flag
    check(model.isTouched() == 1, "touched: set after cell change");

    // Clear touched
    model.clearTouched();
    check(model.isTouched() == 0, "touched: cleared");
}

void testModelCursorMovement() {
    printf("\n== CxSheetModel Cursor Movement Tests ==\n");

    CxSheetModel model;

    // Move right
    {
        CxSheetModel::ACTION action = model.cursorRightRequest();
        check(action == CxSheetModel::CURSOR_RIGHT, "cursor right: action");
        CxSheetCellCoordinate pos = model.getCurrentPosition();
        check(pos.getCol() == 1, "cursor right: col is 1");
    }

    // Move down
    {
        CxSheetModel::ACTION action = model.cursorDownRequest();
        check(action == CxSheetModel::CURSOR_DOWN, "cursor down: action");
        CxSheetCellCoordinate pos = model.getCurrentPosition();
        check(pos.getRow() == 1, "cursor down: row is 1");
    }

    // Move left
    {
        CxSheetModel::ACTION action = model.cursorLeftRequest();
        check(action == CxSheetModel::CURSOR_LEFT, "cursor left: action");
        CxSheetCellCoordinate pos = model.getCurrentPosition();
        check(pos.getCol() == 0, "cursor left: col is 0");
    }

    // Move up
    {
        CxSheetModel::ACTION action = model.cursorUpRequest();
        check(action == CxSheetModel::CURSOR_UP, "cursor up: action");
        CxSheetCellCoordinate pos = model.getCurrentPosition();
        check(pos.getRow() == 0, "cursor up: row is 0");
    }

    // Can't move left from col 0
    {
        CxSheetModel::ACTION action = model.cursorLeftRequest();
        check(action == CxSheetModel::NONE, "cursor left at 0: action is NONE");
    }

    // Can't move up from row 0
    {
        CxSheetModel::ACTION action = model.cursorUpRequest();
        check(action == CxSheetModel::NONE, "cursor up at 0: action is NONE");
    }

    // Jump to cell
    {
        CxSheetModel::ACTION action = model.jumpToCell(CxSheetCellCoordinate(10, 20));
        check(action == CxSheetModel::JUMP_DIRECT, "jump: action");
        CxSheetCellCoordinate pos = model.getCurrentPosition();
        check(pos.getRow() == 10, "jump: row");
        check(pos.getCol() == 20, "jump: col");
    }
}

void testModelSimpleFormula() {
    printf("\n== CxSheetModel Simple Formula Tests ==\n");

    CxSheetModel model;

    // Set a cell with a simple formula (no cell references)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("5+3"));
        model.setCell(CxSheetCellCoordinate(0, 0), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 0));
        check(retrieved.getType() == CxSheetCell::FORMULA, "simple formula: type");
        check(doubleEqual(retrieved.getEvaluatedValue().value, 8.0), "simple formula: 5+3 = 8");
    }

    // More complex formula
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("(10+5)*2"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 30.0), "complex formula: (10+5)*2 = 30");
    }
}

void testModelCellReferenceFormula() {
    printf("\n== CxSheetModel Cell Reference Formula Tests ==\n");

    CxSheetModel model;

    // Set cell A1 (0,0) to value 10
    {
        CxSheetCell cell(CxDouble(10.0));
        model.setCell(CxSheetCellCoordinate(0, 0), cell);
    }

    // Set cell B1 (0,1) to value 5
    {
        CxSheetCell cell(CxDouble(5.0));
        model.setCell(CxSheetCellCoordinate(0, 1), cell);
    }

    // Set cell C1 (0,2) to formula "A1+B1"
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("A1+B1"));
        model.setCell(CxSheetCellCoordinate(0, 2), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(retrieved.getType() == CxSheetCell::FORMULA, "cell ref formula: type");
        check(doubleEqual(retrieved.getEvaluatedValue().value, 15.0), "cell ref formula: A1+B1 = 10+5 = 15");
    }

    // Now change A1 to 20 and verify C1 recalculates
    {
        CxSheetCell cell(CxDouble(20.0));
        model.setCell(CxSheetCellCoordinate(0, 0), cell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 25.0), "recalc: A1 changed to 20, C1 = 20+5 = 25");
    }

    // Change B1 to 10 and verify again
    {
        CxSheetCell cell(CxDouble(10.0));
        model.setCell(CxSheetCellCoordinate(0, 1), cell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 30.0), "recalc: B1 changed to 10, C1 = 20+10 = 30");
    }
}

void testModelChainedFormulas() {
    printf("\n== CxSheetModel Chained Formula Tests ==\n");

    CxSheetModel model;

    // A1 = 5
    {
        CxSheetCell cell(CxDouble(5.0));
        model.setCell(CxSheetCellCoordinate(0, 0), cell);
    }

    // B1 = A1 * 2  (should be 10)
    {
        CxSheetCell cell;
        cell.setFormula(CxString("A1*2"));
        model.setCell(CxSheetCellCoordinate(0, 1), cell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 10.0), "chained: B1 = A1*2 = 10");
    }

    // C1 = B1 + 3  (should be 13)
    {
        CxSheetCell cell;
        cell.setFormula(CxString("B1+3"));
        model.setCell(CxSheetCellCoordinate(0, 2), cell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 13.0), "chained: C1 = B1+3 = 13");
    }

    // Change A1 to 10, B1 should become 20, C1 should become 23
    {
        CxSheetCell cell(CxDouble(10.0));
        model.setCell(CxSheetCellCoordinate(0, 0), cell);

        CxSheetCell b1 = model.getCell(CxSheetCellCoordinate(0, 1));
        check(doubleEqual(b1.getEvaluatedValue().value, 20.0), "chained recalc: B1 = 10*2 = 20");

        CxSheetCell c1 = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(c1.getEvaluatedValue().value, 23.0), "chained recalc: C1 = 20+3 = 23");
    }
}

void testModelCircularReference() {
    printf("\n== CxSheetModel Circular Reference Tests ==\n");

    // Test 1: Direct circular reference (A1 references itself)
    {
        CxSheetModel model;

        CxSheetCell cell;
        cell.setFormula(CxString("A1+1"));
        model.setCell(CxSheetCellCoordinate(0, 0), cell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 0));
        // Should evaluate to 0 due to circular reference
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0), "self-reference: A1=A1+1 evaluates to 0");
    }

    // Test 2: Two-cell circular reference (A1 references B1, B1 references A1)
    {
        CxSheetModel model;

        // Set A1 = B1 + 1
        CxSheetCell cellA;
        cellA.setFormula(CxString("B1+1"));
        model.setCell(CxSheetCellCoordinate(0, 0), cellA);

        // Set B1 = A1 + 1
        CxSheetCell cellB;
        cellB.setFormula(CxString("A1+1"));
        model.setCell(CxSheetCellCoordinate(0, 1), cellB);

        // Both should evaluate to 0 due to circular reference
        CxSheetCell retrievedA = model.getCell(CxSheetCellCoordinate(0, 0));
        CxSheetCell retrievedB = model.getCell(CxSheetCellCoordinate(0, 1));

        // At least one of them should detect the circular reference
        int circularDetected = doubleEqual(retrievedA.getEvaluatedValue().value, 0.0) ||
                               doubleEqual(retrievedB.getEvaluatedValue().value, 0.0);
        check(circularDetected, "mutual-reference: A1=B1+1, B1=A1+1 detects cycle");
    }

    // Test 3: Three-cell circular reference chain (A1 -> B1 -> C1 -> A1)
    {
        CxSheetModel model;

        // Set A1 = B1 + 1
        CxSheetCell cellA;
        cellA.setFormula(CxString("B1+1"));
        model.setCell(CxSheetCellCoordinate(0, 0), cellA);

        // Set B1 = C1 + 1
        CxSheetCell cellB;
        cellB.setFormula(CxString("C1+1"));
        model.setCell(CxSheetCellCoordinate(0, 1), cellB);

        // Set C1 = A1 + 1 (creates the cycle)
        CxSheetCell cellC;
        cellC.setFormula(CxString("A1+1"));
        model.setCell(CxSheetCellCoordinate(0, 2), cellC);

        // Should detect circular reference
        CxSheetCell retrievedA = model.getCell(CxSheetCellCoordinate(0, 0));
        CxSheetCell retrievedB = model.getCell(CxSheetCellCoordinate(0, 1));
        CxSheetCell retrievedC = model.getCell(CxSheetCellCoordinate(0, 2));

        // At least one should be 0 due to circular reference detection
        int circularDetected = doubleEqual(retrievedA.getEvaluatedValue().value, 0.0) ||
                               doubleEqual(retrievedB.getEvaluatedValue().value, 0.0) ||
                               doubleEqual(retrievedC.getEvaluatedValue().value, 0.0);
        check(circularDetected, "chain-reference: A1->B1->C1->A1 detects cycle");
    }

    // Test 4: Non-circular chain should still work
    {
        CxSheetModel model;

        // Set A1 = 5
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(5.0)));

        // Set B1 = A1 + 1 (should be 6)
        CxSheetCell cellB;
        cellB.setFormula(CxString("A1+1"));
        model.setCell(CxSheetCellCoordinate(0, 1), cellB);

        // Set C1 = B1 + 1 (should be 7)
        CxSheetCell cellC;
        cellC.setFormula(CxString("B1+1"));
        model.setCell(CxSheetCellCoordinate(0, 2), cellC);

        CxSheetCell retrievedC = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrievedC.getEvaluatedValue().value, 7.0), "non-circular chain: C1 = 5+1+1 = 7");
    }
}

void testModelCopyConstructor() {
    printf("\n== CxSheetModel Copy Constructor Tests ==\n");

    CxSheetModel model1;

    // Set up some cells
    model1.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(100.0)));
    model1.setCell(CxSheetCellCoordinate(0, 1), CxSheetCell(CxString("Hello")));

    // Copy
    CxSheetModel model2(model1);

    // Verify copy has same cells
    {
        CxSheetCell cell = model2.getCell(CxSheetCellCoordinate(0, 0));
        check(cell.getType() == CxSheetCell::DOUBLE, "copy: cell type preserved");
        check(doubleEqual(cell.getDouble().value, 100.0), "copy: cell value preserved");
    }

    // Modify original, copy should not change
    model1.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(999.0)));

    {
        CxSheetCell cell = model2.getCell(CxSheetCellCoordinate(0, 0));
        check(doubleEqual(cell.getDouble().value, 100.0), "copy: independent from original");
    }
}

void testModelAffectedCells() {
    printf("\n== CxSheetModel getLastAffectedCells Tests ==\n");

    // Test 1: Single cell change - only that cell is affected
    {
        CxSheetModel model;

        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));

        CxSList<CxSheetCellCoordinate> affected = model.getLastAffectedCells();
        check(affected.entries() == 1, "single cell: one cell affected");
        check(affected.at(0) == CxSheetCellCoordinate(0, 0), "single cell: correct cell");
    }

    // Test 2: Cell with dependent formula - both cells affected
    {
        CxSheetModel model;

        // Set A1 = 10
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));

        // Set B1 = A1 * 2
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("A1*2"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        // Now change A1 - both A1 and B1 should be affected
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(20.0)));

        CxSList<CxSheetCellCoordinate> affected = model.getLastAffectedCells();
        check(affected.entries() == 2, "dependent formula: two cells affected");

        // A1 should be first (the changed cell)
        check(affected.at(0) == CxSheetCellCoordinate(0, 0), "dependent formula: A1 first");
        // B1 should be second (the dependent)
        check(affected.at(1) == CxSheetCellCoordinate(0, 1), "dependent formula: B1 second");
    }

    // Test 3: Chain of dependencies - all cells in order
    {
        CxSheetModel model;

        // A1 = 5
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(5.0)));

        // B1 = A1 * 2
        CxSheetCell cellB;
        cellB.setFormula(CxString("A1*2"));
        model.setCell(CxSheetCellCoordinate(0, 1), cellB);

        // C1 = B1 + 3
        CxSheetCell cellC;
        cellC.setFormula(CxString("B1+3"));
        model.setCell(CxSheetCellCoordinate(0, 2), cellC);

        // Change A1 - should affect A1, B1, C1 in that order
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));

        CxSList<CxSheetCellCoordinate> affected = model.getLastAffectedCells();
        check(affected.entries() == 3, "chain: three cells affected");
        check(affected.at(0) == CxSheetCellCoordinate(0, 0), "chain: A1 first");
        check(affected.at(1) == CxSheetCellCoordinate(0, 1), "chain: B1 second");
        check(affected.at(2) == CxSheetCellCoordinate(0, 2), "chain: C1 third");
    }

    // Test 4: Diamond dependency (D1 depends on both B1 and C1, both depend on A1)
    {
        CxSheetModel model;

        // A1 = 10
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));

        // B1 = A1 * 2
        CxSheetCell cellB;
        cellB.setFormula(CxString("A1*2"));
        model.setCell(CxSheetCellCoordinate(0, 1), cellB);

        // C1 = A1 + 5
        CxSheetCell cellC;
        cellC.setFormula(CxString("A1+5"));
        model.setCell(CxSheetCellCoordinate(0, 2), cellC);

        // D1 = B1 + C1
        CxSheetCell cellD;
        cellD.setFormula(CxString("B1+C1"));
        model.setCell(CxSheetCellCoordinate(0, 3), cellD);

        // Verify D1 is correct: B1=20, C1=15, D1=35
        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 3));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 35.0), "diamond: D1 = 20+15 = 35");

        // Change A1 - should affect A1, B1, C1, D1
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(5.0)));

        CxSList<CxSheetCellCoordinate> affected = model.getLastAffectedCells();
        check(affected.entries() == 4, "diamond: four cells affected");

        // A1 must be first
        check(affected.at(0) == CxSheetCellCoordinate(0, 0), "diamond: A1 first");

        // D1 must be last (depends on B1 and C1)
        check(affected.at(3) == CxSheetCellCoordinate(0, 3), "diamond: D1 last");

        // Verify recalculation is correct: B1=10, C1=10, D1=20
        retrieved = model.getCell(CxSheetCellCoordinate(0, 3));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 20.0), "diamond: D1 = 10+10 = 20");
    }

    // Test 5: Cell change with no dependents
    {
        CxSheetModel model;

        // Set up: A1 = 10, B1 = A1 * 2
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
        CxSheetCell cellB;
        cellB.setFormula(CxString("A1*2"));
        model.setCell(CxSheetCellCoordinate(0, 1), cellB);

        // Change B1 to a plain value - only B1 is affected (no dependents)
        model.setCell(CxSheetCellCoordinate(0, 1), CxSheetCell(CxDouble(99.0)));

        CxSList<CxSheetCellCoordinate> affected = model.getLastAffectedCells();
        check(affected.entries() == 1, "no dependents: one cell affected");
        check(affected.at(0) == CxSheetCellCoordinate(0, 1), "no dependents: B1 only");
    }
}

void testModelSaveLoad() {
    printf("\n== CxSheetModel Save/Load Tests ==\n");

    const char* testFile = "/tmp/cxsheetmodel_test.json";

    // Test 1: Save and load basic cells
    {
        CxSheetModel model;

        // Set various cell types
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(42.5)));
        model.setCell(CxSheetCellCoordinate(0, 1), CxSheetCell(CxString("Hello World")));
        model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(100.0)));

        // Set cursor position
        model.jumpToCell(CxSheetCellCoordinate(1, 1));

        // Save
        int saveResult = model.saveSheet(CxString(testFile));
        check(saveResult == 1, "save: returns success");
        check(model.isTouched() == 0, "save: clears touched flag");
    }

    // Test 2: Load into new model
    {
        CxSheetModel model;

        int loadResult = model.loadSheet(CxString(testFile));
        check(loadResult == 1, "load: returns success");

        // Check cursor position
        CxSheetCellCoordinate pos = model.getCurrentPosition();
        check(pos.getRow() == 1 && pos.getCol() == 1, "load: cursor position restored");

        // Check double cell
        CxSheetCell cell1 = model.getCell(CxSheetCellCoordinate(0, 0));
        check(cell1.getType() == CxSheetCell::DOUBLE, "load: double cell type");
        check(doubleEqual(cell1.getDouble().value, 42.5), "load: double cell value");

        // Check text cell
        CxSheetCell cell2 = model.getCell(CxSheetCellCoordinate(0, 1));
        check(cell2.getType() == CxSheetCell::TEXT, "load: text cell type");
        check(cell2.getText() == "Hello World", "load: text cell value");

        // Check second double cell
        CxSheetCell cell3 = model.getCell(CxSheetCellCoordinate(1, 0));
        check(cell3.getType() == CxSheetCell::DOUBLE, "load: second double cell type");
        check(doubleEqual(cell3.getDouble().value, 100.0), "load: second double cell value");

        // Check touched flag is clear after load
        check(model.isTouched() == 0, "load: touched flag clear");
    }

    // Test 3: Save and load formula cells
    {
        CxSheetModel model;

        // Set A1 = 10
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));

        // Set B1 = A1 * 2  (formula)
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("A1*2"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        // Save
        int saveResult = model.saveSheet(CxString(testFile));
        check(saveResult == 1, "save formula: returns success");
    }

    // Test 4: Load formula and verify it recalculates
    {
        CxSheetModel model;

        int loadResult = model.loadSheet(CxString(testFile));
        check(loadResult == 1, "load formula: returns success");

        // Check A1
        CxSheetCell cell1 = model.getCell(CxSheetCellCoordinate(0, 0));
        check(cell1.getType() == CxSheetCell::DOUBLE, "load formula: A1 is double");
        check(doubleEqual(cell1.getDouble().value, 10.0), "load formula: A1 = 10");

        // Check B1 formula type
        CxSheetCell cell2 = model.getCell(CxSheetCellCoordinate(0, 1));
        check(cell2.getType() == CxSheetCell::FORMULA, "load formula: B1 is formula");

        // Check B1 evaluated value (A1*2 = 10*2 = 20)
        check(doubleEqual(cell2.getEvaluatedValue().value, 20.0), "load formula: B1 = A1*2 = 20");
    }

    // Test 5: Load non-existent file
    {
        CxSheetModel model;
        int loadResult = model.loadSheet(CxString("/tmp/nonexistent_file_12345.json"));
        check(loadResult == 0, "load: fails for non-existent file");
    }

    // Test 6: After load, getLastAffectedCells returns all loaded cells
    {
        // First save a model with some cells
        CxSheetModel saveModel;
        saveModel.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
        saveModel.setCell(CxSheetCellCoordinate(0, 1), CxSheetCell(CxString("Hello")));
        saveModel.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(20.0)));
        saveModel.saveSheet(CxString(testFile));

        // Load into new model
        CxSheetModel loadModel;
        loadModel.loadSheet(CxString(testFile));

        // After load, all loaded cells should be in the affected list
        CxSList<CxSheetCellCoordinate> affected = loadModel.getLastAffectedCells();
        check(affected.entries() == 3, "load affected: three cells affected");
    }

    // Test 7: Save and load empty cell with appAttributes (like symbolFill)
    {
        CxSheetModel model;

        // Create an empty cell with symbolFill appAttribute
        CxSheetCell symbolCell;
        symbolCell.setAppAttribute("symbolFill", "horizontal");
        model.setCell(CxSheetCellCoordinate(2, 2), symbolCell);  // C3

        // Also add a regular cell for comparison
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(42.0)));

        // Debug: verify cell before save
        CxSheetCell* beforeSave = model.getCellPtr(CxSheetCellCoordinate(2, 2));
        check(beforeSave != NULL, "appAttr: cell exists before save");
        check(beforeSave->getType() == CxSheetCell::EMPTY, "appAttr: cell type is EMPTY before save");
        check(beforeSave->appAttributes != NULL, "appAttr: appAttributes not NULL before save");
        check(beforeSave->hasAppAttribute("symbolFill"), "appAttr: has symbolFill before save");

        // Save
        int saveResult = model.saveSheet(CxString(testFile));
        check(saveResult == 1, "appAttr: save returns success");
    }

    // Test 8: Load and verify appAttributes survived
    {
        CxSheetModel model;

        int loadResult = model.loadSheet(CxString(testFile));
        check(loadResult == 1, "appAttr: load returns success");

        // Check the regular cell
        CxSheetCell cell1 = model.getCell(CxSheetCellCoordinate(0, 0));
        check(cell1.getType() == CxSheetCell::DOUBLE, "appAttr: A1 is double");
        check(doubleEqual(cell1.getDouble().value, 42.0), "appAttr: A1 = 42");

        // Check the empty cell with appAttributes
        CxSheetCell cell2 = model.getCell(CxSheetCellCoordinate(2, 2));
        check(cell2.getType() == CxSheetCell::EMPTY, "appAttr: C3 is EMPTY type");
        check(cell2.hasAppAttribute("symbolFill"), "appAttr: C3 has symbolFill");
        check(cell2.getAppAttributeString("symbolFill") == "horizontal", "appAttr: symbolFill = horizontal");
    }

    // Clean up test file
    remove(testFile);
}

//-----------------------------------------------------------------------------------------
// CxSheetCellRange tests
//-----------------------------------------------------------------------------------------
void testRangeBasics() {
    printf("\n== CxSheetCellRange Basic Tests ==\n");

    // Parse a simple range
    {
        CxSheetCellRange range("A1:A4");
        check(range.isValid() == 1, "parse A1:A4: valid");
        check(range.cellCount() == 4, "parse A1:A4: 4 cells");

        // Check iteration
        CxSheetCellCoordinate c0 = range.cellAt(0);
        check(c0.getRow() == 0 && c0.getCol() == 0, "A1:A4 cellAt(0) = A1");

        CxSheetCellCoordinate c3 = range.cellAt(3);
        check(c3.getRow() == 3 && c3.getCol() == 0, "A1:A4 cellAt(3) = A4");
    }

    // Parse a 2D range
    {
        CxSheetCellRange range("A1:C2");
        check(range.isValid() == 1, "parse A1:C2: valid");
        check(range.cellCount() == 6, "parse A1:C2: 6 cells (3 cols x 2 rows)");

        // Iteration is row-major: A1, B1, C1, A2, B2, C2
        CxSheetCellCoordinate c0 = range.cellAt(0);
        check(c0.getRow() == 0 && c0.getCol() == 0, "A1:C2 cellAt(0) = A1");

        CxSheetCellCoordinate c2 = range.cellAt(2);
        check(c2.getRow() == 0 && c2.getCol() == 2, "A1:C2 cellAt(2) = C1");

        CxSheetCellCoordinate c3 = range.cellAt(3);
        check(c3.getRow() == 1 && c3.getCol() == 0, "A1:C2 cellAt(3) = A2");

        CxSheetCellCoordinate c5 = range.cellAt(5);
        check(c5.getRow() == 1 && c5.getCol() == 2, "A1:C2 cellAt(5) = C2");
    }

    // Parse absolute range
    {
        CxSheetCellRange range("$A$1:$B$2");
        check(range.isValid() == 1, "parse $A$1:$B$2: valid");
        check(range.cellCount() == 4, "parse $A$1:$B$2: 4 cells");
    }

    // Inverted range (end before start) should be normalized
    {
        CxSheetCellRange range("B2:A1");
        check(range.isValid() == 1, "parse B2:A1: valid (normalized)");
        check(range.cellCount() == 4, "parse B2:A1: 4 cells after normalization");

        CxSheetCellCoordinate start = range.getStartCell();
        check(start.getRow() == 0 && start.getCol() == 0, "B2:A1 normalized start = A1");

        CxSheetCellCoordinate end = range.getEndCell();
        check(end.getRow() == 1 && end.getCol() == 1, "B2:A1 normalized end = B2");
    }

    // Invalid ranges
    {
        CxSheetCellRange range1("A1");  // No colon
        check(range1.isValid() == 0, "parse A1: invalid (no colon)");

        CxSheetCellRange range2(":A1");  // Starts with colon
        check(range2.isValid() == 0, "parse :A1: invalid (starts with colon)");

        CxSheetCellRange range3("A1:");  // Ends with colon
        check(range3.isValid() == 0, "parse A1:: invalid (ends with colon)");
    }

    // toAddress
    {
        CxSheetCellRange range("B3:D5");
        CxString addr = range.toAddress();
        check(addr == "B3:D5", "toAddress: B3:D5");
    }
}

void testRangeFunctions() {
    printf("\n== CxSheetModel Range Function Tests ==\n");

    CxSheetModel model;

    // Set up cells A1:A4 with values 1, 2, 3, 4
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(1.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(2.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(3.0)));
    model.setCell(CxSheetCellCoordinate(3, 0), CxSheetCell(CxDouble(4.0)));

    // Test SUM(A1:A4) = 1+2+3+4 = 10
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("SUM(A1:A4)"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        check(retrieved.getType() == CxSheetCell::FORMULA, "SUM(A1:A4): type is formula");
        check(doubleEqual(retrieved.getEvaluatedValue().value, 10.0), "SUM(A1:A4) = 10");
    }

    // Test AVERAGE(A1:A4) = 2.5
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("AVERAGE(A1:A4)"));
        model.setCell(CxSheetCellCoordinate(0, 2), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 2.5), "AVERAGE(A1:A4) = 2.5");
    }

    // Test COUNT(A1:A4) = 4
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("COUNT(A1:A4)"));
        model.setCell(CxSheetCellCoordinate(0, 3), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 3));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 4.0), "COUNT(A1:A4) = 4");
    }

    // Test MIN(A1:A4) = 1
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("MIN(A1:A4)"));
        model.setCell(CxSheetCellCoordinate(0, 4), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 4));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "MIN(A1:A4) = 1");
    }

    // Test MAX(A1:A4) = 4
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("MAX(A1:A4)"));
        model.setCell(CxSheetCellCoordinate(0, 5), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 5));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 4.0), "MAX(A1:A4) = 4");
    }
}

void testRangeDependencies() {
    printf("\n== CxSheetModel Range Dependency Tests ==\n");

    CxSheetModel model;

    // Set up cells A1:A4 with values 1, 2, 3, 4
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(1.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(2.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(3.0)));
    model.setCell(CxSheetCellCoordinate(3, 0), CxSheetCell(CxDouble(4.0)));

    // Set B1 = SUM(A1:A4)
    CxSheetCell formulaCell;
    formulaCell.setFormula(CxString("SUM(A1:A4)"));
    model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

    CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
    check(doubleEqual(retrieved.getEvaluatedValue().value, 10.0), "initial SUM = 10");

    // Change A3 (middle of range) from 3 to 10
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(10.0)));

    // SUM should recalculate: 1 + 2 + 10 + 4 = 17
    retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
    check(doubleEqual(retrieved.getEvaluatedValue().value, 17.0), "after A3 change: SUM = 17");

    // Verify A3 change affects B1 (B1 is in affected cells list)
    CxSList<CxSheetCellCoordinate> affected = model.getLastAffectedCells();
    int sumAffected = 0;
    for (int i = 0; i < (int)affected.entries(); i++) {
        if (affected.at(i) == CxSheetCellCoordinate(0, 1)) {
            sumAffected = 1;
            break;
        }
    }
    check(sumAffected == 1, "B1 (SUM formula) is in affected cells after A3 change");
}

void testRange2D() {
    printf("\n== CxSheetModel 2D Range Tests ==\n");

    CxSheetModel model;

    // Set up a 2x2 grid: A1=1, B1=2, A2=3, B2=4
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(1.0)));
    model.setCell(CxSheetCellCoordinate(0, 1), CxSheetCell(CxDouble(2.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(3.0)));
    model.setCell(CxSheetCellCoordinate(1, 1), CxSheetCell(CxDouble(4.0)));

    // SUM(A1:B2) should be 1+2+3+4 = 10
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("SUM(A1:B2)"));
        model.setCell(CxSheetCellCoordinate(2, 0), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(2, 0));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 10.0), "SUM(A1:B2) = 10");
    }
}

void testRangeHorizontal() {
    printf("\n== CxSheetModel Horizontal Range Tests ==\n");

    CxSheetModel model;

    // Set up A1=1, B1=2, C1=3 (three cells in a row)
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(1.0)));  // A1
    model.setCell(CxSheetCellCoordinate(0, 1), CxSheetCell(CxDouble(2.0)));  // B1
    model.setCell(CxSheetCellCoordinate(0, 2), CxSheetCell(CxDouble(3.0)));  // C1

    // SUM(A1:C1) should be 1+2+3 = 6
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("SUM(A1:C1)"));
        model.setCell(CxSheetCellCoordinate(1, 0), formulaCell);  // A2

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(1, 0));
        printf("  DEBUG: SUM(A1:C1) evaluated value = %f\n", retrieved.getEvaluatedValue().value);
        check(doubleEqual(retrieved.getEvaluatedValue().value, 6.0), "SUM(A1:C1) = 6");
    }
}

void testRangeVertical3() {
    printf("\n== CxSheetModel Vertical Range (3 cells) Tests ==\n");

    CxSheetModel model;

    // Set up A1=1, A2=2, A3=3 (three cells in a column)
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(1.0)));  // A1
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(2.0)));  // A2
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(3.0)));  // A3

    // SUM(A1:A3) should be 1+2+3 = 6
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("SUM(A1:A3)"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);  // B1

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        printf("  DEBUG: SUM(A1:A3) evaluated value = %f\n", retrieved.getEvaluatedValue().value);
        check(doubleEqual(retrieved.getEvaluatedValue().value, 6.0), "SUM(A1:A3) = 6");
    }
}

void testRangeLowercase() {
    printf("\n== CxSheetModel Case-Insensitive Function Tests ==\n");

    CxSheetModel model;

    // Set up A1=1, A2=2, A3=3
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(1.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(2.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(3.0)));

    // Test lowercase sum(A1:A3) - should work (case-insensitive)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("sum(A1:A3)"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 6.0), "sum(A1:A3) = 6 (lowercase)");
    }

    // Test mixed case Sum(A1:A3)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("Sum(A1:A3)"));
        model.setCell(CxSheetCellCoordinate(0, 2), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 6.0), "Sum(A1:A3) = 6 (mixed case)");
    }

    // Test all lowercase average
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("average(A1:A3)"));
        model.setCell(CxSheetCellCoordinate(0, 3), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 3));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 2.0), "average(A1:A3) = 2 (lowercase)");
    }
}

void testIFFunction() {
    printf("\n== CxSheetModel IF Function Tests ==\n");

    CxSheetModel model;

    // Set up some cells with values
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));  // A1 = 10
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(5.0)));   // A2 = 5
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(0.0)));   // A3 = 0

    // Test IF with true condition (non-zero)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IF(1, 100, 200)"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 100.0), "IF(1, 100, 200) = 100");
    }

    // Test IF with false condition (zero)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IF(0, 100, 200)"));
        model.setCell(CxSheetCellCoordinate(0, 2), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 200.0), "IF(0, 100, 200) = 200");
    }

    // Test IF with comparison: A1 > A2 (10 > 5 = true)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IF(A1>A2, A1, A2)"));
        model.setCell(CxSheetCellCoordinate(0, 3), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 3));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 10.0), "IF(A1>A2, A1, A2) = 10");
    }

    // Test IF with comparison: A1 < A2 (10 < 5 = false)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IF(A1<A2, A1, A2)"));
        model.setCell(CxSheetCellCoordinate(0, 4), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 4));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 5.0), "IF(A1<A2, A1, A2) = 5");
    }

    // Test IF with equality: A1 = 10 (true)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IF(A1=10, 1, 0)"));
        model.setCell(CxSheetCellCoordinate(0, 5), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 5));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "IF(A1=10, 1, 0) = 1");
    }

    // Test IF with inequality: A1 <> 5 (true)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IF(A1<>5, 1, 0)"));
        model.setCell(CxSheetCellCoordinate(0, 6), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 6));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "IF(A1<>5, 1, 0) = 1");
    }

    // Test case-insensitive: if (lowercase)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("if(1, 50, 60)"));
        model.setCell(CxSheetCellCoordinate(0, 7), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 7));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 50.0), "if(1, 50, 60) = 50 (lowercase)");
    }

    // Test case-insensitive: If (mixed case)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("If(0, 50, 60)"));
        model.setCell(CxSheetCellCoordinate(0, 8), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 8));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 60.0), "If(0, 50, 60) = 60 (mixed case)");
    }

    // Test IF with arithmetic in condition: (5+5)=10
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IF((5+5)=10, 1, 0)"));
        model.setCell(CxSheetCellCoordinate(0, 9), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 9));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "IF((5+5)=10, 1, 0) = 1");
    }

    // Test IF with zero value in cell (A3=0, should be false)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IF(A3, 1, 0)"));
        model.setCell(CxSheetCellCoordinate(0, 10), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 10));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0), "IF(A3, 1, 0) = 0 (A3 is 0)");
    }

    // Test IF with non-zero value in cell (A1=10, should be true)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IF(A1, 1, 0)"));
        model.setCell(CxSheetCellCoordinate(0, 11), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 11));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "IF(A1, 1, 0) = 1 (A1 is 10)");
    }
}

void testANDFunction() {
    printf("\n== CxSheetModel AND Function Tests ==\n");

    CxSheetModel model;

    // Set up cells: A1=10, A2=5, A3=0
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));  // A1 = 10
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(5.0)));   // A2 = 5
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(0.0)));   // A3 = 0

    // AND(1, 1) = 1 (all true)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("AND(1, 1)"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "AND(1, 1) = 1");
    }

    // AND(1, 0) = 0 (one false)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("AND(1, 0)"));
        model.setCell(CxSheetCellCoordinate(0, 2), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0), "AND(1, 0) = 0");
    }

    // AND(0, 0) = 0 (all false)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("AND(0, 0)"));
        model.setCell(CxSheetCellCoordinate(0, 3), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 3));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0), "AND(0, 0) = 0");
    }

    // AND with cell references: AND(A1, A2) = 1 (both non-zero)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("AND(A1, A2)"));
        model.setCell(CxSheetCellCoordinate(0, 4), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 4));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "AND(A1, A2) = 1");
    }

    // AND with zero cell: AND(A1, A3) = 0 (A3 is 0)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("AND(A1, A3)"));
        model.setCell(CxSheetCellCoordinate(0, 5), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 5));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0), "AND(A1, A3) = 0");
    }

    // AND with comparisons: AND(A1>5, A2>2) = 1 (both true)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("AND(A1>5, A2>2)"));
        model.setCell(CxSheetCellCoordinate(0, 6), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 6));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "AND(A1>5, A2>2) = 1");
    }

    // AND with comparisons: AND(A1>5, A2>10) = 0 (second false)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("AND(A1>5, A2>10)"));
        model.setCell(CxSheetCellCoordinate(0, 7), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 7));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0), "AND(A1>5, A2>10) = 0");
    }

    // AND with multiple args: AND(1, 1, 1, 1) = 1
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("AND(1, 1, 1, 1)"));
        model.setCell(CxSheetCellCoordinate(0, 8), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 8));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "AND(1, 1, 1, 1) = 1");
    }

    // AND with multiple args with one false: AND(1, 1, 0, 1) = 0
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("AND(1, 1, 0, 1)"));
        model.setCell(CxSheetCellCoordinate(0, 9), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 9));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0), "AND(1, 1, 0, 1) = 0");
    }

    // Case insensitive: and(1, 1) = 1
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("and(1, 1)"));
        model.setCell(CxSheetCellCoordinate(0, 10), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 10));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "and(1, 1) = 1 (lowercase)");
    }
}

void testORFunction() {
    printf("\n== CxSheetModel OR Function Tests ==\n");

    CxSheetModel model;

    // Set up cells: A1=10, A2=5, A3=0
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));  // A1 = 10
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(5.0)));   // A2 = 5
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(0.0)));   // A3 = 0

    // OR(1, 1) = 1 (all true)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("OR(1, 1)"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "OR(1, 1) = 1");
    }

    // OR(1, 0) = 1 (one true)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("OR(1, 0)"));
        model.setCell(CxSheetCellCoordinate(0, 2), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "OR(1, 0) = 1");
    }

    // OR(0, 0) = 0 (all false)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("OR(0, 0)"));
        model.setCell(CxSheetCellCoordinate(0, 3), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 3));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0), "OR(0, 0) = 0");
    }

    // OR with cell references: OR(A1, A3) = 1 (A1 is non-zero)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("OR(A1, A3)"));
        model.setCell(CxSheetCellCoordinate(0, 4), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 4));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "OR(A1, A3) = 1");
    }

    // OR with zero cells only: OR(A3, A3) = 0 (both zero)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("OR(A3, A3)"));
        model.setCell(CxSheetCellCoordinate(0, 5), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 5));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0), "OR(A3, A3) = 0");
    }

    // OR with comparisons: OR(A1>100, A2>2) = 1 (second true)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("OR(A1>100, A2>2)"));
        model.setCell(CxSheetCellCoordinate(0, 6), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 6));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "OR(A1>100, A2>2) = 1");
    }

    // OR with comparisons: OR(A1>100, A2>100) = 0 (both false)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("OR(A1>100, A2>100)"));
        model.setCell(CxSheetCellCoordinate(0, 7), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 7));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0), "OR(A1>100, A2>100) = 0");
    }

    // OR with multiple args: OR(0, 0, 0, 1) = 1
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("OR(0, 0, 0, 1)"));
        model.setCell(CxSheetCellCoordinate(0, 8), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 8));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "OR(0, 0, 0, 1) = 1");
    }

    // OR with multiple args all false: OR(0, 0, 0, 0) = 0
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("OR(0, 0, 0, 0)"));
        model.setCell(CxSheetCellCoordinate(0, 9), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 9));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0), "OR(0, 0, 0, 0) = 0");
    }

    // Case insensitive: or(1, 0) = 1
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("or(1, 0)"));
        model.setCell(CxSheetCellCoordinate(0, 10), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 10));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0), "or(1, 0) = 1 (lowercase)");
    }
}

void testFinancialFunctions() {
    printf("\n== CxSheetModel Financial Function Tests ==\n");

    CxSheetModel model;

    // PMT(rate, nper, pv) - loan payment calculation
    // Example: $10,000 loan at 5% annual (0.05/12 monthly) for 12 months
    // PMT(0.05/12, 12, 10000) ≈ 856.07
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("PMT(0.05/12, 12, 10000)"));
        model.setCell(CxSheetCellCoordinate(0, 0), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 0));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 856.07, 0.01), "PMT(0.05/12, 12, 10000) ≈ 856.07");
    }

    // PMT with 0% interest
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("PMT(0, 10, 1000)"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 100.0), "PMT(0, 10, 1000) = 100");
    }

    // FV(rate, nper, pmt) - future value
    // Example: $100/month for 12 months at 5% annual (0.05/12 monthly)
    // FV(0.05/12, 12, 100) ≈ 1227.89
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("FV(0.05/12, 12, 100)"));
        model.setCell(CxSheetCellCoordinate(0, 2), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1227.89, 0.01), "FV(0.05/12, 12, 100) ≈ 1227.89");
    }

    // FV with 0% interest
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("FV(0, 12, 100)"));
        model.setCell(CxSheetCellCoordinate(0, 3), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 3));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1200.0), "FV(0, 12, 100) = 1200");
    }

    // PV(rate, nper, pmt) - present value
    // Example: $100/month for 12 months at 5% annual (0.05/12 monthly)
    // PV(0.05/12, 12, 100) ≈ 1168.12
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("PV(0.05/12, 12, 100)"));
        model.setCell(CxSheetCellCoordinate(0, 4), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 4));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1168.12, 0.01), "PV(0.05/12, 12, 100) ≈ 1168.12");
    }

    // PV with 0% interest
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("PV(0, 12, 100)"));
        model.setCell(CxSheetCellCoordinate(0, 5), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 5));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1200.0), "PV(0, 12, 100) = 1200");
    }

    // Case insensitive: pmt, fv, pv
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("pmt(0, 10, 1000)"));
        model.setCell(CxSheetCellCoordinate(0, 6), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 6));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 100.0), "pmt(0, 10, 1000) = 100 (lowercase)");
    }
}

void testNPVFunction() {
    printf("\n== CxSheetModel NPV Function Tests ==\n");

    CxSheetModel model;

    // Set up cash flow values in cells
    // A1=-1000 (initial investment), A2=300, A3=400, A4=500
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(-1000.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(300.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(400.0)));
    model.setCell(CxSheetCellCoordinate(3, 0), CxSheetCell(CxDouble(500.0)));

    // NPV with scalar values
    // NPV(0.1, 100, 200, 300) = 100/1.1 + 200/1.21 + 300/1.331 ≈ 481.59
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("NPV(0.1, 100, 200, 300)"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 481.59, 0.01), "NPV(0.1, 100, 200, 300) ≈ 481.59");
    }

    // NPV with range
    // NPV(0.1, A2:A4) where A2=300, A3=400, A4=500
    // = 300/1.1 + 400/1.21 + 500/1.331 ≈ 978.96
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("NPV(0.1, A2:A4)"));
        model.setCell(CxSheetCellCoordinate(0, 2), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 978.96, 0.01), "NPV(0.1, A2:A4) ≈ 978.96");
    }

    // NPV at 0% rate = simple sum
    // NPV(0, 100, 200, 300) = 100 + 200 + 300 = 600
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("NPV(0, 100, 200, 300)"));
        model.setCell(CxSheetCellCoordinate(0, 3), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 3));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 600.0), "NPV(0, 100, 200, 300) = 600");
    }

    // Case insensitive
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("npv(0, 100, 200)"));
        model.setCell(CxSheetCellCoordinate(0, 4), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 4));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 300.0), "npv(0, 100, 200) = 300 (lowercase)");
    }
}

void testIRRFunction() {
    printf("\n== CxSheetModel IRR Function Tests ==\n");

    CxSheetModel model;

    // Set up cash flows in cells
    // A1=-1000 (initial investment), A2=300, A3=400, A4=500
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(-1000.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(300.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(400.0)));
    model.setCell(CxSheetCellCoordinate(3, 0), CxSheetCell(CxDouble(500.0)));

    // IRR with scalar values
    // IRR(-1000, 300, 400, 500) ≈ 0.0890 (8.90%)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IRR(-1000, 300, 400, 500)"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0890, 0.001), "IRR(-1000, 300, 400, 500) ≈ 0.0890");
    }

    // IRR with range
    // IRR(A1:A4) where A1=-1000, A2=300, A3=400, A4=500
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IRR(A1:A4)"));
        model.setCell(CxSheetCellCoordinate(0, 2), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 0.0890, 0.001), "IRR(A1:A4) ≈ 0.0890");
    }

    // IRR where return exactly doubles investment: -100, 200
    // IRR(-100, 200) = 1.0 (100%)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("IRR(-100, 200)"));
        model.setCell(CxSheetCellCoordinate(0, 3), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 3));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0, 0.001), "IRR(-100, 200) = 1.0");
    }

    // Case insensitive
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("irr(-100, 200)"));
        model.setCell(CxSheetCellCoordinate(0, 4), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 4));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 1.0, 0.001), "irr(-100, 200) = 1.0 (lowercase)");
    }
}

void testDateFunctions() {
    printf("\n== CxSheetModel Date Function Tests ==\n");

    CxSheetModel model;

    // DATE(year, month, day) - create serial date
    // DATE(2024, 1, 1) should return serial date for Jan 1, 2024
    // Excel serial: Jan 1, 2024 = 45292
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("DATE(2024, 1, 1)"));
        model.setCell(CxSheetCellCoordinate(0, 0), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 0));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 45292.0, 1.0), "DATE(2024, 1, 1) ≈ 45292");
    }

    // DATE(2000, 6, 15) = June 15, 2000
    // Excel serial: 36692
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("DATE(2000, 6, 15)"));
        model.setCell(CxSheetCellCoordinate(0, 1), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 1));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 36692.0, 1.0), "DATE(2000, 6, 15) ≈ 36692");
    }

    // YEAR(serial_date) - extract year
    // YEAR(DATE(2024, 1, 1)) = 2024
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("YEAR(DATE(2024, 1, 1))"));
        model.setCell(CxSheetCellCoordinate(0, 2), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 2));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 2024.0), "YEAR(DATE(2024, 1, 1)) = 2024");
    }

    // MONTH(serial_date) - extract month
    // MONTH(DATE(2024, 7, 15)) = 7
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("MONTH(DATE(2024, 7, 15))"));
        model.setCell(CxSheetCellCoordinate(0, 3), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 3));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 7.0), "MONTH(DATE(2024, 7, 15)) = 7");
    }

    // DAY(serial_date) - extract day
    // DAY(DATE(2024, 7, 25)) = 25
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("DAY(DATE(2024, 7, 25))"));
        model.setCell(CxSheetCellCoordinate(0, 4), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 4));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 25.0), "DAY(DATE(2024, 7, 25)) = 25");
    }

    // NOW() - returns current date/time as serial
    // Just verify it returns a reasonable value (recent date)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("NOW()"));
        model.setCell(CxSheetCellCoordinate(0, 5), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 5));
        double now = retrieved.getEvaluatedValue().value;
        // Should be > Jan 1, 2024 (45292) and < some future date
        check(now > 45292.0 && now < 55000.0, "NOW() returns reasonable date");
    }

    // TODAY() - returns current date (no time)
    // Should be an integer (no fractional part)
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("TODAY()"));
        model.setCell(CxSheetCellCoordinate(0, 6), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 6));
        double today = retrieved.getEvaluatedValue().value;
        // Should be an integer
        check(doubleEqual(today, floor(today)), "TODAY() returns integer");
        // Should be reasonable
        check(today > 45292.0 && today < 55000.0, "TODAY() returns reasonable date");
    }

    // Case insensitive tests
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("date(2024, 1, 1)"));
        model.setCell(CxSheetCellCoordinate(0, 7), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 7));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 45292.0, 1.0), "date(2024, 1, 1) (lowercase)");
    }

    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("year(DATE(2024, 1, 1))"));
        model.setCell(CxSheetCellCoordinate(0, 8), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 8));
        check(doubleEqual(retrieved.getEvaluatedValue().value, 2024.0), "year (lowercase)");
    }

    // Date arithmetic: DATE + 30 days
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("DATE(2024, 1, 1) + 30"));
        model.setCell(CxSheetCellCoordinate(0, 9), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 9));
        // Jan 1 + 30 days = Jan 31
        check(doubleEqual(retrieved.getEvaluatedValue().value, 45322.0, 1.0), "DATE + 30 days");
    }

    // Extract year from date arithmetic result
    {
        CxSheetCell formulaCell;
        formulaCell.setFormula(CxString("YEAR(DATE(2024, 12, 31) + 1)"));
        model.setCell(CxSheetCellCoordinate(0, 10), formulaCell);

        CxSheetCell retrieved = model.getCell(CxSheetCellCoordinate(0, 10));
        // Dec 31, 2024 + 1 = Jan 1, 2025
        check(doubleEqual(retrieved.getEvaluatedValue().value, 2025.0), "YEAR(DATE + 1) crosses year");
    }
}

//-----------------------------------------------------------------------------------------
// AppData tests
//-----------------------------------------------------------------------------------------
void testModelAppData() {
    printf("\n== CxSheetModel AppData Tests ==\n");

    const char* testFile = "/tmp/cxsheetmodel_appdata_test.json";

    // Test 1: getAppData returns NULL initially
    {
        CxSheetModel model;
        CxJSONUTFObject *appData = model.getAppData();
        check(appData == NULL, "appData: initially NULL");
    }

    // Test 2: setAppData/getAppData round trip
    {
        CxSheetModel model;

        // Create app data with a "columns" entry
        CxJSONUTFObject *appData = new CxJSONUTFObject();
        CxJSONUTFObject *columns = new CxJSONUTFObject();
        CxJSONUTFNumber *widthA = new CxJSONUTFNumber(15.0);
        CxJSONUTFNumber *widthB = new CxJSONUTFNumber(25.0);
        columns->append(new CxJSONUTFMember("A", widthA));
        columns->append(new CxJSONUTFMember("B", widthB));
        appData->append(new CxJSONUTFMember("columns", columns));

        model.setAppData(appData);

        // Retrieve and verify
        CxJSONUTFObject *retrieved = model.getAppData();
        check(retrieved != NULL, "appData: getAppData returns non-NULL after set");

        CxJSONUTFMember *colMember = retrieved->find("columns");
        check(colMember != NULL, "appData: has columns member");

        if (colMember) {
            CxJSONUTFObject *cols = (CxJSONUTFObject *)colMember->object();
            CxJSONUTFMember *aMember = cols->find("A");
            check(aMember != NULL, "appData: has A column entry");
            if (aMember) {
                CxJSONUTFNumber *aWidth = (CxJSONUTFNumber *)aMember->object();
                check(doubleEqual(aWidth->get(), 15.0), "appData: A width is 15");
            }
        }
    }

    // Test 3: AppData preserved through save/load
    {
        CxSheetModel model;

        // Set some cells
        model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(42.0)));
        model.setCell(CxSheetCellCoordinate(0, 1), CxSheetCell(CxString("Test")));

        // Create and set app data
        CxJSONUTFObject *appData = new CxJSONUTFObject();
        CxJSONUTFObject *columns = new CxJSONUTFObject();
        columns->append(new CxJSONUTFMember("A", new CxJSONUTFNumber(12.0)));
        columns->append(new CxJSONUTFMember("B", new CxJSONUTFNumber(30.0)));
        columns->append(new CxJSONUTFMember("C", new CxJSONUTFNumber(8.0)));
        appData->append(new CxJSONUTFMember("columns", columns));
        model.setAppData(appData);

        // Save
        int saveResult = model.saveSheet(CxString(testFile));
        check(saveResult == 1, "appData save: returns success");
    }

    // Test 4: Load and verify appData preserved
    {
        CxSheetModel model;
        int loadResult = model.loadSheet(CxString(testFile));
        check(loadResult == 1, "appData load: returns success");

        // Verify cells loaded
        CxSheetCell cell1 = model.getCell(CxSheetCellCoordinate(0, 0));
        check(cell1.getType() == CxSheetCell::DOUBLE, "appData load: cell A1 is double");
        check(doubleEqual(cell1.getDouble().value, 42.0), "appData load: cell A1 value");

        // Verify appData preserved
        CxJSONUTFObject *appData = model.getAppData();
        check(appData != NULL, "appData load: appData preserved");

        if (appData) {
            CxJSONUTFMember *colMember = appData->find("columns");
            check(colMember != NULL, "appData load: columns preserved");

            if (colMember) {
                CxJSONUTFObject *cols = (CxJSONUTFObject *)colMember->object();
                check(cols->entries() == 3, "appData load: 3 column entries");

                CxJSONUTFMember *aMember = cols->find("A");
                CxJSONUTFMember *bMember = cols->find("B");
                CxJSONUTFMember *cMember = cols->find("C");

                check(aMember != NULL, "appData load: A column present");
                check(bMember != NULL, "appData load: B column present");
                check(cMember != NULL, "appData load: C column present");

                if (aMember) {
                    CxJSONUTFNumber *aWidth = (CxJSONUTFNumber *)aMember->object();
                    check(doubleEqual(aWidth->get(), 12.0), "appData load: A width is 12");
                }
                if (bMember) {
                    CxJSONUTFNumber *bWidth = (CxJSONUTFNumber *)bMember->object();
                    check(doubleEqual(bWidth->get(), 30.0), "appData load: B width is 30");
                }
                if (cMember) {
                    CxJSONUTFNumber *cWidth = (CxJSONUTFNumber *)cMember->object();
                    check(doubleEqual(cWidth->get(), 8.0), "appData load: C width is 8");
                }
            }
        }
    }

    // Test 5: setAppData(NULL) clears app data
    {
        CxSheetModel model;

        // Set app data
        CxJSONUTFObject *appData = new CxJSONUTFObject();
        appData->append(new CxJSONUTFMember("test", new CxJSONUTFNumber(99.0)));
        model.setAppData(appData);

        check(model.getAppData() != NULL, "appData: non-NULL before clear");

        // Clear it
        model.setAppData(NULL);
        check(model.getAppData() == NULL, "appData: NULL after setAppData(NULL)");
    }
}


//-----------------------------------------------------------------------------------------
// CxSheetInputParser tests - tryParseNumber
//-----------------------------------------------------------------------------------------
void testInputParserNumber() {
    printf("\n== CxSheetInputParser Number Tests ==\n");

    double value;
    int hasCurrency, hasPercent, hasThousands;

    // Plain integers
    {
        int result = CxSheetInputParser::tryParseNumber("123", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 1, "parseNumber: '123' parses");
        check(doubleEqual(value, 123.0), "parseNumber: '123' value");
        check(hasCurrency == 0, "parseNumber: '123' no currency");
        check(hasPercent == 0, "parseNumber: '123' no percent");
        check(hasThousands == 0, "parseNumber: '123' no thousands");
    }

    // Decimal numbers
    {
        int result = CxSheetInputParser::tryParseNumber("123.45", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 1, "parseNumber: '123.45' parses");
        check(doubleEqual(value, 123.45), "parseNumber: '123.45' value");
    }

    // Negative numbers
    {
        int result = CxSheetInputParser::tryParseNumber("-456.78", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 1, "parseNumber: '-456.78' parses");
        check(doubleEqual(value, -456.78), "parseNumber: '-456.78' value");
    }

    // Thousands separators
    {
        int result = CxSheetInputParser::tryParseNumber("1,234.56", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 1, "parseNumber: '1,234.56' parses");
        check(doubleEqual(value, 1234.56), "parseNumber: '1,234.56' value");
        check(hasThousands == 1, "parseNumber: '1,234.56' has thousands");
    }

    // Currency
    {
        int result = CxSheetInputParser::tryParseNumber("$99.99", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 1, "parseNumber: '$99.99' parses");
        check(doubleEqual(value, 99.99), "parseNumber: '$99.99' value");
        check(hasCurrency == 1, "parseNumber: '$99.99' has currency");
    }

    // Currency with thousands
    {
        int result = CxSheetInputParser::tryParseNumber("$1,234.56", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 1, "parseNumber: '$1,234.56' parses");
        check(doubleEqual(value, 1234.56), "parseNumber: '$1,234.56' value");
        check(hasCurrency == 1, "parseNumber: '$1,234.56' has currency");
        check(hasThousands == 1, "parseNumber: '$1,234.56' has thousands");
    }

    // Percent
    {
        int result = CxSheetInputParser::tryParseNumber("50%", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 1, "parseNumber: '50%' parses");
        check(doubleEqual(value, 0.5), "parseNumber: '50%' value (0.5)");
        check(hasPercent == 1, "parseNumber: '50%' has percent");
    }

    // Percent with decimal
    {
        int result = CxSheetInputParser::tryParseNumber("12.5%", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 1, "parseNumber: '12.5%' parses");
        check(doubleEqual(value, 0.125), "parseNumber: '12.5%' value (0.125)");
        check(hasPercent == 1, "parseNumber: '12.5%' has percent");
    }

    // Leading/trailing whitespace
    {
        int result = CxSheetInputParser::tryParseNumber("  42  ", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 1, "parseNumber: '  42  ' parses");
        check(doubleEqual(value, 42.0), "parseNumber: '  42  ' value");
    }

    // Invalid: letters
    {
        int result = CxSheetInputParser::tryParseNumber("12abc", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 0, "parseNumber: '12abc' fails");
    }

    // Invalid: multiple decimals
    {
        int result = CxSheetInputParser::tryParseNumber("1.2.3", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 0, "parseNumber: '1.2.3' fails");
    }

    // Invalid: empty
    {
        int result = CxSheetInputParser::tryParseNumber("", &value, &hasCurrency, &hasPercent, &hasThousands);
        check(result == 0, "parseNumber: '' fails");
    }
}

//-----------------------------------------------------------------------------------------
// CxSheetInputParser tests - tryParseDate
//-----------------------------------------------------------------------------------------
void testInputParserDate() {
    printf("\n== CxSheetInputParser Date Tests ==\n");

    double serialDate;
    CxString dateFormat;

    // US format mm/dd/yyyy
    {
        int result = CxSheetInputParser::tryParseDate("10/20/2026", &serialDate, &dateFormat);
        check(result == 1, "parseDate: '10/20/2026' parses");
        check(dateFormat == "mm/dd/yyyy", "parseDate: '10/20/2026' format is mm/dd/yyyy");
        // Verify it round-trips
        CxString formatted = CxSheetInputParser::formatDate(serialDate, dateFormat);
        check(formatted == "10/20/2026", "parseDate: '10/20/2026' round-trips");
    }

    // ISO format yyyy-mm-dd
    {
        int result = CxSheetInputParser::tryParseDate("2026-10-20", &serialDate, &dateFormat);
        check(result == 1, "parseDate: '2026-10-20' parses");
        check(dateFormat == "yyyy-mm-dd", "parseDate: '2026-10-20' format is yyyy-mm-dd");
        CxString formatted = CxSheetInputParser::formatDate(serialDate, dateFormat);
        check(formatted == "2026-10-20", "parseDate: '2026-10-20' round-trips");
    }

    // Dash format mm-dd-yyyy
    {
        int result = CxSheetInputParser::tryParseDate("10-20-2026", &serialDate, &dateFormat);
        check(result == 1, "parseDate: '10-20-2026' parses");
        check(dateFormat == "mm-dd-yyyy", "parseDate: '10-20-2026' format is mm-dd-yyyy");
        CxString formatted = CxSheetInputParser::formatDate(serialDate, dateFormat);
        check(formatted == "10-20-2026", "parseDate: '10-20-2026' round-trips");
    }

    // Two-digit year (20xx assumed for < 30)
    {
        int result = CxSheetInputParser::tryParseDate("1/15/26", &serialDate, &dateFormat);
        check(result == 1, "parseDate: '1/15/26' parses");
        CxString formatted = CxSheetInputParser::formatDate(serialDate, dateFormat);
        check(formatted == "01/15/2026", "parseDate: '1/15/26' expands to 2026");
    }

    // Whitespace handling
    {
        int result = CxSheetInputParser::tryParseDate("  3/5/2025  ", &serialDate, &dateFormat);
        check(result == 1, "parseDate: '  3/5/2025  ' parses with whitespace");
    }

    // Invalid: bad month
    {
        int result = CxSheetInputParser::tryParseDate("13/20/2026", &serialDate, &dateFormat);
        check(result == 0, "parseDate: '13/20/2026' fails (bad month)");
    }

    // Invalid: not a date
    {
        int result = CxSheetInputParser::tryParseDate("hello", &serialDate, &dateFormat);
        check(result == 0, "parseDate: 'hello' fails");
    }

    // Invalid: empty
    {
        int result = CxSheetInputParser::tryParseDate("", &serialDate, &dateFormat);
        check(result == 0, "parseDate: '' fails");
    }
}

//-----------------------------------------------------------------------------------------
// CxSheetInputParser tests - formatDate
//-----------------------------------------------------------------------------------------
void testInputParserFormatDate() {
    printf("\n== CxSheetInputParser Format Date Tests ==\n");

    // Create a known date: January 1, 2000
    double serial = CxSheetInputParser::dateToSerial(2000, 1, 1);

    {
        CxString result = CxSheetInputParser::formatDate(serial, "mm/dd/yyyy");
        check(result == "01/01/2000", "formatDate: mm/dd/yyyy");
    }

    {
        CxString result = CxSheetInputParser::formatDate(serial, "yyyy-mm-dd");
        check(result == "2000-01-01", "formatDate: yyyy-mm-dd");
    }

    {
        CxString result = CxSheetInputParser::formatDate(serial, "mm-dd-yyyy");
        check(result == "01-01-2000", "formatDate: mm-dd-yyyy");
    }

    // Test date components round-trip
    {
        int year, month, day;
        CxSheetInputParser::serialToComponents(serial, &year, &month, &day);
        check(year == 2000, "serialToComponents: year 2000");
        check(month == 1, "serialToComponents: month 1");
        check(day == 1, "serialToComponents: day 1");
    }

    // Test a different date: October 20, 2026
    {
        double serial2 = CxSheetInputParser::dateToSerial(2026, 10, 20);
        CxString result = CxSheetInputParser::formatDate(serial2, "mm/dd/yyyy");
        check(result == "10/20/2026", "formatDate: Oct 20, 2026");
    }
}


//-----------------------------------------------------------------------------------------
// CxSheetInputParser tests - error messages
//-----------------------------------------------------------------------------------------
void testInputParserErrorMessages() {
    printf("\n== CxSheetInputParser Error Message Tests ==\n");

    double value;
    int hasCurrency, hasPercent, hasThousands;
    CxString errorMsg;

    // Number parsing errors
    {
        int result = CxSheetInputParser::tryParseNumber("", &value, &hasCurrency, &hasPercent, &hasThousands, &errorMsg);
        check(result == 0, "parseNumber empty: fails");
        check(errorMsg.length() > 0, "parseNumber empty: has error message");
        check(errorMsg == "Empty input", "parseNumber empty: correct message");
    }

    {
        int result = CxSheetInputParser::tryParseNumber("$", &value, &hasCurrency, &hasPercent, &hasThousands, &errorMsg);
        check(result == 0, "parseNumber '$': fails");
        check(errorMsg == "Expected number after $", "parseNumber '$': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseNumber("-", &value, &hasCurrency, &hasPercent, &hasThousands, &errorMsg);
        check(result == 0, "parseNumber '-': fails");
        check(errorMsg == "Expected number after -", "parseNumber '-': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseNumber("abc", &value, &hasCurrency, &hasPercent, &hasThousands, &errorMsg);
        check(result == 0, "parseNumber 'abc': fails");
        check(errorMsg == "Expected number", "parseNumber 'abc': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseNumber("12.34.56", &value, &hasCurrency, &hasPercent, &hasThousands, &errorMsg);
        check(result == 0, "parseNumber '12.34.56': fails");
        check(errorMsg == "Multiple decimal points not allowed", "parseNumber '12.34.56': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseNumber("12.34,56", &value, &hasCurrency, &hasPercent, &hasThousands, &errorMsg);
        check(result == 0, "parseNumber '12.34,56': fails");
        check(errorMsg == "Comma not allowed after decimal point", "parseNumber '12.34,56': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseNumber("123xyz", &value, &hasCurrency, &hasPercent, &hasThousands, &errorMsg);
        check(result == 0, "parseNumber '123xyz': fails");
        check(errorMsg == "Unexpected character 'x'", "parseNumber '123xyz': correct message");
    }

    // Date parsing errors
    double serialDate;
    CxString dateFormat;

    {
        int result = CxSheetInputParser::tryParseDate("", &serialDate, &dateFormat, &errorMsg);
        check(result == 0, "parseDate empty: fails");
        check(errorMsg == "Empty input", "parseDate empty: correct message");
    }

    {
        int result = CxSheetInputParser::tryParseDate("abc", &serialDate, &dateFormat, &errorMsg);
        check(result == 0, "parseDate 'abc': fails");
        check(errorMsg == "Expected month or year", "parseDate 'abc': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseDate("10", &serialDate, &dateFormat, &errorMsg);
        check(result == 0, "parseDate '10': fails");
        check(errorMsg == "Expected date separator (/ or -)", "parseDate '10': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseDate("10.20.2026", &serialDate, &dateFormat, &errorMsg);
        check(result == 0, "parseDate '10.20.2026': fails");
        check(errorMsg == "Invalid separator '.', expected / or -", "parseDate '10.20.2026': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseDate("10/20-2026", &serialDate, &dateFormat, &errorMsg);
        check(result == 0, "parseDate '10/20-2026': fails");
        check(errorMsg == "Date separators must match", "parseDate '10/20-2026': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseDate("13/20/2026", &serialDate, &dateFormat, &errorMsg);
        check(result == 0, "parseDate '13/20/2026': fails");
        check(errorMsg == "Invalid month 13 (must be 1-12)", "parseDate '13/20/2026': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseDate("10/32/2026", &serialDate, &dateFormat, &errorMsg);
        check(result == 0, "parseDate '10/32/2026': fails");
        check(errorMsg == "Invalid day 32 (must be 1-31)", "parseDate '10/32/2026': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseDate("10/20/1899", &serialDate, &dateFormat, &errorMsg);
        check(result == 0, "parseDate '10/20/1899': fails");
        check(errorMsg == "Invalid year 1899 (must be 1900-9999)", "parseDate '10/20/1899': correct message");
    }

    {
        int result = CxSheetInputParser::tryParseDate("10/20/2026abc", &serialDate, &dateFormat, &errorMsg);
        check(result == 0, "parseDate '10/20/2026abc': fails");
        check(errorMsg == "Unexpected character 'a' after date", "parseDate '10/20/2026abc': correct message");
    }

    // Verify NULL errorMsg doesn't crash
    {
        int result = CxSheetInputParser::tryParseNumber("abc", &value, &hasCurrency, &hasPercent, &hasThousands, NULL);
        check(result == 0, "parseNumber with NULL errorMsg: fails safely");
    }

    {
        int result = CxSheetInputParser::tryParseDate("abc", &serialDate, &dateFormat, NULL);
        check(result == 0, "parseDate with NULL errorMsg: fails safely");
    }
}


//-----------------------------------------------------------------------------------------
// detectInputIntent tests
//-----------------------------------------------------------------------------------------
void testDetectInputIntent() {
    printf("\n== CxSheetInputParser detectInputIntent Tests ==\n");

    // Date intent - has / or - between digits
    check((CxSheetInputParser::detectInputIntent("10/20/2026") & INTENT_DATE) != 0,
          "date intent: '10/20/2026' has INTENT_DATE");
    check((CxSheetInputParser::detectInputIntent("2026-10-20") & INTENT_DATE) != 0,
          "date intent: '2026-10-20' has INTENT_DATE");
    check((CxSheetInputParser::detectInputIntent("13/20/2026") & INTENT_DATE) != 0,
          "date intent: '13/20/2026' (invalid) still has INTENT_DATE");

    // Currency intent - starts with $
    check((CxSheetInputParser::detectInputIntent("$100") & INTENT_CURRENCY) != 0,
          "currency intent: '$100' has INTENT_CURRENCY");
    check((CxSheetInputParser::detectInputIntent("$1,234.56") & INTENT_CURRENCY) != 0,
          "currency intent: '$1,234.56' has INTENT_CURRENCY");
    check((CxSheetInputParser::detectInputIntent("$abc") & INTENT_CURRENCY) != 0,
          "currency intent: '$abc' (invalid) still has INTENT_CURRENCY");

    // Percent intent - ends with %
    check((CxSheetInputParser::detectInputIntent("50%") & INTENT_PERCENT) != 0,
          "percent intent: '50%' has INTENT_PERCENT");
    check((CxSheetInputParser::detectInputIntent("12.5%") & INTENT_PERCENT) != 0,
          "percent intent: '12.5%' has INTENT_PERCENT");
    check((CxSheetInputParser::detectInputIntent("abc%") & INTENT_PERCENT) != 0,
          "percent intent: 'abc%' (invalid) still has INTENT_PERCENT");

    // Number intent - all numeric-like characters
    check((CxSheetInputParser::detectInputIntent("123.45") & INTENT_NUMBER) != 0,
          "number intent: '123.45' has INTENT_NUMBER");
    check((CxSheetInputParser::detectInputIntent("1,234") & INTENT_NUMBER) != 0,
          "number intent: '1,234' has INTENT_NUMBER");
    check((CxSheetInputParser::detectInputIntent("-42") & INTENT_NUMBER) != 0,
          "number intent: '-42' has INTENT_NUMBER");
    check((CxSheetInputParser::detectInputIntent("+99") & INTENT_NUMBER) != 0,
          "number intent: '+99' has INTENT_NUMBER");

    // Text - no numeric intent
    check(CxSheetInputParser::detectInputIntent("hello") == INTENT_NONE,
          "text: 'hello' has no intent");
    check(CxSheetInputParser::detectInputIntent("abc123") == INTENT_NONE,
          "text: 'abc123' has no intent");
    check(CxSheetInputParser::detectInputIntent("") == INTENT_NONE,
          "empty: '' has no intent");

    // Combined intents
    int dollarPercent = CxSheetInputParser::detectInputIntent("$50%");
    check((dollarPercent & INTENT_CURRENCY) != 0 && (dollarPercent & INTENT_PERCENT) != 0,
          "combined: '$50%' has both INTENT_CURRENCY and INTENT_PERCENT");
}


//-----------------------------------------------------------------------------------------
// parseAndClassify tests
//-----------------------------------------------------------------------------------------
void testParseAndClassify() {
    printf("\n== CxSheetInputParser parseAndClassify Tests ==\n");

    // Empty input -> EMPTY cell
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("");
        check(r.success == 1, "empty: succeeds");
        check(r.cellType == 0, "empty: EMPTY type");
    }

    // Plain text
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("hello");
        check(r.success == 1, "text: succeeds");
        check(r.cellType == 1, "text: TEXT type");
        check(r.textValue == "hello", "text: value preserved");
    }

    // Mixed alphanumeric text
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("abc123xyz");
        check(r.success == 1, "mixed text: succeeds");
        check(r.cellType == 1, "mixed text: TEXT type");
        check(r.textValue == "abc123xyz", "mixed text: value preserved");
    }

    // Plain number
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("123.45");
        check(r.success == 1, "number: succeeds");
        check(r.cellType == 2, "number: DOUBLE type");
        check(doubleEqual(r.doubleValue, 123.45), "number: correct value");
        check(r.hasCurrency == 0, "number: no currency flag");
        check(r.hasPercent == 0, "number: no percent flag");
    }

    // Integer
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("42");
        check(r.success == 1, "integer: succeeds");
        check(r.cellType == 2, "integer: DOUBLE type");
        check(doubleEqual(r.doubleValue, 42.0), "integer: correct value");
    }

    // Negative number
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("-99.5");
        check(r.success == 1, "negative: succeeds");
        check(r.cellType == 2, "negative: DOUBLE type");
        check(doubleEqual(r.doubleValue, -99.5), "negative: correct value");
    }

    // Currency
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("$1,234.56");
        check(r.success == 1, "currency: succeeds");
        check(r.cellType == 2, "currency: DOUBLE type");
        check(doubleEqual(r.doubleValue, 1234.56), "currency: correct value");
        check(r.hasCurrency == 1, "currency: flag set");
        check(r.hasThousands == 1, "currency: thousands flag set");
    }

    // Simple currency
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("$100");
        check(r.success == 1, "simple currency: succeeds");
        check(r.hasCurrency == 1, "simple currency: flag set");
        check(doubleEqual(r.doubleValue, 100.0), "simple currency: correct value");
    }

    // Percent
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("50%");
        check(r.success == 1, "percent: succeeds");
        check(r.cellType == 2, "percent: DOUBLE type");
        check(r.hasPercent == 1, "percent: flag set");
        check(doubleEqual(r.doubleValue, 0.5), "percent: value /100");
    }

    // Decimal percent
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("12.5%");
        check(r.success == 1, "decimal percent: succeeds");
        check(r.hasPercent == 1, "decimal percent: flag set");
        check(doubleEqual(r.doubleValue, 0.125), "decimal percent: value /100");
    }

    // Number with thousands
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("1,000,000");
        check(r.success == 1, "millions: succeeds");
        check(r.hasThousands == 1, "millions: thousands flag set");
        check(doubleEqual(r.doubleValue, 1000000.0), "millions: correct value");
    }

    // Date mm/dd/yyyy
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("10/20/2026");
        check(r.success == 1, "date mm/dd/yyyy: succeeds");
        check(r.cellType == 2, "date: DOUBLE type (serial)");
        check(r.dateFormat == "mm/dd/yyyy", "date: format detected");
        check(r.doubleValue > 40000, "date: has serial value");  // sanity check
    }

    // Date yyyy-mm-dd (ISO)
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("2026-10-20");
        check(r.success == 1, "date ISO: succeeds");
        check(r.dateFormat == "yyyy-mm-dd", "date ISO: format detected");
    }

    // Invalid date - should fail with error
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("13/20/2026");
        check(r.success == 0, "bad date month: fails");
        check(r.errorMsg.length() > 0, "bad date month: has error message");
    }

    // Invalid date day
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("10/35/2026");
        check(r.success == 0, "bad date day: fails");
        check(r.errorMsg.length() > 0, "bad date day: has error message");
    }

    // Invalid currency - should fail with error
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("$abc");
        check(r.success == 0, "bad currency: fails");
        check(r.errorMsg.length() > 0, "bad currency: has error message");
    }

    // Invalid percent - should fail with error
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("abc%");
        check(r.success == 0, "bad percent: fails");
        check(r.errorMsg.length() > 0, "bad percent: has error message");
    }

    // Multiple decimals - should fail
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("34.4.4");
        check(r.success == 0, "multi-decimal: fails");
        check(r.errorMsg.length() > 0, "multi-decimal: has error message");
    }

    // Alphanumeric starting with digits - treated as text (not numeric intent)
    {
        CxSheetInputParseResult r = CxSheetInputParser::parseAndClassify("123x");
        check(r.success == 1, "123x: succeeds as text");
        check(r.cellType == 1, "123x: TEXT type");
        check(r.textValue == "123x", "123x: value preserved");
    }
}


//-----------------------------------------------------------------------------------------
// applyParsingAttributes tests
//-----------------------------------------------------------------------------------------
void testApplyParsingAttributes() {
    printf("\n== CxSheetInputParser applyParsingAttributes Tests ==\n");

    // Test date format attribute
    {
        CxSheetCell cell;
        CxSheetInputParseResult result = CxSheetInputParser::parseAndClassify("10/20/2026");
        CxSheetInputParser::applyParsingAttributes(&cell, result);
        check(cell.hasAppAttribute("dateFormat"), "date: has dateFormat attribute");
        check(cell.getAppAttributeString("dateFormat") == "mm/dd/yyyy",
              "date: dateFormat is mm/dd/yyyy");
    }

    // Test currency attribute
    {
        CxSheetCell cell;
        CxSheetInputParseResult result = CxSheetInputParser::parseAndClassify("$100");
        CxSheetInputParser::applyParsingAttributes(&cell, result);
        check(cell.hasAppAttribute("currency"), "currency: has currency attribute");
        check(cell.getAppAttributeBool("currency", false) == true, "currency: attribute is true");
    }

    // Test percent attribute
    {
        CxSheetCell cell;
        CxSheetInputParseResult result = CxSheetInputParser::parseAndClassify("50%");
        CxSheetInputParser::applyParsingAttributes(&cell, result);
        check(cell.hasAppAttribute("percent"), "percent: has percent attribute");
        check(cell.getAppAttributeBool("percent", false) == true, "percent: attribute is true");
    }

    // Test thousands attribute
    {
        CxSheetCell cell;
        CxSheetInputParseResult result = CxSheetInputParser::parseAndClassify("$1,234,567");
        CxSheetInputParser::applyParsingAttributes(&cell, result);
        check(cell.hasAppAttribute("thousands"), "thousands: has thousands attribute");
        check(cell.hasAppAttribute("currency"), "thousands: also has currency");
    }

    // Test plain number - no attributes
    {
        CxSheetCell cell;
        CxSheetInputParseResult result = CxSheetInputParser::parseAndClassify("42");
        CxSheetInputParser::applyParsingAttributes(&cell, result);
        check(!cell.hasAppAttribute("dateFormat"), "plain number: no dateFormat");
        check(!cell.hasAppAttribute("currency"), "plain number: no currency");
        check(!cell.hasAppAttribute("percent"), "plain number: no percent");
        check(!cell.hasAppAttribute("thousands"), "plain number: no thousands");
    }

    // Test NULL cell - should not crash
    {
        CxSheetInputParseResult result = CxSheetInputParser::parseAndClassify("$100");
        CxSheetInputParser::applyParsingAttributes(NULL, result);
        check(1, "NULL cell: does not crash");
    }
}


//-----------------------------------------------------------------------------------------
// Insert/Delete Row tests
//-----------------------------------------------------------------------------------------
void testInsertRowBasic() {
    printf("\n== Insert Row Basic Tests ==\n");

    CxSheetModel model;

    // Set up: A1=10, A2=20, A3=30
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(20.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(30.0)));

    // Insert row at row 1 (before A2)
    model.insertRow(1);

    // A1 should be unchanged
    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(0, 0));
        check(c.getType() == CxSheetCell::DOUBLE, "insert row: A1 still double");
        check(doubleEqual(c.getEvaluatedValue().value, 10.0), "insert row: A1 = 10");
    }

    // A2 should now be empty (inserted row)
    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(1, 0));
        check(c.getType() == CxSheetCell::EMPTY, "insert row: A2 is empty (inserted)");
    }

    // A3 should be old A2 = 20
    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(2, 0));
        check(c.getType() == CxSheetCell::DOUBLE, "insert row: A3 is double");
        check(doubleEqual(c.getEvaluatedValue().value, 20.0), "insert row: A3 = 20 (shifted from A2)");
    }

    // A4 should be old A3 = 30
    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(3, 0));
        check(c.getType() == CxSheetCell::DOUBLE, "insert row: A4 is double");
        check(doubleEqual(c.getEvaluatedValue().value, 30.0), "insert row: A4 = 30 (shifted from A3)");
    }
}

void testDeleteRowBasic() {
    printf("\n== Delete Row Basic Tests ==\n");

    CxSheetModel model;

    // Set up: A1=10, A2=20, A3=30, A4=40
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(20.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(30.0)));
    model.setCell(CxSheetCellCoordinate(3, 0), CxSheetCell(CxDouble(40.0)));

    // Delete row 1 (A2=20)
    model.deleteRow(1);

    // A1 unchanged
    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(0, 0));
        check(doubleEqual(c.getEvaluatedValue().value, 10.0), "delete row: A1 = 10");
    }

    // A2 should now be old A3 = 30
    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(1, 0));
        check(doubleEqual(c.getEvaluatedValue().value, 30.0), "delete row: A2 = 30 (shifted from A3)");
    }

    // A3 should now be old A4 = 40
    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(2, 0));
        check(doubleEqual(c.getEvaluatedValue().value, 40.0), "delete row: A3 = 40 (shifted from A4)");
    }

    // A4 should be empty (no orphan)
    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(3, 0));
        check(c.getType() == CxSheetCell::EMPTY, "delete row: A4 is empty (no orphan)");
    }
}

void testDeleteRowNoOrphans() {
    printf("\n== Delete Row No Orphan Cells ==\n");

    CxSheetModel model;

    // Set up a grid: 5 rows x 3 columns with values
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 3; c++) {
            double val = (r + 1) * 10.0 + (c + 1);
            model.setCell(CxSheetCellCoordinate(r, c), CxSheetCell(CxDouble(val)));
        }
    }

    // Delete row 2 (middle row, values 31, 32, 33)
    model.deleteRow(2);

    // Row 0 and 1 unchanged
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 0)).getEvaluatedValue().value, 11.0),
          "no orphan: A1 = 11");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(1, 0)).getEvaluatedValue().value, 21.0),
          "no orphan: A2 = 21");

    // Row 2 should be old row 3 (41, 42, 43)
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(2, 0)).getEvaluatedValue().value, 41.0),
          "no orphan: A3 = 41 (shifted from row 4)");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(2, 1)).getEvaluatedValue().value, 42.0),
          "no orphan: B3 = 42 (shifted from row 4)");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(2, 2)).getEvaluatedValue().value, 43.0),
          "no orphan: C3 = 43 (shifted from row 4)");

    // Row 3 should be old row 4 (51, 52, 53)
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(3, 0)).getEvaluatedValue().value, 51.0),
          "no orphan: A4 = 51 (shifted from row 5)");

    // Row 4 should be completely empty — no orphan cells
    check(model.getCell(CxSheetCellCoordinate(4, 0)).getType() == CxSheetCell::EMPTY,
          "no orphan: A5 empty");
    check(model.getCell(CxSheetCellCoordinate(4, 1)).getType() == CxSheetCell::EMPTY,
          "no orphan: B5 empty");
    check(model.getCell(CxSheetCellCoordinate(4, 2)).getType() == CxSheetCell::EMPTY,
          "no orphan: C5 empty");
}

void testInsertRowFormulaAdjust() {
    printf("\n== Insert Row Formula Adjustment ==\n");

    CxSheetModel model;

    // A1=10, A2=20, A3=A1+A2 (=30)
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(20.0)));
    {
        CxSheetCell f;
        f.setFormula(CxString("A1+A2"));
        model.setCell(CxSheetCellCoordinate(2, 0), f);
    }

    // Verify before insert
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(2, 0)).getEvaluatedValue().value, 30.0),
          "before insert: A3 = A1+A2 = 30");

    // Insert row at row 1 (before A2). A2 shifts to A3, A3(formula) shifts to A4
    model.insertRow(1);

    // A4 should now have formula A1+A3 (refs shifted) = 10+20 = 30
    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(3, 0));
        check(c.getType() == CxSheetCell::FORMULA, "insert formula: A4 is formula");
        check(doubleEqual(c.getEvaluatedValue().value, 30.0), "insert formula: A4 = 30");
    }
}

void testDeleteRowFormulaAdjust() {
    printf("\n== Delete Row Formula Adjustment ==\n");

    CxSheetModel model;

    // A1=10, A2=text, A3=20, A4=A1+A3 (=30)
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxString("filler")));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(20.0)));
    {
        CxSheetCell f;
        f.setFormula(CxString("A1+A3"));
        model.setCell(CxSheetCellCoordinate(3, 0), f);
    }

    check(doubleEqual(model.getCell(CxSheetCellCoordinate(3, 0)).getEvaluatedValue().value, 30.0),
          "before delete: A4 = A1+A3 = 30");

    // Delete row 1 (the filler). A3 shifts to A2, A4(formula) shifts to A3
    model.deleteRow(1);

    // A3 should now have formula A1+A2 = 10+20 = 30
    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(2, 0));
        check(c.getType() == CxSheetCell::FORMULA, "delete formula: A3 is formula");
        check(doubleEqual(c.getEvaluatedValue().value, 30.0), "delete formula: A3 = A1+A2 = 30");
    }

    // A4 should be empty
    check(model.getCell(CxSheetCellCoordinate(3, 0)).getType() == CxSheetCell::EMPTY,
          "delete formula: A4 is empty (no orphan)");
}

void testDeleteRowRangeFormulaShrink() {
    printf("\n== Delete Row Range Formula Shrink ==\n");

    CxSheetModel model;

    // A1=10, A2=20, A3=30, A4=40, A5=50
    // A6=SUM(A1:A5) = 150
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(20.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(30.0)));
    model.setCell(CxSheetCellCoordinate(3, 0), CxSheetCell(CxDouble(40.0)));
    model.setCell(CxSheetCellCoordinate(4, 0), CxSheetCell(CxDouble(50.0)));
    {
        CxSheetCell f;
        f.setFormula(CxString("SUM(A1:A5)"));
        model.setCell(CxSheetCellCoordinate(5, 0), f);
    }

    check(doubleEqual(model.getCell(CxSheetCellCoordinate(5, 0)).getEvaluatedValue().value, 150.0),
          "range shrink: SUM(A1:A5) = 150");

    // Delete row 2 (A3=30). Range should shrink to SUM(A1:A4), value = 120
    model.deleteRow(2);

    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(4, 0));
        check(c.getType() == CxSheetCell::FORMULA, "range shrink: A5 is formula");
        check(doubleEqual(c.getEvaluatedValue().value, 120.0),
              "range shrink: SUM shrinks, value = 10+20+40+50 = 120");
    }

    // No orphan at original row 5
    check(model.getCell(CxSheetCellCoordinate(5, 0)).getType() == CxSheetCell::EMPTY,
          "range shrink: A6 is empty (no orphan)");
}

void testSuccessiveDeletesRangeShrinkToOne() {
    printf("\n== Successive Deletes - Range Shrinks to Single Cell ==\n");

    CxSheetModel model;

    // A1=10, A2=20, A3=30, A4=40, A5=50
    // B1=SUM(A1:A5) = 150
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(20.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(30.0)));
    model.setCell(CxSheetCellCoordinate(3, 0), CxSheetCell(CxDouble(40.0)));
    model.setCell(CxSheetCellCoordinate(4, 0), CxSheetCell(CxDouble(50.0)));
    {
        CxSheetCell f;
        f.setFormula(CxString("SUM(A1:A5)"));
        model.setCell(CxSheetCellCoordinate(0, 1), f);
    }

    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 1)).getEvaluatedValue().value, 150.0),
          "successive: initial SUM(A1:A5) = 150");

    // Delete row 4 (A5=50): SUM(A1:A4) = 100
    model.deleteRow(4);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 1)).getEvaluatedValue().value, 100.0),
          "successive: after del row 4, SUM = 100");

    // Verify no orphan at row 4
    check(model.getCell(CxSheetCellCoordinate(4, 0)).getType() == CxSheetCell::EMPTY,
          "successive: row 4 empty after first delete");

    // Delete row 3 (A4=40): SUM(A1:A3) = 60
    model.deleteRow(3);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 1)).getEvaluatedValue().value, 60.0),
          "successive: after del row 3, SUM = 60");

    check(model.getCell(CxSheetCellCoordinate(3, 0)).getType() == CxSheetCell::EMPTY,
          "successive: row 3 empty after second delete");

    // Delete row 2 (A3=30): SUM(A1:A2) = 30
    model.deleteRow(2);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 1)).getEvaluatedValue().value, 30.0),
          "successive: after del row 2, SUM = 30");

    check(model.getCell(CxSheetCellCoordinate(2, 0)).getType() == CxSheetCell::EMPTY,
          "successive: row 2 empty after third delete");

    // Delete row 1 (A2=20): range collapses to SUM(A1:A1) = 10
    model.deleteRow(1);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 1)).getEvaluatedValue().value, 10.0),
          "successive: after del row 1, SUM collapses to single cell = 10");

    check(model.getCell(CxSheetCellCoordinate(1, 1)).getType() == CxSheetCell::EMPTY,
          "successive: no orphan B2 after fourth delete");

    // Delete row 0 (A1=10): now the last cell in the range is deleted
    // The formula cell (B1) was also at row 0, so it gets deleted too
    model.deleteRow(0);

    // Everything should be empty
    check(model.getCell(CxSheetCellCoordinate(0, 0)).getType() == CxSheetCell::EMPTY,
          "successive: A1 empty after final delete");
    check(model.getCell(CxSheetCellCoordinate(0, 1)).getType() == CxSheetCell::EMPTY,
          "successive: B1 empty after final delete");
}

void testSuccessiveDeletesFromMiddle() {
    printf("\n== Successive Deletes From Middle ==\n");

    CxSheetModel model;

    // A1=10, A2=20, A3=30, A4=40, A5=50
    // A7=SUM(A1:A5) = 150  (formula below the data, not in the range)
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(20.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(30.0)));
    model.setCell(CxSheetCellCoordinate(3, 0), CxSheetCell(CxDouble(40.0)));
    model.setCell(CxSheetCellCoordinate(4, 0), CxSheetCell(CxDouble(50.0)));
    {
        CxSheetCell f;
        f.setFormula(CxString("SUM(A1:A5)"));
        model.setCell(CxSheetCellCoordinate(6, 0), f);
    }

    check(doubleEqual(model.getCell(CxSheetCellCoordinate(6, 0)).getEvaluatedValue().value, 150.0),
          "middle: initial SUM(A1:A5) = 150");

    // Delete row 2 (A3=30) repeatedly from the middle.
    // After: A1=10, A2=20, A3=40, A4=50, A6=SUM(A1:A4)=120
    model.deleteRow(2);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(5, 0)).getEvaluatedValue().value, 120.0),
          "middle: delete row 2 first time, SUM = 120");

    // Delete row 2 again (now A3=40)
    // After: A1=10, A2=20, A3=50, A5=SUM(A1:A3)=80
    model.deleteRow(2);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(4, 0)).getEvaluatedValue().value, 80.0),
          "middle: delete row 2 second time, SUM = 80");

    // Delete row 2 again (now A3=50)
    // After: A1=10, A2=20, A4=SUM(A1:A2)=30
    model.deleteRow(2);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(3, 0)).getEvaluatedValue().value, 30.0),
          "middle: delete row 2 third time, SUM = 30");

    // Delete row 2 again (now A3 is the formula row after shift, but wait —
    // let's check what's at row 2 now)
    // State: A1=10 (row 0), A2=20 (row 1), formula at row 3 → SUM(A1:A2)
    // Row 2 should be empty (gap between data and formula)
    check(model.getCell(CxSheetCellCoordinate(2, 0)).getType() == CxSheetCell::EMPTY,
          "middle: row 2 is empty gap");

    // Delete the empty row 2 — formula shifts from row 3 to row 2
    model.deleteRow(2);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(2, 0)).getEvaluatedValue().value, 30.0),
          "middle: delete empty row, formula shifts to row 2, SUM = 30");

    // No orphan at row 3
    check(model.getCell(CxSheetCellCoordinate(3, 0)).getType() == CxSheetCell::EMPTY,
          "middle: no orphan at row 3");
}

//-----------------------------------------------------------------------------------------
// Insert/Delete Column tests
//-----------------------------------------------------------------------------------------
void testInsertColumnBasic() {
    printf("\n== Insert Column Basic Tests ==\n");

    CxSheetModel model;

    // A1=10, B1=20, C1=30
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(0, 1), CxSheetCell(CxDouble(20.0)));
    model.setCell(CxSheetCellCoordinate(0, 2), CxSheetCell(CxDouble(30.0)));

    // Insert column at col 1 (before B1)
    model.insertColumn(1);

    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 0)).getEvaluatedValue().value, 10.0),
          "insert col: A1 = 10");
    check(model.getCell(CxSheetCellCoordinate(0, 1)).getType() == CxSheetCell::EMPTY,
          "insert col: B1 is empty (inserted)");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 2)).getEvaluatedValue().value, 20.0),
          "insert col: C1 = 20 (shifted from B)");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 3)).getEvaluatedValue().value, 30.0),
          "insert col: D1 = 30 (shifted from C)");
}

void testDeleteColumnBasic() {
    printf("\n== Delete Column Basic Tests ==\n");

    CxSheetModel model;

    // A1=10, B1=20, C1=30, D1=40
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(0, 1), CxSheetCell(CxDouble(20.0)));
    model.setCell(CxSheetCellCoordinate(0, 2), CxSheetCell(CxDouble(30.0)));
    model.setCell(CxSheetCellCoordinate(0, 3), CxSheetCell(CxDouble(40.0)));

    // Delete column 1 (B1=20)
    model.deleteColumn(1);

    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 0)).getEvaluatedValue().value, 10.0),
          "delete col: A1 = 10");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 1)).getEvaluatedValue().value, 30.0),
          "delete col: B1 = 30 (shifted from C)");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 2)).getEvaluatedValue().value, 40.0),
          "delete col: C1 = 40 (shifted from D)");
    check(model.getCell(CxSheetCellCoordinate(0, 3)).getType() == CxSheetCell::EMPTY,
          "delete col: D1 is empty (no orphan)");
}

void testDeleteColumnFormulaAdjust() {
    printf("\n== Delete Column Formula Adjustment ==\n");

    CxSheetModel model;

    // A1=10, B1=filler, C1=20, D1=A1+C1 (=30)
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(0, 1), CxSheetCell(CxString("filler")));
    model.setCell(CxSheetCellCoordinate(0, 2), CxSheetCell(CxDouble(20.0)));
    {
        CxSheetCell f;
        f.setFormula(CxString("A1+C1"));
        model.setCell(CxSheetCellCoordinate(0, 3), f);
    }

    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 3)).getEvaluatedValue().value, 30.0),
          "col formula: D1 = A1+C1 = 30");

    // Delete col 1 (B). C1→B1, D1→C1. Formula adjusts: A1+B1 = 10+20 = 30
    model.deleteColumn(1);

    {
        CxSheetCell c = model.getCell(CxSheetCellCoordinate(0, 2));
        check(c.getType() == CxSheetCell::FORMULA, "col formula: C1 is formula after delete");
        check(doubleEqual(c.getEvaluatedValue().value, 30.0), "col formula: C1 = A1+B1 = 30");
    }

    check(model.getCell(CxSheetCellCoordinate(0, 3)).getType() == CxSheetCell::EMPTY,
          "col formula: D1 is empty (no orphan)");
}

void testSuccessiveDeleteColumns() {
    printf("\n== Successive Column Deletes with Range ==\n");

    CxSheetModel model;

    // Row 1: A1=10, B1=20, C1=30, D1=40, E1=50
    // Row 2: A2=SUM(A1:E1) = 150
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(0, 1), CxSheetCell(CxDouble(20.0)));
    model.setCell(CxSheetCellCoordinate(0, 2), CxSheetCell(CxDouble(30.0)));
    model.setCell(CxSheetCellCoordinate(0, 3), CxSheetCell(CxDouble(40.0)));
    model.setCell(CxSheetCellCoordinate(0, 4), CxSheetCell(CxDouble(50.0)));
    {
        CxSheetCell f;
        f.setFormula(CxString("SUM(A1:E1)"));
        model.setCell(CxSheetCellCoordinate(1, 0), f);
    }

    check(doubleEqual(model.getCell(CxSheetCellCoordinate(1, 0)).getEvaluatedValue().value, 150.0),
          "col successive: initial SUM(A1:E1) = 150");

    // Delete col 4 (E1=50): SUM(A1:D1) = 100
    model.deleteColumn(4);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(1, 0)).getEvaluatedValue().value, 100.0),
          "col successive: after del col E, SUM = 100");

    // Delete col 3 (D1=40): SUM(A1:C1) = 60
    model.deleteColumn(3);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(1, 0)).getEvaluatedValue().value, 60.0),
          "col successive: after del col D, SUM = 60");

    // Delete col 2 (C1=30): SUM(A1:B1) = 30
    model.deleteColumn(2);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(1, 0)).getEvaluatedValue().value, 30.0),
          "col successive: after del col C, SUM = 30");

    // Delete col 1 (B1=20): SUM(A1:A1) = 10
    model.deleteColumn(1);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(1, 0)).getEvaluatedValue().value, 10.0),
          "col successive: range collapses to single cell, SUM = 10");

    // No orphan cells in columns B-E
    check(model.getCell(CxSheetCellCoordinate(0, 1)).getType() == CxSheetCell::EMPTY,
          "col successive: B1 empty");
    check(model.getCell(CxSheetCellCoordinate(0, 2)).getType() == CxSheetCell::EMPTY,
          "col successive: C1 empty");
}

void testInsertDeleteInterleaved() {
    printf("\n== Insert and Delete Interleaved ==\n");

    CxSheetModel model;

    // A1=100, A2=200, A3=SUM(A1:A2) = 300
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(100.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(200.0)));
    {
        CxSheetCell f;
        f.setFormula(CxString("SUM(A1:A2)"));
        model.setCell(CxSheetCellCoordinate(2, 0), f);
    }

    check(doubleEqual(model.getCell(CxSheetCellCoordinate(2, 0)).getEvaluatedValue().value, 300.0),
          "interleaved: initial SUM = 300");

    // Insert row at 1: A1=100, A2=empty, A3=200, A4=SUM(A1:A3) = 300
    model.insertRow(1);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(3, 0)).getEvaluatedValue().value, 300.0),
          "interleaved: after insert, SUM = 300");

    // Set the inserted row: A2=50
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(50.0)));
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(3, 0)).getEvaluatedValue().value, 350.0),
          "interleaved: A2=50 added to range, SUM = 350");

    // Delete row 2 (A3=200): A1=100, A2=50, A3=SUM(A1:A2) = 150
    model.deleteRow(2);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(2, 0)).getEvaluatedValue().value, 150.0),
          "interleaved: after delete, SUM = 150");

    // No orphan at row 3
    check(model.getCell(CxSheetCellCoordinate(3, 0)).getType() == CxSheetCell::EMPTY,
          "interleaved: no orphan at row 3");
}

void testDeleteRowAtZero() {
    printf("\n== Delete Row at Row 0 ==\n");

    CxSheetModel model;

    // A1=10, A2=20, A3=30
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(20.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(30.0)));

    model.deleteRow(0);

    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 0)).getEvaluatedValue().value, 20.0),
          "delete row 0: A1 = 20");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(1, 0)).getEvaluatedValue().value, 30.0),
          "delete row 0: A2 = 30");
    check(model.getCell(CxSheetCellCoordinate(2, 0)).getType() == CxSheetCell::EMPTY,
          "delete row 0: A3 empty (no orphan)");
}

void testInsertRowAtZero() {
    printf("\n== Insert Row at Row 0 ==\n");

    CxSheetModel model;

    // A1=10, A2=20
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(20.0)));

    model.insertRow(0);

    check(model.getCell(CxSheetCellCoordinate(0, 0)).getType() == CxSheetCell::EMPTY,
          "insert row 0: A1 empty (inserted)");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(1, 0)).getEvaluatedValue().value, 10.0),
          "insert row 0: A2 = 10 (shifted)");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(2, 0)).getEvaluatedValue().value, 20.0),
          "insert row 0: A3 = 20 (shifted)");
}

void testDeleteAllRowsSuccessively() {
    printf("\n== Delete All Rows Successively ==\n");

    CxSheetModel model;

    // 3 rows of data
    model.setCell(CxSheetCellCoordinate(0, 0), CxSheetCell(CxDouble(10.0)));
    model.setCell(CxSheetCellCoordinate(1, 0), CxSheetCell(CxDouble(20.0)));
    model.setCell(CxSheetCellCoordinate(2, 0), CxSheetCell(CxDouble(30.0)));

    // Delete row 0 three times — each time the first row
    model.deleteRow(0);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 0)).getEvaluatedValue().value, 20.0),
          "delete all: first delete, A1 = 20");

    model.deleteRow(0);
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(0, 0)).getEvaluatedValue().value, 30.0),
          "delete all: second delete, A1 = 30");

    model.deleteRow(0);
    check(model.getCell(CxSheetCellCoordinate(0, 0)).getType() == CxSheetCell::EMPTY,
          "delete all: third delete, A1 empty - all data gone");
    check(model.getCell(CxSheetCellCoordinate(1, 0)).getType() == CxSheetCell::EMPTY,
          "delete all: no orphan at A2");
    check(model.getCell(CxSheetCellCoordinate(2, 0)).getType() == CxSheetCell::EMPTY,
          "delete all: no orphan at A3");
}

void testDeleteRowMultiColumn() {
    printf("\n== Delete Row with Multiple Columns ==\n");

    CxSheetModel model;

    // 3x3 grid with formula row
    // A1=1, B1=2, C1=3
    // A2=4, B2=5, C2=6
    // A3=7, B3=8, C3=9
    // A4=SUM(A1:A3), B4=SUM(B1:B3), C4=SUM(C1:C3)
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            double val = r * 3 + c + 1;
            model.setCell(CxSheetCellCoordinate(r, c), CxSheetCell(CxDouble(val)));
        }
    }
    {
        CxSheetCell f;
        f.setFormula(CxString("SUM(A1:A3)"));
        model.setCell(CxSheetCellCoordinate(3, 0), f);
    }
    {
        CxSheetCell f;
        f.setFormula(CxString("SUM(B1:B3)"));
        model.setCell(CxSheetCellCoordinate(3, 1), f);
    }
    {
        CxSheetCell f;
        f.setFormula(CxString("SUM(C1:C3)"));
        model.setCell(CxSheetCellCoordinate(3, 2), f);
    }

    // Verify initial sums: A4=12, B4=15, C4=18
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(3, 0)).getEvaluatedValue().value, 12.0),
          "multi-col: initial SUM(A1:A3) = 12");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(3, 1)).getEvaluatedValue().value, 15.0),
          "multi-col: initial SUM(B1:B3) = 15");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(3, 2)).getEvaluatedValue().value, 18.0),
          "multi-col: initial SUM(C1:C3) = 18");

    // Delete row 1 (A2=4, B2=5, C2=6)
    model.deleteRow(1);

    // Sums should be: A3=1+7=8, B3=2+8=10, C3=3+9=12
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(2, 0)).getEvaluatedValue().value, 8.0),
          "multi-col: after delete, SUM(A) = 8");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(2, 1)).getEvaluatedValue().value, 10.0),
          "multi-col: after delete, SUM(B) = 10");
    check(doubleEqual(model.getCell(CxSheetCellCoordinate(2, 2)).getEvaluatedValue().value, 12.0),
          "multi-col: after delete, SUM(C) = 12");

    // No orphans at row 3
    check(model.getCell(CxSheetCellCoordinate(3, 0)).getType() == CxSheetCell::EMPTY,
          "multi-col: no orphan A4");
    check(model.getCell(CxSheetCellCoordinate(3, 1)).getType() == CxSheetCell::EMPTY,
          "multi-col: no orphan B4");
    check(model.getCell(CxSheetCellCoordinate(3, 2)).getType() == CxSheetCell::EMPTY,
          "multi-col: no orphan C4");
}


//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxSheetModel Test Suite\n");
    printf("=======================\n");

    // Coordinate tests
    testCoordinateConstructors();
    testCoordinateColumnLetters();
    testCoordinateAddressParsing();
    testCoordinateToAddress();
    testCoordinateHashAndEquality();

    // Cell tests
    testCellConstructors();
    testCellSetters();

    // Model tests
    testModelBasics();
    testModelCellStorage();
    testModelCursorMovement();
    testModelSimpleFormula();
    testModelCellReferenceFormula();
    testModelChainedFormulas();
    testModelCircularReference();
    testModelAffectedCells();
    testModelCopyConstructor();
    testModelSaveLoad();
    testModelAppData();

    // Range tests
    testRangeBasics();
    testRangeFunctions();
    testRangeDependencies();
    testRange2D();
    testRangeHorizontal();
    testRangeVertical3();
    testRangeLowercase();

    // Logical function tests
    testIFFunction();
    testANDFunction();
    testORFunction();

    // Financial function tests
    testFinancialFunctions();
    testNPVFunction();
    testIRRFunction();

    // Date function tests
    testDateFunctions();

    // Input parser tests
    testInputParserNumber();
    testInputParserDate();
    testInputParserFormatDate();
    testInputParserErrorMessages();

    // New parseAndClassify tests
    testDetectInputIntent();
    testParseAndClassify();
    testApplyParsingAttributes();

    // Insert/delete row tests
    testInsertRowBasic();
    testDeleteRowBasic();
    testDeleteRowNoOrphans();
    testInsertRowFormulaAdjust();
    testDeleteRowFormulaAdjust();
    testDeleteRowRangeFormulaShrink();
    testSuccessiveDeletesRangeShrinkToOne();
    testSuccessiveDeletesFromMiddle();
    testInsertDeleteInterleaved();
    testDeleteRowAtZero();
    testInsertRowAtZero();
    testDeleteAllRowsSuccessively();
    testDeleteRowMultiColumn();

    // Insert/delete column tests
    testInsertColumnBasic();
    testDeleteColumnBasic();
    testDeleteColumnFormulaAdjust();
    testSuccessiveDeleteColumns();

    printf("\n=======================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
