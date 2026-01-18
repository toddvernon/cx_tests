//-----------------------------------------------------------------------------------------
// cxfunctor_test.cpp - CxFunctor, CxDeferCall, and related classes unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <cx/functor/functor.h>
#include <cx/functor/freefunctor.h>
#include <cx/functor/memberfunctor.h>
#include <cx/functor/defercall.h>
#include <cx/functor/reference.h>
#include <cx/functor/simplereferencer.h>

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
// Global variables to track function calls (since functors return void)
//-----------------------------------------------------------------------------------------
static int g_callCount = 0;
static int g_lastValue = 0;
static int g_sum = 0;
static int g_product = 0;

void resetGlobals() {
    g_callCount = 0;
    g_lastValue = 0;
    g_sum = 0;
    g_product = 0;
}

//-----------------------------------------------------------------------------------------
// Free functions for testing (0-6 parameters)
//-----------------------------------------------------------------------------------------
void freeFunc0() {
    g_callCount++;
}

void freeFunc1(int a) {
    g_callCount++;
    g_lastValue = a;
}

void freeFunc2(int a, int b) {
    g_callCount++;
    g_sum = a + b;
}

void freeFunc3(int a, int b, int c) {
    g_callCount++;
    g_sum = a + b + c;
}

void freeFunc4(int a, int b, int c, int d) {
    g_callCount++;
    g_sum = a + b + c + d;
}

void freeFunc5(int a, int b, int c, int d, int e) {
    g_callCount++;
    g_sum = a + b + c + d + e;
}

void freeFunc6(int a, int b, int c, int d, int e, int f) {
    g_callCount++;
    g_sum = a + b + c + d + e + f;
}

// Function that modifies a reference parameter
void freeFuncRef(int& val) {
    val = 999;
}

// Function with return value (return is ignored by functor)
int freeFuncWithReturn() {
    g_callCount++;
    return 42;
}

//-----------------------------------------------------------------------------------------
// Test class for member function testing
//-----------------------------------------------------------------------------------------
class TestClass {
public:
    int _value;
    int _callCount;

    TestClass() : _value(0), _callCount(0) {}

    void memberFunc0() {
        _callCount++;
    }

    void memberFunc1(int a) {
        _callCount++;
        _value = a;
    }

    void memberFunc2(int a, int b) {
        _callCount++;
        _value = a + b;
    }

    void memberFunc3(int a, int b, int c) {
        _callCount++;
        _value = a + b + c;
    }

    void memberFunc4(int a, int b, int c, int d) {
        _callCount++;
        _value = a + b + c + d;
    }

    void memberFunc5(int a, int b, int c, int d, int e) {
        _callCount++;
        _value = a + b + c + d + e;
    }

    void memberFunc6(int a, int b, int c, int d, int e, int f) {
        _callCount++;
        _value = a + b + c + d + e + f;
    }

    // Const member function
    void constMemberFunc0() const {
        g_callCount++;
    }

    void constMemberFunc1(int a) const {
        g_callCount++;
        g_lastValue = a;
    }

    // Member that returns a value
    int memberWithReturn() {
        _callCount++;
        return _value;
    }
};

//-----------------------------------------------------------------------------------------
// CxFreeFunctor tests
//-----------------------------------------------------------------------------------------
void testFreeFunctor0() {
    printf("\n== CxFreeFunctor0 Tests ==\n");

    resetGlobals();

    // Create functor for 0-arg free function
    CxFreeFunctor0<void(*)()> functor(freeFunc0);
    check(g_callCount == 0, "functor created, not yet called");

    // Call via operator()
    functor();
    check(g_callCount == 1, "functor() calls free function");

    // Call again
    functor();
    check(g_callCount == 2, "functor can be called multiple times");

    // Test with function that has return value
    resetGlobals();
    CxFreeFunctor0<int(*)()> functorRet(freeFuncWithReturn);
    functorRet();
    check(g_callCount == 1, "functor with return type works");
}

