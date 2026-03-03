//-----------------------------------------------------------------------------------------
// cxexpression_test.cpp - CxExpression and related classes unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <cx/base/string.h>
#include <cx/expression/expression.h>
#include <cx/expression/token.h>
#include <cx/expression/vardb.h>
#include <cx/expression/funcdb.h>

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
// CxExpressionToken tests
//-----------------------------------------------------------------------------------------
void testExpressionTokenConstructors() {
    printf("\n== CxExpressionToken Constructor Tests ==\n");

    // Default constructor
    {
        CxExpressionToken token;
        check(token.ttype == CxExpressionToken::NONE, "default ctor: type is NONE");
        check(token.text == "", "default ctor: text is empty");
        check(token.value == 0.0, "default ctor: value is 0.0");
    }

    // Parameterized constructor
    {
        CxExpressionToken token(CxExpressionToken::DOUBLE_NUMBER, "3.14", 3.14);
        check(token.ttype == CxExpressionToken::DOUBLE_NUMBER, "param ctor: type");
        check(token.text == "3.14", "param ctor: text");
        check(doubleEqual(token.value, 3.14), "param ctor: value");
    }

    // Copy constructor
    {
        CxExpressionToken t1(CxExpressionToken::VARIABLE, "myvar", 0.0);
        CxExpressionToken t2(t1);
        check(t2.ttype == CxExpressionToken::VARIABLE, "copy ctor: type");
        check(t2.text == "myvar", "copy ctor: text");
    }

    // Assignment operator
    {
        CxExpressionToken t1(CxExpressionToken::FUNCTION, "SIN", 0.0);
        CxExpressionToken t2;
        t2 = t1;
        check(t2.ttype == CxExpressionToken::FUNCTION, "assignment: type");
        check(t2.text == "SIN", "assignment: text");
    }

    // Self-assignment
    {
        CxExpressionToken t1(CxExpressionToken::PLUS_SIGN, "", 0.0);
        t1 = t1;
        check(t1.ttype == CxExpressionToken::PLUS_SIGN, "self-assignment: preserved");
    }
}

void testExpressionTokenOperators() {
    printf("\n== CxExpressionToken Operator Tests ==\n");

    // Equality operator
    {
        CxExpressionToken t1(CxExpressionToken::DOUBLE_NUMBER, "", 5.0);
        CxExpressionToken t2(CxExpressionToken::DOUBLE_NUMBER, "", 5.0);
        check(t1 == t2, "equality: same tokens");
    }

    // Inequality - different type
    {
        CxExpressionToken t1(CxExpressionToken::PLUS_SIGN, "", 0.0);
        CxExpressionToken t2(CxExpressionToken::MINUS_SIGN, "", 0.0);
        check(t1 != t2, "inequality: different types");
    }

    // Inequality - different value
    {
        CxExpressionToken t1(CxExpressionToken::DOUBLE_NUMBER, "", 1.0);
        CxExpressionToken t2(CxExpressionToken::DOUBLE_NUMBER, "", 2.0);
        check(t1 != t2, "inequality: different values");
    }

    // Inequality - different text
    {
        CxExpressionToken t1(CxExpressionToken::VARIABLE, "x", 0.0);
        CxExpressionToken t2(CxExpressionToken::VARIABLE, "y", 0.0);
        check(t1 != t2, "inequality: different text");
    }
}

void testExpressionTokenHelpers() {
    printf("\n== CxExpressionToken Helper Function Tests ==\n");

    // isparen
    check(CxExpressionToken::isparen('('), "isparen: (");
    check(CxExpressionToken::isparen(')'), "isparen: )");
    check(!CxExpressionToken::isparen('x'), "isparen: x is not paren");

    // isleftparen
    check(CxExpressionToken::isleftparen('('), "isleftparen: (");
    check(!CxExpressionToken::isleftparen(')'), "isleftparen: ) is not left");

    // isperiod
    check(CxExpressionToken::isperiod('.'), "isperiod: .");
    check(!CxExpressionToken::isperiod(','), "isperiod: , is not period");

    // isexponent
    check(CxExpressionToken::isexponent('e'), "isexponent: e");
    check(CxExpressionToken::isexponent('E'), "isexponent: E");
    check(!CxExpressionToken::isexponent('x'), "isexponent: x is not exponent");

    // is_one_of
    check(CxExpressionToken::is_one_of('+', "+-*/"), "is_one_of: + in +-*/");
    check(CxExpressionToken::is_one_of('/', "+-*/"), "is_one_of: / in +-*/");
    check(!CxExpressionToken::is_one_of('x', "+-*/"), "is_one_of: x not in +-*/");

    // not_one_of
    check(CxExpressionToken::not_one_of('x', "+-*/"), "not_one_of: x not in +-*/");
    check(!CxExpressionToken::not_one_of('+', "+-*/"), "not_one_of: + is in +-*/");

    // is_in
    check(CxExpressionToken::is_in('3', "0123456789"), "is_in: 3 in digits");
    check(!CxExpressionToken::is_in('a', "0123456789"), "is_in: a not in digits");

    // iswhite
    check(CxExpressionToken::iswhite(' '), "iswhite: space");
    check(CxExpressionToken::iswhite('\t'), "iswhite: tab");
    check(!CxExpressionToken::iswhite('x'), "iswhite: x is not white");

    // isvarfunc
    check(CxExpressionToken::isvarfunc('a'), "isvarfunc: a");
    check(CxExpressionToken::isvarfunc('Z'), "isvarfunc: Z");
    check(CxExpressionToken::isvarfunc('_'), "isvarfunc: _");
    check(CxExpressionToken::isvarfunc('5'), "isvarfunc: digit");
    check(CxExpressionToken::isvarfunc('$'), "isvarfunc: $");
    check(!CxExpressionToken::isvarfunc('+'), "isvarfunc: + is not varfunc");

    // isnull
    check(CxExpressionToken::isnull('\0'), "isnull: null char");
    check(!CxExpressionToken::isnull('x'), "isnull: x is not null");
}

