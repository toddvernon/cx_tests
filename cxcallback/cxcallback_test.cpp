//-----------------------------------------------------------------------------------------
// cxcallback_test.cpp - CxCountedBody, CxFunctor, and CxCallback unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <cx/base/cntbody.h>
#include <cx/base/callback.h>
#include <cx/functor/functor.h>

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
// Global tracking for callback invocations
//-----------------------------------------------------------------------------------------
static int gFunctionCallCount = 0;
static int gLastFunctionArg = 0;
static int gMemberCallCount = 0;
static int gLastMemberArg = 0;

void resetCallCounters() {
    gFunctionCallCount = 0;
    gLastFunctionArg = 0;
    gMemberCallCount = 0;
    gLastMemberArg = 0;
}

//-----------------------------------------------------------------------------------------
// Test function for function callbacks
//-----------------------------------------------------------------------------------------
void testFunction(int arg) {
    gFunctionCallCount++;
    gLastFunctionArg = arg;
}

void testFunctionDouble(int arg) {
    gFunctionCallCount++;
    gLastFunctionArg = arg * 2;
}

void testFunctionTriple(int arg) {
    gFunctionCallCount++;
    gLastFunctionArg = arg * 3;
}

//-----------------------------------------------------------------------------------------
// Test class for member callbacks
//-----------------------------------------------------------------------------------------
class CallbackReceiver {
public:
    int _value;
    int _callCount;

    CallbackReceiver() : _value(0), _callCount(0) {}

    void onCallback(int arg) {
        _callCount++;
        _value = arg;
        gMemberCallCount++;
        gLastMemberArg = arg;
    }

    void onCallbackDouble(int arg) {
        _callCount++;
        _value = arg * 2;
        gMemberCallCount++;
        gLastMemberArg = arg * 2;
    }

    void onCallbackAccumulate(int arg) {
        _callCount++;
        _value += arg;
        gMemberCallCount++;
        gLastMemberArg = _value;
    }
};

//-----------------------------------------------------------------------------------------
// Derived CxFunctor for testing
//-----------------------------------------------------------------------------------------
class TestFunctor : public CxFunctor {
public:
    int _callCount;
    int _value;

    TestFunctor() : _callCount(0), _value(0) {}
    TestFunctor(int initialValue) : _callCount(0), _value(initialValue) {}

    void operator()() {
        _callCount++;
        _value++;
    }
};

class AccumulatorFunctor : public CxFunctor {
public:
    int _total;

    AccumulatorFunctor() : _total(0) {}

    void operator()() {
        _total += 10;
    }
};

//-----------------------------------------------------------------------------------------
// CxCountedBody tests
//-----------------------------------------------------------------------------------------
void testCountedBody() {
    printf("\n== CxCountedBody Tests ==\n");

    // Initial count is 0
    {
        CxCountedBody cb;
        check(cb.count() == 0, "initial count is 0");
    }

    // incCount increments
    {
        CxCountedBody cb;
        cb.incCount();
        check(cb.count() == 1, "incCount increments to 1");
    }

    // Multiple incCount calls
    {
        CxCountedBody cb;
        cb.incCount();
        cb.incCount();
        cb.incCount();
        check(cb.count() == 3, "three incCount calls gives count of 3");
    }

    // decCount decrements
    {
        CxCountedBody cb;
        cb.incCount();
        cb.incCount();
        cb.decCount();
        check(cb.count() == 1, "decCount decrements");
    }

    // decCount to zero
    {
        CxCountedBody cb;
        cb.incCount();
        cb.decCount();
        check(cb.count() == 0, "decCount to zero");
    }

    // Mixed inc/dec operations
    {
        CxCountedBody cb;
        cb.incCount();
        cb.incCount();
        cb.decCount();
        cb.incCount();
        cb.incCount();
        cb.decCount();
        check(cb.count() == 2, "mixed inc/dec gives correct count");
    }
}