void testFreeFunctor1() {
    printf("\n== CxFreeFunctor1 Tests ==\n");

    resetGlobals();

    // Create functor with 1 parameter
    CxFreeFunctor1<void(*)(int), int> functor(freeFunc1, 42);
    functor();
    check(g_callCount == 1, "functor1 called");
    check(g_lastValue == 42, "functor1 passed parameter correctly");

    // Different parameter value
    CxFreeFunctor1<void(*)(int), int> functor2(freeFunc1, 100);
    functor2();
    check(g_lastValue == 100, "functor1 with different value works");
}

void testFreeFunctor2() {
    printf("\n== CxFreeFunctor2 Tests ==\n");

    resetGlobals();

    CxFreeFunctor2<void(*)(int,int), int, int> functor(freeFunc2, 10, 20);
    functor();
    check(g_callCount == 1, "functor2 called");
    check(g_sum == 30, "functor2 passed both parameters");
}

void testFreeFunctor3() {
    printf("\n== CxFreeFunctor3 Tests ==\n");

    resetGlobals();

    CxFreeFunctor3<void(*)(int,int,int), int, int, int> functor(freeFunc3, 1, 2, 3);
    functor();
    check(g_callCount == 1, "functor3 called");
    check(g_sum == 6, "functor3 passed all parameters (1+2+3=6)");
}

void testFreeFunctor4() {
    printf("\n== CxFreeFunctor4 Tests ==\n");

    resetGlobals();

    CxFreeFunctor4<void(*)(int,int,int,int), int, int, int, int> functor(freeFunc4, 1, 2, 3, 4);
    functor();
    check(g_callCount == 1, "functor4 called");
    check(g_sum == 10, "functor4 passed all parameters (1+2+3+4=10)");
}

void testFreeFunctor5() {
    printf("\n== CxFreeFunctor5 Tests ==\n");

    resetGlobals();

    CxFreeFunctor5<void(*)(int,int,int,int,int), int, int, int, int, int> functor(freeFunc5, 1, 2, 3, 4, 5);
    functor();
    check(g_callCount == 1, "functor5 called");
    check(g_sum == 15, "functor5 passed all parameters (1+2+3+4+5=15)");
}

void testFreeFunctor6() {
    printf("\n== CxFreeFunctor6 Tests ==\n");

    resetGlobals();

    CxFreeFunctor6<void(*)(int,int,int,int,int,int), int, int, int, int, int, int> functor(freeFunc6, 1, 2, 3, 4, 5, 6);
    functor();
    check(g_callCount == 1, "functor6 called");
    check(g_sum == 21, "functor6 passed all parameters (1+2+3+4+5+6=21)");
}

//-----------------------------------------------------------------------------------------
// CxMemberFunctor tests
//-----------------------------------------------------------------------------------------
void testMemberFunctor0() {
    printf("\n== CxMemberFunctor0 Tests ==\n");

    TestClass obj;
    TestClass* pObj = &obj;

    CxMemberFunctor0<TestClass*, void(TestClass::*)()> functor(pObj, &TestClass::memberFunc0);
    check(obj._callCount == 0, "member functor created, not called");

    functor();
    check(obj._callCount == 1, "member functor0 called method");
}

void testMemberFunctor1() {
    printf("\n== CxMemberFunctor1 Tests ==\n");

    TestClass obj;
    TestClass* pObj = &obj;

    CxMemberFunctor1<TestClass*, void(TestClass::*)(int), int> functor(pObj, &TestClass::memberFunc1, 77);
    functor();
    check(obj._callCount == 1, "member functor1 called");
    check(obj._value == 77, "member functor1 passed parameter");
}

