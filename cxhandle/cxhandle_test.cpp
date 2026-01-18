//-----------------------------------------------------------------------------------------
// cxhandle_test.cpp - CxCounted, CxCountedPtr, and CxHandle unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <cx/base/handle.h>

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
// Test class for tracking construction/destruction
//-----------------------------------------------------------------------------------------
static int gConstructCount = 0;
static int gDestructCount = 0;
static int gLastDestructedId = -1;

class TestObject {
public:
    int _id;
    int _value;

    TestObject() : _id(gConstructCount++), _value(0) {
    }

    TestObject(int value) : _id(gConstructCount++), _value(value) {
    }

    ~TestObject() {
        gLastDestructedId = _id;
        gDestructCount++;
    }

    int getValue() const { return _value; }
    void setValue(int v) { _value = v; }
};

void resetCounters() {
    gConstructCount = 0;
    gDestructCount = 0;
    gLastDestructedId = -1;
}

//-----------------------------------------------------------------------------------------
// Another test class (for multiple types)
//-----------------------------------------------------------------------------------------
class SimpleObject {
public:
    int data;
    SimpleObject() : data(0) {}
    SimpleObject(int d) : data(d) {}
};

//-----------------------------------------------------------------------------------------
// Derived CxCounted class for testing (since CxCounted ctor is protected)
//-----------------------------------------------------------------------------------------
class TestCounted : public CxCounted {
public:
    TestCounted() : CxCounted() {}
};

//-----------------------------------------------------------------------------------------
// CxCounted tests
//-----------------------------------------------------------------------------------------
void testCxCounted() {
    printf("\n== CxCounted Tests ==\n");

    // Add increments count
    {
        TestCounted c;
        unsigned count = c.Add();
        check(count == 1, "Add() returns 1 first time");
    }

    // Multiple Add calls
    {
        TestCounted c;
        c.Add();
        c.Add();
        unsigned count = c.Add();
        check(count == 3, "Add() returns 3 after three calls");
    }

    // Remove decrements count
    {
        TestCounted c;
        c.Add();
        c.Add();
        c.Add();
        unsigned count = c.Remove();
        check(count == 2, "Remove() returns 2 after Add x3, Remove x1");
    }

    // Remove to zero
    {
        TestCounted c;
        c.Add();
        unsigned count = c.Remove();
        check(count == 0, "Remove() returns 0 when count reaches zero");
    }

    // Reset sets count to zero
    {
        TestCounted c;
        c.Add();
        c.Add();
        c.Add();
        c.Reset();
        unsigned count = c.Add();
        check(count == 1, "Reset() sets count to 0, next Add returns 1");
    }

    // Add/Remove sequence
    {
        TestCounted c;
        c.Add();
        c.Add();
        c.Remove();
        c.Add();
        c.Add();
        unsigned count = c.Remove();
        check(count == 2, "Add/Remove sequence maintains correct count");
    }
}