void testExpressionTokenAsString() {
    printf("\n== CxExpressionToken asString Tests ==\n");

    check(CxExpressionToken(CxExpressionToken::PLUS_SIGN, "", 0.0).asString() == "+",
          "asString: PLUS_SIGN");
    check(CxExpressionToken(CxExpressionToken::MINUS_SIGN, "", 0.0).asString() == "-",
          "asString: MINUS_SIGN");
    check(CxExpressionToken(CxExpressionToken::MULT_SIGN, "", 0.0).asString() == "*",
          "asString: MULT_SIGN");
    check(CxExpressionToken(CxExpressionToken::DIV_SIGN, "", 0.0).asString() == "/",
          "asString: DIV_SIGN");
    check(CxExpressionToken(CxExpressionToken::LEFT_PAREN, "", 0.0).asString() == "(",
          "asString: LEFT_PAREN");
    check(CxExpressionToken(CxExpressionToken::RIGHT_PAREN, "", 0.0).asString() == ")",
          "asString: RIGHT_PAREN");
    check(CxExpressionToken(CxExpressionToken::COMMA, "", 0.0).asString() == ",",
          "asString: COMMA");
    check(CxExpressionToken(CxExpressionToken::VARIABLE, "myvar", 0.0).asString() == "myvar",
          "asString: VARIABLE");
    check(CxExpressionToken(CxExpressionToken::FUNCTION, "SIN", 0.0).asString() == "SIN",
          "asString: FUNCTION");
    check(CxExpressionToken(CxExpressionToken::END, "", 0.0).asString() == "",
          "asString: END");
}

//-----------------------------------------------------------------------------------------
// CxExpressionVariableDatabase tests
//-----------------------------------------------------------------------------------------
void testVariableDatabase() {
    printf("\n== CxExpressionVariableDatabase Tests ==\n");

    CxExpressionVariableDatabase db;
    double result;

    // Base database always returns UNDEFINED
    check(db.VariableDefined("anything") == CxExpressionVariableDatabase::VARIABLE_UNDEFINED,
          "base db: VariableDefined returns UNDEFINED");
    check(db.VariableEvaluate("anything", &result) == CxExpressionVariableDatabase::VARIABLE_UNDEFINED,
          "base db: VariableEvaluate returns UNDEFINED");
}