void testMemberFunctor2() {
    printf("\n== CxMemberFunctor2 Tests ==\n");

    TestClass obj;
    TestClass* pObj = &obj;

    CxMemberFunctor2<TestClass*, void(TestClass::*)(int,int), int, int> functor(pObj, &TestClass::memberFunc2, 30, 40);
    functor();
    check(obj._callCount == 1, "member functor2 called");
    check(obj._value == 70, "member functor2 passed parameters (30+40=70)");
}

void testMemberFunctor3() {
    printf("\n== CxMemberFunctor3 Tests ==\n");

    TestClass obj;
    TestClass* pObj = &obj;

    CxMemberFunctor3<TestClass*, void(TestClass::*)(int,int,int), int, int, int> functor(
        pObj, &TestClass::memberFunc3, 10, 20, 30);
    functor();
    check(obj._callCount == 1, "member functor3 called");
    check(obj._value == 60, "member functor3 passed all parameters");
}

void testMemberFunctor4() {
    printf("\n== CxMemberFunctor4 Tests ==\n");

    TestClass obj;
    TestClass* pObj = &obj;

    CxMemberFunctor4<TestClass*, void(TestClass::*)(int,int,int,int), int, int, int, int> functor(
        pObj, &TestClass::memberFunc4, 1, 2, 3, 4);
    functor();
    check(obj._callCount == 1, "member functor4 called");
    check(obj._value == 10, "member functor4 passed all parameters");
}

void testMemberFunctor5() {
    printf("\n== CxMemberFunctor5 Tests ==\n");

    TestClass obj;
    TestClass* pObj = &obj;

    CxMemberFunctor5<TestClass*, void(TestClass::*)(int,int,int,int,int), int, int, int, int, int> functor(
        pObj, &TestClass::memberFunc5, 1, 2, 3, 4, 5);
    functor();
    check(obj._callCount == 1, "member functor5 called");
    check(obj._value == 15, "member functor5 passed all parameters");
}

void testMemberFunctor6() {
    printf("\n== CxMemberFunctor6 Tests ==\n");

    TestClass obj;
    TestClass* pObj = &obj;

    CxMemberFunctor6<TestClass*, void(TestClass::*)(int,int,int,int,int,int), int, int, int, int, int, int> functor(
        pObj, &TestClass::memberFunc6, 1, 2, 3, 4, 5, 6);
    functor();
    check(obj._callCount == 1, "member functor6 called");
    check(obj._value == 21, "member functor6 passed all parameters");
}

//-----------------------------------------------------------------------------------------
// CxDeferCall tests (factory function)
//-----------------------------------------------------------------------------------------
void testDeferCallFreeFunc() {
    printf("\n== CxDeferCall Free Function Tests ==\n");

    resetGlobals();

    // 0 args
    {
        CxFunctor* pFunctor = CxDeferCall(freeFunc0);
        check(pFunctor != 0, "CxDeferCall creates functor for 0-arg func");
        (*pFunctor)();
        check(g_callCount == 1, "deferred 0-arg call executed");
        delete pFunctor;
    }

    resetGlobals();

    // 1 arg
    {
        CxFunctor* pFunctor = CxDeferCall(freeFunc1, 55);
        (*pFunctor)();
        check(g_lastValue == 55, "deferred 1-arg call passed parameter");
        delete pFunctor;
    }

    resetGlobals();

    // 2 args
    {
        CxFunctor* pFunctor = CxDeferCall(freeFunc2, 100, 200);
        (*pFunctor)();
        check(g_sum == 300, "deferred 2-arg call passed parameters");
        delete pFunctor;
    }

    resetGlobals();

    // 3 args
    {
        CxFunctor* pFunctor = CxDeferCall(freeFunc3, 10, 20, 30);
        (*pFunctor)();
        check(g_sum == 60, "deferred 3-arg call passed parameters");
        delete pFunctor;
    }

    resetGlobals();

    // 4 args
    {
        CxFunctor* pFunctor = CxDeferCall(freeFunc4, 1, 2, 3, 4);
        (*pFunctor)();
        check(g_sum == 10, "deferred 4-arg call passed parameters");
        delete pFunctor;
    }

    resetGlobals();

    // 5 args
    {
        CxFunctor* pFunctor = CxDeferCall(freeFunc5, 1, 2, 3, 4, 5);
        (*pFunctor)();
        check(g_sum == 15, "deferred 5-arg call passed parameters");
        delete pFunctor;
    }

    resetGlobals();

    // 6 args
    {
        CxFunctor* pFunctor = CxDeferCall(freeFunc6, 1, 2, 3, 4, 5, 6);
        (*pFunctor)();
        check(g_sum == 21, "deferred 6-arg call passed parameters");
        delete pFunctor;
    }
}

