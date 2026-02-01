//-------------------------------------------------------------------------------------------------
//
// utfstringlist_test.cpp
//
// Test program for CxUTFStringList
//
//-------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cx/editbuffer/utfstringlist.h>

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST_ASSERT(condition, testName) \
    do { \
        if (condition) { \
            testsPassed++; \
            printf("  PASS: %s\n", testName); \
        } else { \
            testsFailed++; \
            printf("  FAIL: %s (line %d)\n", testName, __LINE__); \
        } \
    } while(0)


void test_basic(void)
{
    printf("\n--- Testing basic operations ---\n");

    CxUTFStringList list;
    TEST_ASSERT(list.entries() == 0, "Empty list has 0 entries");
    TEST_ASSERT(list.isEmpty() == 1, "Empty list isEmpty returns true");
    TEST_ASSERT(list.at(0) == 0, "at(0) on empty returns NULL");

    // Append
    CxUTFString s1;
    s1.fromBytes("Line 1", 6, 4);
    list.append(s1);

    TEST_ASSERT(list.entries() == 1, "After append, 1 entry");
    TEST_ASSERT(list.isEmpty() == 0, "After append, not empty");
    TEST_ASSERT(list.at(0)->charCount() == 6, "First line has 6 chars");

    CxUTFString s2;
    s2.fromBytes("Line 2", 6, 4);
    list.append(s2);

    TEST_ASSERT(list.entries() == 2, "After second append, 2 entries");
    TEST_ASSERT(list.at(1)->at(5)->codepoint() == '2', "Second line ends with 2");
}


void test_insert(void)
{
    printf("\n--- Testing insert ---\n");

    CxUTFStringList list;

    CxUTFString s1, s2, s3;
    s1.fromBytes("First", 5, 4);
    s2.fromBytes("Third", 5, 4);
    s3.fromBytes("Second", 6, 4);

    list.append(s1);
    list.append(s2);

    // Insert in middle
    list.insertBefore(1, s3);

    TEST_ASSERT(list.entries() == 3, "After insert, 3 entries");
    TEST_ASSERT(list.at(0)->at(0)->codepoint() == 'F', "First is First");
    TEST_ASSERT(list.at(1)->at(0)->codepoint() == 'S', "Second is Second");
    TEST_ASSERT(list.at(2)->at(0)->codepoint() == 'T', "Third is Third");

    // Insert at start
    CxUTFString s0;
    s0.fromBytes("Zero", 4, 4);
    list.insertBefore(0, s0);

    TEST_ASSERT(list.entries() == 4, "After insert at start, 4 entries");
    TEST_ASSERT(list.at(0)->at(0)->codepoint() == 'Z', "First is Zero");

    // Insert after
    CxUTFString s4;
    s4.fromBytes("After", 5, 4);
    list.insertAfter(1, s4);

    TEST_ASSERT(list.entries() == 5, "After insertAfter, 5 entries");
    TEST_ASSERT(list.at(2)->at(0)->codepoint() == 'A', "Third is After");
}


void test_remove(void)
{
    printf("\n--- Testing remove ---\n");

    CxUTFStringList list;

    CxUTFString s1, s2, s3;
    s1.fromBytes("A", 1, 4);
    s2.fromBytes("B", 1, 4);
    s3.fromBytes("C", 1, 4);

    list.append(s1);
    list.append(s2);
    list.append(s3);

    // Remove middle
    list.removeAt(1);

    TEST_ASSERT(list.entries() == 2, "After remove, 2 entries");
    TEST_ASSERT(list.at(0)->at(0)->codepoint() == 'A', "First is A");
    TEST_ASSERT(list.at(1)->at(0)->codepoint() == 'C', "Second is C");

    // Remove first
    list.removeAt(0);
    TEST_ASSERT(list.entries() == 1, "After remove first, 1 entry");
    TEST_ASSERT(list.at(0)->at(0)->codepoint() == 'C', "First is C");

    // Remove last remaining
    list.removeAt(0);
    TEST_ASSERT(list.entries() == 0, "After remove all, 0 entries");
}


void test_replace(void)
{
    printf("\n--- Testing replace ---\n");

    CxUTFStringList list;

    CxUTFString s1, s2;
    s1.fromBytes("Old", 3, 4);
    s2.fromBytes("New", 3, 4);

    list.append(s1);
    list.replaceAt(0, s2);

    TEST_ASSERT(list.at(0)->at(0)->codepoint() == 'N', "Replaced with New");
}


void test_copy_assignment(void)
{
    printf("\n--- Testing copy and assignment ---\n");

    CxUTFStringList orig;

    CxUTFString s1, s2;
    s1.fromBytes("Line 1", 6, 4);
    s2.fromBytes("Line 2", 6, 4);
    orig.append(s1);
    orig.append(s2);

    // Copy constructor
    CxUTFStringList copy(orig);
    TEST_ASSERT(copy.entries() == 2, "Copy has 2 entries");
    TEST_ASSERT(copy.at(0)->charCount() == 6, "Copy first line has 6 chars");

    // Modify copy, original unchanged
    copy.removeAt(0);
    TEST_ASSERT(copy.entries() == 1, "Copy modified");
    TEST_ASSERT(orig.entries() == 2, "Original unchanged");

    // Assignment
    CxUTFStringList assigned;
    assigned = orig;
    TEST_ASSERT(assigned.entries() == 2, "Assigned has 2 entries");
}


void test_clear(void)
{
    printf("\n--- Testing clear ---\n");

    CxUTFStringList list;

    CxUTFString s;
    s.fromBytes("Test", 4, 4);
    list.append(s);
    list.append(s);

    TEST_ASSERT(list.entries() == 2, "Before clear, 2 entries");

    list.clear();
    TEST_ASSERT(list.entries() == 0, "After clear, 0 entries");
    TEST_ASSERT(list.isEmpty() == 1, "After clear, isEmpty");

    // Can still use after clear
    list.append(s);
    TEST_ASSERT(list.entries() == 1, "After clear and append, 1 entry");
}


void test_preallocated(void)
{
    printf("\n--- Testing preallocated ---\n");

    CxUTFStringList list(100);
    TEST_ASSERT(list.entries() == 0, "Preallocated list has 0 entries");

    CxUTFString s;
    s.fromBytes("Test", 4, 4);
    list.append(s);
    TEST_ASSERT(list.entries() == 1, "Can append to preallocated list");
}


int main(int argc, char **argv)
{
    printf("=== CxUTFStringList Test Suite ===\n");

    test_basic();
    test_insert();
    test_remove();
    test_replace();
    test_copy_assignment();
    test_clear();
    test_preallocated();

    printf("\n=================================\n");
    printf("Tests passed: %d\n", testsPassed);
    printf("Tests failed: %d\n", testsFailed);
    printf("=================================\n");

    return testsFailed > 0 ? 1 : 0;
}