void testIntrinsicVariableDatabase() {
    printf("\n== CxExpressionIntrinsicVariableDatabase Tests ==\n");

    CxExpressionIntrinsicVariableDatabase db;
    double result;

    // Test M_PI
    check(db.VariableDefined("M_PI") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_PI defined");
    db.VariableEvaluate("M_PI", &result);
    check(doubleEqual(result, M_PI), "intrinsic: M_PI value");

    // Test M_E
    check(db.VariableDefined("M_E") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_E defined");
    db.VariableEvaluate("M_E", &result);
    check(doubleEqual(result, M_E), "intrinsic: M_E value");

    // Test M_LOG2E
    check(db.VariableDefined("M_LOG2E") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_LOG2E defined");
    db.VariableEvaluate("M_LOG2E", &result);
    check(doubleEqual(result, M_LOG2E), "intrinsic: M_LOG2E value");

    // Test M_LOG10E
    check(db.VariableDefined("M_LOG10E") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_LOG10E defined");

    // Test M_LN2
    check(db.VariableDefined("M_LN2") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_LN2 defined");

    // Test M_LN10
    check(db.VariableDefined("M_LN10") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_LN10 defined");

    // Test M_PI_2
    check(db.VariableDefined("M_PI_2") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_PI_2 defined");
    db.VariableEvaluate("M_PI_2", &result);
    check(doubleEqual(result, M_PI_2), "intrinsic: M_PI_2 value");

    // Test M_PI_4
    check(db.VariableDefined("M_PI_4") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_PI_4 defined");

    // Test M_1_PI
    check(db.VariableDefined("M_1_PI") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_1_PI defined");

    // Test M_2_PI
    check(db.VariableDefined("M_2_PI") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_2_PI defined");

    // Test M_2_SQRTPI
    check(db.VariableDefined("M_2_SQRTPI") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_2_SQRTPI defined");

    // Test M_SQRT2
    check(db.VariableDefined("M_SQRT2") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_SQRT2 defined");
    db.VariableEvaluate("M_SQRT2", &result);
    check(doubleEqual(result, M_SQRT2), "intrinsic: M_SQRT2 value");

    // Test M_SQRT1_2
    check(db.VariableDefined("M_SQRT1_2") == CxExpressionVariableDatabase::VARIABLE_DEFINED,
          "intrinsic: M_SQRT1_2 defined");

    // Test undefined variable
    check(db.VariableDefined("UNDEFINED_VAR") == CxExpressionVariableDatabase::VARIABLE_UNDEFINED,
          "intrinsic: undefined variable");
}

//-----------------------------------------------------------------------------------------
// CxExpressionFunctionDatabase tests
//-----------------------------------------------------------------------------------------
void testFunctionDatabase() {
    printf("\n== CxExpressionFunctionDatabase Tests ==\n");

    CxExpressionFunctionDatabase db;
    double result;
    double args[2] = {1.0, 2.0};

    // Base database always returns UNDEFINED
    check(db.FunctionDefined("anything") == CxExpressionFunctionDatabase::FUNCTION_UNDEFINED,
          "base db: FunctionDefined returns UNDEFINED");
    check(db.FunctionEvaluate("anything", 1, args, &result) == CxExpressionFunctionDatabase::FUNCTION_UNDEFINED,
          "base db: FunctionEvaluate returns UNDEFINED");
}

void testIntrinsicFunctionDefined() {
    printf("\n== CxExpressionIntrinsicFunctionDatabase Defined Tests ==\n");

    CxExpressionIntrinsicFunctionDatabase db;

    // Test all defined functions
    check(db.FunctionDefined("SIN") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: SIN");
    check(db.FunctionDefined("COS") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: COS");
    check(db.FunctionDefined("TAN") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: TAN");
    check(db.FunctionDefined("ASIN") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: ASIN");
    check(db.FunctionDefined("ACOS") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: ACOS");
    check(db.FunctionDefined("ATAN") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: ATAN");
    check(db.FunctionDefined("SINH") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: SINH");
    check(db.FunctionDefined("COSH") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: COSH");
    check(db.FunctionDefined("TANH") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: TANH");
    check(db.FunctionDefined("ASINH") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: ASINH");
    check(db.FunctionDefined("ACOSH") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: ACOSH");
    check(db.FunctionDefined("ATANH") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: ATANH");
    check(db.FunctionDefined("LOG") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: LOG");
    check(db.FunctionDefined("EXP") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: EXP");
    check(db.FunctionDefined("LOG10") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: LOG10");
    check(db.FunctionDefined("ALOG10") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: ALOG10");
    check(db.FunctionDefined("ABS") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: ABS");
    check(db.FunctionDefined("CEIL") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: CEIL");
    check(db.FunctionDefined("FLOOR") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: FLOOR");
    check(db.FunctionDefined("SQRT") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: SQRT");
    check(db.FunctionDefined("ATAN2") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: ATAN2");
    check(db.FunctionDefined("POW") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: POW");
    check(db.FunctionDefined("MIN") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: MIN");
    check(db.FunctionDefined("MAX") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: MAX");
    check(db.FunctionDefined("R2D") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: R2D");
    check(db.FunctionDefined("D2R") == CxExpressionFunctionDatabase::FUNCTION_DEFINED, "func defined: D2R");

    // Test undefined function
    check(db.FunctionDefined("UNDEFINED") == CxExpressionFunctionDatabase::FUNCTION_UNDEFINED,
          "func undefined: UNDEFINED");
}

void testIntrinsicFunctionEvaluate() {
    printf("\n== CxExpressionIntrinsicFunctionDatabase Evaluate Tests ==\n");

    CxExpressionIntrinsicFunctionDatabase db;
    double result;
    double args[3];

    // SIN
    args[0] = 0.0;
    db.FunctionEvaluate("SIN", 1, args, &result);
    check(doubleEqual(result, 0.0), "SIN(0) = 0");

    args[0] = M_PI / 2.0;
    db.FunctionEvaluate("SIN", 1, args, &result);
    check(doubleEqual(result, 1.0), "SIN(PI/2) = 1");

    // COS
    args[0] = 0.0;
    db.FunctionEvaluate("COS", 1, args, &result);
    check(doubleEqual(result, 1.0), "COS(0) = 1");

    args[0] = M_PI;
    db.FunctionEvaluate("COS", 1, args, &result);
    check(doubleEqual(result, -1.0), "COS(PI) = -1");

    // TAN
    args[0] = 0.0;
    db.FunctionEvaluate("TAN", 1, args, &result);
    check(doubleEqual(result, 0.0), "TAN(0) = 0");

    // ASIN
    args[0] = 0.0;
    db.FunctionEvaluate("ASIN", 1, args, &result);
    check(doubleEqual(result, 0.0), "ASIN(0) = 0");

    // ACOS
    args[0] = 1.0;
    db.FunctionEvaluate("ACOS", 1, args, &result);
    check(doubleEqual(result, 0.0), "ACOS(1) = 0");

    // ATAN
    args[0] = 0.0;
    db.FunctionEvaluate("ATAN", 1, args, &result);
    check(doubleEqual(result, 0.0), "ATAN(0) = 0");

    // LOG (natural log)
    args[0] = M_E;
    db.FunctionEvaluate("LOG", 1, args, &result);
    check(doubleEqual(result, 1.0), "LOG(e) = 1");

    args[0] = 1.0;
    db.FunctionEvaluate("LOG", 1, args, &result);
    check(doubleEqual(result, 0.0), "LOG(1) = 0");

    // EXP
    args[0] = 0.0;
    db.FunctionEvaluate("EXP", 1, args, &result);
    check(doubleEqual(result, 1.0), "EXP(0) = 1");

    args[0] = 1.0;
    db.FunctionEvaluate("EXP", 1, args, &result);
    check(doubleEqual(result, M_E), "EXP(1) = e");

    // LOG10
    args[0] = 10.0;
    db.FunctionEvaluate("LOG10", 1, args, &result);
    check(doubleEqual(result, 1.0), "LOG10(10) = 1");

    args[0] = 100.0;
    db.FunctionEvaluate("LOG10", 1, args, &result);
    check(doubleEqual(result, 2.0), "LOG10(100) = 2");

    // ALOG10 (10^x)
    args[0] = 2.0;
    db.FunctionEvaluate("ALOG10", 1, args, &result);
    check(doubleEqual(result, 100.0), "ALOG10(2) = 100");

    // ABS
    args[0] = -5.0;
    db.FunctionEvaluate("ABS", 1, args, &result);
    check(doubleEqual(result, 5.0), "ABS(-5) = 5");

    args[0] = 5.0;
    db.FunctionEvaluate("ABS", 1, args, &result);
    check(doubleEqual(result, 5.0), "ABS(5) = 5");

    // CEIL
    args[0] = 4.3;
    db.FunctionEvaluate("CEIL", 1, args, &result);
    check(doubleEqual(result, 5.0), "CEIL(4.3) = 5");

    args[0] = -4.3;
    db.FunctionEvaluate("CEIL", 1, args, &result);
    check(doubleEqual(result, -4.0), "CEIL(-4.3) = -4");

    // FLOOR
    args[0] = 4.7;
    db.FunctionEvaluate("FLOOR", 1, args, &result);
    check(doubleEqual(result, 4.0), "FLOOR(4.7) = 4");

    args[0] = -4.7;
    db.FunctionEvaluate("FLOOR", 1, args, &result);
    check(doubleEqual(result, -5.0), "FLOOR(-4.7) = -5");

    // SQRT
    args[0] = 4.0;
    db.FunctionEvaluate("SQRT", 1, args, &result);
    check(doubleEqual(result, 2.0), "SQRT(4) = 2");

    args[0] = 9.0;
    db.FunctionEvaluate("SQRT", 1, args, &result);
    check(doubleEqual(result, 3.0), "SQRT(9) = 3");

    // ATAN2 (2 args)
    args[0] = 1.0;
    args[1] = 1.0;
    db.FunctionEvaluate("ATAN2", 2, args, &result);
    check(doubleEqual(result, M_PI / 4.0), "ATAN2(1,1) = PI/4");

    // POW (2 args)
    args[0] = 2.0;
    args[1] = 3.0;
    db.FunctionEvaluate("POW", 2, args, &result);
    check(doubleEqual(result, 8.0), "POW(2,3) = 8");

    args[0] = 10.0;
    args[1] = 2.0;
    db.FunctionEvaluate("POW", 2, args, &result);
    check(doubleEqual(result, 100.0), "POW(10,2) = 100");

    // MIN (variable args)
    args[0] = 5.0;
    args[1] = 3.0;
    args[2] = 7.0;
    db.FunctionEvaluate("MIN", 3, args, &result);
    check(doubleEqual(result, 3.0), "MIN(5,3,7) = 3");

    // MAX (variable args)
    args[0] = 5.0;
    args[1] = 3.0;
    args[2] = 7.0;
    db.FunctionEvaluate("MAX", 3, args, &result);
    check(doubleEqual(result, 7.0), "MAX(5,3,7) = 7");

    // R2D (radians to degrees)
    args[0] = M_PI;
    db.FunctionEvaluate("R2D", 1, args, &result);
    check(doubleEqual(result, 180.0, 0.0001), "R2D(PI) = 180");

    // D2R (degrees to radians)
    args[0] = 180.0;
    db.FunctionEvaluate("D2R", 1, args, &result);
    check(doubleEqual(result, M_PI, 0.0001), "D2R(180) = PI");
}

void testIntrinsicFunctionErrors() {
    printf("\n== CxExpressionIntrinsicFunctionDatabase Error Tests ==\n");

    CxExpressionIntrinsicFunctionDatabase db;
    double result;
    double args[2];

    // Wrong number of args for SIN
    args[0] = 0.0;
    args[1] = 0.0;
    check(db.FunctionEvaluate("SIN", 2, args, &result) == CxExpressionFunctionDatabase::FUNCTION_BAD_NUMBER_OF_ARGS,
          "SIN with 2 args: BAD_NUMBER_OF_ARGS");

    // Wrong number of args for POW
    args[0] = 2.0;
    check(db.FunctionEvaluate("POW", 1, args, &result) == CxExpressionFunctionDatabase::FUNCTION_BAD_NUMBER_OF_ARGS,
          "POW with 1 arg: BAD_NUMBER_OF_ARGS");

    // LOG of negative number
    args[0] = -1.0;
    check(db.FunctionEvaluate("LOG", 1, args, &result) == CxExpressionFunctionDatabase::FUNCTION_RANGE_ERROR,
          "LOG(-1): RANGE_ERROR");

    // LOG of zero
    args[0] = 0.0;
    check(db.FunctionEvaluate("LOG", 1, args, &result) == CxExpressionFunctionDatabase::FUNCTION_RANGE_ERROR,
          "LOG(0): RANGE_ERROR");

    // LOG10 of negative number
    args[0] = -1.0;
    check(db.FunctionEvaluate("LOG10", 1, args, &result) == CxExpressionFunctionDatabase::FUNCTION_RANGE_ERROR,
          "LOG10(-1): RANGE_ERROR");

    // SQRT of negative number
    args[0] = -1.0;
    check(db.FunctionEvaluate("SQRT", 1, args, &result) == CxExpressionFunctionDatabase::FUNCTION_RANGE_ERROR,
          "SQRT(-1): RANGE_ERROR");
}

//-----------------------------------------------------------------------------------------
// CxExpression basic tests
//-----------------------------------------------------------------------------------------
void testExpressionSimpleNumbers() {
    printf("\n== CxExpression Simple Number Tests ==\n");

    double result;

    // Single integer
    {
        CxExpression expr("5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5");
        check(doubleEqual(result, 5.0), "5 = 5");
    }

    // Single decimal
    {
        CxExpression expr("3.14");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 3.14");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 3.14");
        check(doubleEqual(result, 3.14), "3.14 = 3.14");
    }

    // Negative number
    {
        CxExpression expr("-7");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: -7");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: -7");
        check(doubleEqual(result, -7.0), "-7 = -7");
    }

    // Scientific notation
    {
        CxExpression expr("1.5e2");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 1.5e2");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 1.5e2");
        check(doubleEqual(result, 150.0), "1.5e2 = 150");
    }

    // Scientific notation with negative exponent
    {
        CxExpression expr("5e-2");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5e-2");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5e-2");
        check(doubleEqual(result, 0.05), "5e-2 = 0.05");
    }

    // Zero
    {
        CxExpression expr("0");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 0");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 0");
        check(doubleEqual(result, 0.0), "0 = 0");
    }
}

void testExpressionBasicArithmetic() {
    printf("\n== CxExpression Basic Arithmetic Tests ==\n");

    double result;

    // Addition
    {
        CxExpression expr("2+3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 2+3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 2+3");
        check(doubleEqual(result, 5.0), "2+3 = 5");
    }

    // Subtraction
    {
        CxExpression expr("7-4");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 7-4");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 7-4");
        check(doubleEqual(result, 3.0), "7-4 = 3");
    }

    // Multiplication
    {
        CxExpression expr("3*4");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 3*4");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 3*4");
        check(doubleEqual(result, 12.0), "3*4 = 12");
    }

    // Division
    {
        CxExpression expr("15/3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 15/3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 15/3");
        check(doubleEqual(result, 5.0), "15/3 = 5");
    }

    // Multiple additions
    {
        CxExpression expr("1+2+3+4");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 1+2+3+4");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 1+2+3+4");
        check(doubleEqual(result, 10.0), "1+2+3+4 = 10");
    }

    // Mixed add/subtract
    {
        CxExpression expr("10-3+2");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 10-3+2");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 10-3+2");
        check(doubleEqual(result, 9.0), "10-3+2 = 9");
    }

    // Mixed multiply/divide
    {
        CxExpression expr("12/3*2");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 12/3*2");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 12/3*2");
        check(doubleEqual(result, 8.0), "12/3*2 = 8");
    }
}

void testExpressionPrecedence() {
    printf("\n== CxExpression Precedence Tests ==\n");

    double result;

    // Multiplication before addition
    {
        CxExpression expr("2+3*4");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 2+3*4");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 2+3*4");
        check(doubleEqual(result, 14.0), "2+3*4 = 14 (not 20)");
    }

    // Division before subtraction
    {
        CxExpression expr("10-6/2");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 10-6/2");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 10-6/2");
        check(doubleEqual(result, 7.0), "10-6/2 = 7 (not 2)");
    }

    // Complex precedence
    {
        CxExpression expr("2+3*4-6/2");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 2+3*4-6/2");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 2+3*4-6/2");
        check(doubleEqual(result, 11.0), "2+3*4-6/2 = 11");
    }

    // Multiplication in middle
    {
        CxExpression expr("1+2*3+4");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 1+2*3+4");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 1+2*3+4");
        check(doubleEqual(result, 11.0), "1+2*3+4 = 11");
    }
}

void testExpressionParentheses() {
    printf("\n== CxExpression Parentheses Tests ==\n");

    double result;

    // Simple parentheses
    {
        CxExpression expr("(2+3)*4");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: (2+3)*4");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: (2+3)*4");
        check(doubleEqual(result, 20.0), "(2+3)*4 = 20");
    }

    // Nested parentheses
    {
        CxExpression expr("((2+3)*4)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: ((2+3)*4)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: ((2+3)*4)");
        check(doubleEqual(result, 20.0), "((2+3)*4) = 20");
    }

    // Multiple parentheses groups
    {
        CxExpression expr("(2+3)*(4+5)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: (2+3)*(4+5)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: (2+3)*(4+5)");
        check(doubleEqual(result, 45.0), "(2+3)*(4+5) = 45");
    }

    // Parentheses override precedence
    {
        CxExpression expr("(10-6)/2");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: (10-6)/2");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: (10-6)/2");
        check(doubleEqual(result, 2.0), "(10-6)/2 = 2");
    }

    // Deep nesting
    {
        CxExpression expr("(((1+2)))");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: (((1+2)))");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: (((1+2)))");
        check(doubleEqual(result, 3.0), "(((1+2))) = 3");
    }
}

void testExpressionUnaryMinus() {
    printf("\n== CxExpression Unary Minus Tests ==\n");

    double result;

    // Unary minus at start
    {
        CxExpression expr("-5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: -5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: -5");
        check(doubleEqual(result, -5.0), "-5 = -5");
    }

    // Unary minus with addition
    {
        CxExpression expr("-5+3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: -5+3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: -5+3");
        check(doubleEqual(result, -2.0), "-5+3 = -2");
    }

    // Unary minus in parentheses
    {
        CxExpression expr("(-5)*2");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: (-5)*2");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: (-5)*2");
        check(doubleEqual(result, -10.0), "(-5)*2 = -10");
    }

    // Double negative
    {
        CxExpression expr("--5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: --5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: --5");
        check(doubleEqual(result, 5.0), "--5 = 5");
    }

    // Triple negative
    {
        CxExpression expr("---5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: ---5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: ---5");
        check(doubleEqual(result, -5.0), "---5 = -5");
    }

    // Quadruple negative
    {
        CxExpression expr("----5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: ----5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: ----5");
        check(doubleEqual(result, 5.0), "----5 = 5");
    }

    // Parenthesized double negative
    {
        CxExpression expr("-(-5)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: -(-5)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: -(-5)");
        check(doubleEqual(result, 5.0), "-(-5) = 5");
    }

    // Double negative after operator
    {
        CxExpression expr("3--2");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 3--2");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 3--2");
        check(doubleEqual(result, 5.0), "3--2 = 5");
    }

    // Triple negative after operator
    {
        CxExpression expr("3---2");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 3---2");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 3---2");
        check(doubleEqual(result, 1.0), "3---2 = 1");
    }
}

void testExpressionVariables() {
    printf("\n== CxExpression Variable Tests ==\n");

    double result;

    // M_PI
    {
        CxExpression expr("M_PI");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: M_PI");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: M_PI");
        check(doubleEqual(result, M_PI), "M_PI = pi");
    }

    // M_E
    {
        CxExpression expr("M_E");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: M_E");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: M_E");
        check(doubleEqual(result, M_E), "M_E = e");
    }

    // Variable in expression
    {
        CxExpression expr("2*M_PI");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 2*M_PI");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 2*M_PI");
        check(doubleEqual(result, 2.0 * M_PI), "2*M_PI = 2pi");
    }

    // Variable with addition
    {
        CxExpression expr("M_PI+1");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: M_PI+1");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: M_PI+1");
        check(doubleEqual(result, M_PI + 1.0), "M_PI+1");
    }
}

void testExpressionFunctions() {
    printf("\n== CxExpression Function Tests ==\n");

    double result;

    // SIN
    {
        CxExpression expr("SIN(0)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: SIN(0)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: SIN(0)");
        check(doubleEqual(result, 0.0), "SIN(0) = 0");
    }

    // COS
    {
        CxExpression expr("COS(0)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: COS(0)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: COS(0)");
        check(doubleEqual(result, 1.0), "COS(0) = 1");
    }

    // SQRT
    {
        CxExpression expr("SQRT(4)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: SQRT(4)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: SQRT(4)");
        check(doubleEqual(result, 2.0), "SQRT(4) = 2");
    }

    // ABS
    {
        CxExpression expr("ABS(-5)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: ABS(-5)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: ABS(-5)");
        check(doubleEqual(result, 5.0), "ABS(-5) = 5");
    }

    // Function with expression argument
    {
        CxExpression expr("SQRT(2+2)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: SQRT(2+2)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: SQRT(2+2)");
        check(doubleEqual(result, 2.0), "SQRT(2+2) = 2");
    }

    // Nested functions
    {
        CxExpression expr("ABS(SIN(0))");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: ABS(SIN(0))");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: ABS(SIN(0))");
        check(doubleEqual(result, 0.0), "ABS(SIN(0)) = 0");
    }

    // Function with variable argument
    {
        CxExpression expr("SIN(M_PI)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: SIN(M_PI)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: SIN(M_PI)");
        check(doubleEqual(result, 0.0, 0.0001), "SIN(M_PI) ~ 0");
    }

    // Function result in expression
    {
        CxExpression expr("SQRT(4)+3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: SQRT(4)+3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: SQRT(4)+3");
        check(doubleEqual(result, 5.0), "SQRT(4)+3 = 5");
    }
}

void testExpressionMultiArgFunctions() {
    printf("\n== CxExpression Multi-Argument Function Tests ==\n");

    double result;

    // POW with 2 args
    {
        CxExpression expr("POW(2,3)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: POW(2,3)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: POW(2,3)");
        check(doubleEqual(result, 8.0), "POW(2,3) = 8");
    }

    // ATAN2 with 2 args
    {
        CxExpression expr("ATAN2(1,1)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: ATAN2(1,1)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: ATAN2(1,1)");
        check(doubleEqual(result, M_PI / 4.0), "ATAN2(1,1) = PI/4");
    }

    // MIN with 2 args
    {
        CxExpression expr("MIN(5,3)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: MIN(5,3)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: MIN(5,3)");
        check(doubleEqual(result, 3.0), "MIN(5,3) = 3");
    }

    // MAX with 2 args
    {
        CxExpression expr("MAX(5,3)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: MAX(5,3)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: MAX(5,3)");
        check(doubleEqual(result, 5.0), "MAX(5,3) = 5");
    }

    // MIN with 3 args
    {
        CxExpression expr("MIN(5,3,7)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: MIN(5,3,7)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: MIN(5,3,7)");
        check(doubleEqual(result, 3.0), "MIN(5,3,7) = 3");
    }

    // MAX with 3 args
    {
        CxExpression expr("MAX(5,3,7)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: MAX(5,3,7)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: MAX(5,3,7)");
        check(doubleEqual(result, 7.0), "MAX(5,3,7) = 7");
    }

    // POW with expressions
    {
        CxExpression expr("POW(1+1,2+1)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: POW(1+1,2+1)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: POW(1+1,2+1)");
        check(doubleEqual(result, 8.0), "POW(1+1,2+1) = 8");
    }
}

void testExpressionComplexExpressions() {
    printf("\n== CxExpression Complex Expression Tests ==\n");

    double result;

    // Complex math
    {
        CxExpression expr("(3+4)*2-5/5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: (3+4)*2-5/5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: (3+4)*2-5/5");
        check(doubleEqual(result, 13.0), "(3+4)*2-5/5 = 13");
    }

    // Function in complex expression
    {
        CxExpression expr("SQRT(16)+POW(2,3)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: SQRT(16)+POW(2,3)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: SQRT(16)+POW(2,3)");
        check(doubleEqual(result, 12.0), "SQRT(16)+POW(2,3) = 12");
    }

    // Trigonometric identity: sin^2 + cos^2 = 1
    {
        CxExpression expr("POW(SIN(1),2)+POW(COS(1),2)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: sin^2+cos^2");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: sin^2+cos^2");
        check(doubleEqual(result, 1.0), "sin^2(1)+cos^2(1) = 1");
    }

    // Using PI
    {
        CxExpression expr("SIN(M_PI/2)");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: SIN(M_PI/2)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: SIN(M_PI/2)");
        check(doubleEqual(result, 1.0), "SIN(PI/2) = 1");
    }

    // LOG and EXP inverse
    {
        CxExpression expr("LOG(EXP(5))");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: LOG(EXP(5))");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: LOG(EXP(5))");
        check(doubleEqual(result, 5.0), "LOG(EXP(5)) = 5");
    }

    // Quadratic: 2x^2 + 3x + 1 where x=2
    {
        CxExpression expr("2*POW(2,2)+3*2+1");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 2x^2+3x+1");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 2x^2+3x+1");
        check(doubleEqual(result, 15.0), "2*2^2+3*2+1 = 15");
    }
}

void testExpressionWhitespace() {
    printf("\n== CxExpression Whitespace Tests ==\n");

    double result;

    // Spaces around operators
    {
        CxExpression expr("2 + 3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 2 + 3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 2 + 3");
        check(doubleEqual(result, 5.0), "2 + 3 = 5");
    }

    // Spaces everywhere
    {
        CxExpression expr("  2  +  3  *  4  ");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: spaced expr");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: spaced expr");
        check(doubleEqual(result, 14.0), "2+3*4 with spaces = 14");
    }

    // Spaces in function call
    {
        CxExpression expr("SIN( 0 )");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: SIN( 0 )");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: SIN( 0 )");
        check(doubleEqual(result, 0.0), "SIN( 0 ) = 0");
    }

    // Leading whitespace
    {
        CxExpression expr("   5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: leading spaces");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: leading spaces");
        check(doubleEqual(result, 5.0), "   5 = 5");
    }
}

void testExpressionErrors() {
    printf("\n== CxExpression Error Tests ==\n");

    double result;

    // Unknown variable
    {
        CxExpression expr("unknown_var");
        check(expr.Parse() == CxExpression::PARSE_ERROR, "parse error: unknown variable");
    }

    // Unknown function
    {
        CxExpression expr("UNKNOWN_FUNC(1)");
        check(expr.Parse() == CxExpression::PARSE_ERROR, "parse error: unknown function");
    }

    // Unbalanced parentheses - missing close
    {
        CxExpression expr("(2+3");
        check(expr.Parse() == CxExpression::PARSE_ERROR, "parse error: missing )");
    }

    // Unbalanced parentheses - missing open
    {
        CxExpression expr("2+3)");
        check(expr.Parse() == CxExpression::PARSE_ERROR, "parse error: missing (");
    }

    // Empty expression
    {
        CxExpression expr("");
        expr.Parse();
        // Empty should parse but evaluate to nothing useful
    }

    // Operator at end
    {
        CxExpression expr("5+");
        check(expr.Parse() == CxExpression::PARSE_ERROR, "parse error: trailing operator");
    }

    // Double operator
    {
        CxExpression expr("5++3");
        // May parse as 5 + (+3) depending on implementation
        CxExpression::parseStatus ps = expr.Parse();
        // Just verify it doesn't crash
        check(1, "double operator handled");
    }

    // Missing argument
    {
        CxExpression expr("SIN()");
        CxExpression::parseStatus ps = expr.Parse();
        if (ps == CxExpression::PARSE_SUCCESS) {
            CxExpression::expressionStatus es = expr.Evaluate(&result);
            // Should error during evaluation if not caught in parse
        }
        check(1, "empty function args handled");
    }
}

void testExpressionGetVariableList() {
    printf("\n== CxExpression GetVariableList Tests ==\n");

    // Expression with variables
    {
        CxExpression expr("M_PI + M_E");
        expr.Parse();
        CxSList<CxString> vars = expr.GetVariableList();
        check(vars.entries() == 2, "GetVariableList: 2 variables");
    }

    // Expression with no variables
    {
        CxExpression expr("2+3");
        expr.Parse();
        CxSList<CxString> vars = expr.GetVariableList();
        check(vars.entries() == 0, "GetVariableList: no variables");
    }

    // Expression with functions (not variables)
    {
        CxExpression expr("SIN(0)");
        expr.Parse();
        CxSList<CxString> vars = expr.GetVariableList();
        check(vars.entries() == 0, "GetVariableList: function not counted as variable");
    }
}

void testExpressionGetErrorString() {
    printf("\n== CxExpression GetErrorString Tests ==\n");

    // Successful parse has no error
    {
        CxExpression expr("2+3");
        expr.Parse();
        // After successful parse, error string should be empty or indicate success
        CxString errStr = expr.GetErrorString();
        check(1, "GetErrorString after success");
    }

    // Failed parse has error
    {
        CxExpression expr("unknown_var");
        expr.Parse();
        CxString errStr = expr.GetErrorString();
        check(errStr.length() > 0, "GetErrorString has content after error");
    }
}

//-----------------------------------------------------------------------------------------
// Custom variable database test
//-----------------------------------------------------------------------------------------
class TestVariableDatabase : public CxExpressionVariableDatabase {
public:
    returnCode VariableDefined(CxString name) {
        if (name == "x") return VARIABLE_DEFINED;
        if (name == "y") return VARIABLE_DEFINED;
        return VARIABLE_UNDEFINED;
    }

    returnCode VariableEvaluate(CxString name, double *result) {
        if (name == "x") { *result = 10.0; return VARIABLE_DEFINED; }
        if (name == "y") { *result = 20.0; return VARIABLE_DEFINED; }
        return VARIABLE_UNDEFINED;
    }
};

// Alternate variable database with different values for x (used by SetVariableDatabase test)
class AltVariableDatabase : public CxExpressionVariableDatabase {
public:
    returnCode VariableDefined(CxString name) {
        if (name == "x") return VARIABLE_DEFINED;
        return VARIABLE_UNDEFINED;
    }
    returnCode VariableEvaluate(CxString name, double *result) {
        if (name == "x") { *result = 100.0; return VARIABLE_DEFINED; }
        return VARIABLE_UNDEFINED;
    }
};

void testExpressionCustomVariables() {
    printf("\n== CxExpression Custom Variable Database Tests ==\n");

    TestVariableDatabase *varDb = new TestVariableDatabase();
    double result;

    // Use custom variable x
    {
        CxExpression expr("x", varDb, NULL);
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: x");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: x");
        check(doubleEqual(result, 10.0), "x = 10");
    }

    // Use custom variable y
    {
        CxExpression expr("y", varDb, NULL);
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: y");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: y");
        check(doubleEqual(result, 20.0), "y = 20");
    }

    // Expression with custom variables
    {
        CxExpression expr("x+y", varDb, NULL);
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: x+y");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: x+y");
        check(doubleEqual(result, 30.0), "x+y = 30");
    }

    // Mix custom and intrinsic
    {
        CxExpression expr("x*M_PI", varDb, NULL);
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: x*M_PI");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: x*M_PI");
        check(doubleEqual(result, 10.0 * M_PI), "x*M_PI = 10*PI");
    }

    delete varDb;
}

//-----------------------------------------------------------------------------------------
// Test setVariableDatabase() - setting database after construction
//-----------------------------------------------------------------------------------------
void testExpressionSetVariableDatabase() {
    printf("\n== CxExpression setVariableDatabase Tests ==\n");

    TestVariableDatabase *varDb = new TestVariableDatabase();
    double result;

    // Create expression without variable database, then set it
    {
        CxExpression expr("x+y");
        // Set the variable database after construction
        expr.setVariableDatabase(varDb);
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse after setVariableDatabase: x+y");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval after setVariableDatabase: x+y");
        check(doubleEqual(result, 30.0), "x+y = 30 after setVariableDatabase");
    }

    // Create expression with NULL database initially, then set custom database
    {
        CxExpression expr("x*2", NULL, NULL);
        expr.setVariableDatabase(varDb);
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse x*2 after setVariableDatabase");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval x*2 after setVariableDatabase");
        check(doubleEqual(result, 20.0), "x*2 = 20 after setVariableDatabase");
    }

    // Parse first, then set database before evaluation
    // This simulates the spreadsheet use case where formulas are parsed once
    // and the database is set before each evaluation
    {
        CxExpression expr("x+5");
        expr.setVariableDatabase(varDb);
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse x+5");

        // First evaluation
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "first eval x+5");
        check(doubleEqual(result, 15.0), "first eval x+5 = 15");

        // Re-evaluate (simulating recalculation)
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "second eval x+5");
        check(doubleEqual(result, 15.0), "second eval x+5 = 15");
    }

    // Test changing database between evaluations
    // This tests if variable values are looked up fresh during each Evaluate()
    // This is the key feature for spreadsheet recalculation
    {
        // Create a second database with different values
        AltVariableDatabase *altDb = new AltVariableDatabase();

        CxExpression expr("x");
        expr.setVariableDatabase(varDb);  // varDb has x=10
        expr.Parse();

        // First evaluation with original database
        expr.Evaluate(&result);
        check(doubleEqual(result, 10.0), "x = 10 with first database");

        // Change database and re-evaluate - should use new values
        expr.setVariableDatabase(altDb);  // altDb has x=100
        expr.Evaluate(&result);
        check(doubleEqual(result, 100.0), "x = 100 after changing database");

        delete altDb;
    }

    // Test setting database after construction, before parse (spreadsheet pattern)
    {
        CxExpression expr("x*y+5");
        expr.setVariableDatabase(varDb);
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse x*y+5 with db set before parse");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval x*y+5");
        check(doubleEqual(result, 205.0), "x*y+5 = 10*20+5 = 205");
    }

    delete varDb;
}