void testDeferCallMemberFunc() {
    printf("\n== CxDeferCall Member Function Tests ==\n");

    // 0 args
    {
        TestClass obj;
        CxFunctor* pFunctor = CxDeferCall(&obj, &TestClass::memberFunc0);
        check(pFunctor != 0, "CxDeferCall creates functor for member func");
        (*pFunctor)();
        check(obj._callCount == 1, "deferred member 0-arg call executed");
        delete pFunctor;
    }

    // 1 arg
    {
        TestClass obj;
        CxFunctor* pFunctor = CxDeferCall(&obj, &TestClass::memberFunc1, 88);
        (*pFunctor)();
        check(obj._value == 88, "deferred member 1-arg call passed parameter");
        delete pFunctor;
    }

    // 2 args
    {
        TestClass obj;
        CxFunctor* pFunctor = CxDeferCall(&obj, &TestClass::memberFunc2, 50, 60);
        (*pFunctor)();
        check(obj._value == 110, "deferred member 2-arg call passed parameters");
        delete pFunctor;
    }

    // 3 args
    {
        TestClass obj;
        CxFunctor* pFunctor = CxDeferCall(&obj, &TestClass::memberFunc3, 10, 20, 30);
        (*pFunctor)();
        check(obj._value == 60, "deferred member 3-arg call passed parameters");
        delete pFunctor;
    }

    // 4 args
    {
        TestClass obj;
        CxFunctor* pFunctor = CxDeferCall(&obj, &TestClass::memberFunc4, 1, 2, 3, 4);
        (*pFunctor)();
        check(obj._value == 10, "deferred member 4-arg call passed parameters");
        delete pFunctor;
    }

    // 5 args
    {
        TestClass obj;
        CxFunctor* pFunctor = CxDeferCall(&obj, &TestClass::memberFunc5, 1, 2, 3, 4, 5);
        (*pFunctor)();
        check(obj._value == 15, "deferred member 5-arg call passed parameters");
        delete pFunctor;
    }

    // 6 args
    {
        TestClass obj;
        CxFunctor* pFunctor = CxDeferCall(&obj, &TestClass::memberFunc6, 1, 2, 3, 4, 5, 6);
        (*pFunctor)();
        check(obj._value == 21, "deferred member 6-arg call passed parameters");
        delete pFunctor;
    }
}

void testDeferCallConstMemberFunc() {
    printf("\n== CxDeferCall Const Member Function Tests ==\n");

    resetGlobals();

    // 0 args const
    {
        TestClass obj;
        CxFunctor* pFunctor = CxDeferCall(&obj, &TestClass::constMemberFunc0);
        (*pFunctor)();
        check(g_callCount == 1, "deferred const member 0-arg call executed");
        delete pFunctor;
    }

    resetGlobals();

    // 1 arg const
    {
        TestClass obj;
        CxFunctor* pFunctor = CxDeferCall(&obj, &TestClass::constMemberFunc1, 123);
        (*pFunctor)();
        check(g_lastValue == 123, "deferred const member 1-arg call passed parameter");
        delete pFunctor;
    }
}