//-----------------------------------------------------------------------------------------
// CxFunctor tests
//-----------------------------------------------------------------------------------------
void testFunctor() {
    printf("\n== CxFunctor Tests ==\n");

    // TestFunctor default constructor
    {
        TestFunctor f;
        check(f._callCount == 0, "functor default ctor: callCount is 0");
        check(f._value == 0, "functor default ctor: value is 0");
    }

    // TestFunctor with initial value
    {
        TestFunctor f(42);
        check(f._value == 42, "functor with initial value");
    }

    // Invoking functor
    {
        TestFunctor f;
        f();
        check(f._callCount == 1, "functor invocation increments callCount");
        check(f._value == 1, "functor invocation increments value");
    }

    // Multiple invocations
    {
        TestFunctor f;
        f();
        f();
        f();
        check(f._callCount == 3, "multiple invocations: callCount is 3");
        check(f._value == 3, "multiple invocations: value is 3");
    }

    // Functor through base pointer
    {
        TestFunctor tf;
        CxFunctor* f = &tf;
        (*f)();
        (*f)();
        check(tf._callCount == 2, "functor through base pointer works");
    }

    // AccumulatorFunctor
    {
        AccumulatorFunctor f;
        f();
        f();
        f();
        check(f._total == 30, "accumulator functor works");
    }

    // Different functors are independent
    {
        TestFunctor f1;
        TestFunctor f2;
        f1();
        f1();
        f2();
        check(f1._callCount == 2, "functor f1 independent");
        check(f2._callCount == 1, "functor f2 independent");
    }
}

//-----------------------------------------------------------------------------------------
// CxCallback nil callback tests
//-----------------------------------------------------------------------------------------
void testCallbackNil() {
    printf("\n== CxCallback Nil Callback Tests ==\n");

    // Default constructor creates nil callback
    {
        CxCallback<int> cb;
        int threw = 0;
        try {
            cb(42);
        } catch (CxCallbackException& e) {
            threw = 1;
        }
        check(threw == 1, "nil callback throws CxCallbackException");
    }

    // Multiple nil callbacks are independent
    {
        CxCallback<int> cb1;
        CxCallback<int> cb2;
        int threw1 = 0, threw2 = 0;
        try { cb1(1); } catch (CxCallbackException&) { threw1 = 1; }
        try { cb2(2); } catch (CxCallbackException&) { threw2 = 1; }
        check(threw1 == 1 && threw2 == 1, "multiple nil callbacks both throw");
    }
}

//-----------------------------------------------------------------------------------------
// CxCallback function callback tests
//-----------------------------------------------------------------------------------------
void testCallbackFunction() {
    printf("\n== CxCallback Function Callback Tests ==\n");

    // Create function callback using make_callback
    {
        resetCallCounters();
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, testFunction);
        cb(42);
        check(gFunctionCallCount == 1, "function callback invokes function");
        check(gLastFunctionArg == 42, "function callback passes argument");
    }

    // Multiple invocations
    {
        resetCallCounters();
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, testFunction);
        cb(1);
        cb(2);
        cb(3);
        check(gFunctionCallCount == 3, "function callback invoked 3 times");
        check(gLastFunctionArg == 3, "function callback last arg is 3");
    }

    // Different functions
    {
        resetCallCounters();
        CxCallback<int> cb1 = make_callback((CxCallback<int>*)0, testFunction);
        CxCallback<int> cb2 = make_callback((CxCallback<int>*)0, testFunctionDouble);
        cb1(5);
        check(gLastFunctionArg == 5, "first function: arg unchanged");
        cb2(5);
        check(gLastFunctionArg == 10, "second function: arg doubled");
    }

    // Function callback does not throw
    {
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, testFunction);
        int threw = 0;
        try {
            cb(100);
        } catch (...) {
            threw = 1;
        }
        check(threw == 0, "function callback does not throw");
    }
}

