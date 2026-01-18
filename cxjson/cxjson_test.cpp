//-----------------------------------------------------------------------------------------
// cxjson_test.cpp - CxJSON unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <cx/base/string.h>
#include <cx/json/json_factory.h>

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
// CxJSONString tests
//-----------------------------------------------------------------------------------------
void testJSONString() {
    printf("\n== CxJSONString Tests ==\n");

    // Default constructor
    {
        CxJSONString s;
        check(s.type() == CxJSONBase::STRING, "default ctor sets STRING type");
        check(s.get().length() == 0, "default ctor creates empty string");
    }

    // Constructor with value
    {
        CxJSONString s("hello");
        check(s.type() == CxJSONBase::STRING, "value ctor sets STRING type");
        check(strcmp(s.get().data(), "hello") == 0, "value ctor stores string");
    }

    // set and get
    {
        CxJSONString s;
        s.set("world");
        check(strcmp(s.get().data(), "world") == 0, "set/get works");
    }

    // Empty string
    {
        CxJSONString s("");
        check(s.get().length() == 0, "empty string works");
    }

    // String with special characters
    {
        CxJSONString s("hello\tworld\n");
        check(s.get().index("\t") != -1, "string with tab");
        check(s.get().index("\n") != -1, "string with newline");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONNumber tests
//-----------------------------------------------------------------------------------------
void testJSONNumber() {
    printf("\n== CxJSONNumber Tests ==\n");

    // Default constructor
    {
        CxJSONNumber n;
        check(n.type() == CxJSONBase::NUMBER, "default ctor sets NUMBER type");
    }

    // Constructor with value
    {
        CxJSONNumber n(42.5);
        check(n.type() == CxJSONBase::NUMBER, "value ctor sets NUMBER type");
        check(fabs(n.get() - 42.5) < 0.001, "value ctor stores number");
    }

    // set and get
    {
        CxJSONNumber n;
        n.set(123.456);
        check(fabs(n.get() - 123.456) < 0.001, "set/get works");
    }

    // Zero
    {
        CxJSONNumber n(0.0);
        check(fabs(n.get()) < 0.001, "zero works");
    }

    // Negative number
    {
        CxJSONNumber n(-99.9);
        check(n.get() < 0, "negative number works");
        check(fabs(n.get() - (-99.9)) < 0.001, "negative value correct");
    }

    // Integer value as double
    {
        CxJSONNumber n(100);
        check(fabs(n.get() - 100.0) < 0.001, "integer value works");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONBoolean tests
//-----------------------------------------------------------------------------------------
void testJSONBoolean() {
    printf("\n== CxJSONBoolean Tests ==\n");

    // Default constructor
    {
        CxJSONBoolean b;
        check(b.type() == CxJSONBase::BOOLEAN, "default ctor sets BOOLEAN type");
    }

    // Constructor with true
    {
        CxJSONBoolean b(1);
        check(b.type() == CxJSONBase::BOOLEAN, "value ctor sets BOOLEAN type");
        check(b.get() == 1, "true value stored");
    }

    // Constructor with false
    {
        CxJSONBoolean b(0);
        check(b.get() == 0, "false value stored");
    }

    // set and get
    {
        CxJSONBoolean b;
        b.set(1);
        check(b.get() == 1, "set true works");
        b.set(0);
        check(b.get() == 0, "set false works");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONNull tests
//-----------------------------------------------------------------------------------------
void testJSONNull() {
    printf("\n== CxJSONNull Tests ==\n");

    // Constructor
    {
        CxJSONNull n;
        check(n.type() == CxJSONBase::JNULL, "ctor sets JNULL type");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONMember tests
//-----------------------------------------------------------------------------------------
void testJSONMember() {
    printf("\n== CxJSONMember Tests ==\n");

    // Default constructor
    {
        CxJSONMember m;
        check(m.var().length() == 0, "default ctor empty var");
        check(m.object() == NULL, "default ctor null object");
    }

    // Constructor with name and value
    {
        CxJSONString* s = new CxJSONString("test value");
        CxJSONMember m("key", s);
        check(strcmp(m.var().data(), "key") == 0, "name stored correctly");
        check(m.object() != NULL, "object not null");
        check(m.object()->type() == CxJSONBase::STRING, "object type correct");
    }

    // Member with number value
    {
        CxJSONNumber* n = new CxJSONNumber(42.0);
        CxJSONMember m("count", n);
        check(strcmp(m.var().data(), "count") == 0, "number member name");
        CxJSONNumber* retrieved = (CxJSONNumber*)m.object();
        check(fabs(retrieved->get() - 42.0) < 0.001, "number member value");
    }

    // removeObject
    {
        CxJSONString* s = new CxJSONString("removable");
        CxJSONMember m("temp", s);
        CxJSONBase* removed = m.removeObject();
        check(removed != NULL, "removeObject returns object");
        check(removed->type() == CxJSONBase::STRING, "removed object type correct");
        check(m.object() == NULL, "object is null after remove");
        delete removed;
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONObject tests
//-----------------------------------------------------------------------------------------
void testJSONObject() {
    printf("\n== CxJSONObject Tests ==\n");

    // Default constructor
    {
        CxJSONObject obj;
        check(obj.type() == CxJSONBase::OBJECT, "ctor sets OBJECT type");
        check(obj.entries() == 0, "default ctor empty object");
    }

    // append and entries
    {
        CxJSONObject obj;
        obj.append(new CxJSONMember("key1", new CxJSONString("value1")));
        check(obj.entries() == 1, "append increases entries");
        obj.append(new CxJSONMember("key2", new CxJSONNumber(42)));
        check(obj.entries() == 2, "append second member");
    }

    // at
    {
        CxJSONObject obj;
        obj.append(new CxJSONMember("first", new CxJSONString("one")));
        obj.append(new CxJSONMember("second", new CxJSONString("two")));

        CxJSONMember* m0 = obj.at(0);
        check(m0 != NULL, "at(0) returns member");
        check(strcmp(m0->var().data(), "first") == 0, "at(0) correct member");

        CxJSONMember* m1 = obj.at(1);
        check(m1 != NULL, "at(1) returns member");
        check(strcmp(m1->var().data(), "second") == 0, "at(1) correct member");
    }

    // find
    {
        CxJSONObject obj;
        obj.append(new CxJSONMember("name", new CxJSONString("John")));
        obj.append(new CxJSONMember("age", new CxJSONNumber(30)));

        CxJSONMember* found = obj.find("name");
        check(found != NULL, "find existing member");
        check(strcmp(found->var().data(), "name") == 0, "find returns correct member");

        CxJSONMember* notFound = obj.find("missing");
        check(notFound == NULL, "find non-existing returns NULL");
    }

    // removeAt
    {
        CxJSONObject obj;
        obj.append(new CxJSONMember("a", new CxJSONString("A")));
        obj.append(new CxJSONMember("b", new CxJSONString("B")));
        obj.append(new CxJSONMember("c", new CxJSONString("C")));

        CxJSONMember* removed = obj.removeAt(1);
        check(removed != NULL, "removeAt returns member");
        check(strcmp(removed->var().data(), "b") == 0, "removeAt correct member");
        check(obj.entries() == 2, "removeAt decreases entries");
        delete removed;

        // Check remaining members
        check(strcmp(obj.at(0)->var().data(), "a") == 0, "first member intact");
        check(strcmp(obj.at(1)->var().data(), "c") == 0, "last member shifted");
    }

    // clear
    {
        CxJSONObject obj;
        obj.append(new CxJSONMember("x", new CxJSONString("X")));
        obj.append(new CxJSONMember("y", new CxJSONString("Y")));
        obj.clear();
        check(obj.entries() == 0, "clear empties object");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONArray tests
//-----------------------------------------------------------------------------------------
void testJSONArray() {
    printf("\n== CxJSONArray Tests ==\n");

    // Default constructor
    {
        CxJSONArray arr;
        check(arr.type() == CxJSONBase::ARRAY, "ctor sets ARRAY type");
        check(arr.entries() == 0, "default ctor empty array");
    }

    // append and entries
    {
        CxJSONArray arr;
        arr.append(new CxJSONString("one"));
        check(arr.entries() == 1, "append increases entries");
        arr.append(new CxJSONString("two"));
        arr.append(new CxJSONString("three"));
        check(arr.entries() == 3, "multiple appends");
    }

    // at
    {
        CxJSONArray arr;
        arr.append(new CxJSONNumber(1));
        arr.append(new CxJSONNumber(2));
        arr.append(new CxJSONNumber(3));

        CxJSONBase* item0 = arr.at(0);
        check(item0 != NULL, "at(0) returns item");
        check(item0->type() == CxJSONBase::NUMBER, "at(0) correct type");

        CxJSONNumber* n0 = (CxJSONNumber*)item0;
        check(fabs(n0->get() - 1.0) < 0.001, "at(0) correct value");

        CxJSONNumber* n2 = (CxJSONNumber*)arr.at(2);
        check(fabs(n2->get() - 3.0) < 0.001, "at(2) correct value");
    }

    // Mixed types in array
    {
        CxJSONArray arr;
        arr.append(new CxJSONString("text"));
        arr.append(new CxJSONNumber(42));
        arr.append(new CxJSONBoolean(1));
        arr.append(new CxJSONNull());

        check(arr.entries() == 4, "mixed array has 4 entries");
        check(arr.at(0)->type() == CxJSONBase::STRING, "first is string");
        check(arr.at(1)->type() == CxJSONBase::NUMBER, "second is number");
        check(arr.at(2)->type() == CxJSONBase::BOOLEAN, "third is boolean");
        check(arr.at(3)->type() == CxJSONBase::JNULL, "fourth is null");
    }

    // clear
    {
        CxJSONArray arr;
        arr.append(new CxJSONString("a"));
        arr.append(new CxJSONString("b"));
        arr.clear();
        check(arr.entries() == 0, "clear empties array");
    }

    // Nested array
    {
        CxJSONArray outer;
        CxJSONArray* inner = new CxJSONArray();
        inner->append(new CxJSONNumber(1));
        inner->append(new CxJSONNumber(2));
        outer.append(inner);
        outer.append(new CxJSONString("after"));

        check(outer.entries() == 2, "outer array has 2 entries");
        check(outer.at(0)->type() == CxJSONBase::ARRAY, "first is nested array");
        CxJSONArray* retrieved = (CxJSONArray*)outer.at(0);
        check(retrieved->entries() == 2, "nested array has 2 entries");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONFactory parse tests
//-----------------------------------------------------------------------------------------
void testJSONFactory() {
    printf("\n== CxJSONFactory Parse Tests ==\n");

    // NOTE: The parser only supports objects and arrays at the top level.
    // Standalone primitives (strings, numbers, booleans, null) return NULL.
    // This is a design decision, not a bug.

    // Standalone primitives return NULL (by design)
    {
        check(CxJSONFactory::parse("\"hello\"") == NULL, "standalone string returns NULL");
        check(CxJSONFactory::parse("42.5") == NULL, "standalone number returns NULL");
        check(CxJSONFactory::parse("true") == NULL, "standalone true returns NULL");
        check(CxJSONFactory::parse("false") == NULL, "standalone false returns NULL");
        check(CxJSONFactory::parse("null") == NULL, "standalone null returns NULL");
    }

    // Parse empty object
    {
        CxJSONBase* result = CxJSONFactory::parse("{}");
        check(result != NULL, "parse empty object not null");
        if (result) {
            check(result->type() == CxJSONBase::OBJECT, "parsed empty object type");
            CxJSONObject* obj = (CxJSONObject*)result;
            check(obj->entries() == 0, "empty object has 0 entries");
            delete result;
        }
    }

    // Parse simple object
    {
        CxJSONBase* result = CxJSONFactory::parse("{\"name\": \"John\", \"age\": 30}");
        check(result != NULL, "parse simple object not null");
        if (result) {
            check(result->type() == CxJSONBase::OBJECT, "parsed object type");
            CxJSONObject* obj = (CxJSONObject*)result;
            check(obj->entries() == 2, "object has 2 members");

            CxJSONMember* name = obj->find("name");
            check(name != NULL, "found name member");
            if (name) {
                CxJSONString* nameVal = (CxJSONString*)name->object();
                check(strcmp(nameVal->get().data(), "John") == 0, "name value correct");
            }

            CxJSONMember* age = obj->find("age");
            check(age != NULL, "found age member");
            if (age) {
                CxJSONNumber* ageVal = (CxJSONNumber*)age->object();
                check(fabs(ageVal->get() - 30) < 0.001, "age value correct");
            }
            delete result;
        }
    }

    // Parse empty array
    {
        CxJSONBase* result = CxJSONFactory::parse("[]");
        check(result != NULL, "parse empty array not null");
        if (result) {
            check(result->type() == CxJSONBase::ARRAY, "parsed empty array type");
            CxJSONArray* arr = (CxJSONArray*)result;
            check(arr->entries() == 0, "empty array has 0 entries");
            delete result;
        }
    }

    // Parse simple array
    {
        CxJSONBase* result = CxJSONFactory::parse("[1, 2, 3]");
        check(result != NULL, "parse simple array not null");
        if (result) {
            check(result->type() == CxJSONBase::ARRAY, "parsed array type");
            CxJSONArray* arr = (CxJSONArray*)result;
            check(arr->entries() == 3, "array has 3 entries");

            CxJSONNumber* n0 = (CxJSONNumber*)arr->at(0);
            check(fabs(n0->get() - 1) < 0.001, "array[0] correct");
            CxJSONNumber* n2 = (CxJSONNumber*)arr->at(2);
            check(fabs(n2->get() - 3) < 0.001, "array[2] correct");
            delete result;
        }
    }

    // Parse mixed array
    {
        CxJSONBase* result = CxJSONFactory::parse("[\"text\", 42, true, null]");
        check(result != NULL, "parse mixed array not null");
        if (result) {
            CxJSONArray* arr = (CxJSONArray*)result;
            check(arr->entries() == 4, "mixed array has 4 entries");
            check(arr->at(0)->type() == CxJSONBase::STRING, "mixed[0] is string");
            check(arr->at(1)->type() == CxJSONBase::NUMBER, "mixed[1] is number");
            check(arr->at(2)->type() == CxJSONBase::BOOLEAN, "mixed[2] is boolean");
            check(arr->at(3)->type() == CxJSONBase::JNULL, "mixed[3] is null");
            delete result;
        }
    }

    // Parse nested object
    {
        CxJSONBase* result = CxJSONFactory::parse(
            "{\"person\": {\"name\": \"Alice\", \"age\": 25}}");
        check(result != NULL, "parse nested object not null");
        if (result) {
            CxJSONObject* obj = (CxJSONObject*)result;
            CxJSONMember* person = obj->find("person");
            check(person != NULL, "found person member");
            if (person) {
                check(person->object()->type() == CxJSONBase::OBJECT, "person is object");
                CxJSONObject* personObj = (CxJSONObject*)person->object();
                check(personObj->entries() == 2, "nested object has 2 members");
                CxJSONMember* name = personObj->find("name");
                check(name != NULL, "found nested name");
            }
            delete result;
        }
    }

    // Parse array in object
    {
        CxJSONBase* result = CxJSONFactory::parse(
            "{\"numbers\": [1, 2, 3]}");
        check(result != NULL, "parse object with array not null");
        if (result) {
            CxJSONObject* obj = (CxJSONObject*)result;
            CxJSONMember* numbers = obj->find("numbers");
            check(numbers != NULL, "found numbers member");
            if (numbers) {
                check(numbers->object()->type() == CxJSONBase::ARRAY, "numbers is array");
                CxJSONArray* arr = (CxJSONArray*)numbers->object();
                check(arr->entries() == 3, "numbers array has 3 entries");
            }
            delete result;
        }
    }

    // Parse object in array
    {
        CxJSONBase* result = CxJSONFactory::parse(
            "[{\"x\": 1}, {\"x\": 2}]");
        check(result != NULL, "parse array of objects not null");
        if (result) {
            CxJSONArray* arr = (CxJSONArray*)result;
            check(arr->entries() == 2, "array has 2 objects");
            check(arr->at(0)->type() == CxJSONBase::OBJECT, "arr[0] is object");
            check(arr->at(1)->type() == CxJSONBase::OBJECT, "arr[1] is object");
            delete result;
        }
    }
}

//-----------------------------------------------------------------------------------------
// Type checking tests
//-----------------------------------------------------------------------------------------
void testTypeChecking() {
    printf("\n== Type Checking Tests ==\n");

    // Verify all types
    {
        CxJSONString s;
        CxJSONNumber n;
        CxJSONBoolean b;
        CxJSONNull nu;
        CxJSONObject obj;
        CxJSONArray arr;

        check(s.type() == CxJSONBase::STRING, "CxJSONString type is STRING");
        check(n.type() == CxJSONBase::NUMBER, "CxJSONNumber type is NUMBER");
        check(b.type() == CxJSONBase::BOOLEAN, "CxJSONBoolean type is BOOLEAN");
        check(nu.type() == CxJSONBase::JNULL, "CxJSONNull type is JNULL");
        check(obj.type() == CxJSONBase::OBJECT, "CxJSONObject type is OBJECT");
        check(arr.type() == CxJSONBase::ARRAY, "CxJSONArray type is ARRAY");
    }

    // Type preserved through base pointer
    {
        CxJSONBase* items[4];
        items[0] = new CxJSONString("test");
        items[1] = new CxJSONNumber(42);
        items[2] = new CxJSONBoolean(1);
        items[3] = new CxJSONNull();

        check(items[0]->type() == CxJSONBase::STRING, "base ptr string type");
        check(items[1]->type() == CxJSONBase::NUMBER, "base ptr number type");
        check(items[2]->type() == CxJSONBase::BOOLEAN, "base ptr boolean type");
        check(items[3]->type() == CxJSONBase::JNULL, "base ptr null type");

        for (int i = 0; i < 4; i++) delete items[i];
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxJSON Test Suite\n");
    printf("=================\n");

    testJSONString();
    testJSONNumber();
    testJSONBoolean();
    testJSONNull();
    testJSONMember();
    testJSONObject();
    testJSONArray();
    testJSONFactory();
    testTypeChecking();

    printf("\n=================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