//-----------------------------------------------------------------------------------------
// CxSimpleReferencer tests
//-----------------------------------------------------------------------------------------
void testSimpleReferencer() {
    printf("\n== CxSimpleReferencer Tests ==\n");

    int value = 42;
    CxSimpleReferencer<int> ref(value);

    // Conversion operator
    int& retrieved = ref;
    check(retrieved == 42, "CxSimpleReferencer stores reference");

    // Modify through reference
    retrieved = 100;
    check(value == 100, "modification through referencer affects original");

    // Original modification reflects
    value = 200;
    int& retrieved2 = ref;
    check(retrieved2 == 200, "referencer sees changes to original");
}

//-----------------------------------------------------------------------------------------
// CxReference tests
//-----------------------------------------------------------------------------------------
void testReference() {
    printf("\n== CxReference Tests ==\n");

    int value = 50;
    CxSimpleReferencer<int> ref = CxReference(value);

    int& retrieved = ref;
    check(retrieved == 50, "CxReference creates valid referencer");

    retrieved = 75;
    check(value == 75, "modification through CxReference affects original");
}

//-----------------------------------------------------------------------------------------
// Polymorphism tests (using CxFunctor base class)
//-----------------------------------------------------------------------------------------
void testFunctorPolymorphism() {
    printf("\n== CxFunctor Polymorphism Tests ==\n");

    resetGlobals();

    // Create different functors and call through base pointer
    CxFunctor* functors[3];

    functors[0] = CxDeferCall(freeFunc0);
    functors[1] = CxDeferCall(freeFunc1, 10);
    functors[2] = CxDeferCall(freeFunc2, 5, 5);

    // Call all through base class
    for (int i = 0; i < 3; i++) {
        (*functors[i])();
    }

    check(g_callCount == 3, "all functors called through base pointer");
    check(g_sum == 10, "last functor result correct");

    // Clean up
    for (int i = 0; i < 3; i++) {
        delete functors[i];
    }
}

//-----------------------------------------------------------------------------------------
// Edge case tests
//-----------------------------------------------------------------------------------------
void testEdgeCases() {
    printf("\n== Edge Case Tests ==\n");

    // Functor with reference parameter using CxReference
    {
        int value = 0;
        CxFunctor* pFunctor = CxDeferCall(freeFuncRef, CxReference(value));
        (*pFunctor)();
        check(value == 999, "functor with reference parameter modifies original");
        delete pFunctor;
    }

    // Multiple calls to same functor
    {
        resetGlobals();
        CxFunctor* pFunctor = CxDeferCall(freeFunc0);
        for (int i = 0; i < 5; i++) {
            (*pFunctor)();
        }
        check(g_callCount == 5, "functor can be called multiple times");
        delete pFunctor;
    }

    // Functor for member function with pointer stored elsewhere
    {
        TestClass* pObj = new TestClass();
        CxFunctor* pFunctor = CxDeferCall(pObj, &TestClass::memberFunc1, 42);
        (*pFunctor)();
        check(pObj->_value == 42, "functor works with heap-allocated object");
        delete pFunctor;
        delete pObj;
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxFunctor Test Suite\n");
    printf("====================\n");

    // Free functor tests
    testFreeFunctor0();
    testFreeFunctor1();
    testFreeFunctor2();
    testFreeFunctor3();
    testFreeFunctor4();
    testFreeFunctor5();
    testFreeFunctor6();

    // Member functor tests
    testMemberFunctor0();
    testMemberFunctor1();
    testMemberFunctor2();
    testMemberFunctor3();
    testMemberFunctor4();
    testMemberFunctor5();
    testMemberFunctor6();

    // CxDeferCall tests
    testDeferCallFreeFunc();
    testDeferCallMemberFunc();
    testDeferCallConstMemberFunc();

    // Reference tests
    testSimpleReferencer();
    testReference();

    // Polymorphism tests
    testFunctorPolymorphism();

    // Edge cases
    testEdgeCases();

    printf("\n====================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