//-----------------------------------------------------------------------------------------
// CxCallback member callback tests
//-----------------------------------------------------------------------------------------
void testCallbackMember() {
    printf("\n== CxCallback Member Callback Tests ==\n");

    // Create member callback using make_callback
    {
        resetCallCounters();
        CallbackReceiver receiver;
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, receiver, &CallbackReceiver::onCallback);
        cb(99);
        check(receiver._callCount == 1, "member callback invokes member");
        check(receiver._value == 99, "member callback passes argument");
    }

    // Multiple invocations
    {
        resetCallCounters();
        CallbackReceiver receiver;
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, receiver, &CallbackReceiver::onCallback);
        cb(10);
        cb(20);
        cb(30);
        check(receiver._callCount == 3, "member callback invoked 3 times");
        check(receiver._value == 30, "member callback last value is 30");
    }

    // Different member functions
    {
        CallbackReceiver receiver;
        CxCallback<int> cb1 = make_callback((CxCallback<int>*)0, receiver, &CallbackReceiver::onCallback);
        CxCallback<int> cb2 = make_callback((CxCallback<int>*)0, receiver, &CallbackReceiver::onCallbackDouble);
        cb1(5);
        check(receiver._value == 5, "first member: arg unchanged");
        cb2(5);
        check(receiver._value == 10, "second member: arg doubled");
    }

    // Accumulator member callback
    {
        CallbackReceiver receiver;
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, receiver, &CallbackReceiver::onCallbackAccumulate);
        cb(10);
        cb(20);
        cb(30);
        check(receiver._value == 60, "accumulator member callback: 10+20+30=60");
    }

    // Multiple receivers
    {
        CallbackReceiver r1, r2;
        CxCallback<int> cb1 = make_callback((CxCallback<int>*)0, r1, &CallbackReceiver::onCallback);
        CxCallback<int> cb2 = make_callback((CxCallback<int>*)0, r2, &CallbackReceiver::onCallback);
        cb1(100);
        cb2(200);
        check(r1._value == 100, "receiver 1 got 100");
        check(r2._value == 200, "receiver 2 got 200");
    }
}

//-----------------------------------------------------------------------------------------
// CxCallback copy constructor tests
//-----------------------------------------------------------------------------------------
void testCallbackCopy() {
    printf("\n== CxCallback Copy Constructor Tests ==\n");

    // Copy constructor shares body
    {
        resetCallCounters();
        CxCallback<int> cb1 = make_callback((CxCallback<int>*)0, testFunction);
        CxCallback<int> cb2(cb1);
        cb2(42);
        check(gFunctionCallCount == 1, "copy invokes same function");
        check(gLastFunctionArg == 42, "copy passes argument correctly");
    }

    // Both copies work independently
    {
        resetCallCounters();
        CxCallback<int> cb1 = make_callback((CxCallback<int>*)0, testFunction);
        CxCallback<int> cb2(cb1);
        cb1(1);
        cb2(2);
        check(gFunctionCallCount == 2, "both copies invoke function");
    }

    // Copy of nil callback is also nil
    {
        CxCallback<int> cb1;
        CxCallback<int> cb2(cb1);
        int threw = 0;
        try { cb2(42); } catch (CxCallbackException&) { threw = 1; }
        check(threw == 1, "copy of nil callback is also nil");
    }

    // Multiple copies
    {
        resetCallCounters();
        CxCallback<int> cb1 = make_callback((CxCallback<int>*)0, testFunction);
        CxCallback<int> cb2(cb1);
        CxCallback<int> cb3(cb2);
        CxCallback<int> cb4(cb3);
        cb4(100);
        check(gLastFunctionArg == 100, "deeply copied callback works");
    }
}