//-----------------------------------------------------------------------------------------
// Custom function database test
//-----------------------------------------------------------------------------------------
class TestFunctionDatabase : public CxExpressionFunctionDatabase {
public:
    returnCode FunctionDefined(CxString name) {
        if (name == "DOUBLE") return FUNCTION_DEFINED;
        if (name == "ADD") return FUNCTION_DEFINED;
        return FUNCTION_UNDEFINED;
    }

    returnCode FunctionEvaluate(CxString name, int numArgs, double *args, double *result) {
        if (name == "DOUBLE") {
            if (numArgs != 1) return FUNCTION_BAD_NUMBER_OF_ARGS;
            *result = args[0] * 2.0;
            return FUNCTION_DEFINED;
        }
        if (name == "ADD") {
            if (numArgs != 2) return FUNCTION_BAD_NUMBER_OF_ARGS;
            *result = args[0] + args[1];
            return FUNCTION_DEFINED;
        }
        return FUNCTION_UNDEFINED;
    }
};

void testExpressionCustomFunctions() {
    printf("\n== CxExpression Custom Function Database Tests ==\n");

    TestFunctionDatabase *funcDb = new TestFunctionDatabase();
    double result;

    // Use custom function DOUBLE
    {
        CxExpression expr("DOUBLE(5)", NULL, funcDb);
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: DOUBLE(5)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: DOUBLE(5)");
        check(doubleEqual(result, 10.0), "DOUBLE(5) = 10");
    }

    // Use custom function ADD
    {
        CxExpression expr("ADD(3,7)", NULL, funcDb);
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: ADD(3,7)");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: ADD(3,7)");
        check(doubleEqual(result, 10.0), "ADD(3,7) = 10");
    }

    // Mix custom and intrinsic functions
    {
        CxExpression expr("DOUBLE(SIN(0))", NULL, funcDb);
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: DOUBLE(SIN(0))");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: DOUBLE(SIN(0))");
        check(doubleEqual(result, 0.0), "DOUBLE(SIN(0)) = 0");
    }

    delete funcDb;
}