//-----------------------------------------------------------------------------------------
// CxCountedPtr tests
//-----------------------------------------------------------------------------------------
void testCxCountedPtr() {
    printf("\n== CxCountedPtr Tests ==\n");

    // Default constructor initializes to NULL
    {
        CxCountedPtr cp;
        check(cp._pData == NULL, "default ctor sets _pData to NULL");
    }

    // Pointer constructor
    {
        int value = 42;
        CxCountedPtr cp(&value);
        check(cp._pData == &value, "pointer ctor sets _pData correctly");
    }

    // CxCountedPtr inherits Add/Remove
    {
        CxCountedPtr cp;
        cp.Add();
        cp.Add();
        unsigned count = cp.Remove();
        check(count == 1, "CxCountedPtr inherits Add/Remove from CxCounted");
    }

    // CxCountedPtr with dynamic allocation
    {
        int* p = new int(100);
        CxCountedPtr cp(p);
        check(*((int*)cp._pData) == 100, "CxCountedPtr holds dynamic pointer");
        delete p;
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle default constructor tests
//-----------------------------------------------------------------------------------------
void testHandleDefaultCtor() {
    printf("\n== CxHandle Default Constructor Tests ==\n");

    // Default constructor creates null handle
    {
        CxHandle<TestObject> h;
        check(h.IsNull(), "default ctor creates null handle");
    }

    // Multiple default handles are independent
    {
        CxHandle<TestObject> h1;
        CxHandle<TestObject> h2;
        check(h1.IsNull() && h2.IsNull(), "multiple default handles are all null");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle pointer constructor tests
//-----------------------------------------------------------------------------------------
void testHandlePointerCtor() {
    printf("\n== CxHandle Pointer Constructor Tests ==\n");

    resetCounters();

    // Pointer constructor takes ownership
    {
        resetCounters();
        {
            CxHandle<TestObject> h(new TestObject(42));
            check(!h.IsNull(), "pointer ctor creates non-null handle");
            check(gConstructCount == 1, "one object constructed");
        }
        check(gDestructCount == 1, "object destroyed when handle goes out of scope");
    }

    // Access object through handle
    {
        resetCounters();
        CxHandle<TestObject> h(new TestObject(100));
        check(h->getValue() == 100, "can access object through ->");
    }

    // Modify object through handle
    {
        CxHandle<TestObject> h(new TestObject(0));
        h->setValue(999);
        check(h->getValue() == 999, "can modify object through ->");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle copy constructor tests
//-----------------------------------------------------------------------------------------
void testHandleCopyCtor() {
    printf("\n== CxHandle Copy Constructor Tests ==\n");

    // Copy constructor shares reference
    {
        resetCounters();
        {
            CxHandle<TestObject> h1(new TestObject(50));
            {
                CxHandle<TestObject> h2(h1);
                check(!h2.IsNull(), "copy is non-null");
                check(h2->getValue() == 50, "copy has same value");
                check(gDestructCount == 0, "object not destroyed while both handles exist");
            }
            check(gDestructCount == 0, "object not destroyed while h1 still exists");
        }
        check(gDestructCount == 1, "object destroyed after all handles gone");
    }

    // Modification through copy affects original
    {
        CxHandle<TestObject> h1(new TestObject(10));
        CxHandle<TestObject> h2(h1);
        h2->setValue(20);
        check(h1->getValue() == 20, "modification through copy affects original");
    }

    // Multiple copies
    {
        resetCounters();
        {
            CxHandle<TestObject> h1(new TestObject(1));
            CxHandle<TestObject> h2(h1);
            CxHandle<TestObject> h3(h2);
            CxHandle<TestObject> h4(h3);
            check(gDestructCount == 0, "object alive with 4 handles");
        }
        check(gDestructCount == 1, "object destroyed after all 4 handles gone");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle assignment operator tests
//-----------------------------------------------------------------------------------------
void testHandleAssignment() {
    printf("\n== CxHandle Assignment Operator Tests ==\n");

    // Assignment shares reference
    {
        resetCounters();
        {
            CxHandle<TestObject> h1(new TestObject(100));
            CxHandle<TestObject> h2;
            h2 = h1;
            check(!h2.IsNull(), "assigned handle is non-null");
            check(h2->getValue() == 100, "assigned handle has correct value");
        }
        check(gDestructCount == 1, "object destroyed after handles gone");
    }

    // Assignment releases previous reference
    {
        resetCounters();
        {
            CxHandle<TestObject> h1(new TestObject(1));
            CxHandle<TestObject> h2(new TestObject(2));
            int firstId = h2->_id;
            h2 = h1;  // Should release object 2
            check(gDestructCount == 1, "previous object destroyed on reassignment");
            check(gLastDestructedId == firstId, "correct object was destroyed");
        }
        check(gDestructCount == 2, "both objects destroyed at end");
    }

    // Self-assignment
    {
        resetCounters();
        CxHandle<TestObject> h(new TestObject(42));
        h = h;  // Self-assignment
        check(h->getValue() == 42, "self-assignment preserves value");
        check(gDestructCount == 0, "self-assignment doesn't destroy object");
    }

    // Assignment of handle that already shares same reference
    {
        resetCounters();
        CxHandle<TestObject> h1(new TestObject(99));
        CxHandle<TestObject> h2(h1);  // h2 shares reference with h1
        h1 = h2;  // Should be a no-op since they share same _pCount
        check(h1->getValue() == 99, "shared-ref assignment preserves value");
        check(gDestructCount == 0, "shared-ref assignment doesn't destroy object");
    }

    // Chain assignment
    {
        resetCounters();
        {
            CxHandle<TestObject> h1(new TestObject(1));
            CxHandle<TestObject> h2;
            CxHandle<TestObject> h3;
            h3 = h2 = h1;
            check(h2->getValue() == 1, "chain assignment h2 correct");
            check(h3->getValue() == 1, "chain assignment h3 correct");
        }
        check(gDestructCount == 1, "one object destroyed after chain");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle dereference operator tests
//-----------------------------------------------------------------------------------------
void testHandleDereference() {
    printf("\n== CxHandle Dereference Operator Tests ==\n");

    // operator* returns reference
    {
        CxHandle<TestObject> h(new TestObject(77));
        TestObject& ref = *h;
        check(ref.getValue() == 77, "operator* returns correct reference");
    }

    // Modify through operator*
    {
        CxHandle<TestObject> h(new TestObject(0));
        (*h).setValue(123);
        check(h->getValue() == 123, "can modify through operator*");
    }

    // operator-> works
    {
        CxHandle<TestObject> h(new TestObject(55));
        check(h->getValue() == 55, "operator-> works for method call");
    }

    // operator& returns pointer
    {
        CxHandle<TestObject> h(new TestObject(88));
        TestObject* ptr = &h;
        check(ptr->getValue() == 88, "operator& returns valid pointer");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle implicit conversion tests
//-----------------------------------------------------------------------------------------
void testHandleConversion() {
    printf("\n== CxHandle Conversion Tests ==\n");

    // Implicit conversion to T*
    {
        CxHandle<TestObject> h(new TestObject(42));
        TestObject* ptr = h;  // Implicit conversion
        check(ptr != NULL, "implicit conversion to T* works");
        check(ptr->getValue() == 42, "converted pointer has correct value");
    }

    // NULL handle converts to NULL pointer
    {
        CxHandle<TestObject> h;
        TestObject* ptr = h;
        check(ptr == NULL, "null handle converts to NULL pointer");
    }

    // Can pass handle to function expecting pointer
    {
        CxHandle<TestObject> h(new TestObject(99));
        // This would work: someFunction(h) where someFunction takes TestObject*
        TestObject* p = h;
        check(p->getValue() == 99, "handle usable as pointer parameter");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle IsNull tests
//-----------------------------------------------------------------------------------------
void testHandleIsNull() {
    printf("\n== CxHandle IsNull Tests ==\n");

    // Default handle is null
    {
        CxHandle<TestObject> h;
        check(h.IsNull() == 1, "default handle IsNull returns true");
    }

    // Pointer-constructed handle is not null
    {
        CxHandle<TestObject> h(new TestObject());
        check(h.IsNull() == 0, "pointer handle IsNull returns false");
    }

    // NULL pointer handle is null
    {
        CxHandle<TestObject> h((TestObject*)NULL);
        check(h.IsNull() == 1, "NULL pointer handle IsNull returns true");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle NullCheck tests
//-----------------------------------------------------------------------------------------
void testHandleNullCheck() {
    printf("\n== CxHandle NullCheck Tests ==\n");

    // NullCheck throws on null dereference via operator*
    {
        CxHandle<TestObject> h;
        int threw = 0;
        try {
            TestObject& ref = *h;  // Should throw
            (void)ref;
        } catch (...) {
            threw = 1;
        }
        check(threw == 1, "operator* throws on null handle");
    }

    // NullCheck throws on null access via operator->
    {
        CxHandle<TestObject> h;
        int threw = 0;
        try {
            h->getValue();  // Should throw
        } catch (...) {
            threw = 1;
        }
        check(threw == 1, "operator-> throws on null handle");
    }

    // NullCheck throws on null access via operator&
    {
        CxHandle<TestObject> h;
        int threw = 0;
        try {
            TestObject* p = &h;  // Should throw
            (void)p;
        } catch (...) {
            threw = 1;
        }
        check(threw == 1, "operator& throws on null handle");
    }

    // Valid handle does not throw
    {
        CxHandle<TestObject> h(new TestObject(42));
        int threw = 0;
        try {
            int val = h->getValue();
            (void)val;
        } catch (...) {
            threw = 1;
        }
        check(threw == 0, "valid handle does not throw");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle reference counting tests
//-----------------------------------------------------------------------------------------
void testHandleRefCounting() {
    printf("\n== CxHandle Reference Counting Tests ==\n");

    // Single handle destroys on scope exit
    {
        resetCounters();
        {
            CxHandle<TestObject> h(new TestObject());
        }
        check(gDestructCount == 1, "single handle: object destroyed on scope exit");
    }

    // Two handles keep object alive
    {
        resetCounters();
        CxHandle<TestObject>* h2 = NULL;
        {
            CxHandle<TestObject> h1(new TestObject());
            h2 = new CxHandle<TestObject>(h1);
            check(gDestructCount == 0, "two handles: object alive");
        }
        check(gDestructCount == 0, "h1 gone but h2 still holds reference");
        delete h2;
        check(gDestructCount == 1, "both handles gone: object destroyed");
    }

    // Deep copy chain
    {
        resetCounters();
        {
            CxHandle<TestObject> h1(new TestObject());
            CxHandle<TestObject> h2(h1);
            CxHandle<TestObject> h3(h2);
            CxHandle<TestObject> h4(h3);
            CxHandle<TestObject> h5(h4);
            check(gDestructCount == 0, "5 handles: object alive");
        }
        check(gDestructCount == 1, "all 5 gone: one object destroyed");
    }

    // Assignment chain with different initial objects
    {
        resetCounters();
        {
            CxHandle<TestObject> h1(new TestObject(1));
            CxHandle<TestObject> h2(new TestObject(2));
            CxHandle<TestObject> h3(new TestObject(3));
            check(gConstructCount == 3, "three objects created");
            h2 = h1;  // Object 2 destroyed
            check(gDestructCount == 1, "reassignment destroys object 2");
            h3 = h1;  // Object 3 destroyed
            check(gDestructCount == 2, "reassignment destroys object 3");
        }
        check(gDestructCount == 3, "all objects destroyed at end");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle with different types tests
//-----------------------------------------------------------------------------------------
void testHandleDifferentTypes() {
    printf("\n== CxHandle Different Types Tests ==\n");

    // Handle to SimpleObject
    {
        CxHandle<SimpleObject> h(new SimpleObject(42));
        check(h->data == 42, "handle to SimpleObject works");
    }

    // Handle to int
    {
        CxHandle<int> h(new int(100));
        check(*h == 100, "handle to int works");
    }

    // Handle to char
    {
        CxHandle<char> h(new char('X'));
        check(*h == 'X', "handle to char works");
    }

    // Handle to double
    {
        CxHandle<double> h(new double(3.14));
        check(*h > 3.13 && *h < 3.15, "handle to double works");
    }

    // Handle to array (single element)
    {
        CxHandle<int> h(new int[1]);  // Note: only first element managed
        *h = 999;
        check(*h == 999, "handle to array element works");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle scope and lifetime tests
//-----------------------------------------------------------------------------------------
void testHandleLifetime() {
    printf("\n== CxHandle Lifetime Tests ==\n");

    // Nested scopes
    {
        resetCounters();
        {
            CxHandle<TestObject> h1(new TestObject(1));
            {
                CxHandle<TestObject> h2(h1);
                {
                    CxHandle<TestObject> h3(h2);
                    check(gDestructCount == 0, "nested scopes: all alive");
                }
                check(gDestructCount == 0, "h3 gone, h1 h2 still hold ref");
            }
            check(gDestructCount == 0, "h2 gone, h1 still holds ref");
        }
        check(gDestructCount == 1, "h1 gone: object destroyed");
    }

    // Handle returned from function scope
    {
        resetCounters();
        // Simulating function return by creating in inner scope
        CxHandle<TestObject> outer;
        {
            CxHandle<TestObject> inner(new TestObject(42));
            outer = inner;
            check(gDestructCount == 0, "inner scope: alive");
        }
        check(gDestructCount == 0, "inner gone but outer holds ref");
        check(outer->getValue() == 42, "outer has correct value");
    }

    // Multiple reassignments
    {
        resetCounters();
        CxHandle<TestObject> h(new TestObject(1));
        h = CxHandle<TestObject>(new TestObject(2));
        check(gDestructCount == 1, "first object destroyed on reassign");
        h = CxHandle<TestObject>(new TestObject(3));
        check(gDestructCount == 2, "second object destroyed on reassign");
        h = CxHandle<TestObject>(new TestObject(4));
        check(gDestructCount == 3, "third object destroyed on reassign");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle edge cases
//-----------------------------------------------------------------------------------------
void testHandleEdgeCases() {
    printf("\n== CxHandle Edge Cases ==\n");

    // Handle to NULL then assign valid
    {
        resetCounters();
        CxHandle<TestObject> h;
        check(h.IsNull(), "initially null");
        h = CxHandle<TestObject>(new TestObject(42));
        check(!h.IsNull(), "now valid after assignment");
        check(h->getValue() == 42, "has correct value");
    }

    // Valid handle assigned NULL handle
    {
        resetCounters();
        CxHandle<TestObject> h(new TestObject(42));
        check(!h.IsNull(), "initially valid");
        h = CxHandle<TestObject>();  // Assign null handle
        check(h.IsNull(), "now null after assignment");
        check(gDestructCount == 1, "object was destroyed");
    }

    // Copy null handle
    {
        CxHandle<TestObject> h1;
        CxHandle<TestObject> h2(h1);
        check(h2.IsNull(), "copy of null handle is null");
    }

    // Assign null to null
    {
        CxHandle<TestObject> h1;
        CxHandle<TestObject> h2;
        h1 = h2;
        check(h1.IsNull(), "null assigned to null is still null");
    }

    // Mixed operations
    {
        resetCounters();
        CxHandle<TestObject> h1(new TestObject(1));
        CxHandle<TestObject> h2(new TestObject(2));
        CxHandle<TestObject> h3(h1);
        h1 = h2;
        // Now h3 is only ref to object 1, h1 and h2 ref object 2
        check(h3->getValue() == 1, "h3 still references object 1");
        check(h1->getValue() == 2, "h1 now references object 2");
        check(gDestructCount == 0, "both objects still alive");
    }
}

//-----------------------------------------------------------------------------------------
// CxHandle stress test
//-----------------------------------------------------------------------------------------
void testHandleStress() {
    printf("\n== CxHandle Stress Tests ==\n");

    // Many handles to same object
    {
        resetCounters();
        {
            CxHandle<TestObject> base(new TestObject(42));
            CxHandle<TestObject> handles[100];
            for (int i = 0; i < 100; i++) {
                handles[i] = base;
            }
            check(gDestructCount == 0, "100 handles: object alive");
        }
        check(gDestructCount == 1, "all handles gone: one object destroyed");
    }

    // Many objects each with one handle
    {
        resetCounters();
        {
            CxHandle<TestObject> handles[50];
            for (int i = 0; i < 50; i++) {
                handles[i] = CxHandle<TestObject>(new TestObject(i));
            }
            check(gConstructCount == 50, "50 objects created");
            check(gDestructCount == 0, "all 50 alive");
        }
        check(gDestructCount == 50, "all 50 destroyed");
    }

    // Repeated reassignment
    {
        resetCounters();
        CxHandle<TestObject> h(new TestObject(0));
        for (int i = 1; i <= 20; i++) {
            h = CxHandle<TestObject>(new TestObject(i));
        }
        check(gConstructCount == 21, "21 objects created");
        check(gDestructCount == 20, "20 objects destroyed during reassignment");
        check(h->getValue() == 20, "final value correct");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxHandle Test Suite\n");
    printf("===================\n");

    testCxCounted();
    testCxCountedPtr();
    testHandleDefaultCtor();
    testHandlePointerCtor();
    testHandleCopyCtor();
    testHandleAssignment();
    testHandleDereference();
    testHandleConversion();
    testHandleIsNull();
    testHandleNullCheck();
    testHandleRefCounting();
    testHandleDifferentTypes();
    testHandleLifetime();
    testHandleEdgeCases();
    testHandleStress();

    printf("\n===================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