//-----------------------------------------------------------------------------------------
// CxCallback assignment operator tests
//-----------------------------------------------------------------------------------------
void testCallbackAssignment() {
    printf("\n== CxCallback Assignment Operator Tests ==\n");

    // Assignment replaces callback
    {
        resetCallCounters();
        CxCallback<int> cb;  // nil
        CxCallback<int> cb2 = make_callback((CxCallback<int>*)0, testFunction);
        cb = cb2;
        cb(42);
        check(gFunctionCallCount == 1, "assigned callback invokes function");
    }

    // Nil callback can be assigned over valid callback
    {
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, testFunction);
        CxCallback<int> nilCb;
        cb = nilCb;
        int threw = 0;
        try { cb(42); } catch (CxCallbackException&) { threw = 1; }
        check(threw == 1, "nil assigned over valid throws");
    }

    // Self-assignment (same body)
    {
        resetCallCounters();
        CxCallback<int> cb1 = make_callback((CxCallback<int>*)0, testFunction);
        CxCallback<int> cb2(cb1);  // shares body
        cb1 = cb2;  // Same body assignment
        cb1(42);
        check(gLastFunctionArg == 42, "same-body assignment works");
    }

    // Chain assignment
    {
        resetCallCounters();
        CxCallback<int> cb1 = make_callback((CxCallback<int>*)0, testFunction);
        CxCallback<int> cb2;
        CxCallback<int> cb3;
        cb3 = cb2 = cb1;
        cb3(77);
        check(gLastFunctionArg == 77, "chain assignment works");
    }

    // Replace function callback with different function
    {
        resetCallCounters();
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, testFunction);
        cb(5);
        check(gLastFunctionArg == 5, "first function: 5");
        cb = make_callback((CxCallback<int>*)0, testFunctionDouble);
        cb(5);
        check(gLastFunctionArg == 10, "replaced with double: 10");
    }
}

//-----------------------------------------------------------------------------------------
// CxCallback with different parameter types
//-----------------------------------------------------------------------------------------
static double gDoubleArg = 0.0;
void doubleCallback(double arg) { gDoubleArg = arg; }

static const char* gStringArg = NULL;
void stringCallback(const char* arg) { gStringArg = arg; }

class Point {
public:
    int x, y;
    Point() : x(0), y(0) {}
    Point(int x_, int y_) : x(x_), y(y_) {}
};

static Point gPointArg;
void pointCallback(Point p) { gPointArg = p; }

void testCallbackTypes() {
    printf("\n== CxCallback Different Types Tests ==\n");

    // Callback with double parameter
    {
        gDoubleArg = 0.0;
        CxCallback<double> cb = make_callback((CxCallback<double>*)0, doubleCallback);
        cb(3.14159);
        check(gDoubleArg > 3.14 && gDoubleArg < 3.15, "double callback works");
    }

    // Callback with const char* parameter
    {
        gStringArg = NULL;
        CxCallback<const char*> cb = make_callback((CxCallback<const char*>*)0, stringCallback);
        cb("Hello");
        check(gStringArg != NULL && strcmp(gStringArg, "Hello") == 0, "string callback works");
    }

    // Callback with class parameter (by value)
    {
        gPointArg.x = 0;
        gPointArg.y = 0;
        CxCallback<Point> cb = make_callback((CxCallback<Point>*)0, pointCallback);
        Point p(10, 20);
        cb(p);
        check(gPointArg.x == 10 && gPointArg.y == 20, "class callback works");
    }
}