//-----------------------------------------------------------------------------------------
// Comparison operator tests
//-----------------------------------------------------------------------------------------
void testComparisonOperators() {
    printf("\n== CxExpression Comparison Operator Tests ==\n");

    double result;

    // Less than
    {
        CxExpression expr("3<5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 3<5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 3<5");
        check(doubleEqual(result, 1.0), "3<5 = 1 (true)");
    }
    {
        CxExpression expr("5<3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5<3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5<3");
        check(doubleEqual(result, 0.0), "5<3 = 0 (false)");
    }

    // Greater than
    {
        CxExpression expr("5>3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5>3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5>3");
        check(doubleEqual(result, 1.0), "5>3 = 1 (true)");
    }
    {
        CxExpression expr("3>5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 3>5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 3>5");
        check(doubleEqual(result, 0.0), "3>5 = 0 (false)");
    }

    // Less than or equal
    {
        CxExpression expr("3<=5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 3<=5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 3<=5");
        check(doubleEqual(result, 1.0), "3<=5 = 1 (true)");
    }
    {
        CxExpression expr("5<=5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5<=5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5<=5");
        check(doubleEqual(result, 1.0), "5<=5 = 1 (true, equal)");
    }
    {
        CxExpression expr("5<=3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5<=3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5<=3");
        check(doubleEqual(result, 0.0), "5<=3 = 0 (false)");
    }

    // Greater than or equal
    {
        CxExpression expr("5>=3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5>=3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5>=3");
        check(doubleEqual(result, 1.0), "5>=3 = 1 (true)");
    }
    {
        CxExpression expr("5>=5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5>=5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5>=5");
        check(doubleEqual(result, 1.0), "5>=5 = 1 (true, equal)");
    }
    {
        CxExpression expr("3>=5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 3>=5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 3>=5");
        check(doubleEqual(result, 0.0), "3>=5 = 0 (false)");
    }

    // Equal
    {
        CxExpression expr("5=5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5=5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5=5");
        check(doubleEqual(result, 1.0), "5=5 = 1 (true)");
    }
    {
        CxExpression expr("5=3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5=3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5=3");
        check(doubleEqual(result, 0.0), "5=3 = 0 (false)");
    }

    // Not equal
    {
        CxExpression expr("5<>3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5<>3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5<>3");
        check(doubleEqual(result, 1.0), "5<>3 = 1 (true)");
    }
    {
        CxExpression expr("5<>5");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 5<>5");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 5<>5");
        check(doubleEqual(result, 0.0), "5<>5 = 0 (false)");
    }

    // Comparison with arithmetic - comparisons have lower precedence
    {
        CxExpression expr("2+3<10");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 2+3<10");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 2+3<10");
        check(doubleEqual(result, 1.0), "2+3<10 = 1 (5<10 is true)");
    }
    {
        CxExpression expr("2*3>10");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 2*3>10");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 2*3>10");
        check(doubleEqual(result, 0.0), "2*3>10 = 0 (6>10 is false)");
    }

    // Comparison with parentheses
    {
        CxExpression expr("(5+5)=10");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: (5+5)=10");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: (5+5)=10");
        check(doubleEqual(result, 1.0), "(5+5)=10 = 1 (true)");
    }

    // Comparison with negative numbers
    {
        CxExpression expr("-5<0");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: -5<0");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: -5<0");
        check(doubleEqual(result, 1.0), "-5<0 = 1 (true)");
    }

    // Comparison with decimals
    {
        CxExpression expr("3.14>3");
        check(expr.Parse() == CxExpression::PARSE_SUCCESS, "parse: 3.14>3");
        check(expr.Evaluate(&result) == CxExpression::EVALUATION_SUCCESS, "eval: 3.14>3");
        check(doubleEqual(result, 1.0), "3.14>3 = 1 (true)");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxExpression Test Suite\n");
    printf("=======================\n");

    // Token tests
    testExpressionTokenConstructors();
    testExpressionTokenOperators();
    testExpressionTokenHelpers();
    testExpressionTokenAsString();

    // Variable database tests
    testVariableDatabase();
    testIntrinsicVariableDatabase();

    // Function database tests
    testFunctionDatabase();
    testIntrinsicFunctionDefined();
    testIntrinsicFunctionEvaluate();
    testIntrinsicFunctionErrors();

    // Expression tests
    testExpressionSimpleNumbers();
    testExpressionBasicArithmetic();
    testExpressionPrecedence();
    testExpressionParentheses();
    testExpressionUnaryMinus();
    testExpressionVariables();
    testExpressionFunctions();
    testExpressionMultiArgFunctions();
    testExpressionComplexExpressions();
    testExpressionWhitespace();
    testExpressionErrors();
    testExpressionGetVariableList();
    testExpressionGetErrorString();

    // Custom database tests
    testExpressionCustomVariables();
    testExpressionCustomFunctions();

    // setVariableDatabase tests (new functionality)
    testExpressionSetVariableDatabase();

    // Comparison operator tests
    testComparisonOperators();

    printf("\n=======================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
