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

    // Range tests
    testRangeBasics();
    testRangeFunctions();
    testRangeDependencies();
    testRange2D();
    testRangeHorizontal();
    testRangeVertical3();
    testRangeLowercase();

    printf("\n=======================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