//-----------------------------------------------------------------------------------------
// CxCallback lifetime tests
//-----------------------------------------------------------------------------------------
void testCallbackLifetime() {
    printf("\n== CxCallback Lifetime Tests ==\n");

    // Callback survives original going out of scope
    {
        resetCallCounters();
        CxCallback<int> outer;
        {
            CxCallback<int> inner = make_callback((CxCallback<int>*)0, testFunction);
            outer = inner;
        }
        // inner is gone, but outer should still work
        outer(42);
        check(gFunctionCallCount == 1, "callback survives scope");
        check(gLastFunctionArg == 42, "callback works after original gone");
    }

    // Multiple scopes
    {
        resetCallCounters();
        CxCallback<int> level1;
        {
            CxCallback<int> level2;
            {
                CxCallback<int> level3 = make_callback((CxCallback<int>*)0, testFunction);
                level2 = level3;
            }
            level1 = level2;
        }
        level1(99);
        check(gLastFunctionArg == 99, "callback survives multiple scopes");
    }

    // Reassignment releases old body
    {
        resetCallCounters();
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, testFunction);
        cb = make_callback((CxCallback<int>*)0, testFunctionDouble);
        cb = make_callback((CxCallback<int>*)0, testFunctionTriple);
        cb(10);
        check(gLastFunctionArg == 30, "reassignment to triple: 10*3=30");
    }
}

//-----------------------------------------------------------------------------------------
// CxCallback edge cases
//-----------------------------------------------------------------------------------------
void testCallbackEdgeCases() {
    printf("\n== CxCallback Edge Cases Tests ==\n");

    // Zero argument
    {
        resetCallCounters();
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, testFunction);
        cb(0);
        check(gLastFunctionArg == 0, "zero argument works");
    }

    // Negative argument
    {
        resetCallCounters();
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, testFunction);
        cb(-100);
        check(gLastFunctionArg == -100, "negative argument works");
    }

    // Large argument
    {
        resetCallCounters();
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, testFunction);
        cb(2147483647);  // INT_MAX
        check(gLastFunctionArg == 2147483647, "large argument works");
    }

    // Null string argument
    {
        gStringArg = "not null";
        CxCallback<const char*> cb = make_callback((CxCallback<const char*>*)0, stringCallback);
        cb(NULL);
        check(gStringArg == NULL, "null string argument works");
    }

    // Empty string argument
    {
        gStringArg = NULL;
        CxCallback<const char*> cb = make_callback((CxCallback<const char*>*)0, stringCallback);
        cb("");
        check(gStringArg != NULL && gStringArg[0] == '\0', "empty string argument works");
    }
}

//-----------------------------------------------------------------------------------------
// CxCallback stress tests
//-----------------------------------------------------------------------------------------
void testCallbackStress() {
    printf("\n== CxCallback Stress Tests ==\n");

    // Many invocations
    {
        resetCallCounters();
        CxCallback<int> cb = make_callback((CxCallback<int>*)0, testFunction);
        for (int i = 0; i < 1000; i++) {
            cb(i);
        }
        check(gFunctionCallCount == 1000, "1000 invocations");
        check(gLastFunctionArg == 999, "last arg is 999");
    }

    // Many copies
    {
        resetCallCounters();
        CxCallback<int> base = make_callback((CxCallback<int>*)0, testFunction);
        CxCallback<int> copies[100];
        for (int i = 0; i < 100; i++) {
            copies[i] = base;
        }
        copies[99](42);
        check(gLastFunctionArg == 42, "100 copies all reference same body");
    }

    // Many different callbacks
    {
        resetCallCounters();
        for (int i = 0; i < 50; i++) {
            CxCallback<int> cb = make_callback((CxCallback<int>*)0, testFunction);
            cb(i);
        }
        check(gFunctionCallCount == 50, "50 different callbacks");
    }

    // Rapid reassignment
    {
        resetCallCounters();
        CxCallback<int> cb;
        for (int i = 0; i < 100; i++) {
            cb = make_callback((CxCallback<int>*)0, testFunction);
        }
        cb(42);
        check(gLastFunctionArg == 42, "rapid reassignment: final callback works");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxCallback Test Suite\n");
    printf("=====================\n");

    testCountedBody();
    testFunctor();
    testCallbackNil();
    testCallbackFunction();
    testCallbackMember();
    testCallbackCopy();
    testCallbackAssignment();
    testCallbackTypes();
    testCallbackLifetime();
    testCallbackEdgeCases();
    testCallbackStress();

    printf("\n=====================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
