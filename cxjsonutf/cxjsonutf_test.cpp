//-----------------------------------------------------------------------------------------
// cxjsonutf_test.cpp - CxJSONUTF (UTF-8 aware JSON) unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <cx/base/string.h>
#include <cx/base/utfstring.h>
#include <cx/json/json_utf_base.h>
#include <cx/json/json_utf_string.h>
#include <cx/json/json_utf_number.h>
#include <cx/json/json_utf_boolean.h>
#include <cx/json/json_utf_null.h>
#include <cx/json/json_utf_member.h>
#include <cx/json/json_utf_object.h>
#include <cx/json/json_utf_array.h>

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
// CxJSONUTFString tests
//-----------------------------------------------------------------------------------------
void testJSONUTFString() {
    printf("\n== CxJSONUTFString Tests ==\n");

    // Default constructor
    {
        CxJSONUTFString s;
        check(s.type() == CxJSONUTFBase::STRING, "default ctor sets STRING type");
        check(s.get().charCount() == 0, "default ctor creates empty string");
    }

    // Constructor with CxUTFString
    {
        CxUTFString us;
        us.fromUTF8Bytes("hello", 5);
        CxJSONUTFString s(us);
        check(s.type() == CxJSONUTFBase::STRING, "CxUTFString ctor sets STRING type");
        check(s.get().charCount() == 5, "CxUTFString ctor stores string");
    }

    // Constructor with C string
    {
        CxJSONUTFString s("world");
        check(s.type() == CxJSONUTFBase::STRING, "char* ctor sets STRING type");
        check(s.get().charCount() == 5, "char* ctor stores string");
    }

    // set and get with CxUTFString
    {
        CxJSONUTFString s;
        CxUTFString us;
        us.fromUTF8Bytes("test", 4);
        s.set(us);
        check(s.get().charCount() == 4, "set/get with CxUTFString works");
    }

    // set and get with C string
    {
        CxJSONUTFString s;
        s.set("testing");
        check(s.get().charCount() == 7, "set/get with char* works");
    }

    // Empty string
    {
        CxJSONUTFString s("");
        check(s.get().charCount() == 0, "empty string works");
    }

    // UTF-8 string - Japanese
    {
        // "Hello" in Japanese: こんにちは (5 characters, 15 bytes)
        const char* japanese = "\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1\xe3\x81\xaf";
        CxJSONUTFString s(japanese);
        check(s.get().charCount() == 5, "Japanese UTF-8 string has 5 characters");
    }

    // UTF-8 string - Emoji
    {
        // Smiley face emoji: U+1F600 (4 bytes in UTF-8)
        const char* emoji = "\xf0\x9f\x98\x80";
        CxJSONUTFString s(emoji);
        check(s.get().charCount() == 1, "single emoji is 1 character");
    }

    // UTF-8 string - Mixed ASCII and UTF-8
    {
        // "Hello " + smiley emoji
        const char* mixed = "Hello \xf0\x9f\x98\x80";
        CxJSONUTFString s(mixed);
        check(s.get().charCount() == 7, "mixed ASCII and emoji: 7 characters");
    }

    // toJsonString escapes special characters
    {
        CxJSONUTFString s("line1\nline2\ttab");
        CxString json = s.toJsonString();
        check(json.index("\\n") != -1, "toJsonString escapes newline");
        check(json.index("\\t") != -1, "toJsonString escapes tab");
    }

    // toJsonString with quotes
    {
        CxJSONUTFString s("say \"hello\"");
        CxString json = s.toJsonString();
        check(json.index("\\\"") != -1, "toJsonString escapes quotes");
    }

    // toJsonString preserves UTF-8
    {
        const char* japanese = "\xe3\x81\x93\xe3\x82\x93";  // こん
        CxJSONUTFString s(japanese);
        CxString json = s.toJsonString();
        // Should contain the UTF-8 bytes (not escaped to \uXXXX for BMP chars)
        check(json.index("\xe3\x81\x93") != -1, "toJsonString preserves UTF-8 bytes");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONUTFNumber tests
//-----------------------------------------------------------------------------------------
void testJSONUTFNumber() {
    printf("\n== CxJSONUTFNumber Tests ==\n");

    // Default constructor
    {
        CxJSONUTFNumber n;
        check(n.type() == CxJSONUTFBase::NUMBER, "default ctor sets NUMBER type");
        check(fabs(n.get()) < 0.001, "default ctor initializes to 0");
    }

    // Constructor with value
    {
        CxJSONUTFNumber n(42.5);
        check(n.type() == CxJSONUTFBase::NUMBER, "value ctor sets NUMBER type");
        check(fabs(n.get() - 42.5) < 0.001, "value ctor stores number");
    }

    // set and get
    {
        CxJSONUTFNumber n;
        n.set(123.456);
        check(fabs(n.get() - 123.456) < 0.001, "set/get works");
    }

    // Negative number
    {
        CxJSONUTFNumber n(-99.9);
        check(n.get() < 0, "negative number works");
    }

    // toJsonString
    {
        CxJSONUTFNumber n(42.5);
        CxString json = n.toJsonString();
        check(json.index("42") != -1, "toJsonString contains number");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONUTFBoolean tests
//-----------------------------------------------------------------------------------------
void testJSONUTFBoolean() {
    printf("\n== CxJSONUTFBoolean Tests ==\n");

    // Default constructor
    {
        CxJSONUTFBoolean b;
        check(b.type() == CxJSONUTFBase::BOOLEAN, "default ctor sets BOOLEAN type");
        check(b.get() == 0, "default ctor is false");
    }

    // Constructor with true
    {
        CxJSONUTFBoolean b(1);
        check(b.get() == 1, "true value stored");
    }

    // Constructor with false
    {
        CxJSONUTFBoolean b(0);
        check(b.get() == 0, "false value stored");
    }

    // set and get
    {
        CxJSONUTFBoolean b;
        b.set(1);
        check(b.get() == 1, "set true works");
        b.set(0);
        check(b.get() == 0, "set false works");
    }

    // toJsonString
    {
        CxJSONUTFBoolean bTrue(1);
        CxJSONUTFBoolean bFalse(0);
        check(strcmp(bTrue.toJsonString().data(), "true") == 0, "true toJsonString");
        check(strcmp(bFalse.toJsonString().data(), "false") == 0, "false toJsonString");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONUTFNull tests
//-----------------------------------------------------------------------------------------
void testJSONUTFNull() {
    printf("\n== CxJSONUTFNull Tests ==\n");

    // Constructor
    {
        CxJSONUTFNull n;
        check(n.type() == CxJSONUTFBase::JNULL, "ctor sets JNULL type");
    }

    // toJsonString
    {
        CxJSONUTFNull n;
        check(strcmp(n.toJsonString().data(), "null") == 0, "toJsonString returns null");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONUTFMember tests
//-----------------------------------------------------------------------------------------
void testJSONUTFMember() {
    printf("\n== CxJSONUTFMember Tests ==\n");

    // Default constructor
    {
        CxJSONUTFMember m;
        check(m.var().charCount() == 0, "default ctor empty var");
        check(m.object() == NULL, "default ctor null object");
    }

    // Constructor with CxUTFString name and value
    {
        CxUTFString key;
        key.fromUTF8Bytes("key", 3);
        CxJSONUTFString* s = new CxJSONUTFString("test value");
        CxJSONUTFMember m(key, s);
        check(m.var().charCount() == 3, "CxUTFString key stored");
        check(m.object() != NULL, "object not null");
        check(m.object()->type() == CxJSONUTFBase::STRING, "object type correct");
    }

    // Constructor with C string name
    {
        CxJSONUTFNumber* n = new CxJSONUTFNumber(42.0);
        CxJSONUTFMember m("count", n);
        check(m.var().charCount() == 5, "char* key stored");
        CxJSONUTFNumber* retrieved = (CxJSONUTFNumber*)m.object();
        check(fabs(retrieved->get() - 42.0) < 0.001, "number value correct");
    }

    // UTF-8 key - Japanese
    {
        // "name" in Japanese: 名前 (2 characters)
        const char* jaKey = "\xe5\x90\x8d\xe5\x89\x8d";
        CxJSONUTFString* s = new CxJSONUTFString("value");
        CxJSONUTFMember m(jaKey, s);
        check(m.var().charCount() == 2, "Japanese key has 2 characters");
    }

    // removeObject
    {
        CxJSONUTFString* s = new CxJSONUTFString("removable");
        CxJSONUTFMember m("temp", s);
        CxJSONUTFBase* removed = m.removeObject();
        check(removed != NULL, "removeObject returns object");
        check(removed->type() == CxJSONUTFBase::STRING, "removed object type correct");
        check(m.object() == NULL, "object is null after remove");
        delete removed;
    }

    // toJsonString
    {
        CxJSONUTFMember m("name", new CxJSONUTFString("John"));
        CxString json = m.toJsonString();
        check(json.index("\"name\"") != -1, "toJsonString contains key");
        check(json.index("\"John\"") != -1, "toJsonString contains value");
        check(json.index(":") != -1, "toJsonString contains colon");
    }

    // toJsonString with UTF-8 key
    {
        const char* jaKey = "\xe5\x90\x8d";  // 名
        CxJSONUTFMember m(jaKey, new CxJSONUTFString("value"));
        CxString json = m.toJsonString();
        check(json.index("\xe5\x90\x8d") != -1, "toJsonString preserves UTF-8 key");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONUTFObject tests
//-----------------------------------------------------------------------------------------
void testJSONUTFObject() {
    printf("\n== CxJSONUTFObject Tests ==\n");

    // Default constructor
    {
        CxJSONUTFObject obj;
        check(obj.type() == CxJSONUTFBase::OBJECT, "ctor sets OBJECT type");
        check(obj.entries() == 0, "default ctor empty object");
    }

    // append and entries
    {
        CxJSONUTFObject obj;
        obj.append(new CxJSONUTFMember("key1", new CxJSONUTFString("value1")));
        check(obj.entries() == 1, "append increases entries");
        obj.append(new CxJSONUTFMember("key2", new CxJSONUTFNumber(42)));
        check(obj.entries() == 2, "append second member");
    }

    // at
    {
        CxJSONUTFObject obj;
        obj.append(new CxJSONUTFMember("first", new CxJSONUTFString("one")));
        obj.append(new CxJSONUTFMember("second", new CxJSONUTFString("two")));

        CxJSONUTFMember* m0 = obj.at(0);
        check(m0 != NULL, "at(0) returns member");
        CxString key0 = m0->var().toBytes();
        check(strcmp(key0.data(), "first") == 0, "at(0) correct member");
    }

    // find with C string
    {
        CxJSONUTFObject obj;
        obj.append(new CxJSONUTFMember("name", new CxJSONUTFString("John")));
        obj.append(new CxJSONUTFMember("age", new CxJSONUTFNumber(30)));

        CxJSONUTFMember* found = obj.find("name");
        check(found != NULL, "find existing member");

        CxJSONUTFMember* notFound = obj.find("missing");
        check(notFound == NULL, "find non-existing returns NULL");
    }

    // find with CxUTFString
    {
        CxJSONUTFObject obj;
        obj.append(new CxJSONUTFMember("test", new CxJSONUTFString("value")));

        CxUTFString key;
        key.fromUTF8Bytes("test", 4);
        CxJSONUTFMember* found = obj.find(key);
        check(found != NULL, "find with CxUTFString works");
    }

    // find with UTF-8 key
    {
        const char* jaKey = "\xe5\x90\x8d";  // 名
        CxJSONUTFObject obj;
        obj.append(new CxJSONUTFMember(jaKey, new CxJSONUTFString("value")));

        CxJSONUTFMember* found = obj.find(jaKey);
        check(found != NULL, "find with UTF-8 key works");
    }

    // removeAt
    {
        CxJSONUTFObject obj;
        obj.append(new CxJSONUTFMember("a", new CxJSONUTFString("A")));
        obj.append(new CxJSONUTFMember("b", new CxJSONUTFString("B")));

        CxJSONUTFMember* removed = obj.removeAt(0);
        check(removed != NULL, "removeAt returns member");
        check(obj.entries() == 1, "removeAt decreases entries");
        delete removed;
    }

    // clear
    {
        CxJSONUTFObject obj;
        obj.append(new CxJSONUTFMember("x", new CxJSONUTFString("X")));
        obj.clear();
        check(obj.entries() == 0, "clear empties object");
    }

    // toJsonString
    {
        CxJSONUTFObject obj;
        obj.append(new CxJSONUTFMember("name", new CxJSONUTFString("John")));
        CxString json = obj.toJsonString();
        check(json.index("{") == 0, "toJsonString starts with {");
        check(json.index("}") != -1, "toJsonString ends with }");
        check(json.index("\"name\"") != -1, "toJsonString contains key");
    }

    // toPrettyJsonString
    {
        CxJSONUTFObject obj;
        obj.append(new CxJSONUTFMember("key", new CxJSONUTFString("value")));
        CxString pretty = obj.toPrettyJsonString();
        check(pretty.index("\n") != -1, "toPrettyJsonString contains newlines");
        check(pretty.index("  ") != -1, "toPrettyJsonString contains indentation");
    }
}

//-----------------------------------------------------------------------------------------
// CxJSONUTFArray tests
//-----------------------------------------------------------------------------------------
void testJSONUTFArray() {
    printf("\n== CxJSONUTFArray Tests ==\n");

    // Default constructor
    {
        CxJSONUTFArray arr;
        check(arr.type() == CxJSONUTFBase::ARRAY, "ctor sets ARRAY type");
        check(arr.entries() == 0, "default ctor empty array");
    }

    // append and entries
    {
        CxJSONUTFArray arr;
        arr.append(new CxJSONUTFString("one"));
        check(arr.entries() == 1, "append increases entries");
        arr.append(new CxJSONUTFString("two"));
        arr.append(new CxJSONUTFString("three"));
        check(arr.entries() == 3, "multiple appends");
    }

    // at
    {
        CxJSONUTFArray arr;
        arr.append(new CxJSONUTFNumber(1));
        arr.append(new CxJSONUTFNumber(2));
        arr.append(new CxJSONUTFNumber(3));

        CxJSONUTFBase* item0 = arr.at(0);
        check(item0 != NULL, "at(0) returns item");
        check(item0->type() == CxJSONUTFBase::NUMBER, "at(0) correct type");
    }

    // Mixed types in array
    {
        CxJSONUTFArray arr;
        arr.append(new CxJSONUTFString("text"));
        arr.append(new CxJSONUTFNumber(42));
        arr.append(new CxJSONUTFBoolean(1));
        arr.append(new CxJSONUTFNull());

        check(arr.entries() == 4, "mixed array has 4 entries");
        check(arr.at(0)->type() == CxJSONUTFBase::STRING, "first is string");
        check(arr.at(1)->type() == CxJSONUTFBase::NUMBER, "second is number");
        check(arr.at(2)->type() == CxJSONUTFBase::BOOLEAN, "third is boolean");
        check(arr.at(3)->type() == CxJSONUTFBase::JNULL, "fourth is null");
    }

    // clear
    {
        CxJSONUTFArray arr;
        arr.append(new CxJSONUTFString("a"));
        arr.append(new CxJSONUTFString("b"));
        arr.clear();
        check(arr.entries() == 0, "clear empties array");
    }

    // Nested array
    {
        CxJSONUTFArray outer;
        CxJSONUTFArray* inner = new CxJSONUTFArray();
        inner->append(new CxJSONUTFNumber(1));
        inner->append(new CxJSONUTFNumber(2));
        outer.append(inner);

        check(outer.entries() == 1, "outer has nested array");
        check(outer.at(0)->type() == CxJSONUTFBase::ARRAY, "nested is array type");
    }

    // Array with UTF-8 strings
    {
        CxJSONUTFArray arr;
        // Japanese: こんにちは
        arr.append(new CxJSONUTFString("\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1\xe3\x81\xaf"));
        // Emoji
        arr.append(new CxJSONUTFString("\xf0\x9f\x98\x80"));

        check(arr.entries() == 2, "array has 2 UTF-8 strings");
        CxJSONUTFString* s0 = (CxJSONUTFString*)arr.at(0);
        check(s0->get().charCount() == 5, "first string has 5 chars");
        CxJSONUTFString* s1 = (CxJSONUTFString*)arr.at(1);
        check(s1->get().charCount() == 1, "second string has 1 char (emoji)");
    }

    // toJsonString
    {
        CxJSONUTFArray arr;
        arr.append(new CxJSONUTFNumber(1));
        arr.append(new CxJSONUTFNumber(2));
        CxString json = arr.toJsonString();
        check(json.index("[") == 0, "toJsonString starts with [");
        check(json.index("]") != -1, "toJsonString ends with ]");
    }

    // toPrettyJsonString
    {
        CxJSONUTFArray arr;
        arr.append(new CxJSONUTFString("a"));
        arr.append(new CxJSONUTFString("b"));
        CxString pretty = arr.toPrettyJsonString();
        check(pretty.index("\n") != -1, "toPrettyJsonString contains newlines");
    }
}

//-----------------------------------------------------------------------------------------
// Serialization tests
//-----------------------------------------------------------------------------------------
void testSerialization() {
    printf("\n== Serialization Tests ==\n");

    // Complex object serialization
    {
        CxJSONUTFObject obj;
        obj.append(new CxJSONUTFMember("name", new CxJSONUTFString("Test")));
        obj.append(new CxJSONUTFMember("count", new CxJSONUTFNumber(42)));
        obj.append(new CxJSONUTFMember("active", new CxJSONUTFBoolean(1)));

        CxJSONUTFArray* arr = new CxJSONUTFArray();
        arr->append(new CxJSONUTFNumber(1));
        arr->append(new CxJSONUTFNumber(2));
        obj.append(new CxJSONUTFMember("items", arr));

        CxString json = obj.toJsonString();
        check(json.index("\"name\"") != -1, "complex obj has name");
        check(json.index("\"count\"") != -1, "complex obj has count");
        check(json.index("\"active\"") != -1, "complex obj has active");
        check(json.index("\"items\"") != -1, "complex obj has items");
        check(json.index("[") != -1, "complex obj has array");
    }

    // UTF-8 content serialization
    {
        CxJSONUTFObject obj;
        // Japanese key: 名前 (name)
        const char* jaKey = "\xe5\x90\x8d\xe5\x89\x8d";
        // Japanese value: 太郎 (Taro)
        const char* jaValue = "\xe5\xa4\xaa\xe9\x83\x8e";
        obj.append(new CxJSONUTFMember(jaKey, new CxJSONUTFString(jaValue)));

        CxString json = obj.toJsonString();
        check(json.index(jaKey) != -1, "serialized JSON contains UTF-8 key");
        check(json.index(jaValue) != -1, "serialized JSON contains UTF-8 value");
    }

    // Pretty print indentation
    {
        CxJSONUTFObject obj;
        CxJSONUTFObject* nested = new CxJSONUTFObject();
        nested->append(new CxJSONUTFMember("inner", new CxJSONUTFString("value")));
        obj.append(new CxJSONUTFMember("outer", nested));

        CxString pretty = obj.toPrettyJsonString();
        // Should have multiple levels of indentation
        check(pretty.index("    ") != -1, "nested object has deeper indentation");
    }
}

//-----------------------------------------------------------------------------------------
// Type checking tests
//-----------------------------------------------------------------------------------------
void testTypeChecking() {
    printf("\n== Type Checking Tests ==\n");

    // Verify all types
    {
        CxJSONUTFString s;
        CxJSONUTFNumber n;
        CxJSONUTFBoolean b;
        CxJSONUTFNull nu;
        CxJSONUTFObject obj;
        CxJSONUTFArray arr;

        check(s.type() == CxJSONUTFBase::STRING, "CxJSONUTFString type is STRING");
        check(n.type() == CxJSONUTFBase::NUMBER, "CxJSONUTFNumber type is NUMBER");
        check(b.type() == CxJSONUTFBase::BOOLEAN, "CxJSONUTFBoolean type is BOOLEAN");
        check(nu.type() == CxJSONUTFBase::JNULL, "CxJSONUTFNull type is JNULL");
        check(obj.type() == CxJSONUTFBase::OBJECT, "CxJSONUTFObject type is OBJECT");
        check(arr.type() == CxJSONUTFBase::ARRAY, "CxJSONUTFArray type is ARRAY");
    }

    // Type preserved through base pointer
    {
        CxJSONUTFBase* items[4];
        items[0] = new CxJSONUTFString("test");
        items[1] = new CxJSONUTFNumber(42);
        items[2] = new CxJSONUTFBoolean(1);
        items[3] = new CxJSONUTFNull();

        check(items[0]->type() == CxJSONUTFBase::STRING, "base ptr string type");
        check(items[1]->type() == CxJSONUTFBase::NUMBER, "base ptr number type");
        check(items[2]->type() == CxJSONUTFBase::BOOLEAN, "base ptr boolean type");
        check(items[3]->type() == CxJSONUTFBase::JNULL, "base ptr null type");

        for (int i = 0; i < 4; i++) delete items[i];
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxJSONUTF Test Suite (UTF-8 Aware JSON)\n");
    printf("=======================================\n");

    testJSONUTFString();
    testJSONUTFNumber();
    testJSONUTFBoolean();
    testJSONUTFNull();
    testJSONUTFMember();
    testJSONUTFObject();
    testJSONUTFArray();
    testSerialization();
    testTypeChecking();

    printf("\n=======================================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
