/***************************************************************************
 *
 *  cxhashmap_test.cpp
 *
 *  Test suite for CxHashmap template class
 *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include "cx/base/hashmap.h"

static int g_passed = 0;
static int g_failed = 0;

void check(int condition, const char* testName)
{
    if (condition) {
        printf("  PASS: %s\n", testName);
        g_passed++;
    } else {
        printf("  FAIL: %s\n", testName);
        g_failed++;
    }
}

//-------------------------------------------------------------------------
// IntKey - simple integer key for testing
//
//-------------------------------------------------------------------------
class IntKey
{
public:
    int _value;

    IntKey() : _value(0) {}
    IntKey(int v) : _value(v) {}
    IntKey(const IntKey& other) : _value(other._value) {}

    unsigned int hashValue() const {
        // Simple hash for integers
        unsigned int h = (unsigned int)_value;
        h ^= (h >> 16);
        h *= 0x85ebca6b;
        h ^= (h >> 13);
        return h;
    }

    bool operator==(const IntKey& other) const {
        return _value == other._value;
    }
};

//-------------------------------------------------------------------------
// StringKey - string key for testing
//
//-------------------------------------------------------------------------
class StringKey
{
public:
    char _str[256];

    StringKey() { _str[0] = '\0'; }

    StringKey(const char* s) {
        if (s) {
            strncpy(_str, s, 255);
            _str[255] = '\0';
        } else {
            _str[0] = '\0';
        }
    }

    StringKey(const StringKey& other) {
        strncpy(_str, other._str, 255);
        _str[255] = '\0';
    }

    unsigned int hashValue() const {
        // djb2 hash
        unsigned int hash = 5381;
        const char* p = _str;
        while (*p) {
            hash = ((hash << 5) + hash) + (unsigned char)*p;
            p++;
        }
        return hash;
    }

    bool operator==(const StringKey& other) const {
        return strcmp(_str, other._str) == 0;
    }
};

//-------------------------------------------------------------------------
// IntEntry - simple integer entry for testing
//
//-------------------------------------------------------------------------
class IntEntry
{
public:
    int _value;

    IntEntry() : _value(0) {}
    IntEntry(int v) : _value(v) {}
    IntEntry(const IntEntry& other) : _value(other._value) {}
};

//-------------------------------------------------------------------------
// StringEntry - string entry for testing
//
//-------------------------------------------------------------------------
class StringEntry
{
public:
    char _str[256];

    StringEntry() { _str[0] = '\0'; }

    StringEntry(const char* s) {
        if (s) {
            strncpy(_str, s, 255);
            _str[255] = '\0';
        } else {
            _str[0] = '\0';
        }
    }

    StringEntry(const StringEntry& other) {
        strncpy(_str, other._str, 255);
        _str[255] = '\0';
    }
};


//=========================================================================
// Constructor Tests
//=========================================================================
void testConstructor()
{
    printf("\n== CxHashmap Constructor Tests ==\n");

    // Default constructor
    {
        CxHashmap<IntKey, IntEntry> map;
        check(map.getSize() == 0, "default constructor: size is 0");
    }

    // Constructor with size
    {
        CxHashmap<IntKey, IntEntry> map(64);
        check(map.getSize() == 0, "sized constructor: size is 0");
    }

    // Constructor with small size
    {
        CxHashmap<IntKey, IntEntry> map(8);
        check(map.getSize() == 0, "small sized constructor: size is 0");
    }
}


//=========================================================================
// Insert Tests
//=========================================================================
void testInsert()
{
    printf("\n== CxHashmap Insert Tests ==\n");

    CxHashmap<IntKey, IntEntry> map;

    // Insert first item
    bool replaced = map.insert(IntKey(1), IntEntry(100));
    check(!replaced, "insert first: returns false (no replacement)");
    check(map.getSize() == 1, "insert first: size is 1");

    // Insert second item
    replaced = map.insert(IntKey(2), IntEntry(200));
    check(!replaced, "insert second: returns false");
    check(map.getSize() == 2, "insert second: size is 2");

    // Insert with same key (replacement)
    replaced = map.insert(IntKey(1), IntEntry(101));
    check(replaced, "insert duplicate: returns true (replaced)");
    check(map.getSize() == 2, "insert duplicate: size unchanged");

    // Verify replacement value
    const IntEntry* found = map.find(IntKey(1));
    check(found != 0 && found->_value == 101, "insert duplicate: value updated");
}


//=========================================================================
// Find Tests
//=========================================================================
void testFind()
{
    printf("\n== CxHashmap Find Tests ==\n");

    CxHashmap<IntKey, IntEntry> map;

    // Find in empty map
    const IntEntry* found = map.find(IntKey(1));
    check(found == 0, "find in empty: returns NULL");

    // Insert and find
    map.insert(IntKey(42), IntEntry(4200));
    found = map.find(IntKey(42));
    check(found != 0, "find existing: returns non-NULL");
    check(found != 0 && found->_value == 4200, "find existing: correct value");

    // Find non-existent
    found = map.find(IntKey(99));
    check(found == 0, "find non-existent: returns NULL");

    // Multiple items
    map.insert(IntKey(10), IntEntry(1000));
    map.insert(IntKey(20), IntEntry(2000));
    map.insert(IntKey(30), IntEntry(3000));

    found = map.find(IntKey(10));
    check(found != 0 && found->_value == 1000, "find multiple: key 10");
    found = map.find(IntKey(20));
    check(found != 0 && found->_value == 2000, "find multiple: key 20");
    found = map.find(IntKey(30));
    check(found != 0 && found->_value == 3000, "find multiple: key 30");
    found = map.find(IntKey(42));
    check(found != 0 && found->_value == 4200, "find multiple: key 42 still works");
}


//=========================================================================
// Operator[] Tests
//=========================================================================
void testOperatorBracket()
{
    printf("\n== CxHashmap operator[] Tests ==\n");

    CxHashmap<IntKey, IntEntry> map;

    // Access creates entry
    map[IntKey(5)]._value = 500;
    check(map.getSize() == 1, "operator[]: creates entry, size is 1");

    const IntEntry* found = map.find(IntKey(5));
    check(found != 0 && found->_value == 500, "operator[]: value stored correctly");

    // Access existing entry
    map[IntKey(5)]._value = 555;
    check(map.getSize() == 1, "operator[] existing: size unchanged");
    found = map.find(IntKey(5));
    check(found != 0 && found->_value == 555, "operator[] existing: value updated");

    // Multiple entries via operator[]
    map[IntKey(1)]._value = 100;
    map[IntKey(2)]._value = 200;
    map[IntKey(3)]._value = 300;
    check(map.getSize() == 4, "operator[] multiple: size is 4");
}


//=========================================================================
// Iterator Tests
//=========================================================================
void testIterator()
{
    printf("\n== CxHashmapIterator Tests ==\n");

    CxHashmap<IntKey, IntEntry> map;

    // Iterator on empty map
    {
        CxHashmapIterator<IntKey, IntEntry> iter(&map);
        check(!iter.next(), "iterator empty: next() returns false");
    }

    // Add items and iterate
    map.insert(IntKey(1), IntEntry(100));
    map.insert(IntKey(2), IntEntry(200));
    map.insert(IntKey(3), IntEntry(300));

    {
        CxHashmapIterator<IntKey, IntEntry> iter(&map);
        int count = 0;
        int sum = 0;
        while (iter.next()) {
            count++;
            const IntKey* k = iter.getKey();
            IntEntry* e = iter.getEntry();
            if (k && e) {
                sum += e->_value;
            }
        }
        check(count == 3, "iterator: visits 3 items");
        check(sum == 600, "iterator: sum of values is 600");
    }

    // getKey and getEntry before next()
    {
        CxHashmapIterator<IntKey, IntEntry> iter(&map);
        check(iter.getKey() == 0, "iterator: getKey before next() returns NULL");
        check(iter.getEntry() == 0, "iterator: getEntry before next() returns NULL");
    }
}


//=========================================================================
// String Key/Entry Tests
//=========================================================================
void testStringKeyEntry()
{
    printf("\n== CxHashmap String Key/Entry Tests ==\n");

    CxHashmap<StringKey, StringEntry> map;

    // Insert string entries
    map.insert(StringKey("apple"), StringEntry("red fruit"));
    map.insert(StringKey("banana"), StringEntry("yellow fruit"));
    map.insert(StringKey("cherry"), StringEntry("small red fruit"));

    check(map.getSize() == 3, "string map: size is 3");

    // Find string entries
    const StringEntry* found = map.find(StringKey("apple"));
    check(found != 0 && strcmp(found->_str, "red fruit") == 0, "string find: apple");

    found = map.find(StringKey("banana"));
    check(found != 0 && strcmp(found->_str, "yellow fruit") == 0, "string find: banana");

    found = map.find(StringKey("cherry"));
    check(found != 0 && strcmp(found->_str, "small red fruit") == 0, "string find: cherry");

    // Find non-existent
    found = map.find(StringKey("grape"));
    check(found == 0, "string find: non-existent returns NULL");

    // operator[] with strings
    map[StringKey("date")]._str[0] = '\0';
    strncpy(map[StringKey("date")]._str, "sweet fruit", 255);
    check(map.getSize() == 4, "string operator[]: size is 4");
}


//=========================================================================
// Resize Tests (triggered by adding many items)
//=========================================================================
void testResize()
{
    printf("\n== CxHashmap Resize Tests ==\n");

    // Start with small map to force resize
    CxHashmap<IntKey, IntEntry> map(8);

    // Insert enough items to trigger resize
    for (int i = 0; i < 100; i++) {
        map.insert(IntKey(i), IntEntry(i * 10));
    }

    check(map.getSize() == 100, "resize: 100 items inserted");

    // Verify all items still accessible
    int found_count = 0;
    for (int i = 0; i < 100; i++) {
        const IntEntry* found = map.find(IntKey(i));
        if (found && found->_value == i * 10) {
            found_count++;
        }
    }
    check(found_count == 100, "resize: all 100 items findable");

    // Iterator should still work
    CxHashmapIterator<IntKey, IntEntry> iter(&map);
    int iter_count = 0;
    while (iter.next()) {
        iter_count++;
    }
    check(iter_count == 100, "resize: iterator visits 100 items");
}


//=========================================================================
// Collision Tests (keys with same hash bucket)
//=========================================================================
void testCollisions()
{
    printf("\n== CxHashmap Collision Tests ==\n");

    // Use small map to increase collision probability
    CxHashmap<IntKey, IntEntry> map(8);

    // Insert items that may collide
    map.insert(IntKey(0), IntEntry(0));
    map.insert(IntKey(8), IntEntry(80));    // Likely same bucket as 0 in size-8 map
    map.insert(IntKey(16), IntEntry(160));  // Likely same bucket
    map.insert(IntKey(24), IntEntry(240));  // Likely same bucket

    check(map.getSize() == 4, "collision: all 4 items stored");

    // Verify each item is findable
    const IntEntry* f0 = map.find(IntKey(0));
    const IntEntry* f8 = map.find(IntKey(8));
    const IntEntry* f16 = map.find(IntKey(16));
    const IntEntry* f24 = map.find(IntKey(24));

    check(f0 != 0 && f0->_value == 0, "collision: find key 0");
    check(f8 != 0 && f8->_value == 80, "collision: find key 8");
    check(f16 != 0 && f16->_value == 160, "collision: find key 16");
    check(f24 != 0 && f24->_value == 240, "collision: find key 24");
}


//=========================================================================
// Edge Cases
//=========================================================================
void testEdgeCases()
{
    printf("\n== CxHashmap Edge Case Tests ==\n");

    // Zero key
    {
        CxHashmap<IntKey, IntEntry> map;
        map.insert(IntKey(0), IntEntry(999));
        const IntEntry* found = map.find(IntKey(0));
        check(found != 0 && found->_value == 999, "edge: zero key works");
    }

    // Negative key values
    {
        CxHashmap<IntKey, IntEntry> map;
        map.insert(IntKey(-1), IntEntry(100));
        map.insert(IntKey(-100), IntEntry(200));
        map.insert(IntKey(-999), IntEntry(300));

        check(map.getSize() == 3, "edge: negative keys stored");

        const IntEntry* found = map.find(IntKey(-1));
        check(found != 0 && found->_value == 100, "edge: find negative key -1");
        found = map.find(IntKey(-100));
        check(found != 0 && found->_value == 200, "edge: find negative key -100");
    }

    // Empty string key
    {
        CxHashmap<StringKey, StringEntry> map;
        map.insert(StringKey(""), StringEntry("empty key value"));
        const StringEntry* found = map.find(StringKey(""));
        check(found != 0 && strcmp(found->_str, "empty key value") == 0,
              "edge: empty string key works");
    }

    // Single character keys
    {
        CxHashmap<StringKey, StringEntry> map;
        map.insert(StringKey("a"), StringEntry("value a"));
        map.insert(StringKey("b"), StringEntry("value b"));
        map.insert(StringKey("c"), StringEntry("value c"));
        check(map.getSize() == 3, "edge: single char keys stored");
    }

    // Large number of items
    {
        CxHashmap<IntKey, IntEntry> map;
        for (int i = 0; i < 1000; i++) {
            map.insert(IntKey(i), IntEntry(i));
        }
        check(map.getSize() == 1000, "edge: 1000 items stored");

        // Spot check
        const IntEntry* f500 = map.find(IntKey(500));
        check(f500 != 0 && f500->_value == 500, "edge: find item 500 of 1000");
    }
}


//=========================================================================
// getSize Tests
//=========================================================================
void testGetSize()
{
    printf("\n== CxHashmap getSize Tests ==\n");

    CxHashmap<IntKey, IntEntry> map;

    check(map.getSize() == 0, "getSize: empty map is 0");

    map.insert(IntKey(1), IntEntry(1));
    check(map.getSize() == 1, "getSize: after 1 insert is 1");

    map.insert(IntKey(2), IntEntry(2));
    map.insert(IntKey(3), IntEntry(3));
    check(map.getSize() == 3, "getSize: after 3 inserts is 3");

    // Replacement doesn't change size
    map.insert(IntKey(1), IntEntry(100));
    check(map.getSize() == 3, "getSize: replacement doesn't increase");

    // operator[] creates new entry
    map[IntKey(4)]._value = 4;
    check(map.getSize() == 4, "getSize: operator[] new key increases");

    // operator[] on existing doesn't change size
    map[IntKey(4)]._value = 40;
    check(map.getSize() == 4, "getSize: operator[] existing key unchanged");
}


//=========================================================================
// Mixed Key Types in Same Program
//=========================================================================
void testMixedTypes()
{
    printf("\n== CxHashmap Mixed Types Tests ==\n");

    // Int to Int
    CxHashmap<IntKey, IntEntry> intMap;
    intMap.insert(IntKey(1), IntEntry(100));

    // String to String
    CxHashmap<StringKey, StringEntry> strMap;
    strMap.insert(StringKey("key"), StringEntry("value"));

    // Int to String
    CxHashmap<IntKey, StringEntry> intStrMap;
    intStrMap.insert(IntKey(42), StringEntry("forty-two"));

    // String to Int
    CxHashmap<StringKey, IntEntry> strIntMap;
    strIntMap.insert(StringKey("answer"), IntEntry(42));

    check(intMap.getSize() == 1, "mixed: int->int map works");
    check(strMap.getSize() == 1, "mixed: string->string map works");
    check(intStrMap.getSize() == 1, "mixed: int->string map works");
    check(strIntMap.getSize() == 1, "mixed: string->int map works");

    const StringEntry* se = intStrMap.find(IntKey(42));
    check(se != 0 && strcmp(se->_str, "forty-two") == 0, "mixed: int->string lookup");

    const IntEntry* ie = strIntMap.find(StringKey("answer"));
    check(ie != 0 && ie->_value == 42, "mixed: string->int lookup");
}


//=========================================================================
// Main
//=========================================================================
int main()
{
    printf("CxHashmap Test Suite\n");
    printf("====================\n");

    testConstructor();
    testInsert();
    testFind();
    testOperatorBracket();
    testIterator();
    testStringKeyEntry();
    testResize();
    testCollisions();
    testEdgeCases();
    testGetSize();
    testMixedTypes();

    printf("\n====================\n");
    printf("Results: %d passed, %d failed\n", g_passed, g_failed);

    return g_failed > 0 ? 1 : 0;
}
